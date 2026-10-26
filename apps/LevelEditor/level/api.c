#include "api.h"

#define set_constant(name) \
lua_pushnumber(L, name); \
lua_setfield(L, -2, #name);

static const luaL_Reg lib[] =
{
    { NULL, NULL }
};

int luaopen_level(lua_State *L)
{
    luaL_newlib(L, lib);
    luaopen_level_vec2_INT8(L); lua_setfield(L, -2, "Vector2int8");
    luaopen_level_vec3int8(L); lua_setfield(L, -2, "Vector3int8");
    luaopen_level_region2_INT8(L); lua_setfield(L, -2, "Region2int8");
    luaopen_level_block(L); lua_setfield(L, -2, "Block");
    luaopen_level_object(L); lua_setfield(L, -2, "Object");
    luaopen_level_room(L); lua_setfield(L, -2, "Room");
    luaopen_level_level(L); lua_setfield(L, -2, "Level");

    set_constant(WORLD_SIZE);
	set_constant(ROOM_SIZE);
	set_constant(BLOCK_SIZE);
	set_constant(CLICK_SIZE);
	set_constant(DEFAULT_CEILING);
	set_constant(NUM_CLICKS);

    return 1;
}
