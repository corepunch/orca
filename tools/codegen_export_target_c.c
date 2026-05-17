#include "codegen_export_backend.h"

#include <stdlib.h>
#include <string.h>

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
        free(an);
        free(at);
        free(output);
        free(bufsize);
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
        if (!fname || !ftype) {
            free(fname);
            free(ftype);
            free(farr);
            continue;
        }
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
        free(fname);
        free(ftype);
        free(farr);
    }
    LINE("};");

    LINE("static luaL_Reg _%s_Methods[] = {", name);
    EACH_ELEMENT(method, methods, "method") {
        char *mn = xml_prop_dup(method, "name");
        char *me = xml_prop_dup(method, "export");
        if (!mn) {
            free(me);
            continue;
        }
        if (!me) me = lcfirst_dup(mn);
        LINE("\t{ \"%s\", f_%s%s },", me, prefix, mn);
        free(mn);
        free(me);
    }
    LINE("\t{ NULL, NULL }");
    LINE("};");
    LINE("STRUCT(%s, %s);", name, exp);

    free(name);
    free(prefix);
    free(exp);
}

int emit_target_c(FILE *out, module_t *m, const char *xml_path) {
    char *mod = xml_prop_dup(m->root, "name");
    const char *base = strrchr(xml_path, '/');
    base = base ? base + 1 : xml_path;
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
        if (!name) {
            free(exp);
            continue;
        }
        if (!exp) exp = lcfirst_dup(name);
        LINE("\t\t{ \"%s\", f_%s%s },", exp, prefix, name);
        free(name);
        free(exp);
    }
    LINE("\t\t{ NULL, NULL }");
    LINE("\t}));");

    EACH_ELEMENT(st, structs, "struct") {
        char *name = xml_prop_dup(st, "name");
        char *exp = xml_prop_dup(st, "export");
        if (!name) {
            free(exp);
            continue;
        }
        if (!exp) exp = dupstr(name);
        LINE("\tlua_setfield(L, ((void)luaopen_orca_%s(L), -2), \"%s\");", name, exp);
        free(name);
        free(exp);
    }
    LINE("\treturn 1;");
    LINE("}");

    free(prefix);
    free(mod);
    return 0;
}
