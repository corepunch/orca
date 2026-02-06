#include "../r_local.h"

#include <libxml/parser.h>

#define API_TYPE_WINDOW "renderer"

#define WINAPI(NAME, ...) \
WI_##NAME(lua_State *L, renderer* self, ##__VA_ARGS__)

// int f_beginFrame(lua_State *L) {
//	R_BeginFrame(WI_Get(L), (struct color){0});
//	return 0;
// }

// int f_endFrame(lua_State *L) {
//	R_EndFrame(WI_Get(L));
//	return 0;
// }

//struct Texture *
//renderer_CreateRenderTarget(lua_State* L, int width, int height) {
//  struct Texture* texture = 0;
//  RenderTexture_Create(
//    &(CREATERTSTRUCT){
//      .Width = width,
//      .Height = height,
//      .Scale = WI_GetScaling(),
//    },
//    &texture);
//  return texture;
//}

lpShader_t
renderer_CreateShader(lua_State* L, lpcString_t vert, lpcString_t frag) {
  API_CallRequire(L, "orca.renderer", 1);
  lua_getfield(L, -1, "Shader");
  lua_remove(L, -2);
  if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
    Con_Error("renderer_CreateShader: %s", lua_tostring(L, -1));
    lua_pop(L, 1);
    return NULL;
  }
  lpObject_t object = luaX_checkObject(L, -1);
  struct Shader* shader = GetShader(object);
  lua_pop(L, 1);
  Shader_LoadFromDef(
    &(struct shader_desc){
      .Version = 150,
      .VertexShader = vert,
      .FragmentShader = frag,
    }, &shader->shader);
  return shader;
}

int renderer_GetSize(lua_State* L) {
  struct WI_Size size;
  WI_GetSize(&size);
  lua_pushnumber(L, size.width);
  lua_pushnumber(L, size.height);
  return 2;
}

void renderer_DrawImage(lua_State* L) {
  luaX_parsefield(struct Texture*, image, 1, lua_touserdata);
  luaX_parsefield(struct rect*, rect, 1, luaL_checkudata, "rect");
  luaX_parsefield(struct rect*, uv, 1, luaL_testudata, "rect");

  R_DrawImage(&(DRAWIMAGESTRUCT){
                .img = image,
                .rect = *rect,
                .uv = uv ? *uv : (struct rect){ 0, 0, 1, 1 },
              });
}
