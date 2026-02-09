#include <arpa/inet.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

// #include <include/orca.h>
// #include <include/api.h>
#include <include/orcadef.h>

#ifdef __QNX__
#include "../source/lua-master/lauxlib.h"
#include "../source/lua-master/lua.h"
#include "../source/lua-master/lualib.h"
#elif __has_include(<lua5.4/lua.h>)
#include <lua5.4/lauxlib.h>
#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
#elif __has_include(<lua5.3/lua.h>)
#include <lua5.3/lauxlib.h>
#include <lua5.3/lua.h>
#include <lua5.3/lualib.h>
#elif __has_include(<lua/lua.h>)
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
#else
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#endif

#include <libxml/tree.h>

#include "someip.h"
#include "vsomeip.h"

#define SOCKET_PATH_3 "/tmp/vsomeip-1380"
#define CLIENT_ID 0x1380

void AcceptConnections(LPMYSERVICE);
void RegisterApplication(LPMYSERVICE);
void ReadClients(LPMYSERVICE);
int OpenListeningSocket(char const* szPath);
void
CloseListeningSocket(int socket);

struct _MYSERVICE d_service;

int read_thread = 0;

int f_updateTime(lua_State* L)
{
  if (d_service.registered) {
    AcceptConnections(&d_service);
    ReadClients(&d_service);
  } else {
    RegisterApplication(&d_service);
  }
  return 0;
}

int f_getCurrentWIM(lua_State* L)
{
  //	if (model->WarningsService.activeWarnings.count == 0) {
  //		lua_pushnil(L);
  //		return 1;
  //	} else {
  //		lua_pushinteger(L,
  // model->WarningsService.activeWarnings.items[0].id); 		return
  // 1;
  //	}
  return 0;
}

xmlDocPtr cardata = NULL;

xmlChar const*
xmlGetAttrTextQuick(xmlAttrPtr attr)
{
  if (attr && attr->children && attr->children->content) {
    return attr->children->content;
  } else {
    return (xmlChar const*)"";
  }
}
xmlChar const*
xmlGetPropQuick(xmlNodePtr node, xmlChar const* name)
{
  return xmlGetAttrTextQuick(xmlHasProp(node, BAD_CAST name));
}

static xmlNodePtr
_FindNode(xmlChar const* name)
{
  xmlForEach(node, xmlDocGetRootElement(cardata))
  {
    if (!xmlStrcmp(xmlGetPropQuick(node, XMLSTR("name")), name)) {
      return node;
    }
  }
  return NULL;
}

#define SET_NODE_TYPE(node, type)                                              \
  else if (!xmlStrcmp(node->name, BAD_CAST #type))                             \
  {                                                                            \
    _SETTYPE(node, type_##type);                                               \
  }

static void
_CompileNode(xmlNodePtr root)
{
  xmlNodePtr type = _FindNode(root->name);
  if (type) {
    if (!xmlStrcmp(type->name, XMLSTR("enum"))) {
      _SETTYPE(root, type_enum);
    } else if (!xmlStrcmp(type->name, XMLSTR("typedef"))) {
    } else if (type->children) {
      xmlForEach(field, type)
      {
        _CompileNode(xmlAddChild(root, xmlCopyNode(field, TRUE)));
      }
    } else {
      Con_Error("Weird type %s", (char const*)type->name);
    }
  } else if (!xmlStrcmp(root->name, XMLSTR("bool"))) {
    _SETTYPE(root, type_bool);
  }
  //	SET_NODE_TYPE(root, bool)
  SET_NODE_TYPE(root, int8)
  SET_NODE_TYPE(root, uint8)
  SET_NODE_TYPE(root, int16)
  SET_NODE_TYPE(root, uint16)
  SET_NODE_TYPE(root, int32)
  SET_NODE_TYPE(root, uint32)
  SET_NODE_TYPE(root, int64)
  SET_NODE_TYPE(root, uint64)
  else
  {
    //		Con_Error("Unknown type %s", (char const *)root->name);
  }
}

void
on_client_added(LPMYSERVICE hService, client_t client);
void
on_service_added(LPMYSERVICE hService, service_data_t* sv);

struct _xmlDoc* FS_LoadXML(char const*);

int new_SpeedTest(lua_State* L)
{
  LPMYSERVICE service = lua_newuserdata(L, sizeof(struct _MYSERVICE));
  luaL_setmetatable(L, "SpeedTest");
  memset(service, 0, sizeof(struct _MYSERVICE));
  service->on_client_added = on_client_added;
  service->on_service_added = on_service_added;
  service->fout = -1;
  service->ident = CLIENT_ID;
  service->fin = OpenListeningSocket(SOCKET_PATH_3);
  RegisterApplication(service);
  d_service = *service;

  char const* szFileName = luaL_checkstring(L, 1);
  if (cardata) {
    xmlFreeDoc(cardata);
    cardata = NULL;
  }
  if ((cardata = FS_LoadXML(szFileName))) {
    xmlForEach(service, xmlDocGetRootElement(cardata))
    if (!xmlStrcmp(service->name, XMLSTR("service"))) {
      _SETTYPE(service, type_service);
      _SETVALUE(service,
                atoi((char const*)xmlGetPropQuick(service, XMLSTR("id"))));
      xmlForEach(method, service)
      {
        _CompileNode(method);
        uint32_t id = atoi((char const*)xmlGetPropQuick(method, XMLSTR("id")));
        uint32_t group =
          atoi((char const*)xmlGetPropQuick(method, XMLSTR("group")));
        _SETVALUE(method, MAKELONG(id, group));
      }
      lua_pushlightuserdata(L, service);
      lua_setfield(L,
                   LUA_REGISTRYINDEX,
                   (char const*)xmlGetPropQuick(service, XMLSTR("name")));
    }
  } else {
    return luaL_error(L, "Can't find protocol file %s", szFileName);
  }

  int Read_Service(long dwService, long dwMethod, long dwLength, void* lpData);

  uint16_t data[3] = { 200, 1, 0 };
  Read_Service(5000, 32769, 6, data);

  return 1;
}

static int gc_SpeedTest(lua_State* L)
{
  LPMYSERVICE service = luaL_checkudata(L, 1, "SpeedTest");
  FOR_LOOP(client_id, service->numclients)
  {
    SafeDelete(service->clients[client_id], close);
  }
  SafeDelete(service->fout, close);
  SafeDelete(service->fin, CloseListeningSocket);
  SafeDelete(cardata, xmlFreeDoc);
  return 0;
}

static int f_index(lua_State* L)
{
  xmlNodePtr service = _FindNode((xmlChar const*)luaL_checkstring(L, 2));
  if (_CHECKTYPE(service, type_service)) {
    xmlNodePtr* node = lua_newuserdata(L, sizeof(xmlNodePtr));
    luaL_setmetatable(L, "DataNode");
    *node = service;
    return 1;
  } else {
    return 0;
  }
}

static int push_enum(lua_State* L, xmlNodePtr field)
{
  xmlNodePtr type = _FindNode(field->name);
  if (type) {
    uint32_t value = *(uint32_t*)&field->_private;
    FOR_EACH_LIST(xmlAttr, attr, type->properties) {
      if (!xmlStrcmp(attr->name, BAD_CAST"name"))
        continue;
      if (value == atoi((char const*)xmlGetAttrTextQuick(attr))) {
        lua_pushstring(L, (char const*)attr->name);
        return 1;
      }
//    xmlForEach(item, type)
//    {
//      if (value == atoi((char const*)xmlGetPropQuick((xmlNodePtr)item, XMLSTR("value")))) {
//        lua_pushstring(L, (char const*)xmlGetPropQuick((xmlNodePtr)item, XMLSTR("name")));
//        return 1;
//      }
    }
  }
  lua_pushnil(L);
  return 1;
}

#define PUSH_ATOM(type)                                                        \
  else if (_CHECKTYPE(field, type_##type))                                     \
  {                                                                            \
    type##_t value = *(type##_t*)&field->_private;                             \
    lua_pushinteger(L, value);                                                 \
    return 1;                                                                  \
  }

static int f_datanode_index(lua_State* L)
{
  xmlNodePtr* self = luaL_checkudata(L, 1, "DataNode");
  xmlChar const* name = (xmlChar const*)luaL_checkstring(L, 2);
  xmlForEach(field, *self)
  {
    if (xmlStrcmp(xmlGetPropQuick(field, XMLSTR("name")), name)) {
      // pass
    } else if (field->children) {
      xmlNodePtr* node = lua_newuserdata(L, sizeof(xmlNodePtr));
      luaL_setmetatable(L, "DataNode");
      *node = field;
      return 1;
    } else if (_CHECKTYPE(field, type_enum)) {
      push_enum(L, field);
      return 1;
    } else if (_CHECKTYPE(field, type_bool)) {
      lua_pushboolean(L, _GETVALUE(field) != 0);
      return 1;
    }
    PUSH_ATOM(int8)
    PUSH_ATOM(uint8)
    PUSH_ATOM(int16)
    PUSH_ATOM(uint16)
    PUSH_ATOM(int32)
    PUSH_ATOM(uint32)
    PUSH_ATOM(int64)
    PUSH_ATOM(uint64)
    else
    {
      return 0;
    }
  }
  return 0;
}

static int f_datanode_tostring(lua_State* L)
{
  xmlNodePtr* self = luaL_checkudata(L, 1, "DataNode");
  xmlChar const* name = xmlGetPropQuick(*self, XMLSTR("name"));
  lua_pushfstring(L, "DataNode [%s %s]", (*self)->name, name);
  return 1;
}

ORCA_API int luaopen_dataplugin_DataNode(lua_State* L)
{
  luaL_newmetatable(L, "DataNode");
  luaL_setfuncs(L,
                (luaL_Reg[]){
                  { "__index", f_datanode_index },
                  { "__tostring", f_datanode_tostring },
                  { 0, 0 },
                },
                0);
  return 1;
}

ORCA_API int luaopen_dataplugin_SpeedTest(lua_State* L)
{
  luaL_newmetatable(L, "SpeedTest");
  luaL_setfuncs(L,
                (luaL_Reg[]){
                  { "new", new_SpeedTest },
                  { "__gc", gc_SpeedTest },
                  { "__index", f_index },
                  { 0, 0 },
                },
                0);
  return 1;
}

ORCA_API int luaopen_orca_vsomeip(lua_State* L)
{
  luaL_newlib(L,
              ((luaL_Reg[]){
                { "updateTime", f_updateTime },
                { "getCurrentWIM", f_getCurrentWIM },
                { 0, 0 },
              }));
  luaopen_dataplugin_SpeedTest(L);
  lua_setfield(L, -2, "SpeedTest");
  luaopen_dataplugin_DataNode(L);
  lua_setfield(L, -2, "DataNode");
  return 1;
}
