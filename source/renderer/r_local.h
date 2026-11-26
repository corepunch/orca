#ifndef __R_LOCAL_H__
#define __R_LOCAL_H__

#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <renderer/renderer.h>

#if ORCA_METAL
#include "metal/metal.h"
#include "metal/msl_program.h"
// The renderer is written in GL vocabulary; on Metal builds its types,
// constants and calls are the Metal backend's.
typedef mtlEnum GLenum;
typedef bool GLboolean;
typedef void GLvoid;
typedef int8_t GLbyte;
typedef uint32_t GLbitfield;
typedef int GLint;
typedef int GLsizei;
typedef uint32_t GLuint;
typedef float GLfloat;
typedef char GLchar;
typedef uint8_t GLubyte;
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;
#define GL_FALSE                                     MTL_FALSE
#define GL_TRUE                                      MTL_TRUE
#define GL_NONE                                      MTL_NONE
#define GL_ZERO                                      MTL_ZERO
#define GL_ONE                                       MTL_ONE
#define GL_NO_ERROR                                  MTL_NO_ERROR
#define GL_INVALID_ENUM                              MTL_INVALID_ENUM
#define GL_INVALID_VALUE                             MTL_INVALID_VALUE
#define GL_INVALID_OPERATION                         MTL_INVALID_OPERATION
#define GL_OUT_OF_MEMORY                             MTL_OUT_OF_MEMORY
#define GL_INVALID_FRAMEBUFFER_OPERATION             MTL_INVALID_FRAMEBUFFER_OPERATION
#define GL_DEPTH_BUFFER_BIT                          MTL_DEPTH_BUFFER_BIT
#define GL_STENCIL_BUFFER_BIT                        MTL_STENCIL_BUFFER_BIT
#define GL_COLOR_BUFFER_BIT                          MTL_COLOR_BUFFER_BIT
#define GL_POINTS                                    MTL_POINTS
#define GL_LINES                                     MTL_LINES
#define GL_LINE_STRIP                                MTL_LINE_STRIP
#define GL_TRIANGLES                                 MTL_TRIANGLES
#define GL_TRIANGLE_STRIP                            MTL_TRIANGLE_STRIP
#define GL_NEVER                                     MTL_NEVER
#define GL_LESS                                      MTL_LESS
#define GL_EQUAL                                     MTL_EQUAL
#define GL_LEQUAL                                    MTL_LEQUAL
#define GL_GREATER                                   MTL_GREATER
#define GL_NOTEQUAL                                  MTL_NOTEQUAL
#define GL_GEQUAL                                    MTL_GEQUAL
#define GL_ALWAYS                                    MTL_ALWAYS
#define GL_SRC_COLOR                                 MTL_SRC_COLOR
#define GL_ONE_MINUS_SRC_COLOR                       MTL_ONE_MINUS_SRC_COLOR
#define GL_SRC_ALPHA                                 MTL_SRC_ALPHA
#define GL_ONE_MINUS_SRC_ALPHA                       MTL_ONE_MINUS_SRC_ALPHA
#define GL_DST_ALPHA                                 MTL_DST_ALPHA
#define GL_ONE_MINUS_DST_ALPHA                       MTL_ONE_MINUS_DST_ALPHA
#define GL_DST_COLOR                                 MTL_DST_COLOR
#define GL_ONE_MINUS_DST_COLOR                       MTL_ONE_MINUS_DST_COLOR
#define GL_FRONT                                     MTL_FRONT
#define GL_BACK                                      MTL_BACK
#define GL_FRONT_AND_BACK                            MTL_FRONT_AND_BACK
#define GL_CW                                        MTL_CW
#define GL_CCW                                       MTL_CCW
#define GL_CULL_FACE                                 MTL_CULL_FACE
#define GL_DEPTH_TEST                                MTL_DEPTH_TEST
#define GL_STENCIL_TEST                              MTL_STENCIL_TEST
#define GL_VIEWPORT                                  MTL_VIEWPORT
#define GL_BLEND                                     MTL_BLEND
#define GL_SCISSOR_TEST                              MTL_SCISSOR_TEST
#define GL_UNPACK_ALIGNMENT                          MTL_UNPACK_ALIGNMENT
#define GL_PACK_ALIGNMENT                            MTL_PACK_ALIGNMENT
#define GL_MAX_TEXTURE_SIZE                          MTL_MAX_TEXTURE_SIZE
#define GL_TEXTURE_2D                                MTL_TEXTURE_2D
#define GL_KEEP                                      MTL_KEEP
#define GL_REPLACE                                   MTL_REPLACE
#define GL_INCR                                      MTL_INCR
#define GL_DECR                                      MTL_DECR
#define GL_INVERT                                    MTL_INVERT
#define GL_INCR_WRAP                                 MTL_INCR_WRAP
#define GL_DECR_WRAP                                 MTL_DECR_WRAP
#define GL_BYTE                                      MTL_BYTE
#define GL_UNSIGNED_BYTE                             MTL_UNSIGNED_BYTE
#define GL_SHORT                                     MTL_SHORT
#define GL_UNSIGNED_SHORT                            MTL_UNSIGNED_SHORT
#define GL_INT                                       MTL_INT
#define GL_UNSIGNED_INT                              MTL_UNSIGNED_INT
#define GL_FLOAT                                     MTL_FLOAT
#define GL_HALF_FLOAT                                MTL_HALF_FLOAT
#define GL_UNSIGNED_INT_8_8_8_8_REV                  MTL_UNSIGNED_INT_8_8_8_8_REV
#define GL_UNSIGNED_INT_24_8                         MTL_UNSIGNED_INT_24_8
#define GL_COLOR                                     MTL_COLOR
#define GL_DEPTH                                     MTL_DEPTH
#define GL_STENCIL                                   MTL_STENCIL
#define GL_DEPTH_COMPONENT                           MTL_DEPTH_COMPONENT
#define GL_RED                                       MTL_RED
#define GL_GREEN                                     MTL_GREEN
#define GL_BLUE                                      MTL_BLUE
#define GL_ALPHA                                     MTL_ALPHA
#define GL_RGB                                       MTL_RGB
#define GL_RGBA                                      MTL_RGBA
#define GL_LUMINANCE                                 MTL_LUMINANCE
#define GL_BGRA                                      MTL_BGRA
#define GL_R8                                        MTL_R8
#define GL_RGB8                                      MTL_RGB8
#define GL_RGBA8                                     MTL_RGBA8
#define GL_SRGB                                      MTL_SRGB
#define GL_SRGB8                                     MTL_SRGB8
#define GL_SRGB8_ALPHA8                              MTL_SRGB8_ALPHA8
#define GL_DEPTH_STENCIL                             MTL_DEPTH_STENCIL
#define GL_DEPTH24_STENCIL8                          MTL_DEPTH24_STENCIL8
#define GL_DEPTH_COMPONENT16                         MTL_DEPTH_COMPONENT16
#define GL_DEPTH_COMPONENT24                         MTL_DEPTH_COMPONENT24
#define GL_DEPTH_COMPONENT32F                        MTL_DEPTH_COMPONENT32F
#define GL_LINE                                      MTL_LINE
#define GL_FILL                                      MTL_FILL
#define GL_VENDOR                                    MTL_VENDOR
#define GL_RENDERER                                  MTL_RENDERER
#define GL_VERSION                                   MTL_VERSION
#define GL_EXTENSIONS                                MTL_EXTENSIONS
#define GL_NUM_EXTENSIONS                            MTL_NUM_EXTENSIONS
#define GL_NEAREST                                   MTL_NEAREST
#define GL_LINEAR                                    MTL_LINEAR
#define GL_NEAREST_MIPMAP_NEAREST                    MTL_NEAREST_MIPMAP_NEAREST
#define GL_LINEAR_MIPMAP_NEAREST                     MTL_LINEAR_MIPMAP_NEAREST
#define GL_NEAREST_MIPMAP_LINEAR                     MTL_NEAREST_MIPMAP_LINEAR
#define GL_LINEAR_MIPMAP_LINEAR                      MTL_LINEAR_MIPMAP_LINEAR
#define GL_TEXTURE_MAG_FILTER                        MTL_TEXTURE_MAG_FILTER
#define GL_TEXTURE_MIN_FILTER                        MTL_TEXTURE_MIN_FILTER
#define GL_TEXTURE_WRAP_S                            MTL_TEXTURE_WRAP_S
#define GL_TEXTURE_WRAP_T                            MTL_TEXTURE_WRAP_T
#define GL_TEXTURE_WRAP_R                            MTL_TEXTURE_WRAP_R
#define GL_REPEAT                                    MTL_REPEAT
#define GL_CLAMP_TO_EDGE                             MTL_CLAMP_TO_EDGE
#define GL_MIRRORED_REPEAT                           MTL_MIRRORED_REPEAT
#define GL_TEXTURE_BASE_LEVEL                        MTL_TEXTURE_BASE_LEVEL
#define GL_TEXTURE_MAX_LEVEL                         MTL_TEXTURE_MAX_LEVEL
#define GL_TEXTURE_SWIZZLE_R                         MTL_TEXTURE_SWIZZLE_R
#define GL_TEXTURE_SWIZZLE_G                         MTL_TEXTURE_SWIZZLE_G
#define GL_TEXTURE_SWIZZLE_B                         MTL_TEXTURE_SWIZZLE_B
#define GL_TEXTURE_SWIZZLE_A                         MTL_TEXTURE_SWIZZLE_A
#define GL_TEXTURE_BINDING_2D                        MTL_TEXTURE_BINDING_2D
#define GL_TEXTURE_RECTANGLE                         MTL_TEXTURE_RECTANGLE
#define GL_TEXTURE_CUBE_MAP                          MTL_TEXTURE_CUBE_MAP
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X               MTL_TEXTURE_CUBE_MAP_POSITIVE_X
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X               MTL_TEXTURE_CUBE_MAP_NEGATIVE_X
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y               MTL_TEXTURE_CUBE_MAP_POSITIVE_Y
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y               MTL_TEXTURE_CUBE_MAP_NEGATIVE_Y
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z               MTL_TEXTURE_CUBE_MAP_POSITIVE_Z
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z               MTL_TEXTURE_CUBE_MAP_NEGATIVE_Z
#define GL_TEXTURE0                                  MTL_TEXTURE0
#define GL_ACTIVE_TEXTURE                            MTL_ACTIVE_TEXTURE
#define GL_SAMPLES                                   MTL_SAMPLES
#define GL_ARRAY_BUFFER                              MTL_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER                      MTL_ELEMENT_ARRAY_BUFFER
#define GL_PIXEL_UNPACK_BUFFER                       MTL_PIXEL_UNPACK_BUFFER
#define GL_STREAM_DRAW                               MTL_STREAM_DRAW
#define GL_STATIC_DRAW                               MTL_STATIC_DRAW
#define GL_DYNAMIC_DRAW                              MTL_DYNAMIC_DRAW
#define GL_WRITE_ONLY                                MTL_WRITE_ONLY
#define GL_FRAGMENT_SHADER                           MTL_FRAGMENT_SHADER
#define GL_VERTEX_SHADER                             MTL_VERTEX_SHADER
#define GL_COMPILE_STATUS                            MTL_COMPILE_STATUS
#define GL_LINK_STATUS                               MTL_LINK_STATUS
#define GL_INFO_LOG_LENGTH                           MTL_INFO_LOG_LENGTH
#define GL_ACTIVE_UNIFORMS                           MTL_ACTIVE_UNIFORMS
#define GL_ACTIVE_ATTRIBUTES                         MTL_ACTIVE_ATTRIBUTES
#define GL_FLOAT_VEC2                                MTL_FLOAT_VEC2
#define GL_FLOAT_VEC3                                MTL_FLOAT_VEC3
#define GL_FLOAT_VEC4                                MTL_FLOAT_VEC4
#define GL_INT_VEC2                                  MTL_INT_VEC2
#define GL_INT_VEC3                                  MTL_INT_VEC3
#define GL_INT_VEC4                                  MTL_INT_VEC4
#define GL_BOOL                                      MTL_BOOL
#define GL_BOOL_VEC2                                 MTL_BOOL_VEC2
#define GL_BOOL_VEC3                                 MTL_BOOL_VEC3
#define GL_BOOL_VEC4                                 MTL_BOOL_VEC4
#define GL_FLOAT_MAT2                                MTL_FLOAT_MAT2
#define GL_FLOAT_MAT3                                MTL_FLOAT_MAT3
#define GL_FLOAT_MAT4                                MTL_FLOAT_MAT4
#define GL_SAMPLER_2D                                MTL_SAMPLER_2D
#define GL_SAMPLER_3D                                MTL_SAMPLER_3D
#define GL_SAMPLER_CUBE                              MTL_SAMPLER_CUBE
#define GL_SAMPLER_2D_SHADOW                         MTL_SAMPLER_2D_SHADOW
#define GL_UNSIGNED_INT_VEC2                         MTL_UNSIGNED_INT_VEC2
#define GL_UNSIGNED_INT_VEC3                         MTL_UNSIGNED_INT_VEC3
#define GL_UNSIGNED_INT_VEC4                         MTL_UNSIGNED_INT_VEC4
#define GL_FRAMEBUFFER                               MTL_FRAMEBUFFER
#define GL_RENDERBUFFER                              MTL_RENDERBUFFER
#define GL_READ_FRAMEBUFFER                          MTL_READ_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER                          MTL_DRAW_FRAMEBUFFER
#define GL_FRAMEBUFFER_BINDING                       MTL_FRAMEBUFFER_BINDING
#define GL_DRAW_FRAMEBUFFER_BINDING                  MTL_DRAW_FRAMEBUFFER_BINDING
#define GL_READ_FRAMEBUFFER_BINDING                  MTL_READ_FRAMEBUFFER_BINDING
#define GL_RENDERBUFFER_BINDING                      MTL_RENDERBUFFER_BINDING
#define GL_COLOR_ATTACHMENT0                         MTL_COLOR_ATTACHMENT0
#define GL_DEPTH_ATTACHMENT                          MTL_DEPTH_ATTACHMENT
#define GL_STENCIL_ATTACHMENT                        MTL_STENCIL_ATTACHMENT
#define GL_DEPTH_STENCIL_ATTACHMENT                  MTL_DEPTH_STENCIL_ATTACHMENT
#define GL_FRAMEBUFFER_COMPLETE                      MTL_FRAMEBUFFER_COMPLETE
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT         MTL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT MTL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING     MTL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE         MTL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE
#define GL_FRAMEBUFFER_SRGB                          MTL_FRAMEBUFFER_SRGB
#define glActiveTexture                       mtlActiveTexture
#define glAttachShader                        mtlAttachShader
#define glBindBuffer                          mtlBindBuffer
#define glBindFramebuffer                     mtlBindFramebuffer
#define glBindRenderbuffer                    mtlBindRenderbuffer
#define glBindTexture                         mtlBindTexture
#define glBindVertexArray                     mtlBindVertexArray
#define glBlendFunc                           mtlBlendFunc
#define glBlendFuncSeparate                   mtlBlendFuncSeparate
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
  kShaderUniform_LightsCount,
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
#if ORCA_METAL
  // Metal draws with MSL, not with the GLSL above: the source of each stage.
  // A definition without them has no program on a Metal build.
  lpcString_t MetalSource[MSL_STAGES];
#endif
};

// Names a built-in shader's two MSL stages (builtin/r_builtin_shaders_msl.h)
// in its shader_desc initializer; nothing on builds that compile the GLSL.
#if ORCA_METAL
#define SHADER_METAL(vertex, fragment) .MetalSource = { vertex, fragment },
#else
#define SHADER_METAL(vertex, fragment)
#endif

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

uint32_t
Shader_BuildDesc(struct Object *object, struct Shader const *shader,
                struct shader_desc *desc, bool_t *hasVertexStage);

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
