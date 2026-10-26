#include "api.h"

int lua_pushregion2int8(lua_State *L, region2int8_t const rect)
{
	coord_t *self = lua_newuserdata(L, sizeof(region2int8_t));
	luaL_setmetatable(L, API_TYPE_REGION2_INT8);
	memcpy(self, rect, sizeof(region2int8_t));
	return 1;
}

static int f_new(lua_State *L)
{
	region2int8_t const rect = {
		luaL_checknumber(L, 1),
		luaL_checknumber(L, 2),
		luaL_checknumber(L, 3),
		luaL_checknumber(L, 4),
	};
	return lua_pushregion2int8(L, rect);
}

static int f_add(lua_State *L)
{
	coord_t const *self = luaL_checkudata(L, 1, API_TYPE_REGION2_INT8);
	coord_t const *v = luaL_checkudata(L, 2, API_TYPE_VECTOR2_INT8);
	region2int8_t const out = { self[0] + v[0], self[1] + v[1], self[2] + v[0], self[3] + v[1] };
	return lua_pushregion2int8(L, out);
}

static int f_sub(lua_State *L)
{
	coord_t const *self = luaL_checkudata(L, 1, API_TYPE_REGION2_INT8);
	coord_t const *v = luaL_checkudata(L, 2, API_TYPE_VECTOR2_INT8);
	region2int8_t const out = { self[0] - v[0], self[1] - v[1], self[2] - v[0], self[3] - v[1] };
	return lua_pushregion2int8(L, out);
}

static int f_contains(lua_State *L)
{
	coord_t const *a = luaL_checkudata(L, 1, API_TYPE_REGION2_INT8);
	coord_t const *b = luaL_checkudata(L, 2, API_TYPE_REGION2_INT8);
	lua_pushboolean(L, a[0] <= b[0] && a[1] <= b[1] && a[2] >= b[2] && a[3] >= b[3]);
	return 1;
}

static int f_containsPoint(lua_State *L)
{
	coord_t const *a = luaL_checkudata(L, 1, API_TYPE_REGION2_INT8);
	int x = luaL_checknumber(L, 2);
	int y = luaL_checknumber(L, 3);
	lua_pushboolean(L, a[0] <= x && a[1] <= y && a[2] >= x && a[3] >= y);
	return 1;
}

static int f_getProperty(lua_State *L)
{
	coord_t const *self = luaL_checkudata(L, 1, API_TYPE_REGION2_INT8);
	const char *name = luaL_checkstring(L, 2);
	if (!strcmp(name, "min")) { lua_pushvec2int8(L, &self[0]); return 1; }
	if (!strcmp(name, "max")) { lua_pushvec2int8(L, &self[2]); return 1; }
	if (!strcmp(name, "width")) { lua_pushinteger(L, abs(self[2] - self[0])); return 1; }
	if (!strcmp(name, "height")) { lua_pushinteger(L, abs(self[3] - self[1])); return 1; }
	if (!strcmp(name, "contains")) { lua_pushcfunction(L, f_contains); return 1; }
	if (!strcmp(name, "containsPoint")) { lua_pushcfunction(L, f_containsPoint); return 1; }
	return 0;
}

static const luaL_Reg lib[] = {
	{"new", f_new},
	{"__index", f_getProperty},
	{"__add", f_add},
	{"__sub", f_sub},
	{NULL, NULL}
};

int luaopen_level_region2_INT8(lua_State *L)
{
	luaL_newmetatable(L, API_TYPE_REGION2_INT8);
	luaL_setfuncs(L, lib, 0);
	return 1;
}
