#include <math.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <include/orca.h>
#include <include/renderer.h>
#include <renderer/renderer.h>
#if __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GLES3/gl3.h>
#endif
#include "test_local.h"

extern int luaopen_orca(lua_State *L);
extern int luaopen_orca_core(lua_State *L);
extern int luaopen_orca_renderer(lua_State *L);

int main(void)
{
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  luaL_requiref(L, "orca", luaopen_orca, 1);
  lua_pop(L, 1);
  luaopen_orca_core(L);
  lua_pop(L, 1);
  luaopen_orca_renderer(L);
  lua_pop(L, 1);
  renderer_Init(64, 64, TRUE);

  struct ScreenSpaceCapture capture = {0};
  GLint original_fbo;
  GLuint verify_fbo = 0;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &original_fbo);
  // Scene target with colour and depth, like a window back buffer; the
  // offscreen platform surface has colour only.
  GLuint scene_fbo, scene_color, scene_depth;
  glGenFramebuffers(1, &scene_fbo);
  glGenTextures(1, &scene_color);
  glGenRenderbuffers(1, &scene_depth);
  glBindTexture(GL_TEXTURE_2D, scene_color);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glBindRenderbuffer(GL_RENDERBUFFER, scene_depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 64, 64);
  glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene_color, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, scene_depth);
  glViewport(0, 0, 64, 64);
  glClearColor(0.23, 0.41, 0.67, 1.0);
  glClearDepthf(0.375);
  glDepthMask(GL_TRUE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  unsigned char before[4], after[4];
  glReadPixels(8, 8, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, before);

  RUN_TEST("scene_color_and_depth_snapshot", {
    EXPECT(SSR_CaptureFrame(&capture));
    EXPECT(capture.Color->Width == 64 && capture.Color->Height == 64);
    EXPECT(capture.Depth->Width == 64 && capture.Depth->Height == 64);
    glGenFramebuffers(1, &verify_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, verify_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, capture.Color->texnum, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, capture.Depth->texnum, 0);
    EXPECT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glReadPixels(8, 8, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, after);
    EXPECT(memcmp(before, after, sizeof(before)) == 0);
#if __APPLE__
    float depth = 0;
    glReadPixels(8, 8, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
    EXPECT(fabsf(depth - 0.375f) < 0.00001f);
#endif
    EXPECT(glGetError() == GL_NO_ERROR);
  });
  glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);
  if (verify_fbo) glDeleteFramebuffers(1, &verify_fbo);

  RUN_TEST("scene_snapshot_resizes_to_physical_viewport", {
    glViewport(0, 0, 32, 16);
    EXPECT(SSR_CaptureFrame(&capture));
    EXPECT(capture.Color->Width == 32 && capture.Color->Height == 16);
    EXPECT(capture.Depth->Width == 32 && capture.Depth->Height == 16);
    glViewport(0, 0, 0, 0);
    EXPECT(!SSR_CaptureFrame(&capture));
    EXPECT(glGetError() == GL_NO_ERROR);
  });
  SSR_Release(&capture);
  glBindFramebuffer(GL_FRAMEBUFFER, original_fbo);
  glDeleteFramebuffers(1, &scene_fbo);
  glDeleteTextures(1, &scene_color);
  glDeleteRenderbuffers(1, &scene_depth);
  renderer_Shutdown();
  lua_close(L);
  printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
  return s_tests_failed ? 1 : 0;
}
