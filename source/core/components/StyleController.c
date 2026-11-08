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
  else if (len == 13 && !strncmp(token, "focus-visible", 13)) return STYLE_FOCUS_VISIBLE;
  else if (len == 6 && !strncmp(token, "active", 6)) return STYLE_SELECT;
  // OF_SELECTED marks both a pressed control and a selected tab or list row.
  else if (len == 8 && !strncmp(token, "selected", 8)) return STYLE_SELECT;
  else if (len == 4 && !strncmp(token, "dark",   4)) return STYLE_DARK;
  else if (len == 7 && !strncmp(token, "checked", 7)) return STYLE_CHECKED;
  else if (len == 13 && !strncmp(token, "indeterminate", 13)) return STYLE_INDETERMINATE;
  else if (len == 8 && !strncmp(token, "disabled", 8)) return STYLE_DISABLED;
  else if (len == 7 && !strncmp(token, "enabled", 7)) return STYLE_ENABLED;
  return 0;
}

uint32_t
OBJ_ParseStylePseudo(lpcString_t str)
{
  uint32_t flags = 0;
  for (lpcString_t tok = str; tok && *tok; ) {
    lpcString_t end = strchr(tok, ':');
    size_t len = end ? (size_t)(end - tok) : strlen(tok);
    uint32_t flag = _PseudoFlagFromToken(tok, len);
    if (!flag || (end && !end[1])) return STYLE_INVALID;
    flags |= flag;
    tok = end ? end + 1 : NULL;
  }
  return flags;
}

static void
_SplitSelectorPseudo(lpcString_t selector,
                    char *baseOut,
                    size_t baseOutSize,
                    uint32_t *pseudoFlags)
{
  if (baseOut && baseOutSize) baseOut[0] = '\0';
  if (pseudoFlags) *pseudoFlags = 0;
  if (!selector) return;

  lpcString_t colon = strchr(selector, ':');
  if (!colon) {
    if (baseOut && baseOutSize) {
      size_t len = strlen(selector);
      if (len >= baseOutSize) len = baseOutSize - 1;
      memcpy(baseOut, selector, len);
      baseOut[len] = '\0';
    }
    return;
  }

  if (baseOut && baseOutSize) {
    lpcString_t start = selector;
    lpcString_t end = colon;
    while (start < end && isspace((unsigned char)*start)) start++;
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    size_t len = (size_t)(end - start);
    if (len >= baseOutSize) len = baseOutSize - 1;
    memcpy(baseOut, start, len);
    baseOut[len] = '\0';
  }
  if (pseudoFlags) {
    *pseudoFlags = OBJ_ParseStylePseudo(colon + 1);
  }
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

  // Support both suffix pseudo syntax (bg-blue:hover) and Tailwind-like
  // prefix syntax (hover:bg-blue). We pick the non-pseudo segment as class.
  lpcString_t baseStart = str;
  size_t baseLen = strlen(str);
  bool_t foundBase = FALSE;
  for (lpcString_t tok = str; tok && *tok; ) {
    lpcString_t end = strchr(tok, ':');
    size_t len = end ? (size_t)(end - tok) : strlen(tok);
    uint32_t flag = _PseudoFlagFromToken(tok, len);
    if (flag) cls->flags |= flag;
    else if (foundBase) cls->flags |= STYLE_INVALID;
    else {
      baseStart = tok;
      baseLen = len;
      foundBase = TRUE;
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
  cls->hash = fnv1a32(cls->value);

  cls->opacity = slash ? (byte_t)atoi(slash + 1) : 100;
  return cls;
}

static struct style_class_selector*
_FindMatchingClass(struct Object *obj, uint32_t hash, lpcString_t className, uint32_t objFlags)
{
  struct StyleController* sc = GetStyleController(obj);
  if (!sc) return NULL;
  FOR_EACH_LIST(struct style_class_selector, cls, sc->classes) {
    if (cls->hash == hash && (objFlags & cls->flags) == cls->flags && !strcmp(cls->value, className))
      return cls;
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

static bool_t
_IsDarkAppearance(struct Object *object)
{
  for (struct Object *scope = object; scope; scope = OBJ_GetParent(scope)) {
    struct StyleController *style = GetStyleController(scope);
    if (!style || style->Appearance == kAppearanceInherit) continue;
    if (style->Appearance == kAppearanceLight) return FALSE;
    if (style->Appearance == kAppearanceDark) return TRUE;
    break;
  }
  return axIsDarkTheme();
}

static bool_t
_IsDisabled(struct Object *object)
{
  for (struct Object *p = object; p; p = OBJ_GetParent(p))
    if (OBJ_GetFlags(p) & OF_DISABLED) return TRUE;
  return FALSE;
}

// Pseudo-state flags from the object's own state plus the inherited ones.
static uint32_t
_StyleFlags(struct Object *pobj, bool_t disabled, bool_t dark)
{
  uint32_t dwValue = 0;
  if (core_GetHover() == pobj) dwValue |= STYLE_HOVER;
  if (core_GetFocus() == pobj) dwValue |= STYLE_FOCUS;
  if (core_GetFocus() == pobj && core.keyboardNavigation) dwValue |= STYLE_FOCUS_VISIBLE;
  uint32_t flags = OBJ_GetFlags(pobj);
  if (flags & OF_SELECTED) dwValue |= STYLE_SELECT;
  if (flags & OF_CHECKED) dwValue |= STYLE_CHECKED;
  if (flags & OF_INDETERMINATE) dwValue |= STYLE_INDETERMINATE;
  dwValue |= disabled ? STYLE_DISABLED : STYLE_ENABLED;
  if (dark) dwValue |= STYLE_DARK;
  return dwValue;
}

// Return the current pseudo-state flags for an object (hover, focus, dark, selected)
uint32_t
OBJ_GetStyleFlags(struct Object *pobj)
{
  return _StyleFlags(pobj, _IsDisabled(pobj), _IsDarkAppearance(pobj));
}

// The target's ancestor chain with each object's pseudo-state flags, resolved
// once per restyle. Combinator matching walks this instead of recomputing the
// inherited disabled and appearance state for every rule.
#define MAX_STYLE_CHAIN 64
struct style_chain
{
  int count;
  struct Object *objects[MAX_STYLE_CHAIN];
  uint32_t flags[MAX_STYLE_CHAIN];
};

static void
_BuildStyleChain(struct Object *target, struct style_chain *chain)
{
  chain->count = 0;
  for (struct Object *p = target; p && chain->count < MAX_STYLE_CHAIN; p = OBJ_GetParent(p))
    chain->objects[chain->count++] = p;
  struct Object *above = OBJ_GetParent(chain->objects[chain->count - 1]);
  bool_t system = axIsDarkTheme();
  bool_t disabled = above ? _IsDisabled(above) : FALSE;
  bool_t dark = above ? _IsDarkAppearance(above) : system;
  for (int i = chain->count - 1; i >= 0; i--) {
    struct Object *object = chain->objects[i];
    if (OBJ_GetFlags(object) & OF_DISABLED) disabled = TRUE;
    struct StyleController *style = GetStyleController(object);
    if (style && style->Appearance == kAppearanceLight) dark = FALSE;
    else if (style && style->Appearance == kAppearanceDark) dark = TRUE;
    else if (style && style->Appearance == kAppearanceSystem) dark = system;
    chain->flags[i] = _StyleFlags(object, disabled, dark);
  }
}

static uint32_t
_ChainFlags(struct style_chain const *chain, int index, struct Object *object)
{
  return index < chain->count ? chain->flags[index] : OBJ_GetStyleFlags(object);
}

// Split "Type.class#name:hover" into its compiled fields. A part that cannot
// match anything keeps STYLE_INVALID so the rule is skipped without parsing.
static void
_CompileSelectorPart(struct style_selector_part *part)
{
  char base[MAX_PROPERTY_STRING];
  _SplitSelectorPseudo(part->selector, base, sizeof(base), &part->pseudo);
  lpcString_t p = base;
  if (!*p) goto invalid;
  if (*p != '.' && *p != '#') {
    lpcString_t start = p;
    while (*p && *p != '.' && *p != '#') p++;
    shortStr_t token;
    _CopyTrim(token, sizeof(token), start, p);
    if (!strcmp(token, "body")) part->body = TRUE;
    else {
      strcpy(part->type, token);
      part->typeHash = fnv1a32(token);
      // Compatibility with programmatic StyleRule users: a bare selector
      // still matches a style class. CSS authors should prefer ".class".
      part->bare = !*p;
    }
  }
  while (*p) {
    char marker = *p++;
    lpcString_t start = p;
    while (*p && *p != '.' && *p != '#') p++;
    if (start == p) goto invalid;
    if (marker == '#') {
      _CopyTrim(part->name, sizeof(part->name), start, p);
      continue;
    }
    if (part->classCount >= MAX_STYLE_PART_CLASSES) goto invalid;
    _CopyTrim(part->classes[part->classCount], sizeof(shortStr_t), start, p);
    part->classHashes[part->classCount] = fnv1a32(part->classes[part->classCount]);
    part->classCount++;
  }
  return;
invalid:
  part->pseudo |= STYLE_INVALID;
}

static bool_t
_MatchSimpleSelectorImpl(struct Object *target,
                         struct style_selector_part const *part,
                         uint32_t objFlags,
                         struct style_class_selector** matchedClass)
{
  if ((objFlags & part->pseudo) != part->pseudo) return FALSE;
  if (part->body && OBJ_GetParent(target)) return FALSE;
  if (part->type[0]) {
    lpcString_t className = OBJ_GetClassName(target);
    if (!className || strcmp(className, part->type)) {
      if (!part->bare) return FALSE;
      struct style_class_selector *cls = _FindMatchingClass(target, part->typeHash, part->type, objFlags);
      if (!cls) return FALSE;
      if (matchedClass) *matchedClass = cls;
    }
  }
  if (part->name[0]) {
    lpcString_t name = OBJ_GetName(target);
    if (!name || strcmp(name, part->name)) return FALSE;
  }
  FOR_LOOP(i, part->classCount) {
    struct style_class_selector* cls =
      _FindMatchingClass(target, part->classHashes[i], part->classes[i], objFlags);
    if (!cls) return FALSE;
    if (matchedClass && !*matchedClass) *matchedClass = cls;
  }
  return TRUE;
}

static bool_t
_MatchSimpleSelector(struct Object *target, struct style_selector_part const *part,
                     uint32_t objFlags, struct style_class_selector **matchedClass,
                     bool_t discoverHover)
{
  bool_t matched = _MatchSimpleSelectorImpl(target, part,
                                            discoverHover ? ~STYLE_INVALID : objFlags,
                                            matchedClass);
  if (matched && discoverHover && (part->pseudo & STYLE_HOVER))
    OBJ_SetFlags(target, OBJ_GetFlags(target) | OF_HOVERABLE);
  return matched;
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
  FOR_LOOP(i, count) {
    _CompileSelectorPart(&compiled->parts[i]);
    compiled->pseudo |= compiled->parts[i].pseudo;
  }
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
                       struct style_class_selector** matchedClass,
                       bool_t discoverHover,
                       struct style_chain const *chain)
{
  if (!selector || selector->count == 0) return FALSE;

  if (!_MatchSimpleSelector(target, &selector->parts[selector->count - 1], objFlags,
                            matchedClass, discoverHover)) {
    return FALSE;
  }

  struct Object *cursor = target;
  int depth = 0;
  for (int i = selector->count - 1; i > 0; i--) {
    struct style_selector_part const *part = &selector->parts[i - 1];
    if (selector->parts[i].combinator == STYLE_COMBINATOR_CHILD) {
      cursor = OBJ_GetParent(cursor);
      depth++;
      if (!cursor) return FALSE;
      if (!_MatchSimpleSelector(cursor, part, _ChainFlags(chain, depth, cursor), NULL, discoverHover))
        return FALSE;
      continue;
    }

    struct Object *ancestor = OBJ_GetParent(cursor);
    depth++;
    while (ancestor) {
      if (_MatchSimpleSelector(ancestor, part, _ChainFlags(chain, depth, ancestor), NULL, discoverHover))
        break;
      ancestor = OBJ_GetParent(ancestor);
      depth++;
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
                      struct style_class_selector** matchedClass,
                      struct style_chain const *chain)
{
  if (!sr || !sr->selector) return FALSE;
  if (matchedClass) *matchedClass = NULL;

  struct style_selector *selector = sr->selector;
  if (selector->pseudo & STYLE_HOVER)
    _SelectorMatchesTarget(target, selector, objFlags, NULL, TRUE, chain);
  return _SelectorMatchesTarget(target, selector, objFlags, matchedClass, FALSE, chain);
}

// Apply all C property overrides from ruleObj to the target object.
// ruleObj->properties contains the pre-parsed overrides (PF_PROPERTY_TYPE not set).
// ruleFlags: the rule's state flags (0 = default, non-zero = pseudo-state rule).
// cls: the resolved class selector; carries per-usage opacity (cls->opacity).
static void
_ApplyStyleRule(struct Object *target, struct Object *ruleObj,
                uint32_t ruleFlags, struct style_class_selector* cls,
                bool_t defaults)
{
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

    struct Property *hprop = OBJ_FindLongProperty(target, pdesc->FullIdentifier);
    if (!hprop) { skip_hprop++; continue; }

    // Like a browser's user-agent sheet, the default stylesheet never
    // overrides a value authored inline in XML or Lua, in any state.
    uint32_t pflags = PROP_GetFlags(hprop);
    if (defaults && (pflags & PF_MODIFIED) && !(pflags & PF_STYLED)) continue;

    if (ruleFlags) {
      PROP_SetFlag(hprop, PF_SPECIALIZED);
    }

    // Check if the rule property has a binding (e.g., from CSS unit like "1.5rem")
    // If so, transfer the binding to the target property instead of copying the value.
    // The binding expression was created during CSS parsing and resolves relative
    // to the styled object's parent chain.
    if (PROP_HasProgram(rp)) {
      lpcString_t expr = PROP_GetBindingExpression(rp);
      if (expr && expr[0]) {
        PROP_SetBinding(hprop, expr, kBindingModeOneWay, TRUE);
        PROP_Update(hprop);  // Force immediate evaluation of the binding
        PROP_SetFlag(hprop, PF_STYLED);
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
    PROP_SetFlag(hprop, PF_STYLED);
  }
}

// Walk one stylesheet's rules for a matching selector and invoke _ApplyStyleRule.
static void
_EnumSheetRules(struct Object *sheetObj, struct Object *target,
                uint32_t objFlags, bool_t defaults, struct style_chain const *chain)
{
  if (!sheetObj) return;
  bool_t stateClasses = FALSE;
  struct StyleController *sc = GetStyleController(target);
  if (sc) {
    FOR_EACH_LIST(struct style_class_selector, cls, sc->classes)
      if (cls->flags) { stateClasses = TRUE; break; }
  }
  // State rules override base rules within a sheet. Later sheets can override
  // either, including an application's base skin over global dark/checked rules.
  for (int statePass = 0; statePass < 2; statePass++) {
    FOR_EACH_OBJECT(child, sheetObj) {
      struct StyleRule* sr = GetStyleRule(child);
      if (!sr) continue;
      // Rules created outside StyleRule.PropertyChanged compile on first use.
      if (!sr->selector && sr->ClassName && sr->ClassName[0])
        sr->selector = OBJ_CompileStyleSelector(sr->ClassName);
      if (!sr->selector) continue;
      bool_t selectorState = sr->flags || sr->selector->pseudo;
      if ((!statePass && selectorState) || (statePass && !selectorState && !stateClasses)) continue;
      struct style_class_selector* cls = NULL;
      if (!_StyleRuleBaseMatches(target, sr, objFlags, &cls, chain)) continue;
      if (sr->flags & STYLE_HOVER)
        OBJ_SetFlags(target, OBJ_GetFlags(target) | OF_HOVERABLE);
      if ((objFlags & sr->flags) != sr->flags) continue;
      uint32_t state = sr->flags | (cls ? cls->flags : 0);
      if (selectorState) state |= STYLE_SELECT;
      if (!!state != statePass) continue;
      _ApplyStyleRule(target, child, state, cls, defaults);
    }
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
  struct style_chain chain;
  _BuildStyleChain(pobj, &chain);
  uint32_t objFlags = chain.flags[0];

  // Global stylesheet first
  _EnumSheetRules(static_stylesheet, pobj, objFlags, TRUE, &chain);

  // Walk up the object hierarchy
  for (struct Object *p = pobj; p; p = OBJ_GetParent(p)) {
    struct StyleController* sc = GetStyleController(p);
    if (!sc || !sc->StyleSheet) continue;
    _EnumSheetRules(CMP_GetObject(sc->StyleSheet), pobj, objFlags, FALSE, &chain);
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
  _SendMessage(hObject, StyleController, ThemeChanged, .recursive = TRUE);
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
  _SendMessage(hObject, StyleController, ThemeChanged, .recursive = TRUE);
  return TRUE;
}

/* StateClasses owns the classes it added: a new value replaces only those, so
 * authored classes survive and descendant selectors restyle with the record. */
HANDLER(StyleController, Object, PropertyChanged)
{
  if (!pPropertyChanged || !pPropertyChanged->Property) return FALSE;
  uint32_t property = PROP_GetLongIdentifier(pPropertyChanged->Property);
  if (property == ID_StyleController_Appearance) {
    _SendMessage(hObject, StyleController, ThemeChanged, .recursive = TRUE);
    OBJ_SetDirty(hObject);
    return FALSE;
  }
  if (property != ID_StyleController_StateClasses) return FALSE;
  struct style_class_selector **link = &pStyleController->classes;
  while (*link) {
    struct style_class_selector *cls = *link;
    if (!cls->state) { link = &cls->next; continue; }
    *link = cls->next;
    free(cls);
  }
  if (pStyleController->StateClasses && *pStyleController->StateClasses) {
    WITH(char, classes, strdup(pStyleController->StateClasses), free)
    {
      for (lpcString_t s = strtok(classes, " "); s; s = strtok(NULL, " ")) {
        struct style_class_selector *cls = _ParseClass(s);
        cls->state = TRUE;
        _AddClass(hObject, cls);
      }
    }
  }
  _SendMessage(hObject, StyleController, ThemeChanged, .recursive = TRUE);
  return FALSE;
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
    if (!n || cls->state) continue;
    if (pos > 0 && pos < len - 1) buf[pos++] = ' ';
    if (pos + n >= len) break;
    memcpy(buf + pos, cls->value, n);
    pos += n;
    buf[pos] = 0;
  }
  return pos > 0 ? buf : NULL;
}

