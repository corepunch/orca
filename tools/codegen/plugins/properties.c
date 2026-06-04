#include "cg_api.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- output buffer ------------------------------------------------ */

typedef struct { char *s; size_t n, cap; } ob;

static int ob_printf(ob *b, char const *fmt, ...) {
    va_list ap, cp; int need;
    va_start(ap, fmt); va_copy(cp, ap);
    need = vsnprintf(NULL, 0, fmt, ap); va_end(ap);
    if (need < 0) { va_end(cp); return -1; }
    if (b->n + (size_t)need + 1u > b->cap) {
        size_t nc = b->cap ? b->cap * 2u : 4096u;
        while (nc < b->n + (size_t)need + 1u) nc *= 2u;
        char *ns = realloc(b->s, nc);
        if (!ns) { va_end(cp); return -1; }
        b->s = ns; b->cap = nc;
    }
    vsnprintf(b->s + b->n, b->cap - b->n, fmt, cp); va_end(cp);
    b->n += (size_t)need;
    return 0;
}

/* ---------- string helpers ----------------------------------------------- */

static void str_upper(char *dst, size_t dsz, char const *s) {
    size_t i = 0;
    for (; *s && i + 1 < dsz; ++s, ++i) {
        char c = *s;
        dst[i] = (c >= 'a' && c <= 'z') ? (char)(c - 32) : c;
    }
    dst[i] = 0;
}

static char const *base_name(char const *path) {
    char const *slash = path ? strrchr(path, '/') : NULL;
    return slash ? slash + 1 : (path ? path : "");
}

/* ucfirst: capitalise first char of s into dst */
static void ucfirst_into(char *dst, size_t dsz, char const *s) {
    if (!s || !*s || dsz < 2) { if (dsz) dst[0] = 0; return; }
    dst[0] = (char)toupper((unsigned char)s[0]);
    snprintf(dst + 1, dsz - 1, "%s", s + 1);
}

/* strip chars '.' '[' ']' from src into dst */
static void strip_brackets_dots(char *dst, size_t dsz, char const *src) {
    size_t i = 0;
    for (; *src && i + 1 < dsz; ++src) {
        if (*src != '.' && *src != '[' && *src != ']') dst[i++] = *src;
    }
    dst[i] = 0;
}

/* ---------- axis name transform ------------------------------------------ */

/* Names for axis index 0/1/2 */
static char const *k_ax[3]      = {"Horizontal", "Vertical", "Depth"};
static char const *k_left[3]    = {"Left",   "Top",    "Front"};
static char const *k_right[3]   = {"Right",  "Bottom", "Back"};
/* Size axis sub-field names and their axis-mapped display names */
static char const *k_size_sub[5]     = {"Requested","Desired","Min","Actual","Scroll"};
static char const *k_size_names[3][5] = {
    {"Width","DesiredWidth","MinWidth","ActualWidth","ScrollWidth"},
    {"Height","DesiredHeight","MinHeight","ActualHeight","ScrollHeight"},
    {"Depth","DesiredDepth","MinDepth","ActualDepth","ScrollDepth"},
};

/* Transform dot-joined ucfirsted path (WITHOUT class prefix) to final name.
 * The rules mirror the legacy Axis property-name transform order. */
static void axis_transform(char const *path, char *out, size_t out_sz) {
    char result[512];
    int n;
    /* --- rules 7-9: Size.Axis[N].{Sub} --- */
    for (n = 0; n < 3; n++) {
        char tok[64]; snprintf(tok, sizeof(tok), "Size.Axis[%d].", n);
        if (strncmp(path, tok, strlen(tok)) == 0) {
            char const *sub = path + strlen(tok);
            int k;
            /* strip leading "Requested" if present */
            if (strncmp(sub, "Requested", 9) == 0) {
                char const *rest = sub + 9;
                snprintf(out, out_sz, "%s%s", k_size_names[n][0], rest);
                goto done;
            }
            for (k = 1; k < 5; k++) {
                if (strcmp(sub, k_size_sub[k]) == 0) {
                    snprintf(out, out_sz, "%s", k_size_names[n][k]);
                    goto done;
                }
            }
            /* fallthrough: unknown sub-field */
        }
    }
    /* --- rules 1-6: (.+).Axis[N].Left/Right --- */
    for (n = 0; n < 3; n++) {
        char lt[64], rt[64];
        snprintf(lt, sizeof(lt), ".Axis[%d].Left", n);
        snprintf(rt, sizeof(rt), ".Axis[%d].Right", n);
        char *pos;
        if ((pos = strstr((char *)path, lt)) != NULL) {
            size_t pl = (size_t)(pos - path);
                char prefix[256]; memcpy(prefix, path, pl); prefix[pl] = 0;
                char const *suf = pos + strlen(lt);
                snprintf(result, sizeof(result), "%s%s%s", prefix, k_left[n], suf);
                strip_brackets_dots(out, out_sz, result);
            return;
        }
        if ((pos = strstr((char *)path, rt)) != NULL) {
            size_t pl = (size_t)(pos - path);
            char prefix[256]; memcpy(prefix, path, pl); prefix[pl] = 0;
            char const *suf = pos + strlen(rt);
            snprintf(result, sizeof(result), "%s%s%s", prefix, k_right[n], suf);
            strip_brackets_dots(out, out_sz, result);
            return;
        }
    }
    /* --- rules 10-12: (.+).Axis[N] at end --- */
    for (n = 0; n < 3; n++) {
        char tok[32]; snprintf(tok, sizeof(tok), ".Axis[%d]", n);
        size_t plen = strlen(path), tlen = strlen(tok);
        if (plen > tlen && strcmp(path + plen - tlen, tok) == 0) {
            char prefix[256]; memcpy(prefix, path, plen - tlen); prefix[plen - tlen] = 0;
            snprintf(result, sizeof(result), "%s%s", k_ax[n], prefix);
            strip_brackets_dots(out, out_sz, result);
            return;
        }
    }
    /* --- rule 13: Border.Radius.(.+)Radius --- */
    if (strncmp(path, "Border.Radius.", 14) == 0) {
        char const *tail = path + 14;
        size_t len = strlen(tail);
        if (len > 6 && strcmp(tail + len - 6, "Radius") == 0) {
            snprintf(result, sizeof(result), "Border%.*sRadius", (int)(len - 6), tail);
            strip_brackets_dots(out, out_sz, result);
            return;
        }
    }
    /* --- rules 14-19: Axis[N].Left/Right without leading segment --- */
    for (n = 0; n < 3; n++) {
        char lt[64], rt[64];
        snprintf(lt, sizeof(lt), "Axis[%d].Left", n);
        snprintf(rt, sizeof(rt), "Axis[%d].Right", n);
        if (strncmp(path, lt, strlen(lt)) == 0) {
            snprintf(result, sizeof(result), "%s%s", k_left[n], path + strlen(lt));
            strip_brackets_dots(out, out_sz, result);
            return;
        }
        if (strncmp(path, rt, strlen(rt)) == 0) {
            snprintf(result, sizeof(result), "%s%s", k_right[n], path + strlen(rt));
            strip_brackets_dots(out, out_sz, result);
            return;
        }
    }
    /* --- rules 20-22: bare Axis[N] --- */
    for (n = 0; n < 3; n++) {
        char tok[32]; snprintf(tok, sizeof(tok), "Axis[%d]", n);
        if (strstr(path, tok) != NULL) {
            char const *pos = strstr(path, tok);
            size_t pl = (size_t)(pos - path);
            char prefix[256];
            memcpy(prefix, path, pl);
            prefix[pl] = 0;
            snprintf(result, sizeof(result), "%s%s%s",
                    prefix, k_ax[n], pos + strlen(tok));
            strip_brackets_dots(out, out_sz, result);
            return;
        }
    }
    /* No pattern matched: strip [,],. */
    strip_brackets_dots(out, out_sz, path);
    return;
done:
    /* out already written; just ensure no [,],. */
    ;
}

/* ---------- struct registry ---------------------------------------------- */

typedef struct { char const *name; uint32_t id; int sealed; } sentry;

static sentry *build_struct_map(cg_model const *m, size_t *count_out) {
    size_t n = 0;
    size_t mi;
    for (mi = 0; mi < m->node_count; ++mi)
        if (m->nodes[mi].kind == CG_KIND_STRUCT) n++;
    sentry *e = calloc(n ? n : 1, sizeof(*e));
    if (!e) return NULL;
    size_t i = 0;
    for (mi = 0; mi < m->node_count; ++mi) {
        cg_node const *s = &m->nodes[mi];
        if (s->kind != CG_KIND_STRUCT) continue;
        e[i].name   = s->name;
        e[i].id     = s->id;
        e[i].sealed = (s->flags & CG_FLAG_SEALED) != 0;
        i++;
    }
    *count_out = n;
    return e;
}

static sentry const *find_struct(sentry const *map, size_t cnt, char const *type) {
    size_t i;
    for (i = 0; i < cnt; i++) if (strcmp(map[i].name, type) == 0) return &map[i];
    return NULL;
}

/* ---------- recursive property walker ------------------------------------ */

/* segs[MAX_DEPTH][64] — path segments (excluding class name), may contain "Axis[N]" */
#define MAX_DEPTH 16

typedef struct {
    ob                *b;
    cg_model const    *m;
    sentry const      *smap;
    size_t             scount;
    cg_host_v1 const  *h;
    char const        *class_name;
} walk_ctx;

static int walk_prop(walk_ctx *ctx, char segs[MAX_DEPTH][64], int n_segs, char const *type_name);

static int walk_fixed_array(walk_ctx *ctx, char segs[MAX_DEPTH][64], int n_segs,
                            char const *field_name, char const *field_type, int fixed) {
    int ai;
    for (ai = 0; ai < fixed; ai++) {
        char new_segs[MAX_DEPTH][64];
        memcpy(new_segs, segs, sizeof(new_segs));
        snprintf(new_segs[n_segs], 64, "%s[%d]", field_name, ai);
        if (walk_prop(ctx, new_segs, n_segs + 1, field_type) < 0) return -1;
    }
    return 0;
}

static int walk_prop(walk_ctx *ctx, char segs[MAX_DEPTH][64], int n_segs, char const *type_name) {
    sentry const *s;
    /* Build dot-joined ucfirsted path */
    char path[512] = "";
    int i;
    for (i = 0; i < n_segs; i++) {
        char seg[64]; ucfirst_into(seg, sizeof(seg), segs[i]);
        if (i > 0) { size_t pl = strlen(path); if (pl + 1 < sizeof(path)) { path[pl] = '.'; path[pl+1] = 0; } }
        size_t pl = strlen(path); snprintf(path + pl, sizeof(path) - pl, "%s", seg);
    }
    /* Axis transform → leaf name */
    char leaf[256]; axis_transform(path, leaf, sizeof(leaf));
    /* Emit ID macro */
    char full[512]; snprintf(full, sizeof(full), "%s.%s", ctx->class_name, leaf);
    uint32_t h = ctx->h->fnv1a32(full);
    if (ob_printf(ctx->b, "#define ID_%s_%s 0x%08x // %s\n",
            ctx->class_name, leaf, h, full) < 0) return -1;
    s = find_struct(ctx->smap, ctx->scount, type_name);
    /* Recurse into non-sealed struct */
    if (s && !s->sealed && n_segs < MAX_DEPTH - 1) {
        cg_foreach(ctx->m, s->id, CG_KIND_FIELD, f) {
            char fn[64]; snprintf(fn, sizeof(fn), "%s", f->name);
            int fixed = (f->extra && f->extra[0]) ? atoi(f->extra) : 0;
            if (fixed > 0) {
                if (walk_fixed_array(ctx, segs, n_segs, fn, f->type, fixed) < 0) return -1;
            } else {
                char new_segs[MAX_DEPTH][64];
                memcpy(new_segs, segs, sizeof(new_segs));
                snprintf(new_segs[n_segs], 64, "%s", fn);
                if (walk_prop(ctx, new_segs, n_segs + 1, f->type) < 0) return -1;
            }
        }
    }
    return 0;
}

/* ---------- emit helpers ------------------------------------------------- */

static uint32_t hash2(cg_host_v1 const *h, char const *a, char const *b) {
    char k[1024]; snprintf(k, sizeof(k), "%s.%s", a, b);
    return h->fnv1a32(k);
}

static cg_node const *node_parent(cg_model const *m, cg_node const *n) {
    return n ? cg_node_by_id(m, n->parent) : NULL;
}

static int local_node(cg_node const *n) {
    return n && n->parent == 0;
}

static cg_node const *find_event(cg_model const *m, cg_node const *owner, char const *name) {
    if (!name || !name[0]) return NULL;
    if (owner) {
        cg_foreach(m, owner->id, CG_KIND_MESSAGE, msg)
            if (!strcmp(msg->name, name)) return msg;
    }
    for (size_t i = 0; i < m->node_count; ++i)
        if (m->nodes[i].kind == CG_KIND_MESSAGE && !strcmp(m->nodes[i].name, name))
            return &m->nodes[i];
    return NULL;
}

static cg_node const *event_parent(cg_model const *m, cg_node const *msg) {
    return (msg->extra && msg->extra[0])
        ? find_event(m, node_parent(m, msg), msg->extra)
        : NULL;
}

static void message_action_name(char *dst, size_t dsz, cg_model const *m, cg_node const *msg) {
    cg_node const *owner = node_parent(m, msg);
    snprintf(dst, dsz, "%s_%sAction", owner ? owner->name : "", msg->name);
}

static void message_xml_name(char *dst, size_t dsz, cg_model const *m, cg_node const *msg) {
    cg_node const *owner = node_parent(m, msg);
    snprintf(dst, dsz, "%s.%s", owner ? owner->name : "", msg->name);
}


static int emit_message_action_field_ids(cg_host_v1 const *h, ob *b, cg_model const *m,
                                         cg_node const *msg, char const *action) {
    cg_node const *p = event_parent(m, msg);
    if (p && emit_message_action_field_ids(h, b, m, p, action) < 0) return -1;
    cg_foreach(m, msg->id, CG_KIND_FIELD, f) {
        if (ob_printf(b, "#define ID_%s_%s 0x%08x // %s.%s\n",
                action, f->name, hash2(h, action, f->name), action, f->name) < 0) return -1;
    }
    return 0;
}

static int emit_message_action(cg_host_v1 const *h, ob *b, cg_model const *m, cg_node const *msg) {
    char action[256], xml_name[512];
    message_action_name(action, sizeof(action), m, msg);
    message_xml_name(xml_name, sizeof(xml_name), m, msg);
    if (ob_printf(b,
            "// %s\n"
            "#define ID_%s 0x%08x // %s\n"
            "#define Get%s(_P) ((struct %s*)((_P)?OBJ_GetComponent(_P,ID_%s):NULL))\n"
            "#define %s_GetProperty(_P,FIELD) OBJ_GetPropertyByOffset(_P,ID_%s,offsetof(struct %s,FIELD))\n",
            xml_name, action, h->fnv1a32(xml_name), xml_name,
            action, action, action,
            action, action, action) < 0) return -1;
    if (emit_message_action_field_ids(h, b, m, msg, action) < 0) return -1;
    return 0;
}

static cg_node const *find_mixin(cg_model const *m, char const *name) {
    size_t i;
    if (!name || !name[0]) return NULL;
    for (i = 0; i < m->node_count; ++i)
        if (m->nodes[i].kind == CG_KIND_MIXIN && !strcmp(m->nodes[i].name, name))
            return &m->nodes[i];
    return NULL;
}

static int emit_prop_ids(cg_host_v1 const *h, ob *b, cg_model const *m, cg_node const *c,
                         uint32_t owner_id, sentry const *smap, size_t scount) {
    walk_ctx ctx = {0};
    ctx.b = b; ctx.m = m; ctx.smap = smap; ctx.scount = scount;
    ctx.h = h; ctx.class_name = c->name;
    cg_foreach(m, owner_id, CG_KIND_PROPERTY, p) {
        char segs[MAX_DEPTH][64];
        snprintf(segs[0], 64, "%s", p->name);
        if (walk_prop(&ctx, segs, 1, p->type) < 0) return -1;
        if (p->flags & CG_FLAG_ARRAY) {
            char num[256]; snprintf(num, sizeof(num), "Num%s", p->name);
            if (ob_printf(b, "#define ID_%s_%s 0x%08x // %s.%s\n",
                    c->name, num, hash2(h, c->name, num), c->name, num) < 0) return -1;
        }
    }
    return 0;
}

static int emit_class(cg_host_v1 const *h, ob *b, cg_model const *m, cg_node const *c,
                      sentry const *smap, size_t scount) {
    cg_node const *mx = find_mixin(m, c->aux);

    if (ob_printf(b,
            "// %s\n"
            "#define ID_%s 0x%08x\n"
            "#define Get%s(_P) ((struct %s*)((_P)?OBJ_GetComponent(_P,ID_%s):NULL))\n"
            "#define %s_GetProperty(_P,FIELD) OBJ_GetPropertyByOffset(_P,ID_%s,offsetof(struct %s,FIELD))\n",
            c->name, c->name, h->fnv1a32(c->name),
            c->name, c->name, c->name,
            c->name, c->name, c->name) < 0) return -1;

    if (mx && emit_prop_ids(h, b, m, c, mx->id, smap, scount) < 0) return -1;
    if (emit_prop_ids(h, b, m, c, c->id, smap, scount) < 0) return -1;
    cg_foreach(m, c->id, CG_KIND_MESSAGE, msg) {
        if (ob_printf(b, "#define ID_%s_%s 0x%08x // %s.%s\n",
                c->name, msg->name, hash2(h, c->name, msg->name),
                c->name, msg->name) < 0) return -1;
    }
    return 0;
}

static int emit_interface_msgs(cg_host_v1 const *h, ob *b, cg_model const *m, cg_node const *iface) {
    cg_foreach(m, iface->id, CG_KIND_MESSAGE, msg) {
        if (ob_printf(b, "#define ID_%s_%s 0x%08x // %s.%s\n",
                iface->name, msg->name, hash2(h, iface->name, msg->name),
                iface->name, msg->name) < 0) return -1;
    }
    return 0;
}

static int emit_struct(cg_host_v1 const *h, ob *b, cg_model const *m, cg_node const *s,
                       sentry const *smap, size_t scount) {
    if (ob_printf(b, "#define ID_%s 0x%08x\n", s->name, h->fnv1a32(s->name)) < 0) return -1;
    cg_foreach(m, s->id, CG_KIND_FIELD, f) {
        walk_ctx ctx = {0};
        char segs[MAX_DEPTH][64] = {{0}};
        int fixed = (f->extra && f->extra[0]) ? atoi(f->extra) : 0;
        ctx.b = b; ctx.m = m; ctx.h = h; ctx.class_name = s->name;
        ctx.smap = smap; ctx.scount = scount;
        if (fixed > 0) {
            if (walk_fixed_array(&ctx, segs, 0, f->name, f->type, fixed) < 0) return -1;
        } else {
            snprintf(segs[0], 64, "%s", f->name);
            if (walk_prop(&ctx, segs, 1, f->type) < 0) return -1;
        }
    }
    return 0;
}

static int emit_event_args(cg_host_v1 const *h, ob *b, cg_model const *m,
                           cg_node const *owner, cg_node const *msg) {
    char ea[512];
    cg_node const *fields_msg = msg;
    if (msg->extra && msg->extra[0]) {
        cg_foreach(m, owner->id, CG_KIND_MESSAGE, candidate) {
            if (!strcmp(candidate->name, msg->extra)) {
                fields_msg = candidate;
                break;
            }
        }
    }
    snprintf(ea, sizeof(ea), "%s_%sEventArgs", owner->name, msg->name);
    if (ob_printf(b, "#define ID_%s 0x%08x\n\n", ea, h->fnv1a32(ea)) < 0) return -1;
    cg_foreach(m, fields_msg->id, CG_KIND_FIELD, f) {
        if (ob_printf(b, "#define ID_%s_%s 0x%08x // %s.%s\n",
                ea, f->name, hash2(h, ea, f->name), ea, f->name) < 0) return -1;
    }
    return 0;
}

/* ---------- plugin entry ------------------------------------------------- */

static int emit_properties(cg_host_v1 const *host, cg_model const *model, char const *output) {
    ob b = {0};
    char guard[256]; str_upper(guard, sizeof(guard), model->module_name ? model->module_name : "module");
    size_t scount = 0;
    sentry *smap = build_struct_map(model, &scount);
    if (!smap) return -1;

    if (ob_printf(&b,
            "// Auto-generated from %s by tools/codegen/plugins/properties.c\n"
            "// DO NOT EDIT — run 'cd tools && make' to regenerate.\n"
            "#ifndef __%s_PROPERTIES_H__\n"
            "#define __%s_PROPERTIES_H__\n\n",
            base_name(model->xml_path), guard, guard) < 0) goto fail;

    cg_foreach(model, 0, CG_KIND_CLASS, c) {
        if (emit_class(host, &b, model, c, smap, scount) < 0) goto fail;
    }
    for (size_t i = 0; i < model->node_count; ++i) {
        cg_node const *msg = &model->nodes[i];
        cg_node const *owner = node_parent(model, msg);
        if (msg->kind == CG_KIND_MESSAGE && local_node(owner) &&
            (owner->kind == CG_KIND_INTERFACE || owner->kind == CG_KIND_CLASS)) {
            if (emit_message_action(host, &b, model, msg) < 0) goto fail;
        }
    }
    if (ob_printf(&b, "\n") < 0) goto fail;
    cg_foreach(model, 0, CG_KIND_INTERFACE, iface) {
        if (emit_interface_msgs(host, &b, model, iface) < 0) goto fail;
    }
    if (ob_printf(&b, "\n") < 0) goto fail;
    cg_foreach(model, 0, CG_KIND_STRUCT, s) {
        if (emit_struct(host, &b, model, s, smap, scount) < 0) goto fail;
    }
    if (ob_printf(&b, "\t\n\n") < 0) goto fail;
    cg_foreach(model, 0, CG_KIND_INTERFACE, iface) {
        cg_foreach(model, iface->id, CG_KIND_MESSAGE, msg) {
            if (emit_event_args(host, &b, model, iface, msg) < 0) goto fail;
        }
    }
    cg_foreach(model, 0, CG_KIND_CLASS, c) {
        cg_foreach(model, c->id, CG_KIND_MESSAGE, msg) {
            if (emit_event_args(host, &b, model, c, msg) < 0) goto fail;
        }
    }
    if (ob_printf(&b, "\t\n\n#endif\n") < 0) goto fail;

    free(smap);
    if (host->write_file(output, b.s, b.n) < 0) { host->logf("properties: write failed"); free(b.s); return -1; }
    free(b.s);
    return 0;
fail:
    free(smap); free(b.s); return -1;
}

static cg_plugin_v1 const plugin = { CG_API_VERSION, "properties", emit_properties };
cg_plugin_v1 const *cg_plugin_init_v1(void) { return &plugin; }
