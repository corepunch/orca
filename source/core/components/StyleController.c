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
//   - ClassName:  the base class name (e.g., "button") — matched against the
//                 object's class list.
//   - PseudoClass: colon-separated pseudo-states (e.g., "hover:focus") —
//                 empty means the rule applies to all states.
//   - class_id / flags: cached from ClassName / PseudoClass
//   - Attached C properties (ruleObj->properties with !PF_PROPERTY_TYPE):
//     the pre-parsed CSS property overrides.  _ApplyStyleRule copies these
//     directly to the target — no string parsing on every ThemeChanged.
//
// style_class_selector: per-object parsed class token list (StyleController.classes).
//   - opacity: per-usage opacity from "/N" syntax (e.g., class="text-color/50").
//

// Global stylesheet (applies before per-object stylesheets)
/*static*/ struct Object *static_stylesheet = NULL;

// Forward declarations for classes defined in core_export.c (needed for lazy
// early registration before luaopen_orca_core runs, e.g. from tailwind.lua)
extern struct ClassDesc _StyleSheet;
extern struct ClassDesc _StyleRule;

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

// Parse pseudo-state flags from a colon-separated class string.
static uint32_t
_PseudoFlagFromToken(lpcString_t token, size_t len)
{
  if      (len == 5 && !strncmp(token, "hover",  5)) return STYLE_HOVER;
  else if (len == 5 && !strncmp(token, "focus",  5)) return STYLE_FOCUS;
  else if (len == 6 && !strncmp(token, "active", 6)) return STYLE_SELECT;
  else if (len == 4 && !strncmp(token, "dark",   4)) return STYLE_DARK;
  return 0;
}

static uint32_t
_ParsePseudoStateFlags(lpcString_t str)
{
  uint32_t flags = 0;
  for (lpcString_t tok = str; tok && *tok; ) {
    lpcString_t end = strchr(tok, ':');
    size_t len = end ? (size_t)(end - tok) : strlen(tok);
    flags |= _PseudoFlagFromToken(tok, len);
    tok = end ? end + 1 : NULL;
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

  // Support both suffix pseudo syntax (bg-blue:hover) and Tailwind-like
  // prefix syntax (hover:bg-blue). We pick the non-pseudo segment as class.
  lpcString_t baseStart = str;
  size_t baseLen = strlen(str);
  bool_t foundBase = FALSE;
  for (lpcString_t tok = str; tok && *tok; ) {
    lpcString_t end = strchr(tok, ':');
    size_t len = end ? (size_t)(end - tok) : strlen(tok);
    if (_PseudoFlagFromToken(tok, len) == 0) {
      baseStart = tok;
      baseLen = len;
      foundBase = TRUE;
      break;
    }
    tok = end ? end + 1 : NULL;
  }

  if (!foundBase) {
    // Degenerate case like "hover" only.
    baseStart = str;
    baseLen = strlen(str);
  }

  lpcString_t slash = memchr(baseStart, '/', baseLen);
  size_t classLen = slash ? (size_t)(slash - baseStart) : baseLen;
  classLen = MIN(classLen, sizeof(shortStr_t) - 1);
  memcpy(cls->value, baseStart, classLen);
  cls->value[classLen] = '\0';

  cls->opacity = slash ? (byte_t)atoi(slash + 1) : 100;
  return cls;
}

// Append a parsed style_class_selector to the object's class list
void
_AddClass(struct Object *obj, struct style_class_selector* cls)
{
  struct StyleController* sc = GetStyleController(obj);
  if (!sc) { free(cls); return; }
  ADD_TO_LIST_END(struct style_class_selector, cls, sc->classes);
  if (cls->flags & STYLE_HOVER) {
    OBJ_SetFlags(obj, OBJ_GetFlags(obj) | OF_HOVERABLE);
  }
}

// Release a StyleRule native object (and its string Selector field).
static void
_ReleaseNativeStyleRule(struct Object *ruleObj)
{
  if (!ruleObj) return;
  struct StyleRule* sr = GetStyleRule(ruleObj);
  if (sr) {
    // Free Selector string if it was set directly (not via property system)
    // In practice Selector is set via PROP_Create; OBJ_ReleaseProperties frees it.
  }
  OBJ_ReleaseProperties(ruleObj);
  OBJ_ReleaseComponents(ruleObj);
  free((void*)OBJ_GetName(ruleObj));
  free(ruleObj);
}

// Release a StyleSheet native object and all its StyleRule children.
static void
_ReleaseNativeStyleSheet(struct Object *sheetObj)
{
  if (!sheetObj) return;
  struct Object *child = OBJ_GetFirstChild(sheetObj);
  while (child) {
    struct Object *next = OBJ_GetNext(child);
    _ReleaseNativeStyleRule(child);
    child = next;
  }
  OBJ_ReleaseProperties(sheetObj);
  OBJ_ReleaseComponents(sheetObj);
  free((void*)OBJ_GetName(sheetObj));
  free(sheetObj);
}

// ============================================================================
// STYLE RESOLUTION
// ============================================================================

// Return the current pseudo-state flags for an object (hover, focus, dark, selected)
uint32_t
OBJ_GetStyleFlags(struct Object *pobj)
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
// cls: the resolved class selector; carries per-usage opacity (cls->opacity).
static void
_ApplyStyleRule(struct Object *target, struct Object *ruleObj,
                uint32_t ruleFlags, struct style_class_selector* cls)
{
  for (struct Property *rp = OBJ_GetProperties(ruleObj); rp; rp = PROP_GetNext(rp)) {
    // Skip StyleRule's own component properties (ClassName, PseudoClass, etc.)
    if (PROP_GetFlags(rp) & PF_PROPERTY_TYPE) continue;
    if (PROP_IsNull(rp)) continue;

    struct PropertyType const *pdesc = PROP_GetDesc(rp);
    if (!pdesc || !pdesc->Name) continue;

    struct Property *hprop = NULL;
    if (FAILED(OBJ_FindShortProperty(target, pdesc->Name, &hprop))) continue;

    if (ruleFlags) {
      PROP_SetFlag(hprop, PF_SPECIALIZED);
    } else if (PROP_GetFlags(hprop) & PF_SPECIALIZED) {
      // A state-specific rule (e.g., :hover) was already applied — skip the default
      continue;
    }

    PROP_SetValue(hprop, PROP_GetValue(rp));

    // Apply per-usage opacity (from "class/N" on the node) to color properties.
    // This is node-side, not rule-side: the same rule can be used at different opacities.
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
_EnumSheetRules(struct Object *sheetObj, struct Object *target,
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
OBJ_EnumStyleClasses(struct Object *pobj,
                     lpcString_t classname,
                     struct style_class_selector* cls)
{
  uint32_t dwClassID = fnv1a32(classname);
  uint32_t objFlags  = OBJ_GetStyleFlags(pobj);

  // Global stylesheet first
  _EnumSheetRules(static_stylesheet, pobj, dwClassID, objFlags, cls);

  // Walk up the object hierarchy
  for (struct Object *p = pobj; p; p = OBJ_GetParent(p)) {
    struct StyleController* sc = GetStyleController(p);
    if (!sc || !sc->StyleSheet) continue;
    _EnumSheetRules(CMP_GetObject(sc->StyleSheet), pobj, dwClassID, objFlags, cls);
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
  FOR_EACH_LIST(struct style_class_selector, cls, pStyleController->classes) free(cls);
  pStyleController->classes = NULL;
  
  // Only release the sheet if we created it (owned_sheet).
  // When StyleSheet was assigned externally (e.g. shared between controllers),
  // we just clear our pointer without freeing the underlying object.
  if (pStyleController->StyleSheet && pStyleController->owned_sheet) {
    _ReleaseNativeStyleSheet(CMP_GetObject(pStyleController->StyleSheet));
    pStyleController->owned_sheet = FALSE;
  }
  pStyleController->StyleSheet = NULL;
  return FALSE;
}

#define ID_body 0xdbaa7975 // "body" class

// Recalculate and apply all active style rules to this object.
HANDLER(StyleController, StyleController, ThemeChanged) {
  bool_t recursive = pThemeChanged ? pThemeChanged->recursive : FALSE;

  PROP_ClearSpecialized(OBJ_GetProperties(hObject));

  // Root objects: apply "body" rules from the local stylesheet
  // A workaround, ideally 'body' style should just propagate to all children
  if (!OBJ_GetParent(hObject) && pStyleController->StyleSheet) {
    struct Object *sheetObj = CMP_GetObject(pStyleController->StyleSheet);
    FOR_EACH_OBJECT(child, sheetObj) {
      struct StyleRule* sr = GetStyleRule(child);
      if (sr && sr->class_id == ID_body) {
        _ApplyStyleRule(hObject, child, 0, &(struct style_class_selector){ .opacity = 100 });
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
  _SendMessage(hObject, StyleController, ThemeChanged, .recursive = FALSE);
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
  _SendMessage(hObject, StyleController, ThemeChanged, .recursive = FALSE);
  return TRUE;
}
