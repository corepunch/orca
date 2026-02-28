#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

//#include <lua.h>
//#include <lauxlib.h>
//#include <string.h>


#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <errno.h>
#endif

/* ── userdata types ─────────────────────────────────────────────────────── */

#ifdef _WIN32
typedef struct {
  HANDLE        hFind;
  WIN32_FIND_DATAA fd;
  int           first;   /* 1 = FindFirstFile already populated fd */
} DirIter;
#else
typedef struct {
  DIR *dir;
} DirIter;
#endif

/* ── __gc metamethod – closes the handle when GC'd or loop ends early ───── */

static int iter_gc(lua_State *L) {
#ifdef _WIN32
  DirIter *it = (DirIter *)lua_touserdata(L, 1);
  if (it->hFind != INVALID_HANDLE_VALUE) {
    FindClose(it->hFind);
    it->hFind = INVALID_HANDLE_VALUE;
  }
#else
  DirIter *it = (DirIter *)lua_touserdata(L, 1);
  if (it->dir) {
    closedir(it->dir);
    it->dir = NULL;
  }
#endif
  return 0;
}

/* ── iterator function (the closure called each iteration) ──────────────── */

static int iter_next(lua_State *L) {
  DirIter *it = (DirIter *)lua_touserdata(L, lua_upvalueindex(1));
  
#ifdef _WIN32
  if (it->hFind == INVALID_HANDLE_VALUE)
    return 0; /* exhausted */
  
  for (;;) {
    const char *name;
    if (it->first) {
      it->first = 0;
      name = it->fd.cFileName;
    } else {
      if (!FindNextFileA(it->hFind, &it->fd)) {
        FindClose(it->hFind);
        it->hFind = INVALID_HANDLE_VALUE;
        return 0;
      }
      name = it->fd.cFileName;
    }
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
      continue;
    lua_pushstring(L, name);
    return 1;
  }
  
#else
  if (!it->dir)
    return 0; /* exhausted */
  
  for (;;) {
    struct dirent *entry = readdir(it->dir);
    if (!entry) {
      closedir(it->dir);
      it->dir = NULL;
      return 0;
    }
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;
    lua_pushstring(L, entry->d_name);
    return 1;
  }
#endif
}

/* ── system.list_dir(path) ──────────────────────────────────────────────── */

static int f_list_dir(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  
  DirIter *it = (DirIter *)lua_newuserdata(L, sizeof(DirIter));
  
  /* attach __gc so the handle is always cleaned up */
  if (luaL_newmetatable(L, "DirIter")) {
    lua_pushcfunction(L, iter_gc);
    lua_setfield(L, -2, "__gc");
  }
  lua_setmetatable(L, -2);
  
#ifdef _WIN32
  char search[MAX_PATH];
  snprintf(search, sizeof(search), "%s\\*", path);
  it->hFind = FindFirstFileA(search, &it->fd);
  it->first = 1;
  if (it->hFind == INVALID_HANDLE_VALUE)
    return luaL_error(L, "cannot open directory '%s'", path);
#else
  it->dir = opendir(path);
  if (!it->dir)
    return luaL_error(L, "cannot open directory '%s': %s", path, strerror(errno));
#endif
  
  /* return iterator closure with the userdata as upvalue */
  lua_pushcclosure(L, iter_next, 1);
  return 1;
}

// Timer functions

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

const char *themes[] = { "light", "dark", NULL };

static int f_set_theme(lua_State* L)
{
  dark_theme = luaL_checkoption(L, 1, NULL, themes);
  return 1;
}

static luaL_Reg const lib_system[] = {
  { "getTimeSpan", f_get_time_span },
  { "getTime", f_getTime },
  { "sleep", f_sleep },
  { "set_theme", f_set_theme },
  { "get_theme", f_get_theme },
  { "get_glipboard", f_get_clipboard },
  { "set_clipboard", f_set_clipboard },
  { "list_dir", f_list_dir },
  { NULL, NULL }
};

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
