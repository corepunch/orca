#include "cg_api.h"

#include <ctype.h>
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

static void ucfirst_into(char *dst, size_t dsz, char const *s) {
    if (!s || !*s || dsz < 2u) { if (dsz) dst[0] = 0; return; }
    dst[0] = (char)toupper((unsigned char)s[0]);
    snprintf(dst + 1u, dsz - 1u, "%s", s + 1);
}

static void strip_brackets_dots(char *dst, size_t dsz, char const *src) {
    size_t i = 0;
    for (; *src && i + 1u < dsz; ++src) {
        if (*src != '.' && *src != '[' && *src != ']') dst[i++] = *src;
    }
    dst[i] = 0;
}

static char const *k_ax[3] = {"Horizontal", "Vertical", "Depth"};
static char const *k_left[3] = {"Left", "Top", "Front"};
static char const *k_right[3] = {"Right", "Bottom", "Back"};
static char const *k_size_sub[5] = {"Requested", "Desired", "Min", "Actual", "Scroll"};
static char const *k_size_names[3][5] = {
    {"Width", "DesiredWidth", "MinWidth", "ActualWidth", "ScrollWidth"},
    {"Height", "DesiredHeight", "MinHeight", "ActualHeight", "ScrollHeight"},
    {"Depth", "DesiredDepth", "MinDepth", "ActualDepth", "ScrollDepth"},
};

static void axis_transform(char const *path, char *out, size_t out_sz) {
    char result[512];
    int n;
    for (n = 0; n < 3; ++n) {
        char tok[64];
        snprintf(tok, sizeof(tok), "Size.Axis[%d].", n);
        if (!strncmp(path, tok, strlen(tok))) {
            char const *sub = path + strlen(tok);
            int k;
            if (!strncmp(sub, "Requested", 9)) {
                snprintf(out, out_sz, "%s%s", k_size_names[n][0], sub + 9);
                return;
            }
            for (k = 1; k < 5; ++k) {
                if (!strcmp(sub, k_size_sub[k])) {
                    snprintf(out, out_sz, "%s", k_size_names[n][k]);
                    return;
                }
            }
        }
    }
    for (n = 0; n < 3; ++n) {
        char lt[64], rt[64], *pos;
        snprintf(lt, sizeof(lt), ".Axis[%d].Left", n);
        snprintf(rt, sizeof(rt), ".Axis[%d].Right", n);
        if ((pos = strstr((char *)path, lt))) {
            size_t pl = (size_t)(pos - path);
            char prefix[256];
            memcpy(prefix, path, pl); prefix[pl] = 0;
            snprintf(result, sizeof(result), "%s%s%s", prefix, k_left[n], pos + strlen(lt));
            strip_brackets_dots(out, out_sz, result);
            return;
        }
        if ((pos = strstr((char *)path, rt))) {
            size_t pl = (size_t)(pos - path);
            char prefix[256];
            memcpy(prefix, path, pl); prefix[pl] = 0;
            snprintf(result, sizeof(result), "%s%s%s", prefix, k_right[n], pos + strlen(rt));
            strip_brackets_dots(out, out_sz, result);
            return;
        }
    }
    for (n = 0; n < 3; ++n) {
        char tok[32];
        size_t plen, tlen;
        snprintf(tok, sizeof(tok), ".Axis[%d]", n);
        plen = strlen(path); tlen = strlen(tok);
        if (plen > tlen && !strcmp(path + plen - tlen, tok)) {
            char prefix[256];
            memcpy(prefix, path, plen - tlen); prefix[plen - tlen] = 0;
            snprintf(result, sizeof(result), "%s%s", k_ax[n], prefix);
            strip_brackets_dots(out, out_sz, result);
            return;
        }
    }
    if (!strncmp(path, "Border.Radius.", 14)) {
        char const *tail = path + 14;
        size_t len = strlen(tail);
        if (len > 6u && !strcmp(tail + len - 6u, "Radius")) {
            snprintf(result, sizeof(result), "Border%.*sRadius", (int)(len - 6u), tail);
            strip_brackets_dots(out, out_sz, result);
            return;
        }
    }
    for (n = 0; n < 3; ++n) {
        char lt[64], rt[64];
        snprintf(lt, sizeof(lt), "Axis[%d].Left", n);
        snprintf(rt, sizeof(rt), "Axis[%d].Right", n);
        if (!strncmp(path, lt, strlen(lt))) {
            snprintf(result, sizeof(result), "%s%s", k_left[n], path + strlen(lt));
            strip_brackets_dots(out, out_sz, result);
            return;
        }
        if (!strncmp(path, rt, strlen(rt))) {
            snprintf(result, sizeof(result), "%s%s", k_right[n], path + strlen(rt));
            strip_brackets_dots(out, out_sz, result);
            return;
        }
    }
    for (n = 0; n < 3; ++n) {
        char tok[32], *pos;
        snprintf(tok, sizeof(tok), "Axis[%d]", n);
        if ((pos = strstr((char *)path, tok))) {
            size_t pl = (size_t)(pos - path);
            char prefix[256];
            memcpy(prefix, path, pl); prefix[pl] = 0;
            snprintf(result, sizeof(result), "%s%s%s", prefix, k_ax[n], pos + strlen(tok));
            strip_brackets_dots(out, out_sz, result);
            return;
        }
    }
    strip_brackets_dots(out, out_sz, path);
}

static cg_node const *node_parent(cg_model const *m, cg_node const *n) {
    return n ? cg_node_by_id(m, n->parent) : NULL;
}

static int local_node(cg_node const *n) {
    return n && n->parent == 0;
}

static cg_node const *find_kind(cg_model const *m, char const *name, cg_kind kind) {
    size_t i;
    if (!name || !name[0]) return NULL;
    for (i = 0; i < m->node_count; ++i)
        if (m->nodes[i].kind == kind && !strcmp(m->nodes[i].name, name)) return &m->nodes[i];
    return NULL;
}

/* Walk a comma-separated parent list and return the first that is a local class. */
static cg_node const *find_local_parent(cg_model const *m, char const *parent_list) {
    char buf[512]; char *tok;
    if (!parent_list || !parent_list[0]) return NULL;
    snprintf(buf, sizeof(buf), "%s", parent_list);
    for (tok = strtok(buf, ","); tok; tok = strtok(NULL, ",")) {
        while (*tok == ' ') ++tok;
        cg_node const *p = find_kind(m, tok, CG_KIND_CLASS);
        if (p) return p;
    }
    return NULL;
}

/* Walk the parent list of c and return the first that has external-storage set. */
static cg_node const *find_external_storage_ancestor(cg_model const *m, cg_node const *c) {
    int depth = 0;
    while (c && depth++ < 64) {
        if (c->extra && c->extra[0]) return c; /* has external-storage attr */
        c = find_local_parent(m, c->type);
    }
    return NULL;
}

/* Compute the storage struct name for class c into dst.
 * Returns dst if c has a storage root, else returns NULL (no typedata). */
static char const *class_storage_name(cg_model const *m, cg_node const *c,
                                      char *dst, size_t dsz) {
    cg_node const *ext = find_external_storage_ancestor(m, c);
    if (ext && ext->extra && ext->extra[0]) {
        snprintf(dst, dsz, "%s", ext->extra);
        return dst;
    }
    return NULL;
}

static char const *type_kind(cg_model const *m, char const *type) {
    if (!type || !type[0]) return "void";
    if (!strcmp(type, "float")) return "float";
    if (!strcmp(type, "int")) return "int";
    if (!strcmp(type, "uint")) return "uint";
    if (!strcmp(type, "long")) return "long";
    if (!strcmp(type, "bool") || !strcmp(type, "bool_t")) return "bool";
    if (!strcmp(type, "string") || !strcmp(type, "char*")) return "string";
    if (!strcmp(type, "nresults")) return "nresults";
    if (!strcmp(type, "void")) return "void";
    if (find_kind(m, type, CG_KIND_ENUM)) return "enum";
    if (find_kind(m, type, CG_KIND_STRUCT)) return "struct";
    if (find_kind(m, type, CG_KIND_CLASS)) return "component";
    if (find_kind(m, type, CG_KIND_INTERFACE)) return "interface";
    if (find_kind(m, type, CG_KIND_EXTERNAL)) return "external_struct";
    return type;
}

#define TYPE_DECL_BASE_BUFFER 248 /* leaves room for optional " const*" suffix in 256-byte dst */

static int type_decl(char *dst, size_t dsz, cg_model const *m, char const *type, uint32_t flags) {
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

static void type_check(char *dst, size_t dsz, cg_model const *m,
                       char const *type, uint32_t flags, int idx) {
    char const *kind = type_kind(m, type);
    (void)flags;
    if (!strcmp(kind, "float")) snprintf(dst, dsz, "luaL_checknumber(L, %d)", idx);
    else if (!strcmp(kind, "int")) snprintf(dst, dsz, "(int32_t)luaL_checkinteger(L, %d)", idx);
    else if (!strcmp(kind, "uint")) snprintf(dst, dsz, "(uint32_t)luaL_checkinteger(L, %d)", idx);
    else if (!strcmp(kind, "long")) snprintf(dst, dsz, "luaL_checkinteger(L, %d)", idx);
    else if (!strcmp(kind, "bool")) snprintf(dst, dsz, "lua_toboolean(L, %d)", idx);
    else if (!strcmp(kind, "string")) snprintf(dst, dsz, "luaL_checkstring(L, %d)", idx);
    else if (!strcmp(kind, "external_struct")) snprintf(dst, dsz, "NULL");
    else snprintf(dst, dsz, "luaX_check%s(L, %d)", type, idx);
}

static void type_push(char *dst, size_t dsz, cg_model const *m,
                      char const *type, uint32_t flags, char const *arg) {
    char const *kind = type_kind(m, type);
    if (!strcmp(kind, "float")) snprintf(dst, dsz, "lua_pushnumber(L, %s)", arg);
    else if (!strcmp(kind, "int") || !strcmp(kind, "uint") || !strcmp(kind, "long"))
        snprintf(dst, dsz, "lua_pushinteger(L, %s)", arg);
    else if (!strcmp(kind, "bool")) snprintf(dst, dsz, "lua_pushboolean(L, %s)", arg);
    else if (!strcmp(kind, "string")) snprintf(dst, dsz, "lua_pushstring(L, %s)", arg);
    else if (!strcmp(kind, "struct")) {
        if (flags & CG_FLAG_POINTER) snprintf(dst, dsz, "luaX_push%s(L, &*%s)", type, arg);
        else snprintf(dst, dsz, "luaX_push%s(L, &%s)", type, arg);
    } else {
        snprintf(dst, dsz, "luaX_push%s(L, %s)", type, arg);
    }
}

static cg_node const *method_returns(cg_model const *m, cg_node const *method) {
    return cg_first(m, method->id, CG_KIND_RETURNS);
}

static char const *export_name(cg_node const *n) {
    if (n && n->kind == CG_KIND_CLASS) return n->name;
    return (n->extra && n->extra[0]) ? n->extra : n->name;
}

static char const *method_export_name(cg_node const *n) {
    static char buf[256];
    if (n->extra && n->extra[0]) return n->extra;
    if (!n->name || !n->name[0]) return "";
    snprintf(buf, sizeof(buf), "%s", n->name);
    buf[0] = (char)tolower((unsigned char)buf[0]);
    return buf;
}

static char const *owner_prefix(cg_node const *owner) {
    if (!owner) return "";
    if (owner->type && owner->type[0]) return owner->type;
    if (owner->kind == CG_KIND_INTERFACE || owner->kind == CG_KIND_STRUCT) return owner->name;
    return "";
}

typedef struct {
    char const *name;
    uint32_t id;
    int sealed;
    char const *export_name;
} sentry;

static sentry *build_struct_map(cg_model const *m, size_t *count_out) {
    size_t n = 0, i = 0, mi;
    sentry *e;
    for (mi = 0; mi < m->node_count; ++mi)
        if (m->nodes[mi].kind == CG_KIND_STRUCT) ++n;
    e = (sentry *)calloc(n ? n : 1u, sizeof(*e));
    if (!e) return NULL;
    for (mi = 0; mi < m->node_count; ++mi) {
        cg_node const *s = &m->nodes[mi];
        if (s->kind != CG_KIND_STRUCT) continue;
        e[i].name = s->name;
        e[i].id = s->id;
        e[i].sealed = (s->flags & CG_FLAG_SEALED) != 0;
        e[i].export_name = export_name(s);
        ++i;
    }
    *count_out = n;
    return e;
}

static sentry const *find_struct(sentry const *map, size_t cnt, char const *type) {
    size_t i;
    for (i = 0; i < cnt; ++i)
        if (!strcmp(map[i].name, type)) return &map[i];
    return NULL;
}

static void property_leaf(char segs[][64], int n_segs, char *leaf, size_t leaf_sz) {
    char path[512] = "";
    int i;
    for (i = 0; i < n_segs; ++i) {
        char seg[64];
        size_t pl;
        ucfirst_into(seg, sizeof(seg), segs[i]);
        if (i > 0) {
            pl = strlen(path);
            if (pl + 1u < sizeof(path)) { path[pl] = '.'; path[pl + 1u] = 0; }
        }
        pl = strlen(path);
        snprintf(path + pl, sizeof(path) - pl, "%s", seg);
    }
    axis_transform(path, leaf, leaf_sz);
}

static void property_addr(char segs[][64], int n_segs, char *addr, size_t addr_sz) {
    int i;
    addr[0] = 0;
    for (i = 0; i < n_segs; ++i) {
        size_t pl;
        if (i > 0) {
            pl = strlen(addr);
            if (pl + 1u < addr_sz) { addr[pl] = '.'; addr[pl + 1u] = 0; }
        }
        pl = strlen(addr);
        snprintf(addr + pl, addr_sz - pl, "%s", segs[i]);
    }
}

static char const *property_datatype(cg_model const *m, char const *type) {
    char const *kind = type_kind(m, type);
    if (!strcmp(kind, "component") || !strcmp(kind, "interface")) return "kDataTypeObject";
    if (!strcmp(kind, "struct") && !strcmp(type, "color")) return "kDataTypeColor";
    if (!strcmp(type, "uint")) return "kDataTypeInt";
    if (!strcmp(kind, "external_struct")) return "kDataTypeStruct";
    if (!strcmp(kind, "float")) return "kDataTypeFloat";
    if (!strcmp(kind, "int")) return "kDataTypeInt";
    if (!strcmp(kind, "long")) return "kDataTypeLong";
    if (!strcmp(kind, "bool")) return "kDataTypeBool";
    if (!strcmp(kind, "string")) return "kDataTypeString";
    if (!strcmp(kind, "enum")) return "kDataTypeEnum";
    if (!strcmp(kind, "struct")) return "kDataTypeStruct";
    return "kDataTypeVoid";
}

static int emit_property_row(ob *b, cg_host_v1 const *h, cg_model const *m,
                             char const *owner_name, char segs[][64], int n_segs,
                             char const *type, uint32_t flags, char const *storage) {
    char leaf[256], addr[512];
    char const *kind;
    char actual_type[256];
    uint32_t actual_flags = flags;
    sentry const *s;
    (void)s;
    property_leaf(segs, n_segs, leaf, sizeof(leaf));
    property_addr(segs, n_segs, addr, sizeof(addr));
    snprintf(actual_type, sizeof(actual_type), "%s", type && type[0] ? type : "void");
    if (!strcmp(type_kind(m, actual_type), "int") && find_kind(m, leaf, CG_KIND_ENUM))
        snprintf(actual_type, sizeof(actual_type), "%s", leaf);
    kind = type_kind(m, actual_type);
    if (storage && storage[0]) {
        /* Offsets relative to the storage-family struct (e.g. UIData) */
        const char *macro = (actual_flags & CG_FLAG_ARRAY) ? "UIDATA_ARRAY_DECL" : "UIDATA_DECL";
        if (ob_printf(b, "\t%s(0x%08x, %s, %s, %s, %s",
                macro, h->fnv1a32(leaf), owner_name, leaf, addr,
                property_datatype(m, actual_type)) < 0) return -1;
        if (actual_flags & CG_FLAG_INHERITED) {
            if (ob_printf(b, ", .IsInherited=TRUE") < 0) return -1;
        }
    } else {
        if (ob_printf(b, "\t%s(0x%08x, %s, %s, %s, %s",
                (actual_flags & CG_FLAG_ARRAY) ? "ARRAY_DECL" : "DECL",
                h->fnv1a32(leaf), owner_name, leaf, addr,
                property_datatype(m, actual_type)) < 0) return -1;
        if (actual_flags & CG_FLAG_INHERITED) {
            if (ob_printf(b, ", .IsInherited=TRUE") < 0) return -1;
        }
    }
    if (!strcmp(kind, "enum")) {
        if (ob_printf(b, ", .EnumValues = _%s", actual_type) < 0) return -1;
    } else if (!strcmp(kind, "external_struct")) {
        if (ob_printf(b, ", .TypeString = \"%s\"", actual_type) < 0) return -1;
    } else if (!strcmp(kind, "struct") && strcmp(actual_type, "color")) {
        cg_node const *st = find_kind(m, actual_type, CG_KIND_STRUCT);
        if (ob_printf(b, ", .TypeString = \"%s\"",
                st ? export_name(st) : actual_type) < 0) return -1;
    } else if (!strcmp(kind, "component") || !strcmp(kind, "interface")) {
        cg_node const *n = find_kind(m, actual_type, !strcmp(kind, "component") ? CG_KIND_CLASS : CG_KIND_INTERFACE);
        if (ob_printf(b, ", .TypeString = \"%s\"",
                n ? export_name(n) : actual_type) < 0) return -1;
    }
    if (ob_printf(b, "), // %s.%s\n", owner_name, leaf) < 0) return -1;
    return 0;
}

#define MAX_DEPTH 16

static int emit_walk_property(ob *b, cg_host_v1 const *h, cg_model const *m,
                              sentry const *smap, size_t scount,
                              char const *owner_name, char segs[][64], int n_segs,
                              char const *type, uint32_t flags, char const *storage);

static int emit_fixed_array(ob *b, cg_host_v1 const *h, cg_model const *m,
                            sentry const *smap, size_t scount,
                            char const *owner_name, char segs[][64], int n_segs,
                            char const *field_name, char const *field_type, int fixed,
                            uint32_t flags, char const *storage) {
    int i;
    for (i = 0; i < fixed; ++i) {
        char next[MAX_DEPTH][64];
        memcpy(next, segs, sizeof(next));
        snprintf(next[n_segs], sizeof(next[n_segs]), "%s[%d]", field_name, i);
        if (emit_walk_property(b, h, m, smap, scount, owner_name, next, n_segs + 1,
                field_type, flags & ~(uint32_t)CG_FLAG_ARRAY, storage) < 0) return -1;
    }
    return 0;
}

static int emit_walk_property(ob *b, cg_host_v1 const *h, cg_model const *m,
                              sentry const *smap, size_t scount,
                              char const *owner_name, char segs[][64], int n_segs,
                              char const *type, uint32_t flags, char const *storage) {
    sentry const *s;
    s = find_struct(smap, scount, type);
    if (!(s && !s->sealed && !(flags & CG_FLAG_ARRAY))) {
        if (emit_property_row(b, h, m, owner_name, segs, n_segs, type, flags, storage) < 0) return -1;
    }
    if (s && !s->sealed && n_segs < MAX_DEPTH - 1) {
        cg_foreach(m, s->id, CG_KIND_FIELD, f) {
            char next[MAX_DEPTH][64];
            int fixed = (f->extra && f->extra[0]) ? atoi(f->extra) : 0;
        if (fixed > 0) {
                if (emit_fixed_array(b, h, m, smap, scount, owner_name, segs, n_segs,
                        f->name, f->type, fixed, f->flags | (flags & CG_FLAG_INHERITED), storage) < 0) return -1;
            } else {
                memcpy(next, segs, sizeof(next));
                snprintf(next[n_segs], sizeof(next[n_segs]), "%s", f->name);
                if (emit_walk_property(b, h, m, smap, scount, owner_name, next,
                        n_segs + 1, f->type, f->flags | (flags & CG_FLAG_INHERITED), storage) < 0) return -1;
            }
        }
    }
    return 0;
}

static int emit_properties_for_fields(ob *b, cg_host_v1 const *h, cg_model const *m,
                                      sentry const *smap, size_t scount,
                                      cg_node const *owner, char const *owner_name,
                                      cg_kind kind, int include_array_count,
                                      char const *storage) {
    cg_foreach(m, owner->id, kind, p) {
        char segs[MAX_DEPTH][64] = {{0}};
        int fixed = (kind == CG_KIND_FIELD && p->extra && p->extra[0]) ? atoi(p->extra) : 0;
        if (fixed > 0) {
            if (emit_fixed_array(b, h, m, smap, scount, owner_name, segs, 0,
                    p->name, p->type, fixed, p->flags, storage) < 0) return -1;
        } else {
            snprintf(segs[0], sizeof(segs[0]), "%s", p->name);
            if (emit_walk_property(b, h, m, smap, scount, owner_name, segs, 1,
                    p->type, p->flags, storage) < 0) return -1;
        }
        if (include_array_count && (p->flags & CG_FLAG_ARRAY)) {
            snprintf(segs[0], sizeof(segs[0]), "Num%s", p->name);
            if (emit_property_row(b, h, m, owner_name, segs, 1, "int", 0, storage) < 0) return -1;
        }
    }
    return 0;
}

static cg_node const *event_parent(cg_model const *m, cg_node const *msg);
static void message_action_name(cg_model const *m, cg_node const *msg, char *dst, size_t dsz);

static int emit_getter_case_row(ob *b, char const *owner_name, char segs[][64], int n_segs,
                                uint32_t *idx) {
    char addr[512];
    property_addr(segs, n_segs, addr, sizeof(addr));
    if (ob_printf(b,
            "\t\tcase offsetof(struct %s, %s): return OBJ_GetPropertyAtIndex(_P, ID_%s, sizeof(struct %s), %u);\n",
            owner_name, addr, owner_name, owner_name, *idx) < 0) return -1;
    (*idx)++;
    return 0;
}

static int emit_walk_getter_case(ob *b, cg_model const *m,
                                 sentry const *smap, size_t scount,
                                 char const *owner_name, char segs[][64], int n_segs,
                                 char const *type, uint32_t flags, uint32_t *idx);

static int emit_fixed_getter_case(ob *b, cg_model const *m,
                                  sentry const *smap, size_t scount,
                                  char const *owner_name, char segs[][64], int n_segs,
                                  char const *field_name, char const *field_type, int fixed,
                                  uint32_t flags, uint32_t *idx) {
    int i;
    for (i = 0; i < fixed; ++i) {
        char next[MAX_DEPTH][64];
        memcpy(next, segs, sizeof(next));
        snprintf(next[n_segs], sizeof(next[n_segs]), "%s[%d]", field_name, i);
        if (emit_walk_getter_case(b, m, smap, scount, owner_name, next, n_segs + 1,
                field_type, flags & ~(uint32_t)CG_FLAG_ARRAY, idx) < 0) return -1;
    }
    return 0;
}

static int emit_walk_getter_case(ob *b, cg_model const *m,
                                 sentry const *smap, size_t scount,
                                 char const *owner_name, char segs[][64], int n_segs,
                                 char const *type, uint32_t flags, uint32_t *idx) {
    sentry const *s = find_struct(smap, scount, type);
    if (!(s && !s->sealed && !(flags & CG_FLAG_ARRAY))) {
        return emit_getter_case_row(b, owner_name, segs, n_segs, idx);
    }
    if (n_segs < MAX_DEPTH - 1) {
        cg_foreach(m, s->id, CG_KIND_FIELD, f) {
            char next[MAX_DEPTH][64];
            int fixed = (f->extra && f->extra[0]) ? atoi(f->extra) : 0;
            if (fixed > 0) {
                if (emit_fixed_getter_case(b, m, smap, scount, owner_name, segs, n_segs,
                        f->name, f->type, fixed, f->flags | (flags & CG_FLAG_INHERITED), idx) < 0) return -1;
            } else {
                memcpy(next, segs, sizeof(next));
                snprintf(next[n_segs], sizeof(next[n_segs]), "%s", f->name);
                if (emit_walk_getter_case(b, m, smap, scount, owner_name, next, n_segs + 1,
                        f->type, f->flags | (flags & CG_FLAG_INHERITED), idx) < 0) return -1;
            }
        }
    }
    return 0;
}

static int emit_getter_cases_for_fields(ob *b, cg_model const *m,
                                        sentry const *smap, size_t scount,
                                        cg_node const *owner, char const *owner_name,
                                        cg_kind kind, int include_array_count,
                                        uint32_t *idx) {
    cg_foreach(m, owner->id, kind, p) {
        char segs[MAX_DEPTH][64] = {{0}};
        int fixed = (kind == CG_KIND_FIELD && p->extra && p->extra[0]) ? atoi(p->extra) : 0;
        if (fixed > 0) {
            if (emit_fixed_getter_case(b, m, smap, scount, owner_name, segs, 0,
                    p->name, p->type, fixed, p->flags, idx) < 0) return -1;
        } else {
            snprintf(segs[0], sizeof(segs[0]), "%s", p->name);
            if (emit_walk_getter_case(b, m, smap, scount, owner_name, segs, 1,
                    p->type, p->flags, idx) < 0) return -1;
        }
        if (include_array_count && (p->flags & CG_FLAG_ARRAY)) {
            snprintf(segs[0], sizeof(segs[0]), "Num%s", p->name);
            if (emit_getter_case_row(b, owner_name, segs, 1, idx) < 0) return -1;
        }
    }
    return 0;
}

static int emit_class_getter(ob *b, cg_model const *m,
                             sentry const *smap, size_t scount,
                             cg_node const *c, cg_node const *mx) {
    uint32_t idx = 0;
    if (ob_printf(b,
            "ORCA_API struct Property *%s_GetProperty(struct Object *_P, size_t _O) {\n"
            "\tswitch (_O) {\n",
            c->name) < 0) return -1;
    if (mx && emit_getter_cases_for_fields(b, m, smap, scount, mx, c->name,
            CG_KIND_PROPERTY, 1, &idx) < 0) return -1;
    if (emit_getter_cases_for_fields(b, m, smap, scount, c, c->name,
            CG_KIND_PROPERTY, 1, &idx) < 0) return -1;
    cg_foreach(m, c->id, CG_KIND_MESSAGE, msg) {
        char segs[MAX_DEPTH][64] = {{0}};
        snprintf(segs[0], sizeof(segs[0]), "%s", msg->name);
        if (emit_getter_case_row(b, c->name, segs, 1, &idx) < 0) return -1;
    }
    return ob_printf(b,
            "\t\tdefault: return NULL;\n"
            "\t}\n"
            "}\n");
}

static int emit_message_action_getter_cases(ob *b, cg_model const *m,
                                            cg_node const *msg, char const *action,
                                            uint32_t *idx) {
    cg_node const *p = event_parent(m, msg);
    if (p && emit_message_action_getter_cases(b, m, p, action, idx) < 0) return -1;
    cg_foreach(m, msg->id, CG_KIND_FIELD, f) {
        if (ob_printf(b,
                "\t\tcase offsetof(struct %s, %s): return OBJ_GetPropertyAtIndex(_P, ID_%s, sizeof(struct %s), %u);\n",
                action, f->name, action, action, (*idx)++) < 0) return -1;
    }
    return 0;
}

static int emit_message_action_getter(ob *b, cg_model const *m, cg_node const *msg) {
    char action[256];
    uint32_t idx = 0;
    message_action_name(m, msg, action, sizeof(action));
    if (ob_printf(b,
            "ORCA_API struct Property *%s_GetProperty(struct Object *_P, size_t _O) {\n"
            "\tswitch (_O) {\n",
            action) < 0) return -1;
    if (emit_message_action_getter_cases(b, m, msg, action, &idx) < 0) return -1;
    return ob_printf(b,
            "\t\tdefault: return NULL;\n"
            "\t}\n"
            "}\n");
}


static int emit_shorthand_target_walk(ob *b, cg_host_v1 const *h, cg_model const *m,
                                      sentry const *smap, size_t scount,
                                      char const *owner_name,
                                      char const *shorthand_type,
                                      char full_segs[][64], int n_full,
                                      char field_segs[][64], int n_field,
                                      char const *type,
                                      int *present_bit) {
    sentry const *s = find_struct(smap, scount, type);
    if (s && !s->sealed && n_full < MAX_DEPTH - 1 && n_field < MAX_DEPTH - 1) {
        cg_foreach(m, s->id, CG_KIND_FIELD, f) {
            int fixed = (f->extra && f->extra[0]) ? atoi(f->extra) : 0;
            if (fixed > 0) {
                int i;
                for (i = 0; i < fixed; ++i) {
                    char next_full[MAX_DEPTH][64];
                    char next_field[MAX_DEPTH][64];
                    memcpy(next_full, full_segs, sizeof(next_full));
                    memcpy(next_field, field_segs, sizeof(next_field));
                    snprintf(next_full[n_full], sizeof(next_full[n_full]), "%s[%d]", f->name, i);
                    snprintf(next_field[n_field], sizeof(next_field[n_field]), "%s[%d]", f->name, i);
                    if (emit_shorthand_target_walk(b, h, m, smap, scount,
                            owner_name, shorthand_type,
                            next_full, n_full + 1,
                            next_field, n_field + 1,
                            f->type, present_bit) < 0) return -1;
                }
            } else {
                char next_full[MAX_DEPTH][64];
                char next_field[MAX_DEPTH][64];
                memcpy(next_full, full_segs, sizeof(next_full));
                memcpy(next_field, field_segs, sizeof(next_field));
                snprintf(next_full[n_full], sizeof(next_full[n_full]), "%s", f->name);
                snprintf(next_field[n_field], sizeof(next_field[n_field]), "%s", f->name);
                if (emit_shorthand_target_walk(b, h, m, smap, scount,
                        owner_name, shorthand_type,
                        next_full, n_full + 1,
                        next_field, n_field + 1,
                        f->type, present_bit) < 0) return -1;
            }
        }
        return 0;
    }

    char leaf[256], field[256], addr[512];
    uint64_t bit = *present_bit < 64 ? ((uint64_t)1 << *present_bit) : 0;
    (*present_bit)++;
    property_leaf(full_segs, n_full, leaf, sizeof(leaf));
    property_leaf(field_segs, n_field, field, sizeof(field));
    property_addr(field_segs, n_field, addr, sizeof(addr));
    return ob_printf(b,
            "\tSHORTHAND_TARGET(%s, %s, %s, %s, 0x%llx),\n",
            shorthand_type, addr, owner_name, leaf, (unsigned long long)bit);
}

static void shorthand_target_symbol(char *dst, size_t dsz,
                                    char const *owner_name,
                                    char segs[][64], int n_segs) {
    char leaf[256];
    property_leaf(segs, n_segs, leaf, sizeof(leaf));
    snprintf(dst, dsz, "%s%sShorthandTargets", owner_name, leaf);
}

static int emit_shorthand_arrays_walk(ob *b, cg_host_v1 const *h, cg_model const *m,
                                      sentry const *smap, size_t scount,
                                      char const *owner_name,
                                      char full_segs[][64], int n_full,
                                      char const *type) {
    sentry const *s = find_struct(smap, scount, type);
    int count = 0;
    if (!(s && !s->sealed)) return 0;

    char targets[512];
    char field_segs[MAX_DEPTH][64] = {{0}};
    int present_bit = 0;
    shorthand_target_symbol(targets, sizeof(targets), owner_name, full_segs, n_full);
    if (ob_printf(b, "static struct PropertyShorthandTarget const %s[] = {\n",
            targets) < 0) return -1;
    if (emit_shorthand_target_walk(b, h, m, smap, scount,
            owner_name, type, full_segs, n_full, field_segs, 0, type,
            &present_bit) < 0) return -1;
    if (ob_printf(b, "};\n") < 0) return -1;
    count++;

    if (n_full >= MAX_DEPTH - 1) return count;
    cg_foreach(m, s->id, CG_KIND_FIELD, f) {
        int fixed = (f->extra && f->extra[0]) ? atoi(f->extra) : 0;
        if (fixed > 0) {
            int i;
            for (i = 0; i < fixed; ++i) {
                char next_full[MAX_DEPTH][64];
                int nested;
                memcpy(next_full, full_segs, sizeof(next_full));
                snprintf(next_full[n_full], sizeof(next_full[n_full]), "%s[%d]", f->name, i);
                nested = emit_shorthand_arrays_walk(b, h, m, smap, scount,
                        owner_name, next_full, n_full + 1, f->type);
                if (nested < 0) return -1;
                count += nested;
            }
        } else {
            char next_full[MAX_DEPTH][64];
            int nested;
            memcpy(next_full, full_segs, sizeof(next_full));
            snprintf(next_full[n_full], sizeof(next_full[n_full]), "%s", f->name);
            nested = emit_shorthand_arrays_walk(b, h, m, smap, scount,
                    owner_name, next_full, n_full + 1, f->type);
            if (nested < 0) return -1;
            count += nested;
        }
    }
    return count;
}

static int emit_shorthand_entries_walk(ob *b, cg_host_v1 const *h, cg_model const *m,
                                       sentry const *smap, size_t scount,
                                       char const *owner_name,
                                       char full_segs[][64], int n_full,
                                       char const *type) {
    sentry const *s = find_struct(smap, scount, type);
    if (!(s && !s->sealed)) return 0;

    char leaf[256], targets[512];
    property_leaf(full_segs, n_full, leaf, sizeof(leaf));
    shorthand_target_symbol(targets, sizeof(targets), owner_name, full_segs, n_full);
    if (ob_printf(b,
            "\tSHORTHAND(%s, %s, \"%s\", %s, 0x%08x),\n",
            owner_name, leaf, s->export_name, type,
            h->fnv1a32(leaf)) < 0) return -1;

    if (n_full >= MAX_DEPTH - 1) return 0;
    cg_foreach(m, s->id, CG_KIND_FIELD, f) {
        int fixed = (f->extra && f->extra[0]) ? atoi(f->extra) : 0;
        if (fixed > 0) {
            int i;
            for (i = 0; i < fixed; ++i) {
                char next_full[MAX_DEPTH][64];
                memcpy(next_full, full_segs, sizeof(next_full));
                snprintf(next_full[n_full], sizeof(next_full[n_full]), "%s[%d]", f->name, i);
                if (emit_shorthand_entries_walk(b, h, m, smap, scount,
                        owner_name, next_full, n_full + 1, f->type) < 0) return -1;
            }
        } else {
            char next_full[MAX_DEPTH][64];
            memcpy(next_full, full_segs, sizeof(next_full));
            snprintf(next_full[n_full], sizeof(next_full[n_full]), "%s", f->name);
            if (emit_shorthand_entries_walk(b, h, m, smap, scount,
                    owner_name, next_full, n_full + 1, f->type) < 0) return -1;
        }
    }
    return 0;
}

static int emit_class_shorthands(ob *b, cg_host_v1 const *h, cg_model const *m,
                                 sentry const *smap, size_t scount,
                                 cg_node const *c) {
    int count = 0;
    cg_foreach(m, c->id, CG_KIND_PROPERTY, p) {
        sentry const *s = find_struct(smap, scount, p->type);
        if (s && !s->sealed && !(p->flags & CG_FLAG_ARRAY)) {
            char full_segs[MAX_DEPTH][64] = {{0}};
            int emitted;
            snprintf(full_segs[0], sizeof(full_segs[0]), "%s", p->name);
            emitted = emit_shorthand_arrays_walk(b, h, m, smap, scount,
                    c->name, full_segs, 1, p->type);
            if (emitted < 0) return -1;
            count += emitted;
        }
    }
    if (ob_printf(b, "static struct PropertyShorthand const %sShorthands[] = {\n",
            c->name) < 0) return -1;
    if (count) {
        cg_foreach(m, c->id, CG_KIND_PROPERTY, p) {
            sentry const *s = find_struct(smap, scount, p->type);
            if (s && !s->sealed && !(p->flags & CG_FLAG_ARRAY)) {
                char full_segs[MAX_DEPTH][64] = {{0}};
                snprintf(full_segs[0], sizeof(full_segs[0]), "%s", p->name);
                if (emit_shorthand_entries_walk(b, h, m, smap, scount,
                        c->name, full_segs, 1, p->type) < 0) return -1;
            }
        }
    }
    if (ob_printf(b, "};\n") < 0) return -1;
    return 0;
}

static int emit_raw_field_row(ob *b, cg_host_v1 const *h, cg_model const *m,
                              char const *owner_name, cg_node const *field) {
    char const *kind = type_kind(m, field->type);
    if (ob_printf(b, "\t%s(0x%08x, %s, %s, %s, %s",
            (field->flags & CG_FLAG_ARRAY) ? "ARRAY_DECL" : "DECL",
            h->fnv1a32(field->name), owner_name, field->name, field->name,
            property_datatype(m, field->type)) < 0) return -1;
    if (!strcmp(kind, "enum")) {
        return ob_printf(b, ", .EnumValues = _%s), // %s.%s\n", field->type, owner_name, field->name);
    } else if (!strcmp(kind, "external_struct")) {
        return ob_printf(b, ", .TypeString = \"%s\"), // %s.%s\n", field->type, owner_name, field->name);
    } else if (!strcmp(kind, "struct") && strcmp(field->type, "color")) {
        cg_node const *st = find_kind(m, field->type, CG_KIND_STRUCT);
        return ob_printf(b, ", .TypeString = \"%s\"), // %s.%s\n",
                st ? export_name(st) : field->type, owner_name, field->name);
    } else if (!strcmp(kind, "component") || !strcmp(kind, "interface")) {
        cg_node const *n = find_kind(m, field->type, !strcmp(kind, "component") ? CG_KIND_CLASS : CG_KIND_INTERFACE);
        return ob_printf(b, ", .TypeString = \"%s\"), // %s.%s\n",
                n ? export_name(n) : field->type, owner_name, field->name);
    }
    return ob_printf(b, "), // %s.%s\n", owner_name, field->name);
}

static int emit_enum_macros(ob *b, cg_model const *m) {
    cg_foreach(m, 0, CG_KIND_ENUM, e) {
        int first = 1;
        if (ob_printf(b, "ENUM(%s", e->name) < 0) return -1;
        cg_foreach(m, e->id, CG_KIND_VALUE, v) {
            if (ob_printf(b, "%s \"%s\"", first ? "," : ",", v->name) < 0) return -1;
            first = 0;
        }
        if (ob_printf(b, ")\n") < 0) return -1;
    }
    return 0;
}

static int model_has_enum(cg_model const *m, char const *name) {
    return find_kind(m, name, CG_KIND_ENUM) != NULL;
}

static int emit_struct_parser_helpers(ob *b, cg_model const *m) {
    int has_axis_alignment = !strcmp(m->module_name, "core") &&
                             model_has_enum(m, "HorizontalAlignment") &&
                             model_has_enum(m, "VerticalAlignment") &&
                             model_has_enum(m, "DepthAlignment");
        if (ob_printf(b, "\t}\n\treturn FALSE;\n}\n") < 0) return -1;
    if (has_axis_alignment) {
        if (ob_printf(b,
                "static int cg_axis_enum(struct PropertyType const*p,const char*s,void*d){return !strcmp(p->Name,\"Horizontal\")?cg_enum(_HorizontalAlignment,s,d):!strcmp(p->Name,\"Vertical\")?cg_enum(_VerticalAlignment,s,d):!strcmp(p->Name,\"Depth\")?cg_enum(_DepthAlignment,s,d):FALSE;}\n") < 0) return -1;
    } else {
        if (ob_printf(b,
                "static int cg_axis_enum(struct PropertyType const*p,const char*s,void*d){(void)p;(void)s;(void)d;return FALSE;}\n") < 0) return -1;
    }
    return ob_printf(b,
            "static int cg_p(const char*s,struct PropertyType const*p,void*d){char tmp[MAX_PROPERTY_STRING]={0};if(cg_axis_enum(p,s,d))return TRUE;if(p->DataType==kDataTypeEnum)return cg_enum(p->EnumValues,s,d);if((p->DataType==kDataTypeFloat||p->DataType==kDataTypeInt)&&!cg_num(s))return FALSE;if(p->DataType==kDataTypeStruct)return OBJ_ParseStruct(p->TypeString,s,d,p->DataSize);if(!parse_property(s,p,tmp))return FALSE;memcpy(d,tmp,p->DataSize);return TRUE;}\n"
            "static int cg_put(char*s,void*d,struct PropertyType const*p,int i,uint64_t*m){if(cg_p(s,&p[i],(char*)d+p[i].Offset)){if(i<64)*m|=((uint64_t)1<<i);return TRUE;}return FALSE;}\n"
            "static int cg_box(void*d,struct PropertyType const*p,int n,char**t,int nt,const char*pre,uint64_t*m){char a[32];int L,R,T,B,F,K,k=0;if(nt>4)return 0;while(k<nt&&cg_num(t[k]))k++;if(!k)return 0;snprintf(a,sizeof(a),\"%%sLeft\",pre);L=cg_i(p,n,a);snprintf(a,sizeof(a),\"%%sRight\",pre);R=cg_i(p,n,a);snprintf(a,sizeof(a),\"%%sTop\",pre);T=cg_i(p,n,a);snprintf(a,sizeof(a),\"%%sBottom\",pre);B=cg_i(p,n,a);snprintf(a,sizeof(a),\"%%sFront\",pre);F=cg_i(p,n,a);snprintf(a,sizeof(a),\"%%sBack\",pre);K=cg_i(p,n,a);if(L<0||R<0||T<0||B<0)return 0;cg_put(t[0],d,p,L,m);cg_put(k>2?t[2]:t[0],d,p,R,m);cg_put(k>1?t[1]:t[0],d,p,T,m);cg_put(k>3?t[3]:(k>1?t[1]:t[0]),d,p,B,m);if(pre[0]&&F>=0&&K>=0){cg_put(t[0],d,p,F,m);cg_put(t[0],d,p,K,m);}return k;}\n"
            "static int cg_req(void*d,struct PropertyType const*p,int n,char**t,int nt,uint64_t*m){int H,V,D;if(nt>3)return 0;for(int i=0;i<nt;i++)if(!cg_num(t[i]))return 0;H=cg_i(p,n,\"HorizontalRequested\");V=cg_i(p,n,\"VerticalRequested\");D=cg_i(p,n,\"DepthRequested\");if(H<0||V<0)return 0;cg_put(t[0],d,p,H,m);if(nt>1)cg_put(t[1],d,p,V,m);if(nt>2&&D>=0)cg_put(t[2],d,p,D,m);return nt;}\n"
            "static int cg_same(struct PropertyType const*p,int n){for(int i=1;i<n;i++)if(p[i].DataType!=p[0].DataType||p[i].EnumValues!=p[0].EnumValues)return 0;return n>1;}\n"
            "static int cg_join(char**t,int n,char*b){b[0]=0;for(int i=0;i<n;i++){if(i)strcat(b,\" \");strcat(b,t[i]);}return TRUE;}\n"
            "static int cg_span(char**t,int nt,struct PropertyType const*p,int i,void*d,int*used,uint64_t*m){char b[MAX_PROPERTY_STRING];if(p[i].DataType!=kDataTypeStruct)return nt&&!used[0]&&cg_put(t[0],d,p,i,m)?1:0;for(int c=nt;c>0;c--){int ok=1;for(int j=0;j<c;j++)if(used[j])ok=0;if(!ok)continue;cg_join(t,c,b);if(cg_put(b,d,p,i,m))return c;}return 0;}\n"
            "static int cg_done(uint64_t m){OBJ_SetStructParseMask(m);return TRUE;}\n"
            "static int cg_parse(const char*s,void*d,size_t z,struct PropertyType const*p,int n){char b[MAX_PROPERTY_STRING]={0},*t[32],*x;int nt=0,u[32]={0},f[64]={0},any=FALSE;uint64_t m=0;if(!d)return FALSE;memset(d,0,z);snprintf(b,sizeof(b),\"%%s\",s);for(x=strtok(b,\" \\t\\r\\n\");x&&nt<32;x=strtok(NULL,\" \\t\\r\\n\"))t[nt++]=x;if(!nt)return FALSE;if(cg_box(d,p,n,t,nt,\"\",&m)==nt)return cg_done(m);if(cg_req(d,p,n,t,nt,&m)==nt)return cg_done(m);if(nt==1&&cg_same(p,n)){for(int i=0;i<n;i++)any|=cg_put(t[0],d,p,i,&m);return any?cg_done(m):FALSE;}int c=cg_box(d,p,n,t,nt,\"Width\",&m);for(int i=0;i<c;i++)u[i]=any=TRUE;for(int pass=0;pass<4;pass++)for(int j=0;j<nt;j++)if(!u[j])for(int i=0;i<n;i++){int dt=p[i].DataType;if(f[i]||(pass==0&&dt!=kDataTypeEnum)||(pass==1&&(dt==kDataTypeEnum||dt==kDataTypeColor||dt==kDataTypeObject))||(pass==2&&dt!=kDataTypeColor)||(pass==3&&dt!=kDataTypeObject))continue;c=cg_span(&t[j],nt-j,p,i,d,&u[j],&m);if(c){for(int q=0;q<c;q++)u[j+q]=TRUE;f[i]=any=TRUE;break;}}if(!any)return FALSE;for(int i=0;i<nt;i++)if(!u[i])return FALSE;return cg_done(m);}\n");
}

static int emit_struct_parser_wrappers(ob *b, cg_model const *m) {
    if (ob_printf(b,
            "#define CG_PARSE_WRAPPER(NAME) \\\n"
            "static int cg_parse_##NAME(const char*s,void*d,size_t z){ \\\n"
            "\treturn z==sizeof(struct NAME)&&cg_parse(s,d,z,_##NAME,sizeof(_##NAME)/sizeof(*_##NAME)); \\\n"
            "}\n") < 0) return -1;
    cg_foreach(m, 0, CG_KIND_STRUCT, s) {
        if (ob_printf(b, "CG_PARSE_WRAPPER(%s)\n", s->name) < 0) return -1;
    }
    if (ob_printf(b, "#undef CG_PARSE_WRAPPER\n") < 0) return -1;
    return 0;
}

static int emit_call_args(ob *b, cg_model const *m, cg_node const *method, cg_node const *owner) {
    int first = 1;
    if (method->flags & CG_FLAG_LUA) {
        if (ob_printf(b, "L") < 0) return -1;
        first = 0;
    }
    if (owner && !(method->flags & CG_FLAG_STATIC)) {
        if (ob_printf(b, "%sthis_", first ? "" : ", ") < 0) return -1;
        first = 0;
    }
    cg_foreach(m, method->id, CG_KIND_ARG, arg) {
        if (ob_printf(b, "%s%s", first ? "" : ", ", arg->name) < 0) return -1;
        first = 0;
    }
    return 0;
}

static int emit_method_wrapper(ob *b, cg_model const *m, cg_node const *method,
                               cg_node const *owner, char const *prefix) {
    int lua_idx = 1;
    cg_node const *ret = method_returns(m, method);
    char ret_decl[256], push[512];
    if (ob_printf(b, "int f_%s%s(lua_State *L) {\n", prefix, method->name) < 0) return -1;
    if (owner && !(method->flags & CG_FLAG_STATIC)) {
        uint32_t flags = CG_FLAG_POINTER | ((method->flags & CG_FLAG_CONST) ? CG_FLAG_CONST : 0);
        char decl[256], check[256];
        type_decl(decl, sizeof(decl), m, owner->name, flags);
        type_check(check, sizeof(check), m, owner->name, flags, lua_idx++);
        if (ob_printf(b, "\t%s this_ = %s;\n", decl, check) < 0) return -1;
    }
    cg_foreach(m, method->id, CG_KIND_ARG, arg) {
        if (!strcmp(arg->name, "L")) continue;
        if (arg->aux && arg->aux[0]) {
            if (ob_printf(b, "\t%s %s = {0};\n", arg->aux, arg->name) < 0) return -1;
        } else if (arg->aux2 && arg->aux2[0]) {
            if (ob_printf(b, "\tint32_t %s = sizeof(%s);\n", arg->name, arg->aux2) < 0) return -1;
        } else {
            char decl[256], check[256];
            type_decl(decl, sizeof(decl), m, arg->type, arg->flags);
            type_check(check, sizeof(check), m, arg->type, arg->flags, lua_idx++);
            if (ob_printf(b, "\t%s %s = %s;\n", decl, arg->name, check) < 0) return -1;
        }
    }
    if (ret && !strcmp(type_kind(m, ret->type), "nresults")) {
        if (ob_printf(b, "\treturn %s%s(", prefix, method->name) < 0) return -1;
        if (emit_call_args(b, m, method, owner) < 0) return -1;
        if (ob_printf(b, ");\n}\n") < 0) return -1;
    } else if (ret) {
        type_decl(ret_decl, sizeof(ret_decl), m, ret->type, ret->flags);
        if (ob_printf(b, "\t%s result_ = %s%s(", ret_decl, prefix, method->name) < 0) return -1;
        if (emit_call_args(b, m, method, owner) < 0) return -1;
        if (ob_printf(b, ");\n") < 0) return -1;
        type_push(push, sizeof(push), m, ret->type, ret->flags, "result_");
        if (ob_printf(b, "\t%s;\n\treturn 1;\n}\n", push) < 0) return -1;
    } else {
        if (ob_printf(b, "\t%s%s(", prefix, method->name) < 0) return -1;
        if (emit_call_args(b, m, method, owner) < 0) return -1;
        if (ob_printf(b, " );\n\treturn 0;\n}\n") < 0) return -1;
    }
    return 0;
}

static int emit_method_wrappers(ob *b, cg_model const *m, cg_node const *owner,
                                char const *prefix) {
    cg_foreach(m, owner ? owner->id : 0, CG_KIND_METHOD, method) {
        if (emit_method_wrapper(b, m, method, owner, prefix) < 0) return -1;
    }
    return 0;
}

static int emit_interfaces(ob *b, cg_model const *m) {
    cg_foreach(m, 0, CG_KIND_INTERFACE, iface) {
        char const *prefix = owner_prefix(iface);
        if (ob_printf(b, "\n") < 0) return -1;
        if (emit_method_wrappers(b, m, iface, prefix) < 0) return -1;
        if (ob_printf(b, "\nint luaopen_orca_%s(lua_State *L) {\n"
                "\tluaL_newmetatable(L, \"%s\");\n"
                "\tluaL_setfuncs(L, ((luaL_Reg[]) {\n",
                iface->name, export_name(iface)) < 0) return -1;
        cg_foreach(m, iface->id, CG_KIND_METHOD, method) {
            if (ob_printf(b, "\t\t{ \"%s\", f_%s%s },\n",
                    method_export_name(method), prefix, method->name) < 0) return -1;
            if (method->aux && method->aux[0])
                if (ob_printf(b, "\t\t{ \"%s\", f_%s%s },\n",
                        method->aux, prefix, method->name) < 0) return -1;
        }
        if (ob_printf(b,
                "\t\t{ NULL, NULL },\n"
                "\t}), 0);\n"
                "\tlua_getfield(L, -1, \"__index\");\n"
                "\tif (lua_isnil(L, -1)) {\n"
                "\t\tlua_pop(L, 1);\n"
                "\t\tlua_pushvalue(L, -1);\n"
                "\t\tlua_setfield(L, -2, \"__index\");\n"
                "\t} else {\n"
                "\t\tlua_pop(L, 1);\n"
                "\t}\n"
                "\treturn 1;\n"
                "}\n") < 0) return -1;
    }
    return 0;
}

static int emit_structs(ob *b, cg_host_v1 const *h, cg_model const *m,
                        sentry const *smap, size_t scount) {
    if (ob_printf(b, "extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* struct_ptr);\n"
            "extern int write_property(lua_State *L, struct PropertyType const* prop, void const* struct_ptr);\n"
            "extern int parse_property(const char* str, struct PropertyType const* prop, void* struct_ptr);\n") < 0) return -1;
    cg_foreach(m, 0, CG_KIND_STRUCT, s) {
        char const *prefix = owner_prefix(s);
        if (emit_method_wrappers(b, m, s, prefix) < 0) return -1;
        if (ob_printf(b, "static struct PropertyType _%s[] = {\n", s->name) < 0) return -1;
        if (emit_properties_for_fields(b, h, m, smap, scount, s, s->name, CG_KIND_FIELD, 0, NULL) < 0) return -1;
        if (ob_printf(b, "};\nstatic luaL_Reg _%s_Methods[] = {\n", s->name) < 0) return -1;
        cg_foreach(m, s->id, CG_KIND_METHOD, method)
            if (ob_printf(b, "\t{ \"%s\", f_%s%s },\n",
                    method_export_name(method), prefix, method->name) < 0) return -1;
        if (ob_printf(b, "\t{ NULL, NULL }\n};\n") < 0) return -1;
    }
    if (ob_printf(b, "\n") < 0) return -1;
    if (emit_struct_parser_helpers(b, m) < 0) return -1;
    if (emit_struct_parser_wrappers(b, m) < 0) return -1;
    cg_foreach(m, 0, CG_KIND_STRUCT, s) {
        if (ob_printf(b, "STRUCT_PARSER(%s, %s, cg_parse_%s);\n",
                s->name, export_name(s), s->name) < 0) return -1;
    }
    return 0;
}

static cg_node const *find_event(cg_model const *m, char const *name) {
    size_t i;
    if (!name || !name[0]) return NULL;
    for (i = 0; i < m->node_count; ++i) {
        cg_node const *msg = &m->nodes[i];
        cg_node const *owner;
        if (msg->kind != CG_KIND_MESSAGE) continue;
        owner = node_parent(m, msg);
        if (!local_node(owner)) continue;
        if (!strcmp(msg->name, name)) return msg;
        {
            char full[512];
            snprintf(full, sizeof(full), "%s.%s", owner ? owner->name : "", msg->name);
            if (!strcmp(full, name)) return msg;
        }
    }
    return NULL;
}

static cg_node const *event_parent(cg_model const *m, cg_node const *msg) {
    return (msg->extra && msg->extra[0]) ? find_event(m, msg->extra) : NULL;
}

static void event_struct_name(cg_model const *m, cg_node const *msg, char *dst, size_t dsz) {
    cg_node const *owner = node_parent(m, msg);
    snprintf(dst, dsz, "%s_%sEventArgs", owner ? owner->name : "", msg->name);
}

static int emit_event_fields(ob *b, cg_host_v1 const *h, cg_model const *m,
                             sentry const *smap, size_t scount,
                             cg_node const *msg, char const *struct_name) {
    cg_node const *p = event_parent(m, msg);
    (void)smap;
    (void)scount;
    if (p && emit_event_fields(b, h, m, smap, scount, p, struct_name) < 0) return -1;
    cg_foreach(m, msg->id, CG_KIND_FIELD, f) {
        if (f->flags & CG_FLAG_NOEXPORT) continue;
        if (emit_raw_field_row(b, h, m, struct_name, f) < 0) return -1;
    }
    return 0;
}

static int foreach_local_event(cg_model const *m, size_t *idx, cg_node const **msg_out) {
    while (*idx < m->node_count) {
        cg_node const *n = &m->nodes[(*idx)++];
        cg_node const *owner = node_parent(m, n);
        if (n->kind == CG_KIND_MESSAGE && local_node(owner) &&
            (owner->kind == CG_KIND_INTERFACE || owner->kind == CG_KIND_CLASS)) {
            *msg_out = n;
            return 1;
        }
    }
    return 0;
}

static int emit_events(ob *b, cg_host_v1 const *h, cg_model const *m,
                       sentry const *smap, size_t scount) {
    size_t idx = 0;
    cg_node const *msg;
    while (foreach_local_event(m, &idx, &msg)) {
        cg_node const *p = event_parent(m, msg);
        if (p) {
            char s[256], ps[256];
            event_struct_name(m, msg, s, sizeof(s));
            event_struct_name(m, p, ps, sizeof(ps));
            if (ob_printf(b, "#define %s %s\n", s, ps) < 0) return -1;
        }
    }
    if (ob_printf(b, "\n") < 0) return -1;
    idx = 0;
    while (foreach_local_event(m, &idx, &msg)) {
        char s[256];
        event_struct_name(m, msg, s, sizeof(s));
        if (ob_printf(b, "static luaL_Reg _%s_Methods[] = { { NULL, NULL } };\n"
                "struct PropertyType _%s_Properties[] = {\n", s, s) < 0) return -1;
        if (emit_event_fields(b, h, m, smap, scount, msg, s) < 0) return -1;
        if (ob_printf(b, "};\n#define _%s _%s_Properties\n", s, s) < 0) return -1;
    }
    if (ob_printf(b, "\n") < 0) return -1;
    idx = 0;
    while (foreach_local_event(m, &idx, &msg)) {
        char s[256];
        event_struct_name(m, msg, s, sizeof(s));
        if (ob_printf(b, "STRUCT(%s, %s);\n", s, s) < 0) return -1;
    }
    return 0;
}

static void message_action_name(cg_model const *m, cg_node const *msg, char *dst, size_t dsz) {
    cg_node const *owner = node_parent(m, msg);
    snprintf(dst, dsz, "%s_%sAction", owner ? owner->name : "", msg->name);
}

static void message_xml_name(cg_model const *m, cg_node const *msg, char *dst, size_t dsz) {
    cg_node const *owner = node_parent(m, msg);
    snprintf(dst, dsz, "%s.%s", owner ? owner->name : "", msg->name);
}

static int event_field_count(cg_model const *m, cg_node const *msg) {
    int count = 0;
    cg_node const *p = event_parent(m, msg);
    if (p) count += event_field_count(m, p);
    cg_foreach(m, msg->id, CG_KIND_FIELD, f) {
        (void)f;
        count++;
    }
    return count;
}

static int emit_action_payload_properties(ob *b, cg_host_v1 const *h, cg_model const *m,
                                          cg_node const *msg, char const *action_name) {
    cg_node const *p = event_parent(m, msg);
    if (p && emit_action_payload_properties(b, h, m, p, action_name) < 0) return -1;
    cg_foreach(m, msg->id, CG_KIND_FIELD, f) {
        if (emit_raw_field_row(b, h, m, action_name, f) < 0) return -1;
    }
    return 0;
}

static int emit_message_action(ob *b, cg_host_v1 const *h, cg_model const *m, cg_node const *msg) {
    char action[256], xml_name[512];
    int prop_count = event_field_count(m, msg);
    message_action_name(m, msg, action, sizeof(action));
    message_xml_name(m, msg, xml_name, sizeof(xml_name));

    if (prop_count > 0) {
        if (ob_printf(b,
                "static struct PropertyType const %sProperties[%d] = {\n",
                action, prop_count) < 0) return -1;
        if (emit_action_payload_properties(b, h, m, msg, action) < 0) return -1;
        if (ob_printf(b, "};\n") < 0) return -1;
    }

    if (ob_printf(b,
            "REGISTER_MESSAGE_ACTION(%s, \"%s\", %d, %s%s);\n",
            action, xml_name, prop_count,
            prop_count > 0 ? action : "",
            prop_count > 0 ? "Properties" : "NULL") < 0) return -1;
    if (emit_message_action_getter(b, m, msg) < 0) return -1;
    return 0;
}

static int emit_message_actions(ob *b, cg_host_v1 const *h, cg_model const *m) {
    size_t idx = 0;
    cg_node const *msg;
    while (foreach_local_event(m, &idx, &msg)) {
        if (emit_message_action(b, h, m, msg) < 0) return -1;
    }
    return 0;
}

static int emit_component_handlers(ob *b, cg_node const *c, cg_model const *m) {
    cg_foreach(m, c->id, CG_KIND_HANDLE, hdl) {
        char ident[512];
        size_t i;
        snprintf(ident, sizeof(ident), "%s", hdl->name);
        for (i = 0; ident[i]; ++i) if (ident[i] == '.') ident[i] = ',';
        for (i = 0; ident[i]; ++i) {
            if (ident[i] == ',') {
                size_t len = strlen(ident);
                if (len + 1u < sizeof(ident)) {
                    memmove(ident + i + 2u, ident + i + 1u, len - i);
                    ident[i + 1u] = ' ';
                    ++i;
                }
            }
        }
        if (ob_printf(b, "HANDLER(%s, %s);\n", c->name, ident) < 0) return -1;
    }
    return 0;
}

static char const *after_dot(char const *s) {
    char const *p = strrchr(s, '.');
    return p ? p + 1 : "";
}

static void dot_to_underscore(char *dst, size_t dsz, char const *s) {
    size_t i;
    snprintf(dst, dsz, "%s", s);
    for (i = 0; dst[i]; ++i) if (dst[i] == '.') dst[i] = '_';
}

static int emit_component_parents(ob *b, cg_node const *c) {
    char parents[512];
    char *tok;
    int first = 1;
    if (c->type && c->type[0]) {
        snprintf(parents, sizeof(parents), "%s", c->type);
        for (tok = strtok(parents, ","); tok; tok = strtok(NULL, ",")) {
            while (*tok == ' ' || *tok == '\t') ++tok;
            if (ob_printf(b, "%sID_%s", first ? "" : ", ", tok) < 0) return -1;
            first = 0;
        }
    }
    if (c->extra && c->extra[0]) {
        if (ob_printf(b, "%sID_%s", first ? "" : ", ", c->extra) < 0) return -1;
        first = 0;
    }
    return ob_printf(b, "%s0", first ? "" : ", ");
}

static int emit_defaults_for_props(ob *b, cg_model const *m, uint32_t owner_id) {
    cg_foreach(m, owner_id, CG_KIND_PROPERTY, p) {
        char const *kind = type_kind(m, p->type);
        if (p->flags & CG_FLAG_INHERITED) continue;
        if (p->extra && p->extra[0]) {
            if (!strcmp(kind, "string")) {
                if (ob_printf(b, "\t.%s = \"%s\",\n", p->name, p->extra) < 0) return -1;
            } else if (!strcmp(kind, "enum")) {
                if (ob_printf(b, "\t.%s = k%s%s,\n", p->name, p->type, p->extra) < 0) return -1;
            } else if (!strcmp(kind, "struct") || !strcmp(kind, "component") ||
                       !strcmp(kind, "interface") || !strcmp(kind, "external_struct")) {
                if (ob_printf(b, "\t.%s = {%s},\n", p->name, p->extra) < 0) return -1;
            } else {
                if (ob_printf(b, "\t.%s = %s,\n", p->name, p->extra) < 0) return -1;
            }
        }
    }
    return 0;
}

static int emit_components(ob *b, cg_host_v1 const *h, cg_model const *m,
                           sentry const *smap, size_t scount) {
    cg_foreach(m, 0, CG_KIND_CLASS, c) {
        char ss_buf[256];
        char const *ss = class_storage_name(m, c, ss_buf, sizeof(ss_buf));
        cg_node const *mx = (c->aux && c->aux[0]) ? find_kind(m, c->aux, CG_KIND_CLASS) : NULL;
        if (emit_component_handlers(b, c, m) < 0) return -1;
        if (emit_class_shorthands(b, h, m, smap, scount, c) < 0) return -1;
        if (ss) { if (ob_printf(b, "#define _STORAGE_STRUCT %s\n", ss) < 0) return -1; }
        if (ob_printf(b, "static struct PropertyType const %sProperties[] = {\n",
                c->name) < 0) return -1;
        if (mx && emit_properties_for_fields(b, h, m, smap, scount, mx, c->name, CG_KIND_PROPERTY, 1, ss) < 0) return -1;
        if (emit_properties_for_fields(b, h, m, smap, scount, c, c->name, CG_KIND_PROPERTY, 1, ss) < 0) return -1;
        cg_foreach(m, c->id, CG_KIND_MESSAGE, msg) {
            const char *decl_macro = ss ? "UIDATA_DECL" : "DECL";
            if (ob_printf(b, "\t%s(0x%08x, %s, %s, %s, kDataTypeEvent, .TypeString = \"%s_%sEventArgs\"), // %s.%s\n",
                    decl_macro, h->fnv1a32(msg->name), c->name, msg->name, msg->name,
                    c->name, msg->name, c->name, msg->name) < 0) return -1;
        }
        if (ob_printf(b, "};\nstatic struct %s %sDefaults = {\n", c->name, c->name) < 0) return -1;
        if (mx && emit_defaults_for_props(b, m, mx->id) < 0) return -1;
        if (emit_defaults_for_props(b, m, c->id) < 0) return -1;
        if (ob_printf(b, "};\nLRESULT %sProc(struct Object* object, uint32_t message, wParam_t wparm, lParam_t lparm) {\n"
                "\tstruct %s* cmp = OBJ_GetTypedata(object, ID_%s);\n"
                "\tswitch (message) {\n", c->name, c->name, c->name) < 0) return -1;
        cg_foreach(m, c->id, CG_KIND_HANDLE, hdl) {
            char ident[512];
            dot_to_underscore(ident, sizeof(ident), hdl->name);
            if (ob_printf(b, "\t\tcase ID_%s: return %s_%s(object, cmp, wparm, lparm); // %s\n",
                    ident, c->name, after_dot(hdl->name), hdl->name) < 0) return -1;
        }
        if (ob_printf(b, "\t}\n\treturn FALSE;\n}\n") < 0) return -1;
        if (emit_class_getter(b, m, smap, scount, c, mx) < 0) return -1;
        if (ob_printf(b, "REGISTER_CLASS(%s, ", c->name) < 0) return -1;
        if (emit_component_parents(b, c) < 0) return -1;
        if (ob_printf(b, ");\n") < 0) return -1;
        if (ss) { if (ob_printf(b, "#undef _STORAGE_STRUCT\n") < 0) return -1; }
    }
    return 0;
}

static int emit_luaopen(ob *b, cg_model const *m) {
    if (ob_printf(b, "\nORCA_API int luaopen_orca_%s(lua_State *L) {\n"
            "\tluaL_newlib(L, ((luaL_Reg[]) { \n", m->module_name) < 0) return -1;
    cg_foreach(m, 0, CG_KIND_METHOD, fn) {
        if (ob_printf(b, "\t\t{ \"%s\", f_%s%s },\n",
                method_export_name(fn), m->prefix ? m->prefix : "", fn->name) < 0) return -1;
    }
    if (ob_printf(b, "\t\t{ NULL, NULL } \n\t}));\n") < 0) return -1;
    if (m->on_luaopen && m->on_luaopen[0]) {
        if (ob_printf(b, "\tvoid %s(lua_State *L);\n\t%s(L);\n", m->on_luaopen, m->on_luaopen) < 0) return -1;
    }
    cg_foreach(m, 0, CG_KIND_STRUCT, s)
        if (ob_printf(b, "\tlua_setfield(L, ((void)luaopen_orca_%s(L), -2), \"%s\");\n",
                s->name, export_name(s)) < 0) return -1;
    {
        size_t idx = 0;
        cg_node const *msg;
        while (foreach_local_event(m, &idx, &msg)) {
            char s[256];
            event_struct_name(m, msg, s, sizeof(s));
            if (ob_printf(b, "\tlua_setfield(L, ((void)luaopen_orca_%s(L), -2), \"%s\");\n", s, s) < 0) return -1;
        }
    }
    cg_foreach(m, 0, CG_KIND_INTERFACE, iface)
        if (ob_printf(b, "\tlua_setfield(L, ((void)luaopen_orca_%s(L), -2), \"%s\");\n",
                iface->name, export_name(iface)) < 0) return -1;
    cg_foreach(m, 0, CG_KIND_CLASS, c)
        if (ob_printf(b, "\tlua_setfield(L, ((void)lua_pushclass(L, &_%s), -2), \"%s\");\n",
                c->name, export_name(c)) < 0) return -1;
    {
        size_t idx = 0;
        cg_node const *msg;
        while (foreach_local_event(m, &idx, &msg)) {
            char action[256], xml_name[512];
            message_action_name(m, msg, action, sizeof(action));
            message_xml_name(m, msg, xml_name, sizeof(xml_name));
            if (ob_printf(b, "\tlua_setfield(L, ((void)lua_pushclass(L, &_%s), -2), \"%s\");\n",
                    action, xml_name) < 0) return -1;
        }
    }
    if (m->after_luaopen && m->after_luaopen[0]) {
        if (ob_printf(b, "\tvoid %s(lua_State *L);\n\t%s(L);\n", m->after_luaopen, m->after_luaopen) < 0) return -1;
    }
    return ob_printf(b, "\treturn 1;\n}\n");
}

static int seen_name(char const **names, size_t n, char const *name) {
    size_t i;
    for (i = 0; i < n; ++i)
        if (!strcmp(names[i], name)) return 1;
    return 0;
}

static int emit_extern_enums(ob *b, cg_model const *m) {
    char const *names[256];
    size_t n = 0;
    cg_foreach(m, 0, CG_KIND_CLASS, c) {
        cg_foreach(m, c->id, CG_KIND_PROPERTY, p) {
            char const *kind = type_kind(m, p->type);
            if (!strcmp(kind, "enum") && !local_node(find_kind(m, p->type, CG_KIND_ENUM)) &&
                    !seen_name(names, n, p->type) && n < sizeof(names) / sizeof(names[0])) {
                names[n++] = p->type;
                if (ob_printf(b, "extern const char *_%s[];\n", p->type) < 0) return -1;
            }
        }
    }
    return 0;
}

static int emit_export(cg_host_v1 const *host, cg_model const *model, char const *output) {
    ob b = {0};
    size_t scount = 0;
    sentry *smap = build_struct_map(model, &scount);
    if (!smap) return -1;
    if (ob_printf(&b,
            "// Auto-generated from %s by tools/codegen/plugins/export.c\n"
            "// DO NOT EDIT — run 'cd tools && make' to regenerate.\n"
            "#include <include/api.h>\n"
            "#include <include/codegen.h>\n\n"
            "#include <stdlib.h>\n"
            "#include <string.h>\n"
            "#include <strings.h>\n\n"
            "#include <%s/%s.h>\n\n",
            base_name(model->xml_path), model->module_name, model->module_name) < 0) goto fail;
    cg_foreach(model, 0, CG_KIND_EXTERNAL, ext) {
        if (ob_printf(&b, "// %s\n"
                "extern void luaX_push%s(lua_State *L, struct %s const* value);\n"
                "extern struct %s* luaX_check%s(lua_State *L, int index);\n",
                ext->name, ext->name, ext->name, ext->name, ext->name) < 0) goto fail;
    }
    if (ob_printf(&b, "\n") < 0) goto fail;
    if (strcmp(model->module_name, "core")) {
        if (ob_printf(&b, "extern const char *_DispatchMode[];\n") < 0) goto fail;
    }
    if (emit_extern_enums(&b, model) < 0) goto fail;
    if (emit_enum_macros(&b, model) < 0) goto fail;
    if (emit_interfaces(&b, model) < 0) goto fail;
    if (emit_structs(&b, host, model, smap, scount) < 0) goto fail;
    if (emit_events(&b, host, model, smap, scount) < 0) goto fail;
    if (emit_message_actions(&b, host, model) < 0) goto fail;
    if (emit_components(&b, host, model, smap, scount) < 0) goto fail;
    if (emit_method_wrappers(&b, model, NULL, model->prefix ? model->prefix : "") < 0) goto fail;
    if (emit_luaopen(&b, model) < 0) goto fail;
    free(smap);
    if (host->write_file(output, b.s, b.n) < 0) {
        host->logf("export: write failed");
        free(b.s);
        return -1;
    }
    free(b.s);
    return 0;
fail:
    free(smap);
    free(b.s);
    return -1;
}

static cg_plugin_v1 const plugin = { CG_API_VERSION, "export", emit_export };
cg_plugin_v1 const *cg_plugin_init_v1(void) { return &plugin; }
