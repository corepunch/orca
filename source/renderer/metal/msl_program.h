#ifndef __MSL_PROGRAM_H__
#define __MSL_PROGRAM_H__

// A Metal Shading Language program and its reflection: the two stage
// sources and where each uniform and vertex attribute lives in them.
//
// Loose uniforms of both stages are gathered into one uniform block per
// stage at buffer(MSL_UNIFORM_BUFFER); the reflection gives each uniform's
// byte offset in each stage's block. Vertex attributes keep their locations
// as [[attribute(n)]]. The vertex stage has the hidden uniform MSL_FLIP_Y,
// multiplied into the clip-space y, so offscreen targets can be rendered in
// bottom-up row order.

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MSL_UNIFORM_BUFFER 0
#define MSL_VERTEX_BUFFER_BASE 8
#define MSL_MAX_UNIFORMS 128
#define MSL_MAX_ATTRIBUTES 22
#define MSL_MAX_NAME 64
#define MSL_FLIP_Y "orca_FlipY"

enum msl_stage
{
  MSL_VERTEX,
  MSL_FRAGMENT,
  MSL_STAGES,
};

struct msl_uniform
{
  char name[MSL_MAX_NAME];
  uint32_t type;            // MTL_FLOAT_VEC4, MTL_SAMPLER_2D, ...
  uint32_t arraySize;       // 1 for a non-array uniform
  // Per stage; -1 where the stage does not declare the uniform.
  int32_t offset[MSL_STAGES];
  uint32_t arrayStride[MSL_STAGES];
  uint32_t matrixStride[MSL_STAGES];
  int32_t texture[MSL_STAGES];   // samplers: [[texture(n)]]
  int32_t sampler[MSL_STAGES];   // samplers: [[sampler(n)]]
};

struct msl_attribute
{
  char name[MSL_MAX_NAME];
  uint32_t location;
  uint32_t type;
};

struct msl_program
{
  char *source[MSL_STAGES];
  char entry[MSL_STAGES][MSL_MAX_NAME];
  uint32_t blockSize[MSL_STAGES];  // 0 when the stage has no uniforms
  uint32_t numUniforms;
  uint32_t numAttributes;
  struct msl_uniform uniforms[MSL_MAX_UNIFORMS];
  struct msl_attribute attributes[MSL_MAX_ATTRIBUTES];
  char *log;  // errors and warnings; may be set on success
};

#ifdef __cplusplus
}
#endif

#endif
