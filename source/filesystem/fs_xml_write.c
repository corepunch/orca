#include <source/filesystem/fs_local.h>
#include <source/core/core_local.h>
#include <core/core_properties.h>

#include <libxml/tree.h>

/* -------------------------------------------------------------------------
 * String buffer
 * ---------------------------------------------------------------------- */

struct wbuf {
  char *s;
  size_t n;
  size_t cap;
};

static void
wbuf_grow(struct wbuf *b, size_t need)
{
  if (b->cap > need) return;
  b->cap = b->cap ? b->cap * 2 : 256;
  while (b->cap <= need) b->cap *= 2;
  b->s = realloc(b->s, b->cap);
}

static void
wbuf_put(struct wbuf *b, char const *s)
{
  size_t n = strlen(s);
  wbuf_grow(b, b->n + n + 1);
  memcpy(b->s + b->n, s, n);
  b->n += n;
  b->s[b->n] = 0;
}

static void
wbuf_putf(struct wbuf *b, char const *fmt, ...)
{
  char tmp[512];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(tmp, sizeof(tmp), fmt, ap);
  va_end(ap);
  wbuf_put(b, tmp);
}

/* -------------------------------------------------------------------------
 * Property value → string
 * ---------------------------------------------------------------------- */

static bool_t format_value(struct PropertyType const *pd,
                            void const *value,
                            struct wbuf *out);

static bool_t
format_struct_value(struct PropertyType const *pd,
                    void const *value,
                    struct wbuf *out)
{
  struct StructDesc const *sd = pd->TypeString
    ? OBJ_FindStructDesc(pd->TypeString) : NULL;
  if (!sd) {
    /* Fallback: treat as raw float array */
    size_t count = pd->DataSize / sizeof(float);
    float const *f = (float const *)value;
    for (size_t i = 0; i < count; i++) {
      if (i) wbuf_put(out, " ");
      wbuf_putf(out, "%g", f[i]);
    }
    return TRUE;
  }
  bool_t first = TRUE;
  FOR_LOOP(i, (int)sd->NumProperties) {
    struct PropertyType const *field = &sd->Properties[i];
    void const *fval = (char const *)value + field->Offset;
    struct wbuf fb = {0};
    if (format_value(field, fval, &fb) && fb.s && *fb.s) {
      if (!first) wbuf_put(out, " ");
      wbuf_put(out, fb.s);
      first = FALSE;
    }
    free(fb.s);
  }
  return !first;
}

static bool_t
format_value(struct PropertyType const *pd,
             void const *value,
             struct wbuf *out)
{
  if (!value) return FALSE;
  switch (pd->DataType) {
    case kDataTypeBool:
      wbuf_put(out, *(int const *)value ? "true" : "false");
      return TRUE;
    case kDataTypeInt:
      wbuf_putf(out, "%d", *(int const *)value);
      return TRUE;
    case kDataTypeEnum: {
      lpcString_t const *ev = pd->EnumValues;
      int idx = *(int const *)value;
      if (ev && ev[idx]) { wbuf_put(out, ev[idx]); return TRUE; }
      wbuf_putf(out, "%d", idx);
      return TRUE;
    }
    case kDataTypeFloat:
      wbuf_putf(out, "%g", *(float const *)value);
      return TRUE;
    case kDataTypeColor: {
      float const *f = (float const *)value;
      uint8_t r = (uint8_t)(f[0] * 255.0f + 0.5f);
      uint8_t g = (uint8_t)(f[1] * 255.0f + 0.5f);
      uint8_t b = (uint8_t)(f[2] * 255.0f + 0.5f);
      uint8_t a = (uint8_t)(f[3] * 255.0f + 0.5f);
      if (a < 255)
        wbuf_putf(out, "#%02x%02x%02x%02x", a, r, g, b);
      else
        wbuf_putf(out, "#%02x%02x%02x", r, g, b);
      return TRUE;
    }
    case kDataTypeString: {
      lpcString_t str = *(lpcString_t const *)value;
      if (str) wbuf_put(out, str);
      return TRUE;
    }
    case kDataTypeStruct:
      return format_struct_value(pd, value, out);
    case kDataTypeObject:
    case kDataTypeEvent:
    default:
      return FALSE;
  }
}

/* -------------------------------------------------------------------------
 * Callback context
 * ---------------------------------------------------------------------- */

struct serialize_ctx {
  xmlNodePtr node;
};

/* -------------------------------------------------------------------------
 * Serialize bindings
 *
 * NOTE: The compiled Binding struct (property_internal.h) stores only the
 * compiled token tree.  The original expression string is not retained
 * after PROP_SetBinding compiles it.  We can only emit the Target name;
 * the expression text is irrecoverable without storing it at parse time.
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
    xmlNodePtr bnd = xmlNewChild(node, NULL, XMLSTR("BindingExpression"), NULL);
    xmlSetProp(bnd, XMLSTR("Target"), XMLSTR(target));
  }
}

/* -------------------------------------------------------------------------
 * Serialize array property
 * ---------------------------------------------------------------------- */
static void serialize_object_impl(struct Object const *object, xmlNodePtr parent);

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

  /* Use Category.Name when bare Name collides with a registered class name */
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
    char const *item_tag = sd ? sd->StructName : (pd->TypeString ? pd->TypeString : "Item");
    FOR_LOOP(i, count) {
      char const *item = (char const *)items + (size_t)i * pd->DataSize;
      xmlNodePtr snode = xmlNewChild(arr_node, NULL, XMLSTR(item_tag), NULL);
      if (!sd) continue;
      FOR_LOOP(j, (int)sd->NumProperties) {
        struct PropertyType const *field = &sd->Properties[j];
        void const *fval = item + field->Offset;
        struct wbuf fb = {0};
        if (format_value(field, fval, &fb) && fb.s && *fb.s)
          xmlSetProp(snode, XMLSTR(field->Name), XMLSTR(fb.s));
        free(fb.s);
      }
    }
  }
}

/* -------------------------------------------------------------------------
 * Per-property callback
 * ---------------------------------------------------------------------- */
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
  if (pd->Name[0] == '_') return; /* heuristic: skip internal fields */

  struct Property *prop = NULL;
  if (FAILED(OBJ_FindLongProperty((struct Object *)object,
                                  pd->FullIdentifier, &prop)))
    return;
  if (!prop || PROP_IsNull(prop)) return;
  if (PROP_HasProgram(prop)) return; /* serialised as BindingExpression */

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

  struct wbuf vb = {0};
  if (format_value(pd, value, &vb) && vb.s && *vb.s)
    xmlSetProp(node, XMLSTR(pd->Name), XMLSTR(vb.s));
  free(vb.s);
}

/* -------------------------------------------------------------------------
 * Serialize object tree
 * ---------------------------------------------------------------------- */
static void
serialize_object_impl(struct Object const *object, xmlNodePtr parent)
{
  lpcString_t class_name = OBJ_GetClassName((struct Object *)object);
  if (!class_name || !strcmp(class_name, "(none)")) return;

  xmlNodePtr node = xmlNewChild(parent, NULL, XMLSTR(class_name), NULL);

  lpcString_t name = OBJ_GetName((struct Object *)object);
  if (name && *name)
    xmlSetProp(node, XMLSTR("Name"), XMLSTR(name));

  /* NOTE: StyleClass (CSS class attribute) is not serializable via the
   * public API.  The raw class string is not stored as a normal property;
   * it lives inside the StyleController component's internal linked list.
   * OBJ_EnumStyleClasses (core_local.h) outputs parsed selector structs,
   * not the original string.  Serialization of StyleClass requires either
   * a new OBJ_GetRawStyleClasses() accessor or storing the class string
   * as a normal string property. */

  struct serialize_ctx ctx = { node };
  OBJ_EnumClassProperties((struct Object *)object, serialize_property_cb, &ctx);

  /* Binding stubs (expression text is not recoverable post-compile) */
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

  /* Temporary wrapper root so xmlDocSetRootElement works cleanly */
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
