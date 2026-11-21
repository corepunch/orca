#ifndef __ORCA_METAL_H__
#define __ORCA_METAL_H__

// ORCA renderer backend — Metal implementation.
// Shaders are Metal Shading Language: mtlShaderSource takes a stage's MSL,
// and mtlLinkProgram reads the program's uniforms, textures and vertex
// attributes from Metal's reflection of the two functions; see msl_program.h.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

// A renderer enumerant: one of the MTL_* constants below.
typedef unsigned int mtlEnum;

#define MTL_FALSE 0
#define MTL_TRUE 1
#define MTL_NONE 0
#define MTL_ZERO 0
#define MTL_ONE 1

#define MTL_NO_ERROR 0
#define MTL_INVALID_ENUM 0x0500
#define MTL_INVALID_VALUE 0x0501
#define MTL_INVALID_OPERATION 0x0502
#define MTL_OUT_OF_MEMORY 0x0505
#define MTL_INVALID_FRAMEBUFFER_OPERATION 0x0506

#define MTL_DEPTH_BUFFER_BIT 0x00000100
#define MTL_STENCIL_BUFFER_BIT 0x00000400
#define MTL_COLOR_BUFFER_BIT 0x00004000

#define MTL_POINTS 0x0000
#define MTL_LINES 0x0001
#define MTL_LINE_STRIP 0x0003
#define MTL_TRIANGLES 0x0004
#define MTL_TRIANGLE_STRIP 0x0005

#define MTL_NEVER 0x0200
#define MTL_LESS 0x0201
#define MTL_EQUAL 0x0202
#define MTL_LEQUAL 0x0203
#define MTL_GREATER 0x0204
#define MTL_NOTEQUAL 0x0205
#define MTL_GEQUAL 0x0206
#define MTL_ALWAYS 0x0207

#define MTL_SRC_COLOR 0x0300
#define MTL_ONE_MINUS_SRC_COLOR 0x0301
#define MTL_SRC_ALPHA 0x0302
#define MTL_ONE_MINUS_SRC_ALPHA 0x0303
#define MTL_DST_ALPHA 0x0304
#define MTL_ONE_MINUS_DST_ALPHA 0x0305
#define MTL_DST_COLOR 0x0306
#define MTL_ONE_MINUS_DST_COLOR 0x0307

#define MTL_FRONT 0x0404
#define MTL_BACK 0x0405
#define MTL_FRONT_AND_BACK 0x0408
#define MTL_CW 0x0900
#define MTL_CCW 0x0901

#define MTL_CULL_FACE 0x0B44
#define MTL_DEPTH_TEST 0x0B71
#define MTL_STENCIL_TEST 0x0B90
#define MTL_VIEWPORT 0x0BA2
#define MTL_BLEND 0x0BE2
#define MTL_SCISSOR_TEST 0x0C11
#define MTL_UNPACK_ALIGNMENT 0x0CF5
#define MTL_PACK_ALIGNMENT 0x0D05
#define MTL_MAX_TEXTURE_SIZE 0x0D33
#define MTL_TEXTURE_2D 0x0DE1

#define MTL_KEEP 0x1E00
#define MTL_REPLACE 0x1E01
#define MTL_INCR 0x1E02
#define MTL_DECR 0x1E03
#define MTL_INVERT 0x150A
#define MTL_INCR_WRAP 0x8507
#define MTL_DECR_WRAP 0x8508

#define MTL_BYTE 0x1400
#define MTL_UNSIGNED_BYTE 0x1401
#define MTL_SHORT 0x1402
#define MTL_UNSIGNED_SHORT 0x1403
#define MTL_INT 0x1404
#define MTL_UNSIGNED_INT 0x1405
#define MTL_FLOAT 0x1406
#define MTL_HALF_FLOAT 0x140B
#define MTL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#define MTL_UNSIGNED_INT_24_8 0x84FA

#define MTL_COLOR 0x1800
#define MTL_DEPTH 0x1801
#define MTL_STENCIL 0x1802
#define MTL_DEPTH_COMPONENT 0x1902
#define MTL_RED 0x1903
#define MTL_GREEN 0x1904
#define MTL_BLUE 0x1905
#define MTL_ALPHA 0x1906
#define MTL_RGB 0x1907
#define MTL_RGBA 0x1908
#define MTL_LUMINANCE 0x1909
#define MTL_BGRA 0x80E1
#define MTL_R8 0x8229
#define MTL_RGB8 0x8051
#define MTL_RGBA8 0x8058
#define MTL_SRGB 0x8C40
#define MTL_SRGB8 0x8C41
#define MTL_SRGB8_ALPHA8 0x8C43
#define MTL_DEPTH_STENCIL 0x84F9
#define MTL_DEPTH24_STENCIL8 0x88F0
#define MTL_DEPTH_COMPONENT16 0x81A5
#define MTL_DEPTH_COMPONENT24 0x81A6
#define MTL_DEPTH_COMPONENT32F 0x8CAC

#define MTL_LINE 0x1B01
#define MTL_FILL 0x1B02

#define MTL_VENDOR 0x1F00
#define MTL_RENDERER 0x1F01
#define MTL_VERSION 0x1F02
#define MTL_EXTENSIONS 0x1F03
#define MTL_NUM_EXTENSIONS 0x821D

#define MTL_NEAREST 0x2600
#define MTL_LINEAR 0x2601
#define MTL_NEAREST_MIPMAP_NEAREST 0x2700
#define MTL_LINEAR_MIPMAP_NEAREST 0x2701
#define MTL_NEAREST_MIPMAP_LINEAR 0x2702
#define MTL_LINEAR_MIPMAP_LINEAR 0x2703
#define MTL_TEXTURE_MAG_FILTER 0x2800
#define MTL_TEXTURE_MIN_FILTER 0x2801
#define MTL_TEXTURE_WRAP_S 0x2802
#define MTL_TEXTURE_WRAP_T 0x2803
#define MTL_TEXTURE_WRAP_R 0x8072
#define MTL_REPEAT 0x2901
#define MTL_CLAMP_TO_EDGE 0x812F
#define MTL_MIRRORED_REPEAT 0x8370
#define MTL_TEXTURE_BASE_LEVEL 0x813C
#define MTL_TEXTURE_MAX_LEVEL 0x813D
#define MTL_TEXTURE_SWIZZLE_R 0x8E42
#define MTL_TEXTURE_SWIZZLE_G 0x8E43
#define MTL_TEXTURE_SWIZZLE_B 0x8E44
#define MTL_TEXTURE_SWIZZLE_A 0x8E45
#define MTL_TEXTURE_BINDING_2D 0x8069
#define MTL_TEXTURE_RECTANGLE 0x84F5
#define MTL_TEXTURE_CUBE_MAP 0x8513
#define MTL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define MTL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
#define MTL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
#define MTL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
#define MTL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
#define MTL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
#define MTL_TEXTURE0 0x84C0
#define MTL_ACTIVE_TEXTURE 0x84E0
#define MTL_SAMPLES 0x80A9

#define MTL_ARRAY_BUFFER 0x8892
#define MTL_ELEMENT_ARRAY_BUFFER 0x8893
#define MTL_PIXEL_UNPACK_BUFFER 0x88EC
#define MTL_STREAM_DRAW 0x88E0
#define MTL_STATIC_DRAW 0x88E4
#define MTL_DYNAMIC_DRAW 0x88E8
#define MTL_WRITE_ONLY 0x88B9

#define MTL_FRAGMENT_SHADER 0x8B30
#define MTL_VERTEX_SHADER 0x8B31
#define MTL_COMPILE_STATUS 0x8B81
#define MTL_LINK_STATUS 0x8B82
#define MTL_INFO_LOG_LENGTH 0x8B84
#define MTL_ACTIVE_UNIFORMS 0x8B86
#define MTL_ACTIVE_ATTRIBUTES 0x8B89

#define MTL_FLOAT_VEC2 0x8B50
#define MTL_FLOAT_VEC3 0x8B51
#define MTL_FLOAT_VEC4 0x8B52
#define MTL_INT_VEC2 0x8B53
#define MTL_INT_VEC3 0x8B54
#define MTL_INT_VEC4 0x8B55
#define MTL_BOOL 0x8B56
#define MTL_BOOL_VEC2 0x8B57
#define MTL_BOOL_VEC3 0x8B58
#define MTL_BOOL_VEC4 0x8B59
#define MTL_FLOAT_MAT2 0x8B5A
#define MTL_FLOAT_MAT3 0x8B5B
#define MTL_FLOAT_MAT4 0x8B5C
#define MTL_SAMPLER_2D 0x8B5E
#define MTL_SAMPLER_3D 0x8B5F
#define MTL_SAMPLER_CUBE 0x8B60
#define MTL_SAMPLER_2D_SHADOW 0x8B62
#define MTL_UNSIGNED_INT_VEC2 0x8DC6
#define MTL_UNSIGNED_INT_VEC3 0x8DC7
#define MTL_UNSIGNED_INT_VEC4 0x8DC8

#define MTL_FRAMEBUFFER 0x8D40
#define MTL_RENDERBUFFER 0x8D41
#define MTL_READ_FRAMEBUFFER 0x8CA8
#define MTL_DRAW_FRAMEBUFFER 0x8CA9
#define MTL_FRAMEBUFFER_BINDING 0x8CA6
#define MTL_DRAW_FRAMEBUFFER_BINDING 0x8CA6
#define MTL_READ_FRAMEBUFFER_BINDING 0x8CAA
#define MTL_RENDERBUFFER_BINDING 0x8CA7
#define MTL_COLOR_ATTACHMENT0 0x8CE0
#define MTL_DEPTH_ATTACHMENT 0x8D00
#define MTL_STENCIL_ATTACHMENT 0x8D20
#define MTL_DEPTH_STENCIL_ATTACHMENT 0x821A
#define MTL_FRAMEBUFFER_COMPLETE 0x8CD5
#define MTL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#define MTL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define MTL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING 0x8210
#define MTL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE 0x8216
#define MTL_FRAMEBUFFER_SRGB 0x8DB9

// Buffers
void mtlGenBuffers(int n, uint32_t *buffers);
void mtlDeleteBuffers(int n, uint32_t const *buffers);
void mtlBindBuffer(mtlEnum target, uint32_t buffer);
void mtlBufferData(mtlEnum target, ptrdiff_t size, void const *data, mtlEnum usage);
void *mtlMapBuffer(mtlEnum target, mtlEnum access);
bool mtlUnmapBuffer(mtlEnum target);

// Vertex arrays
void mtlGenVertexArrays(int n, uint32_t *arrays);
void mtlDeleteVertexArrays(int n, uint32_t const *arrays);
void mtlBindVertexArray(uint32_t array);
void mtlVertexAttribPointer(uint32_t index, int size, mtlEnum type, bool normalized,
                           int stride, void const *pointer);
void mtlEnableVertexAttribArray(uint32_t index);

// Textures
void mtlGenTextures(int n, uint32_t *textures);
void mtlDeleteTextures(int n, uint32_t const *textures);
void mtlActiveTexture(mtlEnum texture);
void mtlBindTexture(mtlEnum target, uint32_t texture);
void mtlTexParameteri(mtlEnum target, mtlEnum pname, int param);
void mtlPixelStorei(mtlEnum pname, int param);
void mtlTexImage2D(mtlEnum target, int level, int internalformat, int width, int height,
                  int border, mtlEnum format, mtlEnum type, void const *pixels);
void mtlTexSubImage2D(mtlEnum target, int level, int xoffset, int yoffset, int width,
                     int height, mtlEnum format, mtlEnum type, void const *pixels);
void mtlCompressedTexImage2D(mtlEnum target, int level, mtlEnum internalformat, int width,
                            int height, int border, int imageSize, void const *data);
void mtlCopyTexSubImage2D(mtlEnum target, int level, int xoffset, int yoffset, int x, int y,
                         int width, int height);

// Framebuffers and renderbuffers
void mtlGenFramebuffers(int n, uint32_t *framebuffers);
void mtlDeleteFramebuffers(int n, uint32_t const *framebuffers);
void mtlBindFramebuffer(mtlEnum target, uint32_t framebuffer);
void mtlFramebufferTexture2D(mtlEnum target, mtlEnum attachment, mtlEnum textarget, uint32_t texture, int level);
void mtlFramebufferRenderbuffer(mtlEnum target, mtlEnum attachment, mtlEnum renderbuffertarget, uint32_t renderbuffer);
mtlEnum mtlCheckFramebufferStatus(mtlEnum target);
void mtlGetFramebufferAttachmentParameteriv(mtlEnum target, mtlEnum attachment, mtlEnum pname, int *params);
void mtlGenRenderbuffers(int n, uint32_t *renderbuffers);
void mtlDeleteRenderbuffers(int n, uint32_t const *renderbuffers);
void mtlBindRenderbuffer(mtlEnum target, uint32_t renderbuffer);
void mtlRenderbufferStorage(mtlEnum target, mtlEnum internalformat, int width, int height);
void mtlBlitFramebuffer(int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0,
                       int dstX1, int dstY1, uint32_t mask, mtlEnum filter);
void mtlReadPixels(int x, int y, int width, int height, mtlEnum format, mtlEnum type, void *pixels);

// Shaders and programs
uint32_t mtlCreateShader(mtlEnum type);
void mtlDeleteShader(uint32_t shader);
void mtlShaderSource(uint32_t shader, int count, char const *const *string, int const *length);
void mtlCompileShader(uint32_t shader);
void mtlGetShaderiv(uint32_t shader, mtlEnum pname, int *params);
void mtlGetShaderInfoLog(uint32_t shader, int bufSize, int *length, char *infoLog);
uint32_t mtlCreateProgram(void);
void mtlDeleteProgram(uint32_t program);
void mtlAttachShader(uint32_t program, uint32_t shader);
void mtlDetachShader(uint32_t program, uint32_t shader);
void mtlLinkProgram(uint32_t program);
void mtlUseProgram(uint32_t program);
void mtlGetProgramiv(uint32_t program, mtlEnum pname, int *params);
void mtlGetProgramInfoLog(uint32_t program, int bufSize, int *length, char *infoLog);
void mtlGetActiveUniform(uint32_t program, uint32_t index, int bufSize, int *length, int *size,
                        mtlEnum *type, char *name);
int mtlGetUniformLocation(uint32_t program, char const *name);
int mtlGetAttribLocation(uint32_t program, char const *name);
void mtlUniform1f(int location, float v0);
void mtlUniform2f(int location, float v0, float v1);
void mtlUniform3f(int location, float v0, float v1, float v2);
void mtlUniform4f(int location, float v0, float v1, float v2, float v3);
void mtlUniform1i(int location, int v0);
void mtlUniform2fv(int location, int count, float const *value);
void mtlUniform3fv(int location, int count, float const *value);
void mtlUniform4fv(int location, int count, float const *value);
void mtlUniformMatrix3fv(int location, int count, bool transpose, float const *value);
void mtlUniformMatrix4fv(int location, int count, bool transpose, float const *value);

// State
void mtlEnable(mtlEnum cap);
void mtlDisable(mtlEnum cap);
void mtlBlendFunc(mtlEnum sfactor, mtlEnum dfactor);
void mtlColorMask(bool red, bool green, bool blue, bool alpha);
void mtlCullFace(mtlEnum mode);
void mtlDepthFunc(mtlEnum func);
void mtlDepthMask(bool flag);
void mtlStencilFunc(mtlEnum func, int ref, uint32_t mask);
void mtlStencilMask(uint32_t mask);
void mtlStencilOp(mtlEnum fail, mtlEnum zfail, mtlEnum zpass);
void mtlViewport(int x, int y, int width, int height);
void mtlScissor(int x, int y, int width, int height);
void mtlClearColor(float red, float green, float blue, float alpha);
void mtlClearDepthf(float d);
void mtlClearStencil(int s);
void mtlClear(uint32_t mask);
void mtlGetIntegerv(mtlEnum pname, int *data);
char const *mtlGetString(mtlEnum name);
char const *mtlGetStringi(mtlEnum name, uint32_t index);
mtlEnum mtlGetError(void);

// Drawing and submission
void mtlDrawArrays(mtlEnum mode, int first, int count);
void mtlDrawElements(mtlEnum mode, int count, mtlEnum type, void const *indices);
void mtlFlush(void);
void mtlFinish(void);

// R_BeginFrame opens an autorelease pool for the frame, which mtlPresent
// closes after presenting the drawable and submitting the frame.
void mtlBeginFrame(void);
void mtlPresent(void);
// Wraps an IOSurface as the texture bound to target. Returns 0 on success.
int mtlTexImageIOSurface(mtlEnum target, void *iosurface, int width, int height);

#ifdef __cplusplus
}
#endif

#endif
