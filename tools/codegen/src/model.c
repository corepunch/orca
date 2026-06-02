#include "model.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdlib.h>
#include <string.h>

typedef struct { cg_node *v; size_t n, cap; } nodebuf;

static char *xdup(char const *s) {
    size_t n = s ? strlen(s) : 0u;
    char *d = (char *)malloc(n + 1u);
    if (d) { if (n) memcpy(d, s, n); d[n] = '\0'; }
    return d;
}

static char *xattr(xmlNode const *e, char const *k) {
    xmlChar *v = xmlGetProp((xmlNode *)e, (xmlChar const *)k);
    char *d = xdup(v ? (char const *)v : "");
    if (v) xmlFree(v);
    return d;
}

static char *xtrimdup(char const *src) {
    char const *s = src ? src : "";
    char const *e;
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') ++s;
    e = s + strlen(s);
    while (e > s && (e[-1] == ' ' || e[-1] == '\t' || e[-1] == '\n' || e[-1] == '\r')) --e;
    {
        size_t n = (size_t)(e - s);
        char *d = (char *)malloc(n + 1u);
        if (!d) return NULL;
        if (n) memcpy(d, s, n);
        d[n] = '\0';
        return d;
    }
}

/* Returns <summary> child text or direct inline text; NULL if absent/empty. */
static char *xdoc(xmlNode const *e) {
    xmlNode *c;
    for (c = e->children; c; c = c->next) {
        if (c->type == XML_ELEMENT_NODE && !strcmp((char *)c->name, "summary")) {
            xmlChar *v = xmlNodeGetContent(c);
            char *d = xtrimdup(v ? (char const *)v : "");
            if (v) xmlFree(v);
            if (d && d[0]) return d;
            free(d); return NULL;
        }
        if (c->type == XML_TEXT_NODE) {
            char const *s = (char const *)c->content;
            while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') ++s;
            if (*s) return xtrimdup(s);
        }
    }
    return NULL;
}

static int bflag(xmlNode const *e, char const *attr) {
    xmlChar *v = xmlGetProp((xmlNode *)e, (xmlChar const *)attr);
    int r = v && !strcmp((char const *)v, "true");
    if (v) xmlFree(v);
    return r;
}

static int push_node(nodebuf *b, cg_node n) {
    if (b->n == b->cap) {
        size_t nc = b->cap ? b->cap * 2u : 256u;
        cg_node *nv = (cg_node *)realloc(b->v, nc * sizeof(*b->v));
        if (!nv) return -1;
        b->v = nv; b->cap = nc;
    }
    n.id = (uint32_t)b->n + 1u;
    b->v[b->n++] = n;
    return 0;
}

static cg_kind elem_kind(xmlNode const *e) {
    char const *t = (char const *)e->name;
    if (!strcmp(t, "module"))    return CG_KIND_MODULE;
    if (!strcmp(t, "require"))   return CG_KIND_REQUIRE;
    if (!strcmp(t, "include"))   return CG_KIND_INCLUDE;
    if (!strcmp(t, "external"))  return CG_KIND_EXTERNAL;
    if (!strcmp(t, "enum"))      return CG_KIND_ENUM;
    if (!strcmp(t, "value"))     return CG_KIND_VALUE;
    if (!strcmp(t, "interface")) return CG_KIND_INTERFACE;
    if (!strcmp(t, "struct"))    return CG_KIND_STRUCT;
    if (!strcmp(t, "class"))     return CG_KIND_CLASS;
    if (!strcmp(t, "method"))    return CG_KIND_METHOD;
    if (!strcmp(t, "function"))  return CG_KIND_METHOD;
    if (!strcmp(t, "message"))   return CG_KIND_MESSAGE;
    if (!strcmp(t, "field"))     return CG_KIND_FIELD;
    if (!strcmp(t, "property"))  return CG_KIND_PROPERTY;
    if (!strcmp(t, "handle"))    return CG_KIND_HANDLE;
    if (!strcmp(t, "arg"))       return CG_KIND_ARG;
    if (!strcmp(t, "returns"))   return CG_KIND_RETURNS;
    /* topic only inside methods container */
    if (!strcmp(t, "topic") && e->parent &&
        !strcmp((char const *)e->parent->name, "methods")) return CG_KIND_TOPIC;
    return 0;
}

static char *join_relative(char const *base_file, char const *rel) {
    char const *slash;
    size_t dir_len, rel_len;
    char *out;
    if (!rel || !rel[0]) return NULL;
    if (rel[0] == '/') return xdup(rel);
    slash = strrchr(base_file ? base_file : "", '/');
    dir_len = slash ? (size_t)(slash - base_file + 1) : 0u;
    rel_len = strlen(rel);
    out = (char *)malloc(dir_len + rel_len + 1u);
    if (!out) return NULL;
    if (dir_len) memcpy(out, base_file, dir_len);
    memcpy(out + dir_len, rel, rel_len);
    out[dir_len + rel_len] = '\0';
    return out;
}

static int walk(xmlNode const *e, uint32_t parent, nodebuf *b, char const *base_file) {
    for (; e; e = e->next) {
        cg_node n;
        cg_kind k;
        uint32_t child_parent;
        char *tmp;
        if (e->type != XML_ELEMENT_NODE) continue;
        k = elem_kind(e);
        child_parent = parent;
        if (!k) { if (walk(e->children, parent, b, base_file) < 0) return -1; continue; }
        memset(&n, 0, sizeof(n));
        n.parent = parent;
        n.kind   = k;
        n.doc    = xdoc(e);
        switch (k) {
        case CG_KIND_MODULE:
            n.name  = xattr(e, "name");
            n.type  = xattr(e, "namespace");
            n.extra = xattr(e, "prefix");
            break;
        case CG_KIND_REQUIRE:
        case CG_KIND_INCLUDE:
            tmp = xattr(e, "file");
            n.name  = tmp[0] ? tmp : (free(tmp), xattr(e, "name"));
            n.type  = xdup(""); n.extra = xdup(""); break;
        case CG_KIND_EXTERNAL:
            tmp = xattr(e, "struct");
            n.name  = tmp[0] ? tmp : (free(tmp), xattr(e, "name"));
            n.type  = xdup(""); n.extra = xdup(""); break;
        case CG_KIND_ENUM:
            n.name  = xattr(e, "name");
            n.type  = xdup(""); n.extra = xdup(""); break;
        case CG_KIND_VALUE:
            n.name  = xattr(e, "name");
            { xmlChar *v = xmlNodeGetContent((xmlNode *)e);
              n.type = xdup(v ? (char const *)v : ""); if (v) xmlFree(v); }
            n.extra = xdup(""); break;
        case CG_KIND_INTERFACE:
            n.name  = xattr(e, "name");
            n.type  = xattr(e, "prefix");
            n.extra = xattr(e, "export");
            if (bflag(e, "no-check")) n.flags |= CG_FLAG_NO_CHECK;
            break;
        case CG_KIND_STRUCT:
            n.name  = xattr(e, "name");
            n.type  = xattr(e, "prefix");
            n.extra = xattr(e, "export");
            if (bflag(e, "sealed")) n.flags |= CG_FLAG_SEALED;
            break;
        case CG_KIND_CLASS:
            n.name  = xattr(e, "name");
            n.type  = xattr(e, "parent");
            n.extra = xattr(e, "concept");
            break;
        case CG_KIND_METHOD:
            n.name  = xattr(e, "name");
            n.type  = xattr(e, "type");
            n.extra = xattr(e, "export");
            n.aux   = xattr(e, "alias");
            n.aux2  = xdup("");
            if (bflag(e, "lua"))    n.flags |= CG_FLAG_LUA;
            if (bflag(e, "static")) n.flags |= CG_FLAG_STATIC;
            if (bflag(e, "const"))  n.flags |= CG_FLAG_CONST;
            if (bflag(e, "private")) n.flags |= CG_FLAG_PRIVATE;
            break;
        case CG_KIND_MESSAGE:
            n.name  = xattr(e, "name");
            n.type  = xattr(e, "type");
            n.extra = xattr(e, "same-as");
            n.aux   = xattr(e, "routing");
            n.aux2  = xdup("");
            break;
        case CG_KIND_FIELD:
            n.name  = xattr(e, "name");
            n.type  = xattr(e, "type");
            n.extra = xattr(e, "fixed-array");
            if (bflag(e, "pointer"))  n.flags |= CG_FLAG_POINTER;
            if (bflag(e, "const"))    n.flags |= CG_FLAG_CONST;
            if (bflag(e, "noexport")) n.flags |= CG_FLAG_NOEXPORT;
            tmp = xattr(e, "array"); if (tmp[0]) n.flags |= CG_FLAG_ARRAY; free(tmp);
            break;
        case CG_KIND_PROPERTY:
            n.name  = xattr(e, "name");
            n.type  = xattr(e, "type");
            n.extra = xattr(e, "default");
            if (bflag(e, "pointer")) n.flags |= CG_FLAG_POINTER;
            if (bflag(e, "const"))   n.flags |= CG_FLAG_CONST;
            if (bflag(e, "private")) n.flags |= CG_FLAG_PRIVATE;
            if (bflag(e, "inherited")) n.flags |= CG_FLAG_INHERITED;
            { tmp = xattr(e, "array"); if (tmp[0]) n.flags |= CG_FLAG_ARRAY; free(tmp); }
            break;
        case CG_KIND_HANDLE:
            n.name  = xattr(e, "message");
            n.type  = xdup(""); n.extra = xdup(""); break;
        case CG_KIND_TOPIC:
            n.name  = xattr(e, "title");
            n.type  = xdup(""); n.extra = xdup(""); break;
        case CG_KIND_ARG:
            n.name  = xattr(e, "name");
            n.type  = xattr(e, "type");
            n.extra = xattr(e, "default");
            n.aux   = xattr(e, "output");
            n.aux2  = xattr(e, "bufsize");
            if (bflag(e, "pointer")) n.flags |= CG_FLAG_POINTER;
            if (bflag(e, "const"))   n.flags |= CG_FLAG_CONST;
            if (bflag(e, "array"))   n.flags |= CG_FLAG_ARRAY;
            break;
        case CG_KIND_RETURNS:
            n.name  = xdup("");
            n.type  = xattr(e, "type");
            n.extra = xattr(e, "default");
            n.aux   = xdup("");
            n.aux2  = xdup("");
            if (bflag(e, "pointer")) n.flags |= CG_FLAG_POINTER;
            if (bflag(e, "const"))   n.flags |= CG_FLAG_CONST;
            if (bflag(e, "array"))   n.flags |= CG_FLAG_ARRAY;
            break;
        default:
            n.name = xdup(""); n.type = xdup(""); n.extra = xdup(""); break;
        }
        if (!n.aux) n.aux = xdup("");
        if (!n.aux2) n.aux2 = xdup("");
        if (push_node(b, n) < 0) return -1;
        child_parent = (uint32_t)b->n;
        if (k == CG_KIND_REQUIRE && n.name && n.name[0]) {
            char *dep_path = join_relative(base_file, n.name);
            if (dep_path) {
                xmlDocPtr dep = xmlReadFile(dep_path, NULL, XML_PARSE_NOBLANKS | XML_PARSE_COMPACT);
                if (dep) {
                    xmlNode *dep_root = xmlDocGetRootElement(dep);
                    if (dep_root && walk(dep_root->children, child_parent, b, dep_path) < 0) {
                        xmlFreeDoc(dep);
                        free(dep_path);
                        return -1;
                    }
                    xmlFreeDoc(dep);
                }
                free(dep_path);
            }
        }
        if (walk(e->children, child_parent, b, base_file) < 0) return -1;
    }
    return 0;
}

int cg_model_load(char const *xml_path, cg_model *out) {
    nodebuf nodes = {0};
    xmlDocPtr doc;
    xmlNode *root;
    memset(out, 0, sizeof(*out));
    doc = xmlReadFile(xml_path, NULL, XML_PARSE_NOBLANKS | XML_PARSE_COMPACT);
    if (!doc) return -1;
    root = xmlDocGetRootElement(doc);
    if (!root) { xmlFreeDoc(doc); return -2; }
    /* Walk root's CHILDREN with parent=0 so top-level entities are directly
     * accessible via cg_foreach(model, 0, kind, var). Container wrappers
     * (externals, enums, classes, …) have no kind so their children inherit
     * parent=0 as well. */
    if (walk(root->children, 0u, &nodes, xml_path) < 0) {
        xmlFreeDoc(doc); free(nodes.v); return -3;
    }
    out->xml_path      = xdup(xml_path);
    out->module_name   = xattr(root, "name");
    out->module_ns     = xattr(root, "namespace");
    out->prefix        = xattr(root, "prefix");
    if (out->prefix && !out->prefix[0]) {
        char *p;
        size_t n = strlen(out->module_name ? out->module_name : "");
        free((void *)out->prefix);
        p = (char *)malloc(n + 2u);
        if (!p) {
            xmlFreeDoc(doc);
            return -4;
        }
        if (n) memcpy(p, out->module_name, n);
        p[n] = '_';
        p[n + 1u] = '\0';
        out->prefix = p;
    }
    out->on_luaopen    = xattr(root, "on-luaopen");
    out->after_luaopen = xattr(root, "after-luaopen");
    out->nodes         = nodes.v;
    out->node_count    = nodes.n;
    xmlFreeDoc(doc);
    return 0;
}

void cg_model_free(cg_model *model) {
    size_t i;
    if (!model) return;
    free((void *)model->xml_path);
    free((void *)model->module_name);
    free((void *)model->module_ns);
    free((void *)model->prefix);
    free((void *)model->on_luaopen);
    free((void *)model->after_luaopen);
    for (i = 0; i < model->node_count; ++i) {
        free((void *)model->nodes[i].name);
        free((void *)model->nodes[i].type);
        free((void *)model->nodes[i].extra);
        free((void *)model->nodes[i].aux);
        free((void *)model->nodes[i].aux2);
        free((void *)model->nodes[i].doc);
    }
    free((void *)model->nodes);
    memset(model, 0, sizeof(*model));
}
