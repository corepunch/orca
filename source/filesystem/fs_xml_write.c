#include <source/filesystem/fs_local.h>
#include <source/core/core_local.h>
#include <core/core_properties.h>

#include <libxml/tree.h>

/* -------------------------------------------------------------------------
 * Forward declarations
 * ---------------------------------------------------------------------- */
static void serialize_object_impl(struct Object const *object, xmlNodePtr parent);

/* -------------------------------------------------------------------------
 * Emit binding expressions as child elements
 *
 * PROP_GetBindingExpression now returns the original source text (stored
 * since the fix in property_events.c).  For older properties that were
 * set before the fix, the expression will be NULL — we still emit the
 * Target stub so the property name is preserved.
 * ---------------------------------------------------------------------- */
static void
serialize_bindings(struct Object const *object, xmlNodePtr node)
{
  FOR_EACH_PROPERTY(p, OBJ_GetProperties((struct Object *)object)) {
    if (!PROP_HasProgram((struct Property *)p)) continue;
    struct PropertyType const *pd = PROP_GetDesc(p);
    if (!pd) continue;

    char target[256];
    if (pd->Category && *pd->Category)
      snprintf(target, sizeof(target), "%s.%s", pd->Category, pd->Name);
    else
      snprintf(target, sizeof(target), "%s", pd->Name);

    lpcString_t expr = PROP_GetBindingExpression(p);
    xmlNodePtr bnd = xmlNewChild(node, NULL, XMLSTR("BindingExpression"),
                                  expr ? XMLSTR(expr) : NULL);
    xmlSetProp(bnd, XMLSTR("Target"), XMLSTR(target));
  }
}

/* -------------------------------------------------------------------------
 * Serialize array property as a child element
 * ---------------------------------------------------------------------- */
static void
serialize_array_property(struct Object const *object,
                         struct PropertyType const *pd,
                         struct Property const *prop,
                         xmlNodePtr parent)
{
  void const *raw = PROP_GetValue(prop);
  if (!raw) return;
  void *items = *(void **)raw;
  int count   = ((int const *)raw)[sizeof(void *) / sizeof(int)];
  if (!items || count <= 0) return;

  /* Use Category.Name when the bare Name matches a registered class name */
  char elem_name[256];
  if (OBJ_FindClass(pd->Name) && pd->Category && *pd->Category)
    snprintf(elem_name, sizeof(elem_name), "%s.%s", pd->Category, pd->Name);
  else
    snprintf(elem_name, sizeof(elem_name), "%s", pd->Name);

  xmlNodePtr arr_node = xmlNewChild(parent, NULL, XMLSTR(elem_name), NULL);

  if (pd->DataType == kDataTypeObject) {
    struct Object **objs = (struct Object **)items;
    FOR_LOOP(i, count) {
      if (objs[i]) serialize_object_impl(objs[i], arr_node);
    }
  } else if (pd->DataType == kDataTypeStruct) {
    struct StructDesc const *sd = pd->TypeString
      ? OBJ_FindStructDesc(pd->TypeString) : NULL;
    char const *item_tag = sd ? sd->StructName
                             : (pd->TypeString ? pd->TypeString : "Item");
    FOR_LOOP(i, count) {
      char const *item = (char const *)items + (size_t)i * pd->DataSize;
      xmlNodePtr snode = xmlNewChild(arr_node, NULL, XMLSTR(item_tag), NULL);
      if (!sd) continue;
      FOR_LOOP(j, (int)sd->NumProperties) {
        struct PropertyType const *field = &sd->Properties[j];
        if (field->IsReadOnly) continue;
        void const *fval = item + field->Offset;
        char tmp[MAX_PROPERTY_STRING] = {0};
        if (OBJ_FormatPropertyValue(field, fval, tmp, sizeof(tmp)) && tmp[0])
          xmlSetProp(snode, XMLSTR(field->Name), XMLSTR(tmp));
      }
    }
  }
}

/* -------------------------------------------------------------------------
 * Per-property callback (called by OBJ_EnumClassProperties)
 * ---------------------------------------------------------------------- */
struct serialize_ctx {
  xmlNodePtr node;
};

static void
serialize_property_cb(struct Object const *object,
                      struct PropertyType const *pd,
                      struct ClassDesc const *cdesc,
                      void const *component_data,
                      void *param)
{
  (void)cdesc;
  (void)component_data;
  struct serialize_ctx *ctx = (struct serialize_ctx *)param;
  xmlNodePtr node = ctx->node;

  if (!pd->Name || !*pd->Name) return;
  if (pd->IsReadOnly) return;   /* Issue 5: skip runtime-computed properties */
  if (pd->Name[0] == '_') return;

  struct Property *prop = NULL;
  if (FAILED(OBJ_FindLongProperty((struct Object *)object,
                                  pd->FullIdentifier, &prop)))
    return;
  if (!prop || PROP_IsNull(prop)) return;
  if (PROP_HasProgram(prop)) return; /* serialised as <BindingExpression> */

  if (pd->IsArray) {
    serialize_array_property(object, pd, prop, node);
    return;
  }

  void const *value = PROP_GetValue(prop);
  if (!value) return;

  if (pd->DataType == kDataTypeObject) {
    struct Object *child = *(struct Object *const *)value;
    if (!child) return;
    char elem_name[256];
    if (OBJ_FindClass(pd->Name) && pd->Category && *pd->Category)
      snprintf(elem_name, sizeof(elem_name), "%s.%s", pd->Category, pd->Name);
    else
      snprintf(elem_name, sizeof(elem_name), "%s", pd->Name);
    xmlNodePtr pnode = xmlNewChild(node, NULL, XMLSTR(elem_name), NULL);
    serialize_object_impl(child, pnode);
    return;
  }

  if (pd->DataType == kDataTypeEvent) return;

  /* Scalar / struct → attribute using the public formatter (Issue 1) */
  char tmp[MAX_PROPERTY_STRING] = {0};
  if (OBJ_FormatPropertyValue(pd, value, tmp, sizeof(tmp)) && tmp[0])
    xmlSetProp(node, XMLSTR(pd->Name), XMLSTR(tmp));
}

/* -------------------------------------------------------------------------
 * Serialize object tree
 * ---------------------------------------------------------------------- */
static void
serialize_object_impl(struct Object const *object, xmlNodePtr parent)
{
  /* Issue 4: Prefab/template objects — emit placeholder, not expanded tree.
   * OF_TEMPLATE is set on objects loaded via LayerPrefabPlaceholder /
   * ObjectPrefabPlaceholder.  SourceFile holds the prefab path. */
  uint32_t flags = OBJ_GetFlags((struct Object *)object);
  if (flags & OF_TEMPLATE) {
    lpcString_t src = OBJ_GetSourceFile((struct Object *)object);
    if (src && *src) {
      xmlNodePtr ph = xmlNewChild(parent, NULL,
                                   XMLSTR("LayerPrefabPlaceholder"), NULL);
      xmlSetProp(ph, XMLSTR("PlaceholderTemplate"), XMLSTR(src));

      /* Emit non-default attributes from the expanded object so the
       * placeholder can be re-inflated with the same overrides. */
      lpcString_t name = OBJ_GetName((struct Object *)object);
      if (name && *name)
        xmlSetProp(ph, XMLSTR("Name"), XMLSTR(name));

      struct serialize_ctx ctx = { ph };
      OBJ_EnumClassProperties((struct Object *)object,
                              serialize_property_cb, &ctx);
      return;
    }
  }

  lpcString_t class_name = OBJ_GetClassName((struct Object *)object);
  if (!class_name || !strcmp(class_name, "(none)")) return;

  xmlNodePtr node = xmlNewChild(parent, NULL, XMLSTR(class_name), NULL);

  lpcString_t name = OBJ_GetName((struct Object *)object);
  if (name && *name)
    xmlSetProp(node, XMLSTR("Name"), XMLSTR(name));

  /* Issue 3: StyleClass — now readable via OBJ_GetRawStyleClasses */
  char class_buf[1024] = {0};
  if (OBJ_GetRawStyleClasses((struct Object *)object, class_buf, sizeof(class_buf)))
    xmlSetProp(node, XMLSTR("class"), XMLSTR(class_buf));

  struct serialize_ctx ctx = { node };
  OBJ_EnumClassProperties((struct Object *)object, serialize_property_cb, &ctx);

  /* Issue 2: Binding expressions — expression text is now preserved */
  serialize_bindings(object, node);

  lpcString_t text = OBJ_GetTextContent((struct Object *)object);
  if (text && *text) {
    xmlNodeSetContent(node, XMLSTR(text));
    return;
  }

  FOR_EACH_OBJECT(child, (struct Object *)object) {
    serialize_object_impl(child, node);
  }
}

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

LPSTR
FS_SerializeObjectToXmlString(struct Object const *object)
{
  if (!object) return NULL;

  xmlDocPtr doc = xmlNewDoc(XMLSTR("1.0"));
  if (!doc) return NULL;

  xmlNodePtr wrapper = xmlNewNode(NULL, XMLSTR("_"));
  xmlDocSetRootElement(doc, wrapper);

  serialize_object_impl(object, wrapper);

  xmlNodePtr obj_node = wrapper->children;
  if (!obj_node) {
    xmlFreeDoc(doc);
    return NULL;
  }

  xmlUnlinkNode(obj_node);
  xmlDocSetRootElement(doc, obj_node);
  xmlFreeNode(wrapper);

  xmlChar *xml_buf = NULL;
  int xml_len = 0;
  xmlDocDumpFormatMemory(doc, &xml_buf, &xml_len, 1);
  xmlFreeDoc(doc);

  if (!xml_buf) return NULL;
  LPSTR result = strdup((char const *)xml_buf);
  xmlFree(xml_buf);
  return result;
}
