#include <code/api/api.h>

#include "types.h"

#define LEVEL_API_TYPE_LEVEL "level.Level"
#define LEVEL_API_TYPE_ROOM "level.Room"
#define LEVEL_API_TYPE_BLOCK "level.Block"
#define API_TYPE_REGION2_INT8 "level.Rect"
#define API_TYPE_VECTOR2_INT8 "level.Vector2int8"
#define API_TYPE_VECTOR3_INT8 "level.IntVector3"
#define LEVEL_API_TYPE_OBJECT "level.Object"

typedef struct
{
	room_t *room;
} roomWrapper_t;

typedef struct
{
	block_t *block;
	room_t *room;
	vec2int8_t location;
} blockWrapper_t;

typedef struct
{
	object_t *object;
	room_t *room;
	uint8_t index;
} objectWrapper_t;

static const char *const tiletypes[] = { "none", "floor", "wall", "box", NULL };
static const char *const objecttypes[] = { "PointLight", "SunLight", "Model", "SoundSource", NULL };


int lua_pushroom(lua_State *L, room_t *room);
int lua_pushblock(lua_State *L, room_t *room, vec2int8_t const loc);
int lua_pushobject(lua_State *L, room_t *room, int index);
int lua_pushregion2int8(lua_State *L, region2int8_t const rect);
int lua_pushvec2int8(lua_State *L, vec2int8_t const vec);
int lua_pushvec3int8(lua_State *L, vec3int8_t const vec);

int luaopen_level_level(lua_State *L);
int luaopen_level_room(lua_State *L);
int luaopen_level_block(lua_State *L);
int luaopen_level_object(lua_State *L);
int luaopen_level_region2_INT8(lua_State *L);
int luaopen_level_vec2_INT8(lua_State *L);
int luaopen_level_vec3int8(lua_State *L);