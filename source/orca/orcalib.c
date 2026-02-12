#ifdef __QNX__
#define LUA_USE_LINUX
#define l_likely(x) (x)
#define l_unlikely(x) (x)
#include "lua-master/onelua.c"
#endif

#include <include/api.h>
#include <include/version.h>

int luaL_preload(lua_State* L, lpcString_t name, lua_CFunction f) {
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "preload");
  lua_pushcfunction(L, f);
  lua_setfield(L, -2, name);
  lua_pop(L, 2);
  return 0;
}

int luaopen_orca_core(lua_State*);
int luaopen_orca_filesystem(lua_State*);
int luaopen_orca_filesystem_native(lua_State*);
int luaopen_orca_geometry(lua_State*);
int luaopen_orca_localization(lua_State*);
int luaopen_orca_network(lua_State*);
int luaopen_orca_parsers_css(lua_State*);
int luaopen_orca_parsers_json(lua_State*);
int luaopen_orca_parsers_xml(lua_State*);
int luaopen_orca_renderer(lua_State*);
int luaopen_orca_SceneKit(lua_State*);
int luaopen_orca_backend(lua_State*);
int luaopen_orca_system(lua_State*);
int luaopen_orca_UIKit(lua_State*);
int luaopen_orca_vsomeip(lua_State*);
int luaopen_orca_editor(lua_State*);

static luaL_Reg const orca_modules[] = {
  { "orca.core", luaopen_orca_core },
  { "orca.filesystem", luaopen_orca_filesystem },
  { "orca.filesystem.native", luaopen_orca_filesystem_native },
  { "orca.geometry", luaopen_orca_geometry },
  { "orca.localization", luaopen_orca_localization },
  { "orca.network", luaopen_orca_network },
  { "orca.parsers.css", luaopen_orca_parsers_css },
  { "orca.parsers.json", luaopen_orca_parsers_json },
  { "orca.parsers.xml", luaopen_orca_parsers_xml },
  { "orca.renderer", luaopen_orca_renderer },
  { "orca.SceneKit", luaopen_orca_SceneKit },
  { "orca.backend", luaopen_orca_backend },
  { "orca.system", luaopen_orca_system },
  { "orca.ui", luaopen_orca_UIKit },
  { "orca.vsomeip", luaopen_orca_vsomeip },
  { "orca.editor", luaopen_orca_editor },
  { NULL, NULL }
};

int f_registerEngineClass(lua_State *L) {
  lua_pushvalue(L, 2);
  lua_setfield(L, LUA_REGISTRYINDEX, luaL_checkstring(L, 1));
  return 0;
}

static int f_async(lua_State* L) {
  const int nargs = lua_gettop(L);
  lua_State* co = lua_newthread(L);
  int ref = luaL_ref(L, LUA_REGISTRYINDEX);
  lua_xmove(L, co, nargs);
  WI_PostMessageW(co, kEventResumeCoroutine, nargs-1, (void*)(intptr_t)ref);
  return 0;
}

ORCA_API int luaopen_orca(lua_State* L)
{
  for (luaL_Reg const* fn = orca_modules; fn->name; fn++) {
    luaL_preload(L, fn->name, fn->func);
  }

  luaL_newlib(L, ((luaL_Reg[]){
    { "registerEngineClass", f_registerEngineClass },
    { NULL, NULL }
  }));

  lua_pushstring(L, ORCA_VERSION);
  lua_setfield(L, -2, "version");

  lua_pushstring(L, __DATE__);
  lua_setfield(L, -2, "build");

  lua_pushcfunction(L, f_async);
  lua_setfield(L, -2, "async");
  
  return 1;
}
