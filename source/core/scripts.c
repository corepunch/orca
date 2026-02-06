#include <include/api.h>

static char const behaviour_lua[] =
#include "behaviour.lua"
  ;

#define OPEN_LUA(NAME)                                                         \
  ORCA_API int luaopen_orca_##NAME(lua_State* L)                               \
  {                                                                            \
    if (luaL_loadbuffer(                                                       \
          L, NAME##_lua, sizeof(NAME##_lua) - 1, "=Scripts/" #NAME ".lua") ==  \
        0)                                                                     \
      lua_call(L, 0, 1);                                                       \
    return 1;                                                                  \
  }

OPEN_LUA(behaviour);
