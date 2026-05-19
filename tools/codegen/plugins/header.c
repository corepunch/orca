#include "cg_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
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

static int ob_puts(outbuf *b, char const *s) {
    size_t len = strlen(s);
    if (ob_grow(b, len + 1u) < 0) return -1;
    memcpy(b->s + b->n, s, len);
    b->n += len;
    b->s[b->n] = '\0';
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

static cg_row const *row_by_id(cg_model const *m, uint32_t id) {
    return (id && id <= m->row_count) ? &m->rows[id - 1u] : NULL;
}

static int emit_enum(outbuf *b, cg_model const *m, cg_row const *e) {
    size_t i;
    uint32_t count = 0;
    if (ob_printf(b, "typedef enum %s {\n", e->name) < 0) return -1;
    for (i = 0; i < m->row_count; ++i) {
        cg_row const *r = &m->rows[i];
        if (r->kind != CG_KIND_VALUE || r->owner != e->id) continue;
        if (ob_printf(b, "\tk%s%s,\n", e->name, r->name) < 0) return -1;
        ++count;
    }
    if (ob_printf(b,
            "} e%s_t;\n"
            "#define %s_Count %u\n"
            "ORCA_API const char *%sToString(enum %s value);\n"
            "ORCA_API enum %s luaX_check%s(lua_State *L, int idx);\n"
            "ORCA_API void luaX_push%s(lua_State *L, enum %s value);\n\n",
            e->name, e->name, count, e->name, e->name, e->name, e->name, e->name, e->name) < 0)
        return -1;
    return 0;
}

static int emit_forwards(outbuf *b, cg_model const *m, cg_kind k) {
    size_t i;
    for (i = 0; i < m->row_count; ++i) {
        cg_row const *r = &m->rows[i];
        if (r->kind != k || !r->name[0]) continue;
        if (ob_printf(b, "struct %s;\n", r->name) < 0) return -1;
    }
    return 0;
}

static int emit_messages(outbuf *b, cg_model const *m) {
    size_t i;
    for (i = 0; i < m->row_count; ++i) {
        cg_row const *r = &m->rows[i];
        cg_row const *o;
        if (r->kind != CG_KIND_MESSAGE || !r->name[0]) continue;
        o = row_by_id(m, r->owner);
        if (!o || !o->name[0]) continue;
        if (ob_printf(b,
                "typedef struct %s_%sEventArgs %s_%sMsg_t,* %s_%sMsgPtr;\n",
                o->name, r->name, o->name, r->name, o->name, r->name) < 0)
            return -1;
    }
    return 0;
}

static char *to_upperdup(char const *s) {
    size_t i, n = strlen(s);
    char *d = (char *)malloc(n + 1u);
    if (!d) return NULL;
    for (i = 0; i < n; ++i) {
        char c = s[i];
        d[i] = (c >= 'a' && c <= 'z') ? (char)(c - 32) : c;
    }
    d[n] = '\0';
    return d;
}

static int emit_header(cg_host_v1 const *host, cg_model const *model, char const *output) {
    outbuf b = {0};
    size_t i;
    char *guard = to_upperdup(model->module_name && model->module_name[0] ? model->module_name : "module");
    if (!guard) return -1;

    if (ob_printf(&b,
            "// Auto-generated from %s by tools/codegen/plugins/header.c\n"
            "// DO NOT EDIT — run 'cd tools && make codegen-sample' for now.\n"
            "#ifndef __%s_H__\n"
            "#define __%s_H__\n\n"
            "#include <stdbool.h>\n"
            "#include <stdint.h>\n\n"
            "typedef struct lua_State lua_State;\n\n",
            model->xml_path, guard, guard) < 0) {
        free(guard);
        free(b.s);
        return -1;
    }

    if (emit_forwards(&b, model, CG_KIND_EXTERNAL) < 0) goto fail;
    if (ob_puts(&b, "\n") < 0) goto fail;

    for (i = 0; i < model->row_count; ++i) {
        cg_row const *r = &model->rows[i];
        if (r->kind != CG_KIND_INCLUDE || !r->name[0]) continue;
        if (ob_printf(&b, "#include <%s>\n", r->name) < 0) goto fail;
    }

    if (ob_printf(&b, "\n#include \"%s_properties.h\"\n", model->module_name) < 0) goto fail;

    for (i = 0; i < model->row_count; ++i) {
        cg_row const *r = &model->rows[i];
        if (r->kind != CG_KIND_REQUIRE || !r->name[0]) continue;
        {
            char dep[512];
            size_t len = strlen(r->name);
            if (len >= 4u && !strcmp(r->name + len - 4u, ".xml")) {
                size_t n = len - 4u;
                if (n >= sizeof(dep) - 3u) n = sizeof(dep) - 4u;
                memcpy(dep, r->name, n);
                memcpy(dep + n, ".h", 3u);
            } else {
                snprintf(dep, sizeof(dep), "%s.h", r->name);
            }
            if (ob_printf(&b, "#include \"%s\"\n", dep) < 0) goto fail;
        }
    }

    if (ob_puts(&b, "\n") < 0) goto fail;
    if (emit_messages(&b, model) < 0) goto fail;
    if (ob_puts(&b, "\n") < 0) goto fail;

    for (i = 0; i < model->row_count; ++i) {
        cg_row const *r = &model->rows[i];
        if (r->kind != CG_KIND_ENUM || !r->name[0]) continue;
        if (emit_enum(&b, model, r) < 0) goto fail;
    }

    if (emit_forwards(&b, model, CG_KIND_INTERFACE) < 0) goto fail;
    if (emit_forwards(&b, model, CG_KIND_STRUCT) < 0) goto fail;
    if (emit_forwards(&b, model, CG_KIND_CLASS) < 0) goto fail;

    if (ob_puts(&b, "\n#endif\n") < 0) goto fail;

    free(guard);
    if (host->write_file(output, b.s, b.n) < 0) {
        host->logf("header: failed write %s", output);
        free(b.s);
        return -1;
    }
    free(b.s);
    return 0;

fail:
    free(guard);
    free(b.s);
    return -1;
}

static cg_plugin_v1 const plugin = {
    .abi = CG_API_VERSION,
    .name = "header",
    .emit = emit_header
};

cg_plugin_v1 const *cg_plugin_init_v1(void) {
    return &plugin;
}
