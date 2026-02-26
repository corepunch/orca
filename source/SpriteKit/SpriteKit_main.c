#include <include/api.h>
#include "SpriteKit.h"

static int
f_createAnimation(lua_State *L)
{
  lpTexture_t image = luaX_checkTexture(L, 1);
  luaL_checktype(L, 2, LUA_TTABLE);
  float framerate = (float)luaL_checknumber(L, 3);

  int num_frames = (int)lua_rawlen(L, 2);
  size_t size = sizeof(struct SpriteAnimation)
              + (size_t)num_frames * sizeof(struct SpriteFrame);
  struct SpriteAnimation *anim = malloc(size);
  if (!anim) return luaL_error(L, "out of memory");
  memset(anim, 0, size);
  anim->Image = image;
  anim->Framerate = framerate;
  anim->NumFrames = (uint32_t)num_frames;

  for (int i = 0; i < num_frames; i++) {
    lua_rawgeti(L, 2, i + 1);
    if (lua_istable(L, -1)) {
      struct SpriteFrame *frame = &anim->Frames[i];

      lua_getfield(L, -1, "dx");
      frame->Rect.x = -(float)lua_tonumber(L, -1);
      lua_pop(L, 1);

      lua_getfield(L, -1, "dy");
      frame->Rect.y = -(float)lua_tonumber(L, -1);
      lua_pop(L, 1);

      lua_getfield(L, -1, "w");
      frame->Rect.width = (float)lua_tonumber(L, -1);
      lua_pop(L, 1);

      lua_getfield(L, -1, "h");
      frame->Rect.height = (float)lua_tonumber(L, -1);
      lua_pop(L, 1);

      lua_getfield(L, -1, "u");
      frame->UvRect.x = (float)lua_tonumber(L, -1);
      lua_pop(L, 1);

      lua_getfield(L, -1, "v");
      frame->UvRect.y = (float)lua_tonumber(L, -1);
      lua_pop(L, 1);

      lua_getfield(L, -1, "cu");
      frame->UvRect.width = (float)lua_tonumber(L, -1);
      lua_pop(L, 1);

      lua_getfield(L, -1, "cv");
      frame->UvRect.height = (float)lua_tonumber(L, -1);
      lua_pop(L, 1);
    }
    lua_pop(L, 1);
  }

  luaX_pushSpriteAnimation(L, anim);
  return 1;
}

void
on_spritekit_module_registered(lua_State *L)
{
  lua_pushcfunction(L, f_createAnimation);
  lua_setfield(L, -2, "createAnimation");
}
