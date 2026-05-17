#include <ctype.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define OUT(...) fprintf(out, __VA_ARGS__)
#define LINE(...) do { fprintf(out, __VA_ARGS__); fputc('\n', out); } while (0)
#define EACH_ELEMENT(var, parent, tag) \
    for (xmlNodePtr var = (parent) ? (parent)->children : NULL; var; var = var->next) \
        if (var->type == XML_ELEMENT_NODE && ((tag) == NULL || xmlStrEqual(var->name, BAD_CAST(tag))))

typedef struct {
    const char *key;
    const char *value;
} kv_t;

typedef struct module_t {
    char *path;
    xmlDocPtr doc;
    xmlNodePtr root;
    struct module_t **requires;
    size_t require_count;
} module_t;

typedef enum {
    KIND_FLOAT, KIND_INT, KIND_UINT, KIND_LONG, KIND_BOOL, KIND_STRING,
    KIND_ENUM, KIND_STRUCT, KIND_INTERFACE, KIND_COMPONENT, KIND_EXTERNAL_STRUCT,
    KIND_NRESULTS, KIND_VOID, KIND_UNKNOWN
} kind_t;

typedef struct {
    const char *kind_name;
    const char *decl;
    const char *check;
    const char *push;
} type_rule_t;

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

static void die(const char *msg) {
    fprintf(stderr, "codegen_export: %s\n", msg);
    exit(1);
}

static char *dupstr(const char *s) {
    size_t n = s ? strlen(s) : 0;
    char *r = (char *)malloc(n + 1);
    if (!r) die("oom");
    if (n) memcpy(r, s, n);
    r[n] = 0;
    return r;
}

static char *xml_prop_dup(xmlNodePtr n, const char *name) {
    xmlChar *v = xmlGetProp(n, BAD_CAST(name));
    if (!v) return NULL;
    char *s = dupstr((const char *)v);
    xmlFree(v);
    return s;
}

static int xml_prop_true(xmlNodePtr n, const char *name) {
    char *v = xml_prop_dup(n, name);
    int ok = v && strcmp(v, "true") == 0;
    free(v);
    return ok;
}

static void emit_template(FILE *out, const char *tmpl, const kv_t *kvs, size_t kv_count) {
    const char *p = tmpl;
    while (*p) {
        const char *s = strstr(p, "{{");
        if (!s) { fputs(p, out); return; }
        fwrite(p, 1, (size_t)(s - p), out);
        const char *e = strstr(s + 2, "}}");
        if (!e) { fputs(s, out); return; }
        size_t n = (size_t)(e - (s + 2));
        char key[64];
        if (n >= sizeof(key)) n = sizeof(key) - 1;
        memcpy(key, s + 2, n);
        key[n] = 0;
        const char *val = "";
        for (size_t i = 0; i < kv_count; ++i) {
            if (strcmp(kvs[i].key, key) == 0) { val = kvs[i].value ? kvs[i].value : ""; break; }
        }
        fputs(val, out);
        p = e + 2;
    }
}

static xmlNodePtr find_child(xmlNodePtr parent, const char *name) {
    EACH_ELEMENT(n, parent, name) return n;
    return NULL;
}

static xmlNodePtr find_named(xmlNodePtr parent, const char *container, const char *tag, const char *name) {
    xmlNodePtr c = find_child(parent, container);
    EACH_ELEMENT(n, c, tag) {
        char *v = xml_prop_dup(n, "name");
        int ok = v && strcmp(v, name) == 0;
        free(v);
        if (ok) return n;
    }
    return NULL;
}

static int has_child(xmlNodePtr parent, const char *name) {
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

static module_t *load_module(const char *path, char ***seen, size_t *seen_count) {
    for (size_t i = 0; i < *seen_count; ++i) if (strcmp((*seen)[i], path) == 0) return NULL;
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

static xmlNodePtr lookup(module_t *m, const char *container, const char *tag, const char *name) {
    xmlNodePtr n = find_named(m->root, container, tag, name);
    if (n) return n;
    for (size_t i = 0; i < m->require_count; ++i) {
        n = lookup(m->requires[i], container, tag, name);
        if (n) return n;
    }
    return NULL;
}

static kind_t resolve_kind(module_t *m, const char *type) {
    if (!type) return KIND_UNKNOWN;
    if (strcmp(type, "float") == 0) return KIND_FLOAT;
    if (strcmp(type, "int") == 0) return KIND_INT;
    if (strcmp(type, "uint") == 0) return KIND_UINT;
    if (strcmp(type, "long") == 0) return KIND_LONG;
    if (strcmp(type, "bool") == 0) return KIND_BOOL;
    if (strcmp(type, "string") == 0) return KIND_STRING;
    if (strcmp(type, "nresults") == 0) return KIND_NRESULTS;
    if (strcmp(type, "void") == 0) return KIND_VOID;
    if (lookup(m, "enums", "enum", type)) return KIND_ENUM;
    if (lookup(m, "structs", "struct", type)) return KIND_STRUCT;
    if (lookup(m, "interfaces", "interface", type)) return KIND_INTERFACE;
    if (lookup(m, "classes", "class", type)) return KIND_COMPONENT;
    if (lookup(m, "externals", "external", type)) return KIND_EXTERNAL_STRUCT;
    return KIND_UNKNOWN;
}

static const type_rule_t *rule_for(kind_t kind) {
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
    for (size_t i = 0; i < sizeof(k_rules) / sizeof(k_rules[0]); ++i) {
        if (strcmp(k_rules[i].kind_name, name) == 0) return &k_rules[i];
    }
    return NULL;
}

static void subst2(char *dst, size_t cap, const char *tmpl, const char *type, const char *arg) {
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

static char *lcfirst_dup(const char *s) {
    char *r = dupstr(s ? s : "");
    if (r[0]) r[0] = (char)tolower((unsigned char)r[0]);
    return r;
}

static uint32_t fnv1a32(const char *s) {
    uint32_t h = 0x811c9dc5u;
    while (s && *s) { h ^= (uint8_t)*s++; h *= 0x01000193u; }
    return h;
}

static char *prop_token(const char *path) {
    char *tmp = dupstr(path ? path : "");
    for (size_t i = 0; tmp[i]; ++i) if (i == 0 || tmp[i - 1] == '.' || tmp[i - 1] == '[') tmp[i] = (char)toupper((unsigned char)tmp[i]);
    char *out = (char *)malloc(strlen(tmp) + 1);
    if (!out) die("oom");
    size_t j = 0;
    for (size_t i = 0; tmp[i]; ++i) if (tmp[i] != '.' && tmp[i] != '[' && tmp[i] != ']') out[j++] = tmp[i];
    out[j] = 0;
    free(tmp);
    return out;
}

static const char *dtype_for(kind_t kind, const char *type) {
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

static char *export_type_name(module_t *m, const char *type, kind_t kind) {
    if (kind == KIND_STRUCT) {
        xmlNodePtr st = lookup(m, "structs", "struct", type);
        if (st) {
            char *exp = xml_prop_dup(st, "export");
            if (exp && exp[0]) return exp;
            free(exp);
        }
    }
    return dupstr(type ? type : "");
}

static void emit_method_wrapper(FILE *out, module_t *m, const char *prefix, xmlNodePtr method, const char *owner_name) {
    char *name = xml_prop_dup(method, "name");
    if (!name) return;
    LINE("int f_%s%s(lua_State *L) {", prefix, name);

    char *args[64] = {0};
    int argc = 0;
    int idx = 1;

    if (owner_name && !xml_prop_true(method, "static")) {
        LINE("\tstruct %s%s* this_ = luaX_check%s(L, %d);", owner_name, xml_prop_true(method, "const") ? " const" : "", owner_name, idx++);
        if (argc >= (int)(sizeof(args) / sizeof(args[0]))) die("too many args");
        args[argc++] = dupstr("this_");
    }
    if (xml_prop_true(method, "lua")) {
        if (argc >= (int)(sizeof(args) / sizeof(args[0]))) die("too many args");
        args[argc++] = dupstr("L");
    }

    EACH_ELEMENT(arg, method, "arg") {
        char *an = xml_prop_dup(arg, "name");
        char *at = xml_prop_dup(arg, "type");
        char *output = xml_prop_dup(arg, "output");
        char *bufsize = xml_prop_dup(arg, "bufsize");
        kind_t k = resolve_kind(m, at);
        const type_rule_t *r = rule_for(k);
        if (output && output[0]) {
            LINE("\t%s %s = {0};", output, an);
        } else if (bufsize && bufsize[0]) {
            LINE("\tint32_t %s = sizeof(%s);", an, bufsize);
        } else {
            char decl[256], check[256], idxs[32];
            subst2(decl, sizeof(decl), r ? r->decl : "int", at, NULL);
            if (xml_prop_true(arg, "const")) strncat(decl, " const", sizeof(decl) - strlen(decl) - 1);
            if (xml_prop_true(arg, "pointer") || xml_prop_true(arg, "array")) strncat(decl, "*", sizeof(decl) - strlen(decl) - 1);
            snprintf(idxs, sizeof(idxs), "%d", idx++);
            subst2(check, sizeof(check), (r && r->check) ? r->check : "0", at, idxs);
            LINE("\t%s %s = %s;", decl, an, check);
        }
        if (argc >= (int)(sizeof(args) / sizeof(args[0]))) die("too many args");
        args[argc++] = dupstr(an);
        free(an); free(at); free(output); free(bufsize);
    }

    xmlNodePtr ret = find_child(method, "returns");
    char *rt = ret ? xml_prop_dup(ret, "type") : NULL;
    kind_t rk = resolve_kind(m, rt ? rt : "void");
    const type_rule_t *rr = rule_for(rk);

    OUT("\t");
    if (!ret || rk == KIND_VOID) {
        OUT("%s%s(", prefix, name);
    } else if (rk == KIND_NRESULTS) {
        OUT("return %s%s(", prefix, name);
    } else {
        char decl[256];
        subst2(decl, sizeof(decl), rr ? rr->decl : "int", rt, NULL);
        OUT("%s result_ = %s%s(", decl, prefix, name);
    }
    for (int i = 0; i < argc; ++i) OUT("%s%s", i ? ", " : "", args[i]);
    OUT(");\n");
    if (ret && rk != KIND_VOID && rk != KIND_NRESULTS) {
        char push[256];
        subst2(push, sizeof(push), rr && rr->push ? rr->push : "lua_pushnil(L)", rt, "result_");
        LINE("\t%s;", push);
        LINE("\treturn 1;");
    } else if (!ret || rk == KIND_VOID) {
        LINE("\treturn 0;");
    }
    LINE("}");

    for (int i = 0; i < argc; ++i) free(args[i]);
    free(name);
    free(rt);
}

static void emit_struct(FILE *out, module_t *m, xmlNodePtr st) {
    char *name = xml_prop_dup(st, "name");
    char *prefix = xml_prop_dup(st, "prefix");
    char *exp = xml_prop_dup(st, "export");
    if (!name) return;
    if (!prefix) prefix = dupstr(name);
    if (!exp) exp = dupstr(name);

    xmlNodePtr methods = find_child(st, "methods");
    EACH_ELEMENT(method, methods, "method") emit_method_wrapper(out, m, prefix, method, name);

    LINE("static struct PropertyType _%s[] = {", name);
    xmlNodePtr fields = find_child(st, "fields");
    EACH_ELEMENT(field, fields, "field") {
        char *fname = xml_prop_dup(field, "name");
        char *ftype = xml_prop_dup(field, "type");
        char *farr = xml_prop_dup(field, "fixed-array");
        int fixed = farr ? atoi(farr) : 0;
        int is_array = xml_prop_true(field, "array");
        if (!fname || !ftype) { free(fname); free(ftype); free(farr); continue; }
        if (fixed > 0) {
            for (int i = 0; i < fixed; ++i) {
                char tok_raw[512], addr[512];
                if (snprintf(tok_raw, sizeof(tok_raw), "%s[%d]", fname, i) >= (int)sizeof(tok_raw) ||
                    snprintf(addr, sizeof(addr), "%s[%d]", fname, i) >= (int)sizeof(addr)) {
                    die("field path too long");
                }
                char *tok = prop_token(tok_raw);
                kind_t k = resolve_kind(m, ftype);
                uint32_t id = fnv1a32(tok);
                const char *dt = dtype_for(k, ftype);
                char *tname = export_type_name(m, ftype, k);
                OUT("\tDECL(0x%08x, %s, %s, %s, %s", id, name, tok, addr, dt);
                if (k == KIND_ENUM) OUT(", .EnumValues = _%s", ftype);
                else if (k == KIND_STRUCT && strcmp(ftype, "color") != 0) OUT(", .TypeString = \"%s\"", tname);
                OUT("), // %s.%s\n", name, tok);
                free(tname);
                free(tok);
            }
        } else {
            char *tok = prop_token(fname);
            kind_t k = resolve_kind(m, ftype);
            uint32_t id = fnv1a32(tok);
            const char *dt = dtype_for(k, ftype);
            char *tname = export_type_name(m, ftype, k);
            OUT("\t%s(0x%08x, %s, %s, %s, %s", is_array ? "ARRAY_DECL" : "DECL", id, name, tok, fname, dt);
            if (k == KIND_ENUM) OUT(", .EnumValues = _%s", ftype);
            else if (k == KIND_STRUCT && strcmp(ftype, "color") != 0) OUT(", .TypeString = \"%s\"", tname);
            OUT("), // %s.%s\n", name, tok);
            free(tname);
            free(tok);
        }
        free(fname); free(ftype); free(farr);
    }
    LINE("};");

    LINE("static luaL_Reg _%s_Methods[] = {", name);
    EACH_ELEMENT(method, methods, "method") {
        char *mn = xml_prop_dup(method, "name");
        char *me = xml_prop_dup(method, "export");
        if (!mn) { free(me); continue; }
        if (!me) me = lcfirst_dup(mn);
        LINE("\t{ \"%s\", f_%s%s },", me, prefix, mn);
        free(mn); free(me);
    }
    LINE("\t{ NULL, NULL }");
    LINE("};");
    LINE("STRUCT(%s, %s);", name, exp);

    free(name); free(prefix); free(exp);
}

int main(int argc, char **argv) {
    if (argc != 2) die("usage: codegen_export <module.xml>");
    xmlInitParser();

    char **seen = NULL;
    size_t seen_count = 0;
    module_t *m = load_module(argv[1], &seen, &seen_count);
    if (!m) die("failed to load module");

    /* Current C emitter intentionally targets struct-heavy modules first.
       Fall back to legacy PHP export template when unsupported sections exist. */
    if (has_child(m->root, "interfaces") || has_child(m->root, "classes")) {
        char *const pyphp_argv[] = {"python3", "-m", "pyphp.pyphp", "templates/export.php", argv[1], NULL};
        execvp("python3", pyphp_argv);
        die("failed to exec pyphp fallback exporter");
    }

    FILE *out = stdout;
    char *mod = xml_prop_dup(m->root, "name");
    const char *base = strrchr(argv[1], '/');
    base = base ? base + 1 : argv[1];
    kv_t hdr[] = {{"xml", base}, {"module", mod}};
    emit_template(out,
        "// Auto-generated from {{xml}} by tools/codegen_export.c\n"
        "// DO NOT EDIT — run 'cd tools && make' to regenerate.\n"
        "#include <include/api.h>\n"
        "#include <include/codegen.h>\n\n"
        "#include \"{{module}}.h\"\n\n",
        hdr, 2);

    xmlNodePtr exts = find_child(m->root, "externals");
    EACH_ELEMENT(ex, exts, "external") {
        char *name = xml_prop_dup(ex, "struct");
        if (!name) continue;
        LINE("// %s", name);
        LINE("extern void luaX_push%s(lua_State *L, struct %s const* value);", name, name);
        LINE("extern struct %s* luaX_check%s(lua_State *L, int index);", name, name);
        free(name);
    }

    xmlNodePtr enums = find_child(m->root, "enums");
    EACH_ELEMENT(en, enums, "enum") {
        char *name = xml_prop_dup(en, "name");
        if (!name) continue;
        OUT("ENUM(%s", name);
        EACH_ELEMENT(v, en, "value") {
            char *vn = xml_prop_dup(v, "name");
            if (vn) OUT(", \"%s\"", vn);
            free(vn);
        }
        OUT(")\n");
        free(name);
    }

    LINE("extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* struct_ptr);");
    LINE("extern int write_property(lua_State *L, struct PropertyType const* prop, void const* struct_ptr);");
    LINE("extern int parse_property(const char* str, struct PropertyType const* prop, void* struct_ptr);");

    xmlNodePtr structs = find_child(m->root, "structs");
    EACH_ELEMENT(st, structs, "struct") emit_struct(out, m, st);

    char *prefix = xml_prop_dup(m->root, "prefix");
    if (!prefix) {
        prefix = (char *)malloc(strlen(mod) + 2);
        if (!prefix) die("oom");
        sprintf(prefix, "%s_", mod);
    }
    xmlNodePtr functions = find_child(m->root, "functions");
    EACH_ELEMENT(fn, functions, "function") emit_method_wrapper(out, m, prefix, fn, NULL);

    LINE("ORCA_API int luaopen_orca_%s(lua_State *L) {", mod);
    LINE("\tluaL_newlib(L, ((luaL_Reg[]) {");
    EACH_ELEMENT(fn, functions, "function") {
        char *name = xml_prop_dup(fn, "name");
        char *exp = xml_prop_dup(fn, "export");
        if (!name) { free(exp); continue; }
        if (!exp) exp = lcfirst_dup(name);
        LINE("\t\t{ \"%s\", f_%s%s },", exp, prefix, name);
        free(name); free(exp);
    }
    LINE("\t\t{ NULL, NULL }");
    LINE("\t}));");

    EACH_ELEMENT(st, structs, "struct") {
        char *name = xml_prop_dup(st, "name");
        char *exp = xml_prop_dup(st, "export");
        if (!name) { free(exp); continue; }
        if (!exp) exp = dupstr(name);
        LINE("\tlua_setfield(L, ((void)luaopen_orca_%s(L), -2), \"%s\");", name, exp);
        free(name); free(exp);
    }
    LINE("\treturn 1;");
    LINE("}");

    free(prefix);
    free(mod);
    for (size_t i = 0; i < seen_count; ++i) free(seen[i]);
    free(seen);
    xmlCleanupParser();
    return 0;
}
