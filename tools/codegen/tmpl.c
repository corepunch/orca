/*
 * tmpl.c — simple template engine for ORCA codegen.
 *
 * Template syntax (described in tmpl.h):
 *   {{module.name}}          — scalar value
 *   {{module.name|upper}}    — with filter
 *   {{for enums as e}}...{{end}}   — loop
 *   {{if e.doc}}...{{else}}...{{end}}  — conditional
 *   {{include sub.ctmpl}}    — sub-template
 *   {{fnv1a "literal"}}      — FNV1a32 hex of string literal
 *   {{fnv1a var.path}}       — FNV1a32 hex of runtime value
 *   {{!-- ... --}}           — comment (ignored)
 */
#include "tmpl.h"
#include <ctype.h>
#include <assert.h>

/* ------------------------------------------------------------------ */
/* Context stack for variable resolution                                */
/* ------------------------------------------------------------------ */

#define MAX_SCOPE_DEPTH 16

typedef enum {
    SV_NULL,
    SV_STRING,
    SV_MODULE,
    SV_ENUM,
    SV_ENUM_VALUE,
    SV_STRUCT,
    SV_INTERFACE,
    SV_COMPONENT,
    SV_FIELD,
    SV_PROPERTY,
    SV_METHOD,
    SV_ARG,
    SV_EVENT,
    SV_INCLUDE,
    SV_EXTERNAL,
    SV_REQUIRE,
} ScopeValueKind;

typedef struct {
    const char     *var_name; /* loop variable name */
    ScopeValueKind  kind;
    void           *ptr;      /* points to the actual object */
    char           *str_val;  /* for SV_STRING */
} ScopeValue;

typedef struct {
    ScopeValue items[MAX_SCOPE_DEPTH];
    int        depth;
    Module    *module;        /* root module always available */
    const char *template_dir;
} RenderCtx;

/* ------------------------------------------------------------------ */
/* Template buffer (loaded into memory)                                 */
/* ------------------------------------------------------------------ */

typedef struct {
    char  *data;
    size_t len;
    size_t pos;
} TmplBuf;

static TmplBuf tmpl_load(const char *path) {
    TmplBuf b = {0};
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "tmpl: cannot open template '%s'\n", path);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    b.len = (size_t)ftell(f);
    rewind(f);
    b.data = malloc(b.len + 1);
    b.len  = fread(b.data, 1, b.len, f);
    b.data[b.len] = '\0';
    fclose(f);
    return b;
}

/* ------------------------------------------------------------------ */
/* Scope helpers                                                        */
/* ------------------------------------------------------------------ */

static void ctx_push(RenderCtx *ctx, const char *name, ScopeValueKind kind, void *ptr) {
    if (ctx->depth >= MAX_SCOPE_DEPTH) { fprintf(stderr,"tmpl: scope overflow\n"); exit(1); }
    ctx->items[ctx->depth].var_name = name;
    ctx->items[ctx->depth].kind     = kind;
    ctx->items[ctx->depth].ptr      = ptr;
    ctx->items[ctx->depth].str_val  = NULL;
    ctx->depth++;
}

static void ctx_pop(RenderCtx *ctx) {
    if (ctx->depth > 0) ctx->depth--;
}

/* Look up variable 'name' in scope stack (innermost first) */
static ScopeValue *ctx_find(RenderCtx *ctx, const char *name) {
    for (int i = ctx->depth - 1; i >= 0; i--) {
        if (ctx->items[i].var_name && strcmp(ctx->items[i].var_name, name) == 0)
            return &ctx->items[i];
    }
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Value resolution via dot-path                                       */
/* ------------------------------------------------------------------ */

/*
 * Resolve a dot-separated path like "e.name", "module.name", "comp.doc"
 * against the current scope. Returns a malloc'd string or NULL.
 */
static char *resolve_path(RenderCtx *ctx, const char *path) {
    char buf[256];
    strncpy(buf, path, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';

    char *dot = strchr(buf, '.');
    char *root_name = buf;
    char *rest = NULL;
    if (dot) { *dot = '\0'; rest = dot + 1; }

    /* special root: "module" */
    ScopeValueKind kind = SV_MODULE;
    void *ptr = ctx->module;

    if (strcmp(root_name, "module") != 0) {
        ScopeValue *sv = ctx_find(ctx, root_name);
        if (!sv) return NULL;
        kind = sv->kind;
        ptr  = sv->ptr;
        if (kind == SV_STRING) {
            return str_dup(sv->str_val);
        }
    }

    if (!rest) {
        /* No sub-field requested — return name of the object */
        switch (kind) {
            case SV_MODULE:    return str_dup(((Module*)ptr)->name);
            case SV_ENUM:      return str_dup(((Enum*)ptr)->name);
            case SV_ENUM_VALUE:return str_dup(((EnumValue*)ptr)->name);
            case SV_STRUCT:    return str_dup(((Struct*)ptr)->name);
            case SV_INTERFACE: return str_dup(((Interface*)ptr)->name);
            case SV_COMPONENT: return str_dup(((Component*)ptr)->name);
            case SV_FIELD:     return str_dup(((Field*)ptr)->name.name);
            case SV_PROPERTY:  return str_dup(((Property*)ptr)->name.display);
            case SV_METHOD:    return str_dup(((Method*)ptr)->name);
            case SV_EVENT:     return str_dup(((Event*)ptr)->name);
            case SV_INCLUDE:   return str_dup(((Include*)ptr)->file);
            case SV_EXTERNAL:  return str_dup(((ExternalStruct*)ptr)->name);
            case SV_REQUIRE:   return str_dup(((Module*)ptr)->name);
            default:           return NULL;
        }
    }

    /* Resolve sub-field 'rest' */
    const char *field = rest;

    switch (kind) {
        case SV_MODULE: {
            Module *m = (Module*)ptr;
            if (!strcmp(field,"name"))         return str_dup(m->name);
            if (!strcmp(field,"prefix"))       return str_dup(m->prefix);
            if (!strcmp(field,"on_luaopen"))   return str_dup(m->on_luaopen);
            if (!strcmp(field,"after_luaopen"))return str_dup(m->after_luaopen);
            if (!strcmp(field,"name|upper"))   return str_toupper(m->name);
            return NULL;
        }
        case SV_ENUM: {
            Enum *e = (Enum*)ptr;
            if (!strcmp(field,"name"))  return str_dup(e->name);
            if (!strcmp(field,"doc"))   return str_dup(e->doc);
            if (!strcmp(field,"count")) { char tmp[32]; snprintf(tmp,sizeof(tmp),"%d",e->num_values); return strdup(tmp); }
            return NULL;
        }
        case SV_ENUM_VALUE: {
            EnumValue *v = (EnumValue*)ptr;
            if (!strcmp(field,"name")) return str_dup(v->name);
            if (!strcmp(field,"doc"))  return str_dup(v->doc);
            return NULL;
        }
        case SV_STRUCT: {
            Struct *s = (Struct*)ptr;
            if (!strcmp(field,"name"))        return str_dup(s->name);
            if (!strcmp(field,"doc"))         return str_dup(s->doc);
            if (!strcmp(field,"export"))      return str_dup(s->export_name);
            if (!strcmp(field,"prefix"))      return str_dup(s->prefix);
            if (!strcmp(field,"sealed"))      return str_dup(s->sealed ? "true" : NULL);
            return NULL;
        }
        case SV_INTERFACE: {
            Interface *ifc = (Interface*)ptr;
            if (!strcmp(field,"name"))   return str_dup(ifc->name);
            if (!strcmp(field,"doc"))    return str_dup(ifc->doc);
            if (!strcmp(field,"export")) return str_dup(ifc->export_name);
            if (!strcmp(field,"prefix")) return str_dup(ifc->prefix);
            return NULL;
        }
        case SV_COMPONENT: {
            Component *c = (Component*)ptr;
            if (!strcmp(field,"name"))        return str_dup(c->name);
            if (!strcmp(field,"doc"))         return str_dup(c->doc);
            if (!strcmp(field,"export"))      return str_dup(c->export_name);
            if (!strcmp(field,"attach_only")) return str_dup(c->attach_only ? "true" : NULL);
            return NULL;
        }
        case SV_FIELD: {
            Field *f = (Field*)ptr;
            if (!strcmp(field,"name"))  return str_dup(f->name.name);
            if (!strcmp(field,"doc"))   return str_dup(f->doc);
            if (!strcmp(field,"type"))  return type_decl(&f->type);
            if (!strcmp(field,"id"))    return str_dup(f->name.id);
            return NULL;
        }
        case SV_PROPERTY: {
            Property *p = (Property*)ptr;
            if (!strcmp(field,"name"))    return str_dup(p->name.display);
            if (!strcmp(field,"addr"))    return str_dup(p->name.addr);
            if (!strcmp(field,"id"))      return str_dup(p->name.id);
            if (!strcmp(field,"doc"))     return str_dup(p->doc);
            if (!strcmp(field,"type"))    return type_decl(&p->type);
            if (!strcmp(field,"default")) return str_dup(p->type.default_val);
            return NULL;
        }
        case SV_METHOD: {
            Method *meth = (Method*)ptr;
            if (!strcmp(field,"name"))        return str_dup(meth->name);
            if (!strcmp(field,"full_name"))   return str_dup(meth->full_name);
            if (!strcmp(field,"export"))      return str_dup(meth->export_name);
            if (!strcmp(field,"doc"))         return str_dup(meth->doc);
            if (!strcmp(field,"alias"))       return str_dup(meth->alias);
            if (!strcmp(field,"is_static"))   return str_dup(meth->is_static ? "true" : NULL);
            if (!strcmp(field,"is_lua"))      return str_dup(meth->is_lua ? "true" : NULL);
            return NULL;
        }
        case SV_EVENT: {
            Event *ev = (Event*)ptr;
            if (!strcmp(field,"name"))        return str_dup(ev->name);
            if (!strcmp(field,"ns"))          return str_dup(ev->ns);
            if (!strcmp(field,"doc"))         return str_dup(ev->doc);
            if (!strcmp(field,"parent_name")) return str_dup(ev->parent_name);
            return NULL;
        }
        case SV_INCLUDE: {
            Include *inc = (Include*)ptr;
            if (!strcmp(field,"file")) return str_dup(inc->file);
            return NULL;
        }
        case SV_EXTERNAL: {
            ExternalStruct *ext = (ExternalStruct*)ptr;
            if (!strcmp(field,"name")) return str_dup(ext->name);
            return NULL;
        }
        case SV_REQUIRE: {
            Module *req = (Module*)ptr;
            if (!strcmp(field,"name"))   return str_dup(req->name);
            if (!strcmp(field,"source")) return str_dup(req->source);
            return NULL;
        }
        default: return NULL;
    }
}

/* Is a resolved value "truthy"? */
static int is_truthy(const char *val) {
    if (!val || !*val) return 0;
    if (strcmp(val,"false")==0 || strcmp(val,"0")==0) return 0;
    return 1;
}

/* Apply filter (upper/lower/ucfirst/lcfirst) */
static char *apply_filter(char *val, const char *filter) {
    if (!val) return val;
    if (!filter || !*filter) return val;
    char *result = val;
    if (!strcmp(filter,"upper"))   { result = str_toupper(val); free(val); }
    else if (!strcmp(filter,"lower")) {
        for (char *p = val; *p; p++) *p = (char)tolower((unsigned char)*p);
        result = val;
    }
    else if (!strcmp(filter,"ucfirst")) { result = str_ucfirst(val); free(val); }
    else if (!strcmp(filter,"lcfirst")) { result = str_lcfirst(val); free(val); }
    return result;
}

/* ------------------------------------------------------------------ */
/* Forward declaration                                                  */
/* ------------------------------------------------------------------ */

static void render_buf(RenderCtx *ctx, TmplBuf *b, FILE *out);

/* ------------------------------------------------------------------ */
/* Tag parsing helpers                                                  */
/* ------------------------------------------------------------------ */

/* Read next {{...}} tag from b, return content (malloc'd) or NULL at EOF */
static char *next_tag(TmplBuf *b, FILE *out) {
    if (b->pos >= b->len) return NULL;
    size_t start = b->pos;
    /* Output literal text until {{ */
    while (b->pos < b->len) {
        if (b->data[b->pos] == '{' && b->pos+1 < b->len && b->data[b->pos+1] == '{') {
            if (b->pos > start)
                fwrite(b->data + start, 1, b->pos - start, out);
            b->pos += 2;
            /* Find closing }} */
            size_t tag_start = b->pos;
            while (b->pos < b->len) {
                if (b->data[b->pos] == '}' && b->pos+1 < b->len && b->data[b->pos+1] == '}') {
                    size_t tag_len = b->pos - tag_start;
                    char *tag = strndup(b->data + tag_start, tag_len);
                    b->pos += 2;
                    return tag;
                }
                b->pos++;
            }
            fprintf(stderr, "tmpl: unclosed '{{' in template\n");
            exit(1);
        }
        b->pos++;
    }
    /* Flush remaining literal */
    if (b->pos > start)
        fwrite(b->data + start, 1, b->pos - start, out);
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Collect body until matching {{end}} (handles nesting) */
/* ------------------------------------------------------------------ */

static char *collect_body_until_end(TmplBuf *b, const char *block_kw, int *found_else) {
    /* Collect raw text from current pos until {{end}} (or {{else}} for if) */
    size_t start = b->pos;
    int depth = 1;
    if (found_else) *found_else = 0;
    while (b->pos < b->len) {
        if (b->data[b->pos] == '{' && b->pos+1 < b->len && b->data[b->pos+1] == '{') {
            size_t tag_pos = b->pos;
            b->pos += 2;
            size_t ts = b->pos;
            while (b->pos < b->len && !(b->data[b->pos]=='}' && b->pos+1<b->len && b->data[b->pos+1]=='}'))
                b->pos++;
            size_t te = b->pos;
            if (b->pos < b->len) b->pos += 2;
            char *inner = strndup(b->data+ts, te-ts);
            /* trim */
            char *p = inner; while(*p==' ')p++;
            if (depth == 1 && found_else && strncmp(p,"else",4)==0) {
                char *body = strndup(b->data+start, tag_pos-start);
                free(inner);
                *found_else = 1;
                return body;
            }
            if (strncmp(p,"end",3)==0 && (p[3]=='\0'||p[3]==' '||p[3]=='}')) {
                depth--;
                if (depth == 0) {
                    char *body = strndup(b->data+start, tag_pos-start);
                    free(inner);
                    return body;
                }
            } else if (strncmp(p,"for ",4)==0 || strncmp(p,"if ",3)==0) {
                depth++;
            }
            free(inner);
        } else {
            b->pos++;
        }
    }
    fprintf(stderr, "tmpl: unclosed {{%s}}\n", block_kw);
    exit(1);
}

/* Skip body until {{end}} (after {{else}} body was taken) */
static void skip_until_end(TmplBuf *b) {
    int depth = 1;
    while (b->pos < b->len) {
        if (b->data[b->pos] == '{' && b->pos+1 < b->len && b->data[b->pos+1] == '{') {
            b->pos += 2;
            size_t ts = b->pos;
            while (b->pos < b->len && !(b->data[b->pos]=='}' && b->pos+1<b->len && b->data[b->pos+1]=='}'))
                b->pos++;
            size_t te = b->pos;
            if (b->pos < b->len) b->pos += 2;
            char *inner = strndup(b->data+ts, te-ts);
            char *p = inner; while(*p==' ')p++;
            if (strncmp(p,"end",3)==0 && (p[3]=='\0'||p[3]==' '||p[3]=='}')) {
                depth--;
                if (depth==0) { free(inner); return; }
            } else if (strncmp(p,"for ",4)==0 || strncmp(p,"if ",3)==0) {
                depth++;
            }
            free(inner);
        } else b->pos++;
    }
}

/* ------------------------------------------------------------------ */
/* Render a sub-body string (for loops/ifs)                           */
/* ------------------------------------------------------------------ */

static void render_body(RenderCtx *ctx, const char *body_str, FILE *out) {
    TmplBuf sub;
    sub.data = (char*)body_str;
    sub.len  = strlen(body_str);
    sub.pos  = 0;
    render_buf(ctx, &sub, out);
}

/* ------------------------------------------------------------------ */
/* For-loop expansion                                                  */
/* ------------------------------------------------------------------ */

typedef struct { ScopeValueKind kind; void *ptr; } IterItem;

static void iter_collection(RenderCtx *ctx, const char *collection,
                            IterItem **items_out, int *count_out) {
    /* collection is like "enums", "comp.fields", "e.values" etc. */
    IterItem *items = NULL;
    int count = 0;

    /* Check if it's a sub-field of a loop variable */
    char buf[256];
    strncpy(buf, collection, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';
    char *dot = strchr(buf, '.');
    char *root_name = buf;
    char *field = NULL;
    if (dot) { *dot = '\0'; field = dot+1; }

    void *ptr = ctx->module;
    ScopeValueKind kind = SV_MODULE;

    if (strcmp(root_name,"module") != 0) {
        ScopeValue *sv = ctx_find(ctx, root_name);
        if (!sv) { *items_out = NULL; *count_out = 0; return; }
        kind = sv->kind;
        ptr  = sv->ptr;
    }

    if (field) {
        /* sub-collection of a scope variable */
        switch (kind) {
            case SV_ENUM: {
                Enum *e = (Enum*)ptr;
                if (!strcmp(field,"values")) {
                    items = malloc((size_t)e->num_values * sizeof(IterItem));
                    for (int i = 0; i < e->num_values; i++) {
                        items[count].kind = SV_ENUM_VALUE;
                        items[count].ptr  = &e->values[i];
                        count++;
                    }
                }
                break;
            }
            case SV_STRUCT: {
                Struct *s = (Struct*)ptr;
                if (!strcmp(field,"fields")) {
                    items = malloc((size_t)s->num_fields * sizeof(IterItem));
                    for (int i = 0; i < s->num_fields; i++) {
                        items[count].kind = SV_FIELD;
                        items[count].ptr  = &s->fields[i];
                        count++;
                    }
                } else if (!strcmp(field,"methods")) {
                    items = malloc((size_t)s->num_methods * sizeof(IterItem));
                    for (int i = 0; i < s->num_methods; i++) {
                        items[count].kind = SV_METHOD;
                        items[count].ptr  = &s->methods[i];
                        count++;
                    }
                }
                break;
            }
            case SV_INTERFACE: {
                Interface *ifc = (Interface*)ptr;
                if (!strcmp(field,"methods")) {
                    items = malloc((size_t)ifc->num_methods * sizeof(IterItem));
                    for (int i = 0; i < ifc->num_methods; i++) {
                        items[count].kind = SV_METHOD;
                        items[count].ptr  = &ifc->methods[i];
                        count++;
                    }
                }
                break;
            }
            case SV_COMPONENT: {
                Component *c = (Component*)ptr;
                if (!strcmp(field,"fields")) {
                    items = malloc((size_t)c->num_fields * sizeof(IterItem));
                    for (int i = 0; i < c->num_fields; i++) {
                        items[count].kind = SV_FIELD;
                        items[count].ptr  = &c->fields[i];
                        count++;
                    }
                } else if (!strcmp(field,"methods")) {
                    items = malloc((size_t)c->num_methods * sizeof(IterItem));
                    for (int i = 0; i < c->num_methods; i++) {
                        items[count].kind = SV_METHOD;
                        items[count].ptr  = &c->methods[i];
                        count++;
                    }
                } else if (!strcmp(field,"properties")) {
                    items = malloc((size_t)c->num_properties * sizeof(IterItem));
                    for (int i = 0; i < c->num_properties; i++) {
                        items[count].kind = SV_PROPERTY;
                        items[count].ptr  = &c->properties[i];
                        count++;
                    }
                } else if (!strcmp(field,"handles") || !strcmp(field,"parents")) {
                    /* string arrays — skip for now, handled differently */
                }
                break;
            }
            case SV_EVENT: {
                Event *ev = (Event*)ptr;
                if (!strcmp(field,"fields")) {
                    items = malloc((size_t)ev->num_fields * sizeof(IterItem));
                    for (int i = 0; i < ev->num_fields; i++) {
                        items[count].kind = SV_FIELD;
                        items[count].ptr  = &ev->fields[i];
                        count++;
                    }
                }
                break;
            }
            case SV_METHOD: {
                Method *meth = (Method*)ptr;
                if (!strcmp(field,"args")) {
                    items = malloc((size_t)meth->num_args * sizeof(IterItem));
                    for (int i = 0; i < meth->num_args; i++) {
                        items[count].kind = SV_ARG;
                        items[count].ptr  = &meth->args[i];
                        count++;
                    }
                }
                break;
            }
            default: break;
        }
    } else {
        /* top-level module collections */
        Module *m = (Module*)ptr;
        if (!strcmp(collection,"enums")) {
            items = malloc((size_t)m->num_enums * sizeof(IterItem));
            for (int i = 0; i < m->num_enums; i++) {
                items[count].kind = SV_ENUM;
                items[count].ptr  = &m->enums[i];
                count++;
            }
        } else if (!strcmp(collection,"structs")) {
            items = malloc((size_t)m->num_structs * sizeof(IterItem));
            for (int i = 0; i < m->num_structs; i++) {
                items[count].kind = SV_STRUCT;
                items[count].ptr  = &m->structs[i];
                count++;
            }
        } else if (!strcmp(collection,"interfaces")) {
            items = malloc((size_t)m->num_interfaces * sizeof(IterItem));
            for (int i = 0; i < m->num_interfaces; i++) {
                items[count].kind = SV_INTERFACE;
                items[count].ptr  = &m->interfaces[i];
                count++;
            }
        } else if (!strcmp(collection,"components")) {
            items = malloc((size_t)m->num_components * sizeof(IterItem));
            for (int i = 0; i < m->num_components; i++) {
                items[count].kind = SV_COMPONENT;
                items[count].ptr  = &m->components[i];
                count++;
            }
        } else if (!strcmp(collection,"events")) {
            items = malloc((size_t)m->num_events * sizeof(IterItem));
            for (int i = 0; i < m->num_events; i++) {
                items[count].kind = SV_EVENT;
                items[count].ptr  = m->events[i];
                count++;
            }
        } else if (!strcmp(collection,"includes")) {
            items = malloc((size_t)m->num_includes * sizeof(IterItem));
            for (int i = 0; i < m->num_includes; i++) {
                items[count].kind = SV_INCLUDE;
                items[count].ptr  = &m->includes[i];
                count++;
            }
        } else if (!strcmp(collection,"externals")) {
            items = malloc((size_t)m->num_externals * sizeof(IterItem));
            for (int i = 0; i < m->num_externals; i++) {
                items[count].kind = SV_EXTERNAL;
                items[count].ptr  = &m->externals[i];
                count++;
            }
        } else if (!strcmp(collection,"requires")) {
            items = malloc((size_t)m->num_requires * sizeof(IterItem));
            for (int i = 0; i < m->num_requires; i++) {
                items[count].kind = SV_REQUIRE;
                items[count].ptr  = m->requires[i];
                count++;
            }
        } else if (!strcmp(collection,"functions")) {
            items = malloc((size_t)m->num_functions * sizeof(IterItem));
            for (int i = 0; i < m->num_functions; i++) {
                items[count].kind = SV_METHOD;
                items[count].ptr  = &m->functions[i];
                count++;
            }
        }
    }
    *items_out = items;
    *count_out = count;
}

/* ------------------------------------------------------------------ */
/* Main render function                                                 */
/* ------------------------------------------------------------------ */

static void handle_tag(RenderCtx *ctx, TmplBuf *b, const char *tag, FILE *out) {
    /* Trim whitespace from tag */
    char tagbuf[512];
    strncpy(tagbuf, tag, sizeof(tagbuf)-1);
    tagbuf[sizeof(tagbuf)-1] = '\0';
    char *t = tagbuf;
    while (*t == ' ' || *t == '\t') t++;
    char *tend = t + strlen(t);
    while (tend > t && (*(tend-1)==' '||*(tend-1)=='\t'||*(tend-1)=='\n'||*(tend-1)=='\r')) *--tend = '\0';

    /* Comment */
    if (strncmp(t,"!--",3)==0) return;

    /* {{end}} — handled by caller */
    if (strcmp(t,"end")==0) return;

    /* {{for COLLECTION as VAR}} */
    if (strncmp(t,"for ",4)==0) {
        char coll[128], varname[64];
        if (sscanf(t+4, "%127s as %63s", coll, varname) != 2) {
            fprintf(stderr, "tmpl: bad for syntax: {{%s}}\n", t);
            exit(1);
        }
        int found_else = 0;
        char *body = collect_body_until_end(b, "for", &found_else);
        char *else_body = NULL;
        if (found_else) {
            else_body = collect_body_until_end(b, "for else", NULL);
        }

        IterItem *items = NULL;
        int count = 0;
        iter_collection(ctx, coll, &items, &count);

        if (count == 0 && else_body) {
            render_body(ctx, else_body, out);
        } else {
            for (int i = 0; i < count; i++) {
                ctx_push(ctx, varname, items[i].kind, items[i].ptr);
                render_body(ctx, body, out);
                ctx_pop(ctx);
            }
        }
        free(body);
        if (else_body) free(else_body);
        free(items);
        return;
    }

    /* {{if EXPR}} */
    if (strncmp(t,"if ",3)==0) {
        const char *expr = t + 3;
        while (*expr==' ') expr++;
        char *val = resolve_path(ctx, expr);
        int truthy = is_truthy(val);
        free(val);

        int found_else = 0;
        char *then_body = collect_body_until_end(b, "if", &found_else);
        char *else_body = NULL;
        if (found_else) {
            else_body = collect_body_until_end(b, "if else", NULL);
        }

        if (truthy) {
            render_body(ctx, then_body, out);
        } else if (else_body) {
            render_body(ctx, else_body, out);
        }
        free(then_body);
        if (else_body) free(else_body);
        return;
    }

    /* {{include path/to/sub.ctmpl}} */
    if (strncmp(t,"include ",8)==0) {
        const char *inc_path = t + 8;
        while (*inc_path==' ') inc_path++;
        char full[1024];
        snprintf(full, sizeof(full), "%s/%s", ctx->template_dir, inc_path);
        tmpl_render(full, ctx->template_dir, ctx->module, out);
        return;
    }

    /* {{fnv1a "literal"}} or {{fnv1a var.path}} */
    if (strncmp(t,"fnv1a ",6)==0) {
        const char *arg = t + 6;
        while (*arg==' ') arg++;
        char *s;
        if (*arg == '"') {
            /* literal string */
            arg++;
            const char *end = strchr(arg, '"');
            s = end ? strndup(arg, (size_t)(end-arg)) : strdup(arg);
        } else {
            s = resolve_path(ctx, arg);
        }
        if (s) {
            fprintf(out, "0x%08x", fnv1a32(s));
            free(s);
        }
        return;
    }

    /* {{var.path}} or {{var.path|filter}} — scalar substitution */
    {
        char path[256], filter[64];
        filter[0] = '\0';
        char *pipe = strchr(t, '|');
        if (pipe) {
            size_t plen = (size_t)(pipe - t);
            strncpy(path, t, plen); path[plen] = '\0';
            strncpy(filter, pipe+1, sizeof(filter)-1);
        } else {
            strncpy(path, t, sizeof(path)-1);
        }
        char *val = resolve_path(ctx, path);
        if (val) {
            val = apply_filter(val, filter);
            fputs(val, out);
            free(val);
        }
    }
}

static void render_buf(RenderCtx *ctx, TmplBuf *b, FILE *out) {
    char *tag;
    while ((tag = next_tag(b, out)) != NULL) {
        handle_tag(ctx, b, tag, out);
        free(tag);
    }
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

void tmpl_render(const char *template_file, const char *template_dir,
                 Module *m, FILE *out) {
    TmplBuf buf = tmpl_load(template_file);
    RenderCtx ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.module       = m;
    ctx.template_dir = template_dir;
    render_buf(&ctx, &buf, out);
    free(buf.data);
}
