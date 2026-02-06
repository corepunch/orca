#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

static int f_getTime(lua_State* L)
{
  lua_pushinteger(L, WI_GetMilliseconds());
  return 1;
}

static int f_sleep(lua_State* L)
{
  WI_Sleep(luaL_checkinteger(L, 1));
  return 0;
}

static int f_get_clipboard(lua_State* L)
{
  //  LPSTR  text = SDL_GetClipboardText();
  //  if (!text) { return 0; }
  //  lua_pushstring(L, text);
  //  SDL_free(text);
  //  return 1;
  return 0;
}

static int f_set_clipboard(lua_State* L)
{
  //  lpcString_t  text = luaL_checkstring(L, 1);
  //  SDL_SetClipboardText(text);
  return 0;
}

static int f_get_time_span(lua_State* L)
{
  static longTime_t old = 0;
  lua_pushnumber(L, (WI_GetMilliseconds() - old) / 1000.f);
  old = WI_GetMilliseconds();
  return 1;
}

static bool_t dark_theme = FALSE;

bool_t
WI_IsDarkTheme(void)
{
  return dark_theme;
}

static int f_get_theme(lua_State* L)
{
  lua_pushboolean(L, dark_theme);
  return 1;
}

static int f_set_theme(lua_State* L)
{
  dark_theme = lua_toboolean(L, 1);
  return 1;
}

static luaL_Reg const lib_system[] = { { "getTimeSpan", f_get_time_span },
                                       { "getTime", f_getTime },
                                       { "sleep", f_sleep },
                                       { "set_theme", f_set_theme },
                                       { "get_theme", f_get_theme },
                                       { "get_glipboard", f_get_clipboard },
                                       { "set_clipboard", f_set_clipboard },
                                       { NULL, NULL } };

ORCA_API int luaopen_orca_system(lua_State* L)
{
  //	lua_pushlightuserdata(L, &api_system);
  //	lua_setfield(L, LUA_REGISTRYINDEX, IID_SYSTEM);
  luaL_newlib(L, lib_system);

  lua_pushstring(L, WI_GetPlatform());
  lua_setglobal(L, "PLATFORM");

  lua_pushstring(L, WI_SettingsDirectory());
  lua_setglobal(L, "SETTINGS");

  return 1;
}
