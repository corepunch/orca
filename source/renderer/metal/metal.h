#ifndef __ORCA_METAL_H__
#define __ORCA_METAL_H__

// ORCA renderer backend — Metal implementation.
// Programs use hand-written MSL shaders with pre-computed reflection;
// see source/renderer/metal/r_msl_shaders.h.

#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef signed char GLbyte;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;

#define GL_FALSE 0
#define GL_TRUE 1
#define GL_NONE 0
#define GL_ZERO 0
#define GL_ONE 1

#define GL_NO_ERROR 0
#define GL_INVALID_ENUM 0x0500
#define GL_INVALID_VALUE 0x0501
#define GL_INVALID_OPERATION 0x0502
#define GL_OUT_OF_MEMORY 0x0505
#define GL_INVALID_FRAMEBUFFER_OPERATION 0x0506

#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_COLOR_BUFFER_BIT 0x00004000

#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_LINE_STRIP 0x0003
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005

#define GL_NEVER 0x0200
#define GL_LESS 0x0201
#define GL_EQUAL 0x0202
#define GL_LEQUAL 0x0203
#define GL_GREATER 0x0204
#define GL_NOTEQUAL 0x0205
#define GL_GEQUAL 0x0206
#define GL_ALWAYS 0x0207

#define GL_SRC_COLOR 0x0300
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DST_ALPHA 0x0304
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#define GL_DST_COLOR 0x0306
#define GL_ONE_MINUS_DST_COLOR 0x0307

#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_FRONT_AND_BACK 0x0408
#define GL_CW 0x0900
#define GL_CCW 0x0901

#define GL_CULL_FACE 0x0B44
#define GL_DEPTH_TEST 0x0B71
#define GL_STENCIL_TEST 0x0B90
#define GL_VIEWPORT 0x0BA2
#define GL_BLEND 0x0BE2
#define GL_SCISSOR_TEST 0x0C11
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_PACK_ALIGNMENT 0x0D05
#define GL_MAX_TEXTURE_SIZE 0x0D33
#define GL_TEXTURE_2D 0x0DE1

#define GL_KEEP 0x1E00
#define GL_REPLACE 0x1E01
#define GL_INCR 0x1E02
#define GL_DECR 0x1E03
#define GL_INVERT 0x150A
#define GL_INCR_WRAP 0x8507
#define GL_DECR_WRAP 0x8508

#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406
#define GL_HALF_FLOAT 0x140B
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#define GL_UNSIGNED_INT_24_8 0x84FA

#define GL_COLOR 0x1800
#define GL_DEPTH 0x1801
#define GL_STENCIL 0x1802
#define GL_DEPTH_COMPONENT 0x1902
#define GL_RED 0x1903
#define GL_GREEN 0x1904
#define GL_BLUE 0x1905
#define GL_ALPHA 0x1906
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_LUMINANCE 0x1909
#define GL_BGRA 0x80E1
#define GL_R8 0x8229
#define GL_RGB8 0x8051
#define GL_RGBA8 0x8058
#define GL_SRGB 0x8C40
#define GL_SRGB8 0x8C41
#define GL_SRGB8_ALPHA8 0x8C43
#define GL_DEPTH_STENCIL 0x84F9
#define GL_DEPTH24_STENCIL8 0x88F0
#define GL_DEPTH_COMPONENT16 0x81A5
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_DEPTH_COMPONENT32F 0x8CAC

#define GL_LINE 0x1B01
#define GL_FILL 0x1B02

#define GL_VENDOR 0x1F00
#define GL_RENDERER 0x1F01
#define GL_VERSION 0x1F02
#define GL_EXTENSIONS 0x1F03
#define GL_NUM_EXTENSIONS 0x821D

#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_TEXTURE_WRAP_R 0x8072
#define GL_REPEAT 0x2901
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_MIRRORED_REPEAT 0x8370
#define GL_TEXTURE_BASE_LEVEL 0x813C
#define GL_TEXTURE_MAX_LEVEL 0x813D
#define GL_TEXTURE_SWIZZLE_R 0x8E42
#define GL_TEXTURE_SWIZZLE_G 0x8E43
#define GL_TEXTURE_SWIZZLE_B 0x8E44
#define GL_TEXTURE_SWIZZLE_A 0x8E45
#define GL_TEXTURE_BINDING_2D 0x8069
#define GL_TEXTURE_RECTANGLE 0x84F5
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
#define GL_TEXTURE0 0x84C0
#define GL_ACTIVE_TEXTURE 0x84E0
#define GL_SAMPLES 0x80A9

#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_PIXEL_UNPACK_BUFFER 0x88EC
#define GL_STREAM_DRAW 0x88E0
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_WRITE_ONLY 0x88B9

#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_ACTIVE_UNIFORMS 0x8B86
#define GL_ACTIVE_ATTRIBUTES 0x8B89

#define GL_FLOAT_VEC2 0x8B50
#define GL_FLOAT_VEC3 0x8B51
#define GL_FLOAT_VEC4 0x8B52
#define GL_INT_VEC2 0x8B53
#define GL_INT_VEC3 0x8B54
#define GL_INT_VEC4 0x8B55
#define GL_BOOL 0x8B56
#define GL_BOOL_VEC2 0x8B57
#define GL_BOOL_VEC3 0x8B58
#define GL_BOOL_VEC4 0x8B59
#define GL_FLOAT_MAT2 0x8B5A
#define GL_FLOAT_MAT3 0x8B5B
#define GL_FLOAT_MAT4 0x8B5C
#define GL_SAMPLER_2D 0x8B5E
#define GL_SAMPLER_3D 0x8B5F
#define GL_SAMPLER_CUBE 0x8B60
#define GL_SAMPLER_2D_SHADOW 0x8B62
#define GL_UNSIGNED_INT_VEC2 0x8DC6
#define GL_UNSIGNED_INT_VEC3 0x8DC7
#define GL_UNSIGNED_INT_VEC4 0x8DC8

#define GL_FRAMEBUFFER 0x8D40
#define GL_RENDERBUFFER 0x8D41
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#define GL_FRAMEBUFFER_BINDING 0x8CA6
#define GL_DRAW_FRAMEBUFFER_BINDING 0x8CA6
#define GL_READ_FRAMEBUFFER_BINDING 0x8CAA
#define GL_RENDERBUFFER_BINDING 0x8CA7
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_STENCIL_ATTACHMENT 0x8D20
#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING 0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE 0x8216
#define GL_FRAMEBUFFER_SRGB 0x8DB9

// Buffers
void mtlGenBuffers(GLsizei n, GLuint *buffers);
void mtlDeleteBuffers(GLsizei n, GLuint const *buffers);
void mtlBindBuffer(GLenum target, GLuint buffer);
void mtlBufferData(GLenum target, GLsizeiptr size, void const *data, GLenum usage);
void *mtlMapBuffer(GLenum target, GLenum access);
GLboolean mtlUnmapBuffer(GLenum target);

// Vertex arrays
void mtlGenVertexArrays(GLsizei n, GLuint *arrays);
void mtlDeleteVertexArrays(GLsizei n, GLuint const *arrays);
void mtlBindVertexArray(GLuint array);
void mtlVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized,
                           GLsizei stride, void const *pointer);
void mtlEnableVertexAttribArray(GLuint index);

// Textures
void mtlGenTextures(GLsizei n, GLuint *textures);
void mtlDeleteTextures(GLsizei n, GLuint const *textures);
void mtlActiveTexture(GLenum texture);
void mtlBindTexture(GLenum target, GLuint texture);
void mtlTexParameteri(GLenum target, GLenum pname, GLint param);
void mtlPixelStorei(GLenum pname, GLint param);
void mtlTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
                  GLint border, GLenum format, GLenum type, void const *pixels);
void mtlTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                     GLsizei height, GLenum format, GLenum type, void const *pixels);
void mtlCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width,
                            GLsizei height, GLint border, GLsizei imageSize, void const *data);
void mtlCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y,
                         GLsizei width, GLsizei height);

// Framebuffers and renderbuffers
void mtlGenFramebuffers(GLsizei n, GLuint *framebuffers);
void mtlDeleteFramebuffers(GLsizei n, GLuint const *framebuffers);
void mtlBindFramebuffer(GLenum target, GLuint framebuffer);
void mtlFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
void mtlFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
GLenum mtlCheckFramebufferStatus(GLenum target);
void mtlGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params);
void mtlGenRenderbuffers(GLsizei n, GLuint *renderbuffers);
void mtlDeleteRenderbuffers(GLsizei n, GLuint const *renderbuffers);
void mtlBindRenderbuffer(GLenum target, GLuint renderbuffer);
void mtlRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
void mtlBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0,
                       GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
void mtlReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);

// Shaders and programs
GLuint mtlCreateShader(GLenum type);
void mtlDeleteShader(GLuint shader);
void mtlShaderSource(GLuint shader, GLsizei count, GLchar const *const *string, GLint const *length);
void mtlCompileShader(GLuint shader);
void mtlGetShaderiv(GLuint shader, GLenum pname, GLint *params);
void mtlGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GLuint mtlCreateProgram(void);
void mtlDeleteProgram(GLuint program);
void mtlAttachShader(GLuint program, GLuint shader);
void mtlDetachShader(GLuint program, GLuint shader);
void mtlLinkProgram(GLuint program);
// Label a program with its shader name so mtlLinkProgram can find its MSL
// reflection. Call before mtlLinkProgram.
void mtlProgramLabel(GLuint program, char const *name);
void mtlUseProgram(GLuint program);
void mtlGetProgramiv(GLuint program, GLenum pname, GLint *params);
void mtlGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
void mtlGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size,
                        GLenum *type, GLchar *name);
GLint mtlGetUniformLocation(GLuint program, GLchar const *name);
GLint mtlGetAttribLocation(GLuint program, GLchar const *name);
void mtlUniform1f(GLint location, GLfloat v0);
void mtlUniform2f(GLint location, GLfloat v0, GLfloat v1);
void mtlUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
void mtlUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void mtlUniform1i(GLint location, GLint v0);
void mtlUniform2fv(GLint location, GLsizei count, GLfloat const *value);
void mtlUniform3fv(GLint location, GLsizei count, GLfloat const *value);
void mtlUniform4fv(GLint location, GLsizei count, GLfloat const *value);
void mtlUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, GLfloat const *value);
void mtlUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, GLfloat const *value);

// State
void mtlEnable(GLenum cap);
void mtlDisable(GLenum cap);
void mtlBlendFunc(GLenum sfactor, GLenum dfactor);
void mtlColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void mtlCullFace(GLenum mode);
void mtlDepthFunc(GLenum func);
void mtlDepthMask(GLboolean flag);
void mtlStencilFunc(GLenum func, GLint ref, GLuint mask);
void mtlStencilMask(GLuint mask);
void mtlStencilOp(GLenum fail, GLenum zfail, GLenum zpass);
void mtlViewport(GLint x, GLint y, GLsizei width, GLsizei height);
void mtlScissor(GLint x, GLint y, GLsizei width, GLsizei height);
void mtlClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void mtlClearDepthf(GLfloat d);
void mtlClearStencil(GLint s);
void mtlClear(GLbitfield mask);
void mtlGetIntegerv(GLenum pname, GLint *data);
GLubyte const *mtlGetString(GLenum name);
GLubyte const *mtlGetStringi(GLenum name, GLuint index);
GLenum mtlGetError(void);

// Drawing and submission
void mtlDrawArrays(GLenum mode, GLint first, GLsizei count);
void mtlDrawElements(GLenum mode, GLsizei count, GLenum type, void const *indices);
void mtlFlush(void);
void mtlFinish(void);

// R_BeginFrame opens an autorelease pool for the frame, which mtlPresent
// closes after presenting the drawable and submitting the frame.
void mtlBeginFrame(void);
void mtlPresent(void);
// Wraps an IOSurface as the texture bound to target. Returns 0 on success.
int mtlTexImageIOSurface(GLenum target, void *iosurface, GLsizei width, GLsizei height);

#ifdef __cplusplus
}
#endif

#endif
