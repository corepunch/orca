#ifndef ORCA_DATA_SCHEMA_H
#define ORCA_DATA_SCHEMA_H

#include <include/orca.h>

#define DS_MAX_ENTITIES   32
#define DS_MAX_COLUMNS    64
#define DS_MAX_FIELD_SIZE 256  /* max bytes for one column value in component block */

typedef enum {
  DS_COL_STRING = 0,
  DS_COL_INT,
  DS_COL_FLOAT,
  DS_COL_BOOL,
  DS_COL_RELATION,  /* nested entity — child DataObject container, not a leaf value */
} DSColumnType;

struct ds_column {
  char         name[64];
  DSColumnType type;
  char         entity[64]; /* for DS_COL_RELATION: referenced entity name */
  bool_t       is_key;
};

struct ds_entity {
  char             name[64];
  struct ds_column columns[DS_MAX_COLUMNS];
  int              num_columns;
};

struct ds_schema {
  struct ds_entity entities[DS_MAX_ENTITIES];
  int              num_entities;
  /* dynamically registered ClassDesc pointers — unregistered on schema free */
  struct ClassDesc *classes[DS_MAX_ENTITIES];
  int               num_classes;
};

/* Parse a Schema XML file and register one ClassDesc per entity into the class
   registry.  Returns NULL on failure.  Caller owns the returned schema and must
   call DS_FreeSchema() to unregister classes and release memory. */
struct ds_schema *DS_ParseSchema(const char *path);

/* Parse a Schema from an in-memory XML string.  Same ownership rules as
   DS_ParseSchema. */
struct ds_schema *DS_ParseSchemaFromString(const char *xml);

/* Unregister all entity classes and free the schema. */
void DS_FreeSchema(struct ds_schema *schema);

/* Look up a column descriptor by entity + column name.  Returns NULL if not
   found; entity may be NULL to search all entities (first match wins). */
const struct ds_column *DS_FindColumn(const struct ds_schema *schema,
                                      const char *entity,
                                      const char *column);

/* Return the entity descriptor for the given name, or NULL. */
const struct ds_entity *DS_FindEntity(const struct ds_schema *schema,
                                      const char *name);

/* Map DSColumnType to the property system's eDataType_t. */
eDataType_t DS_ColumnDataType(DSColumnType t);

#endif /* ORCA_DATA_SCHEMA_H */
