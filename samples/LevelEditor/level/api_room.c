#include "api.h"

struct game_import *gi;
color_t worldBitmap[WORLD_SIZE][WORLD_SIZE];

typedef struct {
    room_t *room;
    int index;
} objectIterator_t;

static const char *const portalDirections[] = {
    "floor",
    "ceiling",
    "wall_positive_x",
    "wall_negative_x",
    "wall_positive_y",
    "wall_negative_y",
    NULL
};

static const char *const blockVerticals[] = {
    "floor",
    "ceiling",
    "ed",
    "rf",
    NULL
};

int lua_pushroom(lua_State *L, room_t *room)
{
    roomWrapper_t *self = lua_newuserdata(L, sizeof(roomWrapper_t));
    luaL_setmetatable(L, LEVEL_API_TYPE_ROOM);
    self->room = room;
    return 1;
}

static int f_changeHeight(lua_State *L)
{
    roomWrapper_t *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    coord_t const *region = luaL_checkudata(L, 2, API_TYPE_REGION2_INT8);
    blockLayer_t vertical = luaL_checkoption(L, 3, NULL, blockVerticals);
    int value = luaL_checknumber(L, 4);
    int smooth = lua_toboolean(L, 5);
    int direction = luaL_optnumber(L, 6, 0);
    Room_ChangeHeight(self->room, region, vertical, value, smooth, direction);
    return 1;
}

static int f_setType(lua_State *L)
{
    roomWrapper_t *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    coord_t const *region = luaL_checkudata(L, 2, API_TYPE_REGION2_INT8);
    int type = luaL_checkoption(L, 3, "none", tiletypes);
    Room_SetBlockType(self->room, region, type);
    return 1;
}

static int f_getModel(lua_State *L)
{
    roomWrapper_t *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    model_t **model = (model_t **)lua_newuserdata(L, sizeof(model_t *));
    luaL_setmetatable(L, API_TYPE_MODEL);
    *model = gi->renderer->AddModelRef(self->room->model);
    return 1;
}

static int f_crop(lua_State *L)
{
    roomWrapper_t *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    coord_t const *region = luaL_checkudata(L, 2, API_TYPE_REGION2_INT8);
    Room_Crop(self->room, region);
    return 0;
}

static int f_getRoomView(lua_State *L)
{
    roomWrapper_t const *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    image_t **image = lua_newuserdata(L, sizeof(image_t *));
    luaL_setmetatable(L, API_TYPE_IMAGE);
    *image = gi->renderer->AddImageRef(self->room->map);
    return 1;
}

static int f_getLightmap(lua_State *L)
{
    roomWrapper_t const *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    image_t **image = lua_newuserdata(L, sizeof(image_t *));
    luaL_setmetatable(L, API_TYPE_IMAGE);
    *image = gi->renderer->AddImageRef(self->room->lightmap);
    return 1;
}

static int f_getBlockAt(lua_State *L)
{
    roomWrapper_t const *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    vec2int8_t loc = { luaL_checknumber(L, 2), luaL_checknumber(L, 3) };
    return lua_pushblock(L, self->room, loc);
}

static int f_setBlockAt(lua_State *L)
{
    roomWrapper_t *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    block_t *block = Room_NavigateToBlockAt(self->room, luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    if (block) {
        block->type = luaL_checkoption(L, 4, "none", tiletypes);
    }
    return 0;
}

static int f_raycastObjectPostion(lua_State *L)
{
    // roomWrapper_t const *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    vec_t const *a = luaL_checkudata(L, 2, API_TYPE_VECTOR3);
    vec_t const *b = luaL_checkudata(L, 3, API_TYPE_VECTOR3);
    triangle_t tri = { { 1, 0, 0 }, { 0, 0, 0 }, { 0, 0, 1 } };
    line_t line;
    vec3_t out;
    coord_t *loc = lua_newuserdata(L, sizeof(vec3int8_t));
    luaL_setmetatable(L, API_TYPE_VECTOR3_INT8);
    Vector3Copy(a, line.a);
	Vector3Copy(b, line.b);
    PointOnPlane(&line, &tri, out);
    loc[0] = (out[0]) / 5;
    loc[1] = (out[2]) / 5;
    loc[2] = 0;
    return 1;
}

static int f_raycastBlock(lua_State *L)
{
    roomWrapper_t *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    vec_t const *a = luaL_checkudata(L, 2, API_TYPE_VECTOR3);
    vec_t const *b = luaL_checkudata(L, 3, API_TYPE_VECTOR3);
    color_t value = Room_GetClickedBlock(self->room, a, b);
    if (value.b != 0 || value.a != 0) {
        vec2int8_t loc = { value.b, value.a };
        lua_pushblock(L, self->room, loc);
        lua_pushnumber(L, value.r);
        return 2;
    } else {
        return 0;
    }
}

static int f_setFaceTexCoord(lua_State *L)
{
    roomWrapper_t const *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    int face = luaL_checknumber(L, 3);
    int u = luaL_checknumber(L, 4);
    int v = luaL_checknumber(L, 5);
    int corner = luaL_checknumber(L, 6) - 1;
    coord_t *loc = luaL_testudata(L, 2, API_TYPE_VECTOR2_INT8);
    if (loc) {
        block_t *block = &self->room->blocks[loc[0]][loc[1]];
        block->uv[face][corner][0] = u;
        block->uv[face][corner][1] = v;
    } else {
        coord_t *box = luaL_checkudata(L, 2, API_TYPE_REGION2_INT8);
        FOR_TILES(box[0], box[1], box[2] + 1, box[3] + 1) {
            block_t *block = &self->room->blocks[x][y];
            block->uv[face][corner][0] = u;
            block->uv[face][corner][1] = v;
        }
    }
    return 0;
}

static int f_getLocation(lua_State *L)
{
    roomWrapper_t const *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    return lua_pushvec2int8(L, self->room->location);
}

static int f_setLocation(lua_State *L)
{
    roomWrapper_t *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    coord_t *loc = luaL_checkudata(L, 2, API_TYPE_VECTOR2_INT8);
    self->room->location[0] = loc[0];
    self->room->location[1] = loc[1];
    return 0;
}

static int f_setSelection(lua_State *L)
{
    roomWrapper_t *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    self->room->selection[0] = MIN(luaL_checknumber(L, 2), luaL_checknumber(L, 4));
    self->room->selection[1] = MIN(luaL_checknumber(L, 3), luaL_checknumber(L, 5));
    self->room->selection[2] = MAX(luaL_checknumber(L, 2), luaL_checknumber(L, 4));
    self->room->selection[3] = MAX(luaL_checknumber(L, 3), luaL_checknumber(L, 5));
    return 0;
}

static int f_getSelection(lua_State *L) {
    roomWrapper_t const *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    return lua_pushregion2int8(L, self->room->selection);
}

static int f_buildGeometry(lua_State *L)
{
    roomWrapper_t *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    dsurface_t dsurf = Room_AllocateModel(self->room);
    gi->renderer->ReleaseModel(self->room->model);
    gi->renderer->ReleaseTexture(self->room->map);
    gi->renderer->ReleaseTexture(self->room->lightmap);
    self->room->lightmap = Room_AllocateLightmap(&dsurf);
    self->room->model = gi->renderer->AllocateModel(&dsurf);
    self->room->map = Room_AllocateTexture(self->room);
    return 0;
}

static int f_getVerticalBounds(lua_State *L)
{
    roomWrapper_t const *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    coord_t const *region = luaL_checkudata(L, 2, API_TYPE_REGION2_INT8);
    region2int8_t bounds;
    Room_GetVerticalBounds(self->room, region, bounds);
    lua_pushinteger(L, bounds[0]);
    lua_pushinteger(L, bounds[1]);
    return 2;
}

static int f_getBounds(lua_State *L)
{
    roomWrapper_t const *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    region2int8_t bounds;
    Room_GetBounds(self->room, bounds);
    return lua_pushregion2int8(L, bounds);
}

static int f_getRoomCenter(lua_State *L)
{
    roomWrapper_t const *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    vec_t *loc = lua_newuserdata(L, sizeof(vec3_t));
    region2int8_t bounds;
    vec2int8_t vbounds = { 127, -127 };
    FOR_ALL_TILES()
	{
		block_t const *block = Room_GetCBlockAt(self->room, x, y);
        if (block->type != kBlockTypeFloor)
            continue;
		FOR_LOOP(corner, NUM_CORNERS)
		{
			vbounds[0] = MIN(vbounds[0], block->height[kBlockLayerFloor][corner]);
			vbounds[1] = MAX(vbounds[1], block->height[kBlockLayerCeiling][corner]);
		}
	}
    Room_GetBounds(self->room, bounds);
    luaL_setmetatable(L, API_TYPE_VECTOR3);
    loc[0] = self->room->location[0] + (bounds[0] + bounds[2]) * 0.5;
    loc[1] = self->room->location[1] + (bounds[1] + bounds[3]) * 0.5;
    loc[2] = (vbounds[0] + vbounds[1]) * 0.5;
    return 1;
}

static int f_addPortal(lua_State *L)
{
    roomWrapper_t *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    roomWrapper_t *target = luaL_checkudata(L, 4, LEVEL_API_TYPE_ROOM);
    coord_t const *region = luaL_checkudata(L, 2, API_TYPE_REGION2_INT8);
    portal_t portal;
    portal.direction = luaL_checkoption(L, 3, NULL, portalDirections);
    portal.targetRoom = target->room;
    memcpy(portal.region, region, sizeof(region2int8_t));
    Room_AddPortal(self->room, &portal);
    return 0;
}

static int f_hasOuterWalls(lua_State *L)
{
    roomWrapper_t const *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    coord_t const *region = luaL_checkudata(L, 2, API_TYPE_REGION2_INT8);
    FOR_TILES(region[0], region[1], region[2] + 1, region[3] + 1)
    {
        block_t const *block = Room_NavigateToCBlockAt(self->room, x, y);
        if (block->type == kBlockTypeBox)
        {
            lua_pushboolean(L, 1);
            return 1;
        }
    }
    lua_pushboolean(L, 0);
    return 0;
}

static int f_eq(lua_State *L)
{
    roomWrapper_t const *a = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    roomWrapper_t const *b = luaL_checkudata(L, 2, LEVEL_API_TYPE_ROOM);
    lua_pushboolean(L, a->room == b->room);
    return 1;
}

static int f_objectsIterator(lua_State *L)
{
    objectIterator_t *it = lua_touserdata(L, lua_upvalueindex(1));
    if (it->index < it->room->numObjects) {
        return lua_pushobject(L, it->room, it->index++);
    } else {
        return 0;
    }
}

static int f_getObjects(lua_State *L)
{
    roomWrapper_t *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    objectIterator_t *it = lua_newuserdata(L, sizeof(objectIterator_t));
    it->room = self->room;
    it->index = 0;
    lua_pushcclosure(L, f_objectsIterator, 1);
    return 1;
}

static int f_addObject(lua_State *L)
{
    roomWrapper_t *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    object_t obj;
    coord_t const *coord = luaL_checkudata(L, 3, API_TYPE_VECTOR3_INT8);
    obj.type = luaL_checkoption(L, 2, NULL, objecttypes);
    memcpy(obj.origin, coord, sizeof(vec3int8_t));
    object_t *obj2 = Room_AddObject(self->room, &obj);
    lua_pushobject(L, self->room, obj2 - self->room->objects);
    return 1;
}

extern int num_lines;
void Debug_GetLineAt(room_t const *room, int index, vec3_t a, vec3_t b);
static int f_getLine(lua_State *L)
{
    roomWrapper_t *self = luaL_checkudata(L, 1, LEVEL_API_TYPE_ROOM);
    int index = luaL_checknumber(L, 2);
    vec_t *a = lua_newuserdata(L, sizeof(vec3_t));
    luaL_setmetatable(L, API_TYPE_VECTOR3);
    vec_t *b = lua_newuserdata(L, sizeof(vec3_t));
    luaL_setmetatable(L, API_TYPE_VECTOR3);
    Debug_GetLineAt(self->room, index, a, b);
    return 2;
}

static int f_getNumLines(lua_State *L)
{
    lua_pushnumber(L, num_lines);
    return 1;
}

static const luaL_Reg lib[] =
{
    { "__eq", f_eq },
    { "changeHeight", f_changeHeight },
    { "setType", f_setType },
    { "getModel", f_getModel },
    { "getRoomView", f_getRoomView },
    { "getRoomCenter", f_getRoomCenter },
    { "getLightmap", f_getLightmap },
    { "setBlockAt", f_setBlockAt },
    { "getBlockAt", f_getBlockAt },
    { "getLocation", f_getLocation },
    { "setLocation", f_setLocation },
    { "getSelection", f_getSelection },
    { "setSelection", f_setSelection },
    { "addPortal", f_addPortal },
    { "raycastBlock", f_raycastBlock },
    { "raycastObjectPostion", f_raycastObjectPostion},
    { "setFaceTexCoord", f_setFaceTexCoord },
    { "buildGeometry", f_buildGeometry },
    { "getBounds", f_getBounds },
    { "getVerticalBounds", f_getVerticalBounds },
    { "hasOuterWalls", f_hasOuterWalls },
    { "crop", f_crop },
    { "addObject", f_addObject },
    { "getObjects", f_getObjects },
    { "getLine", f_getLine },
    { "getNumLines", f_getNumLines },
    { NULL, NULL }
};

int luaopen_level_room(lua_State *L)
{
    luaL_newmetatable(L, LEVEL_API_TYPE_ROOM);
    luaL_setfuncs(L, lib, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_getglobal(L, "INTERFACE");
    gi = lua_touserdata(L, -1);
    lua_pop(L, 1);
    return 1;
}
