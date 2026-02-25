#ifndef __SPRITEKIT_PRIVATE_H__
#define __SPRITEKIT_PRIVATE_H__

#include "SpriteKit.h"

struct SpriteFrame {
  struct rect Rect;   ///< Position and size of sprite in local space
  struct rect UvRect; ///< UV coordinates in the texture atlas
};

struct SpriteAnimation {
  struct Texture *Image;    ///< Spritesheet texture
  float Framerate;          ///< Frames per second
  uint32_t NumFrames;       ///< Number of frames
  struct SpriteFrame Frames[]; ///< Variable-length frame data array
};

#endif
