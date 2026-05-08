/*
 * model.c — XML parsing + data model helpers for ORCA codegen
 */
#include "model.h"
#include <assert.h>
#include <ctype.h>
#include <regex.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

/* ------------------------------------------------------------------ helpers */

char *str_dup(const char *s) {
    return s ? strdup(s) : NULL;
}

char *str_ucfirst(const char *s) {
    if (!s || !*s) return str_dup(s);
    char *r = strdup(s);
    r[0] = (char)toupper((unsigned char)r[0]);
    return r;
}

char *str_lcfirst(const char *s) {
    if (!s || !*s) return str_dup(s);
    char *r = strdup(s);
    r[0] = (char)tolower((unsigned char)r[0]);
    return r;
}

char *str_toupper(const char *s) {
    if (!s) return NULL;
    char *r = strdup(s);
    for (char *p = r; *p; p++) *p = (char)toupper((unsigned char)*p);
    return r;
}

/* ------------------------------------------------------------------ FNV-1a 32 */

uint32_t fnv1a32(const char *s) {
    uint32_t h = 0x811c9dc5u;
    while (*s) {
        h ^= (uint8_t)*s++;
        h *= 0x01000193u;
    }
    return h;
}

char *fnv1a32hex(const char *s) {
    char buf[16];
    snprintf(buf, sizeof(buf), "0x%08x", fnv1a32(s));
    return strdup(buf);
}

/* ------------------------------------------------------------------ Axis rename */

/* Strip all '[', ']', '.' from a string (in-place on heap copy, returned) */
static char *strip_brackets_dots(const char *s) {
    char *r = malloc(strlen(s) + 1);
    char *w = r;
    for (const char *p = s; *p; p++) {
        if (*p != '[' && *p != ']' && *p != '.') *w++ = *p;
    }
    *w = '\0';
    return r;
}

/*
 * axis_rename: apply PHP $Axis patterns to input string, then strip [].
 * Returns malloc'd result.
 *
 * Patterns translate PHP PCRE → POSIX ERE:
 *   Patterns 6-8 use (?:Requested)?(.*)  → (Requested)?(.*)
 *     PHP group 1 = (.*) → POSIX group 2
 *   All others: same group numbering
 */

typedef struct {
    const char *pattern;  /* POSIX ERE */
    /* Replacement: seq of (literal, group_index) where group_index<0 = no group */
    /* Max 4 parts alternating literal+group */
    const char *lit[4];
    int         grp[4];   /* -1 = no group ref after this literal */
    int         n_parts;
} AxisPat;

static AxisPat g_axis_pats[] = {
    /* 0 */ {"(.+)\\.Axis\\[0\\]\\.Left(.*)",  {"", "Left", "", NULL}, {1,  -1,  2, -1}, 3},
    /* 1 */ {"(.+)\\.Axis\\[0\\]\\.Right(.*)", {"", "Right","", NULL}, {1,  -1,  2, -1}, 3},
    /* 2 */ {"(.+)\\.Axis\\[1\\]\\.Left(.*)",  {"", "Top",  "", NULL}, {1,  -1,  2, -1}, 3},
    /* 3 */ {"(.+)\\.Axis\\[1\\]\\.Right(.*)", {"", "Bottom","",NULL}, {1,  -1,  2, -1}, 3},
    /* 4 */ {"(.+)\\.Axis\\[2\\]\\.Left(.*)",  {"", "Front","", NULL}, {1,  -1,  2, -1}, 3},
    /* 5 */ {"(.+)\\.Axis\\[2\\]\\.Right(.*)", {"", "Back", "", NULL}, {1,  -1,  2, -1}, 3},
    /* 6 */ {"Size.Axis\\[0\\]\\.(Requested)?(.*)", {"", "Width", NULL,NULL}, {2, -1, -1,-1}, 2},
    /* 7 */ {"Size.Axis\\[1\\]\\.(Requested)?(.*)", {"", "Height",NULL,NULL}, {2, -1, -1,-1}, 2},
    /* 8 */ {"Size.Axis\\[2\\]\\.(Requested)?(.*)", {"", "Depth", NULL,NULL}, {2, -1, -1,-1}, 2},
    /* 9 */ {"(.+)\\.Axis\\[0\\]",             {"Horizontal","",NULL,NULL}, {-1, 1, -1,-1}, 2},
    /*10 */ {"(.+)\\.Axis\\[1\\]",             {"Vertical",  "",NULL,NULL}, {-1, 1, -1,-1}, 2},
    /*11 */ {"(.+)\\.Axis\\[2\\]",             {"Depth",     "",NULL,NULL}, {-1, 1, -1,-1}, 2},
    /*12 */ {"Border\\.Radius\\.(.+)Radius",   {"Border","Radius",NULL,NULL},{-1, 1, -1,-1}, 2},
    /*13 */ {"Axis\\[0\\]\\.Left(.*)",          {"Left", "",  NULL,NULL}, {-1, 1, -1,-1}, 2},
    /*14 */ {"Axis\\[0\\]\\.Right(.*)",         {"Right","",  NULL,NULL}, {-1, 1, -1,-1}, 2},
    /*15 */ {"Axis\\[1\\]\\.Left(.*)",          {"Top",  "",  NULL,NULL}, {-1, 1, -1,-1}, 2},
    /*16 */ {"Axis\\[1\\]\\.Right(.*)",         {"Bottom","", NULL,NULL}, {-1, 1, -1,-1}, 2},
    /*17 */ {"Axis\\[2\\]\\.Left(.*)",          {"Front","",  NULL,NULL}, {-1, 1, -1,-1}, 2},
    /*18 */ {"Axis\\[2\\]\\.Right(.*)",         {"Back", "",  NULL,NULL}, {-1, 1, -1,-1}, 2},
    /*19 */ {"Axis\\[0\\]",                     {"Horizontal",NULL,NULL,NULL},{-1,-1,-1,-1},1},
    /*20 */ {"Axis\\[1\\]",                     {"Vertical",  NULL,NULL,NULL},{-1,-1,-1,-1},1},
    /*21 */ {"Axis\\[2\\]",                     {"Depth",     NULL,NULL,NULL},{-1,-1,-1,-1},1},
};
#define N_AXIS_PATS (int)(sizeof(g_axis_pats)/sizeof(g_axis_pats[0]))

char *axis_rename(const char *in) {
    static regex_t compiled[N_AXIS_PATS];
    static int      ready = 0;
    if (!ready) {
        for (int i = 0; i < N_AXIS_PATS; i++) {
            int r = regcomp(&compiled[i], g_axis_pats[i].pattern, REG_EXTENDED);
            if (r != 0) {
                char errbuf[128];
                regerror(r, &compiled[i], errbuf, sizeof(errbuf));
                fprintf(stderr, "axis_rename: regcomp error for pattern %d: %s\n", i, errbuf);
                exit(1);
            }
        }
        ready = 1;
    }

    regmatch_t rm[8];
    for (int i = 0; i < N_AXIS_PATS; i++) {
        if (regexec(&compiled[i], in, 8, rm, 0) != 0) continue;

        /* Build replacement from lit/grp pairs */
        char repl[1024] = "";
        AxisPat *ap = &g_axis_pats[i];
        for (int p = 0; p < ap->n_parts; p++) {
            if (ap->lit[p]) strncat(repl, ap->lit[p], sizeof(repl)-strlen(repl)-1);
            if (ap->grp[p] >= 0) {
                int g = ap->grp[p];
                if (rm[g].rm_so >= 0) {
                    int len = (int)(rm[g].rm_eo - rm[g].rm_so);
                    strncat(repl, in + rm[g].rm_so, (size_t)(len < (int)(sizeof(repl)-strlen(repl)-1) ? len : (int)(sizeof(repl)-strlen(repl)-1)));
                }
            }
        }

        /* Build: prefix + repl + suffix */
        size_t pre_len  = (size_t)rm[0].rm_so;
        size_t suf_len  = strlen(in) - (size_t)rm[0].rm_eo;
        size_t repl_len = strlen(repl);
        char *full = malloc(pre_len + repl_len + suf_len + 1);
        memcpy(full, in, pre_len);
        memcpy(full + pre_len, repl, repl_len);
        memcpy(full + pre_len + repl_len, in + rm[0].rm_eo, suf_len);
        full[pre_len + repl_len + suf_len] = '\0';

        char *result = strip_brackets_dots(full);
        free(full);
        return result;
    }
    return strip_brackets_dots(in);
}

/* ------------------------------------------------------------------ PropertyName */

PropertyName make_property_name(const char *classname, const char **path, int path_len) {
    PropertyName pn;
    pn.classname = str_dup(classname);
    pn.path = malloc((size_t)path_len * sizeof(char*));
    pn.path_len = path_len;
    for (int i = 0; i < path_len; i++) pn.path[i] = str_dup(path[i]);

    /* addr = join(path, '.') */
    size_t alen = 0;
    for (int i = 0; i < path_len; i++) alen += strlen(path[i]) + 1;
    char *addr = malloc(alen + 1);
    addr[0] = '\0';
    for (int i = 0; i < path_len; i++) {
        if (i > 0) strcat(addr, ".");
        strcat(addr, path[i]);
    }
    pn.addr = addr;

    /* display = ucfirst each segment, join with '.', axis_rename, strip */
    size_t joined_len = alen + path_len + 4;
    char *joined = malloc(joined_len);
    joined[0] = '\0';
    for (int i = 0; i < path_len; i++) {
        if (i > 0) strcat(joined, ".");
        char *uc = str_ucfirst(path[i]);
        strcat(joined, uc);
        free(uc);
    }
    pn.display = axis_rename(joined);
    free(joined);

    /* id = "0x" + fnv1a32(display) */
    char buf[16];
    snprintf(buf, sizeof(buf), "0x%08x", fnv1a32(pn.display));
    pn.id = strdup(buf);

    return pn;
}

void free_property_name(PropertyName *pn) {
    free(pn->classname);
    for (int i = 0; i < pn->path_len; i++) free(pn->path[i]);
    free(pn->path);
    free(pn->addr);
    free(pn->display);
    free(pn->id);
}

/* ------------------------------------------------------------------ type_decl */

char *type_decl(const Type *t) {
    char base[256] = "";
    switch (t->kind) {
        case TK_FLOAT:   strcpy(base, "float"); break;
        case TK_INT:     strcpy(base, "int32_t"); break;
        case TK_UINT:    strcpy(base, "uint32_t"); break;
        case TK_LONG:    strcpy(base, "long"); break;
        case TK_BOOL:    strcpy(base, "bool_t"); break;
        case TK_STRING:  strcpy(base, "const char*"); break;
        case TK_VOID:    snprintf(base, sizeof(base), "%s", t->type_name); break;
        case TK_LUA_STATE: strcpy(base, "lua_State"); break;
        case TK_STRARRAY: strcpy(base, "lpcString_t const*"); break;
        case TK_NRESULTS: strcpy(base, "int"); break;
        case TK_ENUM:
            snprintf(base, sizeof(base), "enum %s", t->type_name);
            break;
        case TK_STRUCT:
        case TK_INTERFACE:
        case TK_COMPONENT:
        case TK_EXTERNAL_STRUCT:
            snprintf(base, sizeof(base), "struct %s", t->type_name);
            break;
        default:
            snprintf(base, sizeof(base), "%s_t", t->type_name);
            break;
    }
    char result[512];
    snprintf(result, sizeof(result), "%s%s%s",
        base,
        t->is_const ? " const" : "",
        (t->is_pointer || t->is_array) ? "*" : "");
    return strdup(result);
}

/* type_get: get a TypeInfos entry value with {type}/{arg}/{addr} substitutions */
char *type_get(const Type *t, const char *key, const char *arg, const char *addr) {
    /* lookup templates */
    static const struct { const char *kind; const char *key; const char *tmpl; } infos[] = {
        {"float",   "check",   "luaL_checknumber(L, {arg})"},
        {"float",   "push",    "lua_pushnumber(L, {arg})"},
        {"float",   "default", "%s"},
        {"float",   "format",  "%f"},
        {"int",     "check",   "(int32_t)luaL_checkinteger(L, {arg})"},
        {"int",     "push",    "lua_pushinteger(L, {arg})"},
        {"int",     "default", "%s"},
        {"int",     "format",  "%d"},
        {"uint",    "check",   "(uint32_t)luaL_checkinteger(L, {arg})"},
        {"uint",    "push",    "lua_pushinteger(L, {arg})"},
        {"uint",    "default", "%s"},
        {"uint",    "format",  "%u"},
        {"long",    "check",   "luaL_checkinteger(L, {arg})"},
        {"long",    "push",    "lua_pushinteger(L, {arg})"},
        {"long",    "default", "%s"},
        {"long",    "format",  "%ld"},
        {"bool",    "check",   "lua_toboolean(L, {arg})"},
        {"bool",    "push",    "lua_pushboolean(L, {arg})"},
        {"bool",    "default", "%s"},
        {"bool",    "format",  "%s"},
        {"string",  "check",   "luaL_checkstring(L, {arg})"},
        {"string",  "push",    "lua_pushstring(L, {arg})"},
        {"string",  "default", "\"%s\""},
        {"string",  "format",  "%s"},
        {"enum",    "check",   "luaX_check{type}(L, {arg})"},
        {"enum",    "push",    "luaX_push{type}(L, {arg})"},
        {"enum",    "default", "k{type}%s"},
        {"enum",    "format",  "%s"},
        {"struct",  "check",   "luaX_check{type}(L, {arg})"},
        {"struct",  "push",    "luaX_push{type}(L, &{arg})"},
        {"struct",  "default", "{%s}"},
        {"struct",  "format",  "%s"},
        {"interface","check",  "luaX_check{type}(L, {arg})"},
        {"interface","push",   "luaX_push{type}(L, {arg})"},
        {"interface","default","{%s}"},
        {"interface","format", "%s"},
        {"component","check",  "luaX_check{type}(L, {arg})"},
        {"component","push",   "luaX_push{type}(L, {arg})"},
        {"component","default","{%s}"},
        {"component","format", "%s"},
        {"external_struct","push","luaX_push{type}(L, {arg})"},
        {"external_struct","default","{%s}"},
        {"nresults","push",    "lua_pushnil(L)"},
        {"nresults","return",  "return {arg};"},
        {"void",    "return",  "{arg};\n\treturn 0;"},
        {NULL, NULL, NULL}
    };

    /* map kind enum to string */
    const char *kstr = NULL;
    switch (t->kind) {
        case TK_FLOAT:   kstr = "float"; break;
        case TK_INT:     kstr = "int"; break;
        case TK_UINT:    kstr = "uint"; break;
        case TK_LONG:    kstr = "long"; break;
        case TK_BOOL:    kstr = "bool"; break;
        case TK_STRING:  kstr = "string"; break;
        case TK_ENUM:    kstr = "enum"; break;
        case TK_STRUCT:  kstr = "struct"; break;
        case TK_INTERFACE: kstr = "interface"; break;
        case TK_COMPONENT: kstr = "component"; break;
        case TK_EXTERNAL_STRUCT: kstr = "external_struct"; break;
        case TK_NRESULTS: kstr = "nresults"; break;
        case TK_VOID:    kstr = "void"; break;
        default: kstr = ""; break;
    }

    const char *tmpl = NULL;
    for (int i = 0; infos[i].kind; i++) {
        if (strcmp(infos[i].kind, kstr) == 0 && strcmp(infos[i].key, key) == 0) {
            tmpl = infos[i].tmpl;
            break;
        }
    }
    if (!tmpl) return strdup("");

    /* handle push for struct+pointer: prepend '*' to arg */
    char real_arg[256];
    if (t->kind == TK_STRUCT && t->is_pointer && strcmp(key,"push")==0) {
        snprintf(real_arg, sizeof(real_arg), "*%s", arg ? arg : "");
        arg = real_arg;
    }

    /* substitute {type}, {arg}, {addr} */
    char result[2048] = "";
    const char *p = tmpl;
    while (*p) {
        if (p[0]=='{' && p[1]=='t' && strncmp(p,"{type}",6)==0) {
            strncat(result, t->type_name ? t->type_name : "", sizeof(result)-strlen(result)-1);
            p += 6;
        } else if (p[0]=='{' && p[1]=='a' && strncmp(p,"{arg}",5)==0) {
            strncat(result, arg ? arg : "", sizeof(result)-strlen(result)-1);
            p += 5;
        } else if (p[0]=='{' && p[1]=='a' && strncmp(p,"{addr}",6)==0) {
            strncat(result, addr ? addr : "", sizeof(result)-strlen(result)-1);
            p += 6;
        } else {
            char ch[2] = {*p++, '\0'};
            strncat(result, ch, sizeof(result)-strlen(result)-1);
        }
    }
    return strdup(result);
}

/* ------------------------------------------------------------------ XML helpers */

static char *xget_prop(xmlNodePtr node, const char *name) {
    xmlChar *val = xmlGetProp(node, (xmlChar*)name);
    if (!val) return NULL;
    char *r = strdup((char*)val);
    xmlFree(val);
    return r;
}

static char *xget_text(xmlNodePtr node) {
    xmlChar *val = xmlNodeGetContent(node);
    if (!val) return NULL;
    /* trim */
    char *s = (char*)val;
    while (*s && isspace((unsigned char)*s)) s++;
    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)*(end-1))) end--;
    char *r = (end > s) ? strndup(s, (size_t)(end-s)) : strdup("");
    xmlFree(val);
    return (strlen(r) > 0) ? r : (free(r), (char*)NULL);
}

static xmlNodePtr xfirst_child(xmlNodePtr node, const char *tag) {
    for (xmlNodePtr c = node->children; c; c = c->next)
        if (c->type == XML_ELEMENT_NODE && strcmp((char*)c->name, tag) == 0) return c;
    return NULL;
}

/* ------------------------------------------------------------------ Type resolution */

TypeKind resolve_kind(Module *m, const char *type_name, void **data_out) {
    if (!type_name) { if(data_out)*data_out=NULL; return TK_VOID; }

    /* search enums */
    for (int i = 0; i < m->num_enums; i++) {
        if (strcmp(m->enums[i].name, type_name) == 0) {
            if (data_out) *data_out = &m->enums[i];
            return TK_ENUM;
        }
    }
    /* search structs */
    for (int i = 0; i < m->num_structs; i++) {
        if (strcmp(m->structs[i].name, type_name) == 0) {
            if (data_out) *data_out = &m->structs[i];
            return TK_STRUCT;
        }
    }
    /* search interfaces */
    for (int i = 0; i < m->num_interfaces; i++) {
        if (strcmp(m->interfaces[i].name, type_name) == 0) {
            if (data_out) *data_out = &m->interfaces[i];
            return TK_INTERFACE;
        }
    }
    /* search components */
    for (int i = 0; i < m->num_components; i++) {
        if (strcmp(m->components[i].name, type_name) == 0) {
            if (data_out) *data_out = &m->components[i];
            return TK_COMPONENT;
        }
    }
    /* search externals */
    for (int i = 0; i < m->num_externals; i++) {
        if (strcmp(m->externals[i].name, type_name) == 0) {
            if (data_out) *data_out = &m->externals[i];
            return TK_EXTERNAL_STRUCT;
        }
    }
    /* recurse into requires */
    for (int i = 0; i < m->num_requires; i++) {
        void *d = NULL;
        TypeKind k = resolve_kind(m->requires[i], type_name, &d);
        if (k != TK_UNKNOWN) {
            if (data_out) *data_out = d;
            return k;
        }
    }
    /* primitive types */
    if (strcmp(type_name,"float")==0)  { if(data_out)*data_out=NULL; return TK_FLOAT; }
    if (strcmp(type_name,"int")==0)    { if(data_out)*data_out=NULL; return TK_INT; }
    if (strcmp(type_name,"uint")==0)   { if(data_out)*data_out=NULL; return TK_UINT; }
    if (strcmp(type_name,"long")==0)   { if(data_out)*data_out=NULL; return TK_LONG; }
    if (strcmp(type_name,"bool")==0)   { if(data_out)*data_out=NULL; return TK_BOOL; }
    if (strcmp(type_name,"string")==0) { if(data_out)*data_out=NULL; return TK_STRING; }
    if (strcmp(type_name,"void")==0)   { if(data_out)*data_out=NULL; return TK_VOID; }
    if (strcmp(type_name,"lua_State")==0){ if(data_out)*data_out=NULL; return TK_LUA_STATE; }
    if (strcmp(type_name,"strarray")==0){ if(data_out)*data_out=NULL; return TK_STRARRAY; }
    if (strcmp(type_name,"nresults")==0){ if(data_out)*data_out=NULL; return TK_NRESULTS; }

    if (data_out) *data_out = NULL;
    return TK_UNKNOWN;
}

/* Build a Type from xml node attrs (type, fixed-array, array, pointer, const, etc.) */
static Type build_type(xmlNodePtr node, Module *m) {
    Type t;
    memset(&t, 0, sizeof(t));
    t.type_name   = xget_prop(node, "type");
    if (!t.type_name) t.type_name = strdup("void");
    t.fixed_array  = 0;
    char *fa = xget_prop(node, "fixed-array");
    if (fa) { t.fixed_array = atoi(fa); free(fa); }
    char *arr = xget_prop(node, "array");
    t.is_array = arr && strcmp(arr,"true")==0; free(arr);
    char *ptr = xget_prop(node, "pointer");
    t.is_pointer = ptr && strcmp(ptr,"true")==0; free(ptr);
    char *cnst = xget_prop(node, "const");
    t.is_const = cnst && strcmp(cnst,"true")==0; free(cnst);
    t.default_val  = xget_prop(node, "default");
    t.output_attr  = xget_prop(node, "output");
    t.bufsize_attr = xget_prop(node, "bufsize");

    void *data = NULL;
    t.kind = resolve_kind(m, t.type_name, &data);
    t.data = data;

    /* export_name: for struct-like types use struct's export attr if available */
    if (t.kind == TK_STRUCT && data) {
        t.export_name = str_dup(((Struct*)data)->export_name);
    } else {
        t.export_name = str_dup(t.type_name);
    }
    return t;
}

/* Build a Field from xml node */
static Field build_field(xmlNodePtr node, Module *m) {
    Field f;
    memset(&f, 0, sizeof(f));
    char *n = xget_prop(node, "name");
    f.name.name = n;
    char buf[64];
    snprintf(buf, sizeof(buf), "0x%08x", fnv1a32(n ? n : ""));
    f.name.id   = strdup(buf);
    f.name.addr = str_dup(n);
    f.type      = build_type(node, m);
    f.doc       = xget_text(node);
    char *noe = xget_prop(node, "noexport");
    f.noexport = noe && strcmp(noe,"true")==0; free(noe);
    return f;
}

/* Build an Arg (same as field but named differently) */
static Arg build_arg(xmlNodePtr node, Module *m) {
    Arg a;
    a.param_name = xget_prop(node, "name");
    a.param_type = build_type(node, m);
    return a;
}

/* Build a Method from xml node. owner_node = parent interface/struct/class node. */
static Method build_method(xmlNodePtr node, Module *m, xmlNodePtr owner_node, const char *owner_prefix) {
    Method meth;
    memset(&meth, 0, sizeof(meth));
    meth.name = xget_prop(node, "name");
    char *exp = xget_prop(node, "export");
    meth.export_name = exp ? exp : str_lcfirst(meth.name);

    char *lcn = str_lcfirst(meth.name);
    char buf[64];
    snprintf(buf, sizeof(buf), "0x%08x", fnv1a32(lcn));
    free(lcn);
    meth.id = strdup(buf);

    char *stc = xget_prop(node, "static");
    meth.is_static = stc && strcmp(stc,"true")==0; free(stc);
    char *lua = xget_prop(node, "lua");
    meth.is_lua = lua && strcmp(lua,"true")==0; free(lua);
    char *cnst = xget_prop(node, "const");
    meth.is_const = cnst && strcmp(cnst,"true")==0; free(cnst);

    /* summary */
    xmlNodePtr sum = xfirst_child(node, "summary");
    if (sum) meth.doc = xget_text(sum);

    meth.alias = xget_prop(node, "alias");

    /* returns */
    xmlNodePtr ret = xfirst_child(node, "returns");
    if (ret) {
        meth.returns = malloc(sizeof(Type));
        *meth.returns = build_type(ret, m);
    }

    /* collect raw args from XML */
    int nargs = 0;
    for (xmlNodePtr c = node->children; c; c = c->next)
        if (c->type == XML_ELEMENT_NODE && strcmp((char*)c->name,"arg")==0) nargs++;

    /* We'll prepend this_ and L later, so total = raw + 2 max */
    meth.args = malloc((size_t)(nargs + 2) * sizeof(Arg));
    int idx = 0;

    /* Prepend lua_State* L if lua=true */
    if (meth.is_lua) {
        Arg la;
        la.param_name = strdup("L");
        la.param_type.type_name = strdup("lua_State");
        la.param_type.kind = TK_LUA_STATE;
        la.param_type.data = NULL;
        la.param_type.fixed_array = 0;
        la.param_type.is_array = 0;
        la.param_type.is_const = 0;
        la.param_type.is_pointer = 1;
        la.param_type.is_array = 0;
        la.param_type.export_name = strdup("lua_State");
        la.param_type.default_val = NULL;
        la.param_type.output_attr = NULL;
        la.param_type.bufsize_attr = NULL;
        meth.args[idx++] = la;
    }

    /* Prepend this_ if not static and has owner */
    if (owner_node && !meth.is_static) {
        char *own_name = xget_prop(owner_node, "name");
        Arg ta;
        ta.param_name = strdup("this_");
        ta.param_type.type_name = own_name;
        void *d = NULL;
        ta.param_type.kind = resolve_kind(m, own_name, &d);
        ta.param_type.data = d;
        ta.param_type.fixed_array = 0;
        ta.param_type.is_array = 0;
        ta.param_type.is_const = meth.is_const;
        ta.param_type.is_pointer = 1;
        if (ta.param_type.kind == TK_STRUCT && d)
            ta.param_type.export_name = str_dup(((Struct*)d)->export_name);
        else
            ta.param_type.export_name = str_dup(own_name);
        ta.param_type.default_val = NULL;
        ta.param_type.output_attr = NULL;
        ta.param_type.bufsize_attr = NULL;
        meth.args[idx++] = ta;
    }

    for (xmlNodePtr c = node->children; c; c = c->next) {
        if (c->type == XML_ELEMENT_NODE && strcmp((char*)c->name,"arg")==0) {
            meth.args[idx++] = build_arg(c, m);
        }
    }
    meth.num_args = idx;

    /* full_name = owner_prefix + name */
    const char *pfx = owner_prefix ? owner_prefix : "";
    size_t flen = strlen(pfx) + strlen(meth.name) + 1;
    meth.full_name = malloc(flen);
    snprintf(meth.full_name, flen, "%s%s", pfx, meth.name);

    return meth;
}

/* ------------------------------------------------------------------ Module loading */

static void load_event_fields(Event *ev, xmlNodePtr msg_node, Module *m) {
    ev->num_fields = 0;
    xmlNodePtr fields_node = xfirst_child(msg_node, "fields");
    if (!fields_node) return;
    int cnt = 0;
    for (xmlNodePtr c = fields_node->children; c; c = c->next)
        if (c->type == XML_ELEMENT_NODE && strcmp((char*)c->name,"field")==0) cnt++;
    ev->fields = malloc((size_t)cnt * sizeof(Field));
    for (xmlNodePtr c = fields_node->children; c; c = c->next) {
        if (c->type == XML_ELEMENT_NODE && strcmp((char*)c->name,"field")==0)
            ev->fields[ev->num_fields++] = build_field(c, m);
    }
}

Module *load_module(const char *xml_file, const char *source_override) {
    xmlDocPtr doc = xmlReadFile(xml_file, NULL, XML_PARSE_NOERROR|XML_PARSE_NOWARNING);
    if (!doc) {
        fprintf(stderr, "codegen: cannot parse XML file '%s'\n", xml_file);
        exit(1);
    }
    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (!root) {
        fprintf(stderr, "codegen: empty XML file '%s'\n", xml_file);
        exit(1);
    }

    Module *m = calloc(1, sizeof(Module));
    m->source = str_dup(source_override ? source_override : xml_file);
    m->name = xget_prop(root, "name");
    m->ns   = xget_prop(root, "namespace");
    m->on_luaopen    = xget_prop(root, "on-luaopen");
    m->after_luaopen = xget_prop(root, "after-luaopen");
    char *pfx = xget_prop(root, "prefix");
    if (pfx) {
        m->prefix = pfx;
    } else {
        size_t plen = strlen(m->name) + 2;
        m->prefix = malloc(plen);
        snprintf(m->prefix, plen, "%s_", m->name);
    }

    /* --- Requires (loaded first, needed for type resolution) --- */
    {
        int cap = 4;
        m->requires = malloc((size_t)cap * sizeof(Module*));
        for (xmlNodePtr c = root->children; c; c = c->next) {
            if (c->type != XML_ELEMENT_NODE) continue;
            if (strcmp((char*)c->name,"require")!=0) continue;
            char *rel = xget_prop(c, "file");
            char *src = xget_prop(c, "file"); /* source_override = relative path */
            /* resolve absolute path */
            char abs_path[1024];
            const char *dir = xml_file;
            const char *slash = strrchr(dir, '/');
            if (slash) {
                int dlen = (int)(slash - dir + 1);
                snprintf(abs_path, sizeof(abs_path), "%.*s%s", dlen, dir, rel);
            } else {
                snprintf(abs_path, sizeof(abs_path), "%s", rel);
            }
            free(rel);
            if (m->num_requires >= cap) {
                cap *= 2;
                m->requires = realloc(m->requires, (size_t)cap * sizeof(Module*));
            }
            m->requires[m->num_requires++] = load_module(abs_path, src);
        }
    }

    /* --- Externals --- */
    {
        int cap = 8;
        m->externals = malloc((size_t)cap * sizeof(ExternalStruct));
        for (xmlNodePtr c = root->children; c; c = c->next) {
            if (c->type != XML_ELEMENT_NODE || strcmp((char*)c->name,"externals")!=0) continue;
            for (xmlNodePtr e = c->children; e; e = e->next) {
                if (e->type != XML_ELEMENT_NODE || strcmp((char*)e->name,"external")!=0) continue;
                if (m->num_externals >= cap) { cap*=2; m->externals=realloc(m->externals,(size_t)cap*sizeof(ExternalStruct)); }
                m->externals[m->num_externals].name = xget_prop(e, "struct");
                m->num_externals++;
            }
        }
    }

    /* --- Includes --- */
    {
        int cap = 8;
        m->includes = malloc((size_t)cap * sizeof(Include));
        for (xmlNodePtr c = root->children; c; c = c->next) {
            if (c->type != XML_ELEMENT_NODE || strcmp((char*)c->name,"includes")!=0) continue;
            for (xmlNodePtr e = c->children; e; e = e->next) {
                if (e->type != XML_ELEMENT_NODE || strcmp((char*)e->name,"include")!=0) continue;
                if (m->num_includes >= cap) { cap*=2; m->includes=realloc(m->includes,(size_t)cap*sizeof(Include)); }
                m->includes[m->num_includes].file = xget_prop(e, "file");
                m->num_includes++;
            }
        }
    }

    /* --- Enums --- */
    {
        int cap = 16;
        m->enums = malloc((size_t)cap * sizeof(Enum));
        xmlNodePtr enums_node = xfirst_child(root, "enums");
        if (enums_node) {
            for (xmlNodePtr c = enums_node->children; c; c = c->next) {
                if (c->type != XML_ELEMENT_NODE || strcmp((char*)c->name,"enum")!=0) continue;
                if (m->num_enums >= cap) { cap*=2; m->enums=realloc(m->enums,(size_t)cap*sizeof(Enum)); }
                Enum *e = &m->enums[m->num_enums++];
                e->name = xget_prop(c, "name");
                xmlNodePtr sum = xfirst_child(c, "summary");
                e->doc = sum ? xget_text(sum) : NULL;
                /* values */
                int vcap = 16;
                e->values = malloc((size_t)vcap * sizeof(EnumValue));
                e->num_values = 0;
                for (xmlNodePtr v = c->children; v; v = v->next) {
                    if (v->type != XML_ELEMENT_NODE || strcmp((char*)v->name,"value")!=0) continue;
                    if (e->num_values >= vcap) { vcap*=2; e->values=realloc(e->values,(size_t)vcap*sizeof(EnumValue)); }
                    e->values[e->num_values].name = xget_prop(v, "name");
                    e->values[e->num_values].doc  = xget_text(v);
                    e->num_values++;
                }
            }
        }
    }

    /* --- Structs (pass 1: create without full type resolution for self-references) --- */
    {
        int cap = 16;
        m->structs = malloc((size_t)cap * sizeof(Struct));
        xmlNodePtr sn = xfirst_child(root, "structs");
        if (sn) {
            for (xmlNodePtr c = sn->children; c; c = c->next) {
                if (c->type != XML_ELEMENT_NODE || strcmp((char*)c->name,"struct")!=0) continue;
                if (m->num_structs >= cap) { cap*=2; m->structs=realloc(m->structs,(size_t)cap*sizeof(Struct)); }
                Struct *s = &m->structs[m->num_structs++];
                memset(s, 0, sizeof(*s));
                s->name = xget_prop(c, "name");
                s->prefix = xget_prop(c, "prefix");
                if (!s->prefix) s->prefix = str_dup(s->name);
                s->export_name = xget_prop(c, "export");
                if (!s->export_name) s->export_name = str_dup(s->name);
                char *sealed = xget_prop(c, "sealed");
                s->sealed = sealed && strcmp(sealed,"true")==0; free(sealed);
                xmlNodePtr sum = xfirst_child(c, "summary");
                s->doc = sum ? xget_text(sum) : NULL;
                s->module = m;

                /* fields */
                xmlNodePtr fn = xfirst_child(c, "fields");
                int fcap = 8; s->fields = malloc((size_t)fcap*sizeof(Field));
                if (fn) {
                    for (xmlNodePtr f = fn->children; f; f = f->next) {
                        if (f->type != XML_ELEMENT_NODE || strcmp((char*)f->name,"field")!=0) continue;
                        if (s->num_fields >= fcap) { fcap*=2; s->fields=realloc(s->fields,(size_t)fcap*sizeof(Field)); }
                        s->fields[s->num_fields++] = build_field(f, m);
                    }
                }

                /* methods */
                xmlNodePtr mn = xfirst_child(c, "methods");
                int mcap = 8; s->methods = malloc((size_t)mcap*sizeof(Method));
                if (mn) {
                    for (xmlNodePtr me = mn->children; me; me = me->next) {
                        if (me->type != XML_ELEMENT_NODE || strcmp((char*)me->name,"method")!=0) continue;
                        if (s->num_methods >= mcap) { mcap*=2; s->methods=realloc(s->methods,(size_t)mcap*sizeof(Method)); }
                        s->methods[s->num_methods++] = build_method(me, m, c, s->prefix);
                    }
                }
            }
        }
    }

    /* --- Interfaces --- */
    {
        int cap = 16;
        m->interfaces = malloc((size_t)cap * sizeof(Interface));
        xmlNodePtr in_node = xfirst_child(root, "interfaces");
        if (in_node) {
            for (xmlNodePtr c = in_node->children; c; c = c->next) {
                if (c->type != XML_ELEMENT_NODE || strcmp((char*)c->name,"interface")!=0) continue;
                if (m->num_interfaces >= cap) { cap*=2; m->interfaces=realloc(m->interfaces,(size_t)cap*sizeof(Interface)); }
                Interface *iface = &m->interfaces[m->num_interfaces++];
                memset(iface, 0, sizeof(*iface));
                iface->name = xget_prop(c, "name");
                iface->prefix = xget_prop(c, "prefix");
                if (!iface->prefix) iface->prefix = str_dup(iface->name);
                iface->export_name = xget_prop(c, "export");
                if (!iface->export_name) iface->export_name = str_dup(iface->name);
                iface->no_check = xget_prop(c, "no-check");
                xmlNodePtr sum = xfirst_child(c, "summary");
                iface->doc = sum ? xget_text(sum) : NULL;

                /* methods */
                xmlNodePtr mn = xfirst_child(c, "methods");
                int mcap = 64; iface->methods = malloc((size_t)mcap*sizeof(Method));
                if (mn) {
                    for (xmlNodePtr me = mn->children; me; me = me->next) {
                        if (me->type != XML_ELEMENT_NODE || strcmp((char*)me->name,"method")!=0) continue;
                        if (iface->num_methods >= mcap) { mcap*=2; iface->methods=realloc(iface->methods,(size_t)mcap*sizeof(Method)); }
                        iface->methods[iface->num_methods++] = build_method(me, m, c, iface->prefix);
                    }
                }
            }
        }
    }

    /* --- Components (classes) --- */
    {
        int cap = 32;
        m->components = malloc((size_t)cap * sizeof(Component));
        xmlNodePtr cn = xfirst_child(root, "classes");
        if (cn) {
            for (xmlNodePtr c = cn->children; c; c = c->next) {
                if (c->type != XML_ELEMENT_NODE || strcmp((char*)c->name,"class")!=0) continue;
                if (m->num_components >= cap) { cap*=2; m->components=realloc(m->components,(size_t)cap*sizeof(Component)); }
                Component *comp = &m->components[m->num_components++];
                memset(comp, 0, sizeof(*comp));
                comp->name = xget_prop(c, "name");
                comp->prefix = xget_prop(c, "prefix");
                if (!comp->prefix) comp->prefix = str_dup(comp->name);
                comp->export_name = xget_prop(c, "export");
                if (!comp->export_name) comp->export_name = str_dup(comp->name);
                char *ao = xget_prop(c, "attach-only");
                comp->attach_only = ao && strcmp(ao,"true")==0; free(ao);
                char *sealed = xget_prop(c, "sealed");
                comp->sealed = sealed && strcmp(sealed,"true")==0; free(sealed);
                xmlNodePtr sum = xfirst_child(c, "summary");
                comp->doc = sum ? xget_text(sum) : NULL;
                comp->module = m;

                /* parents */
                {
                    char *par = xget_prop(c, "parent");
                    char *con = xget_prop(c, "concept");
                    int pcap = 8;
                    comp->parents = malloc((size_t)pcap*sizeof(char*));
                    if (par) {
                        /* comma-separated list */
                        char *copy = strdup(par);
                        char *tok = strtok(copy, ",");
                        while (tok) {
                            while (*tok==' ') tok++;
                            char *end = tok+strlen(tok)-1;
                            while (end>tok && *end==' ') { *end--='\0'; }
                            if (*tok) {
                                if (comp->num_parents >= pcap) { pcap*=2; comp->parents=realloc(comp->parents,(size_t)pcap*sizeof(char*)); }
                                comp->parents[comp->num_parents++] = strdup(tok);
                            }
                            tok = strtok(NULL, ",");
                        }
                        free(copy);
                        free(par);
                    }
                    if (con) {
                        if (comp->num_parents >= pcap) { pcap*=2; comp->parents=realloc(comp->parents,(size_t)pcap*sizeof(char*)); }
                        comp->parents[comp->num_parents++] = con;
                    }
                }

                /* properties (raw, non-recursive) */
                {
                    xmlNodePtr pn = xfirst_child(c, "properties");
                    int pcap = 16; comp->properties = malloc((size_t)pcap*sizeof(Property));
                    if (pn) {
                        for (xmlNodePtr p = pn->children; p; p = p->next) {
                            if (p->type != XML_ELEMENT_NODE || strcmp((char*)p->name,"property")!=0) continue;
                            if (comp->num_properties >= pcap) { pcap*=2; comp->properties=realloc(comp->properties,(size_t)pcap*sizeof(Property)); }
                            char *pname = xget_prop(p, "name");
                            const char *path[1] = {pname};
                            comp->properties[comp->num_properties].name = make_property_name(comp->name, path, 1);
                            comp->properties[comp->num_properties].type = build_type(p, m);
                            xmlNodePtr s2 = xfirst_child(p, "summary");
                            comp->properties[comp->num_properties].doc = s2 ? xget_text(s2) : xget_text(p);
                            comp->num_properties++;
                            free(pname);
                        }
                    }
                }

                /* fields */
                {
                    xmlNodePtr fn = xfirst_child(c, "fields");
                    int fcap = 8; comp->fields = malloc((size_t)fcap*sizeof(Field));
                    if (fn) {
                        for (xmlNodePtr f = fn->children; f; f = f->next) {
                            if (f->type != XML_ELEMENT_NODE || strcmp((char*)f->name,"field")!=0) continue;
                            if (comp->num_fields >= fcap) { fcap*=2; comp->fields=realloc(comp->fields,(size_t)fcap*sizeof(Field)); }
                            comp->fields[comp->num_fields++] = build_field(f, m);
                        }
                    }
                }

                /* methods */
                {
                    xmlNodePtr mn2 = xfirst_child(c, "methods");
                    int mcap = 8; comp->methods = malloc((size_t)mcap*sizeof(Method));
                    if (mn2) {
                        for (xmlNodePtr me = mn2->children; me; me = me->next) {
                            if (me->type != XML_ELEMENT_NODE || strcmp((char*)me->name,"method")!=0) continue;
                            if (comp->num_methods >= mcap) { mcap*=2; comp->methods=realloc(comp->methods,(size_t)mcap*sizeof(Method)); }
                            /* component methods: full_name = compname_methodname (not prefix) */
                            char *mname = xget_prop(me, "name");
                            Method meth = build_method(me, m, c, "");
                            free(meth.full_name);
                            size_t flen = strlen(comp->name)+1+strlen(mname)+1;
                            meth.full_name = malloc(flen);
                            snprintf(meth.full_name, flen, "%s_%s", comp->name, mname);
                            free(mname);
                            comp->methods[comp->num_methods++] = meth;
                        }
                    }
                }

                /* handles */
                {
                    xmlNodePtr hn = xfirst_child(c, "handles");
                    int hcap = 16; comp->handles = malloc((size_t)hcap*sizeof(char*));
                    if (hn) {
                        for (xmlNodePtr h = hn->children; h; h = h->next) {
                            if (h->type != XML_ELEMENT_NODE || strcmp((char*)h->name,"handle")!=0) continue;
                            if (comp->num_handles >= hcap) { hcap*=2; comp->handles=realloc(comp->handles,(size_t)hcap*sizeof(char*)); }
                            comp->handles[comp->num_handles++] = xget_prop(h, "message");
                        }
                    }
                }

                /* messages (on-component) */
                {
                    xmlNodePtr msn = xfirst_child(c, "messages");
                    int msgcap = 8; comp->messages = malloc((size_t)msgcap*sizeof(Event*));
                    if (msn) {
                        for (xmlNodePtr msg = msn->children; msg; msg = msg->next) {
                            if (msg->type != XML_ELEMENT_NODE || strcmp((char*)msg->name,"message")!=0) continue;
                            if (comp->num_messages >= msgcap) { msgcap*=2; comp->messages=realloc(comp->messages,(size_t)msgcap*sizeof(Event*)); }
                            Event *ev = calloc(1, sizeof(Event));
                            ev->name       = xget_prop(msg, "name");
                            ev->ns         = str_dup(comp->name);
                            ev->parent_name= xget_prop(msg, "same-as");
                            xmlNodePtr s2  = xfirst_child(msg, "summary");
                            ev->doc        = s2 ? xget_text(s2) : NULL;
                            load_event_fields(ev, msg, m);
                            comp->messages[comp->num_messages++] = ev;
                        }
                    }
                }
            }
        }
    }

    /* --- Global functions --- */
    {
        int cap = 16;
        m->functions = malloc((size_t)cap * sizeof(Method));
        xmlNodePtr fn = xfirst_child(root, "functions");
        if (fn) {
            for (xmlNodePtr c = fn->children; c; c = c->next) {
                if (c->type != XML_ELEMENT_NODE || strcmp((char*)c->name,"function")!=0) continue;
                if (m->num_functions >= cap) { cap*=2; m->functions=realloc(m->functions,(size_t)cap*sizeof(Method)); }
                m->functions[m->num_functions++] = build_method(c, m, NULL, m->prefix);
            }
        }
    }

    /* --- Events: collect from interfaces + components, in document order --- */
    {
        int cap = 64;
        m->events = malloc((size_t)cap * sizeof(Event*));

        /* interfaces first (in order) */
        xmlNodePtr in_node = xfirst_child(root, "interfaces");
        if (in_node) {
            for (xmlNodePtr iface = in_node->children; iface; iface = iface->next) {
                if (iface->type != XML_ELEMENT_NODE || strcmp((char*)iface->name,"interface")!=0) continue;
                char *ns = xget_prop(iface, "name");
                xmlNodePtr msn = xfirst_child(iface, "messages");
                if (msn) {
                    for (xmlNodePtr msg = msn->children; msg; msg = msg->next) {
                        if (msg->type != XML_ELEMENT_NODE || strcmp((char*)msg->name,"message")!=0) continue;
                        if (m->num_events >= cap) { cap*=2; m->events=realloc(m->events,(size_t)cap*sizeof(Event*)); }
                        Event *ev = calloc(1, sizeof(Event));
                        ev->name       = xget_prop(msg, "name");
                        ev->ns         = str_dup(ns);
                        ev->parent_name= xget_prop(msg, "same-as");
                        xmlNodePtr s2  = xfirst_child(msg, "summary");
                        ev->doc        = s2 ? xget_text(s2) : NULL;
                        load_event_fields(ev, msg, m);
                        m->events[m->num_events++] = ev;
                    }
                }
                free(ns);
            }
        }

        /* components */
        xmlNodePtr cn = xfirst_child(root, "classes");
        if (cn) {
            for (xmlNodePtr cls = cn->children; cls; cls = cls->next) {
                if (cls->type != XML_ELEMENT_NODE || strcmp((char*)cls->name,"class")!=0) continue;
                char *ns = xget_prop(cls, "name");
                xmlNodePtr msn = xfirst_child(cls, "messages");
                if (msn) {
                    for (xmlNodePtr msg = msn->children; msg; msg = msg->next) {
                        if (msg->type != XML_ELEMENT_NODE || strcmp((char*)msg->name,"message")!=0) continue;
                        if (m->num_events >= cap) { cap*=2; m->events=realloc(m->events,(size_t)cap*sizeof(Event*)); }
                        Event *ev = calloc(1, sizeof(Event));
                        ev->name       = xget_prop(msg, "name");
                        ev->ns         = str_dup(ns);
                        ev->parent_name= xget_prop(msg, "same-as");
                        xmlNodePtr s2  = xfirst_child(msg, "summary");
                        ev->doc        = s2 ? xget_text(s2) : NULL;
                        load_event_fields(ev, msg, m);
                        m->events[m->num_events++] = ev;
                    }
                }
                free(ns);
            }
        }
    }

    xmlFreeDoc(doc);
    return m;
}

/* ------------------------------------------------------------------ Event helpers */

/* Find event by name (short name, e.g. "MouseMessage") in module's events array */
Event *module_find_event(Module *m, const char *name) {
    for (int i = 0; i < m->num_events; i++) {
        if (strcmp(m->events[i]->name, name) == 0) return m->events[i];
        /* also check "NS.Name" */
        char key[256];
        snprintf(key, sizeof(key), "%s.%s", m->events[i]->ns, m->events[i]->name);
        if (strcmp(key, name) == 0) return m->events[i];
    }
    for (int i = 0; i < m->num_requires; i++) {
        Event *e = module_find_event(m->requires[i], name);
        if (e) return e;
    }
    return NULL;
}

/* Check if event has its own fields */
int event_has_fields(Event *ev) {
    return ev->num_fields > 0;
}

/* Check if event or any ancestor has fields */
int event_has_any_fields(Event *ev, Module *m) {
    if (ev->num_fields > 0) return 1;
    if (ev->parent_name) {
        Event *parent = module_find_event(m, ev->parent_name);
        if (parent) return event_has_any_fields(parent, m);
    }
    return 0;
}

/* Get effective struct name for typedef (follow parent chain) */
char *event_get_effective_struct_name(Event *ev, Module *m) {
    if (ev->num_fields > 0) {
        size_t n = strlen(ev->ns)+1+strlen(ev->name)+strlen("EventArgs")+1;
        char *r = malloc(n);
        snprintf(r, n, "%s_%sEventArgs", ev->ns, ev->name);
        return r;
    }
    if (ev->parent_name) {
        Event *parent = module_find_event(m, ev->parent_name);
        if (parent) return event_get_effective_struct_name(parent, m);
    }
    /* no fields, no parent — use own name */
    size_t n = strlen(ev->ns)+1+strlen(ev->name)+strlen("EventArgs")+1;
    char *r = malloc(n);
    snprintf(r, n, "%s_%sEventArgs", ev->ns, ev->name);
    return r;
}

/* ------------------------------------------------------------------ walk_properties */

/* Recursive helper. Appends to *out / *out_count. */
static void walk_props_rec(const char *classname, const char **path, int path_len,
                            Type *type, const char *doc, Module *m,
                            Property **out, int *out_count, int *out_cap,
                            int unwrapped) {
    /* fixed-array expansion (only if not already unwrapped) */
    if (type->fixed_array > 0 && !unwrapped) {
        for (int i = 0; i < type->fixed_array; i++) {
            /* last path segment + "[i]" */
            size_t slen = strlen(path[path_len-1]) + 16;
            char *new_seg = malloc(slen);
            snprintf(new_seg, slen, "%s[%d]", path[path_len-1], i);
            const char **new_path = malloc((size_t)path_len * sizeof(char*));
            memcpy(new_path, path, (size_t)(path_len-1)*sizeof(char*));
            new_path[path_len-1] = new_seg;
            walk_props_rec(classname, new_path, path_len, type, doc, m, out, out_count, out_cap, 1);
            free(new_seg);
            free(new_path);
        }
        return;
    }

    /* Create Property */
    if (*out_count >= *out_cap) {
        *out_cap *= 2;
        *out = realloc(*out, (size_t)(*out_cap) * sizeof(Property));
    }
    Property *p = &(*out)[*out_count];
    p->name = make_property_name(classname, path + 0 /* path starts at index 0, skip classname? */,
                                  path_len);
    /* Wait — PHP: path = args[1:] where args = [classname, seg1, seg2, ...] */
    /* So path (for PropertyName) = path[0..path_len-1], classname is separate */
    /* Already correct: we pass path (without classname) and classname separately */
    p->doc  = str_dup(doc);

    /* Enum override: if int and enum exists with display_name */
    Type resolved_type = *type;
    if (type->kind == TK_INT) {
        /* check if display_name matches an enum name in module */
        void *enum_data = NULL;
        TypeKind ek = resolve_kind(m, p->name.display, &enum_data);
        if (ek == TK_ENUM) {
            resolved_type = *type;
            resolved_type.type_name = str_dup(p->name.display);
            resolved_type.kind = TK_ENUM;
            resolved_type.data = enum_data;
            resolved_type.export_name = str_dup(p->name.display);
        }
    }
    p->type = resolved_type;
    if (p->type.type_name != type->type_name) {
        /* newly allocated string, nothing to do */
    }
    (*out_count)++;

    /* Recurse into struct fields (non-sealed) */
    if (resolved_type.kind == TK_STRUCT && resolved_type.data) {
        Struct *s = (Struct*)resolved_type.data;
        if (!s->sealed) {
            for (int i = 0; i < s->num_fields; i++) {
                Field *f = &s->fields[i];
                /* new path = path + [field.name] */
                int new_len = path_len + 1;
                const char **new_path = malloc((size_t)new_len * sizeof(char*));
                memcpy(new_path, path, (size_t)path_len * sizeof(char*));
                new_path[path_len] = f->name.name;
                walk_props_rec(classname, new_path, new_len, &f->type, NULL, m,
                               out, out_count, out_cap, 0);
                free(new_path);
            }
        }
    }
}

Property *walk_properties(const char *classname, const char *prop_attr, Type *base_type,
                           const char *doc, Module *m, int *out_count) {
    int cap = 32;
    Property *out = malloc((size_t)cap * sizeof(Property));
    *out_count = 0;

    const char *path[1] = { prop_attr };
    walk_props_rec(classname, path, 1, base_type, doc, m, &out, out_count, &cap, 0);
    return out;
}
