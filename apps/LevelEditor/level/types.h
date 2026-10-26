#ifndef __TYPES_H__
#define __TYPES_H__

#include <code/shared.h>
#include <code/client/plugapi.h>
#include <code/renderer/r_public.h>

#define ROOM_SIZE 20
#define WORLD_SIZE 128
#define NUM_CORNERS 4
#define NUM_CLICKS 4
#define LIGHTMAP_RES 4 // pixels per quad
#define LIGHTMAP_SIZE 255
#define NUM_UV 2
#define BLOCK_SIZE 20
#define CLICK_SIZE (BLOCK_SIZE / NUM_CLICKS)
#define NUM_BLOCKS ROOM_SIZE *ROOM_SIZE
#define TEX_WIDTH 4
#define TEX_HEIGHT 16
#define CLICK_SCALE_U (1.f / (TEX_WIDTH * NUM_CLICKS))
#define CLICK_SCALE_V (1.f / (TEX_HEIGHT * NUM_CLICKS))
#define DEFAULT_CEILING 20
#define MAX_PORTALS 16
#define MAX_ROOMS 256
#define MAX_OBJECTS 256
#define NUM_SPLITS 5
#define EPS 0.0001

#define FOR_TILES(x_min, y_min, x_max, y_max)                   \
    for (int x = MAX(0, x_min); x < MIN(ROOM_SIZE, x_max); x++) \
        for (int y = MAX(0, y_min); y < MIN(ROOM_SIZE, y_max); y++)

#define FOR_ALL_TILES() FOR_TILES(0, 0, ROOM_SIZE, ROOM_SIZE)

#define SetBox2(out, a, b, c, d) \
    out[0] = a;                  \
    out[1] = b;                  \
    out[2] = c;                  \
    out[3] = d;

typedef enum
{
    kBlockLayerFloor,
    kBlockLayerCeiling,
    kBlockLayerEd,
    kBlockLayerRf,
    kBlockLayerCount
} blockLayer_t;

typedef enum // don't change order!
{
    kBlockFaceNegativeX,
    kBlockFacePositiveX = NUM_SPLITS,
    kBlockFaceNegativeY = NUM_SPLITS * 2,
    kBlockFacePositiveY = NUM_SPLITS * 3,
    kBlockFaceFloor = NUM_SPLITS * 4,
    kBlockFaceCeiling,
    kBlockFaceCount
} blockFace_t;

typedef enum
{
    kBlockTypeNone,
    kBlockTypeFloor,
    kBlockTypeWall,
    kBlockTypeBox,
    kBlockTypeCount,
} blockType_t;

typedef enum
{
    kBlockFlagNone = 0,
    kBlockFlagMonkey = 1,
    kBlockFlagBox = 2,
    kBlockFlagDeath = 4,
    kBlockFlagTrigger = 8,
    kBlockFlagNotWalkable = 16,
} blockFlag_t;

typedef enum
{
    kPortalDirectionFloor,
    kPortalDirectionCeiling,
    kPortalDirectionWallPositiveX,
    kPortalDirectionWallNegativeX,
    kPortalDirectionWallPositiveY,
    kPortalDirectionWallNegativeY,
} portalDirection_t;

typedef enum
{
    kObjectTypePointLight,
    kObjectTypeSunLight,
    kObjectTypeModel,
    kObjectTypeSoundSource,
} objectType_t;

typedef int8_t coord_t;
typedef uint8_t iuv_t[2];
typedef coord_t region2int8_t[4];
typedef coord_t vec2int8_t[2];
typedef coord_t vec3int8_t[3];
typedef iuv_t uvquad_t[4];

typedef struct
{
    vec_t u, v;
} uv_t;

typedef struct
{
    vec3_t a, b;
} line_t;

typedef struct
{
    vec3_t a, b, c;
} triangle_t;

typedef struct
{
    vec_t surface[4];
    vec4_t boundaries[4];
    vec3_t points[4];
    int numBoundaries;
} facet_t;

typedef struct
{
    vec3_t mins, maxs;
    vec3_t origin;
    facet_t *facets;
    int numFacets;
} surfaceTest_t;

typedef struct
{
    vec3_t hit;        // the impact point of a completely opaque surface
    float hitFraction; // 0 = at start, 1.0 = at end
} trace_t;

typedef struct
{
    vec3_t start, end;
    trace_t *trace;
} traceWork_t;

typedef struct
{
    color_t color;
    uint8_t intensity;
    uint8_t lightIn;
    uint8_t lightOut;
} light_t;

typedef struct
{
    uint8_t type;
    vec3int8_t origin;
    light_t light;
} object_t;

typedef struct
{
    coord_t height[kBlockLayerCount][NUM_CORNERS];
    uvquad_t uv[kBlockFaceCount];
    uint8_t type;
    uint8_t flags;
} block_t;

typedef struct
{
    struct room_s *targetRoom;
    uint8_t direction;
    region2int8_t region;
} portal_t;

typedef struct room_s
{
    uint8_t numPortals;
    uint8_t numObjects;
    vec2int8_t location;
    block_t blocks[ROOM_SIZE][ROOM_SIZE];
    portal_t portals[MAX_PORTALS];
    object_t objects[MAX_OBJECTS];
    region2int8_t selection;
    model_t *model;
    image_t *map;
    image_t *lightmap;
} room_t;

typedef struct
{
    room_t rooms[MAX_ROOMS];
    uint8_t numRooms;
} level_t;

int Box2_Contains(region2int8_t const box, int x, int y);
int Box2_AlmostContains(region2int8_t const box, int x, int y);

void Level_Init(level_t *level);
void Level_Release(level_t *level);
void Level_Load(level_t *level, char const *filename);
void Level_Save(level_t const *level, const char *filename);
void Level_RemoveRoom(level_t *level, room_t *room);
room_t *Level_AddRoom(level_t *level);

void Room_Init(room_t *room, coord_t floor, coord_t ceiling);
void Room_Release(room_t *room);
void Room_ChangeHeight(room_t *room, region2int8_t const region, blockLayer_t vertical, int value, int smooth, int direction);
void Room_SetBlockType(room_t *room, region2int8_t const region, blockType_t value);
void Room_StartSelection(room_t *room, int x, int y);
void Room_Crop(room_t *room, region2int8_t const region);
void Room_AddPortal(room_t *room, portal_t const *portal);
color_t Room_GetClickedBlock(room_t const *room, vec3_t const a, vec3_t const b);
void Room_GetBounds(room_t const *room, region2int8_t out);
void Room_GetVerticalBounds(room_t const *room, region2int8_t const region, vec2int8_t out);
void Room_Triangulate(room_t const *room, dsurface_t *dsurf, int splitQuads);
void Room_FillWalls(room_t const *room, dsurface_t *dsurf);
dsurface_t Room_AllocateModel(room_t const *room);
image_t *Room_AllocateTexture(room_t const *room);
image_t *Room_AllocateLightmap(dsurface_t const *dsurf);
block_t *Room_GetBlockAt(room_t *room, int x, int y);
block_t *Room_NavigateToBlockAt(room_t *room, int x, int y);
block_t const *Room_GetCBlockAt(room_t const *room, int x, int y);
block_t const *Room_NavigateToCBlockAt(room_t const *room, int x, int y);
portal_t const *Room_GetPortalAt(room_t const *room, int x, int y);
portal_t const *Room_GetPortalOfTypeAt(room_t const *room, portalDirection_t dir, int x, int y);
object_t *Room_GetObjectAtIndex(room_t *room, int index);
object_t *Room_AddObject(room_t *room, object_t const *object);

// misc
int GetIntersection(line_t const *line, triangle_t const *triangle, vec3_t out);
int PointOnPlane(line_t const *line, triangle_t const *triangle, vec3_t out);
color_t GetBlockTypeColor(blockType_t type);
color_t GetSelectedBlockColor();
color_t GetPortalBlockColor();
drawVert_t const *GetVertexAtEdge(dsurface_t const *dsurf, int index, int vertex);
int AddIndex(dsurface_t *dsurf, drawIndex_t index);
uint8_t CheckTexturing(uvquad_t const quad);
float Vector3Distance(vec3_t const a, vec3_t const b);
int PlaneFromPoints(vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c);
void GetVector3FromColor(color_t normal, vec3_t out);
void Vector3Lerp4(vec3_t const a, vec3_t const b, vec3_t const c, vec3_t const d, vec2_t const uv, vec3_t out);
void ColorLerp4(color_t a, color_t b, color_t c, color_t d, vec2_t const uv, vec3_t out);

extern struct game_import *gi;

#endif