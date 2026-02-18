#ifndef __r_public_h__
#define __r_public_h__

#include <include/orca.h>

#define RF_USE_FONT_HEIGHT (1 << 0)
#define RF_CAMERA_ALIAS (1 << 2)
#define RF_DRAW_DEBUG (1 << 3)

#define RF_CTRL_PRESSED (1 << 2)
#define RF_CTRL_DISABLED (1 << 3)
#define RF_CTRL_HOVERED (1 << 4)
#define RF_CTRL_FOCUSED (1 << 5)
#define RF_CTRL_VALUE (1 << 6)
#define RF_CTRL_SCALE (1 << 7)§
#define RF_CTRL_INTERIOR (1 << 8)
#define RF_CTRL_MIXED (1 << 9)

#define MAX_VIEW_LIGHTS 16

#define DEPTH_BUFFER_BIT 0x00000100
#define STENCIL_BUFFER_BIT 0x00000400
#define COLOR_BUFFER_BIT 0x00004000

struct Texture;
struct shader;
struct fontface;
struct model;
struct shape;
struct user_data;
struct ViewText;

typedef void* handle_t;
typedef uint32_t DRAWINDEX, *PDRAWINDEX;
typedef uint32_t DRAWSURFATTR, *PSURFATTR;

struct color32 { unsigned char r, g, b, a; };

enum vertex_order
{
  VERTEX_ORDER_CW,
  VERTEX_ORDER_CCW,
  VERTEX_ORDER_DEFAULT = VERTEX_ORDER_CW,
};

enum blend_mode
{
  BLEND_MODE_INHERIT,
  BLEND_MODE_OPAQUE,
  BLEND_MODE_ALPHA,
  BLEND_MODE_ADDITIVE,
  BLEND_MODE_PREMULTIPLIED_ALPHA,
  BLEND_MODE_MIXED_ALPHA,
};

/// List of color write modes.
enum color_write_mode
{
  COLOR_WRITE_MODE_RGBA,
  COLOR_WRITE_MODE_RGB,
  COLOR_WRITE_MODE_R,
  COLOR_WRITE_MODE_G,
  COLOR_WRITE_MODE_B,
  COLOR_WRITE_MODE_GB,
  COLOR_WRITE_MODE_A,
  COLOR_WRITE_MODE_NONE,
};

enum uniform_type
{
  UT_NONE,
  UT_FLOAT,
  UT_FLOAT_VEC2,
  UT_FLOAT_VEC3,
  UT_FLOAT_VEC4,
  UT_FLOAT_MAT3,
  UT_FLOAT_MAT4,
  UT_INT,
  UT_BOOL,
  UT_UNSIGNED_INT,
  UT_SAMPLER_2D,
  UT_SAMPLER_2D_RECT,
  UT_SAMPLER_CUBE,
  UT_COLOR,
};

enum cull_mode
{
  CULL_MODE_BACK,
  CULL_MODE_FRONT,
  CULL_MODE_NONE,
};

enum light_mode
{
  LIGHT_MODE_FORWARD_BASE,
  LIGHT_MODE_FORWARD_ADD,
  LIGHT_MODE_SHADOW_CASTER
};

enum compare_func
{
  COMPARE_FUNC_NEVER,
  COMPARE_FUNC_ALWAYS,
  COMPARE_FUNC_LESS,
  COMPARE_FUNC_LEQUAL,
  COMPARE_FUNC_GREATER,
  COMPARE_FUNC_GEQUAL,
  COMPARE_FUNC_EQUAL,
  COMPARE_FUNC_NOTEQUAL,
  COMPARE_FUNC_DISABLED
};

enum stencil_op
{
  STENCIL_OP_KEEP,
  STENCIL_OP_ZERO,
  STENCIL_OP_REPLACE,
  STENCIL_OP_INCREMENT,
  STENCIL_OP_INCREMENTWRAP,
  STENCIL_OP_DECREMENT,
  STENCIL_OP_DECREMENTWRAP,
  STENCIL_OP_INVERT
};

enum vertex_semantic
{
  VERTEX_SEMANTIC_POSITION,        // 0
  VERTEX_SEMANTIC_NORMAL,          // 1
  VERTEX_SEMANTIC_TANGENT,         // 2
  VERTEX_SEMANTIC_BINORMAL,        // 3
  VERTEX_SEMANTIC_COLOR,           // 4
  VERTEX_SEMANTIC_TEXCOORD0,       // 5
  VERTEX_SEMANTIC_TEXCOORD1,       // 6
  VERTEX_SEMANTIC_TEXCOORD2,       // 7
  VERTEX_SEMANTIC_TEXCOORD3,       // 8
  VERTEX_SEMANTIC_TEXCOORD4,       // 9
  VERTEX_SEMANTIC_WEIGHT,          // 10
  VERTEX_SEMANTIC_MATRIX_INDICES,  // 11
  VERTEX_SEMANTIC_MORPH0_POSITION, // 12
  VERTEX_SEMANTIC_MORPH1_POSITION, // 13
  VERTEX_SEMANTIC_MORPH2_POSITION, // 14
  VERTEX_SEMANTIC_MORPH3_POSITION, // 15
  VERTEX_SEMANTIC_MORPH0_NORMAL,   // 16
  VERTEX_SEMANTIC_MORPH1_NORMAL,   // 17
  VERTEX_SEMANTIC_MORPH2_NORMAL,   // 18
  VERTEX_SEMANTIC_MORPH3_NORMAL,   // 19
  VERTEX_SEMANTIC_MORPH0_TANGENT,  // 20
  VERTEX_SEMANTIC_MORPH1_TANGENT,  // 21
  VERTEX_SEMANTIC_MORPH2_TANGENT,  // 22
  VERTEX_SEMANTIC_MORPH3_TANGENT,  // 23
  VERTEX_SEMANTIC_USERDATA,        // 24
  VERTEX_SEMANTIC_COUNT,
};

enum va_datatype
{
  VERTEX_ATTR_DATATYPE_UNDEFINED,
  VERTEX_ATTR_DATATYPE_FLOAT16,
  VERTEX_ATTR_DATATYPE_FLOAT32,
  VERTEX_ATTR_DATATYPE_UINT8,
  VERTEX_ATTR_DATATYPE_INT8,
  VERTEX_ATTR_DATATYPE_NORMALIZED = 64,
};

struct stencil_state
{
  uint32_t mask;
  uint32_t ref;
  bool_t write;
  enum compare_func func;
  enum stencil_op fail;
  enum stencil_op zfail;
  enum stencil_op zpass;
};

struct depth_state
{
  bool_t write;
  enum compare_func func;
};

typedef struct _PIPELINESTATE
{
  enum blend_mode blend_mode;
  enum color_write_mode color_write_mode;
  enum cull_mode cull_mode;
  struct depth_state depth;
  struct stencil_state stencil;
  struct rect viewport;
  struct rect scissor;
} PIPELINESTATE, *PPIPELINESTATE;

struct image_info
{
  uint32_t bmWidth;
  uint32_t bmHeight;
};

struct text_info
{
  uint32_t txWidth;
  uint32_t txHeight;
  struct edges txInsets;
  struct rect cursor;
};

struct morph_target
{
  struct vec3 position;
  struct vec3 normal;
  struct vec3 tangent;
};

typedef struct _DRAWVERT
{
  struct vec3 xyz;
  struct vec3 normal;
  struct vec3 tangent;
  struct vec3 binormal;
  struct vec2 texcoord[5];
  struct vec4 weight;
  struct vec4 matrix_indices;
  struct morph_target morph_target[4];
  struct color32 color;
  struct color32 userdata;
} DRAWVERT, *PDRAWVERT;

typedef struct _DRAWSUBMESH
{
  uint32_t start;
  uint32_t numIndices;
} DRAWSUBMESH, *PDRAWSUBMESH;

typedef struct _DRAWLINESSTRUCT
{
  struct mat4 matrix;
  struct color color;
  struct vec3 const* points;
  uint32_t numPoints;
} DRAWLINESSTRUCT, *PDRAWLINESSTRUCT;

struct uniform
{
  enum uniform_type Type;
  uint32_t Identifier;
  float Value[4];
};

// Boxed mesh type enum - cast to struct Mesh const* for use in ViewEntity.mesh
enum boxed_mesh_type {
  BOXED_MESH_RECTANGLE = 1,
  BOXED_MESH_TEAPOT = 2,
  BOXED_MESH_PLANE = 3,
  BOXED_MESH_DOT = 4,
  BOXED_MESH_CAPSULE = 5,
  BOXED_MESH_ROUNDED_BOX = 6,
  BOXED_MESH_NINEPATCH = 7,
  BOXED_MESH_CINEMATIC = 8,
};

// Visual appearance properties for rendering.
// Note: This struct intentionally does NOT contain uniforms because uniforms
// are aggregated from multiple sources (both object properties and material
// properties). See DESIGN_DECISION_UNIFORMS.md for full rationale.
struct ViewMaterial
{
  struct Texture const* texture;
  struct mat3 textureMatrix;
  struct color color;
  struct uniform const* uniforms;      // Shader parameters from object + material
  uint32_t numUniforms;                // Count of shader parameters
  enum blend_mode blendMode;
  float opacity;
};

// Helper macro to convert rect (x, y, width, height) to box3 (min, max)
#define BOX3_FromRect(r) \
  ((struct box3){ \
    .min = { (r).x, (r).y, 0 }, \
    .max = { (r).x + (r).width, (r).y + (r).height, 0 } \
  })

// Complete rendering descriptor for an entity, aggregating all data needed for rendering.
// Note: uniforms are stored here (not in ViewMaterial) because they aggregate shader
// parameters from multiple sources: both object properties AND material properties.
// See DESIGN_DECISION_UNIFORMS.md for full rationale.
//
// Mesh pointer boxing: The mesh field holds either a real Mesh pointer or a boxed
// entity type constant (MESH_RECTANGLE, MESH_CAPSULE, etc.). Use BOX_IS_PTR() to check.
// Example: ent.mesh = MESH_CAPSULE;
struct ViewEntity
{
  struct mat4 matrix;
  struct ViewMaterial material;
  struct ViewText* text;
  struct Mesh const* mesh;
  struct Shader const* shader;
  struct box3 bbox;
  struct vec4 radius;
  struct vec4 borderWidth;
  float borderOffset;
  uint32_t submesh;
  uint32_t flags;
  uint32_t frame;
  struct mat4 ninepatch; // also has other usecases
};

enum light_type {
  LIGHT_POINT,
  LIGHT_SPOT,
};

struct view_light // shall be size of mat4
{
  struct vec3 xyz;
  enum light_type type;
  struct color color;
  struct vec2 angles;
  float intensity;
  float range;
  struct vec4 padding;
};

struct view_camera
{
  uint32_t identifier;
  uint32_t alias;
  uint32_t alias2;
  struct mat4 matrix;
  float fov;
  float zNear;
  float zFar;
  float eyeSeparation;
  float focalDistance;
  bool_t zPositive;
  bool_t verticalFOV;
};

struct ViewDef
{
  struct mat4 projectionMatrix;
  struct mat4 viewMatrix;
  struct vec2 viewSize;
  struct vec2 screenSize;
  lpObject_t renderPass;
  lpObject_t viewport;
  struct view_light lights[MAX_VIEW_LIGHTS];
  float stereoSeparation;
  uint32_t flags, camera, num_lights;
};

struct _SHADERCONST
{
  uint32_t identifier;
  enum uniform_type type;
	bool_t builtin;
  uint32_t location;
  uint32_t systemType;
  float defaults[4];
};

struct _SHADERUNIFORM
{
  uint32_t identifier;
  enum uniform_type type;
};

typedef void* PCONSOLEBUFFER;

typedef struct _DRAWSURF
{
  PDRAWVERT vertices;
  PDRAWINDEX indices;
  PDRAWSUBMESH submeshes;
  PDRAWINDEX neighbors;
  uint32_t numVertices;
  uint32_t numIndices;
  uint32_t numSubmeshes;
} DRAWSURF, *PDRAWSURF;

#include <source/renderer/api/renderer.h>

typedef struct _LOADIMGSTRUCT
{
  lpcString_t source;
  bool_t premultiply_alpha;
  uint32_t scale;
  enum TextureFilter MinFilter;
  enum TextureFilter MagFilter;
  enum TextureWrap WrapMode;
} LOADIMGSTRUCT, *PLOADIMGSTRUCT;

typedef struct _CREATEIMGSTRUCT
{
  uint32_t Width;
  uint32_t Height;
  uint32_t Scale;
  enum TextureFormat Format;
  void const* ImageData;
  enum TextureFilter MinFilter;
  enum TextureFilter MagFilter;
  enum TextureWrap WrapMode;
} CREATEIMGSTRUCT, *PCREATEIMGSTRUCT;

typedef struct _CREATERTSTRUCT
{
  uint32_t Width;
  uint32_t Height;
  uint32_t Scale;
} CREATERTSTRUCT, *PCREATERTSTRUCT;

typedef struct _DRAWCONSOLESTRUCT
{
  PCONSOLEBUFFER Buffer;
  uint32_t Width;
  uint32_t Height;
  uint16_t SelectedItem;
  uint16_t Cursor;
  struct vec2 Scroll;
  struct rect Rect;
  bool_t SoftSelection;
  bool_t StickHeader;
  bool_t DrawShadow;
//  bool_t DrawOutline;
} DRAWCONSOLESTRUCT, *PDRAWCONSOLESTRUCT;

typedef struct _DRAWTOOLBARICONSTRUCT
{
  float x;
  float y;
  uint8_t icon;
} DRAWTOOLBARICONSTRUCT, *PDRAWTOOLBARICONSTRUCT;

//typedef struct _DRAWCONSOLESTRUCT2
//{
//  uint32_t Width;
//  uint32_t Height;
//  uint32_t Attribute;
//  uint32_t Cursor;
//  uint32_t MaxPosition;
//  uint32_t DrawOutline;
//  uint32_t DrawShadow;
//  struct rect Rect;
//  struct vec2 Scroll;
//  uint32_t StickHeader;
//  uint32_t Buffer[];
//} DRAWCONSOLESTRUCT2, *PTEXTWINDOW;

typedef struct _DRAWIMAGESTRUCT
{
  handle_t img;
  struct rect rect;
  struct rect uv;
  struct color color;
} DRAWIMAGESTRUCT, *PDRAWIMAGESTRUCT;

typedef enum
{
  FS_NORMAL,
  FS_BOLD,
  FS_ITALIC,
  FS_BOLD_ITALIC,
  FS_COUNT,
} FontStyle;

struct ViewTextRun
{
  lpcString_t string;
  lpcFontFamily_t fontFamily; // NOTE: must be second field for R_GetTextHash()
  FontStyle fontStyle;
  uint32_t fontSize;
  uint32_t fixedCharacterWidth;
  uint32_t underlineWidth;
  uint32_t underlineOffset;
  float letterSpacing;
  float lineSpacing;
};

#define MAX_TEXT_RUNS 256

struct ViewText
{
  uint32_t flags;
  uint32_t availableWidth;
  uint32_t cursor;
  uint32_t numTextRuns;
  float scale;
  struct ViewTextRun run[];
};

// typedef struct screen_rect {
//     int x, y, w, h;
// } SCREENRECT, *PSCREENRECT;

enum ServerWindowProperties
{
  kServerWindowRootView,
  kServerWindowWidth,
  kServerWindowHeight,
};

//CLASS_TYPE(ServerWindow, 0x195326fe, Object)
//{
//  lpObject_t RootView;
//  uint32_t Width;
//  uint32_t Height;
//};

typedef struct _DRAWSTRETCHRAWSTRUCT
{
  struct rect rect;
  struct WI_Size size;
  struct color32* palette;
  byte_t* data;
  float opacity;
  bool_t refresh;
} DRAWSTRETCHRAWSTRUCT, *PDRAWSTRETCHRAWSTRUCT;

// #define IID_RENDERER "IRENDERER"
#define IID_WINDOW "IWINDOW"

typedef void (*SHADERENUMPROC)(struct _SHADERCONST *, void*);

ORCA_API HRESULT R_Init(uint32_t width, uint32_t height, bool_t offscreen);
ORCA_API HRESULT R_Shutdown(void);
ORCA_API HRESULT R_DrawLines(struct ViewDef const*, PDRAWLINESSTRUCT);
ORCA_API HRESULT R_DrawEntity(struct ViewDef const*, struct ViewEntity*);
ORCA_API HRESULT R_DrawConsole(PDRAWCONSOLESTRUCT);
ORCA_API HRESULT R_DrawImage(PDRAWIMAGESTRUCT);
ORCA_API HRESULT R_DrawToolbarIcon(PDRAWTOOLBARICONSTRUCT parm);
//ORCA_API HRESULT R_DrawTextWindow(PTEXTWINDOW);
ORCA_API HRESULT R_ClearScreen(struct color color, float depth, byte_t stencil);
ORCA_API HRESULT R_SetPipelineState(PPIPELINESTATE);
ORCA_API HRESULT R_GetPipelineState(PPIPELINESTATE);
ORCA_API HRESULT R_BindFramebuffer(struct Texture*);
ORCA_API HRESULT RenderTexture_Create(PCREATERTSTRUCT, struct Texture**);
ORCA_API HRESULT Text_GetInfo(struct ViewText const*, struct text_info*);
ORCA_API HRESULT Image_GetInfo(struct Texture const*, struct image_info*);
ORCA_API HRESULT Shader_EnumUniforms(struct shader const*, SHADERENUMPROC, void*);
ORCA_API HRESULT Shape_GetPointLocation(struct shape const*, float, struct vec3*);
ORCA_API HRESULT UserData_Create(lpcString_t, uint32_t, void**);

// Window operations

//struct windowAPI {
//  int (*WI_GetScaling)(void);
//  void (*WI_SetSize)(int width, int height);
//  struct WI_Size (*WI_GetSize)(void);
//};
//
// Resource operations
ORCA_API HRESULT Texture_Release(struct Texture*);
ORCA_API lpObject_t R_LoadImageFromMemory(lua_State *L, void*, uint32_t);
ORCA_API HRESULT R_BeginFrame(struct color);
ORCA_API HRESULT R_EndFrame(void);
ORCA_API void R_SetViewportRect(struct rect const *rect);
ORCA_API void R_SetScissorRect(struct rect const *rect);

ORCA_API HRESULT R_CreateIOSurfaceTexture(uint32_t, struct Texture**);

//ORCA_API bool_t WI_SetSize(uint32_t width, uint32_t height, bool_t centered);
//ORCA_API bool_t WI_GetSize(struct WI_Size * pSize);
//ORCA_API int WI_GetScaling(void);

#endif
