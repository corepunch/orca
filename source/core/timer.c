#include "core_local.h"

int OBJ_SetTimer(lua_State* L, lpObject_t self)
{
  luaX_parsefield(uint32_t, duration, 2, luaL_checknumber);
  uint32_t id = WI_SetTimer(self, MAX(duration, 1), NULL, TRUE);
  lua_pushinteger(L, id);
  return 1;
}
