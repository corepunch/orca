#include <source/core/core_local.h>
#include <source/core/core_properties.h>

// ============================================================================
// StyleRule COMPONENT
// ============================================================================
//
// StyleRule is a standalone object that is a child of a StyleSheet object.
// It holds:
//   - Selector: CSS selector string (e.g., ".button:hover")
//   - class_id, flags, opacity: cached fields derived from Selector
//   - Attached C properties: the CSS property overrides (e.g., Width=42.0f).
//     These are stored as Property objects in ruleObj->properties with
//     PF_PROPERTY_TYPE NOT set (so they don't belong to StyleRule's component).
//
// When ThemeChanged fires, _ApplyStyleRule in StyleController.c iterates these
// attached properties and copies their pre-parsed values to matching properties
// on the target object — no string parsing on apply.

// Update cached fields from the Selector string
HANDLER(StyleRule, Object, PropertyChanged) {
  if (!pPropertyChanged || !pPropertyChanged->Property) return FALSE;
  if (PROP_GetLongIdentifier(pPropertyChanged->Property) != ID_StyleRule_Selector) return FALSE;

  const char* selector = pStyleRule->Selector ? pStyleRule->Selector : "";
  lpcString_t base = (*selector == '.') ? selector + 1 : selector;

  // Truncate at first ':' or '/' to get the base name for hashing
  lpcString_t colon  = strchr(base, ':');
  lpcString_t slash  = strchr(base, '/');
  lpcString_t end    = NULL;
  if (colon && slash) end = (colon < slash) ? colon : slash;
  else if (colon)     end = colon;
  else if (slash)     end = slash;

  if (end) {
    char buf[sizeof(shortStr_t)];
    size_t len = MIN((size_t)(end - base), sizeof(buf) - 1);
    memcpy(buf, base, len);
    buf[len] = '\0';
    pStyleRule->class_id = fnv1a32(buf);
  } else {
    pStyleRule->class_id = fnv1a32(base);
  }

  // Parse pseudo-state flags from colon-separated tokens
  pStyleRule->flags = 0;
  for (lpcString_t tok = strchr(selector, ':'); tok; tok = strchr(tok + 1, ':')) {
    lpcString_t s = tok + 1;
    lpcString_t e = strchr(s, ':');
    size_t len = e ? (size_t)(e - s) : strlen(s);
    if      (len == 5 && !strncmp(s, "hover",  5)) pStyleRule->flags |= STYLE_HOVER;
    else if (len == 5 && !strncmp(s, "focus",  5)) pStyleRule->flags |= STYLE_FOCUS;
    else if (len == 6 && !strncmp(s, "active", 6)) pStyleRule->flags |= STYLE_SELECT;
    else if (len == 4 && !strncmp(s, "dark",   4)) pStyleRule->flags |= STYLE_DARK;
  }

  // Parse opacity from /N syntax
  lpcString_t op = strchr(selector, '/');
  pStyleRule->opacity = op ? (byte_t)atoi(op + 1) : 100;

  return FALSE;
}
