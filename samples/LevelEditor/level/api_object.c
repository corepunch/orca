#include "api.h"

int lua_pushobject(lua_State *L, room_t *room, int index)
{
	object_t *object = Room_GetObjectAtIndex(room, index);
	objectWrapper_t *self = lua_newuserdata(L, sizeof(objectWrapper_t));
	luaL_setmetatable(L, LEVEL_API_TYPE_OBJECT);
	self->room = room;
	self->object = object;
	self->index = index;
	return 1;
}

static int f_getProperty(lua_State *L)
{
	objectWrapper_t const *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_OBJECT);
	const char *name = luaL_checkstring(L, 2);
	if (!strcmp(name, "x")) { lua_pushinteger(L, self->object->origin[0]); return 1; }
	if (!strcmp(name, "y")) { lua_pushinteger(L, self->object->origin[1]); return 1; }
	if (!strcmp(name, "z")) { lua_pushinteger(L, self->object->origin[2]); return 1; }
	if (!strcmp(name, "origin")) { lua_pushvec3int8(L, self->object->origin); return 1; }
	if (!strcmp(name, "type")) { lua_pushstring(L, objecttypes[self->object->type]); return 1; }
	return 0;
}

static int f_setProperty(lua_State *L)
{
	objectWrapper_t const *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_OBJECT);
	const char *name = luaL_checkstring(L, 2);
	if (!strcmp(name, "x")) { self->object->origin[0] = luaL_checknumber(L, 3); return 0; }
	if (!strcmp(name, "y")) { self->object->origin[1] = luaL_checknumber(L, 3); return 0; }
	if (!strcmp(name, "z")) { self->object->origin[2] = luaL_checknumber(L, 3); return 0; }
	if (!strcmp(name, "origin")) { lua_pushvec3int8(L, self->object->origin); return 1; }
	if (!strcmp(name, "type")) { lua_pushstring(L, objecttypes[self->object->type]); return 1; }
	return 0;
}

static const luaL_Reg lib[] = {
	{"__index", f_getProperty},
	{"__newindex", f_setProperty},
	{NULL, NULL}
};

int luaopen_level_object(lua_State *L)
{
	luaL_newmetatable(L, LEVEL_API_TYPE_OBJECT);
	luaL_setfuncs(L, lib, 0);
	return 1;
}
