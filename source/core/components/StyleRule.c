#include <source/core/core_local.h>
#include <source/core/core_properties.h>

// ============================================================================
// StyleRule COMPONENT
// ============================================================================
//
// StyleRule is a standalone object that is a child of a StyleSheet object.
// It holds:
//   - ClassName:  the base class name (e.g., "button") — matches nodes that
//                 carry this class in their class attribute.
//   - PseudoClass: colon-separated pseudo-state qualifiers (e.g., "hover" or
//                 "hover:focus") — empty means the rule applies to all states.
//   - class_id:  FNV1a32 of ClassName, cached for fast matching.
//   - flags:     STYLE_* bitmask derived from PseudoClass, cached.
//   - Attached C properties: the CSS property overrides (e.g., Width=42.0f).
//     These are stored as Property objects in ruleObj->properties with
//     PF_PROPERTY_TYPE NOT set (so they don't belong to StyleRule's component).
//
// Opacity (the /N suffix) is a per-usage concern and lives in
// style_class_selector.opacity — not in the rule definition.

static uint32_t
_ParsePseudoClass(lpcString_t pseudo)
{
  if (!pseudo || !pseudo[0]) return 0;
  uint32_t flags = 0;
  // pseudo is like "hover" or "hover:focus" (no leading colon)
  for (lpcString_t tok = pseudo; tok; ) {
    lpcString_t end = strchr(tok, ':');
    size_t len = end ? (size_t)(end - tok) : strlen(tok);
    if      (len == 5 && !strncmp(tok, "hover",  5)) flags |= STYLE_HOVER;
    else if (len == 5 && !strncmp(tok, "focus",  5)) flags |= STYLE_FOCUS;
    else if (len == 6 && !strncmp(tok, "active", 6)) flags |= STYLE_SELECT;
    else if (len == 4 && !strncmp(tok, "dark",   4)) flags |= STYLE_DARK;
    tok = end ? end + 1 : NULL;
  }
  return flags;
}

// Update cached fields whenever ClassName or PseudoClass changes.
HANDLER(StyleRule, Object, PropertyChanged) {
  if (!pPropertyChanged || !pPropertyChanged->Property) return FALSE;
  uint32_t changed = PROP_GetLongIdentifier(pPropertyChanged->Property);

  if (changed == ID_StyleRule_ClassName) {
    lpcString_t name = pStyleRule->ClassName ? pStyleRule->ClassName : "";
    pStyleRule->class_id = fnv1a32(name);
  } else if (changed == ID_StyleRule_PseudoClass) {
    pStyleRule->flags = _ParsePseudoClass(pStyleRule->PseudoClass);
  }

  return FALSE;
}
