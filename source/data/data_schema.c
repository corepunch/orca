#include "data_schema.h"

#include <source/core/core_local.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdlib.h>
#include <string.h>

#define XMLSTR(s) ((const xmlChar *)(s))

/* DataObject class ID — inherits from DataContext which inherits from DataObject.
   Schema-generated entity classes declare this in their ParentClasses array so
   the binding system recognises them as data records. */
#define ID_DataObject  0xeb3560da
#define ID_DataContext 0xa310331c

/* Size of one column's storage slot in the component data block.
   All columns are padded to pointer size so the block is uniformly aligned. */
#define COLUMN_SLOT_SIZE (sizeof(void *))

/* Null ObjProc — schema entity classes inherit all behaviour from DataObject
   via ParentClasses; the entity class itself handles no messages directly. */
static LRESULT
entity_obj_proc(struct Object *o, void *cmp, uint32_t msg, wParam_t w, lParam_t l)
{
  (void)o; (void)cmp; (void)msg; (void)w; (void)l;
  return 0;
}

eDataType_t
DS_ColumnDataType(DSColumnType t)
{
  switch (t) {
    case DS_COL_INT:      return kDataTypeInt;
    case DS_COL_FLOAT:    return kDataTypeFloat;
    case DS_COL_BOOL:     return kDataTypeBool;
    case DS_COL_STRING:
    default:              return kDataTypeString;
  }
}

static uint32_t
col_data_size(DSColumnType t)
{
  switch (t) {
    case DS_COL_INT:    return sizeof(int);
    case DS_COL_FLOAT:  return sizeof(float);
    case DS_COL_BOOL:   return sizeof(bool_t);
    case DS_COL_STRING: return sizeof(char *);
    default:            return 0; /* relation — no inline storage */
  }
}

static DSColumnType
parse_col_type(const char *s)
{
  if (!s) return DS_COL_STRING;
  if (!strcasecmp(s, "int"))      return DS_COL_INT;
  if (!strcasecmp(s, "float"))    return DS_COL_FLOAT;
  if (!strcasecmp(s, "bool"))     return DS_COL_BOOL;
  if (!strcasecmp(s, "relation")) return DS_COL_RELATION;
  return DS_COL_STRING;
}

/* Build and register a ClassDesc for one schema entity.  Returns the
   allocated ClassDesc on success, NULL on failure.  Caller must ensure the
   entity has at least one non-relation column. */
static struct ClassDesc *
register_entity_class(const struct ds_entity *entity)
{
  /* Count leaf columns (relations get no inline slot). */
  int num_leaf = 0;
  for (int i = 0; i < entity->num_columns; i++) {
    if (entity->columns[i].type != DS_COL_RELATION)
      num_leaf++;
  }

  /* Allocate property descriptor array (leaf columns only). */
  struct PropertyType *props = calloc((size_t)num_leaf, sizeof(struct PropertyType));
  if (!props) return NULL;

  /* Build the component data block layout: columns packed at COLUMN_SLOT_SIZE
     intervals so every value pointer is naturally aligned. */
  uint32_t offset = 0;
  int pi = 0;
  for (int i = 0; i < entity->num_columns; i++) {
    const struct ds_column *col = &entity->columns[i];
    if (col->type == DS_COL_RELATION) continue;

    /* Column name strings are owned by the ds_entity which outlives the class.
       PropertyType.Name and .Category point directly into them. */
    props[pi].Name            = col->name;
    props[pi].Category        = entity->name;
    props[pi].DataType        = DS_ColumnDataType(col->type);
    props[pi].DataSize        = col_data_size(col->type);
    /* ShortIdentifier = fnv1a32(col->name) — binding terminal segment match */
    props[pi].ShortIdentifier = fnv1a32(col->name);
    /* FullIdentifier = fnv1a32("Entity.Column") — qualified lookup */
    char full[128];
    snprintf(full, sizeof(full), "%s.%s", entity->name, col->name);
    props[pi].FullIdentifier  = fnv1a32(full);
    props[pi].Offset          = offset;

    offset += COLUMN_SLOT_SIZE;
    pi++;
  }

  /* Allocate ClassDesc.  The ParentClasses array ends with a zero sentinel. */
  struct ClassDesc *cls = calloc(1, sizeof(struct ClassDesc));
  if (!cls) { free(props); return NULL; }

  cls->ObjProc        = entity_obj_proc;
  cls->Properties     = props;
  cls->NumProperties  = (uint32_t)num_leaf;
  cls->ClassName      = entity->name;   /* points into ds_entity, same lifetime */
  cls->ClassID        = fnv1a32(entity->name);
  cls->ClassSize      = offset;         /* total bytes for all leaf column slots */
  cls->MemorySize     = offset;
  cls->ParentClasses[0] = ID_DataObject;
  cls->ParentClasses[1] = ID_DataContext;
  cls->ParentClasses[2] = 0;

  if (!OBJ_RegisterClass(cls)) {
    free(props);
    free(cls);
    return NULL;
  }
  return cls;
}

static struct ds_schema *
parse_schema_doc(xmlDocPtr doc, const char *source_label)
{
  xmlNodePtr root = xmlDocGetRootElement(doc);
  if (!root || strcmp((const char *)root->name, "Schema") != 0) {
    Con_Error("DS_ParseSchema: root element must be <Schema> in '%s'", source_label);
    return NULL;
  }

  struct ds_schema *schema = calloc(1, sizeof(struct ds_schema));
  if (!schema) return NULL;

  for (xmlNodePtr en = root->children; en; en = en->next) {
    if (en->type != XML_ELEMENT_NODE) continue;
    if (strcmp((const char *)en->name, "Entity") != 0) continue;
    if (schema->num_entities >= DS_MAX_ENTITIES) {
      Con_Warning("DS_ParseSchema: too many entities in '%s', ignoring remainder", source_label);
      break;
    }

    xmlChar *ename = xmlGetProp(en, XMLSTR("Name"));
    if (!ename) continue;

    struct ds_entity *entity = &schema->entities[schema->num_entities++];
    strncpy(entity->name, (const char *)ename, sizeof(entity->name) - 1);
    xmlFree(ename);

    for (xmlNodePtr cn = en->children; cn; cn = cn->next) {
      if (cn->type != XML_ELEMENT_NODE) continue;
      if (strcmp((const char *)cn->name, "Column") != 0) continue;
      if (entity->num_columns >= DS_MAX_COLUMNS) continue;

      xmlChar *cname   = xmlGetProp(cn, XMLSTR("Name"));
      xmlChar *ctype   = xmlGetProp(cn, XMLSTR("Type"));
      xmlChar *centity = xmlGetProp(cn, XMLSTR("Entity"));
      xmlChar *ckey    = xmlGetProp(cn, XMLSTR("Key"));
      if (!cname) { xmlFree(ctype); xmlFree(centity); xmlFree(ckey); continue; }

      struct ds_column *col = &entity->columns[entity->num_columns++];
      strncpy(col->name, (const char *)cname, sizeof(col->name) - 1);
      col->type   = parse_col_type(ctype ? (const char *)ctype : NULL);
      col->is_key = ckey && strcasecmp((const char *)ckey, "true") == 0;
      if (centity)
        strncpy(col->entity, (const char *)centity, sizeof(col->entity) - 1);

      xmlFree(cname); xmlFree(ctype); xmlFree(centity); xmlFree(ckey);
    }

    /* Register a class for this entity if it has any leaf columns. */
    int has_leaf = 0;
    for (int i = 0; i < entity->num_columns; i++)
      if (entity->columns[i].type != DS_COL_RELATION) has_leaf = 1;

    if (has_leaf) {
      struct ClassDesc *cls = register_entity_class(entity);
      if (cls && schema->num_classes < DS_MAX_ENTITIES)
        schema->classes[schema->num_classes++] = cls;
    }
  }

  return schema;
}

struct ds_schema *
DS_ParseSchema(const char *path)
{
  if (!path || !*path) return NULL;
  xmlDocPtr doc = xmlParseFile(path);
  if (!doc) {
    Con_Error("DS_ParseSchema: failed to parse '%s'", path);
    return NULL;
  }
  struct ds_schema *schema = parse_schema_doc(doc, path);
  xmlFreeDoc(doc);
  return schema;
}

struct ds_schema *
DS_ParseSchemaFromString(const char *xml)
{
  if (!xml || !*xml) return NULL;
  xmlDocPtr doc = xmlReadMemory(xml, (int)strlen(xml), "schema", NULL, 0);
  if (!doc) {
    Con_Error("DS_ParseSchemaFromString: failed to parse schema XML");
    return NULL;
  }
  struct ds_schema *schema = parse_schema_doc(doc, "<string>");
  xmlFreeDoc(doc);
  return schema;
}

void
DS_FreeSchema(struct ds_schema *schema)
{
  if (!schema) return;
  for (int i = 0; i < schema->num_classes; i++) {
    struct ClassDesc *cls = schema->classes[i];
    if (cls) {
      OBJ_UnregisterClass(cls->ClassID);
      free((void *)cls->Properties); /* cast away const — we own this */
      free(cls);
    }
  }
  free(schema);
}

const struct ds_column *
DS_FindColumn(const struct ds_schema *schema,
              const char *entity,
              const char *column)
{
  if (!schema || !column) return NULL;
  for (int e = 0; e < schema->num_entities; e++) {
    if (entity && strcmp(schema->entities[e].name, entity) != 0) continue;
    for (int c = 0; c < schema->entities[e].num_columns; c++) {
      if (strcmp(schema->entities[e].columns[c].name, column) == 0)
        return &schema->entities[e].columns[c];
    }
  }
  return NULL;
}

const struct ds_entity *
DS_FindEntity(const struct ds_schema *schema, const char *name)
{
  if (!schema || !name) return NULL;
  for (int i = 0; i < schema->num_entities; i++)
    if (strcmp(schema->entities[i].name, name) == 0)
      return &schema->entities[i];
  return NULL;
}
