#include "cg_api.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct outbuf { char *s; size_t n; size_t cap; } outbuf;

static int ob_grow(outbuf *b, size_t add) {
    if (b->n + add <= b->cap) return 0;
    {
        size_t nc = b->cap ? b->cap * 2u : 2048u;
        while (nc < b->n + add) nc *= 2u;
        char *ns = (char *)realloc(b->s, nc);
        if (!ns) return -1;
        b->s = ns;
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

static int emit_docs(cg_host_v1 const *host, cg_model const *model, char const *output) {
    size_t i;
    uint32_t n_enum = 0, n_if = 0, n_struct = 0, n_class = 0;
    outbuf b = {0};

    for (i = 0; i < model->node_count; ++i) {
        n_enum += model->nodes[i].kind == CG_KIND_ENUM;
        n_if += model->nodes[i].kind == CG_KIND_INTERFACE;
        n_struct += model->nodes[i].kind == CG_KIND_STRUCT;
        n_class += model->nodes[i].kind == CG_KIND_CLASS;
    }

    if (ob_printf(&b,
            "# %s API (cgen preview)\n\n"
            "- source: `%s`\n"
            "- enums: %u\n"
            "- interfaces: %u\n"
            "- structs: %u\n"
            "- classes: %u\n\n"
            "## Classes\n",
            model->module_name, model->xml_path, n_enum, n_if, n_struct, n_class) < 0) {
        free(b.s);
        return -1;
    }

    for (i = 0; i < model->node_count; ++i)
        if (model->nodes[i].kind == CG_KIND_CLASS)
            if (ob_printf(&b, "- %s\n", model->nodes[i].name) < 0) {
                free(b.s);
                return -1;
            }

    if (host->write_file(output, b.s, b.n) < 0) {
        host->logf("docs: failed write %s", output);
        free(b.s);
        return -1;
    }
    free(b.s);
    return 0;
}

static cg_plugin_v1 const plugin = { .abi = CG_API_VERSION, .name = "docs", .emit = emit_docs };

cg_plugin_v1 const *cg_plugin_init_v1(void) { return &plugin; }
