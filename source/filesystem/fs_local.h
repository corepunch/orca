#ifndef ORCA_FILESYSTEM_H
#define ORCA_FILESYSTEM_H

#include <include/orca.h>

#include <filesystem/filesystem.h>

struct ds_schema;
struct ds_column {
  char name[64];
  char type[32];
  bool_t is_key;
};

struct _xmlNode*
__xmlNewChild(struct _xmlNode* p, lpcString_t name, lpcString_t args[]);

#define _xmlNewChild(NODE, NAME, ...) \
__xmlNewChild(NODE, NAME, ((lpcString_t[]){ __VA_ARGS__, NULL }));

#define _xmlSetProp(NODE, NAME, VALUE) \
xmlSetProp(NODE, XMLSTR(NAME), XMLSTR(VALUE));

#define _xmlNodeSetContent(NODE, TEXT) \
xmlNodeSetContent(NODE, XMLSTR(TEXT));

#define _xmlNodeSetContentFormat(NODE, FMT, ...) do { \
char buf_##__LINE__[256]; \
snprintf(buf_##__LINE__, sizeof(buf_##__LINE__), FMT, ##__VA_ARGS__); \
xmlNodeSetContent(NODE, XMLSTR(buf_##__LINE__)); \
} while (0)

#define _xmlSetPropFormat(NODE, PROP, FMT, ...) do { \
char buf_##__LINE__[256]; \
snprintf(buf_##__LINE__, sizeof(buf_##__LINE__), FMT, ##__VA_ARGS__); \
xmlSetProp(NODE, XMLSTR(PROP), XMLSTR(buf_##__LINE__)); \
} while (0)

struct file *_ReadOnDisk(FILE *fp);

struct Object *
FS_LoadObjectFromXml(lpcString_t path);

void
FS_RegisterDataSourceProvider(const char *type_name,
                             struct Object *(*fetch)(const char *params));

void
FS_RegisterDataProvider(const char *type_name,
                        struct Object *(*fetch)(const char *params),
                        bool_t (*save)(struct Object *root, const char *params),
                        bool_t (*revert)(struct Object *root, const char *params));

void
FS_RegisterDataSource(const char *name, const char *type, const char *params);

struct ds_schema;
struct ds_column;

struct Object *
FS_ResolveDataSource(const char *name, const char **out_params);

struct ds_schema const *
FS_GetDataSourceSchema(const char *name);

const struct ds_column *
FS_FindSchemaColumn(const struct ds_schema *schema, const char *entity,
                    const char *column);

void
FS_RegisterProjectDataSource(struct Object *project, const char *name,
                             const char *type, const char *params,
                             const char *schema);

void
FS_ClearProjectDataSources(struct Object *project);

void
FS_ClearAllDataSources(void);

void
FS_MarkDataSourceDirty(const char *name);

bool_t
FS_IsDataSourceDirty(const char *name);

bool_t
FS_SaveDataSource(const char *name);

bool_t
FS_RevertDataSource(const char *name);

struct Object *
_xml_ds_fetch(const char *params);

bool_t
_xml_ds_save(struct Object *root, const char *params);

bool_t
_xml_ds_revert(struct Object *root, const char *params);

#endif
