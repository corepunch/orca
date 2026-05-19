#ifndef ORCA_CG_API_H
#define ORCA_CG_API_H

#include <stddef.h>
#include <stdint.h>

#define CG_API_VERSION 1u

typedef enum cg_kind {
    CG_KIND_MODULE = 1,
    CG_KIND_INCLUDE,
    CG_KIND_EXTERNAL,
    CG_KIND_ENUM,
    CG_KIND_INTERFACE,
    CG_KIND_STRUCT,
    CG_KIND_CLASS,
    CG_KIND_METHOD,
    CG_KIND_MESSAGE,
    CG_KIND_FIELD,
    CG_KIND_PROPERTY,
    CG_KIND_HANDLE,
    CG_KIND_TOPIC
} cg_kind;

typedef struct cg_row {
    uint32_t id;
    uint32_t owner;
    cg_kind kind;
    char const *name;
    char const *type;
    char const *value;
} cg_row;

typedef struct cg_model {
    char const *xml_path;
    char const *module_name;
    char const *prefix;
    cg_row const *rows;
    size_t row_count;
} cg_model;

typedef struct cg_host_v1 {
    uint32_t abi;
    void (*logf)(char const *fmt, ...);
    int (*write_file)(char const *path, char const *data, size_t len);
    uint32_t (*fnv1a32)(char const *s);
} cg_host_v1;

typedef struct cg_plugin_v1 {
    uint32_t abi;
    char const *name;
    int (*emit)(cg_host_v1 const *host, cg_model const *model, char const *output);
} cg_plugin_v1;

typedef cg_plugin_v1 const *(*cg_plugin_init_fn)(void);

#endif
