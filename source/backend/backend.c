#include <include/api.h>
#include <include/orca.h>

int f_peek_message(lua_State* L);

int f_dispatch_message(lua_State* L);

//int //f_table_Window(lua_State* L);

int f_event_message(lua_State* L)
{
  struct WI_Message const* msg = luaL_checkudata(L, 1, "Event");
  lua_pushinteger(L, msg->message);
  return 1;
}

int f_event_is(lua_State* L)
{
  struct WI_Message const* msg = luaL_checkudata(L, 1, "Event");
  lpcString_t name = luaL_checkstring(L, 2);
  lua_pushboolean(L, msg->message == fnv1a32(name));
  return 1;
}

int f_event_index(lua_State* L)
{
  struct WI_Message const* msg = luaL_checkudata(L, 1, "Event");
  lpcString_t name = luaL_checkstring(L, 2);
  if (!strcmp(name, "is")) {
    lua_pushcfunction(L, f_event_is);
    return 1;
  }
  if (!strcmp(name, "message")) {
    lua_pushinteger(L, msg->message);
    return 1;
  }
  if (!strcmp(name, "wparam")) {
    lua_pushinteger(L, msg->wParam);
    return 1;
  }
  if (!strcmp(name, "x")) {
    lua_pushinteger(L, LOWORD(msg->wParam));
    return 1;
  }
  if (!strcmp(name, "y")) {
    lua_pushinteger(L, HIWORD(msg->wParam));
    return 1;
  }
  if (!strcmp(name, "dx")) {
    lua_pushinteger(L, LOWORD((intptr_t)msg->lParam));
    return 1;
  }
  if (!strcmp(name, "dy")) {
    lua_pushinteger(L, HIWORD((intptr_t)msg->lParam));
    return 1;
  }
  if (!strcmp(name, "key")) {
    lua_pushinteger(L, LOWORD(msg->wParam));
    return 1;
  }
  if (!strcmp(name, "modflags")) {
    lua_pushinteger(L, HIWORD(msg->wParam));
    return 1;
  }
  return luaL_error(L, "No field %s in Event", name);
}

static lpObject_t f_checkObject(lua_State* L, int arg) {
  if (lua_type(L, arg) == LUA_TTABLE) {
    luaX_parsefield(lpObject_t, __userdata, arg, luaL_checkudata, API_TYPE_OBJECT);
    return __userdata;
  } else {
    return luaL_checkudata(L, arg, API_TYPE_OBJECT);
  }
}

int f_event_new(lua_State* L) {
  struct WI_Message* msg = lua_newuserdata(L, sizeof(struct WI_Message));
  luaL_setmetatable(L, "Event");
  if (lua_type(L, 1) == LUA_TTABLE) {
    lua_pushnil(L);  // first key
    while (lua_next(L, 1) != 0) {
      if (lua_type(L, -2) == LUA_TSTRING) {
        const char* key = lua_tostring(L, -2);
        if (strcmp(key, "target") == 0) {
          msg->target = f_checkObject(L, -1);
        } else if (strcmp(key, "message") == 0) {
          msg->message = fnv1a32(luaL_checkstring(L, -1));
        } else if (strcmp(key, "x") == 0) {
          msg->x = luaL_checknumber(L, -1);
        } else if (strcmp(key, "y") == 0) {
          msg->y = luaL_checknumber(L, -1);
        } else if (strcmp(key, "async") == 0) {
          msg->syncronous = !lua_toboolean(L, -1);
        }
      }
      lua_pop(L, 1); // remove value, keep key for next iteration
    }
  }
  return 1;
}

ORCA_API int luaopen_orca_backend(lua_State* L)
{
  luaL_newlib(L,
              ((luaL_Reg[]){ { "getMessage", f_peek_message },
                             { "dispatchMessage", f_dispatch_message },
                             { NULL, NULL } }));

  luaL_newmetatable(L, "Event");
  lua_pushcfunction(L, f_event_index);
  lua_setfield(L, -2, "__index");
//  lua_setfield(L, -2, "Event");
  lua_pop(L, 1);
  
  lua_pushcfunction(L, f_event_new);
  lua_setfield(L, -2, "Event");

  return 1;
}
