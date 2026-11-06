#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <zlib.h>


#include <source/filesystem/fs_local.h>
#include <source/core/core_local.h>

#include <core/core.h>
#include <UIKit/UIKit.h>

#define REQUIRE(type, name, expr, error) type name = expr; if (!name) return error;
#define ERROR_CANT_FIND_OBJECT "Can't find object"
#define SV_ARG(NAME) rargs_input(rargs, nargs, NAME)
#define MAX_ARGS 16
static int hexval(char c) {
  return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 :
         c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
}

/* Decode %XX escapes in place; the editor encodes values that hold spaces,
   quotes, '&', '=' or '?' (binding expressions, colors, grid tracks). */
static void url_decode(LPSTR s) {
  LPSTR o = s;
  for (; *s; s++) {
    int hi, lo;
    if (*s == '%' && (hi = hexval(s[1])) >= 0 && (lo = hexval(s[2])) >= 0) {
      *o++ = (char)(hi * 16 + lo); s += 2;
    } else *o++ = *s;
  }
  *o = 0;
}

lpcString_t FS_ParseArgs(LPSTR s, reqArg_t *args, size_t maxargs) {
  char *q=strchr(s,'?');
  if(!q) return url_decode(s), s;
  *q++=0;
  url_decode(s);
  reqArg_t *r=args;
  for(char *tok=strtok(q,"&"); tok && maxargs--; tok=strtok(NULL,"&"),r++){
    char *eq=strchr(tok,'=');
    if(eq){ *eq=0; strncpy(r->name,tok,sizeof r->name-1); strncpy(r->value,eq+1,sizeof r->value-1);}
    else { strncpy(r->name,tok,sizeof r->name-1); *r->value=0; }
    url_decode(r->name); url_decode(r->value);
  }
  return s;
}
lpcString_t rargs_input(reqArg_t const *args, size_t maxargs, lpcString_t name) {
  for (reqArg_t const *it = args; it-args<maxargs;it++) {
    if (!strcmp(it->name, name)) {
      return it->value;
    }
  }
  return NULL;
}

void xmlSetIntProp(xmlNodePtr node, xmlChar const* name, int value) {
  char _id[64]={0};
  snprintf(_id, sizeof(_id), "%d", value);
  xmlSetProp(node, name, XMLSTR(_id));
}

bool_t OBJ_IsPrefabView(struct Object const *object);

/* Screens and prefabs are loaded from disk rather than attached to the
   workspace tree, so resolve "Project/Screens/Main/Child" by finding the
   longest prefix that names a scene file, loading (or reusing) that document
   and descending into it. The startup screen is the live object on display. */
static char const editor_documents_lua[] =
  "local fs = require 'orca.filesystem'\n"
  "local docs = {}\n"
  "return function(doc, startup)\n"
  "  local o = docs[doc]\n"
  "  if o then return o end\n"
  "  if doc == startup then\n"
  "    local A = package.loaded['orca.core.application']\n"
  "    o = A and A.app and A.app.screen\n"
  "  end\n"
  "  o = o or fs.loadObject(doc)\n"
  "  docs[doc] = o\n"
  "  return o\n"
  "end\n";

static int editor_documents_ref = LUA_NOREF;

static bool_t is_document(lpcString_t path) {
  lpcString_t rel = strchr(path, '/');
  path_t file = {0};
  if (!rel) return FALSE;
  snprintf(file, sizeof(file), ".%s%s", rel, FS_GetSceneExtension());
  return access(file, R_OK) == 0;
}

static struct Object *find_object(lua_State *L, lpcString_t path) {
  struct Object *o = OBJ_FindByPath(FS_GetWorkspace(), path);
  if (o || !path) return o;
  if (editor_documents_ref == LUA_NOREF) {
    if (luaL_dostring(L, editor_documents_lua) != LUA_OK) {
      Con_Error("%s", lua_tostring(L, -1));
      lua_pop(L, 1);
      return NULL;
    }
    editor_documents_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  struct Object *project = OBJ_GetFirstChild(FS_GetWorkspace());
  lpcString_t startup = project && GetProject(project) ? GetProject(project)->StartupScreen : NULL;
  path_t doc = {0};
  strncpy(doc, path, sizeof(doc) - 1);
  for (LPSTR cut = doc + strlen(doc); cut > doc;) {
    *cut = 0;
    if (is_document(doc)) {
      lua_rawgeti(L, LUA_REGISTRYINDEX, editor_documents_ref);
      lua_pushstring(L, doc);
      lua_pushstring(L, startup ? startup : "");
      if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
        Con_Error("%s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return NULL;
      }
      struct Object *root = lua_isnil(L, -1) ? NULL : luaX_checkObject(L, -1);
      lua_pop(L, 1);
      lpcString_t rest = path + strlen(doc);
      return root && *rest == '/' ? OBJ_FindByPath(root, rest + 1) : root;
    }
    while (cut > doc && *--cut != '/');
  }
  return NULL;
}


static int print_classes(struct ClassDesc const *cd, LPSTR s, int n) {
  if (!cd || n <= 0) return 0;
  int w = snprintf(s, n, "%s,", cd->ClassName);
  n -= w;
  for (uint32_t const *p = cd->ParentClasses; *p && n > 1; p++)
    w += print_classes(OBJ_FindClassW(*p), s + w, n - w);
  return w;
}

static void object_hierarchy(struct Object *object, xmlNodePtr parent) {
  struct ClassDesc const *pClass = OBJ_FindClass(OBJ_GetClassName(object));
  lpcString_t szClassName = OBJ_GetClassName(object);
  xmlNodePtr xml = xmlNewChild(parent, NULL, XMLSTR(szClassName), NULL);
  _xmlSetProp(xml, "name", OBJ_GetName(object));
  if (pClass) {
    _xmlSetProp(xml, "data-content", pClass->ContentType);
  }
  if (OBJ_IsHidden(object) || (GetNode(object) && GetNode(object)->QuickHide)) {
    _xmlSetProp(xml, "data-hidden", "true");
  }
  if (OBJ_GetSourceFile(object) && !GetProject(object)) {
    _xmlSetProp(xml, "data-source", OBJ_GetSourceFile(object));
  }
//  if (OBJ_IsHidden(object) || (GetNode(object) && GetNode(object)->QuickHide)) {
//    _xmlSetProp(xml, "data-content", "Node2D");
//  }
  if (OBJ_IsPrefabView(object)) {
    _xmlSetProp(xml, "data-type", "view");
  } else if ((OBJ_GetFlags(object)&OF_LOADED_FROM_PREFAB)) {
    _xmlSetProp(xml, "data-type", "prefab");
  } else if (OBJ_GetSourceFile(object)) {
    _xmlSetProp(xml, "data-type", "prefab");
  } else {
    _xmlSetProp(xml, "data-type", "object");
  }
  char buf[256]={0};
  if(pClass)print_classes(pClass, buf, sizeof(buf));
  _xmlSetProp(xml, "data-class", buf);
//  Con_Error("%s %s %s", OBJ_GetName(object), buf, OBJ_GetClassName(object));
  FOR_EACH_CHILD(object, object_hierarchy, xml);
}

static SV_CMD(GET, scene_hierarchy) {
  struct Object *object = find_object(L, endpoint);
  _xmlSetProp(response, "source", endpoint);
  if (object) {
    object_hierarchy(object, response);
    return NULL;
  } else {
    return ERROR_CANT_FIND_OBJECT;
  }
}


struct Object *
UI_FindObjectByUniqueID(uint32_t uid, struct Object *object);

static void add_group(struct ClassDesc const *dec, void* parm) {
  xmlNodePtr p = _xmlNewChild(parm, "group", "name", dec->ClassName);
  _xmlSetProp(p, "data-compound", "true");
}

ORCA_API lpcString_t
PDESC_Print(struct PropertyType const *pdesc, LPSTR buffer, uint32_t len, float const* pf);

#define _xmlAddProp(parent, name, value, type)\
_xmlNewChild(parent, DataTypeToString(type), "name", name, "value", value, "data-type", DataTypeToString(type))

/* Look a property up without instantiating it: OBJ_FindLongProperty creates
   class properties on demand, which would mark every inspected value as set. */
static struct Property *
find_existing_property(struct Object const *obj, struct PropertyType const *pdesc)
{
  for (struct Property *p = OBJ_GetProperties((struct Object *)obj); p; p = PROP_GetNext(p)) {
    struct PropertyType const *d = PROP_GetDesc(p);
    if (d == pdesc || (pdesc->FullIdentifier && d && d->FullIdentifier == pdesc->FullIdentifier))
      return p;
  }
  return NULL;
}

static int
add_subproperty(xmlNodePtr xml,
                struct Object const *obj,
                struct PropertyType const *pdesc,
                void const* dest)
{
  /* DataType enum values are generated as: None..Object (0..9).
   * Dynamic/custom properties may carry invalid type IDs; skip them to keep
   * the editor server endpoint robust instead of asserting in DataTypeToString.
   */
  if (!pdesc || pdesc->DataType < 0 || pdesc->DataType > kDataTypeObject) {
    return pdesc ? (pdesc->Offset + pdesc->DataSize) : 0;
  }

  path_t buf = {0};
  if (dest) PDESC_Print(pdesc, buf, sizeof(buf), dest);
  if (!xmlCheckUTF8(XMLSTR(buf))) *buf = 0;
  xmlNodePtr n = _xmlAddProp(xml, pdesc->Name, buf, pdesc->DataType);
  if (pdesc->Category && *pdesc->Category) {
    _xmlSetPropFormat(n, "data-id", "%s.%s", pdesc->Category, pdesc->Name);
  }
  struct Property *prop = find_existing_property(obj, pdesc);
  if (prop && (PROP_GetFlags(prop) & PF_MODIFIED)) {
    _xmlSetProp(n, "data-set", "true");
  }
  lpcString_t expression = prop ? PROP_GetBindingExpression(prop) : NULL;
  if (expression) {
    _xmlSetProp(n, "data-program", expression);
  }
  switch (pdesc->DataType) {
    case kDataTypeObject:
      _xmlSetProp(n, "data-content", pdesc->TypeString);
      break;
    case kDataTypeEnum:
      if (pdesc->EnumValues) {
        for (lpcString_t const *t = pdesc->EnumValues; *t; t++)
          _xmlNewChild(n, "menu-item", "header", *t, "name", *t);
      } else if (pdesc->TypeString) {
        WITH(char, s, strdup(pdesc->TypeString), free) {
          for (LPSTR t = strtok(s,","); t; t = strtok(NULL, ","))
            _xmlNewChild(n, "menu-item", "header", t, "name", t);
        }
      }
      break;
    case kDataTypeStruct:
      _xmlSetProp(n, "data-compound", "true");
      /* Fields come from the struct descriptor; their values are shown for
         reference and edited through the parent's textual value. */
      WITH(struct StructDesc const, sd, OBJ_FindStructDesc(pdesc->TypeString), (void)) {
        FOR_LOOP(i, sd->NumProperties) {
          struct PropertyType const *field = &sd->Properties[i];
          if (!field->Name || field->DataType < 0 || field->DataType > kDataTypeObject) continue;
          path_t fbuf = {0};
          if (dest) PDESC_Print(field, fbuf, sizeof(fbuf), (float const *)((byte_t const *)dest + field->Offset));
          if (!xmlCheckUTF8(XMLSTR(fbuf))) *fbuf = 0;
          xmlNodePtr f = _xmlAddProp(n, field->Name, fbuf, field->DataType);
          _xmlSetProp(f, "data-readonly", "true");
        }
      }
      break;
    default:
      break;
  }
  return pdesc->Offset + pdesc->DataSize;;
}

static xmlNodePtr
xmlFindNode(xmlNodePtr node, xmlChar const *name) {
  xmlForEach(child, node) {
    if (xmlStrcmp(child->name, XMLSTR("group"))) continue;
    WITH(xmlChar, _name, xmlGetProp(child, XMLSTR("name")), xmlFree) {
      if (xmlStrcmp(_name, name)) continue;
      return child;
    }
  }
  return NULL;
}

#define DEFAULT_GROUP "Object"
#define CUSTOM_GROUP "Custom"

static void
add_property(struct Object const *obj,
             struct PropertyType const *pdesc,
             struct ClassDesc const *cdesc,
             void const* dest,
             void* parm)
{
  xmlNodePtr group = xmlFindNode(parm, XMLSTR(cdesc->ClassName));
  if (!group) {
    group = xmlFindNode(parm, XMLSTR(DEFAULT_GROUP));
  }
  add_subproperty(group, obj, pdesc, dest);
}

SV_CMD(GET, node)
{
  REQUIRE(struct Object *, object, find_object(L, endpoint), ERROR_CANT_FIND_OBJECT);
  xmlNodePtr _g = _xmlNewChild(response, "group", "name", DEFAULT_GROUP, "data-compound", "true");
//  xmlNodePtr _c = _xmlAddProp(_g, "Class", OBJ_GetClassName(object), kDataTypeString);
//  _xmlSetProp(_c, "data-readonly", "true");
  if (GetNode(object)) {
    _xmlSetProp(response, "QuickHide", GetNode(object)->QuickHide?"true":"false");
  }
  if (OBJ_GetSourceFile(object)) {
    xmlNodePtr _g = _xmlNewChild(response, "group", "name", "PrefabTemplate", "data-readonly", "true");
    _xmlAddProp(_g, "Template", OBJ_GetSourceFile(object), kDataTypeString);
  }
  _xmlSetProp(response, "source", endpoint);
  _xmlAddProp(_g, "Name", OBJ_GetName(object), kDataTypeString);

  OBJ_EnumObjectClasses(object, add_group, response);
  OBJ_EnumClassProperties(object, add_property, response);
  
  for (struct Property const *p = OBJ_GetProperties(object); p; p = PROP_GetNext(p)) {
    struct PropertyType const *pd = PROP_GetDesc(p);
    /* Class properties were listed in their class groups above. */
    if ((PROP_GetFlags(p)&PF_PROPERTY_TYPE) || !pd || !pd->Name ||
        (pd->Category && xmlFindNode(response, XMLSTR(pd->Category))))
      continue;
    xmlNodePtr group = xmlFindNode(response, XMLSTR(CUSTOM_GROUP));
    if (!group) {
      group = _xmlNewChild(response, "group", "name", CUSTOM_GROUP, "data-compound", "true");
    }
    add_subproperty(group, object, pd, PROP_GetValue(p));
  }

  return NULL;
}


/* Edit journal. The engine serializer resolves styles and theme values, so a
   document is saved by replaying the edits made in this session onto its
   source XML, leaving everything else in the file as authored. */
enum edit_op { EDIT_SET, EDIT_BIND, EDIT_CREATE, EDIT_DELETE, EDIT_DUPLICATE, EDIT_RENAME };

struct edit {
  enum edit_op op;
  char *path, *name, *value, *extra;
  struct edit *next;
};

static struct edit *edits, **edits_tail = &edits;

static void
record_edit(enum edit_op op, lpcString_t path, lpcString_t name, lpcString_t value, lpcString_t extra)
{
  struct edit *e = calloc(1, sizeof(*e));
  if (!e) return;
  e->op = op;
  e->path = strdup(path ? path : "");
  e->name = name ? strdup(name) : NULL;
  e->value = value ? strdup(value) : NULL;
  e->extra = extra ? strdup(extra) : NULL;
  *edits_tail = e;
  edits_tail = &e->next;
}

static void
free_edit(struct edit *e)
{
  free(e->path); free(e->name); free(e->value); free(e->extra); free(e);
}

extern int parse_property(const char *, struct PropertyType const *, void *);

static struct PropertyType const *
find_property_type(struct Object *obj, lpcString_t name)
{
  return strchr(name, '.') ? OBJ_FindExplicitPropertyType(obj, name)
                           : OBJ_FindImplicitPropertyType(obj, name);
}

/* Assign one textual value the way the XML reader does, recording the previous
   value in the response so the editor can undo the edit. */
static lpcString_t
set_property_text(struct Object *obj, lpcString_t path, lpcString_t name, lpcString_t value, xmlNodePtr response)
{
  struct PropertyType const *pd = find_property_type(obj, name);
  if (!pd) {
    if (OBJ_SetShorthandValueFromString(obj, name, value)) {
      record_edit(EDIT_SET, path, name, value, NULL);
      return NULL;
    }
    return "Unknown property";
  }
  struct Property *p = OBJ_FindLongProperty(obj, pd->FullIdentifier);
  if (!p) return "Can't create property";
  path_t old = {0};
  PROP_Print(p, old, sizeof(old));
  _xmlAddProp(response, name, old, pd->DataType);
  if (pd->DataType == kDataTypeString) {
    PROP_SetStringValue(p, value);
  } else {
    char tmp[MAX_PROPERTY_STRING] = {0};
    if (!parse_property(value, pd, tmp)) return "Can't parse value";
    PROP_SetValue(p, tmp);
  }
  record_edit(EDIT_SET, path, pd->Name, value, name);
  return NULL;
}

SV_CMD(PUT, node)
{
  REQUIRE(struct Object *, obj, find_object(L, endpoint), ERROR_CANT_FIND_OBJECT);
  _xmlSetProp(response, "command", "edit");
  _xmlSetProp(response, "source", endpoint);
  for(reqArg_t const* arg = rargs; arg - rargs < nargs && *arg->name; arg++) {
    if (!strcmp(arg->name, "Name")) {
      _xmlAddProp(response, arg->name, OBJ_GetName(obj), kDataTypeString);
      OBJ_SetName(obj, arg->value);
      record_edit(EDIT_RENAME, endpoint, NULL, arg->value, NULL);
    } else {
      lpcString_t err = set_property_text(obj, endpoint, arg->name, arg->value, response);
      if (err) return err;
    }
  }
  return NULL;
}

static lpcString_t const binding_modes[] = { "OneWay", "TwoWay", "OneWayToSource", "Expression", "OneTime" };

/* put/binding/<path>?property=Node.Width&expression=...&mode=Expression
   An empty expression removes the binding. */
SV_CMD(PUT, binding)
{
  REQUIRE(struct Object *, obj, find_object(L, endpoint), ERROR_CANT_FIND_OBJECT);
  REQUIRE(lpcString_t, name, SV_ARG("property"), "Missing `property` argument");
  lpcString_t expression = SV_ARG("expression");
  lpcString_t modeText = SV_ARG("mode");
  struct PropertyType const *pd = find_property_type(obj, name);
  if (!pd) return "Unknown property";
  struct Property *p = OBJ_FindLongProperty(obj, pd->FullIdentifier);
  if (!p) return "Can't create property";
  lpcString_t previous = PROP_GetBindingExpression(p);
  _xmlSetProp(response, "command", "bind");
  _xmlSetProp(response, "source", endpoint);
  _xmlSetProp(response, "property", name);
  _xmlSetProp(response, "previous", previous ? previous : "");
  enum BindingMode mode = kBindingModeExpression;
  FOR_LOOP(i, sizeof(binding_modes)/sizeof(*binding_modes))
    if (modeText && !strcmp(modeText, binding_modes[i])) mode = (enum BindingMode)i;
  if (!expression || !*expression) {
    if (previous) PROP_SetBinding(p, previous, mode, FALSE);
    record_edit(EDIT_BIND, endpoint, name, NULL, binding_modes[mode]);
    return NULL;
  }
  if (!PROP_SetBinding(p, expression, mode, TRUE)) return "Invalid binding expression";
  record_edit(EDIT_BIND, endpoint, name, expression, binding_modes[mode]);
  return NULL;
}

static void
binding_function(xmlNodePtr response, lpcString_t name, lpcString_t signature, lpcString_t summary)
{
  _xmlNewChild(response, "function", "name", name, "signature", signature, "summary", summary);
}

/* The binding VM's function table, for completion and highlighting. */
SV_CMD(GET, binding_functions)
{
  binding_function(response, "IF", "IF(condition, then, else)", "Choose between two values");
  binding_function(response, "STEP", "STEP(edge, x)", "1 when edge <= x, else 0");
  binding_function(response, "LINEARSTEP", "LINEARSTEP(a, b, x)", "Clamped linear ramp");
  binding_function(response, "MIX", "MIX(a, b, t)", "Linear interpolation");
  binding_function(response, "CLAMP", "CLAMP(x, lo, hi)", "Clamp to a range");
  binding_function(response, "MIN", "MIN(a, b)", "Smaller value");
  binding_function(response, "MAX", "MAX(a, b)", "Larger value");
  binding_function(response, "ABS", "ABS(x)", "Absolute value");
  binding_function(response, "ROUND", "ROUND(x)", "Round to nearest");
  binding_function(response, "FLOOR", "FLOOR(x)", "Round down");
  binding_function(response, "CEIL", "CEIL(x)", "Round up");
  binding_function(response, "SQRT", "SQRT(x)", "Square root");
  binding_function(response, "POW", "POW(x, y)", "Power");
  binding_function(response, "MOD", "MOD(x, y)", "Modulo");
  binding_function(response, "AND", "AND(a, b)", "Logical and");
  binding_function(response, "OR", "OR(a, b)", "Logical or");
  binding_function(response, "EQUAL", "EQUAL(a, b)", "Equality test");
  binding_function(response, "STRING", "STRING(x)", "Convert to text");
  binding_function(response, "INT", "INT(x)", "Convert to integer");
  binding_function(response, "FLOAT", "FLOAT(x)", "Convert to number");
  binding_function(response, "BOOL", "BOOL(x)", "Convert to boolean");
  binding_function(response, "VECTOR2", "VECTOR2(x, y)", "Two-component vector");
  binding_function(response, "VECTOR3", "VECTOR3(x, y, z)", "Three-component vector");
  binding_function(response, "VECTOR4", "VECTOR4(x, y, z, w)", "Four-component vector");
  binding_function(response, "COLOR4", "COLOR4(r, g, b, a)", "Color from components");
  binding_function(response, "ANIMATE", "ANIMATE(value)", "Animate towards a value");
  binding_function(response, "ADD", "ADD(a, b)", "Add");
  binding_function(response, "SUB", "SUB(a, b)", "Subtract");
  binding_function(response, "MUL", "MUL(a, b)", "Multiply");
  binding_function(response, "DIV", "DIV(a, b)", "Divide");
  binding_function(response, "REM", "REM(a, b)", "Remainder");
  binding_function(response, "SCALE", "SCALE(x, factor)", "Scale a value");
  binding_function(response, "UNM", "UNM(x)", "Negate");
  binding_function(response, "CREATEROTATION", "CREATEROTATION(x, y, z)", "Rotation from Euler angles");
  binding_function(response, "CREATEROTATIONX", "CREATEROTATIONX(angle)", "Rotation about X");
  binding_function(response, "CREATEROTATIONY", "CREATEROTATIONY(angle)", "Rotation about Y");
  binding_function(response, "CREATEROTATIONZ", "CREATEROTATIONZ(angle)", "Rotation about Z");
  binding_function(response, "VECTOR3X", "VECTOR3X(v)", "X component");
  binding_function(response, "VECTOR3Y", "VECTOR3Y(v)", "Y component");
  binding_function(response, "VECTOR3Z", "VECTOR3Z(v)", "Z component");
  binding_function(response, "COLORR", "COLORR(color)", "Red component");
  binding_function(response, "COLORG", "COLORG(color)", "Green component");
  binding_function(response, "COLORB", "COLORB(color)", "Blue component");
  binding_function(response, "COLORA", "COLORA(color)", "Alpha component");
  binding_function(response, "EXTRACTEULERX", "EXTRACTEULERX(rotation)", "Euler X angle");
  binding_function(response, "EXTRACTEULERY", "EXTRACTEULERY(rotation)", "Euler Y angle");
  binding_function(response, "EXTRACTEULERZ", "EXTRACTEULERZ(rotation)", "Euler Z angle");
  return NULL;
}

/* post/duplicate/<path>: clone a node next to itself. */
SV_CMD(POST, duplicate)
{
  REQUIRE(struct Object *, object, find_object(L, endpoint), ERROR_CANT_FIND_OBJECT);
  REQUIRE(struct Object *, parent, OBJ_GetParent(object), "Can't duplicate a root object");
  REQUIRE(struct Object *, copy, OBJ_Instantiate(object), "Can't duplicate object");
  path_t name = {0};
  snprintf(name, sizeof(name), "%s Copy", OBJ_GetName(object));
  OBJ_SetName(copy, SV_ARG("name") ? SV_ARG("name") : name);
  OBJ_AddChild(parent, copy);
  _xmlSetProp(response, "name", OBJ_GetName(copy));
  record_edit(EDIT_DUPLICATE, endpoint, NULL, OBJ_GetName(copy), NULL);
  return NULL;
}

static void
layout_rects(struct Object *object, xmlNodePtr parent, lpcString_t path)
{
  path_t child_path = {0};
  FOR_EACH_OBJECT(child, object) {
    snprintf(child_path, sizeof(child_path), "%s/%s", path, OBJ_GetName(child));
    struct Node2D *n2 = GetNode2D(child);
    struct Node *n = GetNode(child);
    if (n2 && n) {
      xmlNodePtr r = _xmlNewChild(parent, "rect", "path", child_path);
      _xmlSetPropFormat(r, "x", "%g", n2->Matrix.v[12]);
      _xmlSetPropFormat(r, "y", "%g", n2->Matrix.v[13]);
      _xmlSetPropFormat(r, "w", "%g", n->Size.Axis[0].Actual);
      _xmlSetPropFormat(r, "h", "%g", n->Size.Axis[1].Actual);
    }
    layout_rects(child, parent, child_path);
  }
}

/* get/layout/<screen>: screen-space rectangles of every 2D node, used by the
   editor preview for selection outlines and click-to-select. */
SV_CMD(GET, layout)
{
  REQUIRE(struct Object *, object, find_object(L, endpoint), ERROR_CANT_FIND_OBJECT);
  _xmlSetProp(response, "source", endpoint);
  layout_rects(object, response, endpoint);
  return NULL;
}

SV_CMD(GET, startup_screen) {
  struct Object *project = OBJ_GetFirstChild(FS_GetWorkspace());
  if (project && GetProject(project) && GetProject(project)->StartupScreen && *GetProject(project)->StartupScreen) {
    return GET_scene_hierarchy(L, response, GetProject(project)->StartupScreen, rargs, nargs);
  } else {
    return "No startup screen specified in project or no project opened";
  }
}

static int compare_names(void const *a, void const *b) {
  return strcasecmp(*(char *const *)a, *(char *const *)b);
}

/* List "dir" sorted, skipping hidden entries; the caller frees the result. */
static int list_dir(lpcString_t dir, char ***out) {
  DIR *d = opendir(dir);
  int n = 0, cap = 0;
  char **names = NULL;
  for (struct dirent *e; d && (e = readdir(d));) {
    if (*e->d_name == '.') continue;
    if (n == cap) names = realloc(names, (cap = cap ? cap * 2 : 32) * sizeof(*names));
    names[n++] = strdup(e->d_name);
  }
  if (d) closedir(d);
  if (n) qsort(names, n, sizeof(*names), compare_names);
  *out = names;
  return n;
}

static lpcString_t asset_kind(lpcString_t name) {
  lpcString_t ext = strrchr(name, '.');
  if (!ext) return "file";
  if (!strcasecmp(ext, FS_GetSceneExtension())) return "document";
  if (!strcasecmp(ext, ".png") || !strcasecmp(ext, ".jpg") || !strcasecmp(ext, ".svg")) return "image";
  if (!strcasecmp(ext, ".css")) return "style";
  if (!strcasecmp(ext, ".lua") || !strcasecmp(ext, ".moon")) return "script";
  if (!strcasecmp(ext, ".glsl") || !strcasecmp(ext, ".vert") || !strcasecmp(ext, ".frag")) return "shader";
  return "file";
}

static void list_assets(xmlNodePtr parent, lpcString_t dir, lpcString_t logical, int depth) {
  char **names = NULL;
  int n = list_dir(dir, &names);
  FOR_LOOP(i, n) {
    path_t fsPath = {0}, path = {0};
    snprintf(fsPath, sizeof(fsPath), "%s/%s", dir, names[i]);
    struct stat st;
    if (stat(fsPath, &st)) { free(names[i]); continue; }
    if (S_ISDIR(st.st_mode)) {
      snprintf(path, sizeof(path), "%s/%s", logical, names[i]);
      xmlNodePtr folder = _xmlNewChild(parent, "Folder", "name", names[i], "data-type", "group");
      if (depth < 3) list_assets(folder, fsPath, path, depth + 1);
    } else {
      lpcString_t kind = asset_kind(names[i]);
      xmlNodePtr item = _xmlNewChild(parent, "Asset", "name", names[i], "data-type", kind,
                                     "data-file", fsPath + 2 /* skip "./" */);
      if (!strcmp(kind, "document")) {
        *strrchr(names[i], '.') = 0;
        snprintf(path, sizeof(path), "%s/%s", logical, names[i]);
        xmlSetProp(item, XMLSTR("name"), XMLSTR(names[i]));
        _xmlSetProp(item, "data-source", path);
      }
    }
    free(names[i]);
  }
  free(names);
}

/* The project's assets as they are on disk (the server runs from the project
   directory): scenes open as documents, other files are listed for reference. */
SV_CMD(GET, project_overview) {
  struct Object *proj = OBJ_GetFirstChild(FS_GetWorkspace());
  if (!proj) return "No project opened";
  xmlNodePtr root = _xmlNewChild(response, "Project", "name", OBJ_GetName(proj), "data-type", "group");
  list_assets(root, ".", OBJ_GetName(proj), 0);
  return NULL;
}

SV_CMD(GET, whoami) {
  struct Object *project = OBJ_GetFirstChild(FS_GetWorkspace());
  if (project) {
    GET_node(L, response, OBJ_GetName(project), rargs, nargs);
    return NULL;
  } else {
    return "No project opened";
  }
}

ORCA_API struct Object *OBJ_LoadDocument(xmlDocPtr doc) {
  xmlNodePtr root = doc ? xmlDocGetRootElement(doc) : NULL;
  struct Object *obj = root ? FS_BuildObjectTree(root, NULL) : NULL;
  if (!obj) {
    Con_Error("Failed to build editor object tree");
  }
  return obj;
}

ORCA_API xmlNodePtr ED_ConvertNode(struct Object *object, xmlNodePtr parent);
SV_CMD(POST, node) {
  REQUIRE(struct Object *, root, find_object(L, endpoint), ERROR_CANT_FIND_OBJECT);
  if (SV_ARG("source")) {
    REQUIRE(struct Object *, source, find_object(L, SV_ARG("source")), "Can't find specified source object");
    if (OBJ_GetSourceFile(source)) return "Object is already a prefab";
    WITH(xmlDoc, doc, xmlNewDoc(XMLSTR("1.0")), xmlFree) {
      xmlDocSetRootElement(doc, ED_ConvertNode(source, NULL));
      OBJ_AddChild(root, OBJ_LoadDocument(doc));
    }
    path_t sourcepath = {0};
    OBJ_SetSourceFile(source, FS_JoinPaths(sourcepath, sizeof(sourcepath), endpoint, OBJ_GetName(source)));
    return NULL;
  }
  REQUIRE(lpcString_t, cls, SV_ARG("class"), "Should provide 'class' or 'source' argument");
  lua_getfield(L, LUA_REGISTRYINDEX, cls);
  if (lua_isnil(L, -1)) {
    return "Can't find specified class";
  } else if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
    return luaL_checkstring(L, -1);
  } else {
    struct Object *object = luaX_checkObject(L, -1);
    if (SV_ARG("name")) OBJ_SetName(object, SV_ARG("name"));
    OBJ_AddChild(root, object);
    record_edit(EDIT_CREATE, endpoint, cls, OBJ_GetName(object), NULL);
    lua_pop(L, 1);
    return NULL;
  }
}

SV_CMD(DELETE, node) {
  REQUIRE(struct Object *, object, find_object(L, endpoint), ERROR_CANT_FIND_OBJECT);
  uint32_t index = 0;
  FOR_EACH_OBJECT(child, object) {
    if (child != object) index++;
    else break;
  }
  _xmlSetPropFormat(response, "index", "%d", index);
  _xmlSetProp(response, "command", "create");
  OBJ_RemoveFromParent(object);
  record_edit(EDIT_DELETE, endpoint, NULL, NULL, NULL);
  return NULL;
}

SV_CMD(GET, preview) {
  xmlNodePtr n = xmlNewChild(response, NULL, XMLSTR("IOSurfaceTexture"), NULL);
#if __APPLE__
  extern uint32_t _IOSurface;
  _xmlSetPropFormat(n, "IOSurface", "%d", _IOSurface);
#else
  _xmlSetPropFormat(n, "IOSurface", "%d", 0);
#endif
  return NULL;
}

/* Minimal RGBA PNG encoder for the preview snapshot (zlib does the work). */
static void png_chunk(FILE *f, char const *type, byte_t const *data, uint32_t len) {
  byte_t hdr[8] = { len >> 24, len >> 16, len >> 8, len, type[0], type[1], type[2], type[3] };
  fwrite(hdr, 1, 8, f);
  if (len) fwrite(data, 1, len, f);
  uLong crc = crc32(0, hdr + 4, 4);
  if (len) crc = crc32(crc, data, len); /* crc32(c, NULL, 0) restarts at 0 */
  byte_t c[4] = { crc >> 24, crc >> 16, crc >> 8, crc };
  fwrite(c, 1, 4, f);
}

static bool_t write_png(lpcString_t path, byte_t const *rgba, uint32_t w, uint32_t h) {
  size_t stride = w * 4, rawlen = (stride + 1) * h;
  byte_t *raw = malloc(rawlen);
  uLongf zlen = compressBound(rawlen);
  byte_t *z = malloc(zlen);
  FILE *f = raw && z ? fopen(path, "wb") : NULL;
  if (!f) return free(raw), free(z), FALSE;
  /* GL rows start at the bottom; PNG rows start at the top. */
  FOR_LOOP(y, h) {
    raw[y * (stride + 1)] = 0;
    memcpy(raw + y * (stride + 1) + 1, rgba + (h - 1 - y) * stride, stride);
  }
  compress2(z, &zlen, raw, rawlen, 6);
  static byte_t const sig[8] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1a, '\n' };
  byte_t ihdr[13] = { w >> 24, w >> 16, w >> 8, w, h >> 24, h >> 16, h >> 8, h, 8, 6, 0, 0, 0 };
  fwrite(sig, 1, 8, f);
  png_chunk(f, "IHDR", ihdr, 13);
  png_chunk(f, "IDAT", z, (uint32_t)zlen);
  png_chunk(f, "IEND", NULL, 0);
  fclose(f);
  free(raw); free(z);
  return TRUE;
}

/* get/snapshot/<screen>?width=&height=&file=: render the screen offscreen at
   the requested size and save it as PNG. Works without IOSurface sharing, so
   the editor preview is available on every platform. */
SV_CMD(GET, snapshot)
{
  REQUIRE(struct Object *, screen, find_object(L, endpoint), ERROR_CANT_FIND_OBJECT);
  REQUIRE(lpcString_t, file, SV_ARG("file"), "Missing `file` argument");
  if (!GetScreen(screen)) return "Object is not a screen";
  uint32_t w = SV_ARG("width") ? atoi(SV_ARG("width")) : 1024;
  uint32_t h = SV_ARG("height") ? atoi(SV_ARG("height")) : 768;
  if (!w || !h || w > 4096 || h > 4096) return "Invalid snapshot size";
  struct Texture *rt = NULL;
  if (FAILED(RenderTexture_Create(&(CREATERTSTRUCT){ .Width = w, .Height = h }, &rt))) {
    return "Can't create render target";
  }
  ORCA_API void CORE_AdvanceFrame(void);
  CORE_AdvanceFrame();
  PROP_RunAllPrograms(screen);
  _SendMessage(screen, Screen, UpdateLayout, .Width = w, .Height = h);
  _SendMessage(screen, Node, UpdateMatrix, .parent = MAT4_Identity(), .opacity = 1);
  _SendMessage(screen, Screen, RenderScreen, .width = w, .height = h, .target = rt);
  byte_t *pixels = malloc(w * h * 4);
  bool_t ok = pixels && SUCCEEDED(RenderTexture_ReadPixels(rt, pixels)) && write_png(file, pixels, w, h);
  free(pixels);
  Texture_Release(rt);
  /* Rectangles from this same layout pass, so outlines match the image. */
  layout_rects(screen, response, endpoint);
  _xmlSetProp(response, "file", file);
  _xmlSetPropFormat(response, "width", "%u", w);
  _xmlSetPropFormat(response, "height", "%u", h);
  return ok ? NULL : "Can't write snapshot";
}

static xmlNodePtr
child_named(xmlNodePtr parent, lpcString_t name, size_t len)
{
  for (xmlNodePtr c = parent ? parent->children : NULL; c; c = c->next) {
    if (c->type != XML_ELEMENT_NODE) continue;
    xmlChar *n = xmlGetProp(c, XMLSTR("Name"));
    bool_t match = n && strlen((char *)n) == len && !strncmp((char *)n, name, len);
    xmlFree(n);
    if (match) return c;
  }
  return NULL;
}

/* "Root/NavBar" below the document's root element; "" is the root. */
static xmlNodePtr
element_at(xmlNodePtr root, lpcString_t rel)
{
  xmlNodePtr n = root;
  while (n && *rel) {
    lpcString_t slash = strchr(rel, '/');
    size_t len = slash ? (size_t)(slash - rel) : strlen(rel);
    n = child_named(n, rel, len);
    rel = slash ? slash + 1 : rel + len;
  }
  return n;
}

static int
element_depth(xmlNodePtr n)
{
  int depth = 0;
  for (; n && n->parent && n->parent->type == XML_ELEMENT_NODE; n = n->parent) depth++;
  return depth;
}

/* Append a child on its own line, indented with tabs like the sources. */
static xmlNodePtr
append_child(xmlNodePtr parent, xmlNodePtr child)
{
  char indent[64] = "\n";
  int depth = MIN(element_depth(parent) + 1, (int)sizeof(indent) - 3);
  memset(indent + 1, '\t', depth);
  xmlNodePtr last = parent->last;
  bool_t trailing = last && last->type == XML_TEXT_NODE && xmlIsBlankNode(last);
  xmlNodePtr text = xmlNewText(XMLSTR(indent));
  if (trailing) {
    /* Insert the element first: libxml merges adjacent text nodes. */
    xmlAddPrevSibling(last, child);
    xmlAddPrevSibling(child, text);
  } else {
    xmlAddChild(parent, text);
    xmlAddChild(parent, child);
    indent[depth] = 0; /* closing tag one level up */
    xmlAddChild(parent, xmlNewText(XMLSTR(indent)));
  }
  return child;
}

static void
remove_element(xmlNodePtr n)
{
  xmlNodePtr prev = n->prev;
  if (prev && prev->type == XML_TEXT_NODE && xmlIsBlankNode(prev)) {
    xmlUnlinkNode(prev);
    xmlFreeNode(prev);
  }
  xmlUnlinkNode(n);
  xmlFreeNode(n);
}

static bool_t
apply_edit(xmlDocPtr doc, xmlNodePtr root, struct edit const *e, lpcString_t rel)
{
  xmlNodePtr n = element_at(root, rel);
  if (!n) return FALSE;
  switch (e->op) {
    case EDIT_SET:
      if (e->extra && strcmp(e->extra, e->name)) xmlUnsetProp(n, XMLSTR(e->extra));
      xmlSetProp(n, XMLSTR(e->name), XMLSTR(e->value));
      return TRUE;
    case EDIT_RENAME:
      xmlSetProp(n, XMLSTR("Name"), XMLSTR(e->value));
      return TRUE;
    case EDIT_BIND: {
      for (xmlNodePtr c = n->children, next; c; c = next) {
        next = c->next;
        if (c->type != XML_ELEMENT_NODE) continue;
        if (xmlStrcmp(c->name, XMLSTR("BindingExpression")) && xmlStrcmp(c->name, XMLSTR("Binding"))) continue;
        xmlChar *target = xmlGetProp(c, XMLSTR("Target"));
        bool_t same = target && !xmlStrcmp(target, XMLSTR(e->name));
        xmlFree(target);
        if (same) remove_element(c);
      }
      if (!e->value) return TRUE;
      xmlNodePtr b = xmlNewDocNode(doc, NULL, XMLSTR("BindingExpression"), NULL);
      xmlNodeAddContent(b, XMLSTR(e->value));
      xmlSetProp(b, XMLSTR("Target"), XMLSTR(e->name));
      if (e->extra && strcmp(e->extra, "Expression")) xmlSetProp(b, XMLSTR("Mode"), XMLSTR(e->extra));
      append_child(n, b);
      return TRUE;
    }
    case EDIT_CREATE: {
      xmlNodePtr c = xmlNewDocNode(doc, NULL, XMLSTR(e->name), NULL);
      xmlSetProp(c, XMLSTR("Name"), XMLSTR(e->value));
      append_child(n, c);
      return TRUE;
    }
    case EDIT_DUPLICATE: {
      xmlNodePtr copy = xmlDocCopyNode(n, doc, 1);
      xmlSetProp(copy, XMLSTR("Name"), XMLSTR(e->value));
      xmlNodePtr text = xmlNewText(n->prev && xmlIsBlankNode(n->prev) ? n->prev->content : XMLSTR("\n"));
      xmlAddNextSibling(n, copy);
      xmlAddPrevSibling(copy, text);
      return TRUE;
    }
    case EDIT_DELETE:
      if (n == root) return FALSE;
      remove_element(n);
      return TRUE;
  }
  return FALSE;
}

/* post/save/<document>: replay this session's edits under <document> onto
   its source file ("Project/Screens/Main" is "<project>/Screens/Main.xml").
   Edits that can't be located in the source (unnamed elements, nodes created
   by code) are reported and kept. */
SV_CMD(POST, save)
{
  lpcString_t rel = strchr(endpoint, '/');
  if (!rel) return "Not a document path";
  path_t file = {0};
  snprintf(file, sizeof(file), ".%s%s", rel, FS_GetSceneExtension());
  xmlDocPtr doc = xmlReadFile(file, NULL, XML_PARSE_NONET);
  if (!doc) return "Can't read the document's source file";
  xmlNodePtr root = xmlDocGetRootElement(doc);
  size_t prefix = strlen(endpoint);
  int applied = 0, skipped = 0;
  for (struct edit **it = &edits; *it;) {
    struct edit *e = *it;
    bool_t inside = !strncmp(e->path, endpoint, prefix) && (!e->path[prefix] || e->path[prefix] == '/');
    if (!inside) { it = &e->next; continue; }
    lpcString_t relpath = e->path + prefix + (e->path[prefix] == '/');
    if (apply_edit(doc, root, e, relpath)) {
      applied++;
      *it = e->next;
      free_edit(e);
    } else {
      skipped++;
      _xmlNewChild(response, "skipped", "path", e->path);
      it = &e->next;
    }
  }
  /* Rebuild the tail pointer after removals. */
  edits_tail = &edits;
  while (*edits_tail) edits_tail = &(*edits_tail)->next;
  int written = applied ? xmlSaveFileEnc(file, doc, "UTF-8") : 0;
  xmlFreeDoc(doc);
  _xmlSetProp(response, "file", file);
  _xmlSetPropFormat(response, "applied", "%d", applied);
  _xmlSetPropFormat(response, "skipped", "%d", skipped);
  return written < 0 ? "Can't write file" : NULL;
}

static void output_class(struct ClassDesc const *desc, void *param) {
  _xmlNewChild(param, "menu-item", "header", desc->ClassName, "name", desc->ClassName);
}

SV_CMD(GET, class) {
  OBJ_EnumClasses(fnv1a32(endpoint), output_class, response);
  return NULL;
}

static void output_property(struct ClassDesc const *desc, void *param) {
  xmlNodePtr n = _xmlNewChild(param, "menu-item", "header", desc->ClassName);
  FOR_LOOP(i, desc->NumProperties) {
    char buf[256];
    snprintf(buf, sizeof(buf), "%s.%s", desc->ClassName, desc->Properties[i].Name);
    _xmlNewChild(n, "menu-item", "header", desc->Properties[i].Name, "name", buf);
  }
}

SV_CMD(GET, property_types) {
  REQUIRE(struct Object *, object, find_object(L, endpoint), ERROR_CANT_FIND_OBJECT);
  OBJ_EnumObjectClasses(object, output_property, response);
  return NULL;
}

SV_CMD(POST, property_types) {
  REQUIRE(lpcString_t, classname, SV_ARG("name"), "Missing `name` argument");
  REQUIRE(struct Object *, object, find_object(L, endpoint), ERROR_CANT_FIND_OBJECT);
  struct Property *property = OBJ_FindLongProperty(object, fnv1a32(classname));
  if (!property) {
    return "Can't create property";
  }
  return NULL;
}

/* Kept for older clients; documents are saved one by one with save/<path>. */
SV_CMD(POST, save_project) {
  return "Use post/save/<document>";
}

#define ROUTE(OP, NAME) cmd_##OP, OP##_##NAME

enum cmd { cmd_GET, cmd_PUT, cmd_POST, cmd_DELETE, cmd_num };
static lpcString_t cmdsprfx[cmd_num] = { "get/", "put/", "post/", "delete/" };
struct {
  lpcString_t name;
  enum cmd op;
  lpcString_t (*func)(lua_State*, xmlNodePtr, lpcString_t, reqArg_t const*, uint32_t);
} routes[] = {
  { "whoami", ROUTE(GET, whoami) },
  { "scene-hierarchy/", ROUTE(GET, scene_hierarchy) },
  { "startup-screen", ROUTE(GET, startup_screen) },
  { "project-overview", ROUTE(GET, project_overview) },
  { "node/", ROUTE(GET, node) },
  { "node/", ROUTE(PUT, node) },
  { "node/", ROUTE(POST, node) },
  { "node/", ROUTE(DELETE, node) },
  { "preview", ROUTE(GET, preview) },
  { "class/", ROUTE(GET, class) },
  { "property-types/", ROUTE(GET, property_types) },
  { "property-types/", ROUTE(POST, property_types) },
  { "save-project", ROUTE(POST, save_project) },
  { "save/", ROUTE(POST, save) },
  { "binding/", ROUTE(PUT, binding) },
  { "binding-functions", ROUTE(GET, binding_functions) },
  { "duplicate/", ROUTE(POST, duplicate) },
  { "layout/", ROUTE(GET, layout) },
  { "snapshot/", ROUTE(GET, snapshot) },
};

/* Return the next request line from stdin, keeping partial reads and any
   further pipelined lines for the next call. */
LPSTR UI_ReadClientCommands(void) {
  static char buf[16384], line[sizeof(buf)];
  static size_t len = 0;
  for (;;) {
    char *nl = memchr(buf, '\n', len);
    if (nl) {
      size_t n = nl - buf;
      memcpy(line, buf, n);
      line[n] = 0;
      memmove(buf, nl + 1, len - n - 1);
      len -= n + 1;
      return line;
    }
    if (len == sizeof(buf)) len = 0; /* drop an oversized line */
    ssize_t n = read(STDIN_FILENO, buf + len, sizeof(buf) - len);
    if (n <= 0) return NULL;
    len += n;
  }
}

LRESULT filesystem_handle_event(lua_State *L, struct AXmessage *msg) {
  if (msg->message == kEventReadCommands) {
    LPSTR url = UI_ReadClientCommands();
    if (!url) exit(0);
    WITH(xmlDoc, doc, xmlNewDoc(XMLSTR("1.0")), xmlFreeDoc) {
      xmlNodePtr response = xmlNewNode(NULL, XMLSTR("response"));
      enum cmd cmd = cmd_GET;
      fprintf(stderr, "Requested URL: %s\n", url);
      FOR_LOOP(i, sizeof(cmdsprfx)/sizeof(*cmdsprfx)) {
        if (!strncmp(url, cmdsprfx[i], strlen(cmdsprfx[i]))) {
          url += strlen(cmdsprfx[i]);
          cmd = i;
        }
      }
      FOR_LOOP(i, sizeof(routes)/sizeof(*routes)) {
        if (routes[i].op!=cmd)continue;
        if (strncmp(url, routes[i].name, strlen(routes[i].name))) continue;
        if (strchr(url, '\n')) *strchr(url, '\n') = 0;
        reqArg_t rargs[MAX_ARGS]={0};
        lpcString_t endpoint = FS_ParseArgs(url+strlen(routes[i].name), rargs, MAX_ARGS);
        lpcString_t err = routes[i].func(L, response, endpoint, rargs, MAX_ARGS);
        if (err) {
          _xmlSetProp(response, "status", "error");
          _xmlNodeSetContent(response, err);
        } else {
          _xmlSetProp(response, "status", "success");
        }
        goto send_response;
      }
      _xmlSetProp(response, "status", "error");
      _xmlNodeSetContent(response, "Unknown command");
    send_response:
      if (!response->children) {
        xmlNodeSetContent(response, XMLSTR(" "));
      }
      xmlDocSetRootElement(doc, response);
      xmlDocDump(stdout, doc);
      fflush(stdout);
      //      xmlDocDump(stderr, doc);
    }
    axPostMessageW(msg->target, kEventReadCommands, 0, NULL);
    return TRUE;
  } else {
    return FALSE;
  }
}

