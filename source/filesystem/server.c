#include <unistd.h>

#include "fs_local.h"
#include "../core/core_local.h"

#include <source/UIKit/UIKit.h>

#define REQUIRE(type, name, expr, error) type name = expr; if (!name) return error;
#define ERROR_CANT_FIND_OBJECT "Can't find object"
#define SV_ARG(NAME) rargs_input(rargs, nargs, NAME)
#define MAX_ARGS 16
lpcString_t FS_ParseArgs(LPSTR s, reqArg_t *args, size_t maxargs) {
  char *q=strchr(s,'?'); if(!q) return s;
  *q++=0;
  reqArg_t *r=args;
  for(char *tok=strtok(q,"&"); tok && maxargs--; tok=strtok(NULL,"&"),r++){
    char *eq=strchr(tok,'=');
    if(eq){ *eq=0; strncpy(r->name,tok,sizeof r->name-1); strncpy(r->value,eq+1,sizeof r->value-1);}
    else { strncpy(r->name,tok,sizeof r->name-1); *r->value=0; }
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

bool_t OBJ_IsPrefabView(lpcObject_t object);

static int print_classes(lpcClassDesc_t cd, LPSTR s, int n) {
  if (!cd || n <= 0) return 0;
  int w = snprintf(s, n, "%s,", cd->ClassName);
  n -= w;
  for (lpcClassDesc_t const *p = cd->ParentClasses; *p && n > 1; p++)
    w += print_classes(*p, s + w, n - w);
  return w;
}

static void object_hierarchy(lpObject_t object, xmlNodePtr parent) {
  lpcClassDesc_t pClass = OBJ_FindClass(OBJ_GetClassName(object));
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
  lpObject_t object = OBJ_FindByPath(FS_GetWorkspace(), endpoint);
  _xmlSetProp(response, "source", endpoint);
  if (object) {
    object_hierarchy(object, response);
    return NULL;
  } else {
    return ERROR_CANT_FIND_OBJECT;
  }
}

SV_CMD(GET, project_overview);

lpObject_t 
UI_FindObjectByUniqueID(uint32_t uid, lpObject_t object);

static lpcString_t _types[] = {
  "none", // kDataTypeNone,
  "bool", // kDataTypeBool,
  "int", // kDataTypeInt,
  "enum", // kDataTypeEnum,
  "float", // kDataTypeFloat,
  "vector2", // kDataTypeVector2D,
  "vector3", // kDataTypeVector3D,
  "vector4", // kDataTypeVector4D,
  "matrix4", // kDataTypeMatrix3D,
  "transform2", // kDataTypeTransform2D,
  "transform3", // kDataTypeTransform3D,
  "string", // kDataTypeFixed,
  "longstring", // kDataTypeLongString,
  "color", // kDataTypeColor,
  "edges", // kDataTypeEdges,
  "objecttags", // kDataTypeObjectTags,
  "event", // kDataTypeEvent,
  "project-item", // kDataTypeObject,
  "group", // kDataTypeGroup,
};

static void add_group(lpcClassDesc_t dec, void* parm) {
  xmlNodePtr p = _xmlNewChild(parm, "group", "name", dec->ClassName);
  _xmlSetProp(p, "data-compound", "true");
}

ORCA_API lpcString_t
PDESC_Print(lpcPropertyDesc_t pdesc, LPSTR buffer, uint32_t len, float const* pf);

#define _xmlAddProp(parent, name, value, type)\
_xmlNewChild(parent, _types[type], "name", name, "value", value, "data-type", _types[type])

static int
add_subproperty(xmlNodePtr xml,
                lpcObject_t obj,
                lpcPropertyDesc_t pdesc,
                void const* dest)
{
  path_t buf;
  PDESC_Print(pdesc, buf, sizeof(buf), dest);
  xmlNodePtr n = _xmlAddProp(xml, pdesc->id->Name, buf, pdesc->DataType);
  if (PROP_FindByLongID(OBJ_GetProperties(obj), pdesc->FullIdentifier)) {
    _xmlSetProp(n, "data-set", "true");
  }
  switch (pdesc->DataType) {
    case kDataTypeObject:
      _xmlSetProp(n, "data-content", pdesc->TypeString);
      break;
    case kDataTypeEnum:
      xmlWith(char, s, strdup(pdesc->TypeString), free) {
        for (LPSTR t = strtok(s,","); t; t = strtok(NULL, ",")) {
          _xmlNewChild(n, "menu-item", "header", t, "name", t);
  //          xmlNewChild(n, NULL, XMLSTR("value"), XMLSTR(t));
        }
      }
      break;
    case kDataTypeGroup:
      _xmlSetProp(n, "data-compound", "true");
      FOR_LOOP(i, pdesc->NumComponents) {
        lpcPropertyDesc_t inner = &pdesc[i + 1];
        void* dest2 = ((byte_t*)dest) + inner->Offset - pdesc->Offset;
        i += add_subproperty(n, obj, inner, dest2);
      }
      return pdesc->NumComponents;
    default:
      break;
  }
  return 0;
}

static xmlNodePtr
xmlFindNode(xmlNodePtr node, xmlChar const *name) {
  xmlForEach(child, node) {
    if (xmlStrcmp(child->name, XMLSTR("group"))) continue;
    xmlWith(xmlChar, _name, xmlGetProp(child, XMLSTR("name")), xmlFree) {
      if (xmlStrcmp(_name, name)) continue;
      return child;
    }
  }
  return NULL;
}

#define DEFAULT_GROUP "Object"
#define CUSTOM_GROUP "Custom"

static void
add_property(lpcObject_t obj,
             lpcPropertyDesc_t pdesc,
             lpcClassDesc_t cdesc,
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
  REQUIRE(lpObject_t, object, OBJ_FindByPath(FS_GetWorkspace(), endpoint), ERROR_CANT_FIND_OBJECT);
  xmlNodePtr _g = _xmlNewChild(response, "group", "name", DEFAULT_GROUP, "data-compound", "true");
//  xmlNodePtr _c = _xmlAddProp(_g, "Class", OBJ_GetClassName(object), kDataTypeFixed);
//  _xmlSetProp(_c, "data-readonly", "true");
  if (GetNode(object)) {
    _xmlSetProp(response, "QuickHide", GetNode(object)->QuickHide?"true":"false");
  }
  if (OBJ_GetSourceFile(object)) {
    xmlNodePtr _g = _xmlNewChild(response, "group", "name", "PrefabTemplate", "data-readonly", "true");
    _xmlAddProp(_g, "Template", OBJ_GetSourceFile(object), kDataTypeFixed);
  }
  _xmlSetProp(response, "source", endpoint);
  _xmlAddProp(_g, "Name", OBJ_GetName(object), kDataTypeFixed);

  OBJ_EnumObjectClasses(object, add_group, response);
  OBJ_EnumClassProperties(object, add_property, response);
  
  for (lpcProperty_t p = OBJ_GetProperties(object); p; p = PROP_GetNext(p)) {
    if (PROP_GetFlags(p)&PF_PROPERTY_TYPE)
      continue;
    xmlNodePtr group = xmlFindNode(response, XMLSTR(CUSTOM_GROUP));
    if (!group) {
      group = _xmlNewChild(response, "group", "name", CUSTOM_GROUP, "data-compound", "true");
    }
    add_subproperty(group, object, &(struct PropertyDesc) {
      .id=&(struct ID){.Name = PROP_GetName(p)},
      .DataType = PROP_GetType(p),
      .TypeString = PROP_GetUserData(p),
    }, PROP_GetValue(p));
  }

  return NULL;
}

SV_CMD(PUT, node)
{
  REQUIRE(lpObject_t, obj, OBJ_FindByPath(FS_GetWorkspace(), endpoint), ERROR_CANT_FIND_OBJECT);
  _xmlSetProp(response, "command", "edit");
  _xmlSetProp(response, "source", endpoint);
  for(reqArg_t const* arg = rargs; *arg->name; arg++) {
    if (!strcmp(arg->name, "Name")) {
      _xmlAddProp(response, arg->name, OBJ_GetName(obj), kDataTypeFixed);
      OBJ_SetName(obj, arg->value);
    } else{
      path_t buf={0};
      lpProperty_t property = NULL;
      if (FAILED(OBJ_FindShortProperty(obj, arg->name, &property))) continue;
      PROP_Print(property, buf, sizeof(buf));
      _xmlAddProp(response, arg->name, buf, PROP_GetType(property));
      PROP_Parse(property, arg->value);
    }
  }
  return NULL;;
}
  
SV_CMD(GET, startup_screen) {
  lpObject_t project = OBJ_GetFirstChild(FS_GetWorkspace());
  if (project && GetProject(project) && *GetProject(project)->StartupScreen) {
    GET_scene_hierarchy(L, response, GetProject(project)->StartupScreen, rargs, nargs);
    return NULL;
  } else {
    return "No startup screen specified in project or no project opened";
  }
}

ORCA_API void
FS_EnumDir2(lpcString_t, void (*callback)(lpcString_t,void*), void*);

static void load_library(lpcString_t def, void* param) {
  lua_State *L = ((void**)param)[0];
  lpcString_t path = ((void**)param)[1];
  if (strrchr(def, '.') && !strcasecmp(strrchr(def, '.'), ".xml")) {
    *strrchr(def, '.') = 0;
    char buf[512];
    snprintf(buf,sizeof(buf),"local r = require('orca.filesystem').getWorkspace():findByPath('%s/%s') or require('%s/%s')()",path,def,path,def);
    if (luaL_dostring(L, buf) != LUA_OK) {
      Con_Error("%s", luaL_checkstring(L, -1));
      lua_pop(L, 1);
    }
  }
}

SV_CMD(GET, project_overview) {
  lpObject_t proj = OBJ_GetFirstChild(FS_GetWorkspace());
  if (proj) {
    lpcString_t szRoot = OBJ_GetSourceFile(proj);
    FOR_EACH_OBJECT(lib, proj) {
      xmlWith(char, path, strdup(FS_JoinPaths(szRoot, OBJ_GetName(lib))), free) {
        char buf[256];
        snprintf(buf,sizeof(buf),"%s/%s",OBJ_GetName(proj),OBJ_GetName(lib));
        FS_EnumDir2(path, load_library, ((void*[]){L, buf}));
      }
    }
    GET_scene_hierarchy(L, response, OBJ_GetName(proj), rargs, nargs);
    return NULL;
  } else {
    return "No project opened";
  }
}

SV_CMD(GET, whoami) {
  lpObject_t project = OBJ_GetFirstChild(FS_GetWorkspace());
  if (project) {
    GET_node(L, response, OBJ_GetName(project), rargs, nargs);
    return NULL;
  } else {
    return "No project opened";
  }
}

extern lpObject_t (*_OBJ_LoadDocument)(lua_State* L, xmlDocPtr doc);

ORCA_API bool_t OBJ_SaveDocument(lpObject_t object);
ORCA_API xmlNodePtr ED_ConvertNode(lpObject_t object, xmlNodePtr parent);
SV_CMD(POST, node) {
  REQUIRE(lpObject_t, root, OBJ_FindByPath(FS_GetWorkspace(), endpoint), ERROR_CANT_FIND_OBJECT);
  if (SV_ARG("source")) {
    REQUIRE(lpObject_t, source, OBJ_FindByPath(FS_GetWorkspace(), SV_ARG("source")), "Can't find specified source object");
    if (OBJ_GetSourceFile(source)) return "Object is already a prefab";
    xmlWith(xmlDoc, doc, xmlNewDoc(XMLSTR("1.0")), xmlFree) {
      xmlDocSetRootElement(doc, ED_ConvertNode(source, NULL));
      OBJ_AddChild(root, _OBJ_LoadDocument(L, doc), FALSE);
    }
    OBJ_SetSourceFile(source, FS_JoinPaths(endpoint, OBJ_GetName(source)));
    return NULL;
  }
  REQUIRE(lpcString_t, cls, SV_ARG("class"), "Should provide 'class' or 'source' argument");
  lua_getfield(L, LUA_REGISTRYINDEX, cls);
  if (lua_isnil(L, -1)) {
    return "Can't find specified class";
  } else if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
    return luaL_checkstring(L, -1);
  } else {
    lpObject_t object = luaX_checkObject(L, -1);
    if (SV_ARG("name")) OBJ_SetName(object, SV_ARG("name"));
    OBJ_AddChild(root, object, FALSE);
    lua_pop(L, 1);
    return NULL;
  }
}

SV_CMD(DELETE, node) {
  REQUIRE(lpObject_t, object, OBJ_FindByPath(FS_GetWorkspace(), endpoint), ERROR_CANT_FIND_OBJECT);
  uint32_t index = 0;
  FOR_EACH_OBJECT(child, object) {
    if (child != object) index++;
    else break;
  }
  xmlAddChild(response, ED_ConvertNode(object, NULL));
  _xmlSetPropFormat(response, "index", "%d", index);
  _xmlSetProp(response, "command", "create");
  OBJ_RemoveFromParent(L, object, FALSE);
  return NULL;
}

extern uint32_t _IOSurface;

SV_CMD(GET, preview) {
  xmlNodePtr n = xmlNewChild(response, NULL, XMLSTR("IOSurfaceTexture"), NULL);
  _xmlSetPropFormat(n, "IOSurface", "%d", _IOSurface);
  return NULL;
}

static void output_class(lpcClassDesc_t desc, void *param) {
  _xmlNewChild(param, "menu-item", "header", desc->ClassName, "name", desc->ClassName);
}

SV_CMD(GET, class) {
  OBJ_EnumClasses(OBJ_FindClass(endpoint), output_class, response);
  return NULL;
}

static void output_property(lpcClassDesc_t desc, void *param) {
  xmlNodePtr n = _xmlNewChild(param, "menu-item", "header", desc->ClassName);
  FOR_LOOP(i, desc->NumProperties) {
    char buf[256];
    snprintf(buf, sizeof(buf), "%s.%s", desc->ClassName, desc->Properties[i].id->Name);
    _xmlNewChild(n, "menu-item", "header", desc->Properties[i].id->Name, "name", buf);
  }
}

SV_CMD(GET, property_types) {
  REQUIRE(lpObject_t, object, OBJ_FindByPath(FS_GetWorkspace(), endpoint), ERROR_CANT_FIND_OBJECT);
  OBJ_EnumObjectClasses(object, output_property, response);
  return NULL;
}

SV_CMD(POST, property_types) {
  REQUIRE(lpcString_t, classname, SV_ARG("name"), "Missing `name` argument");
  REQUIRE(lpObject_t, object, OBJ_FindByPath(FS_GetWorkspace(), endpoint), ERROR_CANT_FIND_OBJECT);
  lpProperty_t property;
  if (FAILED(OBJ_FindLongProperty(object, classname, &property))) {
    return "Can't create property";
  }
  return NULL;
}

SV_CMD(POST, save_project) {
  lpObject_t project = OBJ_GetFirstChild(FS_GetWorkspace());
  OBJ_SaveDocument(project);
  return NULL;
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
  { "scene-hierarchy", ROUTE(GET, scene_hierarchy) },
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
};

LPSTR UI_ReadClientCommands(void) {
  static char buf[256]={0};
  size_t total = 0;
  ssize_t n = read(STDIN_FILENO, buf + total, sizeof(buf));
  if (n == 0)
    return NULL;
  total += n;
  buf[n] = '\0';
  //  Con_Error("echo: %.*s", (int)total-1, buf);
  return strtok(buf, "\n");
}

int filesystem_handle_event(lua_State *L, struct WI_Message *msg) {
  if (msg->message == kEventReadCommands) {
    LPSTR url = UI_ReadClientCommands();
    if (!url) exit(0);
    xmlWith(xmlDoc, doc, xmlNewDoc(XMLSTR("1.0")), xmlFreeDoc) {
      xmlNodePtr response = xmlNewNode(NULL, XMLSTR("response"));
      enum cmd cmd = cmd_GET;
      fprintf(stderr, "%s\n", url);
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
      //      xmlDocDump(stderr, doc);
    }
    WI_PostMessageW(msg->target, kEventReadCommands, 0, NULL);
    return TRUE;
  } else {
    return FALSE;
  }
}
