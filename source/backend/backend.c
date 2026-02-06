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

ORCA_API int luaopen_orca_backend(lua_State* L)
{
  luaL_newlib(L,
              ((luaL_Reg[]){ { "getMessage", f_peek_message },
                             { "dispatchMessage", f_dispatch_message },
                             { NULL, NULL } }));

  luaL_newmetatable(L, "Event");
  luaL_setfuncs(
    L, ((luaL_Reg[]){ { "__index", f_event_index }, { NULL, NULL } }), 0);
  lua_setfield(L, -2, "Event");

  return 1;
}
