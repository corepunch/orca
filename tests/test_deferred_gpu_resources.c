// Shaders and images loaded before the renderer has a GL context make no GL
// calls; their program and texture are created the first time they draw.
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
extern int luaopen_orca_filesystem(lua_State *L);
extern struct Object *FS_LoadObjectFromString(const char *source);

static char const kShaderXml[] =
  "<Shader Name=\"DeferredGreen\">"
  "<FragmentShader Out=\"FragColor\">"
  "void main() { FragColor = vec4(0.0, 1.0, 0.0, 1.0); }"
  "</FragmentShader></Shader>";
static char const kImageXml[] =
  "<Image Name=\"DeferredBlue\" Source=\"tests/fixtures/deferred-gpu/blue.png\"/>";

static struct Texture *target;

// Draws one 64x64 entity into the pixel target and reads its center.
static void
draw_center(struct ViewEntity *entity, unsigned char pixel[4])
{
  struct ViewDef view = {
    .viewMatrix = MAT4_Identity(),
    .projectionMatrix = MAT4_Ortho(0, 64, 64, 0, -1, 1),
  };
  R_BeginFrame((struct color){0, 0, 0, 1});
  R_BindFramebuffer(target);
  R_ClearScreen((struct color){0, 0, 0, 1}, 1, 0);
  R_DrawEntity(&view, entity);
  glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
  R_BindFramebuffer(NULL);
}

static struct Shader *shader;
static struct Texture *image;

static void
test_shader_first_draw(void)
{
  unsigned char pixel[4] = {0};
  struct ViewEntity entity = {
    .matrix = MAT4_Identity(),
    .bbox = {.min = {0, 0, 0}, .max = {64, 64, 0}},
    .shader = shader,
    .material = {.textureMatrix = MAT3_Identity(), .color = {1, 1, 1, 1},
                 .opacity = 1, .blendMode = BLEND_MODE_OPAQUE},
  };
  do {
    draw_center(&entity, pixel);
    EXPECT(shader->shader != NULL);
    EXPECT(pixel[0] < 10 && pixel[1] > 245 && pixel[2] < 10);
  } while (0);
}

static void
test_image_first_draw(void)
{
  unsigned char pixel[4] = {0};
  struct ViewEntity entity = {
    .matrix = MAT4_Identity(),
    .bbox = {.min = {0, 0, 0}, .max = {64, 64, 0}},
    .material = {.texture = image, .textureMatrix = MAT3_Identity(),
                 .color = {1, 1, 1, 1}, .opacity = 1, .blendMode = BLEND_MODE_OPAQUE},
  };
  do {
    draw_center(&entity, pixel);
    EXPECT(image->texnum != 0 && !image->deferred);
    EXPECT(pixel[0] < 10 && pixel[1] < 10 && pixel[2] > 245);
    EXPECT(glGetError() == GL_NO_ERROR);
  } while (0);
}

int main(void)
{
  setbuf(stdout, NULL);
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  luaL_requiref(L, "orca", luaopen_orca, 1);
  lua_pop(L, 1);
  luaopen_orca_core(L);
  lua_pop(L, 1);
  luaopen_orca_renderer(L);
  lua_pop(L, 1);
  luaL_requiref(L, "orca.filesystem", luaopen_orca_filesystem, 1);
  lua_pop(L, 1);

  struct Object *shaderObject = FS_LoadObjectFromString(kShaderXml);
  struct Object *imageObject = FS_LoadObjectFromString(kImageXml);
  shader = shaderObject ? GetShader(shaderObject) : NULL;
  image = imageObject ? GetTexture(imageObject) : NULL;

  RUN_TEST("loading_without_a_context_makes_no_gpu_resources", {
    EXPECT(shader && image);
    EXPECT(shader->shader == NULL);
    EXPECT(image->texnum == 0 && image->deferred);
    // Layout still knows the image's size.
    EXPECT(image->Width == 4 && image->Height == 4);
  });

  renderer_Init(64, 64, TRUE);
  if (FAILED(RenderTexture_Create(&(CREATERTSTRUCT){.Width = 64, .Height = 64, .Scale = 1}, &target))) {
    fputs("FAIL: could not create the pixel-test framebuffer\n", stderr);
    return 1;
  }

  RUN_TEST("a_shader_loaded_before_the_context_builds_on_first_draw", test_shader_first_draw(););
  RUN_TEST("an_image_loaded_before_the_context_uploads_on_first_draw", test_image_first_draw(););

  Texture_Release(target);
  OBJ_ReleaseRef(shaderObject);
  OBJ_ReleaseRef(imageObject);
  renderer_Shutdown();
  lua_close(L);
  printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
  return s_tests_failed ? 1 : 0;
}
