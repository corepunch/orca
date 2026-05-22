#include "cg_api.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct outbuf { char *s; size_t n; size_t cap; } outbuf;

static int ob_grow(outbuf *b, size_t add) {
    if (b->n + add <= b->cap) return 0;
    {
        size_t nc = b->cap ? b->cap * 2u : 2048u;
        while (nc < b->n + add) nc *= 2u;
        b->s = (char *)realloc(b->s, nc);
        if (!b->s) return -1;
        b->cap = nc;
    }
    return 0;
}

static int ob_printf(outbuf *b, char const *fmt, ...) {
    va_list ap, cp;
    int need;
    va_start(ap, fmt);
    va_copy(cp, ap);
    need = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (need < 0 || ob_grow(b, (size_t)need + 1u) < 0) { va_end(cp); return -1; }
    vsnprintf(b->s + b->n, b->cap - b->n, fmt, cp);
    va_end(cp);
    b->n += (size_t)need;
    return 0;
}

static int emit_dtd(cg_host_v1 const *host, cg_model const *model, char const *output) {
    size_t i;
    outbuf b = {0};
    if (ob_printf(&b,
            "<!-- Auto-generated from %s by cgen dtd preview plugin -->\n"
            "<!ELEMENT module (enums?, interfaces?, structs?, classes?)>\n"
            "<!ATTLIST module name CDATA #REQUIRED>\n",
            model->xml_path) < 0) {
        free(b.s);
        return -1;
    }

    for (i = 0; i < model->node_count; ++i)
        if (model->nodes[i].kind == CG_KIND_CLASS)
            if (ob_printf(&b,
                    "<!ELEMENT %s ANY>\n"
                    "<!ATTLIST %s Name CDATA #IMPLIED>\n",
                    model->nodes[i].name, model->nodes[i].name) < 0) {
                free(b.s);
                return -1;
            }

    if (host->write_file(output, b.s, b.n) < 0) {
        host->logf("dtd: failed write %s", output);
        free(b.s);
        return -1;
    }
    free(b.s);
    return 0;
}

static cg_plugin_v1 const plugin = { .abi = CG_API_VERSION, .name = "dtd", .emit = emit_dtd };

cg_plugin_v1 const *cg_plugin_init_v1(void) { return &plugin; }
