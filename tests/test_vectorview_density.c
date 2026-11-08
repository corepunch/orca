#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <include/api.h>
#include <include/renderer.h>
#include <renderer/renderer.h>
#include <UIKit/UIKit.h>
#if __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

extern int luaopen_orca(lua_State *L);
extern int luaopen_orca_core(lua_State *L);
extern int luaopen_orca_renderer(lua_State *L);
extern int luaopen_orca_UIKit(lua_State *L);

#if !__APPLE__
/* Exercise display-density changes without requiring a particular monitor. */
static float density = 1;
float axGetScaling(void) { return density; }
#endif

static void set_source(struct Object *icon, char const *path)
{
  struct Property *source = OBJ_FindLongProperty(icon, ID_VectorView_Source);
  char *absolute = realpath(path, NULL);
  assert(source && absolute);
  PROP_SetStringValue(source, absolute);
  free(absolute);
  struct Object_PropertyChangedEventArgs changed = { .Property = source };
  OBJ_SendMessageW(icon, ID_Object_PropertyChanged, 0, &changed);
}

static struct Texture *arrange_icon(struct Object *icon, float logical)
{
  struct Node2D_MeasureEventArgs measure = { .Width = logical, .Height = logical };
  OBJ_SendMessageW(icon, ID_Node2D_MeasureOverride, 0, &measure);
  struct Node2D_ArrangeEventArgs arrange = { .Width = logical, .Height = logical };
  OBJ_SendMessageW(icon, ID_Node2D_ArrangeOverride, 0, &arrange);
  struct Texture *texture = (void *)OBJ_SendMessageW(icon, ID_Node2D_ForegroundContent, 0, NULL);
  assert(texture && texture->Width == logical && texture->Height == logical);
  assert(texture->Scale == axGetScaling());
  GLint width = -1, height = -1;
  glBindTexture(GL_TEXTURE_2D, texture->texnum);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
  assert(width == lroundf(logical * axGetScaling()));
  assert(height == lroundf(logical * axGetScaling()));
  assert(glGetError() == GL_NO_ERROR);
  return texture;
}

int main(void)
{
  lua_State *L = luaL_newstate();
  assert(L);
  luaL_openlibs(L);
  assert(luaL_dostring(L, "package.path = './share/?.lua;./share/?/init.lua;' .. package.path") == LUA_OK);
  luaL_requiref(L, "orca", luaopen_orca, 1); lua_pop(L, 1);
  luaL_requiref(L, "orca.core", luaopen_orca_core, 1); lua_pop(L, 1);
  luaL_requiref(L, "orca.renderer", luaopen_orca_renderer, 1); lua_pop(L, 1);
  luaL_requiref(L, "orca.UIKit", luaopen_orca_UIKit, 1); lua_pop(L, 1);
  assert(renderer_Init(64, 64, TRUE) >= 0);
  if (!glGetString(GL_VERSION)) {
    fputs("UNAVAILABLE: test-vectorview-density requires a native graphics context (use Xvfb on Linux)\n", stderr);
    return 77;
  }
  struct Object *icon = OBJ_Create(ID_VectorView);
  assert(icon);
  set_source(icon, "apps/octopus/assets/icons/file-text.svg");

#if !__APPLE__
  for (unsigned scale = 1; scale <= 3; scale++) {
    density = scale;
#else
  {
#endif
    struct Texture *texture = arrange_icon(icon, 48);
    GLuint cached = texture->texnum;
    assert(arrange_icon(icon, 48)->texnum == cached);
    arrange_icon(icon, 72);
    arrange_icon(icon, 18);
    set_source(icon, "apps/octopus/assets/icons/image.svg");
    arrange_icon(icon, 48);
  }
  puts("PASS: VectorView rasterizes icons at their physical draw size, retains unchanged masks and refreshes size/source/density");
  OBJ_ReleaseRef(icon);
  renderer_Shutdown();
  lua_close(L);
  axShutdown();
  return 0;
}
