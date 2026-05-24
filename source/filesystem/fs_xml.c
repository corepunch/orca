#include <ctype.h>
#include <source/filesystem/fs_local.h>
#include <source/core/core_local.h>
#include <source/core/core_properties.h>
#include "fs_xml_inline.h"

extern int parse_property(const char*, struct PropertyType const*, void*);

typedef struct xml_visitor xml_visitor_t;
struct xml_visitor {
  xml_visitor_t *(*start)(xml_visitor_t*, lpcString_t, lpcString_t const*, lpcString_t const*, int);
  void           (*end)    (xml_visitor_t*);
  void           (*text)   (xml_visitor_t*, lpcString_t, int);
  struct Object *(*result) (xml_visitor_t*);
  void           (*child)  (xml_visitor_t*, struct Object*);
  void           (*destroy)(xml_visitor_t*);
};

#define XML_MAX_DEPTH 64
#define XML_MAX_ATTRS 64

struct sax_ctx { xml_visitor_t *stack[XML_MAX_DEPTH]; int depth; struct Object *result; };

static xml_visitor_t *make_skip_visitor(void);
static xml_visitor_t *make_binding_visitor(struct Object*, lpcString_t);
static xml_visitor_t *make_prop_visitor(struct Object*, struct PropertyType const*);
static xml_visitor_t *visitor_for_element(lpcString_t, lpcString_t const*, lpcString_t const*, int);

/* helpers */

static struct PropertyType const *
propdesc(struct Object *o, lpcString_t n)
{ struct PropertyType const *p = OBJ_FindImplicitProperty(o, n); return p ? p : OBJ_FindExplicitProperty(o, n); }

static lpcString_t
attr_value(lpcString_t const *names, lpcString_t const *values, int n, lpcString_t key)
{ for (int i = 0; i < n; i++) if (!strcmp(names[i], key)) return values[i]; return NULL; }

static bool_t
is_blank(lpcString_t s, size_t n)
{ for (size_t i = 0; i < n; i++) if (!isspace((unsigned char)s[i])) return FALSE; return TRUE; }

static bool_t
special_attr(struct Object *o, lpcString_t name, lpcString_t value)
{
  if (!strcmp(name, "ClassName") || !strcmp(name, "PlaceholderTemplate")) return TRUE;
  if (!strcmp(name, "Name") || !strcmp(name, "id")) { OBJ_SetName(o, value); return TRUE; }
  if (!strcmp(name, "class")) { _SendMessage(o, StyleController, AddClasses, .ClassNames = value); return TRUE; }
  return FALSE;
}

static bool_t
binding_attr(struct Object *o, struct PropertyType const *pd, lpcString_t v)
{
  size_t n = strlen(v); fixedString_t expr = {0};
  if (n <= 10 || strncmp(v, "{Binding ", 9) || v[n-1] != '}') return FALSE;
  snprintf(expr, sizeof(expr), "{%.*s}", (int)(n-10), v+9);
  return OBJ_AttachPropertyProgram(o, pd->Name, expr, kBindingModeExpression, TRUE);
}

static bool_t
append_object(struct Property *p, struct Object *item)
{
  struct PropertyType const *pd = PROP_GetDesc(p);
  void *old = NULL; int old_n = 0;
  if (!pd || !pd->IsArray || pd->DataType != kDataTypeObject) return FALSE;
  if (PROP_GetValue(p)) {
    memcpy(&old, PROP_GetValue(p), sizeof(old));
    memcpy(&old_n, (char*)PROP_GetValue(p)+sizeof(old), sizeof(old_n));
  }
  struct Object **arr = calloc((size_t)(old_n+1), sizeof(*arr));
  if (!arr) return FALSE;
  if (old && old_n) memcpy(arr, old, (size_t)old_n * sizeof(*arr));
  arr[old_n] = item;
  struct { void *p; int n; } v = { arr, old_n+1 };
  PROP_SetValue(p, &v);
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
  snprintf(routed, sizeof(routed), (pd->Category && *pd->Category) ? "%s.%s" : "%s",
           (pd->Category && *pd->Category) ? pd->Category : pd->Name, pd->Name);
  lpcString_t rv = routed;
  if (FAILED(OBJ_SetPropertyValue(trigger, "RoutedEvent", &rv))) {
    OBJ_ReleaseRef(trigger); OBJ_ReleaseRef(action); return FALSE;
  }
  OBJ_AddChild(trigger, action, FALSE);
  if (append_object(triggers, trigger)) return TRUE;
  OBJ_ReleaseRef(trigger); return FALSE;
}

static bool_t
inline_value(struct Object *o, struct PropertyType const *pd, struct Property *p, lpcString_t v)
{
  switch (pd->DataType) {
    case kDataTypeEvent:  return inline_event(o, pd, v);
    case kDataTypeStruct: return _LoadStructFromXmlFragment(p, pd, v);
    default: break;
  }
  struct Object *child = _LoadObjectFromXmlFragment(v, 0);
  if (!child) return FALSE;
  if (GetBinding(child) || GetBindingExpression(child)) {
    bool_t ok = OBJ_SendMessageW(child, ID_Binding_Compile, 0, p);
    OBJ_ReleaseRef(child); return ok;
  }
  switch (pd->DataType) {
    case kDataTypeObject: PROP_SetValue(p, &child); return TRUE;
    default: OBJ_ReleaseRef(child); Con_Error("Unsupported inline object for property '%s'", pd->Name); return FALSE;
  }
}

static void
set_text(struct Object *o, struct PropertyType const *pd, lpcString_t value)
{
  struct Property *p = NULL; char tmp[MAX_PROPERTY_STRING] = {0};
  if (!value || !*value) return;
  if (FAILED(OBJ_FindLongProperty(o, pd->FullIdentifier, &p))) {
    Con_Error("Could not get property slot for '%s'", pd->Name); return;
  }
  if (binding_attr(o, pd, value)) return;
  switch (value[0]) {
    case '$':
      value = FS_GetThemeValue(value);
      if (!value) { Con_Error("Could not resolve theme value for property '%s'", pd->Name); return; }
      break;
    case '@': {
      struct Object *ref = OBJ_FindByPath(o, value+1);
      if (ref && pd->DataType == kDataTypeObject) PROP_SetValue(p, &ref);
      else Con_Error("Could not resolve object reference '%s' for property '%s'", value+1, pd->Name);
      return;
    }
    case '{': case '<': inline_value(o, pd, p, value); return;
  }
  if (parse_property(value, pd, tmp)) {
    PROP_SetValue(p, tmp);
    if (pd->DataType == kDataTypeString) free(*(char**)tmp);
  }
}

static void
apply_attrs(struct Object *o, lpcString_t const *names, lpcString_t const *values, int n)
{
  for (int i = 0; i < n; i++) {
    if (!special_attr(o, names[i], values[i])) {
      struct PropertyType const *pd = propdesc(o, names[i]);
      if (pd) set_text(o, pd, values[i]);
      else Con_Error("Unknown property '%s' for class '%s'", names[i], OBJ_GetClassName(o));
    }
  }
}

static bool_t
fill_struct_from_attrs(void *dst, struct PropertyType const *pd,
                       lpcString_t const *names, lpcString_t const *values, int n)
{
  struct StructDesc const *sd = OBJ_FindStructDesc(pd->TypeString);
  if (!sd || !dst) { Con_Error("Could not prepare struct '%s' for '%s'", pd->TypeString, pd->Name); return FALSE; }
  for (int i = 0; i < n; i++) {
    struct PropertyType const *fd = _FindStructField(sd, names[i]);
    if (!fd || !_SetStructFieldFromString(fd, (char*)dst+fd->Offset, values[i])) {
      _ClearStructValue(sd, dst);
      Con_Error("Could not set field '%s' on struct '%s'", names[i], sd->StructName);
      return FALSE;
    }
  }
  return TRUE;
}

/* skip */

static xml_visitor_t *
skip_start(xml_visitor_t *self, lpcString_t tag, lpcString_t const *ns, lpcString_t const *vs, int n)
{ (void)self; (void)tag; (void)ns; (void)vs; (void)n; return make_skip_visitor(); }

static void skip_destroy(xml_visitor_t *v) { free(v); }

static xml_visitor_t *
make_skip_visitor(void)
{
  xml_visitor_t *v = calloc(1, sizeof(*v));
  if (v) *v = (xml_visitor_t){ .start = skip_start, .destroy = skip_destroy };
  return v;
}

/* binding */

struct bv { xml_visitor_t base; struct Object *parent; char *target, *text; size_t tlen; };

static void bv_text(xml_visitor_t *self, lpcString_t ch, int len) {
  struct bv *bv = (struct bv*)self;
  char *buf = realloc(bv->text, bv->tlen + (size_t)len + 1);
  if (!buf) return;
  memcpy(buf + bv->tlen, ch, (size_t)len);
  buf[(bv->tlen += (size_t)len)] = '\0';
  bv->text = buf;
}

static void bv_end(xml_visitor_t *self) {
  struct bv *bv = (struct bv*)self;
  if (bv->parent && bv->target && bv->text && *bv->text)
    OBJ_AttachPropertyProgram(bv->parent, bv->target, bv->text, kBindingModeExpression, TRUE);
  else
    Con_Error("<Binding>/<BindingExpression> requires Target and text content");
}

static void bv_destroy(xml_visitor_t *v) { struct bv *bv=(struct bv*)v; free(bv->target); free(bv->text); free(bv); }

static xml_visitor_t *
make_binding_visitor(struct Object *parent, lpcString_t target)
{
  struct bv *bv = calloc(1, sizeof(*bv));
  if (!bv) return make_skip_visitor();
  bv->base = (xml_visitor_t){ .start = skip_start, .end = bv_end, .text = bv_text, .destroy = bv_destroy };
  bv->parent = parent;
  bv->target = target ? strdup(target) : NULL;
  return &bv->base;
}

/* prop */

struct pv {
  xml_visitor_t base;
  struct Object *parent;
  struct PropertyType const *pd;
  struct Property *p;
  bool_t has_child;
  char *text; size_t tlen;
  struct Object **objs; int obj_n, obj_c;
  void *structs; int struct_n, struct_c;
};

static xml_visitor_t *
pv_start(xml_visitor_t *self, lpcString_t tag, lpcString_t const *ns, lpcString_t const *vs, int n)
{
  struct pv *pv = (struct pv*)self;
  pv->has_child = TRUE;
  if (pv->pd->IsArray) {
    switch (pv->pd->DataType) {
      case kDataTypeObject:
        return visitor_for_element(tag, ns, vs, n);
      case kDataTypeStruct: {
        struct StructDesc const *sd = OBJ_FindStructDesc(pv->pd->TypeString);
        if (!sd) { Con_Error("Could not resolve struct '%s' for '%s'", pv->pd->TypeString, pv->pd->Name); return make_skip_visitor(); }
        if (pv->struct_n >= pv->struct_c) {
          int cap = pv->struct_c ? pv->struct_c*2 : 4;
          void *buf = realloc(pv->structs, (size_t)cap * (size_t)pv->pd->DataSize);
          if (!buf) return make_skip_visitor();
          pv->structs = buf; pv->struct_c = cap;
        }
        void *dst = (char*)pv->structs + (size_t)pv->struct_n * (size_t)pv->pd->DataSize;
        memset(dst, 0, (size_t)pv->pd->DataSize);
        if (fill_struct_from_attrs(dst, pv->pd, ns, vs, n)) pv->struct_n++;
        return make_skip_visitor();
      }
      default:
        Con_Error("array property '%s' type not supported in XML loader", pv->pd->Name);
        return make_skip_visitor();
    }
  }
  switch (pv->pd->DataType) {
    case kDataTypeStruct: {
      struct StructDesc const *sd = OBJ_FindStructDesc(pv->pd->TypeString);
      void *tmp = sd ? calloc(1, (size_t)sd->StructSize) : NULL;
      if (fill_struct_from_attrs(tmp, pv->pd, ns, vs, n)) PROP_SetValue(pv->p, tmp);
      free(tmp);
      return make_skip_visitor();
    }
    default: return visitor_for_element(tag, ns, vs, n);
  }
}

static void pv_text(xml_visitor_t *self, lpcString_t ch, int len) {
  struct pv *pv = (struct pv*)self;
  if (pv->has_child) return;
  char *buf = realloc(pv->text, pv->tlen + (size_t)len + 1);
  if (!buf) return;
  memcpy(buf + pv->tlen, ch, (size_t)len);
  buf[(pv->tlen += (size_t)len)] = '\0';
  pv->text = buf;
}

static void pv_end(xml_visitor_t *self) {
  struct pv *pv = (struct pv*)self;
  if (pv->pd->IsArray) {
    switch (pv->pd->DataType) {
      case kDataTypeObject:
        if (pv->obj_n) { struct { void *p; int n; } v = {pv->objs, pv->obj_n}; PROP_SetValue(pv->p, &v); pv->objs = NULL; }
        break;
      case kDataTypeStruct:
        if (pv->struct_n) { struct { void *p; int n; } v = {pv->structs, pv->struct_n}; PROP_SetValue(pv->p, &v); pv->structs = NULL; }
        break;
      default: break;
    }
    return;
  }
  if (!pv->has_child && pv->text && pv->tlen && !is_blank(pv->text, pv->tlen))
    set_text(pv->parent, pv->pd, pv->text);
}

static void pv_child(xml_visitor_t *self, struct Object *obj) {
  struct pv *pv = (struct pv*)self;
  if (!obj) return;
  if (pv->pd->IsArray && pv->pd->DataType == kDataTypeObject) {
    if (pv->obj_n >= pv->obj_c) {
      int cap = pv->obj_c ? pv->obj_c*2 : 4;
      struct Object **buf = realloc(pv->objs, (size_t)cap * sizeof(*buf));
      if (!buf) { OBJ_ReleaseRef(obj); return; }
      pv->objs = buf; pv->obj_c = cap;
    }
    pv->objs[pv->obj_n++] = obj; return;
  }
  if (GetBinding(obj) || GetBindingExpression(obj)) {
    OBJ_SendMessageW(obj, ID_Binding_Compile, 0, pv->p); OBJ_ReleaseRef(obj); return;
  }
  switch (pv->pd->DataType) {
    case kDataTypeObject: PROP_SetValue(pv->p, &obj); break;
    default: OBJ_ReleaseRef(obj); Con_Error("Property '%s' does not accept an object child", pv->pd->Name);
  }
}

static void pv_destroy(xml_visitor_t *v) { struct pv *pv=(struct pv*)v; free(pv->text); free(pv->objs); free(pv->structs); free(pv); }

static xml_visitor_t *
make_prop_visitor(struct Object *parent, struct PropertyType const *pd)
{
  struct pv *pv = calloc(1, sizeof(*pv));
  if (!pv) return make_skip_visitor();
  pv->base = (xml_visitor_t){ .start=pv_start, .end=pv_end, .text=pv_text, .child=pv_child, .destroy=pv_destroy };
  pv->parent = parent; pv->pd = pd;
  if (FAILED(OBJ_FindLongProperty(parent, pd->FullIdentifier, &pv->p))) {
    Con_Error("Could not get property slot for '%s'", pd->Name); free(pv); return make_skip_visitor();
  }
  return &pv->base;
}

/* object */

struct ov { xml_visitor_t base; struct Object *obj; bool_t is_prefab, has_ch; char *text; size_t tlen; };

static xml_visitor_t *
ov_start(xml_visitor_t *self, lpcString_t tag, lpcString_t const *ns, lpcString_t const *vs, int n)
{
  struct ov *ov = (struct ov*)self;
  ov->has_ch = TRUE;
  if (!strcmp(tag, "Binding") || !strcmp(tag, "BindingExpression"))
    return make_binding_visitor(ov->obj, attr_value(ns, vs, n, "Target"));
  struct PropertyType const *pd = OBJ_FindExplicitProperty(ov->obj, tag);
  if (pd) {
    switch (pd->DataType) {
      case kDataTypeStruct: if (n > 0) {
        struct Property *p = NULL;
        if (SUCCEEDED(OBJ_FindLongProperty(ov->obj, pd->FullIdentifier, &p))) {
          struct StructDesc const *sd = OBJ_FindStructDesc(pd->TypeString);
          void *tmp = sd ? calloc(1, (size_t)sd->StructSize) : NULL;
          if (fill_struct_from_attrs(tmp, pd, ns, vs, n)) PROP_SetValue(p, tmp);
          free(tmp);
        }
        return make_skip_visitor();
      } break;
      default: break;
    }
    return make_prop_visitor(ov->obj, pd);
  }
  return visitor_for_element(tag, ns, vs, n);
}

static void ov_text(xml_visitor_t *self, lpcString_t ch, int len) {
  struct ov *ov = (struct ov*)self;
  if (ov->has_ch) return;
  char *buf = realloc(ov->text, ov->tlen + (size_t)len + 1);
  if (!buf) return;
  memcpy(buf + ov->tlen, ch, (size_t)len);
  buf[(ov->tlen += (size_t)len)] = '\0';
  ov->text = buf;
}

static void ov_end(xml_visitor_t *self) {
  struct ov *ov = (struct ov*)self;
  if (!ov->has_ch && ov->text && ov->tlen && !is_blank(ov->text, ov->tlen))
    OBJ_SetTextContent(ov->obj, ov->text);
  if (!ov->is_prefab) OBJ_SendMessageW(ov->obj, ID_Object_Start, 0, NULL);
}

static struct Object *ov_result(xml_visitor_t *self) { return ((struct ov*)self)->obj; }
static void ov_child(xml_visitor_t *self, struct Object *obj) { if (obj) OBJ_AddChild(((struct ov*)self)->obj, obj, FALSE); }
static void ov_destroy(xml_visitor_t *v) { struct ov *ov=(struct ov*)v; free(ov->text); free(ov); }

static xml_visitor_t *
make_object_visitor(struct Object *obj, bool_t is_prefab)
{
  struct ov *ov = calloc(1, sizeof(*ov));
  if (!ov) return make_skip_visitor();
  ov->base = (xml_visitor_t){ .start=ov_start, .end=ov_end, .text=ov_text, .result=ov_result, .child=ov_child, .destroy=ov_destroy };
  ov->obj = obj; ov->is_prefab = is_prefab;
  return &ov->base;
}

static xml_visitor_t *
visitor_for_element(lpcString_t tag, lpcString_t const *ns, lpcString_t const *vs, int n)
{
  bool_t is_prefab = !strcmp(tag,"LayerPrefabPlaceholder") || !strcmp(tag,"ObjectPrefabPlaceholder") || !strcmp(tag,"LibraryPlaceholder");
  if (is_prefab) {
    lpcString_t tmpl = attr_value(ns, vs, n, "PlaceholderTemplate");
    struct Object *o = tmpl ? FS_LoadObject(tmpl) : NULL;
    if (o) OBJ_SetFlags(o, OBJ_GetFlags(o) | OF_TEMPLATE);
    else Con_Error("PrefabPlaceholder missing PlaceholderTemplate attribute");
    return o ? make_object_visitor(o, TRUE) : make_skip_visitor();
  }
  struct ClassDesc const *cls = OBJ_FindClass(tag);
  if (!cls) { Con_Error("Unknown element type '%s'", tag); return make_skip_visitor(); }
  struct Object *o = OBJ_Create(cls->ClassID);
  if (!o) return make_skip_visitor();
  apply_attrs(o, ns, vs, n);
  return make_object_visitor(o, FALSE);
}

/* SAX */

static void
sax_start(void *ud, const xmlChar *name, const xmlChar **atts)
{
  struct sax_ctx *ctx = ud;
  if (ctx->depth >= XML_MAX_DEPTH) {
    Con_Error("XML nesting exceeds maximum depth of %d at element '<%s>'", XML_MAX_DEPTH, name);
    return;
  }
  lpcString_t tag = (lpcString_t)name;
  lpcString_t ns[XML_MAX_ATTRS], vs[XML_MAX_ATTRS]; int n = 0;
  if (atts) {
    while (atts[n*2] && n < XML_MAX_ATTRS) { ns[n] = (lpcString_t)atts[n*2]; vs[n] = (lpcString_t)atts[n*2+1]; n++; }
    if (atts[n*2]) Con_Error("Element '<%s>' has more than %d attributes; extras ignored", tag, XML_MAX_ATTRS);
  }
  xml_visitor_t *child = ctx->depth == 0
    ? visitor_for_element(tag, ns, vs, n)
    : ctx->stack[ctx->depth-1]->start(ctx->stack[ctx->depth-1], tag, ns, vs, n);
  ctx->stack[ctx->depth++] = child ? child : make_skip_visitor();
}

static void
sax_end(void *ud, const xmlChar *name)
{
  (void)name;
  struct sax_ctx *ctx = ud;
  if (ctx->depth <= 0) return;
  xml_visitor_t *v = ctx->stack[--ctx->depth];
  if (v->end) v->end(v);
  struct Object *res = v->result ? v->result(v) : NULL;
  if (ctx->depth > 0) { xml_visitor_t *p = ctx->stack[ctx->depth-1]; if (p->child) p->child(p, res); }
  else ctx->result = res;
  v->destroy(v);
}

static void
sax_chars(void *ud, const xmlChar *ch, int len)
{
  struct sax_ctx *ctx = ud;
  if (ctx->depth > 0 && ctx->stack[ctx->depth-1]->text)
    ctx->stack[ctx->depth-1]->text(ctx->stack[ctx->depth-1], (lpcString_t)ch, len);
}

static struct Object *
load_doc(char const *xml, int len)
{
  char *expanded = _ExpandXmlPositionalArgs(xml);
  lpcString_t src = expanded ? expanded : xml;
  int sz = expanded ? (int)strlen(expanded) : len;
  static xmlSAXHandler sax = {
    .startElement = sax_start, .endElement = sax_end,
    .characters = sax_chars, .ignorableWhitespace = sax_chars, .initialized = 1,
  };
  struct sax_ctx ctx = {0};
  xmlSAXUserParseMemory(&sax, &ctx, src, sz);
  free(expanded);
  return ctx.result;
}

struct Object *
FS_LoadObjectFromXml(lpcString_t path)
{
  struct file *fp = FS_LoadFile(path);
  struct Object *o = fp ? load_doc((char const*)fp->data, fp->size) : NULL;
  if (fp) FS_FreeFile(fp);
  if (o) OBJ_SetSourceFile(o, path);
  else Con_Error("Failed to parse '%s'", path);
  return o;
}

ORCA_API struct Object *
FS_LoadObjectFromXmlString(lpcString_t xml)
{
  struct Object *o = load_doc(xml, (int)strlen(xml));
  if (!o) Con_Error("FS_LoadObjectFromXmlString: failed to parse XML string");
  return o;
}
