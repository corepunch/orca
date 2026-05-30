#include <source/filesystem/fs_local.h>
#include <source/core/core_local.h>
#include <core/core_properties.h>
#include "fs_xml_inline.h"

extern int parse_property(const char* str,
                          struct PropertyType const* prop,
                          void* valueptr);

static struct Object *node(struct _xmlNode* x);

static struct PropertyType const *
propdesc(struct Object *o, lpcString_t name)
{
  struct PropertyType const *pd = OBJ_FindImplicitProperty(o, name);
  if (pd) {
    return pd;
  }
  pd = OBJ_FindExplicitProperty(o, name);
  if (pd) {
    return pd;
  }

  struct SendMessageAction const *action = GetSendMessageAction(o);
  if (!action || !action->Message || !*action->Message) {
    return NULL;
  }

  uint32_t count = 0;
  struct PropertyType const *fields = OBJ_FindMessagePropertyTypes(action->Message, &count);
  if (!fields) {
    return NULL;
  }

  FOR_LOOP(i, count) {
    struct PropertyType const *field = &fields[i];
    if (!strcmp(field->Name, name)) {
      return field;
    }
    if (field->Category && *field->Category) {
      fixedString_t full_name = {0};
      snprintf(full_name, sizeof(full_name), "%s.%s", field->Category, field->Name);
      if (!strcmp(full_name, name)) {
        return field;
      }
    }
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
  if (!strcmp(name, "class")) {
    _SendMessage(o, StyleController, AddClasses, .ClassNames = value);
    return TRUE;
  }
  return FALSE;
}

static bool_t
binding_attr(struct Object *o, struct PropertyType const *pd, lpcString_t value)
{
  size_t n = strlen(value);
  fixedString_t expr = {0};
  if (n <= 10 || strncmp(value, "{Binding ", 9) || value[n - 1] != '}') return FALSE;
  snprintf(expr, sizeof(expr), "{%.*s}", (int)(n - 10), value + 9);
  return OBJ_AttachPropertyProgram(o, pd->Name, expr, kBindingModeExpression, TRUE);
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
  }
  items[old_count] = item;

  struct { void *items; int count; } value = { items, count };
  PROP_SetValue(p, &value);
  return TRUE;
}

static bool_t
inline_event(struct Object *o, struct PropertyType const *pd, lpcString_t value)
{
  struct Property *triggers = NULL;
  struct PropertyType const *td = OBJ_FindImplicitProperty(o, "Triggers");
  struct Object *action = _LoadObjectFromXmlFragment(value, 0);
  struct Object *trigger = action ? OBJ_Create(ID_EventTrigger) : NULL;
  char routed[MAX_PROPERTY_STRING] = {0};

  if (!td || FAILED(OBJ_FindLongProperty(o, td->FullIdentifier, &triggers)) || !trigger) {
    if (action) OBJ_ReleaseRef(action);
    Con_Error("Property '%s' does not support inline trigger shorthand", pd->Name);
    return FALSE;
  }
  snprintf(routed, sizeof(routed),
           (pd->Category && *pd->Category) ? "%s.%s" : "%s",
           (pd->Category && *pd->Category) ? pd->Category : pd->Name,
           pd->Name);
  lpcString_t routed_value = routed;
  if (FAILED(OBJ_SetPropertyValue(trigger, "RoutedEvent", &routed_value))) {
    OBJ_ReleaseRef(trigger);
    OBJ_ReleaseRef(action);
    return FALSE;
  }

  OBJ_AddChild(trigger, action);
  if (append_object(triggers, trigger)) return TRUE;
  OBJ_ReleaseRef(trigger);
  return FALSE;
}

static bool_t
inline_value(struct Object *o, struct PropertyType const *pd,
             struct Property *p, lpcString_t value)
{
  if (pd->DataType == kDataTypeEvent) return inline_event(o, pd, value);
  if (pd->DataType == kDataTypeStruct) return _LoadStructFromXmlFragment(p, pd, value);

  struct Object *child = _LoadObjectFromXmlFragment(value, 0);
  if (!child) return FALSE;
  if (GetBinding(child) || GetBindingExpression(child)) {
    bool_t ok = (bool_t)OBJ_SendMessageW(child, ID_Binding_Compile, 0, p);
    OBJ_ReleaseRef(child);
    return ok;
  }
  if (pd->DataType == kDataTypeObject) {
    PROP_SetValue(p, &child);
    return TRUE;
  }

  OBJ_ReleaseRef(child);
  Con_Error("Unsupported inline object for property '%s'", pd->Name);
  return FALSE;
}

static void
set_text(struct Object *o, struct PropertyType const *pd, lpcString_t value)
{
  struct Property *p = NULL;
  char tmp[MAX_PROPERTY_STRING] = {0};
  if (!value || !*value) return;
  if (FAILED(OBJ_FindLongProperty(o, pd->FullIdentifier, &p))) {
    Con_Error("Could not get property slot for '%s'", pd->Name);
    return;
  }
  if (binding_attr(o, pd, value)) return;

  switch (value[0]) {
    case '$':
      value = FS_GetThemeValue(value);
      if (!value) Con_Error("Could not resolve theme value for property '%s'", pd->Name);
      break;
    case '@': {
      struct Object *ref = OBJ_FindByPath(o, value + 1);
      if (ref && pd->DataType == kDataTypeObject) PROP_SetValue(p, &ref);
      else Con_Error("Could not resolve object reference '%s' for property '%s'", value + 1, pd->Name);
      return;
    }
    case '{':
    case '<':
      inline_value(o, pd, p, value);
      return;
  }
  if (!value || !parse_property(value, pd, tmp)) return;
  PROP_SetValue(p, tmp);
  if (pd->DataType == kDataTypeString) free(*(char**)tmp);
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

static void
array_prop(struct Object *o, struct PropertyType const *pd, struct _xmlNode* x)
{
  struct Property *p = NULL;
  struct StructDesc const *sd = pd->DataType == kDataTypeStruct ? OBJ_FindStructDesc(pd->TypeString) : NULL;
  int count = 0;
  int index = 0;

  if (FAILED(OBJ_FindLongProperty(o, pd->FullIdentifier, &p))) {
    Con_Error("Could not get property slot for '%s'", pd->Name);
    return;
  }
  if (pd->DataType != kDataTypeObject && pd->DataType != kDataTypeStruct) {
    Con_Error("array property '%s' not supported in XML loader", pd->Name);
    return;
  }
  if (pd->DataType == kDataTypeStruct && !sd) {
    Con_Error("Could not resolve struct descriptor '%s' for array property '%s'", pd->TypeString, pd->Name);
    return;
  }

  xmlForEach(c, x) count++;
  void *items = count ? calloc((size_t)count, (size_t)pd->DataSize) : NULL;
  if (count && !items) return;

  xmlForEach(c, x) {
    if (pd->DataType == kDataTypeObject) {
      struct Object *child = node(c);
      if (child) ((struct Object **)items)[index++] = child;
    } else {
      void *dst = (char *)items + (size_t)index * pd->DataSize;
      if (fill_struct(dst, pd, c)) index++;
    }
  }

  struct { void *items; int count; } value = { items, index };
  PROP_SetValue(p, &value);
}

static void
property_node(struct Object *o, struct PropertyType const *pd, struct _xmlNode* x)
{
  struct Property *p = NULL;
  if (pd->IsArray) {
    array_prop(o, pd, x);
    return;
  }
  if (FAILED(OBJ_FindLongProperty(o, pd->FullIdentifier, &p))) {
    Con_Error("Could not get property slot for '%s'", pd->Name);
    return;
  }

  if (pd->DataType == kDataTypeStruct && x->properties) {
    set_struct_node(p, pd, x);
    return;
  }

  xmlChar *text = xmlNodeGetContent(x);
  if (text && *text && (!x->children || x->children->type == XML_TEXT_NODE)) {
    set_text(o, pd, (lpcString_t)text);
    xmlFree(text);
    return;
  }
  if (text) xmlFree(text);

  xmlForEach(c, x) {
    if (pd->DataType == kDataTypeStruct) {
      set_struct_node(p, pd, c);
      return;
    }
    struct Object *child = node(c);
    if (!child) return;
    if (GetBinding(child) || GetBindingExpression(child)) {
      OBJ_SendMessageW(child, ID_Binding_Compile, 0, p);
      OBJ_ReleaseRef(child);
    } else if (pd->DataType == kDataTypeObject) {
      PROP_SetValue(p, &child);
    } else {
      OBJ_ReleaseRef(child);
      Con_Error("Property '%s' does not accept object child", pd->Name);
    }
    return;
  }
}

static bool_t
binding_node(struct Object *o, struct _xmlNode* x)
{
  lpcString_t tag = (lpcString_t)x->name;
  if (strcmp(tag, "Binding") && strcmp(tag, "BindingExpression")) {
    return FALSE;
  }

  xmlChar *target = xmlGetProp(x, XMLSTR("Target"));
  xmlChar *text = xmlNodeGetContent(x);
  if (target && text && *text) {
    OBJ_AttachPropertyProgram(o, (lpcString_t)target, (lpcString_t)text,
                              kBindingModeExpression, TRUE);
  } else {
    Con_Error("<%s> requires Target and text content", tag);
  }
  if (target) xmlFree(target);
  if (text) xmlFree(text);
  return TRUE;
}

static void
visit_attr(struct Object *o, xmlAttrPtr a)
{
  xmlChar *value = xmlGetProp(a->parent, a->name);
  lpcString_t name = (lpcString_t)a->name;
  struct PropertyType const *pd = propdesc(o, name);

  if (!value) return;
  if (!special_attr(o, name, (lpcString_t)value)) {
    if (pd) set_text(o, pd, (lpcString_t)value);
    else Con_Error("Unknown property '%s' for class '%s'", name, OBJ_GetClassName(o));
  }
  xmlFree(value);
}

static void
visit_child(struct Object *o, struct _xmlNode* c)
{
  if (binding_node(o, c)) return;

  struct PropertyType const *pd = OBJ_FindExplicitProperty(o, (lpcString_t)c->name);
  if (pd) {
    property_node(o, pd, c);
    return;
  }

  struct Object *child = node(c);
  if (child) OBJ_AddChild(o, child);
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
node(struct _xmlNode* x)
{
  lpcString_t tag = (lpcString_t)x->name;
  bool_t is_prefab = !strcmp(tag, "LayerPrefabPlaceholder") ||
                     !strcmp(tag, "ObjectPrefabPlaceholder") ||
                     !strcmp(tag, "LibraryPlaceholder");
  struct ClassDesc const *cls = is_prefab ? NULL : OBJ_FindClass(tag);
  struct Object *o = is_prefab ? prefab(x) : cls ? OBJ_Create(cls->ClassID) : NULL;

  if (!o) {
    if (!is_prefab) Con_Error("Unknown element type '%s'", tag);
    return NULL;
  }

  if (GetSendMessageAction(o)) {
    FOR_EACH_LIST(xmlAttr, a, x->properties) {
      if (!strcmp((lpcString_t)a->name, "Message")) {
        visit_attr(o, a);
      }
    }
  }

  FOR_EACH_LIST(xmlAttr, a, x->properties) {
    if (GetSendMessageAction(o) && !strcmp((lpcString_t)a->name, "Message")) {
      continue;
    }
    visit_attr(o, a);
  }

  FOR_EACH_LIST(xmlNode, t, x->children) {
    if (t->type == XML_TEXT_NODE && xmlStrlen(t->content) > 0) {
      OBJ_SetTextContent(o, (lpcString_t)t->content);
      if (!is_prefab) OBJ_SendMessageW(o, ID_Object_Start, 0, NULL);
      return o;
    }
  }

  xmlForEach(c, x) visit_child(o, c);
  if (!is_prefab) OBJ_SendMessageW(o, ID_Object_Start, 0, NULL);
  return o;
}

static struct Object *
load_doc(char const *xml, int len, lpcString_t name)
{
  char *expanded = _ExpandXmlPositionalArgs(xml);
  struct _xmlDoc *doc = xmlReadMemory(expanded ? expanded : xml,
                              expanded ? (int)strlen(expanded) : len,
                              name, NULL, XML_FLAGS);
  free(expanded);
  if (!doc) return NULL;

  struct _xmlNode* root = xmlDocGetRootElement(doc);
  struct Object *o = root ? node(root) : NULL;
  xmlFreeDoc(doc);
  return o;
}

struct Object *
FS_LoadObjectFromXml(lpcString_t path)
{
  struct file* fp = FS_LoadFile(path);
  struct Object *o = fp ? load_doc((char const *)fp->data, fp->size, path) : NULL;
  if (fp) FS_FreeFile(fp);
  if (o) OBJ_SetSourceFile(o, path);
  else Con_Error("Failed to parse '%s'", path);
  return o;
}

ORCA_API struct Object *
FS_LoadObjectFromXmlString(lpcString_t xml_string)
{
  struct Object *o = load_doc(xml_string, (int)strlen(xml_string), NULL);
  if (!o) Con_Error("FS_LoadObjectFromXmlString: failed to parse XML string");
  return o;
}
