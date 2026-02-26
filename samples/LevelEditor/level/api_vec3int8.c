#include "api.h"

int lua_pushvec3int8(lua_State *L, vec3int8_t const loc)
{
	coord_t *self = lua_newuserdata(L, sizeof(vec3int8_t));
	luaL_setmetatable(L, API_TYPE_VECTOR3_INT8);
	memcpy(self, loc, sizeof(vec3int8_t));
	return 1;
}

static int f_new(lua_State *L)
{
	vec3int8_t const loc = {
		luaL_checknumber(L, 1),
		luaL_checknumber(L, 2),
		luaL_checknumber(L, 3),
	};
	return lua_pushvec3int8(L, loc);
}

static int f_getProperty(lua_State *L)
{
	coord_t const *self = luaL_checkudata(L, 1, API_TYPE_VECTOR3_INT8);
	const char *name = luaL_checkstring(L, 2);
	if (!strcmp(name, "x")) { lua_pushinteger(L, self[0]); return 1; }
	if (!strcmp(name, "y")) { lua_pushinteger(L, self[1]); return 1; }
	if (!strcmp(name, "z")) { lua_pushinteger(L, self[2]); return 1; }
	return 0;
}

static int f_setProperty(lua_State *L)
{
	coord_t *self = luaL_checkudata(L, 1, API_TYPE_VECTOR3_INT8);
	const char *name = luaL_checkstring(L, 2);
	if (!strcmp(name, "x")) { self[0] = luaL_checknumber(L, 3); return 0; }
	if (!strcmp(name, "y")) { self[1] = luaL_checknumber(L, 3); return 0; }
	if (!strcmp(name, "z")) { self[2] = luaL_checknumber(L, 3); return 0; }
	return 0;
}

static int f_add(lua_State *L)
{
	coord_t const *a = luaL_checkudata(L, 1, API_TYPE_VECTOR3_INT8);
	coord_t const *b = luaL_checkudata(L, 2, API_TYPE_VECTOR3_INT8);
	vec3int8_t const out = { a[0] + b[0], a[1] + b[1], a[2] + b[2] };
	return lua_pushvec3int8(L, out);
}

static int f_sub(lua_State *L)
{
	coord_t const *a = luaL_checkudata(L, 1, API_TYPE_VECTOR3_INT8);
	coord_t const *b = luaL_checkudata(L, 2, API_TYPE_VECTOR3_INT8);
	vec3int8_t const out = { a[0] - b[0], a[1] - b[1], a[2] - b[2] };
	return lua_pushvec3int8(L, out);
}

static const luaL_Reg lib[] = {
	{"new", f_new},
	{"__index", f_getProperty},
	{"__newindex", f_setProperty},
	{"__add", f_add},
	{"__sub", f_sub},
	{NULL, NULL}
};

int luaopen_level_vec3int8(lua_State *L)
{
	luaL_newmetatable(L, API_TYPE_VECTOR3_INT8);
	luaL_setfuncs(L, lib, 0);
	return 1;
}
