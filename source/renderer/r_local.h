#ifndef __R_LOCAL_H__
#define __R_LOCAL_H__

#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <source/renderer/api/renderer.h>

#if __APPLE__
#define GL_SILENCE_DEPRECATION
#include <TargetConditionals.h>
#if !TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR
#include <OpenGL/gl3.h>
#else
#include <OpenGLES/ES3/gl.h>
#endif
// #elif __linux__
#else
#include <GLES3/gl3.h>
#endif

#define GetError()                                                             \
  {                                                                            \
    for (GLenum Error = glGetError(); (GL_NO_ERROR != Error);                  \
         Error = glGetError()) {                                               \
      switch (Error) {                                                         \
        case GL_INVALID_ENUM:                                                  \
          Con_Error("\n%s\n\n", "GL_INVALID_ENUM");                           \
          assert(0);                                                           \
          break;                                                               \
        case GL_INVALID_VALUE:                                                 \
          Con_Error("\n%s\n\n", "GL_INVALID_VALUE");                          \
          assert(0);                                                           \
          break;                                                               \
        case GL_INVALID_OPERATION:                                             \
          Con_Error("\n%s\n\n", "GL_INVALID_OPERATION");                      \
          assert(0);                                                           \
          break;                                                               \
        case GL_OUT_OF_MEMORY:                                                 \
          Con_Error("\n%s\n\n", "GL_OUT_OF_MEMORY");                          \
          assert(0);                                                           \
          break;                                                               \
        default:                                                               \
          break;                                                               \
      }                                                                        \
    }                                                                          \
  }

#define R_Call(func, ...)                                                      \
  func(__VA_ARGS__);                                                           \
  GetError();
#define MAKE_TEX_SIZE(width, height)                                           \
  (struct WI_Size)                                                              \
  {                                                                            \
    (width), (height)                                                          \
  }

#define WINDOWS_TABLE "__windows"

struct fontface;
struct Texture;
struct shader;
struct model;
struct shape;

enum
{
  ASTC_HEADER = 0x5CA1AB13,
  DDS_HEADER = 0x20534444,
  PVR_HEADER = 0x03525650,
  PNG_HEADER = 0x474E5089,
  SVG_HEADER = 0x6776733C, // <svg
  JPEG0_HEADER = 0xE0FFD8FF,
  JPEG1_HEADER = 0xE1FFD8FF,
};

enum shader_uniform
{
  kShaderUniform_ModelTransform,
  kShaderUniform_ModelViewTransform,
  kShaderUniform_ViewTransform,
  kShaderUniform_ProjectionTransform,
  kShaderUniform_ViewProjectionTransform,
  kShaderUniform_NormalTransform,
  kShaderUniform_NormalTransform4D,
  kShaderUniform_ModelViewProjectionTransform,
  kShaderUniform_TextureTransform,
  kShaderUniform_Opacity,
  kShaderUniform_Time,
  kShaderUniform_Viewport,
  kShaderUniform_CameraPosition,
  kShaderUniform_Texture,
  kShaderUniform_TextureSize,
	kShaderUniform_Color,
  kShaderUniform_Charset,
  kShaderUniform_Palette,
  kShaderUniform_CinematicPalette,
  kShaderUniform_MorphWeights,
  kShaderUniform_CursorPosition,
  kShaderUniform_Radius,
  kShaderUniform_BorderWidth,
  kShaderUniform_Rectangle,
  kShaderUniform_Lights,
  kShaderUniform_Count
};

enum cubmap_images
{
  kCubemapBackImage,
  kCubemapFrontImage,
  kCubemapLeftImage,
  kCubemapRightImage,
  kCubemapBottomImage,
  kCubemapTopImage,
  kCubemapNumImages,
};

struct shader_attr
{
  lpcString_t name;
  GLint size;
  GLenum type;
  GLboolean normalized;
  GLvoid const* ptr;
};

struct model
{
  uint32_t numVertices;
  uint32_t numIndices;
  uint32_t numSubmeshes;
  uint32_t vao;
  uint32_t indexBuffer;
  uint32_t vertexBuffers[VERTEX_SEMANTIC_COUNT];
  uint32_t attr[VERTEX_SEMANTIC_COUNT];
  DRAWSUBMESH* submeshes;
};

struct shader_universal_target
{
  bool_t HasBlendMode;
  bool_t HasDepthTestFunction;
  bool_t HasDepthWriteEnabled;
  enum blend_mode BlendMode;
  enum compare_func DepthTestFunction;
  bool_t DepthWriteEnabled;
};

struct shader
{
  path_t filename;
  struct _SHADERCONST *constants;
  struct shader_universal_target target;
  uint32_t numConstants;
  GLuint prognum;
  GLuint uniforms[kShaderUniform_Count];
  GLint attributes[VERTEX_SEMANTIC_COUNT];
};

#define MATERIAL_SIZE(NUM_UNIFORMS)                                            \
  (sizeof(struct _MATERIAL) + sizeof(struct uniform) * NUM_UNIFORMS)

struct _MATERIAL
{
  handle_t shader;
  enum blend_mode blendMode;
  uint32_t numUniforms;
  struct uniform uniforms[];
};

struct shape
{
  float lineLength;
  struct vec3 lineDirection;
};

#define MAX_TEXTCACHE 256

typedef struct _TEXTCACHE
{
  struct Texture* image;
  uint32_t value;
  uint32_t frame;
} TEXTCACHE, *PTEXTCACHE;

enum
{
  SHADER_DEFAULT,
  SHADER_UI,
  SHADER_VERTEXCOLOR,
  SHADER_ERROR,
  SHADER_CHARSET,
  SHADER_CINEMATIC,
#ifdef GL_SAMPLER_2D_RECT
  SHADER_RECTANGLE,
#endif
  SHADER_COUNT,
};

enum
{
  MD_RECTANGLE,
  MD_PLANE,
  MD_DOT,
  MD_ROUNDED_RECT,
  MD_COUNT
};

enum
{
  TX_WHITE,
  TX_BLACK,
  TX_CUBE,
  TX_CHARSET,
  TX_DEBUG,
  TX_PALETTE,
  TX_CINEMATIC,
  TX_CINEMATICPALETTE,
  TX_TOOLBAR,
  TX_COUNT,
};

typedef struct _CINEMATICBUF
{
  uint32_t pbo;
  uint32_t size;
} CINEMATICBUF;

struct lua_State;
struct managed_resource;

struct renderer
{
  uint32_t frame, drawCalls, buffer, vao;
  struct Texture* currentRenderTarget;
  TEXTCACHE textcache[MAX_TEXTCACHE];
  PIPELINESTATE state;
  CINEMATICBUF cinbuf;
  struct managed_resource* resources;
  struct Texture* textures[TX_COUNT];
  struct model* models[MD_COUNT];
  struct Shader shaders[SHADER_COUNT];
};

struct va_datatype_desc
{
  GLsizei size;
  GLenum type;
  GLboolean normalized;
};

#define MAX_SHADER_UNIFORMS 64

enum shader_precision {
  PRECISION_NOT_SET,
  PRECISION_LOW,
  PRECISION_MEDIUM,
  PRECISION_HIGH,
};

struct uniform_def
{
  shortStr_t Name;
  enum uniform_type Type;
  enum shader_precision Precision;
  uint32_t ArraySize;
  float Default[4];
};

struct shader_desc
{
  uint32_t Version;
  lpcString_t Name;
  enum shader_precision Precision;
  enum Shading Shading;
  lpcString_t FragmentOut;
  struct shader_universal_target UniversalTarget;
  struct uniform_def Uniforms[MAX_SHADER_UNIFORMS];
  struct uniform_def Attributes[MAX_SHADER_UNIFORMS];
  struct uniform_def Shared[MAX_SHADER_UNIFORMS];
  lpcString_t VertexShader;
  lpcString_t FragmentShader;
};

extern struct shader_desc shader_default;
extern struct shader_desc shader_ui;
extern struct shader_desc shader_rect;
extern struct shader_desc shader_charset;
extern struct shader_desc shader_cinematic;
extern struct shader_desc shader_vertexcolor;
extern struct shader_desc shader_error;

// font
struct fontface*
Font_Load(lpcString_t, lpFontFamily_t);

// text
struct Texture*
Text_GetImage(struct ViewText*);
HRESULT
Text_Print(struct ViewText const*, struct Texture**, bool_t /*reuse*/);

// shape
struct shape*
Shape_Load(lpcString_t);
HRESULT
Shape_Release(struct shape*);

// FT
void
FT_Init(void);
void
FT_Shutdown(void);

// font
HRESULT
Font_Release(struct fontface*);

// texture
HRESULT
Texture_Create(PCREATEIMGSTRUCT, struct Texture**);
void
Texture_Cleanup(struct Texture*);
HRESULT
Texture_CreateDefaultCube(struct Texture**);
void
Texture_Reallocate(struct Texture*, PCREATEIMGSTRUCT);
void
Texture_Bind(struct Texture const* image, GLenum target, GLint location, GLint unit);

// model
void
Model_Init(struct model*, PSURFATTR, PDRAWSURF);
HRESULT
Model_Release(struct model* model);
HRESULT
Model_Create(PSURFATTR, PDRAWSURF, struct model**);
HRESULT
Model_CreateBox(float w, float h, float d, struct model**);
HRESULT
Model_CreatePlane(float w, float h, struct model**);
HRESULT
Model_CreateSphere(float r, struct model**);
HRESULT
Model_CreateRectangle(struct rect const*,
                      struct rect const*,
                      enum vertex_order,
                      struct model**);
HRESULT
Model_CreateRoundedRectangle(struct model** ppModel);
HRESULT
Model_Save(struct model* mesh, lpcString_t filename);
HRESULT
Model_GenerateTangents(PDRAWSURF model);
struct model*
Model_CreateError(void);

// shader
HRESULT
Shader_LoadFromDef(struct shader_desc*, struct shader**);
void
Shader_BindMaterial(struct shader const*,
										struct ViewDef const*,
										struct ViewEntity const*);

void
Shader_BindConstants(struct shader const*,
										 struct ViewEntity const*);
void
Shader_BindAttributes(struct shader const*,
											struct model const*);
HRESULT
Shader_Release(struct shader*);

struct va_datatype_desc va_datatype_info(enum va_datatype);

struct shader_attr const* vertex_semantic_attributes(enum vertex_semantic);

// cinematic
HRESULT
Cin_Load(lpcString_t, uint32_t /*frame*/);

// misc
void
R_SetPointFiltering(void);
void
R_SetBlendMode(enum blend_mode blendMode);
void
R_SetDepthState(struct depth_state const* depth_state);

struct WI_Size
R_TexImage(GLenum target,
           struct WI_Buffer* sb,
           struct Texture* tex,
           struct Image* img);

struct WI_Size
texture_make_error(GLenum target);

extern struct renderer tr;

#endif
