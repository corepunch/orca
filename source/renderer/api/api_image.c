#include <string.h>

#include "../r_local.h"
#include <include/api.h>

#define API_TYPE_IMAGE "Image"

static int default_image_scale = 100;

static int API_ImageRenderTexture(lua_State* L)
{
  uint32_t width = luaL_checknumber(L, 1);
  uint32_t height = luaL_checknumber(L, 2);
  struct Texture** self = lua_newuserdata(L, sizeof(struct Texture*));
  luaL_setmetatable(L, API_TYPE_IMAGE);
  RenderTexture_Create(
    &(CREATERTSTRUCT){ .Width = width,
                       .Height = height,
                       .Scale = WI_GetScaling() },
    self);
  return 1;
}

static int API_ImageGetNativeSize(lua_State* L)
{
  struct Texture** self = luaL_checkudata(L, 1, API_TYPE_IMAGE);
  lua_pushnumber(L, (*self)->Width);
  lua_pushnumber(L, (*self)->Height);
  return 2;
}

static int API_ImageGetSize(lua_State* L)
{
  struct Texture** self = luaL_checkudata(L, 1, API_TYPE_IMAGE);
  int scale = (*self)->Scale;
  if (scale > 0) {
    lua_pushnumber(L, (*self)->Width * 100 / scale);
    lua_pushnumber(L, (*self)->Height * 100 / scale);
  } else {
    lua_pushnumber(L, (*self)->Width);
    lua_pushnumber(L, (*self)->Height);
  }
  return 2;
}

static int API_ImageGetScale(lua_State* L)
{
  struct Texture** self = luaL_checkudata(L, 1, API_TYPE_IMAGE);
  int scale = (*self)->Scale;
  lua_pushnumber(L, scale > 0 ? scale : 100);
  return 1;
}

static int API_ImageSetScale(lua_State* L)
{
  struct Texture** self = luaL_checkudata(L, 1, API_TYPE_IMAGE);
  (*self)->Scale = luaL_checknumber(L, 2);
  return 0;
}

// static int API_ImageIndex(lua_State *L) {
//     struct Texture** self = luaL_checkudata(L, 1, API_TYPE_IMAGE);
//     lpcString_t  name = luaL_checkstring(L, 2);
//     int scale = (*self)->Scale > 0 ? (*self)->Scale : 100;
//     if (!strcmp(name, "__name")) { lua_pushstring(L, API_TYPE_IMAGE); return
//     1; } if (!strcmp(name, "width")) { lua_pushnumber(L, (*self)->width *
//     100 / scale); return 1; } if (!strcmp(name, "height")) {
//     lua_pushnumber(L,
//     (*self)->height * 100 / scale); return 1; } if (!strcmp(name, "scale"))
//     { lua_pushnumber(L, scale); return 1; } return 0;
// }
//
// static int API_ImageNewindex(lua_State *L) {
//     struct Texture** self = luaL_checkudata(L, 1, API_TYPE_IMAGE);
//     lpcString_t  name = luaL_checkstring(L, 2);
//     if (!strcmp(name, "scale")) { (*self)->Scale = luaL_checknumber(L, 3);
//     return 0; } return 0;
// }

static int API_ImageSetDefaultImageScale(lua_State* L)
{
  default_image_scale = lua_tonumber(L, 1);
  return 0;
}

static luaL_Reg const lib_image[] = {
  { "rt", API_ImageRenderTexture },
  { "setDefaultImageScale", API_ImageSetDefaultImageScale },
  { "getSize", API_ImageGetSize },
  { "getNativeSize", API_ImageGetNativeSize },
  { "getScale", API_ImageGetScale },
  { "setScale", API_ImageSetScale },
  { NULL, NULL }
};

int luaopen_renderer_image(lua_State* L)
{
  luaL_newmetatable(L, API_TYPE_IMAGE);
  luaL_setfuncs(L, lib_image, 0);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  return 1;
}
