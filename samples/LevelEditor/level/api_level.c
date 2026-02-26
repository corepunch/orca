#include "api.h"

struct game_import *gi;
color_t worldBitmap[WORLD_SIZE][WORLD_SIZE];

typedef struct {
    level_t *level;
} levelWrapper_t;

typedef struct {
    level_t *level;
    int index;
} roomIterator_t;

static int IsPixelWithinBounds(int x, int y, int bound)
{
    if (x < 0)
        return 0;
    if (y < 0)
        return 0;
    if (x >= bound)
        return 0;
    if (y >= bound)
        return 0;
    return 1;
}

static int f_new(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    level_t *level = gi->renderer->RegisterUserData(name, sizeof(level_t));
    levelWrapper_t *self = lua_newuserdata(L, sizeof(levelWrapper_t));
    luaL_setmetatable(L, LEVEL_API_TYPE_LEVEL);
    if (level->numRooms == 0) {
        Level_Init(level);
    }
    self->level = level;
    return 1;
}

static int f_load(lua_State *L)
{
    const char *filename = luaL_checkstring(L, 1);
    level_t *level = gi->renderer->RegisterUserData(filename, sizeof(level_t));
    levelWrapper_t *self = lua_newuserdata(L, sizeof(levelWrapper_t));
    luaL_setmetatable(L, LEVEL_API_TYPE_LEVEL);
    if (level->numRooms == 0) {
        Level_Load(level, filename);
    }
    if (level->numRooms == 0) {
        Level_Init(level);
    }
    self->level = level;

    return 1;
}

static int f_save(lua_State *L)
{
    levelWrapper_t *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_LEVEL);
    const char *filename = luaL_checkstring(L, 2);
    Level_Save(self->level, filename);
    return 0;
}

static int f_gc(lua_State *L)
{
    levelWrapper_t *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_LEVEL);
    gi->renderer->ReleaseUserData(self->level);
    return 0;
}

static int f_getWorldView(lua_State *L)
{
    levelWrapper_t const *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_LEVEL);
    roomWrapper_t const *selectedRoom = luaL_testudata(L, 2, LEVEL_API_TYPE_ROOM);
    memset(worldBitmap, 255, sizeof(worldBitmap));
    FOR_LOOP(idx, self->level->numRooms) {
        room_t const *room = &self->level->rooms[idx];
        coord_t const *loc = room->location;
        FOR_TILES(0, 0, ROOM_SIZE, ROOM_SIZE)
        {
            if (!IsPixelWithinBounds(x, y, WORLD_SIZE))
                continue;
            block_t const *block = Room_NavigateToCBlockAt(room, x, y);
            if (block->type != kBlockTypeFloor)
                continue;
            color_t *pixel = &worldBitmap[y + loc[1]][x + loc[0]];
            if (selectedRoom && selectedRoom->room == room) {
                *pixel = GetSelectedBlockColor();
            } else {
                *pixel = GetBlockTypeColor(block->type);
            }
        }
    }
    image_t **image = lua_newuserdata(L, sizeof(image_t *));
    luaL_setmetatable(L, API_TYPE_IMAGE);
    *image = gi->renderer->AllocateTexture(WORLD_SIZE, WORLD_SIZE, (color_t const *)worldBitmap);
    return 1;
}

static int f_addRoom(lua_State *L)
{
    levelWrapper_t *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_LEVEL);
    int floor = luaL_checknumber(L, 2);
    int height = luaL_checknumber(L, 3);
    room_t *room = Level_AddRoom(self->level);
    Room_Init(room, floor, floor + height);
    return lua_pushroom(L, room);
}

static int f_removeRoom(lua_State *L)
{
    levelWrapper_t *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_LEVEL);
    roomWrapper_t *room = luaL_checkudata(L, 2, LEVEL_API_TYPE_ROOM);
    Level_RemoveRoom(self->level, room->room);
    return 0;
}

static int f_roomsIterator(lua_State *L)
{
    roomIterator_t *it = lua_touserdata(L, lua_upvalueindex(1));
    if (it->index < it->level->numRooms) {
        return lua_pushroom(L, &it->level->rooms[it->index++]);
    } else {
        return 0;
    }
}

static int f_getRooms(lua_State *L)
{
    levelWrapper_t *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_LEVEL);
    roomIterator_t *it = lua_newuserdata(L, sizeof(roomIterator_t));
    it->level = self->level;
    it->index = 0;
    lua_pushcclosure(L, f_roomsIterator, 1);
    return 1;
}

static const luaL_Reg lib[] =
{
    { "new", f_new },
    { "load", f_load },
    { "save", f_save },
    { "addRoom", f_addRoom },
    { "removeRoom", f_removeRoom },
    { "getRooms", f_getRooms },
    { "getWorldView", f_getWorldView },
    { "__gc", f_gc },
    { NULL, NULL }
};

int luaopen_level_level(lua_State *L)
{
    luaL_newmetatable(L, LEVEL_API_TYPE_LEVEL);
    luaL_setfuncs(L, lib, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_getglobal(L, "INTERFACE");
    gi = lua_touserdata(L, -1);
    lua_pop(L, 1);

    return 1;
}
