#include "api.h"

int lua_pushblock(lua_State *L, room_t *room, vec2int8_t const loc)
{
	block_t *block = Room_NavigateToBlockAt(room, loc[0], loc[1]);
	if (block) {
		blockWrapper_t *self = lua_newuserdata(L, sizeof(blockWrapper_t));
		luaL_setmetatable(L, LEVEL_API_TYPE_BLOCK);
		self->room = room;
		self->block = Room_NavigateToBlockAt(room, loc[0], loc[1]);
		memcpy(self->location, loc, sizeof(vec2int8_t));
		return 1;
	} else {
		return 0;
	}
}

static int f_getProperty(lua_State *L)
{
	blockWrapper_t const *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_BLOCK);
	const char *name = luaL_checkstring(L, 2);
	if (!strcmp(name, "x")) { lua_pushinteger(L, self->location[0]); return 1; }
	if (!strcmp(name, "y")) { lua_pushinteger(L, self->location[1]); return 1; }
	if (!strcmp(name, "location")) { lua_pushvec2int8(L, self->location); return 1; }
	if (!strcmp(name, "type")) { lua_pushstring(L, tiletypes[self->block->type]); return 1; }
	return 0;
}

static const luaL_Reg lib[] = {
	{"__index", f_getProperty},
	{NULL, NULL}
};

int luaopen_level_block(lua_State *L)
{
	luaL_newmetatable(L, LEVEL_API_TYPE_BLOCK);
	luaL_setfuncs(L, lib, 0);
	return 1;
}
