// Pure-C CSS parser for UIKit.
//
// Parses a CSS text string into an ORCA StyleSheet object containing
// StyleRule children.  Each StyleRule carries:
//   - ClassName   : base selector without leading dot (e.g. "button")
//   - PseudoClass : colon-separated pseudo-states  (e.g. "hover:focus")
//   - ORCA property overrides stored as C properties
//
// Supported CSS subset:
//   - Block comments /* ... */
//   - Simple class selectors: .foo { ... }
//   - Comma-separated selectors: .foo, .bar { ... }
//   - Pseudo-classes: .foo:hover { ... }
//   - @apply directives (merges another selector's declarations)
//   - Standard property declarations: key: value;

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include <plugins/UIKit/UIKit.h>
#include <include/api.h>
#include <source/core/property/property_internal.h>
#include <source/core/core_properties.h>
#include <source/filesystem/filesystem.h>

// parse_property is ORCA_API but not declared in orca.h; forward-declare it.
extern int parse_property(const char* str, struct PropertyType const* prop,
                          void* valueptr);

// ---------------------------------------------------------------------------
// CSS property name → ORCA full property name mapping
// ---------------------------------------------------------------------------
static const struct { const char* css; const char* orca; }
k_css_prop_map[] = {
    { "background-color",  "Node2D.BackgroundColor"        },
    { "color",             "Node2D.ForegroundColor"        },
    { "opacity",           "Node.Opacity"                  },
    { "width",             "Node.Width"                    },
    { "height",            "Node.Height"                   },
    { "min-width",         "Node.MinWidth"                 },
    { "min-height",        "Node.MinHeight"                },
    { "margin",            "Node.Margin"                   },
    { "margin-top",        "Node.MarginTop"                },
    { "margin-right",      "Node.MarginRight"              },
    { "margin-bottom",     "Node.MarginBottom"             },
    { "margin-left",       "Node.MarginLeft"               },
    { "padding",           "Node.Padding"                  },
    { "padding-top",       "Node.PaddingTop"               },
    { "padding-right",     "Node.PaddingRight"             },
    { "padding-bottom",    "Node.PaddingBottom"            },
    { "padding-left",      "Node.PaddingLeft"              },
    { "border",            "Node.Border"                   },
    { "border-color",      "Node.BorderColor"              },
    { "border-width",      "Node.BorderWidth"              },
    { "font-size",         "TextRun.FontSize"              },
    { "font-family",       "TextRun.FontFamily"            },
    { "line-height",       "TextRun.LineHeight"            },
    { "letter-spacing",    "TextRun.LetterSpacing"         },
    { "word-wrap",         "TextBlockConcept.WordWrap"     },
    { "text-overflow",     "TextBlockConcept.TextOverflow" },
    { NULL, NULL }
};

static const char*
css_lookup_orca_property(const char* css_name)
{
    for (int i = 0; k_css_prop_map[i].css; i++) {
        if (!strcasecmp(css_name, k_css_prop_map[i].css))
            return k_css_prop_map[i].orca;
    }
    return NULL;
}

// ---------------------------------------------------------------------------
// Parsed CSS in-memory representation
// ---------------------------------------------------------------------------
#define CSS_MAX_RULES    512
#define CSS_MAX_PROPS    64
#define CSS_MAX_APPLY    8
#define CSS_MAX_SELLEN   256
#define CSS_MAX_PROPNAME 64
#define CSS_MAX_VALLEN   256

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

// selector: ".button:hover" → class_out="button", pseudo_out="hover"
// selector: ".card"         → class_out="card",   pseudo_out=""
// selector: "card"          → class_out="card",   pseudo_out=""
static void
css_split_selector(const char* selector,
                   char* class_out, int class_max,
                   char* pseudo_out, int pseudo_max)
{
    // skip optional leading dot
    const char* base = (selector[0] == '.') ? selector + 1 : selector;

    const char* colon = strchr(base, ':');
    if (colon) {
        int len = (int)(colon - base);
        if (len >= class_max) len = class_max - 1;
        memcpy(class_out, base, len);
        class_out[len] = '\0';
        snprintf(pseudo_out, pseudo_max, "%s", colon + 1);
    } else {
        snprintf(class_out,  class_max,  "%s", base);
        pseudo_out[0] = '\0';
    }
}

// ---------------------------------------------------------------------------
// StyleRule property assignment
// ---------------------------------------------------------------------------

// Apply one CSS declaration (key/value) to an ORCA StyleRule object.
static void
css_apply_decl_to_rule(struct Object *rule_obj,
                       const char* css_key, const char* css_value)
{
    const char* orca_name = css_lookup_orca_property(css_key);
    if (!orca_name) return; // unsupported property

    struct Property *prop = NULL;
    if (!SUCCEEDED(OBJ_FindShortProperty(rule_obj, orca_name, &prop))) return;
    if (!prop || !PROP_GetDesc(prop)) return;

    char buf[MAX_PROPERTY_STRING] = {0};
    if (parse_property(css_value, PROP_GetDesc(prop), buf))
        PROP_SetValue(prop, buf);
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

// ---------------------------------------------------------------------------
// Public: parse a CSS string and return a new StyleSheet object
// ---------------------------------------------------------------------------

struct Object *FS_LoadObjectFromCssString(const char* css_text)
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
        OBJ_AddChild(sheet, rule_obj, FALSE);
    }

    OBJ_SendMessageW(sheet, ID_Object_Start, 0, NULL);
    free(doc);
    return sheet;
}

struct Object *FS_LoadObjectFromCss(const char* path)
{
    char* css_text = fs_load_text_file(path);
    if (!css_text) {
        Con_Error("FS_LoadObjectFromCss: can't load '%s'", path);
        return NULL;
    }

    struct Object *sheet = FS_LoadObjectFromCssString(css_text);
    free(css_text);
    return sheet;
}

