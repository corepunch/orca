// Pure-C CSS parser for UIKit.
//
// Parses a CSS text string into an ORCA StyleSheet object containing
// StyleRule children.  Each StyleRule carries:
//   - ClassName   : selector without pseudo-state (e.g. ".button", "#Logo",
//           "ImageView", "StackView > Label", or ".popup .panel")
//   - PseudoClass : colon-separated pseudo-states  (e.g. "hover:focus")
//   - ORCA property overrides stored as C properties
//
// Supported CSS subset:
//   - Block comments /* ... */
//   - Simple class selectors: .foo { ... }
//   - ID selectors: #Logo { ... }
//   - Type selectors: ImageView { ... }
//   - Compound selectors: Button.primary { ... }, #Save.primary { ... }
//   - Descendant selectors: .popup .panel { ... }
//   - Direct parent selectors: StackView > Label { ... }
//   - Comma-separated selectors: .foo, .bar { ... }
//   - Pseudo-classes: .foo:hover { ... }
//   - File imports: @import "shared.css"; (loadObjectFromCss only)
//   - @apply directives (merges another selector's declarations)
//   - Standard property declarations: key: value;

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include <UIKit/UIKit.h>
#include <renderer/renderer.h>
#include <include/api.h>
#include <source/core/property/property_internal.h>
#include <core/core_properties.h>
#include <filesystem/filesystem.h>

// parse_property is ORCA_API but not declared in orca.h; forward-declare it.
extern int parse_property(const char* str, struct PropertyType const* prop,
              void* valueptr);

static struct Object *
css_create_texture_object(const char *path)
{
  static lpcString_t const exts[] = { ".png", ".jpg", ".jpeg", ".svg", NULL };
  path_t source = {0};
  char query[MAX_PROPERTY_STRING] = {0};
  lpcString_t q = strchr(path, '?');
  int path_len = q ? (int)(q - path) : (int)strlen(path);
  snprintf(source, sizeof(source), "%.*s", path_len, path);
  if (q) snprintf(query, sizeof(query), "%s", q + 1);
  lpcString_t dot = strrchr(source, '.'), slash = strrchr(source, '/');
  if (!dot || dot < slash) {
    for (int i = 0; exts[i]; i++) {
      path_t with_ext = {0};
      snprintf(with_ext, sizeof(with_ext), "%s%s", source, exts[i]);
      if (FS_FileExists(with_ext)) { snprintf(source, sizeof(source), "%s", with_ext); break; }
    }
  }
  struct Object *image = OBJ_Create(ID_Image);
  if (!image) return NULL;
  lpcString_t src = source;
  PROP_SetValue(OBJ_FindLongProperty(image, ID_Image_Source), &src);
  for (char *arg = strtok(query, "&"); arg; arg = strtok(NULL, "&")) {
    char key[MAX_PROPERTY_STRING] = {0}, value[MAX_PROPERTY_STRING] = {0}, tmp[MAX_PROPERTY_STRING] = {0};
    char *eq = strchr(arg, '=');
    if (eq) {snprintf(key, MIN((size_t)(eq - arg) + 1, sizeof(key)), "%s", arg); snprintf(value, sizeof(value), "%s", eq + 1); }
    else { snprintf(key, sizeof(key), "%s", arg); snprintf(value, sizeof(value), "true"); }
    if (key[0] >= 'a' && key[0] <= 'z') key[0] = (char)(key[0] - 'a' + 'A');
    if (value[0] >= 'a' && value[0] <= 'z') value[0] = (char)(value[0] - 'a' + 'A');
    if (!strcmp(key, "Source")) continue;
    struct Property *prop = OBJ_FindShortProperty(image, fnv1a32(key));
    if (prop && parse_property(value, PROP_GetDesc(prop), tmp)) {
      PROP_SetValue(prop, tmp);
      if (PROP_GetType(prop) == kDataTypeString) free(*(char **)tmp);
    }
  }
  return image;
}

static bool_t
css_is_texture_path(const char *value)
{
  return value && value[0] && strcasecmp(value, "none") &&
       !strstr(value, "gradient(");
}

static void
copy_trim(char* dst, const char* s, const char* e, int n);

// ---------------------------------------------------------------------------
// CSS unit detection and binding expression generation
// ---------------------------------------------------------------------------

static bool_t
css_value_has_unit(const char *value)
{
  if (!value) return FALSE;
  size_t len = strlen(value);
  if (len < 3) return FALSE;
  while (len > 0 && isspace((unsigned char)value[len - 1])) len--;
  if (len < 3) return FALSE;
  // Only rem/em create bindings; px/pt/vw/vh/vmin/vmax are treated as literals
  static const char* const binding_units[] = { "rem", "em", NULL };
  for (int i = 0; binding_units[i]; i++) {
    size_t ulen = strlen(binding_units[i]);
    if (len > ulen && strncmp(value + len - ulen, binding_units[i], ulen) == 0) {
      size_t num_end = len - ulen;
      if (num_end > 0 && (isdigit((unsigned char)value[num_end - 1]) || value[num_end - 1] == '.')) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

static void
css_strip_unit(const char *value, char *out, size_t out_size)
{
  if (!value || !out || out_size == 0) return;
  size_t len = strlen(value);
  size_t i = 0;
  while (i < len && isspace((unsigned char)value[i])) i++;
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

static bool_t
css_get_binding_path(const char *orca_name, char *out, size_t out_size)
{
  if (!orca_name || !out || out_size == 0) return FALSE;
  if (!strcasecmp(orca_name, "TextRun.FontSize") ||
      !strcasecmp(orca_name, "Font.Size")) {
    snprintf(out, out_size, "../TextRun.FontSize");
    return TRUE;
  }
  return FALSE;
}

static bool_t
css_is_viewport_unit(const char *value)
{
  if (!value) return FALSE;
  size_t len = strlen(value);
  static const char* const viewport_units[] = { "vw", "vh", "vmin", "vmax", NULL };
  for (int i = 0; viewport_units[i]; i++) {
    size_t ulen = strlen(viewport_units[i]);
    if (len > ulen && strncmp(value + len - ulen, viewport_units[i], ulen) == 0) {
      size_t num_end = len - ulen;
      if (num_end > 0 && (isdigit((unsigned char)value[num_end - 1]) || value[num_end - 1] == '.')) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

static bool_t
css_make_binding_expr(const char *css_value, const char *orca_name,
                      char *out, size_t out_size)
{
  if (!css_value || !orca_name || !out || out_size == 0) return FALSE;

  // Handle viewport units: vw, vh, vmin, vmax
  if (css_is_viewport_unit(css_value)) {
    char num_str[32] = {0};
    css_strip_unit(css_value, num_str, sizeof(num_str));
    if (num_str[0] == '\0') return FALSE;
    double multiplier = atof(num_str);

    // Find the unit suffix (after the numeric portion)
    size_t len = strlen(css_value);
    const char *unit = css_value + len;
    while (unit > css_value && isalpha((unsigned char)unit[-1])) unit--;

    const char *dim = NULL;
    if (!strcasecmp(unit, "vw")) {
      dim = "Screen.Width";
    } else if (!strcasecmp(unit, "vh")) {
      dim = "Screen.Height";
    } else if (!strcasecmp(unit, "vmin")) {
      dim = "Screen.Height"; // approximate: use smaller dimension
    } else if (!strcasecmp(unit, "vmax")) {
      dim = "Screen.Width"; // approximate: use larger dimension
    }

    if (!dim) return FALSE;

    // vw/vh are percentages: 1vw = 1% of viewport width
    double value = multiplier / 100.0;
    if (value == 1.0) {
      snprintf(out, out_size, "{%s}", dim);
    } else {
      // Format multiplier without trailing zeros
      char mul_str[32];
      snprintf(mul_str, sizeof(mul_str), "%g", value);
      snprintf(out, out_size, "{%s} * %s", dim, mul_str);
    }
    Con_Printf("DEBUG css_make_binding_expr (viewport): css_value=%s out=%s\n", css_value, out);
    return TRUE;
  }

  char base_path[64] = {0};
  if (!css_get_binding_path(orca_name, base_path, sizeof(base_path))) {
    return FALSE;
  }
  char num_str[32] = {0};
  css_strip_unit(css_value, num_str, sizeof(num_str));
  if (num_str[0] == '\0') return FALSE;
  double multiplier = atof(num_str);
  Con_Printf("DEBUG css_make_binding_expr: css_value=%s orca_name=%s num_str=%s multiplier=%f\n",
             css_value, orca_name, num_str, multiplier);
  if (multiplier == 1.0) {
    snprintf(out, out_size, "{%s}", base_path);
  } else {
    snprintf(out, out_size, "{%s} * %s", base_path, num_str);
  }
  Con_Printf("DEBUG css_make_binding_expr: out=%s\n", out);
  return TRUE;
}

#define CSS_MAX_RULES  512
#define CSS_MAX_PROPS  64
#define CSS_MAX_APPLY  8
#define CSS_MAX_IMPORT_DEPTH 16
#define CSS_MAX_SELLEN   256
#define CSS_MAX_PROPNAME 64
#define CSS_MAX_VALLEN   256

// ---------------------------------------------------------------------------
// CSS property name → ORCA full property name mapping
// ---------------------------------------------------------------------------
static const struct { const char* css; const char* orca; uint32_t id; }
k_css_prop_map[] = {
  /* Node layout */
  { "background-color",  "Node2D.BackgroundColor",    ID_Node2D_BackgroundColor },
  { "color",       "Node2D.ForegroundColor",    ID_Node2D_ForegroundColor },
  { "opacity",       "Node.Opacity",          ID_Node_Opacity },
  { "width",       "Node.Width",          ID_Node_Width },
  { "height",      "Node.Height",           ID_Node_Height },
  { "min-width",     "Node.MinWidth",         ID_Node_MinWidth },
  { "min-height",    "Node.MinHeight",        ID_Node_MinHeight },
  { "margin",      "Node.Margin",           ID_Node_Margin },
  { "margin-top",    "Node.MarginTop",        ID_Node_MarginTop },
  { "margin-right",    "Node.MarginRight",        ID_Node_MarginRight },
  { "margin-bottom",   "Node.MarginBottom",       ID_Node_MarginBottom },
  { "margin-left",     "Node.MarginLeft",         ID_Node_MarginLeft },
  { "margin-inline",   "Node.HorizontalMargin",     0 },
  { "margin-block",    "Node.VerticalMargin",       0 },
  { "padding",       "Node.Padding",          ID_Node_Padding },
  { "padding-top",     "Node.PaddingTop",         ID_Node_PaddingTop },
  { "padding-right",   "Node.PaddingRight",       ID_Node_PaddingRight },
  { "padding-bottom",  "Node.PaddingBottom",      ID_Node_PaddingBottom },
  { "padding-left",    "Node.PaddingLeft",        ID_Node_PaddingLeft },
  { "padding-inline",  "Node.HorizontalPadding",    0 },
  { "padding-block",   "Node.VerticalPadding",      0 },
  { "border",      "Node.Border",           ID_Node_Border },
  { "border-color",    "Node.BorderColor",        ID_Node_BorderColor },
  { "border-style",    "Node.BorderStyle",        ID_Node_BorderStyle },
  { "border-width",    "Node.BorderWidth",        ID_Node_BorderWidth },
  { "border-left-width", "Node.BorderWidthLeft",      ID_Node_BorderWidthLeft },
  { "border-right-width", "Node.BorderWidthRight",    ID_Node_BorderWidthRight },
  { "border-top-width",  "Node.BorderWidthTop",       ID_Node_BorderWidthTop },
  { "border-bottom-width", "Node.BorderWidthBottom",    ID_Node_BorderWidthBottom },
  { "border-radius",   "Node.BorderRadius",       ID_Node_BorderRadius },
  { "border-top-left-radius", "Node.BorderTopLeftRadius", ID_Node_BorderTopLeftRadius },
  { "border-top-right-radius", "Node.BorderTopRightRadius", ID_Node_BorderTopRightRadius },
  { "border-bottom-right-radius", "Node.BorderBottomRightRadius", ID_Node_BorderBottomRightRadius },
  { "border-bottom-left-radius", "Node.BorderBottomLeftRadius", ID_Node_BorderBottomLeftRadius },
  { "visibility",    "Node.Visible",          ID_Node_Visible },

  /* Node2D rendering */
  { "background",    "Node2D.Background",       ID_Node2D_Background },
  { "background-image",  "Node2D.BackgroundImage",    ID_Node2D_BackgroundImage },
  { "border-image-source", "Node2D.BorderImageSource",  ID_Node2D_BorderImageSource },
  { "border-image-slice", "Node2D.BorderImageSlice",    ID_Node2D_BorderImageSlice },
  { "border-image-repeat", "Node2D.BorderImageRepeat",  ID_Node2D_BorderImageRepeat },
  { "border-image",    "Node2D.BorderImage",      ID_Node2D_BorderImage },
  { "foreground",    "Node2D.Foreground",       ID_Node2D_Foreground },
  { "foreground-color",  "Node2D.ForegroundColor",    ID_Node2D_ForegroundColor },
  { "box-shadow",    "Node2D.BoxShadow",        ID_Node2D_BoxShadow },
  { "box-shadow-color",  "Node2D.BoxShadowColor",     ID_Node2D_BoxShadowColor },
  { "box-shadow-blur",   "Node2D.BoxShadowBlurRadius",  ID_Node2D_BoxShadowBlurRadius },
  { "box-shadow-blur-radius", "Node2D.BoxShadowBlurRadius", ID_Node2D_BoxShadowBlurRadius },
  { "box-shadow-spread", "Node2D.BoxShadowSpreadRadius",  ID_Node2D_BoxShadowSpreadRadius },
  { "box-shadow-spread-radius", "Node2D.BoxShadowSpreadRadius", ID_Node2D_BoxShadowSpreadRadius },
  { "overflow",      "Node2D.Overflow",         ID_Node2D_Overflow },
  { "overflow-x",    "Node2D.OverflowX",        ID_Node2D_OverflowX },
  { "overflow-y",    "Node2D.OverflowY",        ID_Node2D_OverflowY },
  { "ring",        "Node2D.Ring",           ID_Node2D_Ring },
  { "ring-color",    "Node2D.RingColor",        ID_Node2D_RingColor },
  { "ring-offset",     "Node2D.RingOffset",       ID_Node2D_RingOffset },
  { "ring-width",    "Node2D.RingWidth",        ID_Node2D_RingWidth },
  { "content-stretch",   "Node2D.ContentStretch",     ID_Node2D_ContentStretch },
  { "ignore-hit-test",   "Node2D.IgnoreHitTest",      ID_Node2D_IgnoreHitTest },
  { "pointer-events",  "Node2D.IgnoreHitTest",      ID_Node2D_IgnoreHitTest },
  { "size-to-content",   "Node2D.SizeToContent",      ID_Node2D_SizeToContent },
  { "snap-to-pixel",   "Node2D.SnapToPixel",      ID_Node2D_SnapToPixel },

  /* StackView */
  { "align-items",     "StackView.AlignItems",       ID_StackView_AlignItems },
  { "align-items",     "Grid.AlignItems",        ID_Grid_AlignItems },
  { "justify-content",   "StackView.JustifyContent",    ID_StackView_JustifyContent },
  { "flex-direction",  "StackView.Direction",       ID_StackView_Direction },
  { "direction",     "StackView.Direction",       ID_StackView_Direction },
  { "gap",         "StackView.Spacing",       ID_StackView_Spacing },
  { "gap",         "Grid.Spacing",          ID_Grid_Spacing },
  { "spacing",       "StackView.Spacing",       ID_StackView_Spacing },
  { "reversed",      "StackView.Reversed",      ID_StackView_Reversed },

  /* Text */
  { "font",        "TextRun.Font",          ID_TextRun_Font },
  { "font-size",     "TextRun.FontSize",        ID_TextRun_FontSize },
  { "font-family",     "TextRun.FontFamily",      ID_TextRun_FontFamily },
  { "font-weight",     "TextRun.FontWeight",      ID_TextRun_FontWeight },
  { "font-style",    "TextRun.FontStyle",       ID_TextRun_FontStyle },
  { "line-height",     "TextRun.LineHeight",      ID_TextRun_LineHeight },
  { "letter-spacing",  "TextRun.LetterSpacing",     ID_TextRun_LetterSpacing },
  { "character-spacing", "TextRun.CharacterSpacing",    ID_TextRun_CharacterSpacing },
  { "fixed-character-width", "TextRun.FixedCharacterWidth", ID_TextRun_FixedCharacterWidth },
  { "text-decoration",   "TextRun.TextDecoration",       ID_TextRun_TextDecoration },
  { "text-decoration-color", "TextRun.TextDecorationColor",  ID_TextRun_TextDecorationColor },
  { "text-decoration-thickness", "TextRun.TextDecorationWidth", ID_TextRun_TextDecorationWidth },
  { "text-decoration-style", "TextRun.TextDecorationStyle",       ID_TextRun_TextDecorationStyle },
  { "text-decoration-color", "TextRun.TextDecorationColor",    ID_TextRun_TextDecorationColor },
  { "text-decoration-width",   "TextRun.TextDecorationWidth",    ID_TextRun_TextDecorationWidth },
  { "text-decoration-offset", "TextRun.TextDecorationOffset",     ID_TextRun_TextDecorationOffset },
  { "word-wrap",     "TextBlockConcept.WordWrap",   ID_TextBlockConcept_WordWrap },
  { "overflow-wrap",   "TextBlockConcept.WordWrap",   ID_TextBlockConcept_WordWrap },
  { "text-wrap",     "TextBlockConcept.TextWrapping", ID_TextBlockConcept_TextWrapping },
  { "text-overflow",   "TextBlockConcept.TextOverflow", ID_TextBlockConcept_TextOverflow },
  { "text-align",    "TextBlockConcept.TextHorizontalAlignment", ID_TextBlockConcept_TextHorizontalAlignment },
  { "text-vertical-align", "TextBlockConcept.TextVerticalAlignment", ID_TextBlockConcept_TextVerticalAlignment },
  { "placeholder-color", "TextBlockConcept.PlaceholderColor", ID_TextBlockConcept_PlaceholderColor },

  /* Custom properties */
  { "grid-template-columns", "Grid.Columns", ID_Grid_Columns },
  { "grid-template-rows", "Grid.Rows", ID_Grid_Rows },
  { NULL, NULL, 0 }
};

static bool_t
css_name_equals_enum_value(const char *css_value, const char *enum_value)
{
  while (*css_value || *enum_value) {
    while (*css_value == '-' || *css_value == '_' || isspace((unsigned char)*css_value)) {
      css_value++;
    }
    while (*enum_value == '-' || *enum_value == '_' || isspace((unsigned char)*enum_value)) {
      enum_value++;
    }
    if (tolower((unsigned char)*css_value) != tolower((unsigned char)*enum_value)) {
      return FALSE;
    }
    if (*css_value) css_value++;
    if (*enum_value) enum_value++;
  }
  return TRUE;
}

static void
css_normalize_margin_auto_tokens(char *value, size_t value_size)
{
  char normalized[CSS_MAX_VALLEN] = {0};
  const char *p = value;
  size_t used = 0;
  bool_t first = TRUE;

  while (*p) {
    char token[CSS_MAX_VALLEN] = {0};
    const char *start;
    size_t len;
    const char *mapped;
    int wrote;

    while (*p && isspace((unsigned char)*p)) p++;
    if (!*p) break;

    start = p;
    while (*p && !isspace((unsigned char)*p)) p++;
    len = (size_t)(p - start);
    if (len >= sizeof(token)) len = sizeof(token) - 1;
    memcpy(token, start, len);
    token[len] = '\0';

    mapped = !strcasecmp(token, "auto") ? "nan" : token;
    wrote = snprintf(normalized + used, sizeof(normalized) - used,
             "%s%s", first ? "" : " ", mapped);
    if (wrote < 0 || (size_t)wrote >= sizeof(normalized) - used) {
      break;
    }
    used += (size_t)wrote;
    first = FALSE;
  }

  if (normalized[0]) {
    strncpy(value, normalized, value_size - 1);
    value[value_size - 1] = '\0';
  }
}

static const char*
css_normalize_decl_value(const char *css_key,
             const char *css_value,
             char *out,
             size_t out_size)
{
  if (!strcasecmp(css_key, "margin") ||
    !strcasecmp(css_key, "padding") ||
    !strcasecmp(css_key, "border-width") ||
    !strcasecmp(css_key, "border-image-slice")) {
    char values[4][CSS_MAX_VALLEN] = {{0}};
    int count = 0;
    const char *p = css_value;

    while (*p && count < 4) {
      while (*p && isspace((unsigned char)*p)) p++;
      const char *start = p;
      while (*p && !isspace((unsigned char)*p)) p++;
      if (start < p) {
        copy_trim(values[count], start, p, (int)sizeof(values[count]));
        count++;
      }
    }

    if (count == 2) {
      // CSS: vertical horizontal. ORCA Thickness: left top right bottom.
      snprintf(out, out_size, "%s %s %s %s",
           values[1], values[0], values[1], values[0]);
      if (!strncasecmp(css_key, "margin", 6)) {
        css_normalize_margin_auto_tokens(out, out_size);
      }
      return out;
    }

    if (count == 3) {
      // CSS: top horizontal bottom. ORCA: left top right bottom.
      snprintf(out, out_size, "%s %s %s %s",
           values[1], values[0], values[1], values[2]);
      if (!strncasecmp(css_key, "margin", 6)) {
        css_normalize_margin_auto_tokens(out, out_size);
      }
      return out;
    }

    if (count == 4) {
      // CSS: top right bottom left. ORCA: left top right bottom.
      snprintf(out, out_size, "%s %s %s %s",
           values[3], values[0], values[1], values[2]);
      if (!strncasecmp(css_key, "margin", 6)) {
        css_normalize_margin_auto_tokens(out, out_size);
      }
      return out;
    }
  }

  if (!strcasecmp(css_key, "visibility")) {
    if (!strcasecmp(css_value, "hidden") || !strcasecmp(css_value, "collapse")) {
      snprintf(out, out_size, "false");
      return out;
    }
    if (!strcasecmp(css_value, "visible")) {
      snprintf(out, out_size, "true");
      return out;
    }
  }

  if ((!strcasecmp(css_key, "width") ||
     !strcasecmp(css_key, "height") ||
     !strncasecmp(css_key, "margin", 6)) &&
    !strcasecmp(css_value, "auto")) {
    snprintf(out, out_size, "nan");
    return out;
  }

  if (!strcasecmp(css_key, "width") &&
    !strcasecmp(css_value, "100%")) {
    snprintf(out, out_size, "nan");
    return out;
  }

  if (!strcasecmp(css_key, "pointer-events")) {
    if (!strcasecmp(css_value, "none")) {
      snprintf(out, out_size, "true");
      return out;
    }
    if (!strcasecmp(css_value, "auto")) {
      snprintf(out, out_size, "false");
      return out;
    }
  }

  return css_value;
}

static bool_t
css_is_generic_font_family(const char *name)
{
  static const char *generics[] = {
    "serif", "sans-serif", "monospace", "cursive", "fantasy",
    "system", "system-ui", "ui-serif", "ui-sans-serif", "ui-monospace",
    "emoji", "math", "fangsong", "-apple-system", "BlinkMacSystemFont",
    NULL
  };
  for (int i = 0; generics[i]; i++) {
    if (!strcasecmp(name, generics[i])) return TRUE;
  }
  return FALSE;
}

static void
css_copy_font_token(char *dst, size_t dst_size, const char *start, const char *end)
{
  while (start < end && isspace((unsigned char)*start)) start++;
  while (end > start && isspace((unsigned char)end[-1])) end--;
  if (end - start >= 2 &&
    ((*start == '"' && end[-1] == '"') || (*start == '\'' && end[-1] == '\''))) {
    start++;
    end--;
    while (start < end && isspace((unsigned char)*start)) start++;
    while (end > start && isspace((unsigned char)end[-1])) end--;
  }
  size_t len = (size_t)(end - start);
  if (len >= dst_size) len = dst_size - 1;
  memcpy(dst, start, len);
  dst[len] = '\0';
}

static bool_t
css_font_path_exists(const char *path)
{
  path_t with_ext = {0};
  if (FS_FileExists(path)) return TRUE;
  snprintf(with_ext, sizeof(with_ext), "%s.xml", path);
  return FS_FileExists(with_ext);
}

static const char*
css_resolve_font_family(const char *value, char *out, size_t out_size)
{
  const char *p = value;
  while (*p) {
    char quote = 0;
    const char *start = p;
    const char *end;
    while (*start && (isspace((unsigned char)*start) || *start == ',')) start++;
    if (!*start) break;
    end = start;
    while (*end) {
      if (quote) {
        if (*end == quote) quote = 0;
      } else if (*end == '"' || *end == '\'') {
        quote = *end;
      } else if (*end == ',') {
        break;
      }
      end++;
    }
    char name[MAX_PROPERTY_STRING] = {0};
    css_copy_font_token(name, sizeof(name), start, end);
    if (name[0]) {
      lpcString_t registered = CORE_FindFontFamily(name);
      if (registered) {
        snprintf(out, out_size, "%s", registered);
        return out;
      }
      if (!css_is_generic_font_family(name) && css_font_path_exists(name)) {
        snprintf(out, out_size, "%s", name);
        return out;
      }
    }
    p = *end == ',' ? end + 1 : end;
  }
  return value;
}

// Expand CSS font shorthand: "font: 14px/1.5 sans-serif" → separate properties.
// Returns number of expansions written to out_k/out_v arrays (max 4).
static int
css_expand_font_shorthand(const char *value,
              char out_k[][CSS_MAX_PROPNAME],
              char out_v[][CSS_MAX_VALLEN],
              int max_out)
{
  if (!value || max_out < 1) return 0;
  int n = 0;
  const char *p = value;
  while (*p && isspace((unsigned char)*p)) p++;

  // Parse size (optional with /line-height)
  char size_str[CSS_MAX_VALLEN] = {0};
  char lh_str[CSS_MAX_VALLEN] = {0};
  {
    const char *start = p;
    while (*p && !isspace((unsigned char)*p) && *p != '/') p++;
    size_t len = (size_t)(p - start);
    if (len >= sizeof(size_str)) len = sizeof(size_str) - 1;
    memcpy(size_str, start, len);
    size_str[len] = '\0';
  }

  // Parse optional /line-height
  if (*p == '/') {
    p++;
    const char *start = p;
    while (*p && !isspace((unsigned char)*p)) p++;
    size_t len = (size_t)(p - start);
    if (len >= sizeof(lh_str)) len = sizeof(lh_str) - 1;
    memcpy(lh_str, start, len);
    lh_str[len] = '\0';
  }

  // Parse font family (rest of value)
  char family_str[CSS_MAX_VALLEN] = {0};
  while (*p && isspace((unsigned char)*p)) p++;
  if (*p) {
    snprintf(family_str, sizeof(family_str), "%s", p);
  }

  // Emit font-size
  if (size_str[0] && n < max_out) {
    snprintf(out_k[n], CSS_MAX_PROPNAME, "font-size");
    snprintf(out_v[n], CSS_MAX_VALLEN, "%s", size_str);
    n++;
  }

  // Emit line-height
  if (lh_str[0] && n < max_out) {
    snprintf(out_k[n], CSS_MAX_PROPNAME, "line-height");
    snprintf(out_v[n], CSS_MAX_VALLEN, "%s", lh_str);
    n++;
  }

  // Emit font-family
  if (family_str[0] && n < max_out) {
    snprintf(out_k[n], CSS_MAX_PROPNAME, "font-family");
    snprintf(out_v[n], CSS_MAX_VALLEN, "%s", family_str);
    n++;
  }

  return n;
}

// ---------------------------------------------------------------------------
// Parsed CSS in-memory representation
// ---------------------------------------------------------------------------
typedef struct {
  char key[CSS_MAX_PROPNAME]; // CSS name, e.g. "opacity"
  char val[CSS_MAX_VALLEN];   // CSS value, e.g. "0.4"
} css_decl_t;

typedef struct {
  char     sel[CSS_MAX_SELLEN];         // trimmed selector
  css_decl_t decls[CSS_MAX_PROPS];
  int    ndecls;
  char     apply[CSS_MAX_APPLY][CSS_MAX_SELLEN]; // @apply selectors
  int    napply;
} css_rule_t;

typedef struct {
  css_rule_t rules[CSS_MAX_RULES];
  int    nrules;
} css_doc_t;

// ---------------------------------------------------------------------------
// Text helpers
// ---------------------------------------------------------------------------

// Copy at most (n-1) chars of [s, e) into dst; NUL-terminate; trim trailing ws.
static void
copy_trim(char* dst, const char* s, const char* e, int n)
{
  while (s < e && isspace((unsigned char)*s)) s++;
  while (e > s && isspace((unsigned char)(*(e - 1)))) e--;
  int len = (int)(e - s);
  if (len >= n) len = n - 1;
  memcpy(dst, s, len);
  dst[len] = '\0';
}

// ---------------------------------------------------------------------------
// CSS text preprocessing: strip block comments
// ---------------------------------------------------------------------------
static char*
css_strip_comments(const char* src)
{
  size_t len = strlen(src);
  char*  out = (char*)malloc(len + 1);
  if (!out) return NULL;
  char*    w = out;
  const char*  r = src;
  while (*r) {
    if (r[0] == '/' && r[1] == '*') {
      r += 2;
      while (*r && !(r[0] == '*' && r[1] == '/')) {
        if (*r == '\n') *w++ = '\n'; // preserve newlines for positions
        r++;
      }
      if (*r) r += 2;
    } else {
      *w++ = *r++;
    }
  }
  *w = '\0';
  return out;
}

// ---------------------------------------------------------------------------
// CSS parsing: extract rules from a preprocessed block
// ---------------------------------------------------------------------------

// Find or create a rule entry for `selector` in `doc`.
static css_rule_t*
css_get_rule(css_doc_t* doc, const char* selector)
{
  for (int i = 0; i < doc->nrules; i++) {
    if (!strcmp(doc->rules[i].sel, selector))
      return &doc->rules[i];
  }
  if (doc->nrules >= CSS_MAX_RULES) {
    Con_Printf("css_parser: too many rules (max %d)", CSS_MAX_RULES);
    return NULL;
  }
  css_rule_t* r = &doc->rules[doc->nrules++];
  memset(r, 0, sizeof(*r));
  snprintf(r->sel, sizeof(r->sel), "%s", selector);
  return r;
}

// Set (or overwrite) a declaration in `rule`.
static void
css_rule_set(css_rule_t* rule, const char* key, const char* val)
{
  // overwrite if key already exists
  for (int i = 0; i < rule->ndecls; i++) {
    if (!strcasecmp(rule->decls[i].key, key)) {
      snprintf(rule->decls[i].val, sizeof(rule->decls[i].val), "%s", val);
      return;
    }
  }
  if (rule->ndecls >= CSS_MAX_PROPS) {
    Con_Printf("css_parser: rule '%s' has too many declarations (max %d), '%s' dropped",
           rule->sel, CSS_MAX_PROPS, key);
    return;
  }
  snprintf(rule->decls[rule->ndecls].key, sizeof(rule->decls[rule->ndecls].key), "%s", key);
  snprintf(rule->decls[rule->ndecls].val, sizeof(rule->decls[rule->ndecls].val), "%s", val);
  rule->ndecls++;
}

// Parse declarations inside a { ... } block and store them in `rule`.
static void
css_parse_block(css_rule_t* rule, const char* block_start, const char* block_end)
{
  const char* p = block_start;
  while (p < block_end) {
    // find next semicolon
    const char* semi = p;
    while (semi < block_end && *semi != ';') semi++;

    const char* colon = p;
    while (colon < semi && *colon != ':') colon++;
    if (colon >= semi) { p = semi + 1; continue; }

    char key[CSS_MAX_PROPNAME] = {0};
    char val[CSS_MAX_VALLEN]   = {0};
    copy_trim(key, p, colon,   (int)sizeof(key));
    copy_trim(val, colon + 1, semi, (int)sizeof(val));

    if (!key[0] || !val[0]) { p = semi + 1; continue; }

    if (!strcmp(key, "@apply")) {
      // accumulate @apply selectors separated by spaces
      const char* t = val;
      while (*t) {
        while (*t && isspace((unsigned char)*t)) t++;
        const char* te = t;
        while (*te && !isspace((unsigned char)*te)) te++;
        if (te > t) {
          if (rule->napply < CSS_MAX_APPLY) {
            char sel[CSS_MAX_SELLEN] = {0};
            copy_trim(sel, t, te, (int)sizeof(sel));
            snprintf(rule->apply[rule->napply++],
                 sizeof(rule->apply[0]), "%s", sel);
          } else {
            Con_Printf("css_parser: rule '%s' has too many @apply entries (max %d), entry dropped",
                   rule->sel, CSS_MAX_APPLY);
          }
        }
        t = te;
      }
    } else {
      css_rule_set(rule, key, val);
    }
    p = semi + 1;
  }
}

// Parse the whole CSS text (comments already stripped) into `doc`.
static void
css_parse_text(css_doc_t* doc, const char* css)
{
  const char* p = css;
  while (*p) {
    // find opening brace
    const char* brace_open = strchr(p, '{');
    if (!brace_open) break;

    // find matching closing brace (simple, no nesting)
    const char* brace_close = strchr(brace_open + 1, '}');
    if (!brace_close) break;

    // --- selector list (comma-separated) before '{'
    const char* sel_start = p;

    // accumulate selectors separated by commas
    const char* s = sel_start;
    while (s < brace_open) {
      const char* comma = s;
      while (comma < brace_open && *comma != ',') comma++;

      char sel[CSS_MAX_SELLEN] = {0};
      copy_trim(sel, s, comma, (int)sizeof(sel));
      if (sel[0]) {
        css_rule_t* rule = css_get_rule(doc, sel);
        if (rule)
          css_parse_block(rule, brace_open + 1, brace_close);
      }
      s = comma + 1;
    }

    p = brace_close + 1;
  }
}

// ---------------------------------------------------------------------------
// @apply resolution: merge declarations from referenced selectors
// ---------------------------------------------------------------------------
static void
css_resolve_apply(css_doc_t* doc)
{
  int max_passes = 10;
  bool_t changed = TRUE;
  while (changed && max_passes-- > 0) {
    changed = FALSE;
    for (int i = 0; i < doc->nrules; i++) {
      css_rule_t* rule = &doc->rules[i];
      if (!rule->napply) continue;
      for (int ai = 0; ai < rule->napply; ai++) {
        const char* ref = rule->apply[ai];
        // try ".foo" and "foo" interchangeably
        css_rule_t* src = NULL;
        for (int j = 0; j < doc->nrules; j++) {
          if (!strcmp(doc->rules[j].sel, ref)) { src = &doc->rules[j]; break; }
        }
        if (!src && ref[0] == '.') {
          for (int j = 0; j < doc->nrules; j++) {
            if (!strcmp(doc->rules[j].sel, ref + 1)) { src = &doc->rules[j]; break; }
          }
        }
        if (!src) {
          // try ".ref" when ref has no leading dot
          char dotref[CSS_MAX_SELLEN] = {0};
          snprintf(dotref, sizeof(dotref), ".%s", ref);
          for (int j = 0; j < doc->nrules; j++) {
            if (!strcmp(doc->rules[j].sel, dotref)) { src = &doc->rules[j]; break; }
          }
        }
        if (!src) continue;
        for (int di = 0; di < src->ndecls; di++) {
          // only merge if key not already set
          bool_t found = FALSE;
          for (int k = 0; k < rule->ndecls; k++) {
            if (!strcasecmp(rule->decls[k].key, src->decls[di].key)) { found = TRUE; break; }
          }
          if (!found) {
            css_rule_set(rule, src->decls[di].key, src->decls[di].val);
            changed = TRUE;
          }
        }
      }
    }
  }
  // Clear @apply lists only after resolution has converged so that
  // transitive chains (A→B→C) can be followed on subsequent passes.
  for (int i = 0; i < doc->nrules; i++) {
    doc->rules[i].napply = 0;
    memset(doc->rules[i].apply, 0, sizeof(doc->rules[i].apply));
  }
}

// ---------------------------------------------------------------------------
// Selector → (ClassName, PseudoClass) split
// ---------------------------------------------------------------------------

// selector: ".button:hover"   → class_out=".button", pseudo_out="hover"
// selector: "#Logo"       → class_out="#Logo",   pseudo_out=""
// selector: "StackView > Label" → class_out="StackView > Label", pseudo_out=""
// selector: ".popup .panel"   → class_out=".popup .panel", pseudo_out=""
static void
css_split_selector(const char* selector,
           char* class_out, int class_max,
           char* pseudo_out, int pseudo_max)
{
  const char* base = selector;
  const char* colon = strchr(base, ':');
  if (colon) {
    int len = (int)(colon - base);
    if (len >= class_max) len = class_max - 1;
    copy_trim(class_out, base, base + len, class_max);
    snprintf(pseudo_out, pseudo_max, "%s", colon + 1);
  } else {
    copy_trim(class_out, base, base + strlen(base), class_max);
    pseudo_out[0] = '\0';
  }
}

static void
css_copy_cstr(char *dst, size_t dst_size, const char *src)
{
  if (!dst || dst_size == 0) return;
  if (!src) src = "";
  strncpy(dst, src, dst_size - 1);
  dst[dst_size - 1] = '\0';
}

static const char*
css_resolve_theme_value(const char *value, char *out, size_t out_size)
{
  if (!value || !out || out_size == 0) return value;

  char text[CSS_MAX_VALLEN] = {0};
  copy_trim(text, value, value + strlen(value), (int)sizeof(text));
  if (!text[0]) return value;

  if (strncasecmp(text, "var(", 4)) {
    return value;
  }

  const char *start = text + 4;
  const char *end = text + strlen(text);
  while (end > start && isspace((unsigned char)end[-1])) end--;
  if (end <= start || end[-1] != ')') {
    return value;
  }
  end--;

  const char *comma = start;
  while (comma < end && *comma != ',') comma++;

  char token[CSS_MAX_VALLEN] = {0};
  copy_trim(token, start, comma, (int)sizeof(token));
  if (token[0] == '-' && token[1] == '-') {
    lpcString_t theme_value = FS_GetThemeValue(token + 2);
    if (theme_value) {
      css_copy_cstr(out, out_size, theme_value);
      return out;
    }
  }

  if (comma < end) {
    char fallback[CSS_MAX_VALLEN] = {0};
    char resolved_fallback[CSS_MAX_VALLEN] = {0};
    copy_trim(fallback, comma + 1, end, (int)sizeof(fallback));
    const char *resolved = css_resolve_theme_value(fallback, resolved_fallback, sizeof(resolved_fallback));
    css_copy_cstr(out, out_size, resolved);
    return out;
  }

  return value;
}

// ---------------------------------------------------------------------------
// StyleRule property assignment
// ---------------------------------------------------------------------------

static void
css_apply_orca_value_to_rule(struct Object *rule_obj,
               uint32_t orca_id,
               const char* orca_name,
               const char* css_key,
               const char* css_value)
{
  // Check for CSS units BEFORE the shorthand handling
  // If we have a unit that creates a binding, we need to handle it differently
  if (css_value_has_unit(css_value) || css_is_viewport_unit(css_value)) {
    char binding_expr[CSS_MAX_VALLEN] = {0};
    if (css_make_binding_expr(css_value, orca_name, binding_expr, sizeof(binding_expr))) {
      // For shorthand properties (prop is NULL), we need to find/create the target property
      struct Property *prop = OBJ_FindShortProperty(rule_obj, orca_id);
      if (prop) {
        PROP_SetBinding(prop, binding_expr, kBindingModeOneWay, TRUE);
      } else {
        // For shorthands, set via OBJ_SetShorthandValueFromString first to create the property,
        // then find and bind it
        OBJ_SetShorthandValueFromString(rule_obj, orca_name, css_value);
        prop = OBJ_FindShortProperty(rule_obj, orca_id);
        if (prop) {
          PROP_SetBinding(prop, binding_expr, kBindingModeOneWay, TRUE);
        }
      }
      return;
    }
    // Fall through to normal parsing for unsupported units (e.g., px, pt)
  }

  struct Property *prop = OBJ_FindShortProperty(rule_obj, orca_id);
  if (!prop) {
    OBJ_SetShorthandValueFromString(rule_obj, orca_name, css_value);
    return;
  }
  if (!PROP_GetDesc(prop)) return;

  struct PropertyType const* desc = PROP_GetDesc(prop);
  if (desc->DataType == kDataTypeObject && desc->TypeString &&
    !strcmp(desc->TypeString, "Texture")) {
    if (!css_is_texture_path(css_value)) return;
    struct Object *image = css_create_texture_object(css_value);
    if (!image) return;
    PROP_SetValue(prop, &image);
    OBJ_ReleaseRef(image);
    return;
  }

  if (desc->DataType == kDataTypeEnum) {
    lpcString_t const* values = desc->EnumValues;
    for (int i = 0; values && values[i]; i++) {
      if (css_name_equals_enum_value(css_value, values[i])) {
        PROP_SetValue(prop, &i);
        return;
      }
    }
    Con_Error("Invalid enum value %s for CSS property '%s'",
          css_value, desc->Name);
    return;
  }

  char buf[MAX_PROPERTY_STRING] = {0};
  if (parse_property(css_value, desc, buf))
    PROP_SetValue(prop, buf);
}

// Apply one CSS declaration (key/value) to an ORCA StyleRule object.
static void
css_apply_decl_to_rule(struct Object *rule_obj,
             const char* css_key, const char* css_value)
{
  int map_idx = -1;
  for (int i = 0; k_css_prop_map[i].css; i++) {
    if (!strcasecmp(css_key, k_css_prop_map[i].css)) {
      map_idx = i;
      break;
    }
  }
  if (map_idx < 0) return; // unsupported property

  // Expand font shorthand: "font: 14px/1.5 sans-serif" → font-size, line-height, font-family
  if (!strcasecmp(css_key, "font")) {
    char ek[4][CSS_MAX_PROPNAME] = {{0}};
    char ev[4][CSS_MAX_VALLEN] = {{0}};
    int n = css_expand_font_shorthand(css_value, ek, ev, 4);
    for (int i = 0; i < n; i++) {
      css_apply_decl_to_rule(rule_obj, ek[i], ev[i]);
    }
    return;
  }

  char theme_value[CSS_MAX_VALLEN] = {0};
  char resolved_value[CSS_MAX_VALLEN] = {0};
  char normalized_value[CSS_MAX_VALLEN] = {0};
  char url_value[CSS_MAX_VALLEN] = {0};
  css_value = css_resolve_theme_value(css_value, theme_value, sizeof(theme_value));
  /* Strip url("...") / url('...') / url(...) → bare path */
  if (!strncasecmp(css_value, "url(", 4)) {
    const char *inner = css_value + 4;
    const char *end = css_value + strlen(css_value);
    while (end > inner && isspace((unsigned char)end[-1])) end--;
    if (end > inner && end[-1] == ')') end--;
    while (inner < end && isspace((unsigned char)*inner)) inner++;
    if (inner < end && (*inner == '"' || *inner == '\'')) {
      char q = *inner++;
      const char *q_end = end;
      while (q_end > inner && q_end[-1] != q) q_end--;
      if (q_end > inner) end = q_end - 1;
    }
    int len = (int)(end - inner);
    if (len >= (int)sizeof(url_value)) len = (int)sizeof(url_value) - 1;
    memcpy(url_value, inner, len);
    url_value[len] = '\0';
    css_value = url_value;
  }
  const char *css_value_before_normalize = css_value;
  css_value = css_normalize_decl_value(css_key, css_value,
                     normalized_value,
                     sizeof(normalized_value));
  if (!strcasecmp(css_key, "font-family")) {
    css_value = css_resolve_font_family(css_value, resolved_value, sizeof(resolved_value));
  }

  if (!strcasecmp(css_key, "margin") &&
    (strstr(css_value_before_normalize, "auto") || strstr(css_value, "nan"))) {
    char part[4][CSS_MAX_VALLEN] = {{0}};
    char left[CSS_MAX_VALLEN] = {0};
    char top[CSS_MAX_VALLEN] = {0};
    char right[CSS_MAX_VALLEN] = {0};
    char bottom[CSS_MAX_VALLEN] = {0};
    int count = 0;
    const char *p = css_value_before_normalize;

    while (*p && count < 4) {
      while (*p && isspace((unsigned char)*p)) p++;
      const char *start = p;
      while (*p && !isspace((unsigned char)*p)) p++;
      if (start < p) {
        copy_trim(part[count], start, p, (int)sizeof(part[count]));
        if (!strcasecmp(part[count], "auto")) {
          strcpy(part[count], "nan");
        }
        count++;
      }
    }

    if (count == 1) {
      strcpy(top, part[0]);
      strcpy(right, part[0]);
      strcpy(bottom, part[0]);
      strcpy(left, part[0]);
    } else if (count == 2) {
      strcpy(top, part[0]);
      strcpy(right, part[1]);
      strcpy(bottom, part[0]);
      strcpy(left, part[1]);
    } else if (count == 3) {
      strcpy(top, part[0]);
      strcpy(right, part[1]);
      strcpy(bottom, part[2]);
      strcpy(left, part[1]);
    } else if (count == 4) {
      strcpy(top, part[0]);
      strcpy(right, part[1]);
      strcpy(bottom, part[2]);
      strcpy(left, part[3]);
    }

    if (count >= 1 && count <= 4) {
      css_apply_orca_value_to_rule(rule_obj, ID_Node_MarginLeft, "Node.MarginLeft", css_key, left);
      css_apply_orca_value_to_rule(rule_obj, ID_Node_MarginTop, "Node.MarginTop", css_key, top);
      css_apply_orca_value_to_rule(rule_obj, ID_Node_MarginRight, "Node.MarginRight", css_key, right);
      css_apply_orca_value_to_rule(rule_obj, ID_Node_MarginBottom, "Node.MarginBottom", css_key, bottom);
      return;
    }
  }

  for (int i = 0; k_css_prop_map[i].css; i++) {
    if (!strcasecmp(css_key, k_css_prop_map[i].css)) {
      css_apply_orca_value_to_rule(rule_obj,
                     k_css_prop_map[i].id,
                     k_css_prop_map[i].orca,
                     css_key,
                     css_value);
    }
  }
}

static char*
fs_load_text_file(const char* path)
{
  struct file* fp = FS_LoadFile(path);
  if (!fp) return NULL;

  char* buf = (char*)malloc(fp->size + 1);
  if (buf) {
    memcpy(buf, fp->data, fp->size);
    buf[fp->size] = '\0';
  }
  FS_FreeFile(fp);
  return buf;
}

static bool_t
css_append_text(char **out, size_t *len, size_t *cap, const char *text, size_t text_len)
{
  if (!out || !len || !cap || !text) return FALSE;

  if (*len + text_len + 1 > *cap) {
    size_t next_cap = *cap ? *cap : 1024;
    while (*len + text_len + 1 > next_cap) {
      next_cap *= 2;
    }
    char *next = (char*)realloc(*out, next_cap);
    if (!next) return FALSE;
    *out = next;
    *cap = next_cap;
  }

  memcpy(*out + *len, text, text_len);
  *len += text_len;
  (*out)[*len] = '\0';
  return TRUE;
}

static bool_t
css_copy_import_path(char *dst, size_t dst_size, const char *start, const char *end)
{
  if (!dst || dst_size == 0) return FALSE;
  while (start < end && isspace((unsigned char)*start)) start++;
  while (end > start && isspace((unsigned char)end[-1])) end--;
  if (end - start >= 2 &&
    ((*start == '"' && end[-1] == '"') || (*start == '\'' && end[-1] == '\''))) {
    start++;
    end--;
    while (start < end && isspace((unsigned char)*start)) start++;
    while (end > start && isspace((unsigned char)end[-1])) end--;
  }
  copy_trim(dst, start, end, (int)dst_size);
  return dst[0] != '\0';
}

static bool_t
css_parse_import_path(const char *start, const char *end, char *out, size_t out_size)
{
  while (start < end && isspace((unsigned char)*start)) start++;
  if (start >= end) return FALSE;

  if (end - start >= 4 && !strncasecmp(start, "url(", 4)) {
    const char *path_start = start + 4;
    const char *path_end = end;
    while (path_end > path_start && isspace((unsigned char)path_end[-1])) path_end--;
    if (path_end > path_start && path_end[-1] == ')') {
      path_end--;
    }
    return css_copy_import_path(out, out_size, path_start, path_end);
  }

  if (*start == '"' || *start == '\'') {
    char quote = *start++;
    const char *path_end = start;
    while (path_end < end && *path_end != quote) path_end++;
    return css_copy_import_path(out, out_size, start, path_end);
  }

  const char *path_end = start;
  while (path_end < end && !isspace((unsigned char)*path_end)) path_end++;
  return css_copy_import_path(out, out_size, start, path_end);
}

static void
css_resolve_import_path(char *out, size_t out_size, const char *base_path, const char *import_path)
{
  if (!out || out_size == 0) return;
  out[0] = '\0';
  if (!import_path || !import_path[0]) return;

  if (import_path[0] == '/' || strchr(import_path, ':')) {
    snprintf(out, out_size, "%s", import_path);
    return;
  }

  const char *slash = base_path ? strrchr(base_path, '/') : NULL;
  if (slash) {
    size_t base_len = (size_t)(slash - base_path + 1);
    snprintf(out, out_size, "%.*s%s", (int)base_len, base_path, import_path);
  } else {
    snprintf(out, out_size, "%s", import_path);
  }
}

static char*
css_load_text_file_with_imports(const char* path, int depth);

static char*
css_expand_imports(const char *css_text, const char *base_path, int depth)
{
  const char *p = css_text;
  char *out = NULL;
  size_t len = 0;
  size_t cap = 0;

  while (*p) {
    const char *at = strstr(p, "@import");
    if (!at) {
      if (!css_append_text(&out, &len, &cap, p, strlen(p))) {
        free(out);
        return NULL;
      }
      break;
    }

    const char *after = at + 7;
    if (*after && !isspace((unsigned char)*after)) {
      if (!css_append_text(&out, &len, &cap, p, (size_t)(at - p + 1))) {
        free(out);
        return NULL;
      }
      p = at + 1;
      continue;
    }

    if (!css_append_text(&out, &len, &cap, p, (size_t)(at - p))) {
      free(out);
      return NULL;
    }

    const char *semi = strchr(after, ';');
    if (!semi) {
      if (!css_append_text(&out, &len, &cap, at, strlen(at))) {
        free(out);
        return NULL;
      }
      break;
    }

    path_t import_path = {0};
    path_t resolved_path = {0};
    if (css_parse_import_path(after, semi, import_path, sizeof(import_path))) {
      css_resolve_import_path(resolved_path, sizeof(resolved_path), base_path, import_path);
      char *imported = css_load_text_file_with_imports(resolved_path, depth + 1);
      if (imported) {
        if (!css_append_text(&out, &len, &cap, imported, strlen(imported)) ||
          !css_append_text(&out, &len, &cap, "\n", 1)) {
          free(imported);
          free(out);
          return NULL;
        }
        free(imported);
      } else {
        Con_Printf("css_parser: can't import '%s' from '%s'", import_path, base_path ? base_path : "");
      }
    }

    p = semi + 1;
  }

  if (!out) {
    out = (char*)malloc(1);
    if (out) out[0] = '\0';
  }
  return out;
}

static char*
css_load_text_file_with_imports(const char* path, int depth)
{
  if (depth > CSS_MAX_IMPORT_DEPTH) {
    Con_Printf("css_parser: @import depth exceeded while loading '%s'", path ? path : "");
    return NULL;
  }

  char* css_text = fs_load_text_file(path);
  if (!css_text) return NULL;

  char* clean = css_strip_comments(css_text);
  free(css_text);
  if (!clean) return NULL;

  char* expanded = css_expand_imports(clean, path, depth);
  free(clean);
  return expanded;
}

// ---------------------------------------------------------------------------
// Public: parse a CSS string and return a new StyleSheet object
// ---------------------------------------------------------------------------

struct Object *UIKit_LoadObjectFromCssString(const char* css_text)
{
  if (!css_text) return NULL;

  // 1. Strip comments
  char* clean = css_strip_comments(css_text);
  if (!clean) return NULL;

  // 2. Parse into in-memory representation
  css_doc_t* doc = (css_doc_t*)calloc(1, sizeof(css_doc_t));
  if (!doc) { free(clean); return NULL; }
  css_parse_text(doc, clean);
  free(clean);

  // 3. Resolve @apply
  css_resolve_apply(doc);

  // 4. Build StyleSheet object
  struct Object *sheet = OBJ_Create(ID_StyleSheet);
  if (!sheet) { free(doc); return NULL; }

  for (int i = 0; i < doc->nrules; i++) {
    css_rule_t* rule = &doc->rules[i];
    if (!rule->sel[0]) continue;

    char class_name[CSS_MAX_SELLEN] = {0};
    char pseudo[CSS_MAX_SELLEN]   = {0};
    css_split_selector(rule->sel, class_name, sizeof(class_name),
               pseudo, sizeof(pseudo));
    if (!class_name[0]) continue;

    struct Object *rule_obj = OBJ_Create(ID_StyleRule);
    if (!rule_obj) continue;

    // Set ClassName and PseudoClass
    const char* cn = class_name;
    const char* pc = pseudo;
    PROP_SetValue(OBJ_FindLongProperty(rule_obj, ID_StyleRule_ClassName), &cn);
    PROP_SetValue(OBJ_FindLongProperty(rule_obj, ID_StyleRule_PseudoClass), &pc);

    // Apply declarations
    for (int di = 0; di < rule->ndecls; di++)
      css_apply_decl_to_rule(rule_obj,
                   rule->decls[di].key,
                   rule->decls[di].val);

    OBJ_SendMessageW(rule_obj, ID_Object_Start, 0, NULL);
    OBJ_AddChild(sheet, rule_obj);
  }

  OBJ_SendMessageW(sheet, ID_Object_Start, 0, NULL);
  free(doc);
  return sheet;
}

struct Object *UIKit_LoadObjectFromCss(const char* path)
{
  if (!path) return NULL;
  char* css_text = css_load_text_file_with_imports(path, 0);
  if (!css_text) {
    Con_Error("UIKit_LoadObjectFromCss: can't load '%s'", path);
    return NULL;
  }

  struct Object *sheet = UIKit_LoadObjectFromCssString(css_text);
  free(css_text);
  return sheet;
}

