#include <include/api.h>
#include <include/orca.h>

#include <ctype.h>

#include <source/core/core_local.h>
#include <core/core_properties.h>

// ============================================================================
// STYLE SYSTEM
// ============================================================================
//
// StyleSheet: standalone container object holding StyleRule children.
//   - Stored in StyleController.StyleSheet (per-object) or static_stylesheet (global).
//
// StyleRule: child object of a StyleSheet.
//   - ClassName:  selector without pseudo-state (e.g., "button", ".button",
//                 "#Logo", "ImageView", "StackView > Label", or ".popup .panel").
//   - PseudoClass: colon-separated pseudo-states (e.g., "hover:focus") —
//                 empty means the rule applies to all states.
//   - class_id / flags: cached from ClassName / PseudoClass
//     (class_id is retained for compatibility/debugging; matching uses ClassName)
//   - selector: compiled right-to-left selector parts cached from ClassName
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
// CSS UNIT → BINDING CONVERSION
// ============================================================================

// CSS units that map to inherited properties or viewport dimensions.
static bool_t
_css_value_has_unit(lpcString_t value)
{
  if (!value) return FALSE;
  size_t len = strlen(value);
  if (len < 3) return FALSE;
  // Skip trailing whitespace
  while (len > 0 && isspace((unsigned char)value[len - 1])) len--;
  if (len < 3) return FALSE;
  // Check for recognized CSS unit suffix
  static const char* const units[] = { "rem", "em", "vh", "vw", "vmin", "vmax", "px", "pt", NULL };
  for (int i = 0; units[i]; i++) {
    size_t ulen = strlen(units[i]);
    if (len > ulen && strncmp(value + len - ulen, units[i], ulen) == 0) {
      // Verify the character before the unit is a digit
      size_t num_end = len - ulen;
      if (num_end > 0 && (isdigit((unsigned char)value[num_end - 1]) || value[num_end - 1] == '.')) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

// Extract the numeric portion from a CSS value with unit (e.g., "1.5rem" → "1.5").
static void
_css_strip_unit(lpcString_t value, char *out, size_t out_size)
{
  if (!value || !out || out_size == 0) return;
  size_t len = strlen(value);
  size_t i = 0;
  // Skip leading whitespace
  while (i < len && isspace((unsigned char)value[i])) i++;
  // Copy digits and decimal point
  size_t j = 0;
  while (i < len && j < out_size - 1) {
    if (isdigit((unsigned char)value[i]) || value[i] == '.') {
      out[j++] = value[i];
    } else {
      break;
    }
    i++;
  }
  out[j] = '\0';
}

// Map CSS property name to binding expression base path.
static bool_t
_css_get_binding_path(lpcString_t orca_prop_name, char *out, size_t out_size)
{
  if (!orca_prop_name || !out || out_size == 0) return FALSE;
  // Font.Size → ../Font.Size (parent's font size for rem/em)
  if (!strcasecmp(orca_prop_name, "TextRun.FontSize") ||
      !strcasecmp(orca_prop_name, "Font.Size")) {
    snprintf(out, out_size, "../Font.Size");
    return TRUE;
  }
  // Node.Width → ../Width (parent's width)
  if (!strcasecmp(orca_prop_name, "Node.Width") ||
      !strcasecmp(orca_prop_name, "Width")) {
    snprintf(out, out_size, "../Width");
    return TRUE;
  }
  // Node.Height → ../Height (parent's height)
  if (!strcasecmp(orca_prop_name, "Node.Height") ||
      !strcasecmp(orca_prop_name, "Height")) {
    snprintf(out, out_size, "../Height");
    return TRUE;
  }
  return FALSE;
}

// Create a binding expression from a CSS value with unit.
// E.g., "1.5rem" + "TextRun.FontSize" → "{../Font.Size} * 1.5"
static bool_t
_css_make_binding_expr(lpcString_t css_value, lpcString_t orca_prop_name,
                        char *out, size_t out_size)
{
  if (!css_value || !orca_prop_name || !out || out_size == 0) return FALSE;

  char base_path[64] = {0};
  if (!_css_get_binding_path(orca_prop_name, base_path, sizeof(base_path))) {
    return FALSE;
  }

  char num_str[32] = {0};
  _css_strip_unit(css_value, num_str, sizeof(num_str));

  if (num_str[0] == '\0') return FALSE;

  // Check if multiplier is 1.0 (no multiplication needed in binding)
  double multiplier = atof(num_str);
  if (multiplier == 1.0) {
    snprintf(out, out_size, "{%s}", base_path);
  } else {
    snprintf(out, out_size, "{%s} * %s", base_path, num_str);
  }
  return TRUE;
}

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

static void
_CopyTrim(char* dst, size_t dst_size, lpcString_t start, lpcString_t end)
{
  while (start < end && isspace((unsigned char)*start)) start++;
  while (end > start && isspace((unsigned char)*(end - 1))) end--;
  size_t len = (size_t)(end - start);
  if (len >= dst_size) len = dst_size - 1;
  memcpy(dst, start, len);
  dst[len] = '\0';
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

static struct style_class_selector*
_FindMatchingClass(struct Object *obj, lpcString_t className, uint32_t objFlags)
{
  struct StyleController* sc = GetStyleController(obj);
  if (!sc) return NULL;
  Con_Printf("DEBUG _FindMatchingClass: obj=%s looking_for=%s\n", OBJ_GetName(obj), className);
  FOR_EACH_LIST(struct style_class_selector, cls, sc->classes) {
    Con_Printf("DEBUG _FindMatchingClass: obj=%s has_class=%s flags=%d objFlags=%d\n",
               OBJ_GetName(obj), cls->value, cls->flags, objFlags);
    if (!strcmp(cls->value, className) &&
        ((objFlags & cls->flags) == cls->flags)) {
      return cls;
    }
  }
  return NULL;
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
  OBJ_SendMessageW(ruleObj, ID_Object_Release, 0, NULL);
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

static bool_t
_MatchSimpleSelector(struct Object *target,
                     lpcString_t selector,
                     uint32_t objFlags,
                     struct style_class_selector** matchedClass)
{
  if (!selector || !selector[0]) return FALSE;

  bool_t hasCompoundMarker = strchr(selector, '.') || strchr(selector, '#');
  if (hasCompoundMarker) {
    lpcString_t p = selector;

    if (*p != '.' && *p != '#') {
      lpcString_t typeStart = p;
      while (*p && *p != '.' && *p != '#') p++;
      char typeName[256] = {0};
      _CopyTrim(typeName, sizeof(typeName), typeStart, p);

      if (!strcmp(typeName, "body")) {
        if (OBJ_GetParent(target) != NULL) return FALSE;
      } else {
        lpcString_t className = OBJ_GetClassName(target);
        if (!className || strcmp(className, typeName)) return FALSE;
      }
    }

    while (*p) {
      char marker = *p++;
      lpcString_t valueStart = p;
      while (*p && *p != '.' && *p != '#') p++;
      if (valueStart == p) return FALSE;

      char value[256] = {0};
      _CopyTrim(value, sizeof(value), valueStart, p);

      if (marker == '.') {
        struct style_class_selector* cls =
          _FindMatchingClass(target, value, objFlags);
        if (!cls) return FALSE;
        if (matchedClass && !*matchedClass) *matchedClass = cls;
      } else if (marker == '#') {
        lpcString_t name = OBJ_GetName(target);
        if (!name || strcmp(name, value)) return FALSE;
      } else {
        return FALSE;
      }
    }

    return TRUE;
  }

  if (selector[0] == '.') {
    struct style_class_selector* cls =
      _FindMatchingClass(target, selector + 1, objFlags);
    if (!cls) return FALSE;
    if (matchedClass) *matchedClass = cls;
    return TRUE;
  }

  if (selector[0] == '#') {
    lpcString_t name = OBJ_GetName(target);
    return name && !strcmp(name, selector + 1);
  }

  if (!strcmp(selector, "body")) {
    return OBJ_GetParent(target) == NULL;
  }

  lpcString_t className = OBJ_GetClassName(target);
  if (className && !strcmp(className, selector)) {
    return TRUE;
  }

  // Compatibility with existing programmatic StyleRule users: a bare selector
  // still matches a style class. CSS authors should prefer ".class" for class
  // selectors and "TypeName" for type selectors.
  struct style_class_selector* cls =
    _FindMatchingClass(target, selector, objFlags);
  if (!cls) return FALSE;
  if (matchedClass) *matchedClass = cls;
  return TRUE;
}

static int
_ParseSelectorParts(lpcString_t selector,
                    struct style_selector_part* parts,
                    int maxParts)
{
  int count = 0;
  style_combinator_t relation = STYLE_COMBINATOR_NONE;
  lpcString_t p = selector;

  while (p && *p) {
    bool_t sawSpace = FALSE;
    while (*p && isspace((unsigned char)*p)) {
      sawSpace = TRUE;
      p++;
    }
    if (!*p) break;

    if (*p == '>') {
      relation = STYLE_COMBINATOR_CHILD;
      p++;
      continue;
    }

    if (count > 0 && relation == STYLE_COMBINATOR_NONE && sawSpace) {
      relation = STYLE_COMBINATOR_DESCENDANT;
    }

    lpcString_t start = p;
    while (*p && *p != '>' && !isspace((unsigned char)*p)) p++;

    if (start == p) continue;
    if (count >= maxParts) return 0;

    _CopyTrim(parts[count].selector, sizeof(parts[count].selector), start, p);
    parts[count].combinator =
      count == 0 ? STYLE_COMBINATOR_NONE :
                   (relation == STYLE_COMBINATOR_NONE
                      ? STYLE_COMBINATOR_DESCENDANT
                      : relation);
    count++;
    relation = STYLE_COMBINATOR_NONE;
  }

  return count;
}

struct style_selector *
OBJ_CompileStyleSelector(lpcString_t selector)
{
  if (!selector || !selector[0]) return NULL;

  struct style_selector *compiled = ZeroAlloc(sizeof(*compiled));
  int count = _ParseSelectorParts(selector,
                                  compiled->parts,
                                  ARRAY_SIZE(compiled->parts));
  if (count <= 0) {
    free(compiled);
    return NULL;
  }

  compiled->count = (byte_t)count;
  return compiled;
}

void
OBJ_FreeStyleSelector(struct style_selector *selector)
{
  free(selector);
}

static bool_t
_SelectorMatchesTarget(struct Object *target,
                       struct style_selector const *selector,
                       uint32_t objFlags,
                       struct style_class_selector** matchedClass)
{
  if (!selector || selector->count == 0) return FALSE;

  if (!_MatchSimpleSelector(target,
                            selector->parts[selector->count - 1].selector,
                            objFlags,
                            matchedClass)) {
    return FALSE;
  }

  struct Object *cursor = target;
  for (int i = selector->count - 1; i > 0; i--) {
    if (selector->parts[i].combinator == STYLE_COMBINATOR_CHILD) {
      cursor = OBJ_GetParent(cursor);
      if (!cursor) return FALSE;
      uint32_t flags = OBJ_GetStyleFlags(cursor);
      if (!_MatchSimpleSelector(cursor, selector->parts[i - 1].selector, flags, NULL)) {
        return FALSE;
      }
      continue;
    }

    struct Object *ancestor = OBJ_GetParent(cursor);
    while (ancestor) {
      uint32_t flags = OBJ_GetStyleFlags(ancestor);
      if (_MatchSimpleSelector(ancestor, selector->parts[i - 1].selector, flags, NULL)) {
        break;
      }
      ancestor = OBJ_GetParent(ancestor);
    }
    if (!ancestor) return FALSE;
    cursor = ancestor;
  }

  return TRUE;
}

static bool_t
_StyleRuleBaseMatches(struct Object *target,
                      struct StyleRule* sr,
                      uint32_t objFlags,
                      struct style_class_selector** matchedClass)
{
  if (!sr || !sr->ClassName || !sr->ClassName[0]) return FALSE;
  if (matchedClass) *matchedClass = NULL;

  struct style_selector *selector = sr->selector;
  bool_t freeSelector = FALSE;
  if (!selector) {
    selector = OBJ_CompileStyleSelector(sr->ClassName);
    freeSelector = TRUE;
  }

  bool_t matched = _SelectorMatchesTarget(target, selector, objFlags, matchedClass);
  if (freeSelector) OBJ_FreeStyleSelector(selector);
  return matched;
}

// Apply all C property overrides from ruleObj to the target object.
// ruleObj->properties contains the pre-parsed overrides (PF_PROPERTY_TYPE not set).
// ruleFlags: the rule's state flags (0 = default, non-zero = pseudo-state rule).
// cls: the resolved class selector; carries per-usage opacity (cls->opacity).
static void
_ApplyStyleRule(struct Object *target, struct Object *ruleObj,
                uint32_t ruleFlags, struct style_class_selector* cls)
{
  Con_Printf("DEBUG _ApplyStyleRule ENTER: target=%s ruleObj=%s\n", OBJ_GetName(target), OBJ_GetName(ruleObj));
  int prop_count = 0;
  int skip_type = 0, skip_null = 0, skip_pdesc = 0, skip_hprop = 0;
  for (struct Property *rp = OBJ_GetProperties(ruleObj); rp; rp = PROP_GetNext(rp)) {
    prop_count++;
    // Skip StyleRule's own component properties (ClassName, PseudoClass, etc.)
    if (PROP_GetFlags(rp) & PF_PROPERTY_TYPE) { skip_type++; continue; }

    struct PropertyType const *pdesc = PROP_GetDesc(rp);
    if (!pdesc || !pdesc->Name) { skip_pdesc++; continue; }

    // Skip null properties UNLESS they have a binding (from CSS units like rem/em)
    if (PROP_IsNull(rp) && !PROP_HasProgram(rp)) { skip_null++; continue; }

    Con_Printf("DEBUG _ApplyStyleRule: prop=%s PF_PROPERTY_TYPE=%d IsNull=%d hasProgram=%d\n",
               pdesc->Name, (PROP_GetFlags(rp) & PF_PROPERTY_TYPE) != 0,
               PROP_IsNull(rp), PROP_HasProgram(rp));

    struct Property *hprop = OBJ_FindLongProperty(target, pdesc->FullIdentifier);
    if (!hprop) { skip_hprop++; continue; }

    if (ruleFlags) {
      PROP_SetFlag(hprop, PF_SPECIALIZED);
    } else if (PROP_GetFlags(hprop) & PF_SPECIALIZED) {
      // A state-specific rule (e.g., :hover) was already applied — skip the default
      continue;
    }

    // Check if the rule property has a binding (e.g., from CSS unit like "1.5rem")
    // If so, transfer the binding to the target property instead of copying the value.
    // The binding expression was created during CSS parsing and resolves relative
    // to the styled object's parent chain.
    if (PROP_HasProgram(rp)) {
      lpcString_t expr = PROP_GetBindingExpression(rp);
      if (expr && expr[0]) {
        Con_Printf("DEBUG _ApplyStyleRule: TRANSFERRING binding '%s' to %s/%s\n",
                   expr, OBJ_GetName(target), pdesc->Name);
        PROP_SetBinding(hprop, expr, kBindingModeOneWay, TRUE);
        PROP_Update(hprop);  // Force immediate evaluation of the binding
        continue;
      }
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
  Con_Printf("DEBUG _ApplyStyleRule EXIT: target=%s prop_count=%d skip_type=%d skip_null=%d skip_pdesc=%d skip_hprop=%d\n",
             OBJ_GetName(target), prop_count, skip_type, skip_null, skip_pdesc, skip_hprop);
}

// Walk one stylesheet's rules for a matching selector and invoke _ApplyStyleRule.
static void
_EnumSheetRules(struct Object *sheetObj, struct Object *target,
                uint32_t objFlags)
{
  if (!sheetObj) return;
  FOR_EACH_OBJECT(child, sheetObj) {
    struct StyleRule* sr = GetStyleRule(child);
    struct style_class_selector* cls = NULL;
    bool_t matches = _StyleRuleBaseMatches(target, sr, objFlags, &cls);
    Con_Printf("DEBUG _EnumSheetRules: target=%s rule_class=%s matches=%d\n",
               OBJ_GetName(target), sr->ClassName ? sr->ClassName : "NULL", matches);
    if (!_StyleRuleBaseMatches(target, sr, objFlags, &cls)) continue;
    if (sr->flags & STYLE_HOVER) {
      OBJ_SetFlags(target, OBJ_GetFlags(target) | OF_HOVERABLE);
    }
    if ((objFlags & sr->flags) != sr->flags) continue;
    Con_Printf("DEBUG _EnumSheetRules: calling _ApplyStyleRule for %s\n", OBJ_GetName(target));
    _ApplyStyleRule(target, child, sr->flags, cls);
  }
}

// Walk global and per-object stylesheets for rules that match this object,
// and invoke _ApplyStyleRule for each matching rule.
void
OBJ_EnumStyleClasses(struct Object *pobj,
                     lpcString_t classname,
                     struct style_class_selector* cls)
{
  (void)classname;
  (void)cls;
  uint32_t objFlags  = OBJ_GetStyleFlags(pobj);
  Con_Printf("DEBUG OBJ_EnumStyleClasses: pobj=%s\n", OBJ_GetName(pobj));

  // Global stylesheet first
  _EnumSheetRules(static_stylesheet, pobj, objFlags);

  // Walk up the object hierarchy
  for (struct Object *p = pobj; p; p = OBJ_GetParent(p)) {
    struct StyleController* sc = GetStyleController(p);
    if (!sc || !sc->StyleSheet) continue;
    Con_Printf("DEBUG OBJ_EnumStyleClasses: found StyleSheet on %s\n", OBJ_GetName(p));
    _EnumSheetRules(CMP_GetObject(sc->StyleSheet), pobj, objFlags);
  }
}


// ============================================================================
// STYLECONTROLLER COMPONENT HANDLERS
// ============================================================================

// StyleController_Create
HANDLER(StyleController, Object, Create) {
  pStyleController->classes = NULL;
  return FALSE;
}

// StyleController_Release
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

// Recalculate and apply all active style rules to this object.
// StyleController_ThemeChanged
HANDLER(StyleController, StyleController, ThemeChanged) {
  bool_t recursive = pThemeChanged ? pThemeChanged->recursive : FALSE;

  PROP_ClearSpecialized(OBJ_GetProperties(hObject));

  OBJ_EnumStyleClasses(hObject, NULL, NULL);

  if (recursive) {
    FOR_EACH_OBJECT(child, hObject) {
      _SendMessage(child, StyleController, ThemeChanged, .recursive = TRUE);
    }
  }

  return TRUE;
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

/* Reconstruct the space-separated class string from the parsed selector list.
 * Writes into buf (up to len bytes including the null terminator).
 * Returns buf, or NULL if the object has no StyleController or no classes. */
ORCA_API lpcString_t
OBJ_GetRawStyleClasses(struct Object *object, LPSTR buf, uint32_t len)
{
  struct StyleController *sc = GetStyleController(object);
  if (!sc || !sc->classes || !buf || !len) return NULL;
  buf[0] = 0;
  size_t pos = 0;
  FOR_EACH_LIST(struct style_class_selector, cls, sc->classes) {
    size_t n = strlen(cls->value);
    if (!n) continue;
    if (pos > 0 && pos < len - 1) buf[pos++] = ' ';
    if (pos + n >= len) break;
    memcpy(buf + pos, cls->value, n);
    pos += n;
    buf[pos] = 0;
  }
  return pos > 0 ? buf : NULL;
}

