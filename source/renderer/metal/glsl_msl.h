#ifndef __GLSL_MSL_H__
#define __GLSL_MSL_H__

// Offline GLSL to Metal Shading Language translation: the tool that produces
// the MSL sources and reflection tables in r_msl_shaders.h. Both stages are
// linked by glslang into SPIR-V and SPIRV-Cross emits MSL in the layout
// msl_program.h describes; the vertex stage's hidden MSL_FLIP_Y uniform is
// multiplied into gl_Position.y.

#include "msl_program.h"

#ifdef __cplusplus
extern "C" {
#endif

enum glsl_msl_platform
{
  GLSL_MSL_MACOS,
  GLSL_MSL_IOS,
};

// Translates a vertex/fragment GLSL pair to MSL. Returns 1 on success.
// Always call GLSL_FreeProgram afterwards (also after failure) to free log.
// In libglslmsl.a only: the engine does not call it, its shaders are MSL.
int GLSL_TranslateProgram(char const *vertex, char const *fragment,
                          enum glsl_msl_platform platform,
                          struct msl_program *out);
void GLSL_FreeProgram(struct msl_program *program);

#ifdef __cplusplus
}
#endif

#endif
