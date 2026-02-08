#ifdef __QNX__
#define LUA_USE_LINUX
#define l_likely(x) (x)
#define l_unlikely(x) (x)
#include "lua-master/onelua.c"
#endif

#include <include/api.h>
#include <include/version.h>
#include <source/core/core.h>

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
#ifdef ORCA_FEATURE_3D
  { "orca.SceneKit", luaopen_orca_SceneKit },
#endif
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

static int next_coroutine_id = 1;

int f_async(lua_State *L) {
  // Get task function and arguments
  luaL_checktype(L, 1, LUA_TFUNCTION);
  
  // Create coroutine
  lua_State* co = lua_newthread(L);
  int thread_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  
  // Push task to coroutine
  lua_pushvalue(L, 1);
  lua_xmove(L, co, 1);
  
  // Copy arguments to coroutine
  int nargs = lua_gettop(L) - 1;
  if (nargs > 0) {
    for (int i = 2; i <= lua_gettop(L); i++) {
      lua_pushvalue(L, i);
    }
    lua_xmove(L, co, nargs);
  }
  
  // Get handler_id
  int handler_id = next_coroutine_id++;
  
  // Create resume_handler closure
  lua_newtable(L);  // handler table
  lua_pushinteger(L, thread_ref);
  lua_setfield(L, -2, "thread_ref");
  lua_pushinteger(L, nargs);
  lua_setfield(L, -2, "nargs");
  lua_pushboolean(L, 1); // first_call
  lua_setfield(L, -2, "first_call");
  
  lua_pushvalue(L, -1); // duplicate handler table
  
  // Create resume_handler function
  luaL_dostring(L,
    "local handler_table = ...\n"
    "return function()\n"
    "  local thread_ref = handler_table.thread_ref\n"
    "  local co = debug.getregistry()[thread_ref]\n"
    "  if not co then return false end\n"
    "  \n"
    "  local ok, result = coroutine.resume(co)\n"
    "  if not ok then\n"
    "    print('Coroutine error:', result)\n"
    "    return false\n"
    "  end\n"
    "  \n"
    "  if coroutine.status(co) ~= 'dead' then\n"
    "    return true\n"
    "  else\n"
    "    debug.getregistry()[thread_ref] = nil\n"
    "    return false\n"
    "  end\n"
    "end\n"
  );
  
  lua_insert(L, -2); // move function before handler table
  lua_call(L, 1, 1); // call with handler table, returns resume_handler
  
  // Store handler in registry
  lua_getfield(L, LUA_REGISTRYINDEX, "__coroutine_handlers");
  lua_pushinteger(L, handler_id);
  lua_pushvalue(L, -3); // resume_handler
  lua_settable(L, -3);
  lua_pop(L, 1); // pop handlers table
  
  // Post initial kEventCoroutineResume
  WI_PostMessageW(NULL, kEventCoroutineResume, handler_id, NULL);
  
  // Return handler info
  lua_newtable(L);
  lua_pushinteger(L, handler_id);
  lua_setfield(L, -2, "id");
  lua_rawgeti(L, LUA_REGISTRYINDEX, thread_ref);
  lua_setfield(L, -2, "coroutine");
  
  return 1;
}

ORCA_API int luaopen_orca(lua_State* L)
{
  fprintf(stderr, "Available modules: ");
  for (luaL_Reg const* fn = orca_modules; fn->name; fn++) {
    fprintf(stderr, "%s ", &fn->name[5]);
    luaL_preload(L, fn->name, fn->func);
  }
  fprintf(stderr, "\n");

  luaL_newlib(L, ((luaL_Reg[]){
    { "registerEngineClass", f_registerEngineClass },
    { NULL, NULL }
  }));

  lua_pushstring(L, ORCA_VERSION);
  lua_setfield(L, -2, "version");


  lua_pushstring(L, __DATE__);
  lua_setfield(L, -2, "build");

  // Create registry table for coroutine handlers
  lua_newtable(L);
  lua_setfield(L, LUA_REGISTRYINDEX, "__coroutine_handlers");
  
  // Register async function
  lua_pushcfunction(L, f_async);
  lua_setfield(L, -2, "async");
  
  return 1;
}
