#include <source/filesystem/fs_local.h>
#include <source/core/core_local.h>
#include <source/core/property/property_internal.h>
#include <core/core_properties.h>
#include <source/data/data_schema.h>
#include "fs_struct.h"
#include <ctype.h>
#include <strings.h>

extern int parse_property(const char* str,
                          struct PropertyType const* prop,
                          void* valueptr);
extern void const *PROP_GetRawValueSlot(struct Property const *property);

static struct Object *node(struct _xmlNode* x, const struct ds_schema *schema,
                           const char *entity_name);

static bool_t
has_attr_ci(struct _xmlNode *x, lpcString_t key)
{
  FOR_EACH_LIST(xmlAttr, a, x->properties) {
    if (!strcasecmp((lpcString_t)a->name, key)) {
      return TRUE;
    }
  }
  return FALSE;
}

static lpcString_t
alias_class_tag(lpcString_t tag)
{
  if (!tag) return tag;
  if (!strcasecmp(tag, "group")) return "DataObject";
  if (!strcasecmp(tag, "object")) return "DataObject";
  if (!strcasecmp(tag, "string")) return "DataObjectString";
  if (!strcasecmp(tag, "int")) return "DataObjectInt";
  if (!strcasecmp(tag, "float")) return "DataObjectFloat";
  if (!strcasecmp(tag, "bool")) return "DataObjectBool";
  return tag;
}

static lpcString_t
canonical_attr_name(lpcString_t raw, fixedString_t out)
{
  if (!raw || !*raw) return raw;
  if (!strcasecmp(raw, "class")) return "class";
  if (!strcasecmp(raw, "name")) return "Name";
  if (!strcasecmp(raw, "value")) return "Value";
  if (!strcasecmp(raw, "classname")) return "ClassName";
  if (!strcasecmp(raw, "placeholdertemplate")) return "PlaceholderTemplate";
  if (!strcasecmp(raw, "styleclass")) return "StyleClass";
  if (!strcasecmp(raw, "datacontextsource")) return "DataContextSource";
  if (!strcasecmp(raw, "target")) return "Target";

  strncpy(out, raw, sizeof(fixedString_t) - 1);
  out[sizeof(fixedString_t) - 1] = '\0';
  if (out[0]) {
    out[0] = (char)toupper((unsigned char)out[0]);
  }
  return out;
}

static struct PropertyType const *
propdesc(struct Object *o, lpcString_t name)
{
  struct PropertyType const *pd = OBJ_FindImplicitPropertyType(o, name);
  if (pd) {
    return pd;
  }
  pd = OBJ_FindExplicitPropertyType(o, name);
  if (pd) {
    return pd;
  }
  return NULL;
}

static bool_t
special_attr(struct Object *o, lpcString_t name, lpcString_t value)
{
  if (!strcmp(name, "ClassName") || !strcmp(name, "PlaceholderTemplate")) return TRUE;
  if (!strcmp(name, "Name") || !strcmp(name, "id")) {
    OBJ_SetName(o, value);
    return TRUE;
  }
  if (!strcmp(name, "class") || !strcmp(name, "StyleClass")) {
    _SendMessage(o, StyleController, AddClasses, .ClassNames = value);
    return TRUE;
  }
  if (!strcmp(name, "Controller")) {
    struct PropertyType const *pd = propdesc(o, name);
    if (pd && pd->DataType == kDataTypeString)
      PROP_SetStringValue(OBJ_FindLongProperty(o, pd->FullIdentifier), value);
    // Queue controller load immediately so it's processed before any BindHandler.
    struct Object_LoadControllerEventArgs args = { .Path = value };
    axPostMessageDataW(o, ID_Object_LoadController, 0, &args, sizeof(args));
    return TRUE;
  }
  if (!strcmp(name, "DataContextSource")) {
    const char *colon = strrchr(value, ':');
    const char *ds_name = NULL;

    if (colon) {
      // "Project/DataSourceLibrary/Name:child" — strip the child suffix
      static char ds_buf[256];
      size_t pathLen = colon - value;
      if (pathLen >= sizeof(ds_buf)) pathLen = sizeof(ds_buf) - 1;
      memcpy(ds_buf, value, pathLen);
      ds_buf[pathLen] = '\0';
      ds_name = ds_buf;
    } else {
      ds_name = value;
    }

    // Datasources are referenced by their fully qualified
    // "Project/Library/Name" path. Keep direct file paths as a deprecated
    // compatibility fallback.
    struct Object *dataSource = FS_FindDataSource(ds_name);
    struct Object *dataObj = FS_ResolveDataSource(ds_name, NULL);
    if (!dataObj) {
      // Fallback: treat as a direct file path (deprecated)
      Con_Warning("DataContextSource='%s' uses direct file path; migrate to datasource name (e.g. '%s')",
                  value, ds_name);
      dataObj = FS_LoadObject(ds_name);
    }

    if (colon && dataObj) {
      struct Object *child = OBJ_FindByPath(dataObj, colon + 1);
      if (child) {
        PROP_SetValue(OBJ_FindLongProperty(o, ID_Node_DataContext), &child);
      }
    } else if (dataSource) {
      PROP_SetValue(OBJ_FindLongProperty(o, ID_Node_DataContext), &dataSource);
    } else {
      PROP_SetValue(OBJ_FindLongProperty(o, ID_Node_DataContext), &dataObj);
    }
    return TRUE;
  }
  return FALSE;
}

static char *
FS_BindingTrim(char *text)
{
  while (isspace((unsigned char)*text)) text++;
  char *end = text + strlen(text);
  while (end > text && isspace((unsigned char)end[-1])) *--end = 0;
  return text;
}

static bool_t
FS_BindingMode(char const *text, enum BindingMode *mode)
{
  const char *names[] = { "OneWay", "TwoWay", "OneWayToSource", "Expression" };
  for (int i = 0; i < ARRAY_SIZE(names); i++)
    if (!strcmp(text, names[i])) { *mode = i; return TRUE; }
  Con_Error("Unknown binding mode '%s'", text);
  return FALSE;
}

static bool_t
append_object(struct Property *p, struct Object *item)
{
  struct PropertyType const *pd = PROP_GetDesc(p);
  void *old_items = NULL;
  int old_count = 0;
  if (!pd || !pd->IsArray || pd->DataType != kDataTypeObject) return FALSE;
  if (PROP_GetValue(p)) {
    memcpy(&old_items, PROP_GetValue(p), sizeof(old_items));
    memcpy(&old_count, (char *)PROP_GetValue(p) + sizeof(old_items), sizeof(old_count));
  }

  int count = old_count + 1;
  struct Object **items = calloc((size_t)count, sizeof(struct Object *));
  if (!items) return FALSE;
  if (old_items && old_count > 0) {
    memcpy(items, old_items, (size_t)old_count * sizeof(struct Object *));
    for (int i = 0; i < old_count; i++) if (items[i]) OBJ_AddRef(items[i]);
  }
  items[old_count] = item;

  struct { void *items; int count; } value = { items, count };
  PROP_SetValue(p, &value);
  for (int i = 0; i < old_count; i++) if (items[i]) OBJ_ReleaseRef(items[i]);
  return TRUE;
}

static bool_t
inline_event_object(struct Object *o, struct PropertyType const *pd, struct Object *action)
{
  struct Property *triggers = NULL;
  struct PropertyType const *td = OBJ_FindImplicitPropertyType(o, "Triggers");
  struct Object *trigger = action ? OBJ_Create(ID_EventTrigger) : NULL;
  char routed[MAX_PROPERTY_STRING] = {0};

  if (!td || !(triggers = OBJ_FindLongProperty(o, td->FullIdentifier)) || !trigger) {
    if (action) OBJ_ReleaseRef(action);
    if (trigger) OBJ_ReleaseRef(trigger);
    Con_Error("Property '%s' does not support inline trigger shorthand", pd->Name);
    return FALSE;
  }
  snprintf(routed, sizeof(routed),
           (pd->Category && *pd->Category) ? "%s.%s" : "%s",
           (pd->Category && *pd->Category) ? pd->Category : pd->Name,
           pd->Name);
  lpcString_t routed_value = routed;
  struct Property* evt = OBJ_FindLongProperty(trigger, ID_EventTrigger_RoutedEvent);
  if (!evt) {
    OBJ_ReleaseRef(trigger);
    OBJ_ReleaseRef(action);
    return FALSE;
  }
  PROP_SetValue(evt, &routed_value);
  OBJ_AddChild(trigger, action);
  OBJ_ReleaseRef(action);
  bool_t ok = append_object(triggers, trigger);
  OBJ_ReleaseRef(trigger);
  return ok;
}

static void
set_text(struct Object *o, struct PropertyType const *pd, lpcString_t value)
{
  struct Property *p = NULL;
  char tmp[MAX_PROPERTY_STRING] = {0};
  if (!value) return;
  p = OBJ_FindLongProperty(o, pd->FullIdentifier);
  if (!p) {
    Con_Error("Could not get property slot for '%s'", pd->Name);
    return;
  }

  switch (value[0]) {
    case '$':
      value = FS_GetThemeValue(value);
      if (!value) {
        Con_Error("Could not resolve theme value for property '%s'", pd->Name);
        return;
      }
      break;
    case '@': {
      if (pd->DataType == kDataTypeString) break;
      struct Object *ref = OBJ_FindByPath(o, value + 1);
      if (ref && pd->DataType == kDataTypeObject) PROP_SetValue(p, &ref);
      else Con_Error("Could not resolve object reference '%s' for property '%s'", value + 1, pd->Name);
      return;
    }
    case '{':
    case '<':
      if (pd->DataType == kDataTypeString) break;
      Con_Error("Inline XML/brace objects are not supported; use a Lisp node value");
      return;
  }
  // Plain string on an event property → defer to BindHandler (controller not loaded yet).
  if (pd->DataType == kDataTypeEvent) {
    PROP_BindHandler(p, value);
    return;
  }
  if (!parse_property(value, pd, tmp)) {
    Con_Error("Could not convert value '%s' for property '%s' on class '%s'",
              value, pd->Name, OBJ_GetClassName(o));
    return;
  }
  PROP_SetValue(p, tmp);
  if (pd->DataType == kDataTypeString) free(*(char**)tmp);
}

static bool_t
set_shorthand_text(struct Object *o, lpcString_t name, lpcString_t value)
{
  return value && *value && OBJ_SetShorthandValueFromString(o, name, value);
}

static bool_t
fill_struct(void *dst, struct PropertyType const *pd, struct _xmlNode* x)
{
  struct StructDesc const *sd = OBJ_FindStructDesc(pd->TypeString);
  bool_t ok = sd && dst;

  if (!ok) {
    Con_Error("Could not prepare struct '%s' for property '%s'", pd->TypeString, pd->Name);
    return FALSE;
  }

  FOR_EACH_LIST(xmlAttr, a, x->properties) {
    xmlChar *v = xmlGetProp(x, a->name);
    struct PropertyType const *fd = _FindStructField(sd, (lpcString_t)a->name);
    if (!v || !fd || !_SetStructFieldFromString(fd, (char *)dst + fd->Offset, (lpcString_t)v)) {
      ok = FALSE;
      Con_Error("Could not set field '%s' on struct '%s'", a->name, sd->StructName);
    }
    if (v) xmlFree(v);
  }

  if (!ok) _ClearStructValue(sd, dst);
  return ok;
}

static bool_t
set_struct_node(struct Property *p, struct PropertyType const *pd, struct _xmlNode* x)
{
  struct StructDesc const *sd = OBJ_FindStructDesc(pd->TypeString);
  void *tmp = sd ? calloc(1, (size_t)sd->StructSize) : NULL;
  bool_t ok = fill_struct(tmp, pd, x);

  if (ok) PROP_SetValue(p, tmp);
  free(tmp);
  return ok;
}

static bool_t
array_prop(struct Object *o, struct PropertyType const *pd, struct _xmlNode* x)
{
  struct Property *p = NULL;
  struct StructDesc const *sd = pd->DataType == kDataTypeStruct ? OBJ_FindStructDesc(pd->TypeString) : NULL;
  int count = 0;
  int index = 0;

  p = OBJ_FindLongProperty(o, pd->FullIdentifier);
  if (!p) {
    Con_Error("Could not get property slot for '%s'", pd->Name);
    return FALSE;
  }
  if (pd->DataType != kDataTypeObject && pd->DataType != kDataTypeStruct) {
    Con_Error("array property '%s' not supported in XML loader", pd->Name);
    return FALSE;
  }
  if (pd->DataType == kDataTypeStruct && !sd) {
    Con_Error("Could not resolve struct descriptor '%s' for array property '%s'", pd->TypeString, pd->Name);
    return FALSE;
  }

  xmlForEach(c, x) count++;
  void *items = count ? calloc((size_t)count, (size_t)pd->DataSize) : NULL;
  if (count && !items) return FALSE;

  xmlForEach(c, x) {
    if (pd->DataType == kDataTypeObject) {
      struct Object *child = node(c, NULL, NULL);
      if (child) ((struct Object **)items)[index++] = child;
    } else {
      void *dst = (char *)items + (size_t)index * pd->DataSize;
      if (fill_struct(dst, pd, c)) index++;
    }
  }

  if (index != count) {
    for (int i = 0; i < index; i++) {
      if (pd->DataType == kDataTypeObject) OBJ_ReleaseRef(((struct Object **)items)[i]);
      else _ClearStructValue(sd, (char *)items + (size_t)i * pd->DataSize);
    }
    free(items);
    return FALSE;
  }
  struct { void *items; int count; } value = { items, index };
  PROP_SetValue(p, &value);
  if (pd->DataType == kDataTypeObject) {
    for (int i = 0; i < index; i++) OBJ_ReleaseRef(((struct Object **)items)[i]);
  }
  return TRUE;
}

static bool_t
property_node(struct Object *o, struct PropertyType const *pd, struct _xmlNode* x)
{
  struct Property *p = NULL;
  if (pd->IsArray) {
    return array_prop(o, pd, x);
  }
  p = OBJ_FindLongProperty(o, pd->FullIdentifier);
  if (!p) {
    Con_Error("Could not get property slot for '%s'", pd->Name);
    return TRUE;
  }

  if (pd->DataType == kDataTypeStruct && x->properties) {
    return set_struct_node(p, pd, x);
  }

  xmlChar *text = xmlNodeGetContent(x);
  if (text && *text && (!x->children || x->children->type == XML_TEXT_NODE)) {
    set_text(o, pd, (lpcString_t)text);
    xmlFree(text);
    return TRUE;
  }
  if (text) xmlFree(text);

  xmlForEach(c, x) {
    if (pd->DataType == kDataTypeStruct &&
        strcmp((char const *)c->name, "Binding") && strcmp((char const *)c->name, "BindingExpression")) {
      return set_struct_node(p, pd, c);
    }
    struct Object *child = node(c, NULL, NULL);
    if (!child) return FALSE;
    if (pd->DataType == kDataTypeEvent) {
      return inline_event_object(o, pd, child);
    } else if (GetBinding(child) || GetBindingExpression(child)) {
      OBJ_SendMessageW(child, ID_Binding_Compile, 0, p);
      OBJ_ReleaseRef(child);
    } else if (pd->DataType == kDataTypeObject) {
      PROP_SetValue(p, &child);
      OBJ_ReleaseRef(child);
    } else {
      OBJ_ReleaseRef(child);
      Con_Error("Property '%s' does not accept object child", pd->Name);
      return FALSE;
    }
    return TRUE;
  }
  return TRUE;
}

/* Bind a property, or each numeric field a shorthand such as Position or
   Color expands to. Shorthand fields only take one-way values. */
static void
binding_target(struct Object *o, char const *target, char const *program, enum BindingMode mode, bool_t enabled)
{
  struct PropertyShorthand const *sh = OBJ_FindImplicitShorthand(o, target);
  if (!sh) sh = OBJ_FindExplicitShorthand(o, target);
  if (sh) {
    static const char *extract[] = { "VECTOR3X", "VECTOR3Y", "VECTOR3Z", "COLORA" };
    if (mode != kBindingModeOneWay && mode != kBindingModeExpression) {
      Con_Error("Binding shorthand '%s' supports only one-way bindings", target);
      return;
    }
    for (int i = 0; i < sh->NumTargets; i++) {
      const struct PropertyShorthandTarget *field = &sh->Targets[i];
      struct Property *property = OBJ_FindLongProperty(o, field->PropertyID);
      unsigned component = (unsigned)field->Offset / sizeof(float);
      if (!property || PROP_GetType(property) != kDataTypeFloat || component >= 4) {
        Con_Error("Binding shorthand '%s' requires a numeric vector", target);
        return;
      }
      size_t size = strlen(program) + 32;
      char *expression = malloc(size);
      if (!expression) return;
      snprintf(expression, size, "%s(%s)", extract[component], program);
      PROP_SetBinding(property, expression, kBindingModeExpression, enabled);
      free(expression);
    }
    return;
  }
  struct PropertyType const *pd = propdesc(o, target);
  if (pd) PROP_SetBinding(OBJ_FindLongProperty(o, pd->FullIdentifier), program, mode, enabled);
  else Con_Error("Unknown binding target '%s'", target);
}

static bool_t
binding_node(struct Object *o, struct _xmlNode* x)
{
  char const *tag = (char const *)x->name;
  bool_t expressionNode = !strcmp(tag, "BindingExpression");
  if (strcmp(tag, "Binding") && !expressionNode) return FALSE;
  xmlChar *target = xmlGetProp(x, XMLSTR("Target"));
  if (!target) target = xmlGetProp(x, XMLSTR("target"));
  xmlChar *expression = xmlGetProp(x, XMLSTR("Expression"));
  if (!expression) expression = xmlNodeGetContent(x);
  xmlChar *modeText = xmlGetProp(x, XMLSTR("Mode"));
  xmlChar *enabledText = xmlGetProp(x, XMLSTR("Enabled"));
  enum BindingMode mode = expressionNode ? kBindingModeExpression : kBindingModeOneWay;
  bool_t enabled = !enabledText || (strcasecmp((char *)enabledText, "false") && strcmp((char *)enabledText, "0"));
  bool_t valid = !modeText || FS_BindingMode((char *)modeText, &mode);
  if (valid && target && expression && (*expression || !enabled)) {
    char *program = FS_BindingTrim((char *)expression);
    char normalized[MAX_PROPERTY_STRING];
    bool_t simplePath = *program != 0;
    for (char const *c = program; *c; c++)
      if (!isalnum((unsigned char)*c) && !strchr("_./#:", *c)) simplePath = FALSE;
    if (!expressionNode && simplePath) {
      int written = snprintf(normalized, sizeof(normalized), "{%s}", program);
      if (written >= sizeof(normalized)) valid = FALSE;
      program = normalized;
    }
    if (valid) binding_target(o, (char const *)target, program, mode, enabled);
  } else if (valid) Con_Error("<%s> requires Target and an expression", tag);
  if (target) xmlFree(target);
  if (expression) xmlFree(expression);
  if (modeText) xmlFree(modeText);
  if (enabledText) xmlFree(enabledText);
  return TRUE;
}

static void
visit_attr(struct Object *o, xmlAttrPtr a)
{
  xmlChar *value = xmlGetProp(a->parent, a->name);
  fixedString_t norm = {0};
  lpcString_t name = canonical_attr_name((lpcString_t)a->name, norm);
  struct PropertyType const *pd = propdesc(o, (lpcString_t)a->name);
  // Native message fields retain their declared casing (for example timestamp).
  if (pd) name = pd->Name;
  else pd = propdesc(o, name);

  if (!value) return;
  if (!special_attr(o, name, (lpcString_t)value)) {
    if (pd) set_text(o, pd, (lpcString_t)value);
    else if (set_shorthand_text(o, name, (lpcString_t)value)) {}
    else Con_Error("Unknown property '%s' for class '%s'", name, OBJ_GetClassName(o));
  }
  xmlFree(value);
}

/* Set a column attribute on a schema-loaded record object.
   Finds the property by short identifier (column name) and sets it from the
   string value, parsing to the declared type. */
static void
set_column_attr(struct Object *o, const char *col_name, const char *value,
                DSColumnType col_type)
{
  struct Property *p = OBJ_FindShortProperty(o, fnv1a32(col_name));
  if (!p) {
    Con_Error("Schema column '%s' not found on object of class '%s'",
              col_name, OBJ_GetClassName(o));
    return;
  }
  PROP_SetFlag(p, PF_DATA_FIELD);

  eDataType_t dt = DS_ColumnDataType(col_type);
  if (dt == kDataTypeString) {
    PROP_SetStringValue(p, value);
  } else {
    /* Re-use the generic string parser for int/float/bool */
    struct PropertyType const *pd = PROP_GetDesc(p);
    void *slot = (void *)PROP_GetRawValueSlot(p);
    if (pd && slot) parse_property(value, pd, slot);
  }
}

static bool_t
visit_child(struct Object *o, struct _xmlNode* c,
            const struct ds_schema *schema, const char *entity_name)
{
  if (binding_node(o, c)) return TRUE;

  lpcString_t tag = alias_class_tag((lpcString_t)c->name);
  struct PropertyType const *pd = !c->_private && OBJ_FindClass(tag)
    ? NULL
    : propdesc(o, (lpcString_t)c->name);
  if (pd) {
    return property_node(o, pd, c);
  }

  if (c->_private) {
    struct PropertyShorthand const *sh = OBJ_FindImplicitShorthand(o, (char const *)c->name);
    if (!sh) sh = OBJ_FindExplicitShorthand(o, (char const *)c->name);
    struct StructDesc const *sd = sh ? OBJ_FindStructDesc(sh->TypeString) : NULL;
    if (sd) {
      void *tmp = calloc(1, (size_t)sd->StructSize);
      struct PropertyType fake = { .Name = sh->Name, .TypeString = sh->TypeString };
      xmlNodePtr value = c->children;
      if (tmp && value && fill_struct(tmp, &fake, value)) {
        OBJ_SetShorthandValueFromStruct(o, (char const *)c->name, tmp);
        _ClearStructValue(sd, tmp);
      }
      free(tmp);
      return TRUE;
    }
  }

  xmlChar *text = xmlNodeGetContent(c);
  if (text && *text && set_shorthand_text(o, (lpcString_t)c->name, (lpcString_t)text)) {
    xmlFree(text);
    return TRUE;
  }
  if (text) xmlFree(text);

  /* For schema-loaded records, determine child entity from relation columns. */
  const char *child_entity = NULL;
  if (schema && entity_name) {
    const struct ds_entity *ent = DS_FindEntity(schema, entity_name);
    if (ent) {
      for (int i = 0; i < ent->num_columns; i++) {
        if (ent->columns[i].type == DS_COL_RELATION &&
            !strcasecmp(ent->columns[i].name, (const char *)c->name)) {
          child_entity = ent->columns[i].entity[0]
                         ? ent->columns[i].entity
                         : (const char *)c->name;
          break;
        }
      }
    }
  }

  struct Object *child = node(c, schema, child_entity);
  if (!child) return FALSE;
  OBJ_AddChild(o, child);
  OBJ_ReleaseRef(child);
  return TRUE;
}

static struct Object *
prefab(struct _xmlNode* x)
{
  xmlChar* tmpl = xmlGetProp(x, XMLSTR("PlaceholderTemplate"));
  struct Object *o = tmpl ? FS_LoadObject((char*)tmpl) : NULL;
  if (tmpl) xmlFree(tmpl);
  if (o) OBJ_SetFlags(o, OBJ_GetFlags(o) | OF_TEMPLATE);
  else Con_Error("PrefabPlaceholder missing PlaceholderTemplate attribute");
  return o;
}

static struct Object *
node(struct _xmlNode* x, const struct ds_schema *schema, const char *entity_name)
{
  lpcString_t raw_tag = (lpcString_t)x->name;

  /* Schema path: tag matches a registered entity class. */
  if (schema && entity_name) {
    const struct ds_entity *entity = DS_FindEntity(schema, entity_name);
    struct ClassDesc const *cls = entity ? OBJ_FindClass(entity_name) : NULL;
    if (entity) {
      bool_t is_record = cls && !strcasecmp(raw_tag, entity_name);
      struct Object *o = is_record ? OBJ_Create(cls->ClassID) : OBJ_Create(ID_DataObject);
      if (!o) return NULL;

      /* Set Name from the xml "name" or "Name" attribute. */
      xmlChar *xname = xmlGetProp(x, XMLSTR("Name"));
      if (!xname) xname = xmlGetProp(x, XMLSTR("name"));
      if (xname) { OBJ_SetName(o, (const char *)xname); xmlFree(xname); }
      else if (!is_record) OBJ_SetName(o, raw_tag);

      /* Apply each non-Name, non-relation attribute as a column value. */
      FOR_EACH_LIST(xmlAttr, a, x->properties) {
        const char *aname = (const char *)a->name;
        if (!strcasecmp(aname, "name")) continue;
        const struct ds_column *col = DS_FindColumn(schema, entity_name, aname);
        if (col && col->type != DS_COL_RELATION) {
          xmlChar *val = xmlGetProp(x, a->name);
          if (val) {
            set_column_attr(o, col->name, (const char *)val, col->type);
            xmlFree(val);
          }
        }
      }

      /* Recurse into child elements for relation columns. */
      xmlForEach(c, x) {
        if (c->type != XML_ELEMENT_NODE) continue;
        visit_child(o, c, schema, entity_name);
      }
      return o;
    }
  }

  /* Non-schema path: standard object loading. */
  lpcString_t tag = alias_class_tag(raw_tag);
  bool_t is_prefab = !strcmp(tag, "LayerPrefabPlaceholder") ||
                     !strcmp(tag, "ObjectPrefabPlaceholder") ||
                     !strcmp(tag, "LibraryPlaceholder");
  struct ClassDesc const *cls = is_prefab ? NULL : OBJ_FindClass(tag);
  struct Object *o = is_prefab ? prefab(x) : cls ? OBJ_Create(cls->ClassID) : NULL;

  if (!o) {
    if (!is_prefab) Con_Error("Unknown element type '%s' in '%s'", tag, x->doc && x->doc->URL ? (const char *)x->doc->URL : "?");
    return NULL;
  }

  if (!FS_GetSceneFormat()->Normalize(o, x)) {
    OBJ_ReleaseRef(o);
    return NULL;
  }

  // Controller loading precedes named handlers regardless of attribute order.
  FOR_EACH_LIST(xmlAttr, a, x->properties)
    if (!strcasecmp((char const *)a->name, "Controller")) visit_attr(o, a);
  FOR_EACH_LIST(xmlAttr, a, x->properties)
    if (strcasecmp((char const *)a->name, "Controller")) visit_attr(o, a);

  FOR_EACH_LIST(xmlNode, t, x->children) {
    if ((t->type == XML_TEXT_NODE || t->type == XML_CDATA_SECTION_NODE) && xmlStrlen(t->content) > 0) {
      lpcString_t text = (lpcString_t)t->content;
      OBJ_SetTextContent(o, text);
    }
  }

  xmlForEach(c, x) {
    if (!visit_child(o, c, NULL, NULL)) { OBJ_ReleaseRef(o); return NULL; }
  }
  return o;
}

static struct _xmlDoc *
read_doc(char const *xml, int len, lpcString_t name)
{
  struct _xmlDoc *doc = xmlReadMemory(xml, len, name, NULL, XML_FLAGS);
  return doc;
}

static struct _xmlDoc *
read_file(lpcString_t path)
{
  struct file *fp = FS_LoadFile(path);
  struct _xmlDoc *doc = fp ? read_doc((char const *)fp->data, fp->size, path) : NULL;
  if (fp) FS_FreeFile(fp);
  return doc;
}

/* Builds and starts the object for element x. source becomes its SourceFile:
   the path FS_LoadObject resolves back to this element. */
static struct Object *
build(struct _xmlNode *x, lpcString_t source, const struct ds_schema *schema)
{
  /* With a schema, a top-level tag naming an entity is that entity. */
  lpcString_t tag = (lpcString_t)x->name;
  struct Object *o = node(x, schema, schema && DS_FindEntity(schema, tag) ? tag : NULL);
  if (!o) return NULL;
  if (source) OBJ_SetSourceFile(o, source);
  OBJ_SendMessageW(o, ID_Object_Start, 0, NULL);
  return o;
}

/* XML data never dispatches scene classes, controllers, events or bindings. */
static struct Object *
data_node(xmlNodePtr x, const struct ds_schema *schema, const char *entity_name)
{
  const char *tag = (char const *)x->name;
  const struct ds_entity *entity = schema && entity_name ? DS_FindEntity(schema, entity_name) : NULL;
  bool_t record = entity && !strcasecmp(tag, entity_name);
  const char *type = alias_class_tag(tag);
  uint32_t class_id = ID_DataObject;
  if (record) {
    struct ClassDesc const *cls = OBJ_FindClass(entity_name);
    if (cls) class_id = cls->ClassID;
  } else if (!strcmp(type, "DataObjectString") || !strcmp(type, "DataObjectInt") ||
             !strcmp(type, "DataObjectFloat") || !strcmp(type, "DataObjectBool")) {
    struct ClassDesc const *cls = OBJ_FindClass(type);
    if (cls) class_id = cls->ClassID;
  }
  struct Object *o = OBJ_Create(class_id);
  if (!o) return NULL;
  xmlChar *name = xmlGetProp(x, XMLSTR("Name"));
  if (!name) name = xmlGetProp(x, XMLSTR("name"));
  OBJ_SetName(o, name ? (char const *)name : tag);
  if (name) xmlFree(name);
  FOR_EACH_LIST(xmlAttr, a, x->properties) {
    const char *key = (char const *)a->name;
    xmlChar *value = xmlGetProp(x, a->name);
    if (value && record) {
      const struct ds_column *column = DS_FindColumn(schema, entity_name, key);
      if (column && column->type != DS_COL_RELATION)
        set_column_attr(o, column->name, (char const *)value, column->type);
    } else if (value && !strcasecmp(key, "value")) {
      struct PropertyType const *pd = propdesc(o, "Value");
      char tmp[MAX_PROPERTY_STRING] = {0};
      if (pd && parse_property((char const *)value, pd, tmp)) {
        PROP_SetValue(OBJ_FindLongProperty(o, pd->FullIdentifier), tmp);
        if (pd->DataType == kDataTypeString) free(*(char **)tmp);
      }
    }
    if (value) xmlFree(value);
  }
  xmlForEach(c, x) {
    const char *child_entity = schema && DS_FindEntity(schema, (char const *)c->name)
      ? (char const *)c->name : entity_name;
    if (record) {
      const struct ds_column *column = DS_FindColumn(schema, entity_name, (char const *)c->name);
      if (column && column->type == DS_COL_RELATION)
        child_entity = column->entity[0] ? column->entity : column->name;
      else if (!DS_FindEntity(schema, (char const *)c->name)) child_entity = NULL;
    }
    struct Object *child = data_node(c, schema, child_entity);
    if (!child) { OBJ_ReleaseRef(o); return NULL; }
    OBJ_AddChild(o, child);
    OBJ_ReleaseRef(child);
  }
  return o;
}

static struct Object *
build_root(struct _xmlDoc *doc, lpcString_t source, const struct ds_schema *schema)
{
  if (!doc) return NULL;
  struct _xmlNode *root = xmlDocGetRootElement(doc);
  const char *entity = root && schema && DS_FindEntity(schema, (char const *)root->name)
    ? (char const *)root->name : NULL;
  struct Object *o = root ? data_node(root, schema, entity) : NULL;
  if (o) {
    if (source) OBJ_SetSourceFile(o, source);
    OBJ_SendMessageW(o, ID_Object_Start, 0, NULL);
  }
  xmlFreeDoc(doc);
  return o;
}

static struct Object *
load_file(lpcString_t path, const struct ds_schema *schema)
{
  struct Object *o = build_root(read_file(path), path, schema);
  if (!o) Con_Error("Failed to parse '%s'", path);
  return o;
}

struct Object *
FS_LoadXmlDataTree(lpcString_t path, const struct ds_schema *schema)
{
  return load_file(path, schema);
}

ORCA_API struct Object *
FS_LoadXmlDataStringWithSchema(lpcString_t xml_string, lpcString_t schema_xml)
{
  struct ds_schema *schema = NULL;
  if (schema_xml && *schema_xml) {
    /* Write schema XML to a temp file so libxml can parse it via the file API,
       or parse it inline via DS_ParseSchemaFromString if available.
       For now: parse schema from a temp string using xmlReadMemory directly. */
    schema = DS_ParseSchemaFromString(schema_xml);
  }
  struct Object *o = build_root(read_doc(xml_string, (int)strlen(xml_string), NULL), NULL, schema);
  if (!o) Con_Error("FS_LoadXmlDataStringWithSchema: failed to parse XML");
  /* Schema classes stay registered until DS_FreeSchema; caller owns the schema.
     For the in-process test use-case we leave classes registered until shutdown. */
  return o;
}

/* Build a scene from the Lisp reader's tree. The caller owns the tree. */
struct Object *
FS_BuildObjectTree(struct _xmlNode *root, lpcString_t source)
{
  return root ? build(root, source, NULL) : NULL;
}
