#include <include/api.h>
#include <include/orca.h>

#include <source/core/core_local.h>

// ============================================================================
// STYLE SYSTEM DATA STRUCTURES
// ============================================================================
//
// The styling system implements a CSS-like approach with classes and stylesheets:
//
// style_class: Represents a single parsed class name (e.g., "button", "hover")
//   - value: the base class name string (e.g., "button", "blue")
//   - flags: pseudo-state bits (STYLE_HOVER, STYLE_FOCUS, STYLE_DARK, STYLE_SELECT)
//   - opacity: percentage from /N syntax (e.g., "blue/50" = 50% opacity)
//   - Forms a linked list per object (StyleController.classes)
//
// style_sheet: Represents a single style rule (selector + property + value)
//   - classname: CSS class selector (e.g., ".button")
//   - name: property name (e.g., "background-color")
//   - value: property value string (e.g., "#ff0000")
//   - class_id: FNV1a hash of classname for fast matching
//   - prop_id: FNV1a hash of property name
//   - flags: state mask — rule applies only when (object flags & rule flags) == rule flags
//   - Forms a linked list (StyleController.stylesheet per-object, or global static_sheet)

struct style_class
{
  struct style_class* next;
  shortStr_t value;    // base class name (without pseudo-states or opacity suffix)
  byte_t flags;        // STYLE_HOVER / STYLE_FOCUS / STYLE_DARK / STYLE_SELECT bits
  byte_t opacity;      // 0–100 percentage (default 100); sourced from "/N" syntax
};

struct style_sheet
{
  struct style_sheet* next;
  uint32_t class_id;   // FNV1a hash of classname (for O(1) matching)
  uint32_t prop_id;    // FNV1a hash of name
  uint32_t flags;      // state mask: rule is active when (obj state & flags) == flags
  shortStr_t classname; // selector (e.g., ".button")
  shortStr_t name;      // property name (e.g., "background-color")
  shortStr_t value;     // property value (e.g., "#ff0000")
};

// Global stylesheet rules (shared across all objects; applied before per-object rules)
static struct style_sheet* static_sheet = NULL;

// Helper struct for @apply callback — carries selector info into _EnumApplyStyleClass
struct _ADDCLASSSTRUCT
{
  lpcString_t name;
  uint32_t id;
  uint32_t flags;
  lpObject_t obj;
};

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

// Parse pseudo-state flags from a colon-separated class string
// e.g., "button:hover:focus" → STYLE_HOVER | STYLE_FOCUS
static uint32_t
_ParsePseudoStateFlags(lpcString_t str)
{
  uint32_t flags = 0;
  for (lpcString_t s = strchr(str, ':'); s; s = strchr((str = s + 1), ':')) {
    if      (!strncmp(str, "hover",  s - str)) flags |= STYLE_HOVER;
    else if (!strncmp(str, "focus",  s - str)) flags |= STYLE_FOCUS;
    else if (!strncmp(str, "active", s - str)) flags |= STYLE_SELECT;
    else if (!strncmp(str, "dark",   s - str)) flags |= STYLE_DARK;
  }
  return flags;
}

// Add a rule to an object's local stylesheet (or to the global sheet if obj is NULL)
static void
_AddRuleToStylesheet(lpObject_t obj, struct style_sheet* ss)
{
  if (obj) {
    struct StyleController* sc = GetStyleController(obj);
    if (sc) ADD_TO_LIST(ss, sc->stylesheet);
    else free(ss); // object has no StyleController — discard the rule
  } else {
    ADD_TO_LIST(ss, static_sheet);
  }
}

// ============================================================================
// PUBLIC STYLE API
// ============================================================================

// Add a single stylesheet rule directly to an object (used by C callers)
void
OBJ_AddStyleClass(lpObject_t obj,
                  lpcString_t name,
                  lpcString_t property,
                  lpcString_t value,
                  uint32_t flags)
{
  struct StyleController* sc = GetStyleController(obj);
  if (!sc) return;
  struct style_sheet* ss = ZeroAlloc(sizeof(struct style_sheet));
  strncpy(ss->classname, name, sizeof(ss->classname));
  strncpy(ss->value, value, sizeof(ss->value));
  strncpy(ss->name, property, sizeof(ss->name));
  ss->class_id = fnv1a32(name);
  ss->prop_id = fnv1a32(property);
  ss->flags = flags;
  ss->next = NULL;
  ADD_TO_LIST_END(struct style_sheet, ss, sc->stylesheet);
}

// Callback used by OBJ_AddStyleSheet's @apply handler:
// copies a matched global rule into the current object's stylesheet with a new selector
static void
_EnumApplyStyleClass(lpObject_t obj, struct style_sheet* other, void* param)
{
  struct _ADDCLASSSTRUCT* inp = param;
  struct style_sheet* ss = ZeroAlloc(sizeof(struct style_sheet));
  memcpy(ss, other, sizeof(struct style_sheet));
  strncpy(ss->classname, inp->name, sizeof(shortStr_t));
  ss->class_id = inp->id;
  ss->flags = inp->flags;
  ss->next = NULL;
  _AddRuleToStylesheet(obj, ss);
}

// Parse a single style class token (e.g., "blue/50:hover")
// Returns a newly allocated style_class with parsed name, opacity, and pseudo-state flags
struct style_class*
_ParseClass(lpcString_t str)
{
  struct style_class* cls = ZeroAlloc(sizeof(struct style_class));

  // Extract pseudo-state flags from colon-separated suffixes (e.g., ":hover:focus")
  cls->flags = _ParsePseudoStateFlags(str);

  // Extract opacity from /N syntax (e.g., "blue/50" → opacity=50)
  lpcString_t szOpacity = strchr(str, '/');
  if (szOpacity) {
    strncpy(cls->value, str, MIN(sizeof(shortStr_t), szOpacity - str));
    cls->opacity = atoi(szOpacity + 1);
  } else {
    strncpy(cls->value, str, sizeof(shortStr_t));
    cls->opacity = 100; // default: fully opaque
  }

  return cls;
}

// Append a parsed style_class to the object's class list and update hover flag if needed
void
_AddClass(lpObject_t obj, struct style_class* cls)
{
  struct StyleController* sc = GetStyleController(obj);
  if (!sc) { free(cls); return; }
  ADD_TO_LIST_END(struct style_class, cls, sc->classes);
  if (cls->flags & STYLE_HOVER) {
    OBJ_SetFlags(obj, OBJ_GetFlags(obj) | OF_HOVERABLE);
  }
}

// Parse space-separated class attribute string (e.g., "button:hover blue/50 dark")
// and attach each parsed class to the object's StyleController
void
OBJ_ParseClassAttribute(lpObject_t obj, lpcString_t original)
{
  if (!*original)
    return;
  WITH(char, classes, strdup(original), free)
  {
    for (lpcString_t s = strtok(classes, " "); s; s = strtok(NULL, " ")) {
      _AddClass(obj, _ParseClass(s));
    }
  }
}

// Add a single class token to an object at runtime
void
OBJ_AddClass(lpObject_t hobj, lpcString_t cls)
{
  _AddClass(hobj, _ParseClass(cls));
}

// ============================================================================
// STYLESHEET LOADING (Lua API)
// ============================================================================

// Lua state is stored while processing a stylesheet table so that
// parse_property (called from _ApplyRule) can resolve Lua values
static lua_State* g_L = NULL;

// Parse a Lua table of CSS rules and register them on an object (or globally)
// name: selector string (e.g., ".button:hover"); table at stack index 3
void OBJ_AddStyleSheet(lua_State* L, lpObject_t self, lpcString_t name)
{
  uint32_t classID = *name == '.' ? fnv1a32(name + 1) : 0;
  if (lua_type(L, 3) != LUA_TTABLE) {
    lua_error(L);
    return;
  }
  g_L = L;
  lua_pushvalue(L, 3);
  lua_pushnil(L);
  while (lua_next(L, -2)) {
    lpcString_t prop = luaL_checkstring(L, -2);
    uint32_t propID = fnv1a32(prop);

    // @apply: inline another class's rules into this selector
    if (propID == CSS_APPLY) {
      LPSTR str = strdup(luaL_checkstring(L, -1));
      for (lpcString_t s = strtok(str, " "); s; s = strtok(NULL, " ")) {
        struct style_class* cls = _ParseClass(s);
        struct _ADDCLASSSTRUCT add_class = {
          .name = name,
          .id = classID,
          .flags = cls->flags,
          .obj = self,
        };
        OBJ_EnumStyleClasses(self, cls->value, _EnumApplyStyleClass, &add_class);
        free(cls);
      }
      free(str);
      lua_pop(L, 1);
      continue;
    }

    struct style_sheet* ss = ZeroAlloc(sizeof(struct style_sheet));
    ss->class_id = classID;
    ss->prop_id = propID;
    strncpy(ss->classname, name, sizeof(shortStr_t));
    strncpy(ss->name, prop, sizeof(shortStr_t));

    // Parse pseudo-state flags from selector (e.g., ".button:hover" → STYLE_HOVER)
    strtok(ss->classname, ":");
    for (lpcString_t s = strtok(NULL, ":"); s; s = strtok(NULL, ":")) {
      if      (!strcmp(s, "hover"))  ss->flags |= STYLE_HOVER;
      else if (!strcmp(s, "focus"))  ss->flags |= STYLE_FOCUS;
      else if (!strcmp(s, "active")) ss->flags |= STYLE_SELECT;
      else if (!strcmp(s, "dark"))   ss->flags |= STYLE_DARK;
    }

    // Read property value from the Lua stack (string, number, or boolean)
    switch (lua_type(L, -1)) {
      case LUA_TSTRING:
        strncpy(ss->value, luaL_checkstring(L, -1), sizeof(shortStr_t));
        break;
      case LUA_TNUMBER:
        sprintf(ss->value, "%f", luaL_checknumber(L, -1));
        break;
      case LUA_TBOOLEAN:
        strcpy(ss->value, lua_toboolean(L, -1) ? "true" : "false");
        break;
      default:
        lua_error(L);
        return;
    }

    ss->class_id = fnv1a32(ss->classname + 1);
    _AddRuleToStylesheet(self, ss);
    lua_pop(L, 1);
  }
  lua_pop(L, 1);
}

// ============================================================================
// STYLE RESOLUTION
// ============================================================================

// Walk global and per-object stylesheets looking for rules that match classname,
// and invoke Proc for each matching rule whose state flags satisfy the object's current state
void
OBJ_EnumStyleClasses(lpObject_t pobj,
                     lpcString_t classname,
                     STYLEPROC Proc,
                     void* param)
{
  uint32_t dwClassID = fnv1a32(classname);
  uint32_t objFlags = OBJ_GetStyleFlags(pobj);

  // Check global stylesheet first
  FOR_EACH_LIST(struct style_sheet, ss, static_sheet) {
    if (ss->class_id != dwClassID) continue;
    if (ss->flags & STYLE_HOVER) OBJ_SetFlags(pobj, OF_HOVERABLE);
    if ((objFlags & ss->flags) == ss->flags) {
      Proc(pobj, ss, param);
    }
  }

  // Walk up the object hierarchy, checking each ancestor's local stylesheet
  for (lpObject_t p = pobj; p; p = OBJ_GetParent(p)) {
    struct StyleController* sc = GetStyleController(p);
    if (!sc) continue;
    FOR_EACH_LIST(struct style_sheet, ss, sc->stylesheet) {
      if (ss->class_id != dwClassID) continue;
      if (ss->flags & STYLE_HOVER) OBJ_SetFlags(pobj, OF_HOVERABLE);
      if ((objFlags & ss->flags) == ss->flags) {
        Proc(pobj, ss, param);
      }
    }
  }
}

ORCA_API int
parse_property(lua_State* L, const char* str, struct PropertyType const* prop, void* valueptr);

// Apply a single stylesheet rule to a property on the target object
static void
_ApplyRule(lpObject_t pobj, struct style_sheet* ss, void* parm)
{
  lpProperty_t hProperty;
  if (SUCCEEDED(OBJ_FindShortProperty(pobj, ss->name, &hProperty))) {
    if (ss->flags) {
      PROP_SetFlag(hProperty, PF_SPECIALIZED);
    } else if (PROP_GetFlags(hProperty) & PF_SPECIALIZED) {
      // A state-specific rule (e.g., :hover) was already applied — skip the default
      return;
    }

    char buf[MAX_PROPERTY_STRING] = {0};
    parse_property(g_L, ss->value, PROP_GetDesc(hProperty), buf);
    PROP_SetValue(hProperty, buf);

    // Apply opacity to color properties when the class carries a /N opacity modifier
    if (PROP_GetType(hProperty) == kDataTypeColor && parm) {
      struct color clr;
      PROP_CopyValue(hProperty, &clr);
      clr.a = ((struct style_class*)parm)->opacity / 100.f;
      PROP_SetValue(hProperty, &clr);
    }
  } else {
    Con_Error("Can't find property %s", ss->name);
  }
}

// Return the current pseudo-state flags for an object (hover, focus, dark, selected)
uint32_t
OBJ_GetStyleFlags(lpObject_t pobj)
{
  uint32_t dwValue = 0;
  if (core_GetHover() == pobj) dwValue |= STYLE_HOVER;
  if (core_GetFocus() == pobj) dwValue |= STYLE_FOCUS;
  if (OBJ_GetFlags(pobj) & OF_SELECTED) dwValue |= STYLE_SELECT;
  if (WI_IsDarkTheme()) dwValue |= STYLE_DARK;
  return dwValue;
}

// Free all style classes and stylesheet rules owned by this object's StyleController.
// Safe to call on objects that do not have a StyleController.
void
OBJ_ClearStyleClasses(lpObject_t pobj)
{
  struct StyleController* sc = GetStyleController(pobj);
  if (!sc) return;
  FOR_EACH_LIST(struct style_class, cls, sc->classes) free(cls);
  FOR_EACH_LIST(struct style_sheet, ss, sc->stylesheet) free(ss);
  sc->classes = NULL;
  sc->stylesheet = NULL;
}

// ============================================================================
// STYLECONTROLLER COMPONENT HANDLERS
// ============================================================================

// Initialize both style lists to NULL (ZeroAlloc already does this, but explicit for clarity)
HANDLER(StyleController, Object, Create) {
  pStyleController->classes = NULL;
  pStyleController->stylesheet = NULL;
  return FALSE;
}

// Free all style classes and stylesheet rules owned by this object's StyleController
HANDLER(StyleController, Object, Release) {
  OBJ_ClearStyleClasses(hObject);
  return FALSE;
}

// Recalculate and apply all active style rules to this object.
// wParam: bool_t recursive — if TRUE, also apply to child objects
HANDLER(StyleController, StyleController, ApplyStyles) {
  bool_t recursive = (bool_t)wParam;

  PROP_ClearSpecialized(OBJ_GetProperties(hObject));

  // Root objects: apply "body" rules from the local stylesheet
  if (!OBJ_GetParent(hObject)) {
    FOR_EACH_LIST(struct style_sheet, ss, pStyleController->stylesheet) {
      if (*(uint32_t const*)ss->classname == *(uint32_t const*)"body") {
        _ApplyRule(hObject, ss, &(struct style_class){ .opacity = 100 });
      }
    }
  }

  // Apply each class whose state flags match the current object state
  FOR_EACH_LIST(struct style_class, cls, pStyleController->classes) {
    if ((OBJ_GetStyleFlags(hObject) & cls->flags) == cls->flags) {
      OBJ_EnumStyleClasses(hObject, cls->value, _ApplyRule, cls);
    }
  }

  if (recursive) {
    FOR_EACH_CHILD(hObject, OBJ_ApplyStyles, TRUE);
  }

  return FALSE;
}

// Thin public wrapper: send StyleController.ApplyStyles message to the object.
// Objects without a StyleController silently ignore this (no handler is called).
void
OBJ_ApplyStyles(lpObject_t object, bool_t recursive)
{
  OBJ_SendMessageW(object, ID_StyleController_ApplyStyles, recursive, NULL);
}

