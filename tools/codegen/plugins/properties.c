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

static uint32_t hash2(cg_host_v1 const *host, char const *a, char const *b) {
    char k[1024];
    snprintf(k, sizeof(k), "%s.%s", a, b);
    return host->fnv1a32(k);
}

static int emit_class(cg_host_v1 const *host, outbuf *b, cg_model const *m, cg_row const *c) {
    size_t i;
    uint32_t n_props = 0;
    uint32_t n_msgs = 0;
    if (ob_printf(b,
            "// %s\n"
            "#define ID_%s 0x%08x\n"
            "#define Get%s(_P) ((struct %s*)((_P)?OBJ_GetComponent(_P,ID_%s):NULL))\n"
            "#define %s_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_%s,sizeof(struct %s),_N)\n",
            c->name, c->name, host->fnv1a32(c->name), c->name, c->name, c->name, c->name, c->name, c->name) < 0)
        return -1;

    for (i = 0; i < m->row_count; ++i) {
        cg_row const *r = &m->rows[i];
        if (r->owner != c->id) continue;
        if (r->kind == CG_KIND_PROPERTY) {
            if (ob_printf(b, "#define ID_%s_%s 0x%08x // %s.%s\n",
                    c->name, r->name, hash2(host, c->name, r->name), c->name, r->name) < 0)
                return -1;
            ++n_props;
        }
        if (r->kind == CG_KIND_MESSAGE) {
            if (ob_printf(b, "#define ID_%s_%s 0x%08x // %s.%s\n",
                    c->name, r->name, hash2(host, c->name, r->name), c->name, r->name) < 0)
                return -1;
            ++n_msgs;
        }
    }

    if (ob_printf(b, "#define k%sNumProperties %u\n", c->name, n_props + n_msgs) < 0) return -1;

    if (n_props) {
        if (ob_printf(b, "enum %sProperties {\n", c->name) < 0) return -1;
        for (i = 0; i < m->row_count; ++i) {
            cg_row const *r = &m->rows[i];
            if (r->owner == c->id && r->kind == CG_KIND_PROPERTY)
                if (ob_printf(b, "\tk%s%s,\n", c->name, r->name) < 0) return -1;
        }
        for (i = 0; i < m->row_count; ++i) {
            cg_row const *r = &m->rows[i];
            if (r->owner == c->id && r->kind == CG_KIND_MESSAGE)
                if (ob_printf(b, "\tk%s%s,\n", c->name, r->name) < 0) return -1;
        }
        if (ob_printf(b, "};\n") < 0) return -1;
    }

    return ob_printf(b, "\n");
}

static int emit_properties(cg_host_v1 const *host, cg_model const *model, char const *output) {
    outbuf b = {0};
    size_t i;
    char *guard = to_upperdup(model->module_name && model->module_name[0] ? model->module_name : "module");
    if (!guard) return -1;

    if (ob_printf(&b,
            "// Auto-generated from %s by tools/codegen/plugins/properties.c\n"
            "// DO NOT EDIT — run 'cd tools && make codegen-sample' for now.\n"
            "#ifndef __%s_PROPERTIES_H__\n"
            "#define __%s_PROPERTIES_H__\n\n",
            model->xml_path, guard, guard) < 0)
        goto fail;

    for (i = 0; i < model->row_count; ++i)
        if (model->rows[i].kind == CG_KIND_CLASS)
            if (emit_class(host, &b, model, &model->rows[i]) < 0) goto fail;

    for (i = 0; i < model->row_count; ++i) {
        cg_row const *r = &model->rows[i];
        if (r->kind != CG_KIND_INTERFACE) continue;
        {
            size_t j;
            for (j = 0; j < model->row_count; ++j) {
                cg_row const *m = &model->rows[j];
                if (m->owner == r->id && m->kind == CG_KIND_MESSAGE)
                    if (ob_printf(&b, "#define ID_%s_%s 0x%08x // %s.%s\n",
                            r->name, m->name, hash2(host, r->name, m->name), r->name, m->name) < 0)
                        goto fail;
            }
        }
    }

    if (ob_printf(&b, "\n") < 0) goto fail;

    for (i = 0; i < model->row_count; ++i) {
        cg_row const *r = &model->rows[i];
        if (r->kind != CG_KIND_STRUCT) continue;
        if (ob_printf(&b, "#define ID_%s 0x%08x\n", r->name, host->fnv1a32(r->name)) < 0) goto fail;
        {
            size_t j;
            for (j = 0; j < model->row_count; ++j) {
                cg_row const *f = &model->rows[j];
                if (f->owner == r->id && f->kind == CG_KIND_FIELD)
                    if (ob_printf(&b, "#define ID_%s_%s 0x%08x // %s.%s\n",
                            r->name, f->name, hash2(host, r->name, f->name), r->name, f->name) < 0)
                        goto fail;
            }
        }
    }

    if (ob_printf(&b, "\n") < 0) goto fail;

    for (i = 0; i < model->row_count; ++i) {
        cg_row const *m = &model->rows[i];
        cg_row const *owner;
        char ev[512];
        size_t j;
        if (m->kind != CG_KIND_MESSAGE) continue;
        owner = (m->owner && m->owner <= model->row_count) ? &model->rows[m->owner - 1u] : NULL;
        if (!owner || !(owner->kind == CG_KIND_CLASS || owner->kind == CG_KIND_INTERFACE)) continue;
        snprintf(ev, sizeof(ev), "%s_%sEventArgs", owner->name, m->name);
        if (ob_printf(&b, "#define ID_%s 0x%08x\n", ev, host->fnv1a32(ev)) < 0) goto fail;
        for (j = 0; j < model->row_count; ++j) {
            cg_row const *f = &model->rows[j];
            if (f->owner == m->id && f->kind == CG_KIND_FIELD)
                if (ob_printf(&b, "#define ID_%s_%s 0x%08x // %s.%s\n",
                        ev, f->name, hash2(host, ev, f->name), ev, f->name) < 0)
                    goto fail;
        }
    }

    if (ob_printf(&b, "\n#endif\n") < 0) goto fail;

    free(guard);
    if (host->write_file(output, b.s, b.n) < 0) {
        host->logf("properties: failed write %s", output);
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
    .name = "properties",
    .emit = emit_properties
};

cg_plugin_v1 const *cg_plugin_init_v1(void) {
    return &plugin;
}
