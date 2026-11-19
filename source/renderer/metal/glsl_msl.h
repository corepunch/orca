#ifndef __GLSL_MSL_H__
#define __GLSL_MSL_H__

// Load-time GLSL to Metal Shading Language translation for the Metal
// backend. Both stages are linked by glslang into SPIR-V and SPIRV-Cross
// emits MSL, so custom Shader resources keep their GLSL source.
//
// Loose uniforms of both stages are gathered into one uniform block per
// stage at buffer(GLSL_MSL_UNIFORM_BUFFER); the reflection below gives each
// uniform's byte offset in each stage's block. Vertex attributes keep their
// locations as [[attribute(n)]]. The vertex stage gains the hidden uniform
// GLSL_MSL_FLIP_Y, multiplied into gl_Position.y, so offscreen targets can be
// rendered in OpenGL's bottom-up row order.

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GLSL_MSL_UNIFORM_BUFFER 0
#define GLSL_MSL_VERTEX_BUFFER_BASE 8
#define GLSL_MSL_MAX_UNIFORMS 128
#define GLSL_MSL_MAX_ATTRIBUTES 22
#define GLSL_MSL_NAME 64
#define GLSL_MSL_FLIP_Y "orca_FlipY"

enum glsl_msl_stage
{
  GLSL_MSL_VERTEX,
  GLSL_MSL_FRAGMENT,
  GLSL_MSL_STAGES,
};

enum glsl_msl_platform
{
  GLSL_MSL_MACOS,
  GLSL_MSL_IOS,
};

struct glsl_msl_uniform
{
  char name[GLSL_MSL_NAME];
  uint32_t gltype;          // GL_FLOAT_VEC4, GL_SAMPLER_2D, ...
  uint32_t arraySize;       // 1 for a non-array uniform
  // Per stage; -1 where the stage does not declare the uniform.
  int32_t offset[GLSL_MSL_STAGES];
  uint32_t arrayStride[GLSL_MSL_STAGES];
  uint32_t matrixStride[GLSL_MSL_STAGES];
  int32_t texture[GLSL_MSL_STAGES];   // samplers: [[texture(n)]]
  int32_t sampler[GLSL_MSL_STAGES];   // samplers: [[sampler(n)]]
};

struct glsl_msl_attribute
{
  char name[GLSL_MSL_NAME];
  uint32_t location;
  uint32_t gltype;
};

struct glsl_msl_program
{
  char *source[GLSL_MSL_STAGES];
  char entry[GLSL_MSL_STAGES][GLSL_MSL_NAME];
  uint32_t blockSize[GLSL_MSL_STAGES];  // 0 when the stage has no uniforms
  uint32_t numUniforms;
  uint32_t numAttributes;
  struct glsl_msl_uniform uniforms[GLSL_MSL_MAX_UNIFORMS];
  struct glsl_msl_attribute attributes[GLSL_MSL_MAX_ATTRIBUTES];
  char *log;  // errors and warnings; may be set on success
};

// Translates a vertex/fragment pair. Returns 1 on success. Always call
// GLSL_FreeProgram afterwards, also after a failure, to free the log.
int GLSL_TranslateProgram(char const *vertex, char const *fragment,
                          enum glsl_msl_platform platform,
                          struct glsl_msl_program *out);
void GLSL_FreeProgram(struct glsl_msl_program *program);

#ifdef __cplusplus
}
#endif

#endif
