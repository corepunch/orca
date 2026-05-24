/*
 * fs_xml.c – SAX-based XML object loader with visitor pattern
 *
 * Uses libxml2's SAX1 API (xmlSAXUserParseMemory) to process XML events
 * without building a full DOM tree.  Parse state is managed via a stack of
 * xml_visitor objects — one per open element.
 *
 * Visitor interface:
 *   start   – called when a child element opens; returns the visitor to push
 *   end     – called when this element closes; visitor finalises its work
 *   text    – called for each character-data chunk inside this element
 *   result  – returns the Object produced by this visitor (may be NULL)
 *   child   – called after a child visitor has finished, passing its result
 *   destroy – frees visitor resources
 */
#include <ctype.h>
#include <source/filesystem/fs_local.h>
#include <source/core/core_local.h>
#include <source/core/core_properties.h>
#include "fs_xml_inline.h"

extern int parse_property(const char* str,
                          struct PropertyType const* prop,
                          void* valueptr);

/* ── visitor interface ────────────────────────────────────────────────────── */

typedef struct xml_visitor xml_visitor_t;

struct xml_visitor {
  xml_visitor_t *(*start)(xml_visitor_t *self, lpcString_t tag,
                          lpcString_t const *names, lpcString_t const *values,
                          int nattrs);
  void           (*end)    (xml_visitor_t *self);
  void           (*text)   (xml_visitor_t *self, lpcString_t chars, int len);
  struct Object *(*result) (xml_visitor_t *self);
  void           (*child)  (xml_visitor_t *self, struct Object *obj);
  void           (*destroy)(xml_visitor_t *self);
};

/* ── parse stack ──────────────────────────────────────────────────────────── */

#define XML_MAX_DEPTH 64
#define XML_MAX_ATTRS 64

struct sax_ctx {
  xml_visitor_t *stack[XML_MAX_DEPTH];
  int            depth;
  struct Object *result;
};

/* ── forward declarations ─────────────────────────────────────────────────── */

static xml_visitor_t *make_skip_visitor(void);
static xml_visitor_t *make_object_visitor(struct Object *obj, bool_t is_prefab);
static xml_visitor_t *make_prop_visitor(struct Object *parent,
                                        struct PropertyType const *pd);
static xml_visitor_t *visitor_for_element(lpcString_t tag,
                                          lpcString_t const *names,
                                          lpcString_t const *values,
                                          int nattrs);

/* ── shared helpers (unchanged logic, new SAX-friendly signatures) ─────────── */

static struct PropertyType const *
propdesc(struct Object *o, lpcString_t name)
{
  struct PropertyType const *pd = OBJ_FindImplicitProperty(o, name);
  return pd ? pd : OBJ_FindExplicitProperty(o, name);
}

static lpcString_t
attr_value(lpcString_t const *names, lpcString_t const *values,
           int n, lpcString_t key)
{
  for (int i = 0; i < n; i++) {
    if (!strcmp(names[i], key)) return values[i];
  }
  return NULL;
}

static bool_t
is_blank(lpcString_t s, size_t n)
{
  for (size_t i = 0; i < n; i++) {
    if (!isspace((unsigned char)s[i])) return FALSE;
  }
  return TRUE;
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
  if (old_items && old_count > 0)
    memcpy(items, old_items, (size_t)old_count * sizeof(struct Object *));
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
  struct Object *action  = _LoadObjectFromXmlFragment(value, 0);
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

  OBJ_AddChild(trigger, action, FALSE);
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
    bool_t ok = OBJ_SendMessageW(child, ID_Binding_Compile, 0, p);
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
      else Con_Error("Could not resolve object reference '%s' for property '%s'",
                     value + 1, pd->Name);
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

static void
apply_attrs(struct Object *o,
            lpcString_t const *names, lpcString_t const *values, int n)
{
  for (int i = 0; i < n; i++) {
    if (!special_attr(o, names[i], values[i])) {
      struct PropertyType const *pd = propdesc(o, names[i]);
      if (pd) set_text(o, pd, values[i]);
      else Con_Error("Unknown property '%s' for class '%s'",
                     names[i], OBJ_GetClassName(o));
    }
  }
}

/* Fill a struct value from SAX-style name/value attribute arrays. */
static bool_t
fill_struct_from_attrs(void *dst, struct PropertyType const *pd,
                       lpcString_t const *names, lpcString_t const *values,
                       int n)
{
  struct StructDesc const *sd = OBJ_FindStructDesc(pd->TypeString);
  bool_t ok = sd && dst;
  if (!ok) {
    Con_Error("Could not prepare struct '%s' for property '%s'",
              pd->TypeString, pd->Name);
    return FALSE;
  }
  for (int i = 0; i < n && ok; i++) {
    struct PropertyType const *fd = _FindStructField(sd, names[i]);
    if (!fd || !_SetStructFieldFromString(fd, (char *)dst + fd->Offset, values[i])) {
      ok = FALSE;
      Con_Error("Could not set field '%s' on struct '%s'", names[i], sd->StructName);
    }
  }
  if (!ok) _ClearStructValue(sd, dst);
  return ok;
}

/* ── skip_visitor ─────────────────────────────────────────────────────────── */

static xml_visitor_t *
skip_start(xml_visitor_t *self, lpcString_t tag,
           lpcString_t const *names, lpcString_t const *values, int n)
{
  (void)self; (void)tag; (void)names; (void)values; (void)n;
  return make_skip_visitor();
}

static void skip_end    (xml_visitor_t *self) { (void)self; }
static void skip_text   (xml_visitor_t *self, lpcString_t c, int l)
                        { (void)self; (void)c; (void)l; }
static void skip_destroy(xml_visitor_t *self) { free(self); }

static xml_visitor_t *
make_skip_visitor(void)
{
  xml_visitor_t *v = calloc(1, sizeof(*v));
  if (!v) return NULL;
  v->start   = skip_start;
  v->end     = skip_end;
  v->text    = skip_text;
  v->destroy = skip_destroy;
  return v;
}

/* ── binding_visitor ──────────────────────────────────────────────────────── */
/* Handles <Binding Target="…">expr</Binding> / <BindingExpression …>. */

struct binding_visitor {
  xml_visitor_t base;
  struct Object *parent;
  char          *target;
  char          *text;
  size_t         text_len;
};

static xml_visitor_t *
bv_start(xml_visitor_t *self, lpcString_t tag,
         lpcString_t const *names, lpcString_t const *values, int n)
{
  (void)self; (void)tag; (void)names; (void)values; (void)n;
  return make_skip_visitor();
}

static void
bv_text(xml_visitor_t *self, lpcString_t chars, int len)
{
  struct binding_visitor *bv = (struct binding_visitor*)self;
  size_t new_len = bv->text_len + (size_t)len;
  char *buf = realloc(bv->text, new_len + 1);
  if (!buf) return;
  memcpy(buf + bv->text_len, chars, (size_t)len);
  buf[new_len] = '\0';
  bv->text     = buf;
  bv->text_len = new_len;
}

static void
bv_end(xml_visitor_t *self)
{
  struct binding_visitor *bv = (struct binding_visitor*)self;
  if (bv->parent && bv->target && bv->text && *bv->text) {
    OBJ_AttachPropertyProgram(bv->parent, bv->target, bv->text,
                              kBindingModeExpression, TRUE);
  } else {
    Con_Error("<Binding>/<BindingExpression> requires Target attribute and text content");
  }
}

static void
bv_destroy(xml_visitor_t *self)
{
  struct binding_visitor *bv = (struct binding_visitor*)self;
  free(bv->target);
  free(bv->text);
  free(bv);
}

static xml_visitor_t *
make_binding_visitor(struct Object *parent, lpcString_t target)
{
  struct binding_visitor *bv = calloc(1, sizeof(*bv));
  if (!bv) return make_skip_visitor();
  bv->base.start   = bv_start;
  bv->base.end     = bv_end;
  bv->base.text    = bv_text;
  bv->base.destroy = bv_destroy;
  bv->parent = parent;
  bv->target = target ? strdup(target) : NULL;
  return &bv->base;
}

/* ── prop_visitor ─────────────────────────────────────────────────────────── */
/* Handles property-element subtrees: <Grid.Columns>, <Node.Triggers>, etc. */

struct prop_visitor {
  xml_visitor_t           base;
  struct Object          *parent;
  struct PropertyType const *pd;
  struct Property        *p;
  bool_t                  has_child;
  /* accumulated text content */
  char                   *text;
  size_t                  text_len;
  /* array accumulator for DataType == kDataTypeObject */
  struct Object         **obj_items;
  int                     obj_count;
  int                     obj_cap;
  /* array accumulator for DataType == kDataTypeStruct */
  void                   *struct_items;
  int                     struct_count;
  int                     struct_cap;
};

static xml_visitor_t *
pv_start(xml_visitor_t *self, lpcString_t tag,
         lpcString_t const *names, lpcString_t const *values, int n)
{
  struct prop_visitor *pv = (struct prop_visitor*)self;
  struct PropertyType const *pd = pv->pd;
  pv->has_child = TRUE;

  if (pd->IsArray) {
    if (pd->DataType == kDataTypeObject) {
      /* Each child element becomes an object appended to the array. */
      return visitor_for_element(tag, names, values, n);
    }
    if (pd->DataType == kDataTypeStruct) {
      /* Each child element provides struct fields via its attributes. */
      struct StructDesc const *sd = OBJ_FindStructDesc(pd->TypeString);
      if (!sd) {
        Con_Error("Could not resolve struct '%s' for array property '%s'",
                  pd->TypeString, pd->Name);
        return make_skip_visitor();
      }
      if (pv->struct_count >= pv->struct_cap) {
        int cap = pv->struct_cap ? pv->struct_cap * 2 : 4;
        void *buf = realloc(pv->struct_items, (size_t)cap * (size_t)pd->DataSize);
        if (!buf) return make_skip_visitor();
        pv->struct_items = buf;
        pv->struct_cap   = cap;
      }
      void *dst = (char*)pv->struct_items
                  + (size_t)pv->struct_count * (size_t)pd->DataSize;
      memset(dst, 0, (size_t)pd->DataSize);
      if (fill_struct_from_attrs(dst, pd, names, values, n))
        pv->struct_count++;
      return make_skip_visitor();
    }
    Con_Error("array property '%s' not supported type in XML loader", pd->Name);
    return make_skip_visitor();
  }

  /* Non-array struct: child element provides struct fields via its attrs. */
  if (pd->DataType == kDataTypeStruct) {
    struct StructDesc const *sd = OBJ_FindStructDesc(pd->TypeString);
    void *tmp = sd ? calloc(1, (size_t)sd->StructSize) : NULL;
    if (fill_struct_from_attrs(tmp, pd, names, values, n))
      PROP_SetValue(pv->p, tmp);
    free(tmp);
    return make_skip_visitor();
  }

  /* Non-array object (or binding) property: create child object. */
  return visitor_for_element(tag, names, values, n);
}

static void
pv_text(xml_visitor_t *self, lpcString_t chars, int len)
{
  struct prop_visitor *pv = (struct prop_visitor*)self;
  if (pv->has_child) return;
  size_t new_len = pv->text_len + (size_t)len;
  char *buf = realloc(pv->text, new_len + 1);
  if (!buf) return;
  memcpy(buf + pv->text_len, chars, (size_t)len);
  buf[new_len] = '\0';
  pv->text     = buf;
  pv->text_len = new_len;
}

static void
pv_end(xml_visitor_t *self)
{
  struct prop_visitor *pv = (struct prop_visitor*)self;
  struct PropertyType const *pd = pv->pd;

  if (pd->IsArray) {
    if (pd->DataType == kDataTypeObject && pv->obj_count > 0) {
      struct { void *items; int count; } val = { pv->obj_items, pv->obj_count };
      PROP_SetValue(pv->p, &val);
      pv->obj_items = NULL;   /* ownership transferred */
    } else if (pd->DataType == kDataTypeStruct && pv->struct_count > 0) {
      struct { void *items; int count; } val = { pv->struct_items, pv->struct_count };
      PROP_SetValue(pv->p, &val);
      pv->struct_items = NULL; /* ownership transferred */
    }
    return;
  }

  /* Non-array: apply accumulated text if no child element was received. */
  if (!pv->has_child && pv->text && pv->text_len > 0
      && !is_blank(pv->text, pv->text_len)) {
    set_text(pv->parent, pd, pv->text);
  }
}

static void
pv_child(xml_visitor_t *self, struct Object *obj)
{
  struct prop_visitor *pv = (struct prop_visitor*)self;
  if (!obj) return;
  struct PropertyType const *pd = pv->pd;

  if (pd->IsArray && pd->DataType == kDataTypeObject) {
    if (pv->obj_count >= pv->obj_cap) {
      int cap = pv->obj_cap ? pv->obj_cap * 2 : 4;
      struct Object **buf = realloc(pv->obj_items, (size_t)cap * sizeof(*buf));
      if (!buf) { OBJ_ReleaseRef(obj); return; }
      pv->obj_items = buf;
      pv->obj_cap   = cap;
    }
    pv->obj_items[pv->obj_count++] = obj;
    return;
  }

  /* Non-array: binding compile or direct set. */
  if (GetBinding(obj) || GetBindingExpression(obj)) {
    OBJ_SendMessageW(obj, ID_Binding_Compile, 0, pv->p);
    OBJ_ReleaseRef(obj);
  } else if (pd->DataType == kDataTypeObject) {
    PROP_SetValue(pv->p, &obj);
  } else {
    OBJ_ReleaseRef(obj);
    Con_Error("Property '%s' does not accept an object child", pd->Name);
  }
}

static void
pv_destroy(xml_visitor_t *self)
{
  struct prop_visitor *pv = (struct prop_visitor*)self;
  free(pv->text);
  free(pv->obj_items);
  free(pv->struct_items);
  free(pv);
}

static xml_visitor_t *
make_prop_visitor(struct Object *parent, struct PropertyType const *pd)
{
  struct prop_visitor *pv = calloc(1, sizeof(*pv));
  if (!pv) return make_skip_visitor();
  pv->base.start   = pv_start;
  pv->base.end     = pv_end;
  pv->base.text    = pv_text;
  pv->base.child   = pv_child;
  pv->base.destroy = pv_destroy;
  pv->parent = parent;
  pv->pd     = pd;
  if (FAILED(OBJ_FindLongProperty(parent, pd->FullIdentifier, &pv->p))) {
    Con_Error("Could not get property slot for '%s'", pd->Name);
    free(pv);
    return make_skip_visitor();
  }
  return &pv->base;
}

/* ── object_visitor ───────────────────────────────────────────────────────── */
/* Builds a single Object from an XML element and its subtree. */

struct object_visitor {
  xml_visitor_t  base;
  struct Object *obj;
  bool_t         is_prefab;
  bool_t         has_children;
  char          *text;
  size_t         text_len;
};

static xml_visitor_t *
ov_start(xml_visitor_t *self, lpcString_t tag,
         lpcString_t const *names, lpcString_t const *values, int n)
{
  struct object_visitor *ov = (struct object_visitor*)self;
  ov->has_children = TRUE;

  /* Direct binding child — attach to parent object. */
  if (!strcmp(tag, "Binding") || !strcmp(tag, "BindingExpression")) {
    lpcString_t target = attr_value(names, values, n, "Target");
    return make_binding_visitor(ov->obj, target);
  }

  /* Property element (e.g. <Grid.Columns>, <Node.Triggers>). */
  struct PropertyType const *pd = OBJ_FindExplicitProperty(ov->obj, tag);
  if (pd) {
    /* Struct property whose fields are given as attributes on the element. */
    if (pd->DataType == kDataTypeStruct && n > 0) {
      struct Property *p = NULL;
      if (SUCCEEDED(OBJ_FindLongProperty(ov->obj, pd->FullIdentifier, &p))) {
        struct StructDesc const *sd = OBJ_FindStructDesc(pd->TypeString);
        void *tmp = sd ? calloc(1, (size_t)sd->StructSize) : NULL;
        if (fill_struct_from_attrs(tmp, pd, names, values, n))
          PROP_SetValue(p, tmp);
        free(tmp);
      }
      return make_skip_visitor();
    }
    return make_prop_visitor(ov->obj, pd);
  }

  /* Regular child object. */
  return visitor_for_element(tag, names, values, n);
}

static void
ov_text(xml_visitor_t *self, lpcString_t chars, int len)
{
  struct object_visitor *ov = (struct object_visitor*)self;
  if (ov->has_children) return;   /* ignore text after/between elements */
  size_t new_len = ov->text_len + (size_t)len;
  char *buf = realloc(ov->text, new_len + 1);
  if (!buf) return;
  memcpy(buf + ov->text_len, chars, (size_t)len);
  buf[new_len] = '\0';
  ov->text     = buf;
  ov->text_len = new_len;
}

static void
ov_end(xml_visitor_t *self)
{
  struct object_visitor *ov = (struct object_visitor*)self;
  if (!ov->has_children && ov->text && ov->text_len > 0
      && !is_blank(ov->text, ov->text_len)) {
    OBJ_SetTextContent(ov->obj, ov->text);
  }
  if (!ov->is_prefab)
    OBJ_SendMessageW(ov->obj, ID_Object_Start, 0, NULL);
}

static struct Object *
ov_result(xml_visitor_t *self)
{
  return ((struct object_visitor*)self)->obj;
}

static void
ov_child(xml_visitor_t *self, struct Object *obj)
{
  if (obj)
    OBJ_AddChild(((struct object_visitor*)self)->obj, obj, FALSE);
}

static void
ov_destroy(xml_visitor_t *self)
{
  struct object_visitor *ov = (struct object_visitor*)self;
  free(ov->text);
  free(ov);
  /* obj is owned by the object tree, not by us */
}

static xml_visitor_t *
make_object_visitor(struct Object *obj, bool_t is_prefab)
{
  struct object_visitor *ov = calloc(1, sizeof(*ov));
  if (!ov) return make_skip_visitor();
  ov->base.start   = ov_start;
  ov->base.end     = ov_end;
  ov->base.text    = ov_text;
  ov->base.result  = ov_result;
  ov->base.child   = ov_child;
  ov->base.destroy = ov_destroy;
  ov->obj       = obj;
  ov->is_prefab = is_prefab;
  return &ov->base;
}

/* Create the visitor appropriate for an element tag + attributes. */
static xml_visitor_t *
visitor_for_element(lpcString_t tag,
                    lpcString_t const *names, lpcString_t const *values,
                    int nattrs)
{
  bool_t is_prefab = !strcmp(tag, "LayerPrefabPlaceholder") ||
                     !strcmp(tag, "ObjectPrefabPlaceholder") ||
                     !strcmp(tag, "LibraryPlaceholder");

  if (is_prefab) {
    lpcString_t tmpl = attr_value(names, values, nattrs, "PlaceholderTemplate");
    struct Object *o = tmpl ? FS_LoadObject(tmpl) : NULL;
    if (o) OBJ_SetFlags(o, OBJ_GetFlags(o) | OF_TEMPLATE);
    else Con_Error("PrefabPlaceholder missing PlaceholderTemplate attribute");
    return o ? make_object_visitor(o, TRUE) : make_skip_visitor();
  }

  struct ClassDesc const *cls = OBJ_FindClass(tag);
  if (!cls) {
    Con_Error("Unknown element type '%s'", tag);
    return make_skip_visitor();
  }

  struct Object *o = OBJ_Create(cls->ClassID);
  if (!o) return make_skip_visitor();
  apply_attrs(o, names, values, nattrs);
  return make_object_visitor(o, FALSE);
}

/* ── SAX handler callbacks ────────────────────────────────────────────────── */

static void
sax_start(void *user_data, const xmlChar *name, const xmlChar **atts)
{
  struct sax_ctx *ctx = (struct sax_ctx*)user_data;
  if (ctx->depth >= XML_MAX_DEPTH) {
    Con_Error("XML nesting exceeds maximum depth of %d at element '<%s>'",
              XML_MAX_DEPTH, name);
    return;
  }

  lpcString_t tag = (lpcString_t)name;

  /* Unpack SAX1 alternating [name, value, …, NULL] attribute array. */
  lpcString_t names_buf [XML_MAX_ATTRS];
  lpcString_t values_buf[XML_MAX_ATTRS];
  int n = 0;
  if (atts) {
    while (atts[n * 2] && n < XML_MAX_ATTRS) {
      names_buf [n] = (lpcString_t)atts[n * 2];
      values_buf[n] = (lpcString_t)atts[n * 2 + 1];
      n++;
    }
    if (atts[n * 2]) {
      Con_Error("Element '<%s>' has more than %d attributes; extras ignored",
                tag, XML_MAX_ATTRS);
    }
  }

  xml_visitor_t *child;
  if (ctx->depth == 0) {
    child = visitor_for_element(tag, names_buf, values_buf, n);
  } else {
    xml_visitor_t *parent = ctx->stack[ctx->depth - 1];
    child = parent->start(parent, tag, names_buf, values_buf, n);
  }

  ctx->stack[ctx->depth++] = child ? child : make_skip_visitor();
}

static void
sax_end(void *user_data, const xmlChar *name)
{
  (void)name;
  struct sax_ctx *ctx = (struct sax_ctx*)user_data;
  if (ctx->depth <= 0) return;

  xml_visitor_t *v = ctx->stack[--ctx->depth];
  v->end(v);

  struct Object *res = v->result ? v->result(v) : NULL;

  if (ctx->depth > 0) {
    xml_visitor_t *parent = ctx->stack[ctx->depth - 1];
    if (parent->child) parent->child(parent, res);
  } else {
    ctx->result = res;
  }

  v->destroy(v);
}

static void
sax_chars(void *user_data, const xmlChar *ch, int len)
{
  struct sax_ctx *ctx = (struct sax_ctx*)user_data;
  if (ctx->depth <= 0) return;
  xml_visitor_t *v = ctx->stack[ctx->depth - 1];
  if (v->text) v->text(v, (lpcString_t)ch, len);
}

/* ── load entry point ─────────────────────────────────────────────────────── */

static struct Object *
load_doc(char const *xml, int len, lpcString_t name)
{
  (void)name;
  char *expanded = _ExpandXmlPositionalArgs(xml);
  lpcString_t src = expanded ? expanded : xml;
  int          sz  = expanded ? (int)strlen(expanded) : len;

  static xmlSAXHandler handler = {
    .startElement        = sax_start,
    .endElement          = sax_end,
    .characters          = sax_chars,
    .ignorableWhitespace = sax_chars,
    .initialized         = 1,
  };

  struct sax_ctx ctx = {0};
  xmlSAXUserParseMemory(&handler, &ctx, src, sz);
  free(expanded);
  return ctx.result;
}

struct Object *
FS_LoadObjectFromXml(lpcString_t path)
{
  struct file *fp = FS_LoadFile(path);
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
