// Pure-C CSS parser for UIKit.
//
// Parses a CSS text string into an ORCA StyleSheet object containing
// StyleRule children.  Each StyleRule carries:
//   - ClassName   : selector without pseudo-state (e.g. ".button", "#Logo",
//                   "ImageView", "StackView > Label", or ".popup .panel")
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
#include <include/api.h>
#include <source/core/property/property_internal.h>
#include <core/core_properties.h>
#include <filesystem/filesystem.h>

// parse_property is ORCA_API but not declared in orca.h; forward-declare it.
extern int parse_property(const char* str, struct PropertyType const* prop,
                          void* valueptr);

static void
copy_trim(char* dst, const char* s, const char* e, int n);

#define CSS_MAX_RULES    512
#define CSS_MAX_PROPS    64
#define CSS_MAX_APPLY    8
#define CSS_MAX_IMPORT_DEPTH 16
#define CSS_MAX_SELLEN   256
#define CSS_MAX_PROPNAME 64
#define CSS_MAX_VALLEN   256

// ---------------------------------------------------------------------------
// CSS property name → ORCA full property name mapping
// ---------------------------------------------------------------------------
static const struct { const char* css; const char* orca; }
k_css_prop_map[] = {
    /* Node layout */
    { "background-color",  "Node2D.BackgroundColor"        },
    { "color",             "Node2D.ForegroundColor"        },
    { "opacity",           "Node.Opacity"                  },
    { "width",             "Node.Width"                    },
    { "height",            "Node.Height"                   },
    { "min-width",         "Node.MinWidth"                 },
    { "min-height",        "Node.MinHeight"                },
    { "horizontal-align",  "Node.HorizontalAlignment"      },
    { "horizontal-alignment", "Node.HorizontalAlignment"   },
    { "vertical-align",    "Node.VerticalAlignment"        },
    { "vertical-alignment", "Node.VerticalAlignment"       },
    { "align-self",        "Node.HorizontalAlignment"      },
    { "margin",            "Node.Margin"                   },
    { "margin-top",        "Node.MarginTop"                },
    { "margin-right",      "Node.MarginRight"              },
    { "margin-bottom",     "Node.MarginBottom"             },
    { "margin-left",       "Node.MarginLeft"               },
    { "margin-inline",     "Node.HorizontalMargin"         },
    { "margin-block",      "Node.VerticalMargin"           },
    { "padding",           "Node.Padding"                  },
    { "padding-top",       "Node.PaddingTop"               },
    { "padding-right",     "Node.PaddingRight"             },
    { "padding-bottom",    "Node.PaddingBottom"            },
    { "padding-left",      "Node.PaddingLeft"              },
    { "padding-inline",    "Node.HorizontalPadding"        },
    { "padding-block",     "Node.VerticalPadding"          },
    { "border",            "Node.Border"                   },
    { "border-color",      "Node.BorderColor"              },
    { "border-style",      "Node.BorderStyle"              },
    { "border-width",      "Node.BorderWidth"              },
    { "border-left-width", "Node.BorderWidthLeft"          },
    { "border-right-width", "Node.BorderWidthRight"        },
    { "border-top-width",  "Node.BorderWidthTop"           },
    { "border-bottom-width", "Node.BorderWidthBottom"      },
    { "border-radius",     "Node.BorderRadius"             },
    { "border-top-left-radius", "Node.BorderTopLeftRadius" },
    { "border-top-right-radius", "Node.BorderTopRightRadius" },
    { "border-bottom-right-radius", "Node.BorderBottomRightRadius" },
    { "border-bottom-left-radius", "Node.BorderBottomLeftRadius" },
    { "visibility",        "Node.Visible"                  },

    /* Node2D rendering */
    { "background",        "Node2D.Background"             },
    { "background-image",  "Node2D.BackgroundImage"        },
    { "foreground",        "Node2D.Foreground"             },
    { "foreground-color",  "Node2D.ForegroundColor"        },
    { "box-shadow",        "Node2D.BoxShadow"              },
    { "box-shadow-color",  "Node2D.BoxShadowColor"         },
    { "box-shadow-blur",   "Node2D.BoxShadowBlurRadius"    },
    { "box-shadow-blur-radius", "Node2D.BoxShadowBlurRadius" },
    { "box-shadow-spread", "Node2D.BoxShadowSpreadRadius"  },
    { "box-shadow-spread-radius", "Node2D.BoxShadowSpreadRadius" },
    { "overflow",          "Node2D.Overflow"               },
    { "overflow-x",        "Node2D.OverflowX"              },
    { "overflow-y",        "Node2D.OverflowY"              },
    { "ring",              "Node2D.Ring"                   },
    { "ring-color",        "Node2D.RingColor"              },
    { "ring-offset",       "Node2D.RingOffset"             },
    { "ring-width",        "Node2D.RingWidth"              },
    { "content-stretch",   "Node2D.ContentStretch"         },
    { "ignore-hit-test",   "Node2D.IgnoreHitTest"          },
    { "pointer-events",    "Node2D.IgnoreHitTest"          },
    { "size-to-content",   "Node2D.SizeToContent"          },
    { "snap-to-pixel",     "Node2D.SnapToPixel"            },

    /* StackView */
    { "align-items",       "StackView.AlignItems"           },
    { "justify-content",   "StackView.JustifyContent"      },
    { "flex-direction",    "StackView.Direction"           },
    { "direction",         "StackView.Direction"           },
    { "gap",               "StackView.Spacing"             },
    { "gap",               "Grid.Spacing"                  },
    { "spacing",           "StackView.Spacing"             },
    { "reversed",          "StackView.Reversed"            },

    /* Text */
    { "font",              "TextRun.Font"                  },
    { "font-size",         "TextRun.FontSize"              },
    { "font-family",       "TextRun.FontFamily"            },
    { "font-weight",       "TextRun.FontWeight"            },
    { "font-style",        "TextRun.FontStyle"             },
    { "line-height",       "TextRun.LineHeight"            },
    { "letter-spacing",    "TextRun.LetterSpacing"         },
    { "character-spacing", "TextRun.CharacterSpacing"      },
    { "fixed-character-width", "TextRun.FixedCharacterWidth" },
    { "text-decoration",   "TextRun.Underline"             },
    { "text-decoration-color", "TextRun.UnderlineColor"    },
    { "text-decoration-thickness", "TextRun.UnderlineWidth" },
    { "underline",         "TextRun.Underline"             },
    { "underline-color",   "TextRun.UnderlineColor"        },
    { "underline-width",   "TextRun.UnderlineWidth"        },
    { "underline-offset",  "TextRun.UnderlineOffset"       },
    { "word-wrap",         "TextBlockConcept.WordWrap"     },
    { "overflow-wrap",     "TextBlockConcept.WordWrap"     },
    { "text-wrap",         "TextBlockConcept.TextWrapping" },
    { "text-overflow",     "TextBlockConcept.TextOverflow" },
    { "text-align",        "TextBlockConcept.TextHorizontalAlignment" },
    { "text-horizontal-align", "TextBlockConcept.TextHorizontalAlignment" },
    { "text-horizontal-alignment", "TextBlockConcept.TextHorizontalAlignment" },
    { "text-vertical-align", "TextBlockConcept.TextVerticalAlignment" },
    { "text-vertical-alignment", "TextBlockConcept.TextVerticalAlignment" },
    { "placeholder-color", "TextBlockConcept.PlaceholderColor" },
    { NULL, NULL }
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

static const char*
css_normalize_decl_value(const char *css_key,
                         const char *css_value,
                         char *out,
                         size_t out_size)
{
    if (!strcasecmp(css_key, "margin") ||
        !strcasecmp(css_key, "padding") ||
        !strcasecmp(css_key, "border-width")) {
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
            return out;
        }

        if (count == 3) {
            // CSS: top horizontal bottom. ORCA: left top right bottom.
            snprintf(out, out_size, "%s %s %s %s",
                     values[1], values[0], values[1], values[2]);
            return out;
        }

        if (count == 4) {
            // CSS: top right bottom left. ORCA: left top right bottom.
            snprintf(out, out_size, "%s %s %s %s",
                     values[3], values[0], values[1], values[2]);
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

// ---------------------------------------------------------------------------
// Parsed CSS in-memory representation
// ---------------------------------------------------------------------------
typedef struct {
    char key[CSS_MAX_PROPNAME]; // CSS name, e.g. "opacity"
    char val[CSS_MAX_VALLEN];   // CSS value, e.g. "0.4"
} css_decl_t;

typedef struct {
    char       sel[CSS_MAX_SELLEN];                 // trimmed selector
    css_decl_t decls[CSS_MAX_PROPS];
    int        ndecls;
    char       apply[CSS_MAX_APPLY][CSS_MAX_SELLEN]; // @apply selectors
    int        napply;
} css_rule_t;

typedef struct {
    css_rule_t rules[CSS_MAX_RULES];
    int        nrules;
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
    char*        w = out;
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
        copy_trim(key, p, colon,     (int)sizeof(key));
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

// selector: ".button:hover"     → class_out=".button", pseudo_out="hover"
// selector: "#Logo"             → class_out="#Logo",   pseudo_out=""
// selector: "StackView > Label" → class_out="StackView > Label", pseudo_out=""
// selector: ".popup .panel"     → class_out=".popup .panel", pseudo_out=""
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
                             const char* orca_name,
                             const char* css_key,
                             const char* css_value)
{
    struct Property *prop = NULL;
    if (!SUCCEEDED(OBJ_FindShortProperty(rule_obj, orca_name, &prop))) {
        OBJ_SetShorthandValueFromString(rule_obj, orca_name, css_value);
        return;
    }
    if (!prop || !PROP_GetDesc(prop)) return;

    struct PropertyType const* desc = PROP_GetDesc(prop);
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
    bool_t has_mapping = FALSE;
    for (int i = 0; k_css_prop_map[i].css; i++) {
        if (!strcasecmp(css_key, k_css_prop_map[i].css)) {
            has_mapping = TRUE;
            break;
        }
    }
    if (!has_mapping) return; // unsupported property

    char theme_value[CSS_MAX_VALLEN] = {0};
    char resolved_value[CSS_MAX_VALLEN] = {0};
    char normalized_value[CSS_MAX_VALLEN] = {0};
    css_value = css_resolve_theme_value(css_value, theme_value, sizeof(theme_value));
    css_value = css_normalize_decl_value(css_key, css_value,
                                         normalized_value,
                                         sizeof(normalized_value));
    if (!strcasecmp(css_key, "font-family")) {
        css_value = css_resolve_font_family(css_value, resolved_value, sizeof(resolved_value));
    }

    for (int i = 0; k_css_prop_map[i].css; i++) {
        if (!strcasecmp(css_key, k_css_prop_map[i].css)) {
            css_apply_orca_value_to_rule(rule_obj,
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
        char pseudo[CSS_MAX_SELLEN]     = {0};
        css_split_selector(rule->sel, class_name, sizeof(class_name),
                           pseudo, sizeof(pseudo));
        if (!class_name[0]) continue;

        struct Object *rule_obj = OBJ_Create(ID_StyleRule);
        if (!rule_obj) continue;

        // Set ClassName and PseudoClass
        const char* cn = class_name;
        OBJ_SetPropertyValue(rule_obj, "ClassName",   &cn);
        const char* pc = pseudo;
        OBJ_SetPropertyValue(rule_obj, "PseudoClass", &pc);

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
