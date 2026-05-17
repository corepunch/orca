#include "codegen_export_backend.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static const type_rule_t k_rules[] = {
    {"float", "float", "luaL_checknumber(L, {arg})", "lua_pushnumber(L, {arg})"},
    {"int", "int32_t", "(int32_t)luaL_checkinteger(L, {arg})", "lua_pushinteger(L, {arg})"},
    {"uint", "uint32_t", "(uint32_t)luaL_checkinteger(L, {arg})", "lua_pushinteger(L, {arg})"},
    {"long", "long", "luaL_checkinteger(L, {arg})", "lua_pushinteger(L, {arg})"},
    {"bool", "bool_t", "lua_toboolean(L, {arg})", "lua_pushboolean(L, {arg})"},
    {"string", "const char*", "luaL_checkstring(L, {arg})", "lua_pushstring(L, {arg})"},
    {"enum", "enum {type}", "luaX_check{type}(L, {arg})", "luaX_push{type}(L, {arg})"},
    {"struct", "struct {type}", "luaX_check{type}(L, {arg})", "luaX_push{type}(L, &{arg})"},
    {"interface", "struct {type}", "luaX_check{type}(L, {arg})", "luaX_push{type}(L, {arg})"},
    {"component", "struct {type}", "luaX_check{type}(L, {arg})", "luaX_push{type}(L, {arg})"},
    {"external_struct", "struct {type}", "NULL", "luaX_push{type}(L, {arg})"},
    {"nresults", "int", NULL, NULL},
    {"void", "void", NULL, NULL},
};

void die(const char *msg) {
    fprintf(stderr, "codegen_export: %s\n", msg);
    exit(1);
}

char *dupstr(const char *s) {
    size_t n = s ? strlen(s) : 0;
    char *r = (char *)malloc(n + 1);
    if (!r) die("oom");
    if (n) memcpy(r, s, n);
    r[n] = 0;
    return r;
}

char *xml_prop_dup(xmlNodePtr n, const char *name) {
    xmlChar *v = xmlGetProp(n, BAD_CAST(name));
    if (!v) return NULL;
    char *s = dupstr((const char *)v);
    xmlFree(v);
    return s;
}

int xml_prop_true(xmlNodePtr n, const char *name) {
    char *v = xml_prop_dup(n, name);
    int ok = v && strcmp(v, "true") == 0;
    free(v);
    return ok;
}

void emit_template(FILE *out, const char *tmpl, const kv_t *kvs, size_t kv_count) {
    const char *p = tmpl;
    while (*p) {
        const char *s = strstr(p, "{{");
        if (!s) {
            fputs(p, out);
            return;
        }
        fwrite(p, 1, (size_t)(s - p), out);
        const char *e = strstr(s + 2, "}}");
        if (!e) {
            fputs(s, out);
            return;
        }
        size_t n = (size_t)(e - (s + 2));
        char key[64];
        if (n >= sizeof(key)) n = sizeof(key) - 1;
        memcpy(key, s + 2, n);
        key[n] = 0;
        const char *val = "";
        for (size_t i = 0; i < kv_count; ++i) {
            if (strcmp(kvs[i].key, key) == 0) {
                val = kvs[i].value ? kvs[i].value : "";
                break;
            }
        }
        fputs(val, out);
        p = e + 2;
    }
}

xmlNodePtr find_child(xmlNodePtr parent, const char *name) {
    EACH_ELEMENT(n, parent, name) return n;
    return NULL;
}

static xmlNodePtr find_element_by_attr(xmlNodePtr parent, const char *container, const char *tag, const char *attr, const char *value) {
    xmlNodePtr c = find_child(parent, container);
    if (!c) return NULL;
    EACH_ELEMENT(n, c, tag) {
        char *v = xml_prop_dup(n, attr);
        int ok = v && strcmp(v, value) == 0;
        free(v);
        if (ok) return n;
    }
    return NULL;
}

xmlNodePtr find_named(xmlNodePtr parent, const char *container, const char *tag, const char *name) {
    return find_element_by_attr(parent, container, tag, "name", name);
}

int has_child(xmlNodePtr parent, const char *name) {
    return find_child(parent, name) != NULL;
}

static char *join_path(const char *base, const char *rel) {
    if (!rel) return dupstr(base);
    if (rel[0] == '/') return dupstr(rel);
    const char *slash = strrchr(base, '/');
    size_t d = slash ? (size_t)(slash - base + 1) : 0;
    size_t n = d + strlen(rel) + 1;
    char *r = (char *)malloc(n);
    if (!r) die("oom");
    if (d) memcpy(r, base, d);
    strcpy(r + d, rel);
    return r;
}

module_t *load_module(const char *path, char ***seen, size_t *seen_count) {
    for (size_t i = 0; i < *seen_count; ++i)
        if (strcmp((*seen)[i], path) == 0) return NULL;

    *seen = (char **)realloc(*seen, (*seen_count + 1) * sizeof(char *));
    if (!*seen) die("oom");
    (*seen)[*seen_count] = dupstr(path);
    (*seen_count)++;

    xmlDocPtr doc = xmlReadFile(path, NULL, XML_PARSE_NONET | XML_PARSE_COMPACT);
    if (!doc) die("xml parse failed");
    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (!root || !xmlStrEqual(root->name, BAD_CAST("module"))) die("not a module xml");

    module_t *m = (module_t *)calloc(1, sizeof(*m));
    if (!m) die("oom");
    m->path = dupstr(path);
    m->doc = doc;
    m->root = root;

    EACH_ELEMENT(req, root, "require") {
        char *file = xml_prop_dup(req, "file");
        if (!file) continue;
        char *rp = join_path(path, file);
        module_t *sub = load_module(rp, seen, seen_count);
        if (sub) {
            m->requires = (module_t **)realloc(m->requires, (m->require_count + 1) * sizeof(module_t *));
            if (!m->requires) die("oom");
            m->requires[m->require_count++] = sub;
        }
        free(file);
        free(rp);
    }

    return m;
}

static xmlNodePtr lookup(module_t *m, const char *container, const char *tag, const char *attr, const char *name) {
    xmlNodePtr n = find_element_by_attr(m->root, container, tag, attr, name);
    if (n) return n;
    for (size_t i = 0; i < m->require_count; ++i) {
        n = lookup(m->requires[i], container, tag, attr, name);
        if (n) return n;
    }
    return NULL;
}

kind_t resolve_kind(module_t *m, const char *type) {
    if (!type) return KIND_UNKNOWN;
    if (strcmp(type, "float") == 0) return KIND_FLOAT;
    if (strcmp(type, "int") == 0) return KIND_INT;
    if (strcmp(type, "uint") == 0) return KIND_UINT;
    if (strcmp(type, "long") == 0) return KIND_LONG;
    if (strcmp(type, "bool") == 0) return KIND_BOOL;
    if (strcmp(type, "string") == 0) return KIND_STRING;
    if (strcmp(type, "nresults") == 0) return KIND_NRESULTS;
    if (strcmp(type, "void") == 0) return KIND_VOID;
    if (lookup(m, "enums", "enum", "name", type)) return KIND_ENUM;
    if (lookup(m, "structs", "struct", "name", type)) return KIND_STRUCT;
    if (lookup(m, "interfaces", "interface", "name", type)) return KIND_INTERFACE;
    if (lookup(m, "classes", "class", "name", type)) return KIND_COMPONENT;
    /* externals are keyed by 'struct', not 'name' */
    if (lookup(m, "externals", "external", "struct", type)) return KIND_EXTERNAL_STRUCT;
    return KIND_UNKNOWN;
}

const type_rule_t *rule_for(kind_t kind) {
    const char *name = NULL;
    switch (kind) {
        case KIND_FLOAT: name = "float"; break;
        case KIND_INT: name = "int"; break;
        case KIND_UINT: name = "uint"; break;
        case KIND_LONG: name = "long"; break;
        case KIND_BOOL: name = "bool"; break;
        case KIND_STRING: name = "string"; break;
        case KIND_ENUM: name = "enum"; break;
        case KIND_STRUCT: name = "struct"; break;
        case KIND_INTERFACE: name = "interface"; break;
        case KIND_COMPONENT: name = "component"; break;
        case KIND_EXTERNAL_STRUCT: name = "external_struct"; break;
        case KIND_NRESULTS: name = "nresults"; break;
        case KIND_VOID: name = "void"; break;
        default: return NULL;
    }

    for (size_t i = 0; i < sizeof(k_rules) / sizeof(k_rules[0]); ++i)
        if (strcmp(k_rules[i].kind_name, name) == 0) return &k_rules[i];

    return NULL;
}

void subst2(char *dst, size_t cap, const char *tmpl, const char *type, const char *arg) {
    dst[0] = 0;
    const char *p = tmpl ? tmpl : "";
    size_t len = 0;
    while (*p) {
        if (strncmp(p, "{type}", 6) == 0) {
            const char *src = type ? type : "";
            size_t n = strlen(src);
            if (len + n >= cap) n = (cap > len + 1) ? cap - len - 1 : 0;
            if (n) memcpy(dst + len, src, n);
            len += n;
            dst[len] = 0;
            p += 6;
        } else if (strncmp(p, "{arg}", 5) == 0) {
            const char *src = arg ? arg : "";
            size_t n = strlen(src);
            if (len + n >= cap) n = (cap > len + 1) ? cap - len - 1 : 0;
            if (n) memcpy(dst + len, src, n);
            len += n;
            dst[len] = 0;
            p += 5;
        } else {
            if (len + 1 < cap) dst[len++] = *p;
            p++;
            dst[len] = 0;
        }
    }
}

char *lcfirst_dup(const char *s) {
    char *r = dupstr(s ? s : "");
    if (r[0]) r[0] = (char)tolower((unsigned char)r[0]);
    return r;
}

uint32_t fnv1a32(const char *s) {
    uint32_t h = 0x811c9dc5u;
    while (s && *s) {
        h ^= (uint8_t)*s++;
        h *= 0x01000193u;
    }
    return h;
}

char *prop_token(const char *path) {
    char *tmp = dupstr(path ? path : "");
    for (size_t i = 0; tmp[i]; ++i)
        if (i == 0 || tmp[i - 1] == '.' || tmp[i - 1] == '[')
            tmp[i] = (char)toupper((unsigned char)tmp[i]);

    char *out = (char *)malloc(strlen(tmp) + 1);
    if (!out) die("oom");
    size_t j = 0;
    for (size_t i = 0; tmp[i]; ++i)
        if (tmp[i] != '.' && tmp[i] != '[' && tmp[i] != ']')
            out[j++] = tmp[i];
    out[j] = 0;
    free(tmp);
    return out;
}

const char *dtype_for(kind_t kind, const char *type) {
    switch (kind) {
        case KIND_FLOAT: return "kDataTypeFloat";
        case KIND_INT: return "kDataTypeInt";
        case KIND_UINT: return "kDataTypeInt";
        case KIND_BOOL: return "kDataTypeBool";
        case KIND_STRING: return "kDataTypeString";
        case KIND_ENUM: return "kDataTypeEnum";
        case KIND_STRUCT: return (type && strcmp(type, "color") == 0) ? "kDataTypeColor" : "kDataTypeStruct";
        case KIND_INTERFACE:
        case KIND_COMPONENT: return "kDataTypeObject";
        case KIND_EXTERNAL_STRUCT: return "kDataTypeStruct";
        default: return "kDataTypeNone";
    }
}

char *export_type_name(module_t *m, const char *type, kind_t kind) {
    if (kind == KIND_STRUCT) {
        xmlNodePtr st = lookup(m, "structs", "struct", "name", type);
        if (st) {
            char *exp = xml_prop_dup(st, "export");
            if (exp && exp[0]) return exp;
            free(exp);
        }
    }
    return dupstr(type ? type : "");
}

void free_seen(char **seen, size_t seen_count) {
    for (size_t i = 0; i < seen_count; ++i) free(seen[i]);
    free(seen);
}
