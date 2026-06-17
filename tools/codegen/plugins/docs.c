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

/* Find an enum or struct node by name (returns first match). */
static cg_node const *find_enum_or_struct(cg_model const *m, char const *name) {
    size_t i;
    if (!name || !name[0]) return NULL;
    for (i = 0; i < m->node_count; ++i) {
        cg_node const *n = &m->nodes[i];
        if ((n->kind == CG_KIND_ENUM || n->kind == CG_KIND_STRUCT) && !strcmp(n->name, name))
            return n;
    }
    return NULL;
}

/* Strip pointer/array from a type string to get the base name. */
static char const *strip_ptr_array(char const *t) {
    if (!t) return "";
    while (*t == '*' || *t == ' ' || *t == '[') ++t;
    return t;
}

/* Collect unique names from a comma-separated list, storing pointers in out[]. */
static int collect_names(char const **out, int outsz, char const *list) {
    char tmp[1024];
    char *tok;
    int n = 0;
    if (!list || !list[0]) return 0;
    snprintf(tmp, sizeof(tmp), "%s", list);
    for (tok = strtok(tmp, ","); tok; tok = strtok(NULL, ",")) {
        while (*tok == ' ' || *tok == '\t') ++tok;
        if (*tok && n < outsz) {
            out[n++] = tok;
        }
    }
    return n;
}

/* Emit a ## See Also section with parent classes/structs and types used in properties/fields.
 * Returns the number of links emitted, or -1 on error. */
static int emit_see_also(outbuf *b, cg_model const *m, cg_node const *node, cg_kind child_kind, int is_class) {
    char const *parents[64];
    int np, i, count = 0;
    cg_node const *found;
    char const *base;
    int first = 1;

    np = collect_names(parents, sizeof(parents) / sizeof(parents[0]), node->type);

    /* Count total links before emitting the section header. */
    {
        int total = 0;
        cg_foreach(m, node->id, child_kind, _p) {
            if (_p->flags & CG_FLAG_PRIVATE) continue;
            base = strip_ptr_array(_p->type);
            if (find_enum_or_struct(m, base) || find_class(m, base)) total++;
        }
        for (i = 0; i < np; ++i) {
            if (is_class ? find_class(m, parents[i]) : find_enum_or_struct(m, parents[i])) total++;
        }
        if (total == 0) return 0;
        if (ob_printf(b, "\n## See Also\n\n") < 0) return -1;
    }

    cg_foreach(m, node->id, child_kind, pr) {
        if (pr->flags & CG_FLAG_PRIVATE) continue;
        base = strip_ptr_array(pr->type);
        found = find_enum_or_struct(m, base);
        if (!found) found = find_class(m, base);
        if (found) {
            if (found->kind == CG_KIND_CLASS) {
                if (ob_printf(b, "%s- [%s](classes/%s.md)\n",
                        first ? "" : "\n", found->name, found->name) < 0)
                    return -1;
            } else {
                if (ob_printf(b, "%s- [%s](%s/%s.md) — %s\n",
                        first ? "" : "\n",
                        found->name,
                        found->kind == CG_KIND_ENUM ? "enums" : "structs",
                        found->name,
                        found->doc && found->doc[0] ? found->doc : "") < 0)
                    return -1;
            }
            first = 0;
            count++;
        }
    }

    for (i = 0; i < np; ++i) {
        cg_node const *parent = is_class ? find_class(m, parents[i]) : find_enum_or_struct(m, parents[i]);
        if (parent) {
            if (ob_printf(b, "%s- [%s](%s/%s.md)\n",
                    first ? "" : "\n", parent->name,
                    is_class ? "classes" : "structs", parent->name) < 0)
                return -1;
            first = 0;
            count++;
        }
    }
    return count;
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

    /* See Also */
    if (emit_see_also(&b, m, cls, CG_KIND_PROPERTY, 1) < 0) { free(b.s); return -1; }

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

/* Emit the ## Members section for an enum (list of values). */
static int emit_enum_members(outbuf *b, cg_model const *m, cg_node const *en) {
    int has = 0;
    cg_foreach(m, en->id, CG_KIND_VALUE, v) { (void)v; has = 1; break; }
    if (!has) return 0;
    if (ob_printf(b, "\n### Members\n\n") < 0) return -1;
    cg_foreach(m, en->id, CG_KIND_VALUE, v) {
        if (ob_printf(b, "| `%s` | %s |\n", v->name, v->doc ? v->doc : "") < 0) return -1;
    }
    return 0;
}

/* Format args as comma-separated list of types. */
static int emit_args_list(outbuf *b, cg_model const *m, uint32_t parent_id) {
    int first = 1;
    cg_node const *arg;
    for (arg = cg_first(m, parent_id, CG_KIND_ARG); arg;
         arg = cg_next(m, arg, parent_id, CG_KIND_ARG)) {
        if (!first && ob_printf(b, ", ") < 0) return -1;
        if (emit_type(b, arg) < 0) return -1;
        first = 0;
    }
    if (first) return ob_printf(b, "\xe2\x80\x94"); /* — */
    return 0;
}

/* Emit a single method with its args and return. */
static int emit_method_doc(outbuf *b, cg_model const *m, cg_node const *method) {
    cg_node const *ret = cg_first(m, method->id, CG_KIND_RETURNS);
    if (ob_printf(b, "| `%s` | %s | ", method->name, method->doc ? method->doc : "") < 0) return -1;
    if (emit_args_list(b, m, method->id) < 0) return -1;
    if (ret) {
        if (ob_printf(b, " | ") < 0) return -1;
        if (emit_type(b, ret) < 0) return -1;
    } else {
        if (ob_printf(b, " |") < 0) return -1;
    }
    return ob_printf(b, " |\n");
}

/* Emit an arguments table and return type for a function (standalone, parent==0). */
static int emit_function_details(outbuf *b, cg_model const *m, cg_node const *fn) {
    cg_node const *arg;
    cg_node const *ret = cg_first(m, fn->id, CG_KIND_RETURNS);
    int has_args = 0;

    /* Count args */
    for (arg = cg_first(m, fn->id, CG_KIND_ARG); arg;
         arg = cg_next(m, arg, fn->id, CG_KIND_ARG)) {
        has_args = 1;
        break;
    }

    if (has_args) {
        if (ob_printf(b, "\n### Parameters\n\n"
                         "| Name | Type | Summary |\n"
                         "|------|------|---------|\n") < 0) return -1;
        for (arg = cg_first(m, fn->id, CG_KIND_ARG); arg;
             arg = cg_next(m, arg, fn->id, CG_KIND_ARG)) {
            if (ob_printf(b, "| `%s` | ", arg->name) < 0) return -1;
            if (emit_type(b, arg) < 0) return -1;
            if (ob_printf(b, " | %s |\n", arg->doc ? arg->doc : "") < 0) return -1;
        }
    }

    if (ret) {
        if (ob_printf(b, "\n*Returns:* ") < 0) return -1;
        if (emit_type(b, ret) < 0) return -1;
        if (ob_printf(b, "\n") < 0) return -1;
    }

    return 0;
}

/* Emit the ## Methods section for an interface. */
static int emit_interface_methods(outbuf *b, cg_model const *m, cg_node const *iface) {
    int has = 0;
    cg_foreach(m, iface->id, CG_KIND_METHOD, mt) { (void)mt; has = 1; break; }
    if (!has) return 0;
    if (ob_printf(b, "\n### Methods\n\n") < 0) return -1;
    if (ob_printf(b, "| Method | Description | Args | Type |\n"
                      "|--------|-------------|------|------|\n") < 0) return -1;
    cg_foreach(m, iface->id, CG_KIND_METHOD, method) {
        if (method->flags & CG_FLAG_PRIVATE) continue;
        if (emit_method_doc(b, m, method) < 0) return -1;
    }
    return 0;
}

/* Emit full documentation for an interface node. */
static int emit_interface_docs(cg_host_v1 const *host, cg_model const *m, cg_node const *iface,
                               char const *out_path) {
    outbuf b = {0};
    if (ob_printf(&b, "# %s\n\n", iface->name) < 0) { free(b.s); return -1; }
    if (ob_printf(&b, "**Definition:** `%s`\n", m->xml_path) < 0) { free(b.s); return -1; }
    if (iface->aux2 && iface->aux2[0])
        if (ob_printf(&b, "**Namespace:** `%s`\n", iface->aux2) < 0) { free(b.s); return -1; }
    if (iface->doc && iface->doc[0])
        if (ob_printf(&b, "\n## Summary\n\n%s\n", iface->doc) < 0) { free(b.s); return -1; }
    if (emit_interface_methods(&b, m, iface) < 0) { free(b.s); return -1; }
    if (host->write_file(out_path, b.s, b.n) < 0) {
        host->logf("docs: failed write %s", out_path);
        free(b.s);
        return -1;
    }
    free(b.s);
    return 0;
}

/* Emit the ## Fields section for a struct. */
static int emit_struct_fields(outbuf *b, cg_model const *m, cg_node const *st) {
    int has = 0;
    cg_foreach(m, st->id, CG_KIND_FIELD, f) { (void)f; has = 1; break; }
    if (!has) return 0;
    if (ob_printf(b, "\n### Fields\n\n") < 0) return -1;
    cg_foreach(m, st->id, CG_KIND_FIELD, f) {
        if (ob_printf(b, "| Field | Type | Description |\n"
                         "|-------|------|-------------|\n") < 0) return -1;
        if (ob_printf(b, "| `%s` | ", f->name) < 0) return -1;
        if (emit_type(b, f) < 0) return -1;
        if (ob_printf(b, " | %s |\n", f->doc ? f->doc : "") < 0) return -1;
    }
    return 0;
}

/* Emit full documentation for an enum node. */
static int emit_enum_docs(cg_host_v1 const *host, cg_model const *m, cg_node const *en,
                          char const *out_path) {
    outbuf b = {0};
    if (ob_printf(&b, "# %s\n\n", en->name) < 0) { free(b.s); return -1; }
    if (ob_printf(&b, "**Definition:** `%s`\n", m->xml_path) < 0) { free(b.s); return -1; }
    if (en->aux2 && en->aux2[0])
        if (ob_printf(&b, "**Namespace:** `%s`\n", en->aux2) < 0) { free(b.s); return -1; }
    if (en->doc && en->doc[0])
        if (ob_printf(&b, "\n## Summary\n\n%s\n", en->doc) < 0) { free(b.s); return -1; }
    if (emit_enum_members(&b, m, en) < 0) { free(b.s); return -1; }
    if (host->write_file(out_path, b.s, b.n) < 0) {
        host->logf("docs: failed write %s", out_path);
        free(b.s);
        return -1;
    }
    free(b.s);
    return 0;
}

/* Emit full documentation for a struct node. */
static int emit_struct_docs(cg_host_v1 const *host, cg_model const *m, cg_node const *st,
                            char const *out_path) {
    outbuf b = {0};
    if (ob_printf(&b, "# %s\n\n", st->name) < 0) { free(b.s); return -1; }
    if (ob_printf(&b, "**Definition:** `%s`\n", m->xml_path) < 0) { free(b.s); return -1; }
    if (st->aux2 && st->aux2[0])
        if (ob_printf(&b, "**Namespace:** `%s`\n", st->aux2) < 0) { free(b.s); return -1; }
    if (st->doc && st->doc[0])
        if (ob_printf(&b, "\n## Summary\n\n%s\n", st->doc) < 0) { free(b.s); return -1; }
    if (emit_struct_fields(&b, m, st) < 0) { free(b.s); return -1; }
    if (emit_see_also(&b, m, st, CG_KIND_FIELD, 0) < 0) { free(b.s); return -1; }
    if (host->write_file(out_path, b.s, b.n) < 0) {
        host->logf("docs: failed write %s", out_path);
        free(b.s);
        return -1;
    }
    free(b.s);
    return 0;
}

static int emit_docs(cg_host_v1 const *host, cg_model const *model, char const *output) {
    outbuf b = {0};
    size_t i;
    uint32_t n_enum = 0, n_if = 0, n_struct = 0, n_class = 0;
    int is_stdout = (output && !strcmp(output, "-"));

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
            "- **classes:** %u\n\n",
            model->module_name, model->xml_path, n_enum, n_if, n_struct, n_class) < 0) {
        free(b.s);
        return -1;
    }

    /* If stdout, emit all class docs inline; otherwise emit links. */
    if (is_stdout) {
        /* Emit all class docs in one file */
        cg_foreach(model, 0, CG_KIND_CLASS, cls) {
            if (ob_printf(&b, "\n---\n\n") < 0) { free(b.s); return -1; }
            /* Inline class documentation */
            {
                int own_props, inh_props;
                if (ob_printf(&b, "# %s\n\n", cls->name) < 0) { free(b.s); return -1; }
                if (ob_printf(&b, "**Definition:** `%s`\n", model->xml_path) < 0) { free(b.s); return -1; }
                if (cls->aux2 && cls->aux2[0])
                    if (ob_printf(&b, "**Namespace:** `%s`\n", cls->aux2) < 0) { free(b.s); return -1; }
                if (cls->doc && cls->doc[0])
                    if (ob_printf(&b, "\n## Summary\n\n%s\n", cls->doc) < 0) { free(b.s); return -1; }
                if (emit_inheritance(&b, model, cls) < 0) { free(b.s); return -1; }
                own_props = has_any_props(model, cls);
                inh_props = parents_have_props(model, cls->type);
                if (own_props || inh_props) {
                    if (ob_printf(&b, "\n## Properties\n") < 0) { free(b.s); return -1; }
                    if (own_props && emit_props_for_class(&b, model, cls, cls->name, 1) < 0) {
                        free(b.s); return -1;
                    }
                    if (inh_props && emit_inherited_props(&b, model, cls->type) < 0) {
                        free(b.s); return -1;
                    }
                }
                if (emit_handles(&b, model, cls) < 0) { free(b.s); return -1; }
                if (emit_see_also(&b, model, cls, CG_KIND_PROPERTY, 1) < 0) { free(b.s); return -1; }
                if (cls->aux && cls->aux[0])
                    if (ob_printf(&b, "\n## Implementation Details\n\n%s\n", cls->aux) < 0) {
                        free(b.s); return -1;
                    }
            }
        }
        /* Emit all enum docs */
        cg_foreach(model, 0, CG_KIND_ENUM, en) {
            if (ob_printf(&b, "\n---\n\n") < 0) { free(b.s); return -1; }
            if (ob_printf(&b, "# %s\n\n", en->name) < 0) { free(b.s); return -1; }
            if (ob_printf(&b, "**Definition:** `%s`\n", model->xml_path) < 0) { free(b.s); return -1; }
            if (en->aux2 && en->aux2[0])
                if (ob_printf(&b, "**Namespace:** `%s`\n", en->aux2) < 0) { free(b.s); return -1; }
            if (en->doc && en->doc[0])
                if (ob_printf(&b, "\n## Summary\n\n%s\n", en->doc) < 0) { free(b.s); return -1; }
            if (emit_enum_members(&b, model, en) < 0) { free(b.s); return -1; }
        }

        /* Emit all struct docs */
        cg_foreach(model, 0, CG_KIND_STRUCT, st) {
            if (ob_printf(&b, "\n---\n\n") < 0) { free(b.s); return -1; }
            if (ob_printf(&b, "# %s\n\n", st->name) < 0) { free(b.s); return -1; }
            if (ob_printf(&b, "**Definition:** `%s`\n", model->xml_path) < 0) { free(b.s); return -1; }
            if (st->aux2 && st->aux2[0])
                if (ob_printf(&b, "**Namespace:** `%s`\n", st->aux2) < 0) { free(b.s); return -1; }
            if (st->doc && st->doc[0])
                if (ob_printf(&b, "\n## Summary\n\n%s\n", st->doc) < 0) { free(b.s); return -1; }
            if (emit_struct_fields(&b, model, st) < 0) { free(b.s); return -1; }
        }

        /* Emit all interface docs */
        cg_foreach(model, 0, CG_KIND_INTERFACE, iface) {
            if (ob_printf(&b, "\n---\n\n") < 0) { free(b.s); return -1; }
            if (ob_printf(&b, "# %s\n\n", iface->name) < 0) { free(b.s); return -1; }
            if (ob_printf(&b, "**Definition:** `%s`\n", model->xml_path) < 0) { free(b.s); return -1; }
            if (iface->aux2 && iface->aux2[0])
                if (ob_printf(&b, "**Namespace:** `%s`\n", iface->aux2) < 0) { free(b.s); return -1; }
            if (iface->doc && iface->doc[0])
                if (ob_printf(&b, "\n## Summary\n\n%s\n", iface->doc) < 0) { free(b.s); return -1; }
            if (emit_interface_methods(&b, model, iface) < 0) { free(b.s); return -1; }
        }

        /* Emit all function docs */
        {
            size_t i;
            for (i = 0; i < model->node_count; ++i) {
                cg_node const *fn = &model->nodes[i];
                if (fn->kind != CG_KIND_METHOD) continue;
                if (fn->parent != 0) continue;
                if (fn->flags & CG_FLAG_PRIVATE) continue;
                if (ob_printf(&b, "\n---\n\n") < 0) { free(b.s); return -1; }
                if (ob_printf(&b, "# %s\n\n", fn->name) < 0) { free(b.s); return -1; }
                if (ob_printf(&b, "**Definition:** `%s`\n", model->xml_path) < 0) { free(b.s); return -1; }
                if (fn->doc && fn->doc[0])
                    if (ob_printf(&b, "\n## Summary\n\n%s\n", fn->doc) < 0) { free(b.s); return -1; }
                if (emit_function_details(&b, model, fn) < 0) { free(b.s); return -1; }
            }
        }

        if (host->write_file(output, b.s, b.n) < 0) {
            host->logf("docs: failed write %s", output);
            free(b.s);
            return -1;
        }
        free(b.s);
    } else {
        /* Directory mode: emit overview with links and separate class files */
        if (n_enum > 0) {
            if (ob_printf(&b, "\n## Enums\n\n"
                              "| Name | Summary |\n"
                              "|------|---------|\n") < 0) { free(b.s); return -1; }
            for (i = 0; i < model->node_count; ++i)
                if (model->nodes[i].kind == CG_KIND_ENUM)
                    if (ob_printf(&b, "| [%s](enums/%s.md) | %s |\n",
                            model->nodes[i].name, model->nodes[i].name,
                            model->nodes[i].doc ? model->nodes[i].doc : "") < 0) {
                        free(b.s);
                        return -1;
                    }
        }
        if (n_struct > 0) {
            if (ob_printf(&b, "\n## Structs\n\n"
                              "| Name | Summary |\n"
                              "|------|---------|\n") < 0) { free(b.s); return -1; }
            for (i = 0; i < model->node_count; ++i)
                if (model->nodes[i].kind == CG_KIND_STRUCT)
                    if (ob_printf(&b, "| [%s](structs/%s.md) | %s |\n",
                            model->nodes[i].name, model->nodes[i].name,
                            model->nodes[i].doc ? model->nodes[i].doc : "") < 0) {
                        free(b.s);
                        return -1;
                    }
        }
        if (n_if > 0) {
            if (ob_printf(&b, "\n## Interfaces\n\n"
                              "| Name | Summary |\n"
                              "|------|---------|\n") < 0) { free(b.s); return -1; }
            for (i = 0; i < model->node_count; ++i)
                if (model->nodes[i].kind == CG_KIND_INTERFACE)
                    if (ob_printf(&b, "| [%s](interfaces/%s.md) | %s |\n",
                            model->nodes[i].name, model->nodes[i].name,
                            model->nodes[i].doc ? model->nodes[i].doc : "") < 0) {
                        free(b.s);
                        return -1;
                    }
        }

        /* Count and emit functions */
        {
            uint32_t n_fn = 0;
            size_t j;
            for (j = 0; j < model->node_count; ++j)
                if (model->nodes[j].kind == CG_KIND_METHOD && model->nodes[j].parent == 0 && !(model->nodes[j].flags & CG_FLAG_PRIVATE))
                    n_fn++;
            if (n_fn > 0) {
                if (ob_printf(&b, "\n## Functions\n\n"
                                  "| Name | Summary |\n"
                                  "|------|---------|\n") < 0) { free(b.s); return -1; }
                for (j = 0; j < model->node_count; ++j)
                    if (model->nodes[j].kind == CG_KIND_METHOD && model->nodes[j].parent == 0 && !(model->nodes[j].flags & CG_FLAG_PRIVATE))
                        if (ob_printf(&b, "| [%s](functions/%s.md) | %s |\n",
                                model->nodes[j].name, model->nodes[j].name,
                                model->nodes[j].doc ? model->nodes[j].doc : "") < 0) {
                            free(b.s);
                            return -1;
                        }
            }
        }

        if (ob_printf(&b, "\n## Classes\n\n"
                          "| Name | Summary |\n"
                          "|------|---------|\n") < 0) { free(b.s); return -1; }
        for (i = 0; i < model->node_count; ++i)
            if (model->nodes[i].kind == CG_KIND_CLASS)
                if (ob_printf(&b, "| [%s](classes/%s.md) | %s |\n",
                        model->nodes[i].name, model->nodes[i].name,
                        model->nodes[i].doc ? model->nodes[i].doc : "") < 0) {
                    free(b.s);
                    return -1;
                }

        /* Write module overview. */
        {
            char path[512];
            size_t base_len = strlen(output);
            if (base_len + 11 < sizeof(path)) {
                memcpy(path, output, base_len);
                if (base_len > 0 && output[base_len - 1] != '/') {
                    path[base_len] = '/';
                    base_len++;
                }
                strcpy(path + base_len, "overview.md");
                if (host->write_file(path, b.s, b.n) < 0) {
                    host->logf("docs: failed write %s", path);
                    free(b.s);
                    return -1;
                }
            } else {
                host->logf("docs: path too long for overview");
                free(b.s);
                return -1;
            }
        }
        free(b.s);

        /* Emit individual class documentation files. */
        cg_foreach(model, 0, CG_KIND_CLASS, c) {
            char path[512];
            size_t base_len;

            /* Build output path: output/classes/ClassName.md */
            base_len = strlen(output);
            if (base_len + 1 + strlen(c->name) + 12 >= sizeof(path)) {
                host->logf("docs: path too long for %s", c->name);
                continue;
            }
            memcpy(path, output, base_len);
            if (base_len > 0 && output[base_len - 1] != '/') {
                path[base_len] = '/';
                base_len++;
            }
            strcpy(path + base_len, "classes/");
            strcpy(path + base_len + 8, c->name);
            strcpy(path + base_len + 8 + strlen(c->name), ".md");

            if (emit_class_docs(host, model, c, path) < 0)
                return -1;
        }

        /* Emit individual enum documentation files. */
        cg_foreach(model, 0, CG_KIND_ENUM, en) {
            char path[512];
            size_t base_len;
            base_len = strlen(output);
            if (base_len + 1 + strlen(en->name) + 11 >= sizeof(path)) {
                host->logf("docs: path too long for %s", en->name);
                continue;
            }
            memcpy(path, output, base_len);
            if (base_len > 0 && output[base_len - 1] != '/') {
                path[base_len] = '/';
                base_len++;
            }
            strcpy(path + base_len, "enums/");
            strcpy(path + base_len + 6, en->name);
            strcpy(path + base_len + 6 + strlen(en->name), ".md");

            if (emit_enum_docs(host, model, en, path) < 0)
                return -1;
        }

        /* Emit individual struct documentation files. */
        cg_foreach(model, 0, CG_KIND_STRUCT, st) {
            char path[512];
            size_t base_len;
            base_len = strlen(output);
            if (base_len + 1 + strlen(st->name) + 12 >= sizeof(path)) {
                host->logf("docs: path too long for %s", st->name);
                continue;
            }
            memcpy(path, output, base_len);
            if (base_len > 0 && output[base_len - 1] != '/') {
                path[base_len] = '/';
                base_len++;
            }
            strcpy(path + base_len, "structs/");
            strcpy(path + base_len + 8, st->name);
            strcpy(path + base_len + 8 + strlen(st->name), ".md");

            if (emit_struct_docs(host, model, st, path) < 0)
                return -1;
        }

        /* Emit individual interface documentation files. */
        cg_foreach(model, 0, CG_KIND_INTERFACE, iface) {
            char path[512];
            size_t base_len;
            base_len = strlen(output);
            if (base_len + 1 + strlen(iface->name) + 12 >= sizeof(path)) {
                host->logf("docs: path too long for %s", iface->name);
                continue;
            }
            memcpy(path, output, base_len);
            if (base_len > 0 && output[base_len - 1] != '/') {
                path[base_len] = '/';
                base_len++;
            }
            strcpy(path + base_len, "interfaces/");
            strcpy(path + base_len + 11, iface->name);
            strcpy(path + base_len + 11 + strlen(iface->name), ".md");

            if (emit_interface_docs(host, model, iface, path) < 0)
                return -1;
        }

        /* Emit individual function documentation files. */
        {
            size_t i;
            for (i = 0; i < model->node_count; ++i) {
                cg_node const *fn = &model->nodes[i];
                char path[512];
                size_t base_len;
                outbuf fb = {0};

                if (fn->kind != CG_KIND_METHOD) continue;
                if (fn->parent != 0) continue;
                if (fn->flags & CG_FLAG_PRIVATE) continue;

                if (ob_printf(&fb, "# %s\n\n", fn->name) < 0) { free(fb.s); continue; }
                if (ob_printf(&fb, "**Definition:** `%s`\n", model->xml_path) < 0) { free(fb.s); continue; }
                if (fn->doc && fn->doc[0])
                    if (ob_printf(&fb, "\n## Summary\n\n%s\n", fn->doc) < 0) { free(fb.s); continue; }
                if (emit_function_details(&fb, model, fn) < 0) { free(fb.s); continue; }

                base_len = strlen(output);
                if (base_len + 1 + strlen(fn->name) + 11 >= sizeof(path)) {
                    host->logf("docs: path too long for %s", fn->name);
                    free(fb.s);
                    continue;
                }
                memcpy(path, output, base_len);
                if (base_len > 0 && output[base_len - 1] != '/') {
                    path[base_len] = '/';
                    base_len++;
                }
                strcpy(path + base_len, "functions/");
                strcpy(path + base_len + 10, fn->name);
                strcpy(path + base_len + 10 + strlen(fn->name), ".md");

                if (host->write_file(path, fb.s, fb.n) < 0) {
                    host->logf("docs: failed write %s", path);
                    free(fb.s);
                    return -1;
                }
                free(fb.s);
            }
        }
    }

    return 0;
}

static cg_plugin_v1 const plugin = { .abi = CG_API_VERSION, .name = "docs", .emit = emit_docs };

cg_plugin_v1 const *cg_plugin_init_v1(void) { return &plugin; }
