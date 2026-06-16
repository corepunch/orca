#include <string.h>
#include "cg_api.h"
#include "outbuf.h"

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
static int emit_type(outbuf *b, cg_node const *p) {
    if ((p->flags & CG_FLAG_POINTER) && (p->flags & CG_FLAG_ARRAY))
        return ob_printf(b, "`%s*[]`", p->type);
    if (p->flags & CG_FLAG_POINTER)
        return ob_printf(b, "`%s*`", p->type);
    if (p->flags & CG_FLAG_ARRAY)
        return ob_printf(b, "`%s[]`", p->type);
    return ob_printf(b, "`%s`", p->type);
}

/* Return true if any public property exists on cls. */
static int has_any_props(cg_model const *m, cg_node const *cls) {
    cg_foreach(m, cls->id, CG_KIND_PROPERTY, p) {
        if (!(p->flags & CG_FLAG_PRIVATE)) return 1;
    }
    return 0;
}

/* Emit a Markdown properties table for all public properties owned by cls. */
static int emit_props_for_class(outbuf *b, cg_model const *m, cg_node const *cls,
                                char const *section_name, int use_subsection) {
    int has = 0;
    cg_node const *p;
    for (p = cg_first(m, cls->id, CG_KIND_PROPERTY); p;
         p = cg_next(m, p, cls->id, CG_KIND_PROPERTY)) {
        if (!(p->flags & CG_FLAG_PRIVATE)) { has = 1; break; }
    }
    if (!has) return 0;
    
    if (use_subsection && ob_printf(b, "\n### From %s\n\n", section_name) < 0) return -1;
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

/* Walk a comma-separated parent list and emit one properties section per
 * parent that is found in the model (one level deep only). */
static int emit_inherited_props(outbuf *b, cg_model const *m, char const *parent_list) {
    char buf[1024];
    char *tok;
    if (!parent_list || !parent_list[0]) return 0;
    snprintf(buf, sizeof(buf), "%s", parent_list);
    for (tok = strtok(buf, ","); tok; tok = strtok(NULL, ",")) {
        cg_node const *parent;
        while (*tok == ' ' || *tok == '\t') ++tok;
        parent = find_class(m, tok);
        if (parent)
            if (emit_props_for_class(b, m, parent, parent->name, 1) < 0) return -1;
    }
    return 0;
}

/* Emit a single-parent inheritance chain as "Root -> ... -> ClassName".
 * Stops after MAX_CHAIN ancestors or when a multi-parent ancestor is encountered. */
#define MAX_CHAIN 16
static int emit_chain(outbuf *b, cg_model const *m, cg_node const *cls) {
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
static int emit_inheritance(outbuf *b, cg_model const *m, cg_node const *cls) {
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
static int emit_handles(outbuf *b, cg_model const *m, cg_node const *cls) {
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
static int emit_class_docs(cg_host_v1 const *host, cg_model const *m, 
                           cg_node const *cls, char const *out_path) {
    outbuf b = {0};
    int own_props, inh_props;

    /* Heading */
    if (ob_printf(&b, "# %s\n\n", cls->name) < 0) { free(b.s); return -1; }

    /* Definition */
    if (ob_printf(&b, "**Definition:** `%s`\n", m->xml_path) < 0) { free(b.s); return -1; }

    /* Namespace */
    if (cls->aux2 && cls->aux2[0])
        if (ob_printf(&b, "**Namespace:** `%s`\n", cls->aux2) < 0) { free(b.s); return -1; }

    /* Summary */
    if (cls->doc && cls->doc[0])
        if (ob_printf(&b, "\n## Summary\n\n%s\n", cls->doc) < 0) { free(b.s); return -1; }

    /* Inheritance */
    if (emit_inheritance(&b, m, cls) < 0) { free(b.s); return -1; }

    /* Properties */
    own_props = has_any_props(m, cls);
    inh_props = parents_have_props(m, cls->type);
    if (own_props || inh_props) {
        if (ob_printf(&b, "\n## Properties\n") < 0) { free(b.s); return -1; }
        if (own_props && emit_props_for_class(&b, m, cls, cls->name, 1) < 0) { 
            free(b.s); return -1; 
        }
        if (inh_props && emit_inherited_props(&b, m, cls->type) < 0) { 
            free(b.s); return -1; 
        }
    }

    /* Handled messages */
    if (emit_handles(&b, m, cls) < 0) { free(b.s); return -1; }

    /* Implementation details */
    if (cls->aux && cls->aux[0])
        if (ob_printf(&b, "\n## Implementation Details\n\n%s\n", cls->aux) < 0) { 
            free(b.s); return -1; 
        }

    /* Write to file */
    if (host->write_file(out_path, b.s, b.n) < 0) {
        host->logf("docs: failed write %s", out_path);
        free(b.s);
        return -1;
    }
    free(b.s);
    return 0;
}

static int emit_docs(cg_host_v1 const *host, cg_model const *model, char const *output_dir) {
    outbuf b = {0};
    size_t i;
    uint32_t n_enum = 0, n_if = 0, n_struct = 0, n_class = 0;

    /* Count nodes by kind. */
    for (i = 0; i < model->node_count; ++i) {
        n_enum += model->nodes[i].kind == CG_KIND_ENUM;
        n_if += model->nodes[i].kind == CG_KIND_INTERFACE;
        n_struct += model->nodes[i].kind == CG_KIND_STRUCT;
        n_class += model->nodes[i].kind == CG_KIND_CLASS;
    }

    /* Build module overview. */
    if (ob_printf(&b,
            "# %s\n\n"
            "- **source:** `%s`\n"
            "- **enums:** %u\n"
            "- **interfaces:** %u\n"
            "- **structs:** %u\n"
            "- **classes:** %u\n\n"
            "## Classes\n\n",
            model->module_name, model->xml_path, n_enum, n_if, n_struct, n_class) < 0) {
        free(b.s);
        return -1;
    }

    /* List all classes in the overview with links. */
    for (i = 0; i < model->node_count; ++i)
        if (model->nodes[i].kind == CG_KIND_CLASS)
            if (ob_printf(&b, "- [%s](classes/%s.md)\n",
                    model->nodes[i].name, model->nodes[i].name) < 0) {
                free(b.s);
                return -1;
            }

    /* Write module overview. */
    if (host->write_file(output_dir, b.s, b.n) < 0) {
        host->logf("docs: failed write %s", output_dir);
        free(b.s);
        return -1;
    }
    free(b.s);

    /* Emit individual class documentation files. */
    for (i = 0; i < model->node_count; ++i) {
        char path[512];
        size_t base_len;
        if (model->nodes[i].kind != CG_KIND_CLASS) continue;

        /* Build output path: output_dir/classes/ClassName.md */
        base_len = strlen(output_dir);
        if (base_len + 1 + strlen(model->nodes[i].name) + 12 >= sizeof(path)) {
            host->logf("docs: path too long for %s", model->nodes[i].name);
            continue;
        }
        memcpy(path, output_dir, base_len);
        if (base_len > 0 && output_dir[base_len - 1] != '/') {
            path[base_len] = '/';
            base_len++;
        }
        strcpy(path + base_len, "classes/");
        strcpy(path + base_len + 8, model->nodes[i].name);
        strcpy(path + base_len + 8 + strlen(model->nodes[i].name), ".md");

        if (emit_class_docs(host, model, &model->nodes[i], path) < 0)
            return -1;
    }

    return 0;
}

static cg_plugin_v1 const plugin = { .abi = CG_API_VERSION, .name = "docs", .emit = emit_docs };

cg_plugin_v1 const *cg_plugin_init_v1(void) { return &plugin; }
