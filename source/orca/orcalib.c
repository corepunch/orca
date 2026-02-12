#ifdef __QNX__
#define LUA_USE_LINUX
#define l_likely(x) (x)
#define l_unlikely(x) (x)
#include "lua-master/onelua.c"
#endif

#include <include/api.h>
#include <include/version.h>

#define MODULE_NAME_BUFFER_SIZE 256
// Maximum key length: buffer size - "orca." (5 chars) - null terminator (1 char) = 250
#define MAX_MODULE_KEY_LENGTH 250

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

static int f_orca_index(lua_State* L) {
  size_t key_len;
  const char* key = luaL_checklstring(L, 2, &key_len);
  
  // First check if the value already exists in the table
  lua_pushvalue(L, 2);
  lua_rawget(L, 1);
  if (!lua_isnil(L, -1)) {
    return 1;
  }
  lua_pop(L, 1);
  
  // Check key length to prevent buffer overflow
  if (key_len > MAX_MODULE_KEY_LENGTH) {
    lua_pushnil(L);
    return 1;
  }
  
  // Try to require "orca.<key>"
  char module_name[MODULE_NAME_BUFFER_SIZE];
  snprintf(module_name, sizeof(module_name), "orca.%s", key);
  
  // Call require(module_name)
  lua_getglobal(L, "require");
  lua_pushstring(L, module_name);
  
  // Use pcall to catch any errors
  if (lua_pcall(L, 1, 1, 0) == 0) {
    // Cache the result in the table
    lua_pushvalue(L, -1);
    lua_setfield(L, 1, key);
    return 1;
  } else {
    // If require failed, log error and return nil
    // The error message is on top of the stack
    const char* err_msg = lua_tostring(L, -1);
    fprintf(stderr, "Failed to load module '%s': %s\n", 
            module_name, err_msg ? err_msg : "(error message unavailable)");
    lua_pop(L, 1);
    lua_pushnil(L);
    return 1;
  }
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
  
  // Create a metatable for the orca module
  lua_newtable(L);
  lua_pushcfunction(L, f_orca_index);
  lua_setfield(L, -2, "__index");
  lua_setmetatable(L, -2);
  
  return 1;
}
