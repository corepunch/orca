#include <include/api.h>
#include <include/orca.h>

#include <source/core/core_local.h>
#include <source/core/core_properties.h>

// ============================================================================
// STYLE SYSTEM
// ============================================================================
//
// StyleSheet: standalone container object holding StyleRule children.
//   - Stored in StyleController.StyleSheet (per-object) or static_stylesheet (global).
//
// StyleRule: child object of a StyleSheet.
//   - Selector: CSS selector string (e.g., ".button:hover")
//   - class_id / flags / opacity: cached from Selector
//   - Attached C properties (ruleObj->properties with !PF_PROPERTY_TYPE):
//     the pre-parsed CSS property overrides.  _ApplyStyleRule copies these
//     directly to the target — no string parsing on every ThemeChanged.
//
// style_class_selector: per-object parsed class token list (StyleController.classes).
//

// Global stylesheet (applies before per-object stylesheets)
static lpObject_t static_stylesheet = NULL;

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

// Parse pseudo-state flags from a colon-separated class string.
static uint32_t
_ParsePseudoStateFlags(lpcString_t str)
{
  uint32_t flags = 0;
  lpcString_t token = strchr(str, ':');
  while (token) {
    lpcString_t start = token + 1;
    lpcString_t end = strchr(start, ':');
    size_t len = end ? (size_t)(end - start) : strlen(start);
    if      (len == 5 && !strncmp(start, "hover",  5)) flags |= STYLE_HOVER;
    else if (len == 5 && !strncmp(start, "focus",  5)) flags |= STYLE_FOCUS;
    else if (len == 6 && !strncmp(start, "active", 6)) flags |= STYLE_SELECT;
    else if (len == 4 && !strncmp(start, "dark",   4)) flags |= STYLE_DARK;
    token = end;
  }
  return flags;
}

// Parse a single style class token (e.g., "blue/50:hover", "btn:hover:focus")
// Returns a newly allocated style_class_selector.
struct style_class_selector*
_ParseClass(lpcString_t str)
{
  struct style_class_selector* cls = ZeroAlloc(sizeof(struct style_class_selector));
  cls->flags = _ParsePseudoStateFlags(str);

  lpcString_t szOpacity = strchr(str, '/');
  lpcString_t szState   = strchr(str, ':');
  lpcString_t szEnd;
  if (szOpacity && szState) {
    szEnd = (szOpacity < szState) ? szOpacity : szState;
  } else if (szOpacity) {
    szEnd = szOpacity;
  } else {
    szEnd = szState;
  }

  size_t len = szEnd ? (size_t)(szEnd - str) : strlen(str);
  len = MIN(len, sizeof(shortStr_t) - 1);
  memcpy(cls->value, str, len);
  cls->value[len] = '\0';

  cls->opacity = szOpacity ? (byte_t)atoi(szOpacity + 1) : 100;
  return cls;
}

// Append a parsed style_class_selector to the object's class list
void
_AddClass(lpObject_t obj, struct style_class_selector* cls)
{
  struct StyleController* sc = GetStyleController(obj);
  if (!sc) { free(cls); return; }
  ADD_TO_LIST_END(struct style_class_selector, cls, sc->classes);
  if (cls->flags & STYLE_HOVER) {
    OBJ_SetFlags(obj, OBJ_GetFlags(obj) | OF_HOVERABLE);
  }
}

// Get (or create) the StyleSheet for obj (or global when obj==NULL)
static lpObject_t
_GetOrCreateStyleSheet(lpObject_t obj)
{
  if (!obj) {
    if (!static_stylesheet) {
      static_stylesheet = OBJ_MakeNativeObject(ID_StyleSheet);
    }
    return static_stylesheet;
  } else {
    struct StyleController* sc = GetStyleController(obj);
    if (!sc) return NULL;
    if (!sc->StyleSheet) {
      lpObject_t sheetObj = OBJ_MakeNativeObject(ID_StyleSheet);
      sc->StyleSheet = GetStyleSheet(sheetObj);
    }
    return CMP_GetObject(sc->StyleSheet);
  }
}

// Find a PropertyType descriptor by property name.
// Tries full name first (e.g. "Node2D.BackgroundColor") then short name ("BackgroundColor").
static lpcPropertyType_t
_FindPropertyTypeByName(lpcString_t name)
{
  // Try FullIdentifier (dotted long name)
  lpcPropertyType_t pt = OBJ_FindPropertyType(fnv1a32(name));
  if (pt) return pt;

  // Try ShortIdentifier when there's no dot in the name
  if (!strchr(name, '.')) {
    uint32_t short_id = fnv1a32(name);
    for (int i = 0; i < MAX_CLASSES; i++) {
      lpcClassDesc_t cls = core.classes[i];
      if (!cls) continue;
      for (uint32_t j = 0; j < cls->NumProperties; j++) {
        if (cls->Properties[j].ShortIdentifier == short_id) {
          return &cls->Properties[j];
        }
      }
    }
  }
  return NULL;
}

// Create a pre-parsed property override on ruleObj for (name, value) from a C string.
// Uses parse_property to convert the string to the native type once.
static void
_AddStyleRulePropertyFromString(lpObject_t ruleObj, lpcString_t name, lpcString_t value)
{
  lpcPropertyType_t pt = _FindPropertyTypeByName(name);
  if (!pt) return;
  lpProperty_t prop = PROP_Create(NULL, ruleObj, pt);
  char buf[MAX_PROPERTY_STRING] = {0};
  parse_property(core.L, value, pt, buf);
  PROP_SetValue(prop, buf);
}

// Release a StyleRule native object (and its string Selector field).
static void
_ReleaseNativeStyleRule(lpObject_t ruleObj)
{
  if (!ruleObj) return;
  struct StyleRule* sr = GetStyleRule(ruleObj);
  if (sr) {
    // Free Selector string if it was set directly (not via property system)
    // In practice Selector is set via PROP_Create; OBJ_ReleaseProperties frees it.
  }
  OBJ_ReleaseProperties(ruleObj);
  OBJ_ReleaseComponents(ruleObj);
  free(ruleObj);
}

// Release a StyleSheet native object and all its StyleRule children.
static void
_ReleaseNativeStyleSheet(lpObject_t sheetObj)
{
  if (!sheetObj) return;
  lpObject_t child = OBJ_GetFirstChild(sheetObj);
  while (child) {
    lpObject_t next = OBJ_GetNext(child);
    _ReleaseNativeStyleRule(child);
    child = next;
  }
  OBJ_ReleaseProperties(sheetObj);
  OBJ_ReleaseComponents(sheetObj);
  free(sheetObj);
}

// ============================================================================
// PUBLIC STYLE API
// ============================================================================

ORCA_API int
parse_property(lua_State* L, const char* str, struct PropertyType const* prop, void* valueptr);

// Create a StyleRule from a selector + single property/value pair (C API).
void
OBJ_AddStyleClass(lpObject_t obj,
                  lpcString_t selector,
                  lpcString_t property,
                  lpcString_t value,
                  uint32_t flags)
{
  lpObject_t sheetObj = _GetOrCreateStyleSheet(obj);
  if (!sheetObj) return;

  lpObject_t ruleObj = OBJ_MakeNativeObject(ID_StyleRule);
  struct StyleRule* sr = GetStyleRule(ruleObj);

  // Set Selector via the property system so OBJ_ReleaseProperties frees the string.
  lpProperty_t selectorProp = NULL;
  if (SUCCEEDED(OBJ_FindLongProperty(ruleObj, ID_StyleRule_Selector, &selectorProp))) {
    PROP_SetStringValue(selectorProp, selector);
  }

  // Cache class_id / flags / opacity from the selector
  lpcString_t base = (*selector == '.') ? selector + 1 : selector;
  lpcString_t colon = strchr(base, ':');
  lpcString_t slash = strchr(base, '/');
  lpcString_t end = NULL;
  if (colon && slash) end = (colon < slash) ? colon : slash;
  else if (colon)     end = colon;
  else if (slash)     end = slash;

  if (end) {
    char buf[sizeof(shortStr_t)];
    size_t len = MIN((size_t)(end - base), sizeof(buf) - 1);
    memcpy(buf, base, len);
    buf[len] = '\0';
    sr->class_id = fnv1a32(buf);
  } else {
    sr->class_id = fnv1a32(base);
  }
  sr->flags   = flags ? flags : _ParsePseudoStateFlags(selector);
  sr->opacity = slash ? (byte_t)atoi(slash + 1) : 100;

  // Attach the pre-parsed property override
  _AddStyleRulePropertyFromString(ruleObj, property, value);

  OBJ_AddChild(sheetObj, ruleObj, FALSE);
}

// Create StyleRule objects from a Lua table of CSS rules.
// selector: CSS selector string; table at Lua stack index 3.
void OBJ_AddStyleRule(lua_State* L, lpObject_t self, lpcString_t selector)
{
  if (lua_type(L, 3) != LUA_TTABLE) {
    lua_error(L);
    return;
  }

  lpObject_t sheetObj = _GetOrCreateStyleSheet(self);
  if (!sheetObj) return;

  lpObject_t ruleObj = OBJ_MakeNativeObject(ID_StyleRule);
  struct StyleRule* sr = GetStyleRule(ruleObj);

  // Set Selector
  lpProperty_t selectorProp = NULL;
  if (SUCCEEDED(OBJ_FindLongProperty(ruleObj, ID_StyleRule_Selector, &selectorProp))) {
    PROP_SetStringValue(selectorProp, selector);
  }

  // Cache class_id / flags / opacity
  lpcString_t base = (*selector == '.') ? selector + 1 : selector;
  lpcString_t colon = strchr(base, ':');
  lpcString_t slash = strchr(base, '/');
  lpcString_t end = NULL;
  if (colon && slash) end = (colon < slash) ? colon : slash;
  else if (colon)     end = colon;
  else if (slash)     end = slash;
  if (end) {
    char buf[sizeof(shortStr_t)];
    size_t len = MIN((size_t)(end - base), sizeof(buf) - 1);
    memcpy(buf, base, len);
    buf[len] = '\0';
    sr->class_id = fnv1a32(buf);
  } else {
    sr->class_id = fnv1a32(base);
  }
  sr->flags   = _ParsePseudoStateFlags(selector);
  sr->opacity = slash ? (byte_t)atoi(slash + 1) : 100;

  // Iterate the Lua table and attach one C property per override
  lua_pushvalue(L, 3);
  lua_pushnil(L);
  while (lua_next(L, -2)) {
    if (lua_type(L, -2) != LUA_TSTRING) { lua_pop(L, 1); continue; }
    lpcString_t key = lua_tostring(L, -2);
    uint32_t keyID  = fnv1a32(key);

    if (keyID == CSS_APPLY) {
      // @apply: copy property overrides from another selector
      LPSTR str = strdup(luaL_checkstring(L, -1));
      for (lpcString_t s = strtok(str, " "); s; s = strtok(NULL, " ")) {
        struct style_class_selector* cls = _ParseClass(s);
        // Search global + per-object stylesheets for matching rules
        uint32_t applyID = fnv1a32(cls->value);
        // helper lambda not available in C; inline the search
        for (lpObject_t src = static_stylesheet; src; src = NULL) {
          FOR_EACH_OBJECT(child, src) {
            struct StyleRule* csr = GetStyleRule(child);
            if (!csr || csr->class_id != applyID) continue;
            // Copy property overrides from child to ruleObj
            for (lpProperty_t rp = OBJ_GetProperties(child); rp; rp = PROP_GetNext(rp)) {
              if (PROP_GetFlags(rp) & PF_PROPERTY_TYPE) continue;
              if (PROP_IsNull(rp)) continue;
              lpProperty_t cp = PROP_Create(NULL, ruleObj, PROP_GetDesc(rp));
              PROP_SetValue(cp, PROP_GetValue(rp));
            }
          }
        }
        if (self) {
          struct StyleController* sc = GetStyleController(self);
          if (sc && sc->StyleSheet) {
            lpObject_t lsh = CMP_GetObject(sc->StyleSheet);
            FOR_EACH_OBJECT(child, lsh) {
              struct StyleRule* csr = GetStyleRule(child);
              if (!csr || csr->class_id != applyID) continue;
              for (lpProperty_t rp = OBJ_GetProperties(child); rp; rp = PROP_GetNext(rp)) {
                if (PROP_GetFlags(rp) & PF_PROPERTY_TYPE) continue;
                if (PROP_IsNull(rp)) continue;
                lpProperty_t cp = PROP_Create(NULL, ruleObj, PROP_GetDesc(rp));
                PROP_SetValue(cp, PROP_GetValue(rp));
              }
            }
          }
        }
        free(cls);
      }
      free(str);
      lua_pop(L, 1);
      continue;
    }

    lpcPropertyType_t pt = _FindPropertyTypeByName(key);
    if (pt) {
      lpProperty_t prop = PROP_Create(NULL, ruleObj, pt);
      luaX_readProperty(L, -1, prop);
    }
    lua_pop(L, 1);
  }
  lua_pop(L, 1); // pop the table copy

  OBJ_AddChild(sheetObj, ruleObj, FALSE);
}

// ============================================================================
// STYLE RESOLUTION
// ============================================================================

// Return the current pseudo-state flags for an object (hover, focus, dark, selected)
uint32_t
OBJ_GetStyleFlags(lpObject_t pobj)
{
  uint32_t dwValue = 0;
  if (core_GetHover() == pobj) dwValue |= STYLE_HOVER;
  if (core_GetFocus() == pobj) dwValue |= STYLE_FOCUS;
  if (OBJ_GetFlags(pobj) & OF_SELECTED) dwValue |= STYLE_SELECT;
  if (axIsDarkTheme()) dwValue |= STYLE_DARK;
  return dwValue;
}

// Apply all C property overrides from ruleObj to the target object.
// ruleObj->properties contains the pre-parsed overrides (PF_PROPERTY_TYPE not set).
// ruleFlags: the rule's state flags (0 = default, non-zero = pseudo-state rule).
// cls: the resolved class selector (may be NULL for "body" rules).
static void
_ApplyStyleRule(lpObject_t target, lpObject_t ruleObj,
                uint32_t ruleFlags, struct style_class_selector* cls)
{
  for (lpProperty_t rp = OBJ_GetProperties(ruleObj); rp; rp = PROP_GetNext(rp)) {
    // Skip StyleRule's own component properties (Selector, etc.)
    if (PROP_GetFlags(rp) & PF_PROPERTY_TYPE) continue;
    if (PROP_IsNull(rp)) continue;

    lpcPropertyType_t pdesc = PROP_GetDesc(rp);
    if (!pdesc || !pdesc->Name) continue;

    lpProperty_t hprop = NULL;
    if (FAILED(OBJ_FindShortProperty(target, pdesc->Name, &hprop))) continue;

    if (ruleFlags) {
      PROP_SetFlag(hprop, PF_SPECIALIZED);
    } else if (PROP_GetFlags(hprop) & PF_SPECIALIZED) {
      // A state-specific rule (e.g., :hover) was already applied — skip the default
      continue;
    }

    PROP_SetValue(hprop, PROP_GetValue(rp));

    // Apply opacity to color properties
    if (PROP_GetType(hprop) == kDataTypeColor && cls && cls->opacity != 100) {
      struct color clr;
      PROP_CopyValue(hprop, &clr);
      clr.a = cls->opacity / 100.f;
      PROP_SetValue(hprop, &clr);
    }
  }
}

// Walk one stylesheet's rules for a matching selector and invoke _ApplyStyleRule.
static void
_EnumSheetRules(lpObject_t sheetObj, lpObject_t target,
                uint32_t dwClassID, uint32_t objFlags,
                struct style_class_selector* cls)
{
  if (!sheetObj) return;
  FOR_EACH_OBJECT(child, sheetObj) {
    struct StyleRule* sr = GetStyleRule(child);
    if (!sr || sr->class_id != dwClassID) continue;
    if (sr->flags & STYLE_HOVER) OBJ_SetFlags(target, OF_HOVERABLE);
    if ((objFlags & sr->flags) == sr->flags) {
      _ApplyStyleRule(target, child, sr->flags, cls);
    }
  }
}

// Walk global and per-object stylesheets for rules that match classname,
// and invoke _ApplyStyleRule for each matching rule.
void
OBJ_EnumStyleClasses(lpObject_t pobj,
                     lpcString_t classname,
                     struct style_class_selector* cls)
{
  uint32_t dwClassID = fnv1a32(classname);
  uint32_t objFlags  = OBJ_GetStyleFlags(pobj);

  // Global stylesheet first
  _EnumSheetRules(static_stylesheet, pobj, dwClassID, objFlags, cls);

  // Walk up the object hierarchy
  for (lpObject_t p = pobj; p; p = OBJ_GetParent(p)) {
    struct StyleController* sc = GetStyleController(p);
    if (!sc || !sc->StyleSheet) continue;
    _EnumSheetRules(CMP_GetObject(sc->StyleSheet), pobj, dwClassID, objFlags, cls);
  }
}

// Free all style classes and stylesheet rules owned by this object's StyleController.
void
OBJ_ClearStyleClasses(lpObject_t pobj)
{
  struct StyleController* sc = GetStyleController(pobj);
  if (!sc) return;
  FOR_EACH_LIST(struct style_class_selector, cls, sc->classes) free(cls);
  sc->classes = NULL;

  if (sc->StyleSheet) {
    _ReleaseNativeStyleSheet(CMP_GetObject(sc->StyleSheet));
    sc->StyleSheet = NULL;
  }
}

// ============================================================================
// STYLECONTROLLER COMPONENT HANDLERS
// ============================================================================

HANDLER(StyleController, Object, Create) {
  pStyleController->classes = NULL;
  return FALSE;
}

HANDLER(StyleController, Object, Release) {
  OBJ_ClearStyleClasses(hObject);
  return FALSE;
}

// Recalculate and apply all active style rules to this object.
HANDLER(StyleController, StyleController, ThemeChanged) {
  bool_t recursive = pThemeChanged ? pThemeChanged->recursive : FALSE;

  PROP_ClearSpecialized(OBJ_GetProperties(hObject));

  // Root objects: apply "body" rules from the local stylesheet
  if (!OBJ_GetParent(hObject) && pStyleController->StyleSheet) {
    uint32_t body_id = fnv1a32("body");
    lpObject_t sheetObj = CMP_GetObject(pStyleController->StyleSheet);
    FOR_EACH_OBJECT(child, sheetObj) {
      struct StyleRule* sr = GetStyleRule(child);
      if (sr && sr->class_id == body_id) {
        _ApplyStyleRule(hObject, child, 0,
                        &(struct style_class_selector){ .opacity = 100 });
      }
    }
  }

  // Apply each class whose state flags match the current object state
  FOR_EACH_LIST(struct style_class_selector, cls, pStyleController->classes) {
    if ((OBJ_GetStyleFlags(hObject) & cls->flags) == cls->flags) {
      OBJ_EnumStyleClasses(hObject, cls->value, cls);
    }
  }

  if (recursive) {
    FOR_EACH_OBJECT(child, hObject) {
      _SendMessage(child, StyleController, ThemeChanged, .recursive = TRUE);
    }
  }

  return FALSE;
}

HANDLER(StyleController, StyleController, AddClass)
{
  _AddClass(hObject, _ParseClass(pAddClass->ClassName));
  return TRUE;
}

HANDLER(StyleController, StyleController, AddClasses)
{
  WITH(char, classes, strdup(pAddClasses->ClassNames), free)
  {
    for (lpcString_t s = strtok(classes, " "); s; s = strtok(NULL, " ")) {
      _AddClass(hObject, _ParseClass(s));
    }
  }
  return TRUE;
}
