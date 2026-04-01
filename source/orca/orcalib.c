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
int luaopen_orca_system(lua_State*);
int luaopen_orca_editor(lua_State*);

static luaL_Reg const orca_modules[] = {
  { "orca.core", luaopen_orca_core },
  { "orca.filesystem", luaopen_orca_filesystem },
  { "orca.filesystem.native", luaopen_orca_filesystem_native },
  { "orca.geometry", luaopen_orca_geometry },
  { "orca.localization", luaopen_orca_localization },
#ifndef __EMSCRIPTEN__
  { "orca.editor", luaopen_orca_editor },
#endif
  { "orca.network", luaopen_orca_network },
  { "orca.parsers.json", luaopen_orca_parsers_json },
  { "orca.parsers.xml", luaopen_orca_parsers_xml },
  { "orca.renderer", luaopen_orca_renderer },
  { "orca.system", luaopen_orca_system },
  { NULL, NULL }
};

static int f_async(lua_State* L) {
  const int nargs = lua_gettop(L);
  lua_State* co = lua_newthread(L);
  int ref = luaL_ref(L, LUA_REGISTRYINDEX);
  lua_xmove(L, co, nargs);
  WI_PostMessageW(co, ID_ResumeCoroutine, MAKEDWORD(nargs-1, ref), NULL);
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

static int
load_core_module(lua_State *L)
{
  static const char *code =
    "local sys = require 'orca.system'\n"
    "for path in sys.list_dir(SHAREDIR..'/plugins') do\n"
    "  if not path:find('[/\\\\]') and not path:find('%.%.') and path:match('%.lua$') then\n"
    "    local ok, err = xpcall(dofile, function(e) return debug.traceback(e, 2) end,\n"
    "                           SHAREDIR..'/plugins/'..path)\n"
    "    if not ok then\n"
    "      io.stderr:write('Plugin error ('..path..'): '..tostring(err)..'\\n')\n"
    "    end\n"
    "  end\n"
    "end\n";
  if (luaL_dostring(L, code) != LUA_OK) {
    fprintf(stderr, "orca.init error: %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
  }
  return 0;
}

static int f_styles_newindex(lua_State* L) {
  // Stack: 1=styles table, 2=key (class name), 3=value (properties table)
  const char* name = luaL_checkstring(L, 2);
  // Store in the raw table so style[key] reads work (e.g. style[key][prop] = val)
  lua_pushvalue(L, 2);
  lua_pushvalue(L, 3);
  lua_rawset(L, 1);
  // Register in the global static stylesheet (stack: 1=table, 2=key, 3=value)
  if (lua_type(L, 3) == LUA_TTABLE) {
    OBJ_AddStyleSheet(L, NULL, name);
  }
  return 0;
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

#if __has_include("plugins_luaopen.h")
#include "plugins_luaopen.h"
  // Register statically-linked plugin modules (WebGL / single-binary builds).
  for (luaL_Reg const* fn = plugin_modules; fn->name; fn++) {
    luaL_preload(L, fn->name, fn->func);
  }
#endif

  luaL_newlib(L, ((luaL_Reg[]){
    { NULL, NULL }
  }));

  lua_setfield(L, ((void)lua_pushstring(L, ORCA_VERSION), -2), "version");
  lua_setfield(L, ((void)lua_pushstring(L, __DATE__), -2), "build");
  lua_setfield(L, (lua_pushcfunction(L, f_find_metatable), -2), "find_metatable");
  lua_setfield(L, (lua_pushcfunction(L, f_async), -2), "async");
  lua_setfield(L, (lua_pushcfunction(L, load_core_module), -2), "init");
  lua_setfield(L, (lua_newtable(L), -2), "theme");
  lua_setfield(L, (lua_newtable(L), -2), "config");
  // Create styles table with __newindex to register global stylesheets
  lua_newtable(L);
  lua_newtable(L);
  lua_pushcfunction(L, f_styles_newindex);
  lua_setfield(L, -2, "__newindex");
  lua_setmetatable(L, -2);
  lua_setfield(L, -2, "styles");
  
  // lua_newtable(L);
  // lua_setfield(L, -2, "config");
  
#ifdef EASY_MODULE_ACCESS
  // Create a metatable for the orca module
  lua_newtable(L);
  lua_pushcfunction(L, f_orca_index);
  lua_setfield(L, -2, "__index");
  lua_setmetatable(L, -2);
#endif
  
  return 1;
}
