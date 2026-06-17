#include <string.h>
#include "cg_api.h"
#include "outbuf.h"

typedef enum { API_C, API_LUA, API_XML } api_mode;

/* Parse optional "c:", "lua:", or "xml:" prefix from output path.
 * Returns the mode and advances *path past the prefix. */
static api_mode parse_mode(char const **path) {
    if (!*path) return API_C;
    if (strncmp(*path, "lua:", 4) == 0) { *path += 4; return API_LUA; }
    if (strncmp(*path, "xml:", 4) == 0) { *path += 4; return API_XML; }
    if (strncmp(*path, "c:",   2) == 0) { *path += 2; return API_C;   }
    return API_C;
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

/* Emit the full documentation block for one class (C API mode). */
static int emit_class_docs(cg_host_v1 const *host, cg_model const *m,
                           cg_node const *cls, char const *out_path) {
    outbuf b = {0};
    int own_props, inh_props;

    if (ob_printf(&b, "# %s\n\n", cls->name) < 0) { free(b.s); return -1; }
    if (ob_printf(&b, "**Definition:** `%s`\n", m->xml_path) < 0) { free(b.s); return -1; }
    if (cls->aux2 && cls->aux2[0])
        if (ob_printf(&b, "**Namespace:** `%s`\n", cls->aux2) < 0) { free(b.s); return -1; }
    if (cls->doc && cls->doc[0])
        if (ob_printf(&b, "\n## Summary\n\n%s\n", cls->doc) < 0) { free(b.s); return -1; }
    if (emit_inheritance(&b, m, cls) < 0) { free(b.s); return -1; }

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

    if (emit_handles(&b, m, cls) < 0) { free(b.s); return -1; }
    if (emit_see_also(&b, m, cls, CG_KIND_PROPERTY, 1) < 0) { free(b.s); return -1; }

    if (cls->aux && cls->aux[0])
        if (ob_printf(&b, "\n## Implementation Details\n\n%s\n", cls->aux) < 0) {
            free(b.s); return -1;
        }

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
/* prefix: if non-NULL and non-empty, prepended to the method name (e.g. "VEC3_"). */
static int emit_method_doc(outbuf *b, cg_model const *m, cg_node const *method,
                           char const *prefix) {
    cg_node const *ret = cg_first(m, method->id, CG_KIND_RETURNS);
    if (prefix && prefix[0])
        { if (ob_printf(b, "| `%s%s` | %s | ", prefix, method->name, method->doc ? method->doc : "") < 0) return -1; }
    else
        { if (ob_printf(b, "| `%s` | %s | ", method->name, method->doc ? method->doc : "") < 0) return -1; }
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
/* prefix: prepended to each method name; NULL or "" for interfaces. */
static int emit_interface_methods(outbuf *b, cg_model const *m, cg_node const *iface,
                                  char const *prefix) {
    int has = 0;
    cg_foreach(m, iface->id, CG_KIND_METHOD, mt) {
        if (!(mt->flags & CG_FLAG_PRIVATE)) { has = 1; break; }
    }
    if (!has) return 0;
    if (ob_printf(b, "\n### Methods\n\n") < 0) return -1;
    if (ob_printf(b, "| Method | Description | Args | Type |\n"
                      "|--------|-------------|------|------|\n") < 0) return -1;
    cg_foreach(m, iface->id, CG_KIND_METHOD, method) {
        if (method->flags & CG_FLAG_PRIVATE) continue;
        if (emit_method_doc(b, m, method, prefix) < 0) return -1;
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
    if (emit_interface_methods(&b, m, iface, NULL) < 0) { free(b.s); return -1; }
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
    if (emit_interface_methods(&b, m, st, st->type) < 0) { free(b.s); return -1; }
    if (emit_see_also(&b, m, st, CG_KIND_FIELD, 0) < 0) { free(b.s); return -1; }
    if (host->write_file(out_path, b.s, b.n) < 0) {
        host->logf("docs: failed write %s", out_path);
        free(b.s);
        return -1;
    }
    free(b.s);
    return 0;
}

/* ============================================================
 * LUA API mode
 * All public methods are exposed to Lua.
 * Lua name = export attribute if set, else method name with first
 * letter lowercased (camelCase convention).
 * lua="true" flag only means the handler receives a lua_State* —
 * it is NOT a filter for Lua exposure.
 * ============================================================ */

/* Return the Lua-visible name for a method node into buf[bufsz]. */
static char const *lua_method_name(cg_node const *mt, char *buf, size_t bufsz) {
    char const *src;
    if (mt->extra && mt->extra[0]) return mt->extra;
    src = mt->name;
    if (!src || !src[0]) return src;
    snprintf(buf, bufsz, "%c%s",
             (src[0] >= 'A' && src[0] <= 'Z') ? (char)(src[0] + 32) : src[0],
             src + 1);
    return buf;
}

/* Emit Lua method args as "`name`, `name2`, ..." with optional inline doc. */
static int emit_lua_args(outbuf *b, cg_model const *m, uint32_t parent_id) {
    int first = 1;
    cg_node const *arg;
    for (arg = cg_first(m, parent_id, CG_KIND_ARG); arg;
         arg = cg_next(m, arg, parent_id, CG_KIND_ARG)) {
        if (!first && ob_printf(b, ", ") < 0) return -1;
        if (ob_printf(b, "`%s`", arg->name) < 0) return -1;
        if (arg->doc && arg->doc[0])
            if (ob_printf(b, " (%s)", arg->doc) < 0) return -1;
        first = 0;
    }
    if (first) return ob_printf(b, "\xe2\x80\x94"); /* — */
    return 0;
}

/* Emit the Methods table for a Lua-mode node (class or interface). */
static int emit_lua_methods_table(outbuf *b, cg_model const *m, cg_node const *node) {
    int has = 0;
    char namebuf[256];
    cg_foreach(m, node->id, CG_KIND_METHOD, mt) {
        if (!(mt->flags & CG_FLAG_PRIVATE)) { has = 1; break; }
    }
    if (!has) return 0;
    if (ob_printf(b, "\n## Methods\n\n"
                     "| Method | Description | Parameters |\n"
                     "|--------|-------------|------------|\n") < 0) return -1;
    cg_foreach(m, node->id, CG_KIND_METHOD, mt) {
        if (mt->flags & CG_FLAG_PRIVATE) continue;
        if (ob_printf(b, "| `%s` | %s | ",
                      lua_method_name(mt, namebuf, sizeof(namebuf)),
                      mt->doc ? mt->doc : "") < 0) return -1;
        if (emit_lua_args(b, m, mt->id) < 0) return -1;
        if (ob_printf(b, " |\n") < 0) return -1;
    }
    return 0;
}

/* Emit a full inline Lua node block into b (for stdout mode). */
static int emit_lua_methods_table_inline(outbuf *b, cg_model const *m, cg_node const *node) {
    int own, inh;
    if (ob_printf(b, "# %s\n\n", node->name) < 0) return -1;
    if (ob_printf(b, "**Definition:** `%s`\n", m->xml_path) < 0) return -1;
    if (node->doc && node->doc[0])
        if (ob_printf(b, "\n## Summary\n\n%s\n", node->doc) < 0) return -1;
    if (emit_inheritance(b, m, node) < 0) return -1;
    own = has_any_props(m, node);
    inh = parents_have_props(m, node->type);
    if (own || inh) {
        if (ob_printf(b, "\n## Properties\n") < 0) return -1;
        if (own && emit_props_for_class(b, m, node, node->name, 1) < 0) return -1;
        if (inh && emit_inherited_props(b, m, node->type) < 0) return -1;
    }
    return emit_lua_methods_table(b, m, node);
}

/* Emit full Lua docs for one class or interface to out_path. */
static int emit_lua_class_docs(cg_host_v1 const *host, cg_model const *m,
                               cg_node const *node, char const *out_path) {
    outbuf b = {0};

    if (ob_printf(&b, "# %s\n\n", node->name) < 0) { free(b.s); return -1; }
    if (ob_printf(&b, "**Definition:** `%s`\n", m->xml_path) < 0) { free(b.s); return -1; }
    if (node->doc && node->doc[0])
        if (ob_printf(&b, "\n## Summary\n\n%s\n", node->doc) < 0) { free(b.s); return -1; }
    if (emit_inheritance(&b, m, node) < 0) { free(b.s); return -1; }

    {
        int own = has_any_props(m, node);
        int inh = parents_have_props(m, node->type);
        if (own || inh) {
            if (ob_printf(&b, "\n## Properties\n") < 0) { free(b.s); return -1; }
            if (own && emit_props_for_class(&b, m, node, node->name, 1) < 0) { free(b.s); return -1; }
            if (inh && emit_inherited_props(&b, m, node->type) < 0) { free(b.s); return -1; }
        }
    }

    if (emit_lua_methods_table(&b, m, node) < 0) { free(b.s); return -1; }

    if (host->write_file(out_path, b.s, b.n) < 0) {
        host->logf("docs: failed write %s", out_path);
        free(b.s);
        return -1;
    }
    free(b.s);
    return 0;
}

/* ============================================================
 * XML API mode
 * Emits classes with their XML element name (xmlns / class name),
 * settable properties, and handled messages with routing info.
 * ============================================================ */

static int emit_xml_class_docs(cg_host_v1 const *host, cg_model const *m,
                               cg_node const *cls, char const *out_path) {
    outbuf b = {0};
    char const *xml_name = (cls->aux2 && cls->aux2[0]) ? cls->aux2 : cls->name;

    if (ob_printf(&b, "# %s\n\n", cls->name) < 0) { free(b.s); return -1; }
    if (ob_printf(&b, "**XML element:** `<%s>`\n", xml_name) < 0) { free(b.s); return -1; }
    if (ob_printf(&b, "**Definition:** `%s`\n", m->xml_path) < 0) { free(b.s); return -1; }
    if (cls->doc && cls->doc[0])
        if (ob_printf(&b, "\n## Summary\n\n%s\n", cls->doc) < 0) { free(b.s); return -1; }
    if (emit_inheritance(&b, m, cls) < 0) { free(b.s); return -1; }

    /* XML attributes = properties */
    {
        int own = has_any_props(m, cls);
        int inh = parents_have_props(m, cls->type);
        if (own || inh) {
            if (ob_printf(&b, "\n## Attributes\n\n"
                              "> These properties are set as XML attributes on `<%s>`.\n",
                              xml_name) < 0) { free(b.s); return -1; }
            if (own && emit_props_for_class(&b, m, cls, cls->name, 1) < 0) { free(b.s); return -1; }
            if (inh && emit_inherited_props(&b, m, cls->type) < 0) { free(b.s); return -1; }
        }
    }

    /* Messages */
    {
        int has = 0;
        cg_foreach(m, cls->id, CG_KIND_MESSAGE, msg) { (void)msg; has = 1; break; }
        if (has) {
            if (ob_printf(&b, "\n## Messages\n\n"
                              "| Message | Routing | Description |\n"
                              "|---------|---------|-------------|\n") < 0) {
                free(b.s); return -1;
            }
            cg_foreach(m, cls->id, CG_KIND_MESSAGE, msg) {
                char const *routing = (msg->type && msg->type[0]) ? msg->type : "Direct";
                if (ob_printf(&b, "| `%s` | %s | %s |\n",
                              msg->name, routing,
                              msg->doc ? msg->doc : "") < 0) { free(b.s); return -1; }
            }
        }
    }

    /* Handled messages */
    if (emit_handles(&b, m, cls) < 0) { free(b.s); return -1; }

    if (host->write_file(out_path, b.s, b.n) < 0) {
        host->logf("docs: failed write %s", out_path);
        free(b.s);
        return -1;
    }
    free(b.s);
    return 0;
}

/* ============================================================
 * Path helpers
 * ============================================================ */

/* Append subdir/name.md to base, writing result into path[512]. Returns 0 on success. */
static int build_path(char *path, size_t pathsz, char const *base,
                      char const *subdir, size_t subdir_len,
                      char const *name) {
    size_t base_len = strlen(base);
    size_t name_len = strlen(name);
    /* base + '/' + subdir + name + ".md" + '\0' */
    if (base_len + 1 + subdir_len + name_len + 3 + 1 >= pathsz) return -1;
    memcpy(path, base, base_len);
    if (base_len > 0 && base[base_len - 1] != '/') path[base_len++] = '/';
    memcpy(path + base_len, subdir, subdir_len);
    memcpy(path + base_len + subdir_len, name, name_len);
    memcpy(path + base_len + subdir_len + name_len, ".md", 4); /* includes '\0' */
    return 0;
}

/* ============================================================
 * C API mode helpers (C functions with module prefix)
 * ============================================================ */

/* Return the module prefix, always with a trailing '_'.
 * model->prefix is stored as "core_" by model.c; when NULL fall back to
 * module_name with '_' appended into the caller-supplied buf[bufsz]. */
static char const *module_prefix(cg_model const *m, char *buf, size_t bufsz) {
    if (m->prefix && m->prefix[0]) return m->prefix;
    snprintf(buf, bufsz, "%s_", m->module_name ? m->module_name : "");
    return buf;
}

/* Emit a C function doc, prefixing the name with the module prefix. */
static int emit_c_function_docs(cg_host_v1 const *host, cg_model const *m,
                                cg_node const *fn, char const *out_path) {
    outbuf b = {0};
    char pbuf[128];
    char const *prefix = module_prefix(m, pbuf, sizeof(pbuf));

    if (ob_printf(&b, "# %s%s\n\n", prefix, fn->name) < 0) { free(b.s); return -1; }
    if (ob_printf(&b, "**C name:** `%s%s`\n", prefix, fn->name) < 0) { free(b.s); return -1; }
    if (ob_printf(&b, "**Definition:** `%s`\n", m->xml_path) < 0) { free(b.s); return -1; }
    if (fn->doc && fn->doc[0])
        if (ob_printf(&b, "\n## Summary\n\n%s\n", fn->doc) < 0) { free(b.s); return -1; }
    if (emit_function_details(&b, m, fn) < 0) { free(b.s); return -1; }

    if (host->write_file(out_path, b.s, b.n) < 0) {
        host->logf("docs: failed write %s", out_path);
        free(b.s);
        return -1;
    }
    free(b.s);
    return 0;
}

/* ============================================================
 * Overview builder — shared logic, mode-aware
 * ============================================================ */

static int emit_overview(outbuf *b, cg_model const *model, api_mode mode,
                         uint32_t n_enum, uint32_t n_if, uint32_t n_struct,
                         uint32_t n_class) {
    char const *label =
        mode == API_LUA ? "Lua API" :
        mode == API_XML ? "XML API" : "C API";
    size_t i;

    if (ob_printf(b,
            "# %s — %s\n\n"
            "- **source:** `%s`\n"
            "- **enums:** %u\n"
            "- **interfaces:** %u\n"
            "- **structs:** %u\n"
            "- **classes:** %u\n\n",
            model->module_name, label,
            model->xml_path, n_enum, n_if, n_struct, n_class) < 0) return -1;

    if (mode == API_LUA) {
        if (n_if > 0) {
            if (ob_printf(b, "\n## Interfaces\n\n"
                             "| Name | Summary |\n"
                             "|------|---------|\n") < 0) return -1;
            for (i = 0; i < model->node_count; ++i) {
                cg_node const *c = &model->nodes[i];
                if (c->kind != CG_KIND_INTERFACE) continue;
                if (ob_printf(b, "| [%s](interfaces/%s.md) | %s |\n",
                        c->name, c->name, c->doc ? c->doc : "") < 0) return -1;
            }
        }
        if (n_class > 0) {
            if (ob_printf(b, "\n## Classes\n\n"
                             "| Name | Summary |\n"
                             "|------|---------|\n") < 0) return -1;
            for (i = 0; i < model->node_count; ++i) {
                cg_node const *c = &model->nodes[i];
                if (c->kind != CG_KIND_CLASS) continue;
                if (ob_printf(b, "| [%s](classes/%s.md) | %s |\n",
                        c->name, c->name, c->doc ? c->doc : "") < 0) return -1;
            }
        }
        return 0;
    }

    if (mode == API_XML) {
        if (ob_printf(b, "\n## Elements\n\n"
                         "| Element | XML tag | Summary |\n"
                         "|---------|---------|--------|\n") < 0) return -1;
        for (i = 0; i < model->node_count; ++i) {
            cg_node const *c = &model->nodes[i];
            char const *xml_name;
            if (c->kind != CG_KIND_CLASS) continue;
            xml_name = (c->aux2 && c->aux2[0]) ? c->aux2 : c->name;
            if (ob_printf(b, "| [%s](classes/%s.md) | `<%s>` | %s |\n",
                    c->name, c->name, xml_name, c->doc ? c->doc : "") < 0) return -1;
        }
        return 0;
    }

    /* C mode */
    if (n_enum > 0) {
        if (ob_printf(b, "\n## Enums\n\n"
                          "| Name | Summary |\n"
                          "|------|---------|\n") < 0) return -1;
        for (i = 0; i < model->node_count; ++i)
            if (model->nodes[i].kind == CG_KIND_ENUM)
                if (ob_printf(b, "| [%s](enums/%s.md) | %s |\n",
                        model->nodes[i].name, model->nodes[i].name,
                        model->nodes[i].doc ? model->nodes[i].doc : "") < 0) return -1;
    }
    if (n_struct > 0) {
        if (ob_printf(b, "\n## Structs\n\n"
                          "| Name | Summary |\n"
                          "|------|---------|\n") < 0) return -1;
        for (i = 0; i < model->node_count; ++i)
            if (model->nodes[i].kind == CG_KIND_STRUCT)
                if (ob_printf(b, "| [%s](structs/%s.md) | %s |\n",
                        model->nodes[i].name, model->nodes[i].name,
                        model->nodes[i].doc ? model->nodes[i].doc : "") < 0) return -1;
    }
    if (n_if > 0) {
        if (ob_printf(b, "\n## Interfaces\n\n"
                          "| Name | Summary |\n"
                          "|------|---------|\n") < 0) return -1;
        for (i = 0; i < model->node_count; ++i)
            if (model->nodes[i].kind == CG_KIND_INTERFACE)
                if (ob_printf(b, "| [%s](interfaces/%s.md) | %s |\n",
                        model->nodes[i].name, model->nodes[i].name,
                        model->nodes[i].doc ? model->nodes[i].doc : "") < 0) return -1;
    }
    /* Functions (with module prefix) */
    {
        uint32_t n_fn = 0;
        char pbuf[128];
        char const *prefix = module_prefix(model, pbuf, sizeof(pbuf));
        for (i = 0; i < model->node_count; ++i)
            if (model->nodes[i].kind == CG_KIND_METHOD && model->nodes[i].parent == 0
                && !(model->nodes[i].flags & CG_FLAG_PRIVATE))
                n_fn++;
        if (n_fn > 0) {
            if (ob_printf(b, "\n## Functions\n\n"
                              "| Name | Summary |\n"
                              "|------|---------|\n") < 0) return -1;
            for (i = 0; i < model->node_count; ++i) {
                cg_node const *fn = &model->nodes[i];
                if (fn->kind != CG_KIND_METHOD || fn->parent != 0
                    || (fn->flags & CG_FLAG_PRIVATE)) continue;
                if (ob_printf(b, "| [%s%s](functions/%s%s.md) | %s |\n",
                        prefix, fn->name, prefix, fn->name,
                        fn->doc ? fn->doc : "") < 0) return -1;
            }
        }
    }
    if (ob_printf(b, "\n## Classes\n\n"
                      "| Name | Summary |\n"
                      "|------|---------|\n") < 0) return -1;
    for (i = 0; i < model->node_count; ++i)
        if (model->nodes[i].kind == CG_KIND_CLASS)
            if (ob_printf(b, "| [%s](classes/%s.md) | %s |\n",
                    model->nodes[i].name, model->nodes[i].name,
                    model->nodes[i].doc ? model->nodes[i].doc : "") < 0) return -1;
    return 0;
}

/* ============================================================
 * Main emit entry point
 * ============================================================ */

static int emit_docs(cg_host_v1 const *host, cg_model const *model, char const *output) {
    outbuf b = {0};
    size_t i;
    uint32_t n_enum = 0, n_if = 0, n_struct = 0, n_class = 0;
    int is_stdout;
    api_mode mode;
    char path[512];

    mode = parse_mode(&output);
    is_stdout = (output && !strcmp(output, "-"));

    /* Count nodes by kind. */
    for (i = 0; i < model->node_count; ++i) {
        n_enum   += model->nodes[i].kind == CG_KIND_ENUM;
        n_if     += model->nodes[i].kind == CG_KIND_INTERFACE;
        n_struct += model->nodes[i].kind == CG_KIND_STRUCT;
        n_class  += model->nodes[i].kind == CG_KIND_CLASS;
    }

    /* Build overview. */
    if (emit_overview(&b, model, mode, n_enum, n_if, n_struct, n_class) < 0) {
        free(b.s);
        return -1;
    }

    /* Stdout: dump everything inline. */
    if (is_stdout) {
        if (mode == API_LUA) {
            cg_foreach(model, 0, CG_KIND_INTERFACE, iface) {
                if (ob_printf(&b, "\n---\n\n") < 0) { free(b.s); return -1; }
                if (emit_lua_methods_table_inline(&b, model, iface) < 0) { free(b.s); return -1; }
            }
            cg_foreach(model, 0, CG_KIND_CLASS, cls) {
                if (ob_printf(&b, "\n---\n\n") < 0) { free(b.s); return -1; }
                if (emit_lua_methods_table_inline(&b, model, cls) < 0) { free(b.s); return -1; }
            }
        } else if (mode == API_XML) {
            cg_foreach(model, 0, CG_KIND_CLASS, cls) {
                char const *xml_name = (cls->aux2 && cls->aux2[0]) ? cls->aux2 : cls->name;
                if (ob_printf(&b, "\n---\n\n# %s\n\n", cls->name) < 0) { free(b.s); return -1; }
                if (ob_printf(&b, "**XML element:** `<%s>`\n", xml_name) < 0) { free(b.s); return -1; }
                if (ob_printf(&b, "**Definition:** `%s`\n", model->xml_path) < 0) { free(b.s); return -1; }
                if (cls->doc && cls->doc[0])
                    if (ob_printf(&b, "\n## Summary\n\n%s\n", cls->doc) < 0) { free(b.s); return -1; }
                if (emit_inheritance(&b, model, cls) < 0) { free(b.s); return -1; }
                {
                    int own = has_any_props(model, cls);
                    int inh = parents_have_props(model, cls->type);
                    if (own || inh) {
                        if (ob_printf(&b, "\n## Attributes\n\n"
                                         "> These properties are set as XML attributes on `<%s>`.\n",
                                         xml_name) < 0) { free(b.s); return -1; }
                        if (own && emit_props_for_class(&b, model, cls, cls->name, 1) < 0) { free(b.s); return -1; }
                        if (inh && emit_inherited_props(&b, model, cls->type) < 0) { free(b.s); return -1; }
                    }
                }
                {
                    int has_msg = 0;
                    cg_foreach(model, cls->id, CG_KIND_MESSAGE, msg) { (void)msg; has_msg = 1; break; }
                    if (has_msg) {
                        if (ob_printf(&b, "\n## Messages\n\n"
                                         "| Message | Routing | Description |\n"
                                         "|---------|---------|-------------|\n") < 0) { free(b.s); return -1; }
                        cg_foreach(model, cls->id, CG_KIND_MESSAGE, msg) {
                            char const *routing = (msg->type && msg->type[0]) ? msg->type : "Direct";
                            if (ob_printf(&b, "| `%s` | %s | %s |\n",
                                          msg->name, routing, msg->doc ? msg->doc : "") < 0) { free(b.s); return -1; }
                        }
                    }
                }
                if (emit_handles(&b, model, cls) < 0) { free(b.s); return -1; }
            }
        } else {
            /* C mode stdout — same as original */
            cg_foreach(model, 0, CG_KIND_CLASS, cls) {
                int own_props, inh_props;
                if (ob_printf(&b, "\n---\n\n# %s\n\n", cls->name) < 0) { free(b.s); return -1; }
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
                    if (own_props && emit_props_for_class(&b, model, cls, cls->name, 1) < 0) { free(b.s); return -1; }
                    if (inh_props && emit_inherited_props(&b, model, cls->type) < 0) { free(b.s); return -1; }
                }
                if (emit_handles(&b, model, cls) < 0) { free(b.s); return -1; }
                if (emit_see_also(&b, model, cls, CG_KIND_PROPERTY, 1) < 0) { free(b.s); return -1; }
                if (cls->aux && cls->aux[0])
                    if (ob_printf(&b, "\n## Implementation Details\n\n%s\n", cls->aux) < 0) { free(b.s); return -1; }
            }
            cg_foreach(model, 0, CG_KIND_ENUM, en) {
                if (ob_printf(&b, "\n---\n\n# %s\n\n", en->name) < 0) { free(b.s); return -1; }
                if (ob_printf(&b, "**Definition:** `%s`\n", model->xml_path) < 0) { free(b.s); return -1; }
                if (en->aux2 && en->aux2[0])
                    if (ob_printf(&b, "**Namespace:** `%s`\n", en->aux2) < 0) { free(b.s); return -1; }
                if (en->doc && en->doc[0])
                    if (ob_printf(&b, "\n## Summary\n\n%s\n", en->doc) < 0) { free(b.s); return -1; }
                if (emit_enum_members(&b, model, en) < 0) { free(b.s); return -1; }
            }
            cg_foreach(model, 0, CG_KIND_STRUCT, st) {
                if (ob_printf(&b, "\n---\n\n# %s\n\n", st->name) < 0) { free(b.s); return -1; }
                if (ob_printf(&b, "**Definition:** `%s`\n", model->xml_path) < 0) { free(b.s); return -1; }
                if (st->aux2 && st->aux2[0])
                    if (ob_printf(&b, "**Namespace:** `%s`\n", st->aux2) < 0) { free(b.s); return -1; }
                if (st->doc && st->doc[0])
                    if (ob_printf(&b, "\n## Summary\n\n%s\n", st->doc) < 0) { free(b.s); return -1; }
                if (emit_struct_fields(&b, model, st) < 0) { free(b.s); return -1; }
                if (emit_interface_methods(&b, model, st, st->type) < 0) { free(b.s); return -1; }
            }
            cg_foreach(model, 0, CG_KIND_INTERFACE, iface) {
                if (ob_printf(&b, "\n---\n\n# %s\n\n", iface->name) < 0) { free(b.s); return -1; }
                if (ob_printf(&b, "**Definition:** `%s`\n", model->xml_path) < 0) { free(b.s); return -1; }
                if (iface->aux2 && iface->aux2[0])
                    if (ob_printf(&b, "**Namespace:** `%s`\n", iface->aux2) < 0) { free(b.s); return -1; }
                if (iface->doc && iface->doc[0])
                    if (ob_printf(&b, "\n## Summary\n\n%s\n", iface->doc) < 0) { free(b.s); return -1; }
                if (emit_interface_methods(&b, model, iface, NULL) < 0) { free(b.s); return -1; }
            }
            for (i = 0; i < model->node_count; ++i) {
                cg_node const *fn = &model->nodes[i];
                char pbuf[128];
                char const *prefix = module_prefix(model, pbuf, sizeof(pbuf));
                if (fn->kind != CG_KIND_METHOD || fn->parent != 0 || (fn->flags & CG_FLAG_PRIVATE)) continue;
                if (ob_printf(&b, "\n---\n\n# %s%s\n\n", prefix, fn->name) < 0) { free(b.s); return -1; }
                if (ob_printf(&b, "**C name:** `%s%s`\n", prefix, fn->name) < 0) { free(b.s); return -1; }
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
        return 0;
    }

    /* ---- Directory mode ---- */

    /* Write overview.md */
    {
        size_t base_len = strlen(output);
        if (base_len + 12 >= sizeof(path)) {
            host->logf("docs: path too long for overview");
            free(b.s);
            return -1;
        }
        memcpy(path, output, base_len);
        if (base_len > 0 && output[base_len - 1] != '/') path[base_len++] = '/';
        strcpy(path + base_len, "overview.md");
        if (host->write_file(path, b.s, b.n) < 0) {
            host->logf("docs: failed write %s", path);
            free(b.s);
            return -1;
        }
    }
    free(b.s);

    if (mode == API_LUA) {
        cg_foreach(model, 0, CG_KIND_INTERFACE, iface) {
            if (build_path(path, sizeof(path), output, "interfaces/", 11, iface->name) < 0) {
                host->logf("docs: path too long for %s", iface->name); continue;
            }
            if (emit_lua_class_docs(host, model, iface, path) < 0) return -1;
        }
        cg_foreach(model, 0, CG_KIND_CLASS, cls) {
            if (build_path(path, sizeof(path), output, "classes/", 8, cls->name) < 0) {
                host->logf("docs: path too long for %s", cls->name); continue;
            }
            if (emit_lua_class_docs(host, model, cls, path) < 0) return -1;
        }
        return 0;
    }

    if (mode == API_XML) {
        /* XML mode: only class files */
        cg_foreach(model, 0, CG_KIND_CLASS, cls) {
            if (build_path(path, sizeof(path), output, "classes/", 8, cls->name) < 0) {
                host->logf("docs: path too long for %s", cls->name); continue;
            }
            if (emit_xml_class_docs(host, model, cls, path) < 0) return -1;
        }
        return 0;
    }

    /* C mode: full output — enums, structs, interfaces, functions, classes */
    cg_foreach(model, 0, CG_KIND_ENUM, en) {
        if (build_path(path, sizeof(path), output, "enums/", 6, en->name) < 0) {
            host->logf("docs: path too long for %s", en->name); continue;
        }
        if (emit_enum_docs(host, model, en, path) < 0) return -1;
    }
    cg_foreach(model, 0, CG_KIND_STRUCT, st) {
        if (build_path(path, sizeof(path), output, "structs/", 8, st->name) < 0) {
            host->logf("docs: path too long for %s", st->name); continue;
        }
        if (emit_struct_docs(host, model, st, path) < 0) return -1;
    }
    cg_foreach(model, 0, CG_KIND_INTERFACE, iface) {
        if (build_path(path, sizeof(path), output, "interfaces/", 11, iface->name) < 0) {
            host->logf("docs: path too long for %s", iface->name); continue;
        }
        if (emit_interface_docs(host, model, iface, path) < 0) return -1;
    }
    cg_foreach(model, 0, CG_KIND_CLASS, cls) {
        if (build_path(path, sizeof(path), output, "classes/", 8, cls->name) < 0) {
            host->logf("docs: path too long for %s", cls->name); continue;
        }
        if (emit_class_docs(host, model, cls, path) < 0) return -1;
    }
    /* C functions with module prefix */
    {
        char pbuf[128];
        char const *prefix = module_prefix(model, pbuf, sizeof(pbuf));
        for (i = 0; i < model->node_count; ++i) {
            char fn_name[256];
            cg_node const *fn = &model->nodes[i];
            if (fn->kind != CG_KIND_METHOD || fn->parent != 0 || (fn->flags & CG_FLAG_PRIVATE)) continue;
            snprintf(fn_name, sizeof(fn_name), "%s%s", prefix, fn->name);
            if (build_path(path, sizeof(path), output, "functions/", 10, fn_name) < 0) {
                host->logf("docs: path too long for %s", fn_name); continue;
            }
            if (emit_c_function_docs(host, model, fn, path) < 0) return -1;
        }
    }
    return 0;
}

static cg_plugin_v1 const plugin = { .abi = CG_API_VERSION, .name = "docs", .emit = emit_docs };

cg_plugin_v1 const *cg_plugin_init_v1(void) { return &plugin; }
