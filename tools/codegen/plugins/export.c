#include "cg_api.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct outbuf {
    char *s;
    size_t n;
    size_t cap;
} outbuf;

static int ob_grow(outbuf *b, size_t add) {
    if (b->n + add <= b->cap) return 0;
    {
        size_t nc = b->cap ? b->cap * 2u : 4096u;
        while (nc < b->n + add) nc *= 2u;
        char *ns = (char *)realloc(b->s, nc);
        if (!ns) return -1;
        b->s = ns;
        b->cap = nc;
    }
    return 0;
}

static int ob_printf(outbuf *b, char const *fmt, ...) {
    va_list ap;
    va_list cp;
    int need;
    va_start(ap, fmt);
    va_copy(cp, ap);
    need = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (need < 0 || ob_grow(b, (size_t)need + 1u) < 0) {
        va_end(cp);
        return -1;
    }
    vsnprintf(b->s + b->n, b->cap - b->n, fmt, cp);
    va_end(cp);
    b->n += (size_t)need;
    return 0;
}

static int emit_export(cg_host_v1 const *host, cg_model const *model, char const *output) {
    outbuf b = {0};
    size_t i;
    uint32_t n_classes = 0;
    uint32_t n_structs = 0;
    uint32_t n_interfaces = 0;

    for (i = 0; i < model->row_count; ++i) {
        n_classes += model->rows[i].kind == CG_KIND_CLASS;
        n_structs += model->rows[i].kind == CG_KIND_STRUCT;
        n_interfaces += model->rows[i].kind == CG_KIND_INTERFACE;
    }

    if (ob_printf(&b,
            "// Auto-generated from %s by tools/codegen/plugins/export.c\n"
            "// DO NOT EDIT — run 'cd tools && make modules-c-preview' for now.\n"
            "#include \"%s.h\"\n"
            "#include \"%s_properties.h\"\n\n"
            "/* cgen export scaffold: classes=%u structs=%u interfaces=%u */\n\n"
            "int luaopen_orca_%s(lua_State *L) {\n"
            "\t(void)L;\n"
            "\treturn 0;\n"
            "}\n",
            model->xml_path,
            model->module_name,
            model->module_name,
            n_classes,
            n_structs,
            n_interfaces,
            model->module_name) < 0) {
        free(b.s);
        return -1;
    }

    if (host->write_file(output, b.s, b.n) < 0) {
        host->logf("export: failed write %s", output);
        free(b.s);
        return -1;
    }
    free(b.s);
    return 0;
}

static cg_plugin_v1 const plugin = {
    .abi = CG_API_VERSION,
    .name = "export",
    .emit = emit_export
};

cg_plugin_v1 const *cg_plugin_init_v1(void) {
    return &plugin;
}
