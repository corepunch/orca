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

static void
test_render_texture_color_space(void)
{
  struct Texture *target = NULL;
  GLuint reference_fbo = 0, reference_color = 0;
  GLint previous_fbo;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous_fbo);
  glGenFramebuffers(1, &reference_fbo);
  glGenTextures(1, &reference_color);
  glBindTexture(GL_TEXTURE_2D, reference_color);
  glTexImage2D(GL_TEXTURE_2D, 0,
#ifdef R_USE_SRGB
              GL_SRGB8_ALPHA8,
#else
              GL_RGBA,
#endif
              64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glBindFramebuffer(GL_FRAMEBUFFER, reference_fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reference_color, 0);

  do {
    EXPECT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    EXPECT_OK(RenderTexture_Create(&(CREATERTSTRUCT){.Width = 64, .Height = 64}, &target));
    EXPECT(target != NULL);
    struct ViewDef view = {
      .viewMatrix = MAT4_Identity(),
      .projectionMatrix = MAT4_Ortho(0, 64, 64, 0, -1, 1),
    };
    struct ViewEntity entity = {
      .matrix = MAT4_Identity(),
      .bbox = {.min = {0, 0, 0}, .max = {64, 64, 0}},
      .material = {.textureMatrix = MAT3_Identity(), .color = {0.25, 0.5, 0.75, 1},
                   .opacity = 1, .blendMode = BLEND_MODE_OPAQUE},
    };
    unsigned char reference[4], composed[4], pixels[64 * 64 * 4];
    R_BeginFrame((struct color){0, 0, 0, 1});
    glBindFramebuffer(GL_FRAMEBUFFER, reference_fbo);
    R_ClearScreen((struct color){0, 0, 0, 1}, 1, 0);
    EXPECT_OK(R_DrawEntity(&view, &entity));
    glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, reference);
    EXPECT(reference[0] > 50 && reference[1] > 110 && reference[2] > 175);

    // PNG readback must contain the same display-encoded bytes as the window.
    R_BindFramebuffer(target);
    R_ClearScreen((struct color){0, 0, 0, 1}, 1, 0);
    EXPECT_OK(R_DrawEntity(&view, &entity));
    EXPECT_OK(RenderTexture_ReadPixels(target, pixels));
    unsigned char *pixel = pixels + 4 * (32 * 64 + 32);
    FOR_LOOP(i, 4) EXPECT(abs(pixel[i] - reference[i]) <= 1);

    // Sampling the target for composition must decode those bytes once.
    R_BindFramebuffer(NULL);
    glBindFramebuffer(GL_FRAMEBUFFER, reference_fbo);
    R_ClearScreen((struct color){0, 0, 0, 1}, 1, 0);
    entity.material.texture = target;
    entity.material.color = (struct color){1, 1, 1, 1};
    EXPECT_OK(R_DrawEntity(&view, &entity));
    glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, composed);
    FOR_LOOP(i, 4) EXPECT(abs(composed[i] - reference[i]) <= 1);
    EXPECT(glGetError() == GL_NO_ERROR);
  } while (0);
  R_BindFramebuffer(NULL);
  glBindFramebuffer(GL_FRAMEBUFFER, previous_fbo);
  target && Texture_Release(target);
  glDeleteFramebuffers(1, &reference_fbo);
  glDeleteTextures(1, &reference_color);
}

static struct Texture *pixel_target;

// The X11 host can supply a larger window instead of the requested offscreen
// surface. Give pixel tests their own fixed physical buffer on every platform.
static void begin_pixels(void)
{
  R_BeginFrame((struct color){0, 0, 0, 1});
  R_BindFramebuffer(pixel_target);
  PIPELINESTATE state;
  R_GetPipelineState(&state);
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  float density = state.viewport.width > 0 ? viewport[2] / state.viewport.width : 1;
  state.viewport = state.scissor = (struct rect){0, 0, 64.f / density, 64.f / density};
  state.cull_mode = CULL_MODE_BACK;
  R_SetPipelineState(&state);
  R_ClearScreen((struct color){0, 0, 0, 1}, 1, 0);
}

// Logical pixel samples use a top-left origin; GL readback is bottom-up.
static int red_at(float x, float y, int scale)
{
  unsigned char pixel[4] = {0};
  glReadPixels((int)(x * scale), 63 - (int)(y * scale), 1, 1,
               GL_RGBA, GL_UNSIGNED_BYTE, pixel);
  return pixel[0];
}

// Half-opacity red over black, read back in the target's encoding: an sRGB
// target stores linear 0.5 as about 188, a linear target as 128.
#ifdef R_USE_SRGB
#define HALF_RED(red) ((red) > 170 && (red) < 205)
#else
#define HALF_RED(red) ((red) > 100 && (red) < 140)
#endif

static void test_rounded_perimeter(int scale)
{
  struct ViewDef view = {
    .viewMatrix = MAT4_Identity(),
    .projectionMatrix = MAT4_Ortho(0, 64.f / scale, 64.f / scale, 0, -1, 1),
  };
  struct ViewEntity entity = {
    .matrix = MAT4_Identity(),
    .bbox = {.min = {8, 8, 0}, .max = {24, 24, 0}},
    .radius = {8, 8, 8, 8},
    .material = {.textureMatrix = MAT3_Identity(), .color = {1, 0, 0, 1},
                 .opacity = 0.5f, .blendMode = BLEND_MODE_ALPHA},
  };
  begin_pixels();
  do {
    EXPECT_OK(R_DrawEntity(&view, &entity));
    // These pixels are inside each circular arc, but outside the straight
    // diagonal that remains when back-face culling removes the corner fans.
    EXPECT(red_at(10, 11, scale) > 100);
    EXPECT(red_at(21, 11, scale) > 100);
    EXPECT(red_at(21, 20, scale) > 100);
    EXPECT(red_at(10, 20, scale) > 100);
    EXPECT(red_at(8, 8, scale) == 0);
    EXPECT(HALF_RED(red_at(16, 16, scale)));

    // The three-point stroke lies outside the same logical circle. Its
    // curved quadrants and straight sides must all be present, exactly once.
    entity.borderWidth = (struct vec4){3, 3, 3, 3};
    begin_pixels();
    EXPECT_OK(R_DrawEntity(&view, &entity));
    float const samples[][2] = {
      {9, 9}, {22, 9}, {22, 22}, {9, 22},
      {16, 6}, {25, 16}, {16, 25}, {6, 16},
    };
    FOR_LOOP(i, sizeof(samples) / sizeof(*samples)) {
      int red = red_at(samples[i][0], samples[i][1], scale);
      EXPECT(HALF_RED(red));
    }
    EXPECT(red_at(16, 16, scale) == 0);
    EXPECT(red_at(5, 5, scale) == 0);
    EXPECT(glGetError() == GL_NO_ERROR);
  } while (0);
}

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
  if (FAILED(RenderTexture_Create(&(CREATERTSTRUCT){.Width = 64, .Height = 64, .Scale = 1}, &pixel_target))) {
    fputs("FAIL: could not create the pixel-test framebuffer\n", stderr);
    renderer_Shutdown();
    lua_close(L);
    return 1;
  }

  RUN_TEST("rounded_fills_and_strokes_have_curved_complete_perimeters_1x", test_rounded_perimeter(1););
  RUN_TEST("rounded_fills_and_strokes_have_curved_complete_perimeters_2x", test_rounded_perimeter(2););
  RUN_TEST("rounded_fill_preserves_independent_corner_radii", {
    struct ViewDef view = ((struct ViewDef){
      .viewMatrix = MAT4_Identity(),
      .projectionMatrix = MAT4_Ortho(0, 64, 64, 0, -1, 1),
    });
    struct ViewEntity entity = ((struct ViewEntity){
      .matrix = MAT4_Identity(),
      .bbox = {.min = {8, 8, 0}, .max = {56, 40, 0}},
      .radius = {0, 12, 4, 8},
      .material = {.textureMatrix = MAT3_Identity(), .color = {1, 0, 0, 1},
                   .opacity = 1, .blendMode = BLEND_MODE_ALPHA},
    });
    begin_pixels();
    EXPECT_OK(R_DrawEntity(&view, &entity));
    EXPECT(red_at(8, 8, 1) > 200);  // Square top-left.
    EXPECT(red_at(55, 8, 1) == 0);
    EXPECT(red_at(51, 11, 1) > 200); // Twelve-point top-right arc.
    EXPECT(red_at(55, 39, 1) == 0);
    EXPECT(red_at(52, 38, 1) > 200); // Four-point bottom-right arc.
    EXPECT(red_at(8, 39, 1) == 0);
    EXPECT(red_at(10, 36, 1) > 200); // Eight-point bottom-left arc.
  });

  RUN_TEST("render_texture_matches_window_color_and_composition", test_render_texture_color_space(););

  RUN_TEST("solid_image_uses_initialized_white_texture", {
    unsigned char pixel[4] = {0};
    begin_pixels();
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
    begin_pixels();
    EXPECT_OK(R_DrawEntity(&view, &entity));
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &square_indices);
    glReadPixels(0, 0, 64, 64, GL_RGBA, GL_UNSIGNED_BYTE, square);
    EXPECT(square_indices == 6 * sizeof(DRAWINDEX));
    EXPECT(square[4 * (32 * 64 + 32)] > 100);
    EXPECT(square[4 * (4 * 64 + 4)] == 0);

    // An explicit shader preserves the previous rounded-mesh attribute contract.
    entity.shader = BOX_PTR(Shader, SHADER_UI);
    begin_pixels();
    EXPECT_OK(R_DrawEntity(&view, &entity));
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &rounded_indices);
    glReadPixels(0, 0, 64, 64, GL_RGBA, GL_UNSIGNED_BYTE, reference);
    EXPECT(rounded_indices > square_indices);
    EXPECT(memcmp(square, reference, sizeof(square)) == 0);

    entity.shader = NULL;
    entity.radius = ((struct vec4){12, 12, 12, 12});
    begin_pixels();
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
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  R_BindFramebuffer(NULL);
  Texture_Release(pixel_target);
  renderer_Shutdown();
  lua_close(L);
  printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
  return s_tests_failed ? 1 : 0;
}
