#ifndef ORCA_CG_API_H
#define ORCA_CG_API_H

#include <stddef.h>
#include <stdint.h>

#define CG_API_VERSION 1u

/* Node kind — matches XML element name */
typedef enum cg_kind {
    CG_KIND_MODULE = 1, CG_KIND_REQUIRE, CG_KIND_INCLUDE, CG_KIND_EXTERNAL,
    CG_KIND_ENUM, CG_KIND_VALUE,
    CG_KIND_INTERFACE, CG_KIND_STRUCT, CG_KIND_CLASS,
    CG_KIND_METHOD, CG_KIND_MESSAGE, CG_KIND_FIELD, CG_KIND_PROPERTY,
    CG_KIND_HANDLE, CG_KIND_TOPIC, CG_KIND_ARG, CG_KIND_RETURNS
} cg_kind;

/* Flags packed into cg_node.flags */
#define CG_FLAG_SEALED      (1u<<0)  /* struct: sealed="true" */
#define CG_FLAG_STATIC      (1u<<1)  /* method: static="true" */
#define CG_FLAG_LUA         (1u<<2)  /* method: lua="true" */
#define CG_FLAG_NOEXPORT    (1u<<3)  /* field: noexport="true" */
#define CG_FLAG_POINTER     (1u<<4)  /* field/property/arg: pointer="true" */
#define CG_FLAG_CONST       (1u<<5)  /* field/method/arg: const="true" */
#define CG_FLAG_ARRAY       (1u<<6)  /* property/field: array="true" or fixed-array set */
#define CG_FLAG_NO_CHECK    (1u<<7)  /* interface: no-check="true" */
#define CG_FLAG_PRIVATE     (1u<<8)  /* property/method: private="true" */

/*
 * cg_node — one XML entity in the flat model array.
 *
 * name  : primary identifier (element name, message attr, file path, …)
 * type  : C type / routing / prefix / parent-class list   (varies by kind)
 * extra : export name / default / same-as / fixed-array   (varies by kind)
 * doc   : <summary> child text, or direct text content; NULL if absent
 *
 * Field semantics by kind:
 *   MODULE    : name=module-name  type=namespace    extra=prefix
 *   REQUIRE   : name=file         type=""           extra=""
 *   INCLUDE   : name=file         type=""           extra=""
 *   EXTERNAL  : name=struct-name  type=""           extra=""
 *   ENUM      : name=enum-name    type=""           extra=""
 *   VALUE     : name=value-name   type=text-content extra=""
 *   INTERFACE : name=iface-name   type=prefix       extra=export-name
 *   STRUCT    : name=struct-name  type=prefix       extra=export-name
 *   CLASS     : name=class-name   type=parent-list  extra=concept
 *   METHOD    : name=method-name  type=return-type  extra=export-name
 *   MESSAGE   : name=msg-name     type=routing      extra=same-as (parent msg)
 *   FIELD     : name=field-name   type=C-type       extra=fixed-array-count
 *   PROPERTY  : name=prop-name    type=C-type       extra=default-value
 *   HANDLE    : name=NS.MsgName   type=""           extra=""
 *   ARG       : name=arg-name     type=C-type       extra=default-value
 *   RETURNS   : name=""           type=C-type       extra=default-value
 */
typedef struct cg_node {
    uint32_t   id;      /* 1-based sequential */
    uint32_t   parent;  /* parent node id; 0 = module root */
    cg_kind    kind;
    uint32_t   flags;   /* CG_FLAG_* bitmask */
    char const *name;
    char const *type;
    char const *extra;
    char const *aux;
    char const *aux2;
    char const *doc;    /* may be NULL */
} cg_node;

typedef struct cg_model {
    char const  *xml_path;
    char const  *module_name;
    char const  *module_ns;
    char const  *prefix;
    char const  *on_luaopen;
    char const  *after_luaopen;
    cg_node const *nodes;
    size_t         node_count;
} cg_model;

/* --- Inline iteration helpers -------------------------------------------- */

static inline cg_node const *cg_first(cg_model const *m, uint32_t parent, cg_kind k) {
    size_t i;
    for (i = 0; i < m->node_count; ++i) {
        cg_node const *n = &m->nodes[i];
        if (n->parent == parent && (!k || n->kind == k)) return n;
    }
    return NULL;
}

static inline cg_node const *cg_next(cg_model const *m, cg_node const *cur,
                                     uint32_t parent, cg_kind k) {
    cg_node const *p;
    for (p = cur + 1; p < m->nodes + m->node_count; ++p)
        if (p->parent == parent && (!k || p->kind == k)) return p;
    return NULL;
}

static inline cg_node const *cg_node_by_id(cg_model const *m, uint32_t id) {
    return (id && id <= m->node_count) ? &m->nodes[id - 1u] : NULL;
}

/* Iterate direct children of parent_id with given kind (0 = any kind).
 * Usage:  cg_foreach(model, parent_id, CG_KIND_PROPERTY, p) { … }  */
#define cg_foreach(m, parent_id, _kind, var) \
    for (cg_node const *(var) = cg_first(m, parent_id, _kind); \
         (var); (var) = cg_next(m, var, parent_id, _kind))

/* --- Plugin ABI ----------------------------------------------------------- */

typedef struct cg_host_v1 {
    uint32_t abi;
    void     (*logf)(char const *fmt, ...);
    int      (*write_file)(char const *path, char const *data, size_t len);
    uint32_t (*fnv1a32)(char const *s);
} cg_host_v1;

typedef struct cg_plugin_v1 {
    uint32_t   abi;
    char const *name;
    int        (*emit)(cg_host_v1 const *host, cg_model const *model, char const *output);
} cg_plugin_v1;

typedef cg_plugin_v1 const *(*cg_plugin_init_fn)(void);

#endif
