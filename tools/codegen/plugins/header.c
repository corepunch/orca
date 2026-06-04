#include "cg_api.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { char *s; size_t n, cap; } ob;

static int ob_printf(ob *b, char const *fmt, ...) {
    va_list ap, cp;
    int need;
    va_start(ap, fmt);
    va_copy(cp, ap);
    need = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (need < 0) { va_end(cp); return -1; }
    if (b->n + (size_t)need + 1u > b->cap) {
        size_t nc = b->cap ? b->cap * 2u : 4096u;
        while (nc < b->n + (size_t)need + 1u) nc *= 2u;
        char *ns = (char *)realloc(b->s, nc);
        if (!ns) { va_end(cp); return -1; }
        b->s = ns;
        b->cap = nc;
    }
    vsnprintf(b->s + b->n, b->cap - b->n, fmt, cp);
    va_end(cp);
    b->n += (size_t)need;
    return 0;
}

static char const *base_name(char const *path) {
    char const *slash = path ? strrchr(path, '/') : NULL;
    return slash ? slash + 1 : (path ? path : "");
}

static void module_name_from_path(char const *path, char *out, size_t out_sz) {
    char const *base = base_name(path);
    size_t len;
    if (!out_sz) return;
    snprintf(out, out_sz, "%s", base);
    len = strlen(out);
    if (len >= 5u && !strcmp(out + len - 5u, ".cgen")) {
        out[len - 5u] = 0;
    } else if (len >= 4u && !strcmp(out + len - 4u, ".xml")) {
        out[len - 4u] = 0;
    }
}

static char *upper(char const *s) {
    size_t i, n = strlen(s);
    char *d = (char *)malloc(n + 1u);
    if (!d) return NULL;
    for (i = 0; i < n; ++i) {
        char c = s[i];
        d[i] = (c >= 'a' && c <= 'z') ? (char)(c - 32) : c;
    }
    d[n] = '\0';
    return d;
}

static int type_is_kind(cg_model const *m, char const *type, cg_kind k) {
    size_t i;
    if (!type || !type[0]) return 0;
    for (i = 0; i < m->node_count; ++i)
        if (m->nodes[i].kind == k && !strcmp(m->nodes[i].name, type)) return 1;
    return 0;
}

static char const *type_kind(cg_model const *m, char const *type) {
    if (!type || !type[0]) return "void";
    if (type_is_kind(m, type, CG_KIND_ENUM)) return "enum";
    if (type_is_kind(m, type, CG_KIND_STRUCT)) return "struct";
    if (type_is_kind(m, type, CG_KIND_INTERFACE)) return "interface";
    if (type_is_kind(m, type, CG_KIND_CLASS)) return "component";
    if (type_is_kind(m, type, CG_KIND_EXTERNAL)) return "external_struct";
    return type;
}

#define TYPE_DECL_BASE_BUFFER 248 /* leaves room for optional " const*" suffix in 256-byte dst */
#define EVENT_STRUCT_NAME_BUFFER 240 /* leaves room for "struct " prefix in 256-byte dst */

static int type_decl(char *dst, size_t dsz, cg_model const *m,
                     char const *type, uint32_t flags) {
    char const *kind = type_kind(m, type);
    char base[TYPE_DECL_BASE_BUFFER];
    if (!type || !type[0]) type = "void";
    if (!strcmp(kind, "float")) snprintf(base, sizeof(base), "float");
    else if (!strcmp(kind, "int")) snprintf(base, sizeof(base), "int32_t");
    else if (!strcmp(kind, "uint")) snprintf(base, sizeof(base), "uint32_t");
    else if (!strcmp(kind, "long")) snprintf(base, sizeof(base), "long");
    else if (!strcmp(kind, "bool")) snprintf(base, sizeof(base), "bool_t");
    else if (!strcmp(kind, "string")) snprintf(base, sizeof(base), "const char*");
    else if (!strcmp(kind, "nresults")) snprintf(base, sizeof(base), "int");
    else if (!strcmp(kind, "enum")) snprintf(base, sizeof(base), "enum %s", type);
    else if (!strcmp(kind, "struct") || !strcmp(kind, "interface") ||
             !strcmp(kind, "component") || !strcmp(kind, "external_struct"))
        snprintf(base, sizeof(base), "struct %s", type);
    else if (!strcmp(kind, "void")) snprintf(base, sizeof(base), "void");
    else snprintf(base, sizeof(base), "%s_t", type);

    snprintf(dst, dsz, "%s%s%s", base,
            (flags & CG_FLAG_CONST) ? " const" : "",
            ((flags & CG_FLAG_POINTER) || (flags & CG_FLAG_ARRAY)) ? "*" : "");
    return 0;
}

static int type_decl_slot(char *dst, size_t dsz, cg_model const *m,
                          char const *type, uint32_t flags) {
    char base[256];
    if (type_decl(base, sizeof(base), m, type, flags) < 0) return -1;
    snprintf(dst, dsz, "%s*", base);
    return 0;
}

static cg_node const *find_kind_node(cg_model const *m, char const *name, cg_kind kind) {
    size_t i;
    if (!name || !name[0]) return NULL;
    for (i = 0; i < m->node_count; ++i)
        if (m->nodes[i].kind == kind && !strcmp(m->nodes[i].name, name)) return &m->nodes[i];
    return NULL;
}

static int emit_inherited_slot_field(ob *b, cg_model const *m,
                                     char const *type, uint32_t flags,
                                     char const *name, char const *doc) {
    char t[256];
    if (type_decl_slot(t, sizeof(t), m, type, flags) < 0) return -1;
    if (ob_printf(b, "\t%s %s;", t, name) < 0) return -1;
    if (doc && doc[0]) {
        if (ob_printf(b, " ///< %s [inherited].", doc) < 0) return -1;
    } else if (ob_printf(b, " ///< [inherited].") < 0) {
        return -1;
    }
    return ob_printf(b, "\n");
}

#define INHERITED_MAX_DEPTH 16

static int is_expandable_inherited_struct(cg_model const *m, char const *type) {
    cg_node const *st = find_kind_node(m, type, CG_KIND_STRUCT);
    return st && !(st->flags & CG_FLAG_SEALED);
}

static int emit_tabs(ob *b, int count) {
    int i;
    for (i = 0; i < count; ++i)
        if (ob_printf(b, "\t") < 0) return -1;
    return 0;
}

static int emit_inherited_slot_member(ob *b, cg_model const *m,
                                      char const *type, uint32_t flags,
                                      char const *name, char const *doc,
                                      int indent) {
    char t[256];
    if (type_decl_slot(t, sizeof(t), m, type, flags) < 0) return -1;
    if (emit_tabs(b, indent) < 0) return -1;
    if (ob_printf(b, "%s %s;", t, name) < 0) return -1;
    if (doc && doc[0]) {
        if (ob_printf(b, " ///< %s [inherited].", doc) < 0) return -1;
    } else if (ob_printf(b, " ///< [inherited].") < 0) {
        return -1;
    }
    return ob_printf(b, "\n");
}

static int emit_inherited_slot_members(ob *b, cg_model const *m,
                                       char const *type, int indent) {
    cg_node const *st = find_kind_node(m, type, CG_KIND_STRUCT);
    if (!st || (st->flags & CG_FLAG_SEALED))
        return 0;
    cg_foreach(m, st->id, CG_KIND_FIELD, f) {
        int fixed = (f->extra && f->extra[0]) ? atoi(f->extra) : 0;
        if (fixed > 0) continue;
        if (is_expandable_inherited_struct(m, f->type)) {
            if (emit_tabs(b, indent) < 0) return -1;
            if (ob_printf(b, "struct {\n") < 0) return -1;
            if (emit_inherited_slot_members(b, m, f->type, indent + 1) < 0)
                return -1;
            if (emit_tabs(b, indent) < 0) return -1;
            if (ob_printf(b, "} %s;", f->name) < 0) return -1;
            if (f->doc && f->doc[0])
                if (ob_printf(b, " ///< %s", f->doc) < 0) return -1;
            if (ob_printf(b, "\n") < 0) return -1;
        } else if (emit_inherited_slot_member(b, m, f->type, f->flags,
                                              f->name, f->doc, indent) < 0) {
            return -1;
        }
    }
    return 0;
}

static cg_node const *method_returns(cg_model const *m, cg_node const *method) {
    return cg_first(m, method->id, CG_KIND_RETURNS);
}

static int method_return_decl(char *dst, size_t dsz, cg_model const *m, cg_node const *method) {
    cg_node const *ret = method_returns(m, method);
    if (!ret) {
        snprintf(dst, dsz, "void");
        return 0;
    }
    return type_decl(dst, dsz, m, ret->type, ret->flags);
}

static int emit_doc(ob *b, cg_node const *n, int blank_before) {
    if (!n->doc || !n->doc[0]) return 0;
    return ob_printf(b, "%s/// @brief %s\n", blank_before ? "\n" : "", n->doc);
}

static int emit_enum(ob *b, cg_model const *m, cg_node const *e) {
    uint32_t count = 0;
    if (emit_doc(b, e, 1) < 0) return -1;
    if (ob_printf(b, "/** %s enum */\n", e->name) < 0) return -1;
    if (ob_printf(b, "typedef enum %s {\n", e->name) < 0) return -1;
    cg_foreach(m, e->id, CG_KIND_VALUE, v) {
        if (ob_printf(b, "\tk%s%s, ///< %s\n", e->name, v->name,
                v->type ? v->type : "") < 0) return -1;
        ++count;
    }
    return ob_printf(b,
            "} e%s_t;\n"
            "#define %s_Count %u\n"
            "ORCA_API const char *%sToString(enum %s value);\n"
            "ORCA_API enum %s luaX_check%s(lua_State *L, int idx);\n"
            "ORCA_API void luaX_push%s(lua_State *L, enum %s value);\n",
            e->name, e->name, count,
            e->name, e->name, e->name, e->name, e->name, e->name);
}

static int event_has_own_fields(cg_model const *m, cg_node const *msg) {
    return cg_first(m, msg->id, CG_KIND_FIELD) != NULL;
}

static cg_node const *find_event_owner(cg_model const *m, cg_node const *msg) {
    return cg_node_by_id(m, msg->parent);
}

static int is_local_owner(cg_node const *owner) {
    return owner && owner->parent == 0;
}

static cg_node const *find_event_by_name(cg_model const *m, cg_node const *owner,
                                         char const *name) {
    if (!name || !name[0]) return NULL;
    if (owner) {
        cg_foreach(m, owner->id, CG_KIND_MESSAGE, msg)
            if (!strcmp(msg->name, name)) return msg;
    }
    for (size_t i = 0; i < m->node_count; ++i)
        if (m->nodes[i].kind == CG_KIND_MESSAGE && !strcmp(m->nodes[i].name, name))
            return &m->nodes[i];
    return NULL;
}

static cg_node const *event_parent(cg_model const *m, cg_node const *msg) {
    return (msg->extra && msg->extra[0])
        ? find_event_by_name(m, find_event_owner(m, msg), msg->extra)
        : NULL;
}

static int event_has_any_fields(cg_model const *m, cg_node const *msg) {
    cg_node const *p;
    if (event_has_own_fields(m, msg)) return 1;
    p = event_parent(m, msg);
    return p ? event_has_any_fields(m, p) : 0;
}

static void event_struct_name(char *dst, size_t dsz, cg_model const *m, cg_node const *msg) {
    cg_node const *owner = find_event_owner(m, msg);
    snprintf(dst, dsz, "%s_%sEventArgs", owner ? owner->name : "", msg->name);
}

static void message_action_name(char *dst, size_t dsz, cg_model const *m, cg_node const *msg) {
    cg_node const *owner = find_event_owner(m, msg);
    snprintf(dst, dsz, "%s_%sAction", owner ? owner->name : "", msg->name);
}

static void event_effective_struct_name(char *dst, size_t dsz, cg_model const *m, cg_node const *msg) {
    cg_node const *p;
    if (event_has_own_fields(m, msg)) {
        event_struct_name(dst, dsz, m, msg);
        return;
    }
    p = event_parent(m, msg);
    if (p) {
        event_effective_struct_name(dst, dsz, m, p);
        return;
    }
    dst[0] = '\0';
}

static void event_effective_type_decl(char *dst, size_t dsz, cg_model const *m, cg_node const *msg) {
    cg_node const *p;
    if (event_has_own_fields(m, msg)) {
        char s[EVENT_STRUCT_NAME_BUFFER];
        event_struct_name(s, sizeof(s), m, msg);
        snprintf(dst, dsz, "struct %s", s);
        return;
    }
    p = event_parent(m, msg);
    if (p) {
        event_effective_type_decl(dst, dsz, m, p);
        return;
    }
    if (!msg->type || !msg->type[0] || !strcmp(msg->type, "void")) {
        char s[EVENT_STRUCT_NAME_BUFFER];
        event_struct_name(s, sizeof(s), m, msg);
        snprintf(dst, dsz, "struct %s", s);
        return;
    }
    type_decl(dst, dsz, m, msg->type, msg->flags);
}

static int emit_event_typedefs(ob *b, cg_model const *m) {
    size_t i;
    for (i = 0; i < m->node_count; ++i) {
        cg_node const *msg = &m->nodes[i];
        cg_node const *owner;
        if (msg->kind != CG_KIND_MESSAGE) continue;
        owner = find_event_owner(m, msg);
        if (!is_local_owner(owner)) continue;
        if (!owner || !(owner->kind == CG_KIND_INTERFACE || owner->kind == CG_KIND_CLASS)) continue;
        if (event_has_any_fields(m, msg)) {
            char s[256];
            event_effective_struct_name(s, sizeof(s), m, msg);
            if (ob_printf(b, "typedef struct %s %s_%sMsg_t,* %s_%sMsgPtr;\n",
                    s, owner->name, msg->name, owner->name, msg->name) < 0) return -1;
        } else {
            char t[256];
            event_effective_type_decl(t, sizeof(t), m, msg);
            if (ob_printf(b, "typedef %s %s_%sMsg_t,* %s_%sMsgPtr;\n",
                    t, owner->name, msg->name, owner->name, msg->name) < 0) return -1;
        }
    }
    return 0;
}

static int emit_args(ob *b, cg_model const *m, cg_node const *method,
                     cg_node const *owner, int component_method) {
    int emitted = 0;
    if (method->flags & CG_FLAG_LUA) {
        if (ob_printf(b, "%sstruct lua_State*", emitted ? ", " : "") < 0) return -1;
        emitted = 1;
    }
    if (owner && !(method->flags & CG_FLAG_STATIC) && !component_method) {
        uint32_t flags = CG_FLAG_POINTER | ((method->flags & CG_FLAG_CONST) ? CG_FLAG_CONST : 0);
        char t[256];
        type_decl(t, sizeof(t), m, owner->name, flags);
        if (ob_printf(b, "%s%s", emitted ? ", " : "", t) < 0) return -1;
        emitted = 1;
    }
    cg_foreach(m, method->id, CG_KIND_ARG, arg) {
        char t[256];
        type_decl(t, sizeof(t), m, arg->type, arg->flags);
        if (ob_printf(b, "%s%s", emitted ? ", " : "", t) < 0) return -1;
        emitted = 1;
    }
    if (!emitted) return ob_printf(b, "void");
    return 0;
}

static int emit_method_proto(ob *b, cg_model const *m, cg_node const *method,
                             cg_node const *owner, char const *prefix,
                             int component_method, int blank_doc) {
    char ret[256];
    if (emit_doc(b, method, blank_doc) < 0) return -1;
    method_return_decl(ret, sizeof(ret), m, method);
    if (ob_printf(b, "ORCA_API %s\n%s%s(", ret, prefix ? prefix : "", method->name) < 0) return -1;
    if (emit_args(b, m, method, owner, component_method) < 0) return -1;
    return ob_printf(b, ");\n");
}

static int emit_function_proto(ob *b, cg_model const *m, cg_node const *fn) {
    return emit_method_proto(b, m, fn, NULL, m->prefix ? m->prefix : "", 0, 1);
}

static int emit_struct_contents(ob *b, cg_model const *m, uint32_t owner_id,
                                cg_kind kind, int include_num_arrays) {
    cg_foreach(m, owner_id, kind, f) {
        char t[256];
        int fixed = (f->extra && f->extra[0]) ? atoi(f->extra) : 0;
        type_decl(t, sizeof(t), m, f->type, f->flags);
        if (ob_printf(b, "\t%s %s", t, f->name) < 0) return -1;
        if (fixed > 0) {
            if (ob_printf(b, "[%d];", fixed) < 0) return -1;
        } else if (ob_printf(b, ";") < 0) return -1;
        if (f->doc && f->doc[0])
            if (ob_printf(b, " ///< %s", f->doc) < 0) return -1;
        if (ob_printf(b, "\n") < 0) return -1;
        if (include_num_arrays && (f->flags & CG_FLAG_ARRAY)) {
            if (ob_printf(b, "\tint32_t Num%s;\n", f->name) < 0) return -1;
        }
    }
    return 0;
}

static int emit_struct_def(ob *b, cg_model const *m, cg_node const *s) {
    if (emit_doc(b, s, 0) < 0) return -1;
    if (ob_printf(b, "/** %s struct */\nstruct %s {\n", s->name, s->name) < 0) return -1;
    if (emit_struct_contents(b, m, s->id, CG_KIND_FIELD, 0) < 0) return -1;
    return ob_printf(b,
            "};\n"
            "ORCA_API void luaX_push%s(lua_State *L, struct %s const* %s);\n"
            "ORCA_API struct %s* luaX_check%s(lua_State *L, int idx);\n",
            s->name, s->name, s->name, s->name, s->name);
}

static int emit_event_fields(ob *b, cg_model const *m, cg_node const *msg) {
    cg_node const *p = event_parent(m, msg);
    if (p && emit_event_fields(b, m, p) < 0) return -1;
    return emit_struct_contents(b, m, msg->id, CG_KIND_FIELD, 0);
}

static int emit_event_struct(ob *b, cg_model const *m, cg_node const *msg) {
    char s[256];
    if (event_parent(m, msg)) return 0;
    event_struct_name(s, sizeof(s), m, msg);
    if (ob_printf(b, "/** %s struct */\nstruct %s {\n", s, s) < 0) return -1;
    if (emit_event_fields(b, m, msg) < 0) return -1;
    return ob_printf(b,
            "};\n"
            "ORCA_API void luaX_push%s(lua_State *L, struct %s const* data);\n"
            "ORCA_API struct %s* luaX_check%s(lua_State *L, int idx);\n",
            s, s, s, s);
}

static int emit_message_action_struct(ob *b, cg_model const *m, cg_node const *msg) {
    char s[256];
    message_action_name(s, sizeof(s), m, msg);
    if (ob_printf(b, "/** %s generated message action */\nstruct %s {\n", s, s) < 0) return -1;
    if (emit_event_fields(b, m, msg) < 0) return -1;
    if (ob_printf(b,
            "};\n"
            "ORCA_API void luaX_push%s(lua_State *L, struct %s const* %s);\n"
            "ORCA_API struct %s* luaX_check%s(lua_State *L, int idx);\n",
            s, s, s, s, s) < 0) return -1;
    return 0;
}

static cg_node const *find_mixin(cg_model const *m, char const *name) {
    size_t i;
    if (!name || !name[0]) return NULL;
    for (i = 0; i < m->node_count; ++i)
        if (m->nodes[i].kind == CG_KIND_MIXIN && !strcmp(m->nodes[i].name, name))
            return &m->nodes[i];
    return NULL;
}

static int emit_component_props(ob *b, cg_model const *m, uint32_t owner_id) {
    cg_foreach(m, owner_id, CG_KIND_PROPERTY, p) {
        char t[256];
        if (p->flags & CG_FLAG_INHERITED) {
            if (is_expandable_inherited_struct(m, p->type)) {
                if (ob_printf(b, "\tstruct {\n") < 0) return -1;
                if (emit_inherited_slot_members(b, m, p->type, 2) < 0) return -1;
                if (ob_printf(b, "\t} %s;", p->name) < 0) return -1;
                if (p->doc && p->doc[0])
                    if (ob_printf(b, " ///< %s", p->doc) < 0) return -1;
                if (ob_printf(b, "\n") < 0) return -1;
            } else {
                if (emit_inherited_slot_field(b, m, p->type, p->flags, p->name, p->doc) < 0)
                    return -1;
            }
        } else {
            type_decl(t, sizeof(t), m, p->type, p->flags);
            if (ob_printf(b, "\t%s %s;", t, p->name) < 0) return -1;
            if (p->doc && p->doc[0])
                if (ob_printf(b, " ///< %s", p->doc) < 0) return -1;
            if (ob_printf(b, "\n") < 0) return -1;
        }
        if (p->flags & CG_FLAG_ARRAY)
            if (ob_printf(b, "\tint32_t Num%s;\n", p->name) < 0) return -1;
    }
    return 0;
}

static int emit_component_def(ob *b, cg_model const *m, cg_node const *c) {
    cg_node const *mx = find_mixin(m, c->aux);
    if (emit_doc(b, c, 1) < 0) return -1;
    if (ob_printf(b, "/** %s component */\nstruct %s {\n", c->name, c->name) < 0) return -1;
    if (mx) {
        if (ob_printf(b, "\t/* --- %s mixin --- */\n", mx->name) < 0) return -1;
        if (emit_component_props(b, m, mx->id) < 0) return -1;
        if (emit_struct_contents(b, m, mx->id, CG_KIND_FIELD, 0) < 0) return -1;
        if (ob_printf(b, "\t/* --- %s fields --- */\n", c->name) < 0) return -1;
    }
    if (emit_component_props(b, m, c->id) < 0) return -1;
    if (emit_struct_contents(b, m, c->id, CG_KIND_FIELD, 0) < 0) return -1;
    cg_foreach(m, c->id, CG_KIND_MESSAGE, msg)
        if (ob_printf(b, "\tevent_t %s;\n", msg->name) < 0) return -1;
    if (ob_printf(b,
            "};\n"
            "ORCA_API void luaX_push%s(lua_State *L, struct %s const* %s);\n"
            "ORCA_API struct %s* luaX_check%s(lua_State *L, int idx);\n",
            c->name, c->name, c->name, c->name, c->name) < 0) return -1;
    cg_foreach(m, c->id, CG_KIND_METHOD, method) {
        char prefix[256];
        snprintf(prefix, sizeof(prefix), "%s_", c->name);
        if (emit_method_proto(b, m, method, c, prefix, 0, 1) < 0) return -1;
    }
    return 0;
}

static int emit_header(cg_host_v1 const *host, cg_model const *model, char const *output) {
    ob b = {0};
    char *guard = upper(model->module_name && model->module_name[0] ? model->module_name : "module");
    (void)host;
    if (!guard) return -1;

    if (ob_printf(&b,
            "// Auto-generated from %s by tools/codegen/plugins/header.c\n"
            "// DO NOT EDIT — run 'cd tools && make' to regenerate.\n"
            "#ifndef __%s_H__\n"
            "#define __%s_H__\n\n"
            "#include <stdbool.h>\n"
            "#include <stdint.h>\n\n"
            "typedef struct lua_State lua_State;\n\n",
            base_name(model->xml_path), guard, guard) < 0) goto fail;

    cg_foreach(model, 0, CG_KIND_EXTERNAL, ext)
        if (ob_printf(&b, "struct %s;\n", ext->name) < 0) goto fail;
    if (ob_printf(&b, "\n") < 0) goto fail;

    cg_foreach(model, 0, CG_KIND_INCLUDE, inc)
        if (ob_printf(&b, "#include <%s>\n", inc->name) < 0) goto fail;

    if (ob_printf(&b, "\n#include <%s/%s_properties.h>\n", model->module_name, model->module_name) < 0) goto fail;
    cg_foreach(model, 0, CG_KIND_REQUIRE, req) {
        char dep[256];
        module_name_from_path(req->name, dep, sizeof(dep));
        if (ob_printf(&b, "#include <%s/%s.h>\n", dep, dep) < 0) goto fail;
    }
    if (strcmp(model->module_name, "core")) {
        int has_messages = 0;
        for (size_t i = 0; i < model->node_count; ++i) {
            cg_node const *msg = &model->nodes[i];
            cg_node const *owner = msg->kind == CG_KIND_MESSAGE ? find_event_owner(model, msg) : NULL;
            if (is_local_owner(owner) && (owner->kind == CG_KIND_INTERFACE || owner->kind == CG_KIND_CLASS)) {
                has_messages = 1;
                break;
            }
        }
        if (has_messages && ob_printf(&b, "#include <core/core.h>\n") < 0) goto fail;
    }

    if (ob_printf(&b, "\n") < 0) goto fail;
    if (emit_event_typedefs(&b, model) < 0) goto fail;
    if (ob_printf(&b, "\n") < 0) goto fail;

    cg_foreach(model, 0, CG_KIND_ENUM, e) {
        if (emit_enum(&b, model, e) < 0) goto fail;
    }
    if (ob_printf(&b, "\n") < 0) goto fail;
    if (!cg_first(model, 0, CG_KIND_METHOD)) {
        if (ob_printf(&b, "\n") < 0) goto fail;
    }

    cg_foreach(model, 0, CG_KIND_METHOD, fn) {
        if (emit_function_proto(&b, model, fn) < 0) goto fail;
    }
    if (cg_first(model, 0, CG_KIND_METHOD)) {
        if (ob_printf(&b, "\n") < 0) goto fail;
    }

    cg_foreach(model, 0, CG_KIND_INTERFACE, iface) {
        char const *prefix = iface->type && iface->type[0] ? iface->type : iface->name;
        cg_foreach(model, iface->id, CG_KIND_TOPIC, topic) {
            if (ob_printf(&b, "\n/// @name %s\n", topic->name) < 0) goto fail;
            if (topic->doc && topic->doc[0])
                if (ob_printf(&b, "/// %s\n", topic->doc) < 0) goto fail;
            for (cg_node const *n = topic + 1; n < model->nodes + model->node_count; ++n) {
                if (n->parent != iface->id) continue;
                if (n->kind == CG_KIND_TOPIC) break;
                if (n->kind == CG_KIND_METHOD)
                    if (emit_method_proto(&b, model, n, iface, prefix, 0, 1) < 0) goto fail;
            }
        }
        if (!cg_first(model, iface->id, CG_KIND_TOPIC)) {
            cg_foreach(model, iface->id, CG_KIND_METHOD, method)
                if (emit_method_proto(&b, model, method, iface, prefix, 0, 1) < 0) goto fail;
        }
    }
    if (ob_printf(&b, "\n") < 0) goto fail;

    cg_foreach(model, 0, CG_KIND_STRUCT, s) {
        if (emit_struct_def(&b, model, s) < 0) goto fail;
    }

    {
        int has_struct_methods = 0;
        cg_foreach(model, 0, CG_KIND_STRUCT, s) {
            if (cg_first(model, s->id, CG_KIND_METHOD)) has_struct_methods = 1;
        }
        if (has_struct_methods && ob_printf(&b, "\n") < 0) goto fail;
    }

    cg_foreach(model, 0, CG_KIND_STRUCT, s) {
        char const *prefix = s->type && s->type[0] ? s->type : "";
        cg_foreach(model, s->id, CG_KIND_METHOD, method)
            if (emit_method_proto(&b, model, method, s, prefix, 0, 0) < 0) goto fail;
    }
    if (ob_printf(&b, "\n\n") < 0) goto fail;

    {
        int emitted_events = 0;
        for (size_t i = 0; i < model->node_count; ++i)
            if (model->nodes[i].kind == CG_KIND_MESSAGE && is_local_owner(find_event_owner(model, &model->nodes[i]))) {
                emitted_events = 1;
                if (emit_event_struct(&b, model, &model->nodes[i]) < 0) goto fail;
            }
        if (emitted_events && ob_printf(&b, "\n") < 0) goto fail;
        if (!emitted_events && cg_first(model, 0, CG_KIND_CLASS))
            if (ob_printf(&b, "\n") < 0) goto fail;
    }

    cg_foreach(model, 0, CG_KIND_CLASS, c) {
        if (emit_component_def(&b, model, c) < 0) goto fail;
    }

    {
        int emitted_actions = 0;
        for (size_t i = 0; i < model->node_count; ++i) {
            cg_node const *msg = &model->nodes[i];
            if (msg->kind != CG_KIND_MESSAGE || !is_local_owner(find_event_owner(model, msg))) continue;
            if (!emitted_actions && ob_printf(&b, "\n") < 0) goto fail;
            emitted_actions = 1;
            if (emit_message_action_struct(&b, model, msg) < 0) goto fail;
        }
    }

    /* Storage-family block — emit struct <StorageStruct> and Get* accessors */
    if (model->storage_struct && model->storage_struct[0]) {
        const char *ss = model->storage_struct;
        if (ob_printf(&b,
            "\n/*\n"
            " * Monolithic storage block for all %s objects.\n"
            " * Every object in this module has one of these embedded at the end of its\n"
            " * Object allocation. Fields are ordered base-first so offsets remain stable.\n"
            " */\n"
            "struct %s {\n", model->module_name, ss) < 0) goto fail;
        cg_foreach(model, 0, CG_KIND_CLASS, c) {
            if (ob_printf(&b, "  struct %-24s %s;\n", c->name, c->name) < 0) goto fail;
        }
        if (ob_printf(&b, "};\n") < 0) goto fail;

        /* Get* accessor macros */
        if (ob_printf(&b,
            "\n"
            "#define _%s(_P, FIELD) \\\n"
            "  ((_P) && OBJ_IsKindOfW((_P)->type, ID_%s) \\\n"
            "    ? &((struct %s *)(_P)->typedata)->FIELD \\\n"
            "    : NULL)\n\n",
            ss, model->module_name, ss) < 0) goto fail;
        cg_foreach(model, 0, CG_KIND_CLASS, c) {
            if (ob_printf(&b,
                "#undef  Get%s\n"
                "#define Get%s(_P) _%s(_P, %s)\n",
                c->name, c->name, ss, c->name) < 0) goto fail;
        }

        if (ob_printf(&b,
            "\nstruct %s *Object_%s(struct Object *object);\n",
            ss, ss) < 0) goto fail;
    }

    if (ob_printf(&b, "\n#endif\n") < 0) goto fail;

    free(guard);
    if (host->write_file(output, b.s, b.n) < 0) {
        host->logf("header: write failed");
        free(b.s);
        return -1;
    }
    free(b.s);
    return 0;

fail:
    free(guard);
    free(b.s);
    return -1;
}

static cg_plugin_v1 const plugin = { CG_API_VERSION, "header", emit_header };
cg_plugin_v1 const *cg_plugin_init_v1(void) { return &plugin; }
