#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <include/orca.h>
#include <include/renderer.h>
#include <renderer/renderer.h>
#include "test_local.h"

extern int luaopen_orca(lua_State *L);
extern int luaopen_orca_core(lua_State *L);
extern int luaopen_orca_renderer(lua_State *L);
// Arrange an object reference without starting its GPU resources in a headless test.
extern void const *PROP_GetRawValueSlot(struct Property const *property);

static void
test_texture_uniform(char const *name, char const *class_name,
                     enum uniform_type expected)
{
  struct PropertyType type = {
    .Name = name, .Category = "Uniform", .TypeString = class_name,
    .DataType = kDataTypeObject, .DataSize = sizeof(void *),
    .ShortIdentifier = fnv1a32(name), .FullIdentifier = fnv1a32(name),
  };
  struct Object *owner = OBJ_Create(fnv1a32("Material"));
  struct Object *texture = OBJ_Create(fnv1a32(class_name));
  struct Property *property = PROP_Create(NULL, owner, &type);
  struct uniform uniforms[MAX_UNIFORMS] = {0};

  RUN_TEST(name, {
    EXPECT(owner && texture && property);
    *(void **)PROP_GetRawValueSlot(property) =
      OBJ_GetComponent(texture, fnv1a32(class_name));
    OBJ_AddRef(texture);
    PROP_SetFlag(property, PF_MODIFIED);
    uint32_t count = OBJ_GetUniforms(owner, uniforms);
    struct uniform *sampler = NULL;
    for (uint32_t i = 0; i < count; i++) {
      if (uniforms[i].Identifier == fnv1a32(name)) sampler = &uniforms[i];
    }
    EXPECT(sampler);
    EXPECT(sampler->Type == expected);
    // The sampler must receive the base Texture, not the CubeMapTexture component.
    EXPECT(*(struct Texture **)sampler->Value ==
           OBJ_GetComponent(texture, fnv1a32("Texture")));
    PROP_SetValue(property, &(struct Object *){ NULL });
    uint32_t cleared = OBJ_GetUniforms(owner, uniforms);
    EXPECT(cleared == count - 1);
    for (uint32_t i = 0; i < cleared; i++) {
      EXPECT(uniforms[i].Identifier != fnv1a32(name));
    }
  });

  OBJ_ReleaseRef(owner);
  OBJ_ReleaseRef(texture);
}

static void
test_capture_without_renderer(void)
{
  RUN_TEST("screen_space_capture_without_renderer", {
    struct ScreenSpaceCapture capture = {0};
    EXPECT(!SSR_CaptureFrame(NULL));
    EXPECT(!SSR_CaptureFrame(&capture));
    EXPECT(!capture.Color && !capture.Depth);
    SSR_Release(&capture);
    SSR_Release(&capture);
    SSR_Release(NULL);
    EXPECT(!capture.Color && !capture.Depth);
  });
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

  test_texture_uniform("EnvironmentMap", "CubeMapTexture", UT_SAMPLER_CUBE);
  test_texture_uniform("DiffuseMap", "Texture", UT_SAMPLER_2D);
  test_capture_without_renderer();
  lua_close(L);
  printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
  return s_tests_failed ? 1 : 0;
}
