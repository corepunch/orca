#include "model.h"

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <stdlib.h>
#include <string.h>

typedef struct rowbuf {
    cg_row *v;
    size_t n;
    size_t cap;
} rowbuf;

static char *dupstr(char const *s) {
    size_t n = s ? strlen(s) : 0u;
    char *d = (char *)malloc(n + 1u);
    if (!d) return NULL;
    if (n) memcpy(d, s, n);
    d[n] = '\0';
    return d;
}

static char *node_attr(xmlNode const *node, char const *k) {
    xmlChar *v = xmlGetProp((xmlNode *)node, (xmlChar const *)k);
    char *d = dupstr((char const *)(v ? v : (xmlChar const *)""));
    if (v) xmlFree(v);
    return d;
}

static int push_row(rowbuf *b, cg_row row) {
    if (b->n == b->cap) {
        size_t nc = b->cap ? b->cap * 2u : 128u;
        cg_row *nv = (cg_row *)realloc(b->v, nc * sizeof(*b->v));
        if (!nv) return -1;
        b->v = nv;
        b->cap = nc;
    }
    row.id = (uint32_t)b->n + 1u;
    b->v[b->n++] = row;
    return 0;
}

static cg_kind parse_kind(xmlNode const *node, xmlNode const *parent) {
    char const *n = (char const *)node->name;
    char const *pn = parent ? (char const *)parent->name : "";
    if (!strcmp(n, "module")) return CG_KIND_MODULE;
    if (!strcmp(n, "include")) return CG_KIND_INCLUDE;
    if (!strcmp(n, "external")) return CG_KIND_EXTERNAL;
    if (!strcmp(n, "enum")) return CG_KIND_ENUM;
    if (!strcmp(n, "interface")) return CG_KIND_INTERFACE;
    if (!strcmp(n, "struct")) return CG_KIND_STRUCT;
    if (!strcmp(n, "class")) return CG_KIND_CLASS;
    if (!strcmp(n, "method")) return CG_KIND_METHOD;
    if (!strcmp(n, "message")) return CG_KIND_MESSAGE;
    if (!strcmp(n, "field")) return CG_KIND_FIELD;
    if (!strcmp(n, "property")) return CG_KIND_PROPERTY;
    if (!strcmp(n, "handle")) return CG_KIND_HANDLE;
    if (!strcmp(n, "topic") && !strcmp(pn, "methods")) return CG_KIND_TOPIC;
    return 0;
}

static int walk(xmlNode const *node, uint32_t owner, rowbuf *out) {
    for (; node; node = node->next) {
        if (node->type != XML_ELEMENT_NODE) continue;
        cg_kind k = parse_kind(node, node->parent);
        uint32_t row_owner = owner;
        uint32_t row_id = 0;
        if (k) {
            cg_row row = {
                .id = 0,
                .owner = owner,
                .kind = k,
                .name = node_attr(node, "name"),
                .type = node_attr(node, "type"),
                .value = node_attr(node, "message")
            };
            if (!row.value || !row.value[0]) {
                free((void *)row.value);
                row.value = node_attr(node, "title");
            }
            if (!row.name || !row.type || !row.value) {
                free((void *)row.name);
                free((void *)row.type);
                free((void *)row.value);
                return -1;
            }
            if (push_row(out, row) < 0) return -1;
            row_id = (uint32_t)out->n;
            row_owner = row_id;
        }
        if (walk(node->children, row_owner, out) < 0) return -1;
    }
    return 0;
}

int cg_model_load(char const *xml_path, cg_model *out) {
    xmlDocPtr doc = xmlReadFile(xml_path, NULL, XML_PARSE_NOBLANKS | XML_PARSE_COMPACT);
    xmlNode *root;
    rowbuf rows = {0};
    memset(out, 0, sizeof(*out));
    if (!doc) return -1;
    root = xmlDocGetRootElement(doc);
    if (!root) {
        xmlFreeDoc(doc);
        return -2;
    }
    if (walk(root, 0u, &rows) < 0 || !rows.n) {
        xmlFreeDoc(doc);
        free(rows.v);
        return -3;
    }
    out->xml_path = dupstr(xml_path);
    out->module_name = node_attr(root, "name");
    out->prefix = node_attr(root, "prefix");
    out->rows = rows.v;
    out->row_count = rows.n;
    xmlFreeDoc(doc);
    return (!out->xml_path || !out->module_name || !out->prefix) ? -4 : 0;
}

void cg_model_free(cg_model *model) {
    size_t i;
    if (!model) return;
    free((void *)model->xml_path);
    free((void *)model->module_name);
    free((void *)model->prefix);
    for (i = 0; i < model->row_count; ++i) {
        free((void *)model->rows[i].name);
        free((void *)model->rows[i].type);
        free((void *)model->rows[i].value);
    }
    free((void *)model->rows);
    memset(model, 0, sizeof(*model));
}
