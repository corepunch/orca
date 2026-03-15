#ifdef __QNX__
#define LUA_USE_LINUX
#define l_likely(x) (x)
#define l_unlikely(x) (x)
#include "lua-master/onelua.c"
#endif

#include <include/api.h>
#include <include/version.h>

#define EASY_MODULE_ACCESS

#ifdef EASY_MODULE_ACCESS
#define MODULE_NAME_BUFFER_SIZE 256
// Maximum key length: buffer size - "orca." prefix including period (5 chars) - null terminator (1 char) = 250
#define MAX_MODULE_KEY_LENGTH 250
#endif

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
int luaopen_orca_SpriteKit(lua_State*);
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
  { "orca.parsers.json", luaopen_orca_parsers_json },
  { "orca.parsers.xml", luaopen_orca_parsers_xml },
  { "orca.renderer", luaopen_orca_renderer },
  { "orca.SceneKit", luaopen_orca_SceneKit },
  { "orca.SpriteKit", luaopen_orca_SpriteKit },
  { "orca.backend", luaopen_orca_backend },
  { "orca.system", luaopen_orca_system },
  { "orca.ui", luaopen_orca_UIKit },
  { "orca.vsomeip", luaopen_orca_vsomeip },
  { "orca.editor", luaopen_orca_editor },
  { NULL, NULL }
};

static int f_async(lua_State* L) {
  const int nargs = lua_gettop(L);
  lua_State* co = lua_newthread(L);
  int ref = luaL_ref(L, LUA_REGISTRYINDEX);
  lua_xmove(L, co, nargs);
  WI_PostMessageW(co, kEventResumeCoroutine, MAKEDWORD(nargs-1, ref), NULL);
  return 0;
}

#ifdef EASY_MODULE_ACCESS
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
    fprintf(stderr, "Module key '%s' exceeds maximum length (%zu > %d)\n", 
            key, key_len, MAX_MODULE_KEY_LENGTH);
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
    // luaL_tolstring converts the error (at -1) to a string and pushes it on the stack
    // Stack after luaL_tolstring: [..., error, error_string]
    const char* err_msg = luaL_tolstring(L, -1, NULL);
    // Use the error message immediately before popping
    fprintf(stderr, "Failed to load module '%s': %s\n", module_name, err_msg);
    lua_pop(L, 2);  // Pop the converted string and the original error
    lua_rawget(L, -2);
//    lua_pushnil(L);
    return 1;
  }
}
#endif

int load_plugins(lua_State *L)
{
  int no_errors = 1;
  char SHAREDIR[MAX_OSPATH];

  lua_getglobal(L, "SHAREDIR");
  strncpy(SHAREDIR, luaL_checkstring(L, -1), sizeof(SHAREDIR));
  lua_pop(L, 1);

  lua_getglobal(L, "require");
  lua_pushstring(L, "orca.system");
  lua_call(L, 1, 1);                        // sys
    
  lua_getfield(L, -1, "list_dir");
  lua_pushfstring(L, "%s/plugins", SHAREDIR);
  lua_call(L, 1, 1);                        // iterator
  
  for (;;) {
    lua_pushvalue(L, -1);
    lua_call(L, 0, 1);                    // f
    if (lua_isnil(L, -1)) break;
    
    const char *f = lua_tostring(L, -1);
//    size_t n = strlen(f);
//    if (n > 4 && !strcmp(f + n - 4, ".lua")) n -= 4;
//    
//    lua_getglobal(L, "require");
//    lua_pushfstring(L, "plugins.%.*s", (int)n, f);
    
    lua_getglobal(L, "dofile");
    lua_pushfstring(L, "%s/plugins/%s", SHAREDIR, f);
    
    if (lua_pcall(L, 1, 0, 0) == LUA_OK)
//      fprintf(stderr, "Loaded plugin \"%.*s\"\n", (int)n, f);
      fprintf(stderr, "Loaded plugin \"%s\"\n", f);
    else {
      fprintf(stderr, "%s\n", lua_tostring(L, -1));
      lua_pop(L, 1);
      no_errors = 0;
    }
    
    lua_pop(L, 1);                        // pop f
  }
  
  return no_errors;
}

static int f_find_metatable(lua_State* L) {
  lpcString_t tname = luaL_checkstring(L, 1);
  if (luaL_getmetatable(L, tname) == 0) {  // returns 0 if metatable not found
    return luaL_error(L, "metatable '%s' not found", tname);
  }
  assert(lua_type(L, -1) == LUA_TTABLE);
  return 1;  // metatable is already on top of stack
}

ORCA_API int luaopen_orca(lua_State* L)
{
  for (luaL_Reg const* fn = orca_modules; fn->name; fn++) {
    luaL_preload(L, fn->name, fn->func);
  }

  luaL_newlib(L, ((luaL_Reg[]){
    { NULL, NULL }
  }));

  lua_newtable(L);
  lua_setfield(L, LUA_REGISTRYINDEX, "LOADERS");
  
  lua_pushstring(L, ORCA_VERSION);
  lua_setfield(L, -2, "version");

  lua_pushstring(L, __DATE__);
  lua_setfield(L, -2, "build");
  
  lua_pushcfunction(L, f_find_metatable);
  lua_setfield(L, -2, "find_metatable");

  lua_pushcfunction(L, f_async);
  lua_setfield(L, -2, "async");

//  if (luaL_dostring(L,
//                    "return function()\n"
//                    "\tlocal sys = require 'orca.system'\n"
//                    "\tlocal no_errors = true\n"
//                    "\tfor f in sys.list_dir(SHAREDIR..'/plugins') do\n"
//                    "\t\tlocal ok, err = pcall(require, 'plugins.'..f:gsub('.lua$', ''))\n"
//                    "\t\tif ok then io.stderr:write(string.format('Loaded plugin %q\\n', f:gsub('.lua$', '')))\n"
//                    "\t\telse io.stderr:write(err, '\\n') no_errors = false end\n"
//                    "\tend\n"
//                    "\treturn no_errors\n"
//                    "end\n") != LUA_OK)
//  {
//    Con_Error("%s\n", lua_tostring(L, -1));
//  }
//  assert(lua_type(L, -1) == LUA_TFUNCTION);
  lua_pushcfunction(L, load_plugins);
  lua_setfield(L, -2, "init");
  
#ifdef EASY_MODULE_ACCESS
  // Create a metatable for the orca module
  lua_newtable(L);
  lua_pushcfunction(L, f_orca_index);
  lua_setfield(L, -2, "__index");
  lua_setmetatable(L, -2);
#endif
  
  return 1;
}
