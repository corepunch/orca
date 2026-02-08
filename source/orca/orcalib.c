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
  
  // Event-based coroutine system
  luaL_dostring(L,
                "local next_id = 1\n"
                "return function(task, ...)\n"
                "  local co = coroutine.create(task)\n"
                "  local args = {...}\n"
                "  local handler_id = next_id\n"
                "  next_id = next_id + 1\n"
                "  \n"
                "  local function resume_handler()\n"
                "    local ok, result = coroutine.resume(co, table.unpack(args))\n"
                "    if not ok then\n"
                "      print('Coroutine error:', result)\n"
                "      return false\n"
                "    end\n"
                "    if coroutine.status(co) ~= 'dead' then\n"
                "      args = {} -- Clear args after first call\n"
                "      return true\n"
                "    else\n"
                "      return false\n"
                "    end\n"
                "  end\n"
                "  \n"
                "  -- Store handler in registry\n"
                "  local handlers = debug.getregistry().__coroutine_handlers\n"
                "  handlers[handler_id] = resume_handler\n"
                "  \n"
                "  -- Post initial resume event via C API\n"
                "  local orca_core = require('orca.core')\n"
                "  orca_core.post_coroutine_resume(handler_id)\n"
                "  \n"
                "  return { id = handler_id, coroutine = co }\n"
                "end\n");
  lua_setfield(L, -2, "async");
  
  return 1;
}
