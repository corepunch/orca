#ifndef __R_LOCAL_H__
#define __R_LOCAL_H__

#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <renderer/renderer.h>

#if ORCA_METAL
#include "../metal/metal.h"
// Map GL call names to the Metal backend
#define glActiveTexture                       mtlActiveTexture
#define glAttachShader                        mtlAttachShader
#define glBindBuffer                          mtlBindBuffer
#define glBindFramebuffer                     mtlBindFramebuffer
#define glBindRenderbuffer                    mtlBindRenderbuffer
#define glBindTexture                         mtlBindTexture
#define glBindVertexArray                     mtlBindVertexArray
#define glBlendFunc                           mtlBlendFunc
#define glBlitFramebuffer                     mtlBlitFramebuffer
#define glBufferData                          mtlBufferData
#define glCheckFramebufferStatus              mtlCheckFramebufferStatus
#define glClear                               mtlClear
#define glClearColor                          mtlClearColor
#define glClearDepthf                         mtlClearDepthf
#define glClearStencil                        mtlClearStencil
#define glColorMask                           mtlColorMask
#define glCompileShader                       mtlCompileShader
#define glCompressedTexImage2D                mtlCompressedTexImage2D
#define glCopyTexSubImage2D                   mtlCopyTexSubImage2D
#define glCreateProgram                       mtlCreateProgram
#define glCreateShader                        mtlCreateShader
#define glCullFace                            mtlCullFace
#define glDeleteBuffers                       mtlDeleteBuffers
#define glDeleteFramebuffers                  mtlDeleteFramebuffers
#define glDeleteProgram                       mtlDeleteProgram
#define glDeleteRenderbuffers                 mtlDeleteRenderbuffers
#define glDeleteShader                        mtlDeleteShader
#define glDeleteTextures                      mtlDeleteTextures
#define glDeleteVertexArrays                  mtlDeleteVertexArrays
#define glDepthFunc                           mtlDepthFunc
#define glDepthMask                           mtlDepthMask
#define glDetachShader                        mtlDetachShader
#define glDisable                             mtlDisable
#define glDrawArrays                          mtlDrawArrays
#define glDrawElements                        mtlDrawElements
#define glEnable                              mtlEnable
#define glEnableVertexAttribArray              mtlEnableVertexAttribArray
#define glFinish                              mtlFinish
#define glFlush                               mtlFlush
#define glFramebufferRenderbuffer             mtlFramebufferRenderbuffer
#define glFramebufferTexture2D                mtlFramebufferTexture2D
#define glGenBuffers                          mtlGenBuffers
#define glGenFramebuffers                     mtlGenFramebuffers
#define glGenRenderbuffers                    mtlGenRenderbuffers
#define glGenTextures                         mtlGenTextures
#define glGenVertexArrays                     mtlGenVertexArrays
#define glGetActiveUniform                    mtlGetActiveUniform
#define glGetAttribLocation                   mtlGetAttribLocation
#define glGetError                            mtlGetError
#define glGetFramebufferAttachmentParameteriv mtlGetFramebufferAttachmentParameteriv
#define glGetIntegerv                         mtlGetIntegerv
#define glGetProgramInfoLog                   mtlGetProgramInfoLog
#define glGetProgramiv                        mtlGetProgramiv
#define glGetShaderInfoLog                    mtlGetShaderInfoLog
#define glGetShaderiv                         mtlGetShaderiv
#define glGetString                           mtlGetString
#define glGetStringi                          mtlGetStringi
#define glGetUniformLocation                  mtlGetUniformLocation
#define glLinkProgram                         mtlLinkProgram
#define glMapBuffer                           mtlMapBuffer
#define glPixelStorei                         mtlPixelStorei
#define glProgramLabel                        mtlProgramLabel
#define glReadPixels                          mtlReadPixels
#define glRenderbufferStorage                 mtlRenderbufferStorage
#define glScissor                             mtlScissor
#define glShaderSource                        mtlShaderSource
#define glStencilFunc                         mtlStencilFunc
#define glStencilMask                         mtlStencilMask
#define glStencilOp                           mtlStencilOp
#define glTexImage2D                          mtlTexImage2D
#define glTexParameteri                       mtlTexParameteri
#define glTexSubImage2D                       mtlTexSubImage2D
#define glUniform1f                           mtlUniform1f
#define glUniform1i                           mtlUniform1i
#define glUniform2f                           mtlUniform2f
#define glUniform2fv                          mtlUniform2fv
#define glUniform3f                           mtlUniform3f
#define glUniform3fv                          mtlUniform3fv
#define glUniform4f                           mtlUniform4f
#define glUniform4fv                          mtlUniform4fv
#define glUniformMatrix3fv                    mtlUniformMatrix3fv
#define glUniformMatrix4fv                    mtlUniformMatrix4fv
#define glUnmapBuffer                         mtlUnmapBuffer
#define glUseProgram                          mtlUseProgram
#define glVertexAttribPointer                 mtlVertexAttribPointer
#define glViewport                            mtlViewport
#define GLMTL_BeginFrame                      mtlBeginFrame
#define GLMTL_Present                         mtlPresent
#define GLMTL_TexImageIOSurface               mtlTexImageIOSurface
#elif __APPLE__
//#define GL_SILENCE_DEPRECATION
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

// Every GL call is counted for frame statistics. glGetError after each call
// is a driver round trip, so it runs only with ORCA_GL_DEBUG=1; R_EndFrame
// still reports an error once per frame without it.
#define R_Call(func, ...)                                                      \
  func(__VA_ARGS__);                                                           \
  tr.glCalls++;                                                                \
  if (tr.checkErrors) GetError();
#define MAKE_TEX_SIZE(width, height)                                           \
  (struct AXsize)                                                              \
  {                                                                            \
    (width), (height)                                                          \
  }

// The window's framebuffer. With OpenGL the platform owns and binds it; the
// Metal layer's framebuffer 0 is the drawable of the window's CAMetalLayer.
static inline void
R_BindDefaultFramebuffer(void)
{
#if ORCA_METAL
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
#else
  axBindFramebuffer();
#endif
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
  kShaderUniform_BBoxMin,
  kShaderUniform_BBoxMax,
  kShaderUniform_Lights,
  kShaderUniform_SceneColor,
  kShaderUniform_SceneDepth,
  kShaderUniform_SceneTextureSize,
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
  uint32_t vaoSerial;   // shader serial the VAO's attribute pointers were set for
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
  uint32_t serial;      // unique per linked program, never reused
  // Per-view uniforms uploaded last; program uniforms persist, so equal
  // values need no upload on the next entity.
  bool_t viewValid;
  struct mat4 viewMatrix, projectionMatrix;
  struct rect viewport;
};

struct shape
{
  float lineLength;
  struct vec3 lineDirection;
};

// Shader and model enums are now in public renderer.h header

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
  uint32_t frame, drawCalls, glCalls, buffer, vao;
  bool_t checkErrors;          // ORCA_GL_DEBUG=1: glGetError after every call
  uint32_t program;            // bound program, 0 when unknown
  uint32_t shaderSerial;       // last serial handed to a linked shader
  int blendMode;               // -1 when unknown
  int depthWrite;              // -1 when unknown
  struct Texture* currentRenderTarget;
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
extern struct shader_desc shader_ninepatch;
extern struct shader_desc shader_vertexcolor;
extern struct shader_desc shader_error;
extern struct shader_desc shader_button;
extern struct shader_desc shader_roundedbox;
extern struct shader_desc shader_blur_h;
extern struct shader_desc shader_blur_v;
extern struct shader_desc shader_backdrop_blur;

#define SHADER_MAX_TEXTS 256
#define SHADER_TEXT_LENGTH 256

uint32_t
Shader_AssembleSource(struct shader_desc const* def, bool_t vertex,
                      char texts[][SHADER_TEXT_LENGTH], lpcString_t *codes);

// Gives a fragment-only Shader the built-in UI vertex stage (shader_ui): its
// source, attributes, varyings and engine uniforms after the first
// `numUniforms` material uniforms. FALSE if the uniforms don't all fit.
bool_t
Shader_UseUIVertexStage(struct shader_desc *desc, uint32_t numUniforms);

// font
struct fontface*
Font_Load(lpcString_t, struct FontFamily *);

// shape
struct shape*
Shape_Load(lpcString_t);
HRESULT
Shape_Release(struct shape*);

// font rasterizer
void
Font_Shutdown(void);

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
Model_CreateRoundedBorder(struct model** ppModel);
HRESULT
Model_CreateRoundedBox(float width, float height, float depth, float radius, struct model** ppModel);
HRESULT
Model_CreateCapsule(float width, float height, float depth, struct model** ppModel);
HRESULT
Model_Save(struct model* mesh, lpcString_t filename);
HRESULT
Model_GenerateTangents(PDRAWSURF model);
struct model*
Model_CreateError(void);

// shader
HRESULT
Shader_LoadFromDef(struct shader_desc*, struct shader**);

// The GL program of a shader, building a Shader object's program on first
// use (loading a Shader makes no GL calls). NULL if it has none.
struct shader *
Shader_GetProgram(struct Shader const *);

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
void
R_SetDepthWrite(bool_t write);

struct AXsize
R_TexImage(GLenum target,
           struct AXbuffer* sb,
           struct Texture* tex,
           struct Image* img);

struct AXsize
texture_make_error(GLenum target);

void*
texture_downscale(void const* src, int w, int h, int bpp, int dw, int dh);

extern struct renderer tr;

#endif
