#include <string.h>
#include "cg_api.h"
#include "outbuf.h"

<<<<<<< HEAD
typedef struct { char *s; size_t n, cap; } ob;

static int ob_printf(ob *b, char const *fmt, ...) {
    va_list ap, cp;
    int need;
    va_start(ap, fmt);
    va_copy(cp, ap);
    need = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (need < 0 || b->n + (size_t)need + 1u > b->cap) {
        size_t nc = b->cap ? b->cap * 2u : 4096u;
        char *ns;
        while (nc < b->n + (size_t)need + 1u) nc *= 2u;
        ns = (char *)realloc(b->s, nc);
        if (!ns) { va_end(cp); return -1; }
        b->s = ns; b->cap = nc;
    }
    vsnprintf(b->s + b->n, b->cap - b->n, fmt, cp);
    va_end(cp);
    b->n += (size_t)need;
    return 0;
}

/* Return the portion of s after the last '.', or s itself if no dot. */
static char const *after_dot(char const *s) {
    char const *p = s ? strrchr(s, '.') : NULL;
    return p ? p + 1 : (s ? s : "");
}

/* Find a class node by name (returns first match). */
static cg_node const *find_class(cg_model const *m, char const *name) {
    size_t i;
    if (!name || !name[0]) return NULL;
    for (i = 0; i < m->node_count; ++i)
        if (m->nodes[i].kind == CG_KIND_CLASS && !strcmp(m->nodes[i].name, name))
            return &m->nodes[i];
    return NULL;
}

/* Emit the C type string with pointer/array decoration. */
static int emit_type(ob *b, cg_node const *p) {
    if ((p->flags & CG_FLAG_POINTER) && (p->flags & CG_FLAG_ARRAY))
        return ob_printf(b, "`%s*[]`", p->type);
    if (p->flags & CG_FLAG_POINTER)
        return ob_printf(b, "`%s*`", p->type);
    if (p->flags & CG_FLAG_ARRAY)
        return ob_printf(b, "`%s[]`", p->type);
    return ob_printf(b, "`%s`", p->type);
}

/* Emit a Markdown properties table for all public properties owned by cls. */
static int emit_props_for_class(ob *b, cg_model const *m, cg_node const *cls,
                                char const *section_name) {
    int has = 0;
    cg_node const *p;
    for (p = cg_first(m, cls->id, CG_KIND_PROPERTY); p;
         p = cg_next(m, p, cls->id, CG_KIND_PROPERTY)) {
        if (!(p->flags & CG_FLAG_PRIVATE)) { has = 1; break; }
    }
    if (!has) return 0;
    if (ob_printf(b, "\n### From %s\n\n", section_name) < 0) return -1;
    if (ob_printf(b, "| Property | Type | Default | Description |\n"
                     "|----------|------|---------|-------------|\n") < 0) return -1;
    cg_foreach(m, cls->id, CG_KIND_PROPERTY, pr) {
        if (pr->flags & CG_FLAG_PRIVATE) continue;
        if (ob_printf(b, "| `%s` | ", pr->name) < 0) return -1;
        if (emit_type(b, pr) < 0) return -1;
        if (ob_printf(b, " | %s | %s |\n",
                (pr->extra && pr->extra[0]) ? pr->extra : "\xe2\x80\x94" /* — */,
                pr->doc ? pr->doc : "") < 0) return -1;
    }
    return 0;
}

/* Walk a comma-separated parent list and emit one properties section per
 * parent that is found in the model (one level deep only).
 * buf is at least 1024 bytes; parent lists in practice are far shorter. */
static int emit_inherited_props(ob *b, cg_model const *m, char const *parent_list) {
    char buf[1024];
    char *tok;
    if (!parent_list || !parent_list[0]) return 0;
    snprintf(buf, sizeof(buf), "%s", parent_list);
    for (tok = strtok(buf, ","); tok; tok = strtok(NULL, ",")) {
        cg_node const *parent;
        while (*tok == ' ' || *tok == '\t') ++tok;
        parent = find_class(m, tok);
        if (parent)
            if (emit_props_for_class(b, m, parent, parent->name) < 0) return -1;
    }
    return 0;
}

/* Return true if any public property exists on cls. */
static int has_any_props(cg_model const *m, cg_node const *cls) {
    cg_foreach(m, cls->id, CG_KIND_PROPERTY, p) {
        if (!(p->flags & CG_FLAG_PRIVATE)) return 1;
    }
    return 0;
}

/* Return true if any immediate parent (in the model) has public properties. */
static int parents_have_props(cg_model const *m, char const *parent_list) {
    char buf[1024];
    char *tok;
    if (!parent_list || !parent_list[0]) return 0;
    snprintf(buf, sizeof(buf), "%s", parent_list);
    for (tok = strtok(buf, ","); tok; tok = strtok(NULL, ",")) {
        cg_node const *parent;
        while (*tok == ' ' || *tok == '\t') ++tok;
        parent = find_class(m, tok);
        if (parent && has_any_props(m, parent)) return 1;
    }
    return 0;
}

/* Emit a single-parent inheritance chain as "Root -> ... -> ClassName".
 * Stops after MAX_CHAIN ancestors or when a multi-parent ancestor is encountered. */
#define MAX_CHAIN 16
static int emit_chain(ob *b, cg_model const *m, cg_node const *cls) {
    char const *chain[MAX_CHAIN];
    int n = 0;
    cg_node const *cur = cls;
    chain[n++] = cls->name;
    while (cur->type && cur->type[0] && n < MAX_CHAIN) {
        if (strchr(cur->type, ',')) break; /* multi-parent: stop */
        cur = find_class(m, cur->type);
        if (!cur) break;
        chain[n++] = cur->name;
    }
    /* Print reversed */
    {
        int i;
        for (i = n - 1; i >= 0; --i) {
            if (i < n - 1 && ob_printf(b, " \xe2\x86\x92 ") < 0) return -1; /* → */
            if (ob_printf(b, "%s", chain[i]) < 0) return -1;
        }
    }
    return 0;
}

/* Emit the ## Inheritance section. */
static int emit_inheritance(ob *b, cg_model const *m, cg_node const *cls) {
    if (!cls->type || !cls->type[0]) return 0;
    if (ob_printf(b, "\n## Inheritance\n\n") < 0) return -1;
    if (strchr(cls->type, ',')) {
        /* Multiple parents: list them */
        char buf[1024];
        char *tok;
        int first = 1;
        snprintf(buf, sizeof(buf), "%s", cls->type);
        for (tok = strtok(buf, ","); tok; tok = strtok(NULL, ",")) {
            while (*tok == ' ' || *tok == '\t') ++tok;
            if (!first && ob_printf(b, ", ") < 0) return -1;
            if (ob_printf(b, "`%s`", tok) < 0) return -1;
            first = 0;
        }
        if (ob_printf(b, " \xe2\x86\x92 `%s`\n", cls->name) < 0) return -1; /* → */
    } else {
        if (emit_chain(b, m, cls) < 0) return -1;
        if (ob_printf(b, "\n") < 0) return -1;
    }
    return 0;
}

/* Emit the ## Handled Messages section. */
static int emit_handles(ob *b, cg_model const *m, cg_node const *cls) {
    int has = 0;
    cg_foreach(m, cls->id, CG_KIND_HANDLE, h) { (void)h; has = 1; break; }
    if (!has) return 0;
    if (ob_printf(b, "\n## Handled Messages\n\n"
                     "| Message | Handler |\n"
                     "|---------|---------|\n") < 0) return -1;
    cg_foreach(m, cls->id, CG_KIND_HANDLE, hdl) {
        if (ob_printf(b, "| `%s` | `%s_%s` |\n",
                hdl->name, cls->name, after_dot(hdl->name)) < 0) return -1;
    }
    return 0;
}

/* Emit the full documentation block for one class. */
static int emit_class(ob *b, cg_model const *m, cg_node const *cls) {
    int own_props, inh_props;

    /* Heading */
    if (ob_printf(b, "# %s\n\n", cls->name) < 0) return -1;

    /* Definition */
    if (ob_printf(b, "**Definition:** `%s`\n", m->xml_path) < 0) return -1;

    /* Namespace */
    if (cls->aux2 && cls->aux2[0])
        if (ob_printf(b, "**Namespace:** `%s`\n", cls->aux2) < 0) return -1;

    /* Summary */
    if (cls->doc && cls->doc[0])
        if (ob_printf(b, "\n## Summary\n\n%s\n", cls->doc) < 0) return -1;

    /* Inheritance */
    if (emit_inheritance(b, m, cls) < 0) return -1;

    /* Properties */
    own_props = has_any_props(m, cls);
    inh_props = parents_have_props(m, cls->type);
    if (own_props || inh_props) {
        if (ob_printf(b, "\n## Properties\n") < 0) return -1;
        if (own_props && emit_props_for_class(b, m, cls, cls->name) < 0) return -1;
        if (inh_props && emit_inherited_props(b, m, cls->type) < 0) return -1;
    }

    /* Handled messages */
    if (emit_handles(b, m, cls) < 0) return -1;

    /* Implementation details */
    if (cls->aux && cls->aux[0])
        if (ob_printf(b, "\n## Implementation Details\n\n%s\n", cls->aux) < 0) return -1;

    return 0;
}

static int emit_docs(cg_host_v1 const *host, cg_model const *model, char const *output) {
    ob b = {0};
    size_t i;
    uint32_t n_class = 0;
=======
/* Emit a single class documentation block to output buffer. */
static int emit_class_docs(cg_host_v1 const *host, cgen_output const *out,
                           cg_model const *model, cg_node const *cls) {
    outbuf b = {0};
    cg_node const *p;
    char const *parent = NULL;
    int has_details = 0, has_properties = 0, has_messages = 0;
    int n_prop = 0, n_msg = 0, n_arg = 0, n_ret = 0;
    int is_view = 0;

    /* Determine parent from type field. */
    if (cls->type && cls->type[0]) {
        parent = cls->type;
        /* Check if this is a View subclass (has children). */
        is_view = (strstr(parent, "View") != NULL);
    }

    /* Count children of this class. */
    for (p = cg_first(model, cls->id, 0); p; p = cg_next(model, p, cls->id, 0)) {
        if (p->kind == CG_KIND_MESSAGE) { n_msg++; has_messages = 1; }
        if (p->kind == CG_KIND_PROPERTY) { n_prop++; has_properties = 1; }
    }
    if (has_messages) {
        for (p = cg_first(model, cls->id, 0); p; p = cg_next(model, p, cls->id, 0))
            if (p->kind == CG_KIND_ARG) n_arg++;
        for (p = cg_first(model, cls->id, 0); p; p = cg_next(model, p, cls->id, 0))
            if (p->kind == CG_KIND_RETURNS) n_ret++;
    }

    /* Build the class header with badges. */
    if (ob_printf(&b,
            "## %s %s\n\n"
            "%s%s%s%s%s\n",
            cls->name,
            parent ? " extends %s" : "",
            parent ? parent : "",
            parent ? " " : "",
            cls->flags & CG_FLAG_SEALED ? "**sealed** " : "",
            cls->flags & CG_FLAG_STATIC ? "**static** " : "",
            cls->doc ? "" : "") < 0) {
        free(b.s);
        return -1;
    }

    /* Emit summary if present. */
    if (cls->doc) {
        if (ob_printf(&b, "\n%s\n", cls->doc) < 0) {
            free(b.s);
            return -1;
        }
    }

    /* Emit <details> sections for properties, messages, etc. */
    if (has_properties) {
        if (ob_printf(&b,
                "\n<details open>\n<summary>Properties</summary>\n\n"
                "| Name | Type | Default | Attributes |\n"
                "|------|------|---------|------------|\n") < 0) {
            free(b.s);
            return -1;
        }
        for (p = cg_first(model, cls->id, 0); p; p = cg_next(model, p, cls->id, 0)) {
            if (p->kind != CG_KIND_PROPERTY) continue;
            char const *attrs = "";
            if (p->flags & CG_FLAG_READONLY)  attrs = "readonly";
            if (p->flags & CG_FLAG_PRIVATE)   attrs = "private";
            if (p->flags & CG_FLAG_INHERITED) attrs = "inherited";
            if (p->flags & CG_FLAG_ARRAY)     attrs = "array";
            if (p->flags & CG_FLAG_POINTER)   attrs = "pointer";

            /* Escape pipe characters in doc. */
            char const *doc_esc = p->doc ? p->doc : "";

            if (ob_printf(&b, "| `%s` | %s | %s | %s |%s\n",
                    p->name, p->type,
                    p->extra ? p->extra : "(none)",
                    attrs ? attrs : "—",
                    doc_esc[0] ? " " : "",
                    doc_esc) < 0) {
                free(b.s);
                return -1;
            }
        }
        if (ob_printf(&b, "\n</details>\n") < 0) {
            free(b.s);
            return -1;
        }
    }

    if (has_messages) {
        if (ob_printf(&b,
                "\n<details open>\n<summary>Messages</summary>\n\n"
                "| Name | Routing | Args | Returns | Same-As |\n"
                "|------|---------|------|-----------|---------|\n") < 0) {
            free(b.s);
            return -1;
        }
        for (p = cg_first(model, cls->id, 0); p; p = cg_next(model, p, cls->id, 0)) {
            if (p->kind != CG_KIND_MESSAGE) continue;
            char const *same_as = p->extra && p->extra[0] ? p->extra : "—";
            char const *routing = p->type && p->type[0] ? p->type : "—";
            char const *doc_esc = p->doc ? p->doc : "";

            if (ob_printf(&b, "| `%s` | %s | %u | %u | %s |%s\n",
                    p->name, routing, n_arg, n_ret, same_as,
                    doc_esc[0] ? " " : "", doc_esc) < 0) {
                free(b.s);
                return -1;
            }
        }
        if (ob_printf(&b, "\n</details>\n") < 0) {
            free(b.s);
            return -1;
        }
    }

    /* Emit child classes (View children) if any. */
    if (is_view) {
        int n_children = 0;
        cg_node const *child;
        for (child = cg_first(model, cls->id, 0); child; child = cg_next(model, child, cls->id, 0))
            if (child->kind == CG_KIND_CLASS) n_children++;

        if (n_children > 0) {
            if (ob_printf(&b,
                    "\n<details open>\n<summary>Child Views (%u)</summary>\n\n",
                    n_children) < 0) {
                free(b.s);
                return -1;
            }
            for (child = cg_first(model, cls->id, 0); child; child = cg_next(model, child, cls->id, 0))
                if (child->kind == CG_KIND_CLASS)
                    if (ob_printf(&b, "- %s\n", child->name) < 0) {
                        free(b.s);
                        return -1;
                    }
            if (ob_printf(&b, "\n</details>\n") < 0) {
                free(b.s);
                return -1;
            }
        }
    }

    /* Emit the class documentation. */
    if (host->write_file(out->path, b.s, b.n) < 0) {
        host->logf("docs: failed write %s", out->path);
        free(b.s);
        return -1;
    }
    free(b.s);
    return 0;
}

static int emit_docs(cg_host_v1 const *host, cg_model const *model, char const *output_dir) {
    size_t i;
    uint32_t n_enum = 0, n_if = 0, n_struct = 0, n_class = 0;
    outbuf b = {0};
    cgen_output *out;
    int errors = 0;

    /* Count nodes by kind. */
    for (i = 0; i < model->node_count; ++i) {
        n_enum += model->nodes[i].kind == CG_KIND_ENUM;
        n_if += model->nodes[i].kind == CG_KIND_INTERFACE;
        n_struct += model->nodes[i].kind == CG_KIND_STRUCT;
        n_class += model->nodes[i].kind == CG_KIND_CLASS;
    }

    /* Build module overview. */
    if (ob_printf(&b,
            "# %s API (cgen preview)\n\n"
            "- source: `%s`\n"
            "- enums: %u\n"
            "- interfaces: %u\n"
            "- structs: %u\n"
            "- classes: %u\n\n"
            "## Classes\n",
            model->module_name, model->xml_path, n_enum, n_if, n_struct, n_class) < 0) {
        free(b.s);
        return -1;
    }
>>>>>>> origin/feature/list-box

    /* List all classes in the overview. */
    for (i = 0; i < model->node_count; ++i)
<<<<<<< HEAD
        if (model->nodes[i].kind == CG_KIND_CLASS) ++n_class;

    if (ob_printf(&b, "# %s\n\n"
                      "- **source:** `%s`\n"
                      "- **classes:** %u\n\n",
                  model->module_name, model->xml_path, n_class) < 0) {
        free(b.s); return -1;
    }

    cg_foreach(model, 0, CG_KIND_CLASS, cls) {
        if (ob_printf(&b, "---\n\n") < 0) { free(b.s); return -1; }
        if (emit_class(&b, model, cls) < 0) { free(b.s); return -1; }
        if (ob_printf(&b, "\n") < 0) { free(b.s); return -1; }
    }
=======
        if (model->nodes[i].kind == CG_KIND_CLASS)
            if (ob_printf(&b, "- [%s](classes/%s.md)\n",
                    model->nodes[i].name, model->nodes[i].name) < 0) {
                free(b.s);
                return -1;
            }
>>>>>>> origin/feature/list-box

    /* Write module overview. */
    if (host->write_file(output_dir, b.s, b.n) < 0) {
        host->logf("docs: failed write %s", output_dir);
        free(b.s);
        return -1;
    }
    free(b.s);

    /* Emit individual class documentation files. */
    for (i = 0; i < model->node_count; ++i) {
        if (model->nodes[i].kind != CG_KIND_CLASS) continue;

        /* Build output path for this class. */
        char path[512];
        size_t base_len = strlen(output_dir);
        if (base_len + 1 + strlen(model->nodes[i].name) + 5 >= sizeof(path)) {
            host->logf("docs: path too long for %s", model->nodes[i].name);
            continue;
        }
        memcpy(path, output_dir, base_len);
        path[base_len] = '/';
        strcpy(path + base_len + 1, "classes/");
        strcpy(path + base_len + 9, model->nodes[i].name);
        strcpy(path + base_len + 9 + strlen(model->nodes[i].name), ".md");

        /* Build output descriptor. */
        out = (cgen_output*)malloc(sizeof(cgen_output));
        if (!out) continue;
        out->path = path;

        if (emit_class_docs(host, out, model, &model->nodes[i]) < 0) {
            host->logf("docs: failed emitting %s", model->nodes[i].name);
            errors++;
        }
        free(out);
    }

    return errors ? -1 : 0;
}

static cg_plugin_v1 const plugin = { .abi = CG_API_VERSION, .name = "docs", .emit = emit_docs };

cg_plugin_v1 const *cg_plugin_init_v1(void) { return &plugin; }
