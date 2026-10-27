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

// Draws red and blue halves into a 64x64 target and filters the backdrop of
// a bar along the top. EXPECT breaks out of the checks; cleanup always runs.
static void
test_backdrop_filter(void)
{
  GLint previous_fbo, bound_fbo;
  GLuint fbo, color, depth;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous_fbo);
  glGenFramebuffers(1, &fbo);
  glGenTextures(1, &color);
  glGenRenderbuffers(1, &depth);
  glBindTexture(GL_TEXTURE_2D, color);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glBindRenderbuffer(GL_RENDERBUFFER, depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 64, 64);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth);
  // Pipeline rects are in logical pixels; size them to 64 physical pixels.
  PIPELINESTATE state;
  GLint viewport[4];
  R_GetPipelineState(&state);
  state.viewport = state.scissor = (struct rect){0, 0, 64, 64};
  R_SetPipelineState(&state);
  glGetIntegerv(GL_VIEWPORT, viewport);
  float size = 64 * 64.f / viewport[2];
  state.viewport = state.scissor = (struct rect){0, 0, size, size};
  R_SetPipelineState(&state);
  glClearColor(1, 1, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  struct BackdropCache *cache = NULL;
  do {
    // Red on the left half and blue on the right, under a bar along the top.
    struct ViewDef view = {
      .viewMatrix = MAT4_Identity(),
      .projectionMatrix = MAT4_Ortho(0, 64, 64, 0, -1, 1),
    };
    struct ViewEntity half = {
      .matrix = MAT4_Identity(),
      .bbox = {.min = {0, 0, 0}, .max = {32, 64, 0}},
      .material = {.textureMatrix = MAT3_Identity(), .color = {1, 0, 0, 1}, .opacity = 1,
                   .blendMode = BLEND_MODE_OPAQUE},
    };
    EXPECT_OK(R_DrawEntity(&view, &half));
    half.bbox = (struct box3){.min = {32, 0, 0}, .max = {64, 64, 0}};
    half.material.color = (struct color){0, 0, 1, 1};
    EXPECT_OK(R_DrawEntity(&view, &half));

    struct ViewEntity bar = {
      .matrix = MAT4_Identity(),
      .bbox = {.min = {0, 0, 0}, .max = {64, 16, 0}},
      .material = {.textureMatrix = MAT3_Identity(), .opacity = 1, .blendMode = BLEND_MODE_OPAQUE},
    };
    unsigned char row[64 * 4], below[4];
    EXPECT_OK(R_FilterBackdrop(&cache, &view, &bar, 4, 1));
    EXPECT(bar.material.texture != NULL);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &bound_fbo);
    EXPECT(bound_fbo == (GLint)fbo);
    EXPECT_OK(R_DrawEntity(&view, &bar));
    glReadPixels(0, 56, 64, 1, GL_RGBA, GL_UNSIGNED_BYTE, row);
    glReadPixels(31, 8, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, below);
    // The blur mixes the halves at the seam; mirrored edges keep the sides pure.
    EXPECT(row[4 * 31] > 60 && row[4 * 31 + 2] > 60);
    EXPECT(row[4 * 32] > 60 && row[4 * 32 + 2] > 60);
    EXPECT(row[4 * 2] > 200 && row[4 * 2 + 2] < 60);
    EXPECT(row[4 * 61] < 60 && row[4 * 61 + 2] > 200);
    EXPECT(below[0] == 255 && below[2] == 0);

    // Zero saturation leaves gray at the luminance of red.
    EXPECT_OK(R_FilterBackdrop(&cache, &view, &bar, 0, 0));
    EXPECT_OK(R_DrawEntity(&view, &bar));
    glReadPixels(0, 56, 64, 1, GL_RGBA, GL_UNSIGNED_BYTE, row);
    EXPECT(abs(row[4 * 2] - row[4 * 2 + 1]) < 3 && abs(row[4 * 2 + 1] - row[4 * 2 + 2]) < 3);
    EXPECT(row[4 * 2] > 30 && row[4 * 2] < 90);

    bar.bbox = (struct box3){.min = {100, 100, 0}, .max = {120, 120, 0}};
    EXPECT(FAILED(R_FilterBackdrop(&cache, &view, &bar, 4, 1)));
  } while (0);
  R_ReleaseBackdrop(cache);
  do { EXPECT(glGetError() == GL_NO_ERROR); } while (0);
  glBindFramebuffer(GL_FRAMEBUFFER, previous_fbo);
  glDeleteFramebuffers(1, &fbo);
  glDeleteTextures(1, &color);
  glDeleteRenderbuffers(1, &depth);
}

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

  RUN_TEST("solid_image_uses_initialized_white_texture", {
    unsigned char pixel[4] = {0};
    R_BeginFrame((struct color){0, 0, 0, 1});
    R_DrawImage(&(DRAWIMAGESTRUCT){
      .rect = {0, 0, 64, 64},
      .uv = {0, 0, 1, 1},
      .color = {1, 0, 0, 1},
    });
    glReadPixels(8, 8, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    EXPECT(pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 0);
    EXPECT(glGetError() == GL_NO_ERROR);
  });

  RUN_TEST("square_ui_uses_two_triangles_without_changing_pixels", {
    struct ViewDef view = ((struct ViewDef){
      .viewMatrix = MAT4_Identity(),
      .projectionMatrix = MAT4_Ortho(0, 64, 64, 0, -1, 1),
    });
    struct ViewEntity entity = ((struct ViewEntity){
      .matrix = MAT4_Identity(),
      .bbox = {.min = {8, 8, 0}, .max = {56, 56, 0}},
      .material = {
        .textureMatrix = MAT3_Identity(),
        .color = {1, 0, 0, 1},
        .opacity = 0.5f,
        .blendMode = BLEND_MODE_ALPHA,
      },
    });
    unsigned char square[64 * 64 * 4];
    unsigned char reference[sizeof(square)];
    GLint square_indices = 0;
    GLint rounded_indices = 0;
    R_BeginFrame((struct color){0, 0, 0, 1});
    EXPECT_OK(R_DrawEntity(&view, &entity));
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &square_indices);
    glReadPixels(0, 0, 64, 64, GL_RGBA, GL_UNSIGNED_BYTE, square);
    EXPECT(square_indices == 6 * sizeof(DRAWINDEX));
    EXPECT(square[4 * (32 * 64 + 32)] > 100);
    EXPECT(square[4 * (4 * 64 + 4)] == 0);

    // An explicit shader preserves the previous rounded-mesh attribute contract.
    entity.shader = BOX_PTR(Shader, SHADER_UI);
    R_BeginFrame((struct color){0, 0, 0, 1});
    EXPECT_OK(R_DrawEntity(&view, &entity));
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &rounded_indices);
    glReadPixels(0, 0, 64, 64, GL_RGBA, GL_UNSIGNED_BYTE, reference);
    EXPECT(rounded_indices > square_indices);
    EXPECT(memcmp(square, reference, sizeof(square)) == 0);

    entity.shader = NULL;
    entity.radius = ((struct vec4){12, 12, 12, 12});
    R_BeginFrame((struct color){0, 0, 0, 1});
    EXPECT_OK(R_DrawEntity(&view, &entity));
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &rounded_indices);
    glReadPixels(0, 0, 64, 64, GL_RGBA, GL_UNSIGNED_BYTE, reference);
    EXPECT(rounded_indices > square_indices);
    EXPECT(reference[4 * (32 * 64 + 32)] > 100);
    EXPECT(reference[4 * (55 * 64 + 8)] == 0);
    EXPECT(glGetError() == GL_NO_ERROR);
  });

  RUN_TEST("backdrop_filter_blurs_what_is_drawn_behind_an_entity", test_backdrop_filter(););

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
