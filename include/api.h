#ifndef __api_h__
#define __api_h__

#ifdef __QNX__
#include "../source/lua-master/lauxlib.h"
#include "../source/lua-master/lua.h"
#include "../source/lua-master/lualib.h"
#elif __has_include(<lua5.4/lua.h>)
#include <lua5.4/lauxlib.h>
#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
#elif __has_include(<lua5.3/lua.h>)
#include <lua5.3/lauxlib.h>
#include <lua5.3/lua.h>
#include <lua5.3/lualib.h>
#else
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
#endif

#include <include/orca.h>

#define INLINE static inline

#define luaX_parsefield(type, name, unit, func, ...)                            \
  lua_getfield(L, unit, #name);                                                \
  type name = func(L, -1, ##__VA_ARGS__);                                      \
  lua_pop(L, 1);

struct Package;
struct Object;

int f_find_module(lua_State* L);

ORCA_API void luaX_pushProperty(lua_State*, lpcProperty_t);
ORCA_API int luaX_readProperty(lua_State*, int, lpProperty_t);

ORCA_API void luaX_pushKeyframeAnim(lua_State*, lpcKeyframeAnim_t);
ORCA_API lpKeyframeAnim_t luaX_checkKeyframeAnim(lua_State*, int);

#define API_MODULE_SHUTDOWN(L, func)                                           \
  lua_newtable(L);                                                             \
  lua_pushcfunction(L, func);                                                  \
  lua_setfield(L, -2, "__gc");                                                 \
  lua_setmetatable(L, -2);

//INLINE void*
//API_GetModule(lua_State* L, lpcString_t szName)
//{
//  lua_getfield(L, LUA_REGISTRYINDEX, szName);
//  if (lua_islightuserdata(L, -1) || lua_isuserdata(L, -1)) {
//    void* value = lua_touserdata(L, -1);
//    lua_pop(L, 1);
//    return value;
//  } else {
//    lua_pop(L, 1);
//    return NULL;
//  }
//}

INLINE
int
API_CallRequire(lua_State* L, lpcString_t szModuleName, uint32_t dwReturnValues)
{
  lua_getglobal(L, "require");
  lua_pushstring(L, szModuleName);
  if (lua_pcall(L, 1, dwReturnValues, 0) != LUA_OK) {
    Con_Error("%s", lua_tostring(L, -1));
    lua_pop(L, 1);
    return !LUA_OK;
  } else {
    return LUA_OK;
  }
}

INLINE
void
luaX_import(lua_State* L, lpcString_t szModuleName, lpcString_t szFieldName)
{
  lua_getglobal(L, "require");
  lua_pushstring(L, szModuleName);
  if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
    Con_Error("%s", lua_tostring(L, -1));
    lua_pop(L, 1);
  }
  lua_getfield(L, -1, szFieldName);
  lua_remove(L, -2);
}

#define API_TYPE_OBJECT "Object"

ORCA_API lpObject_t luaX_checkObject(lua_State* L, int arg);
ORCA_API void luaX_pushObject(lua_State*, lpcObject_t);
ORCA_API int lua_pushclass(lua_State* L, struct ClassDesc* cl);

void
API_PrintStackTrace(lua_State*);

INLINE bool_t
luaX_executecallback(lua_State* L,
                     lpObject_t object,
                     lpcString_t name,
                     int num_args)
{
  lua_geti(L, LUA_REGISTRYINDEX, OBJ_GetLuaObject(object));
  assert(lua_type(L, -1) == LUA_TTABLE);
  lua_getfield(L, -1, name);
  lua_insert(L, -(num_args + 2)); // Move the function below the arguments
  lua_insert(L, -(num_args + 1)); // Move the object below the arguments
  if (lua_pcall(L, num_args + 1, 1, 0) != LUA_OK) {
    Con_Error("%s(): %s", name, lua_tostring(L, -1));
    lua_pop(L, 1);
    return FALSE;
  }
  bool_t ret = lua_toboolean(L, -1);
  lua_pop(L, 1);
  return ret;
}

// INLINE int luaX_callfunction(lua_State *L, lpcString_t  mod, lpcString_t 
// func, uint32_t num_args, uint32_t num_returns) {
//     // Load the module using require
//     lua_getglobal(L, "require");
//     lua_pushstring(L, mod);
//     if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
//         Con_Error("Error loading module %s: %s", mod, lua_tostring(L,
//         -1)); return LUA_ERRRUN;
//     }
//
//     // Get the function from the module
//     lua_getfield(L, -1, func); // Get the function from the module (e.g.,
//     core.handleEvent) lua_remove(L, -2); // Remove the module from the stack,
//     leaving only the function
//
//     // Check if the retrieved value is a function
//     if (lua_isfunction(L, -1)) {
//         // Move the function below the arguments using lua_insert
//         lua_insert(L, -(num_args + 1)); // Move the function below the
//         arguments
//         // Call the function with num_args and num_returns
//         if (lua_pcall(L, num_args, num_returns, 0) != LUA_OK) {
//             Con_Error("Error calling %s.%s: %s", mod, func,
//             lua_tostring(L, -1)); return LUA_ERRRUN;
//         }
//     } else {
//         Con_Error("Error: %s.%s is not a function", mod, func);
//         lua_pop(L, 1); // Pop the non-function value from the stack
//         return LUA_ERRRUN;
//     }
//     return LUA_OK;
// }

#endif
