#include "SpriteKit.h"

extern unsigned long WI_GetMilliseconds(void);

enum {
  kSKNodeAnchorRight = 2,
  kSKNodeAnchorTop   = 4,
};

static uint32_t
get_frame_index(uint32_t num_frames, float framerate, int32_t freeze_frame)
{
  if (freeze_frame >= 0) {
    return (uint32_t)freeze_frame % num_frames;
  }
  unsigned long ms = WI_GetMilliseconds();
  return (uint32_t)((double)ms * framerate / 1000.0) % num_frames;
}

HANDLER(SKSpriteNode, Render)
{
  SKNodePtr node = GetSKNode(hObject);
  if (!node) return FALSE;

  float const refW = Node2D_GetFrame(GetNode2D(pRender->viewport), kBox3FieldWidth);
  float const refH = Node2D_GetFrame(GetNode2D(pRender->viewport), kBox3FieldHeight);
  lpTexture_t image = pSKSpriteNode->Image;
  struct SpriteAnimation const *anim = pSKSpriteNode->Animation;
  struct mat3 texmat = MAT3_Identity();
  struct rect bbox = {0};

  if (anim && anim->NumFrames > 0) {
    uint32_t idx = get_frame_index(anim->NumFrames, anim->Framerate, pSKSpriteNode->FreezeFrame);
    struct SpriteFrame const *frame = &anim->Frames[idx];
    image = anim->Image;

    bbox = frame->Rect;
    bbox.x = -bbox.x;
    bbox.y = -bbox.y;

    MAT3_Translate(&texmat, &(struct vec2){
      frame->UvRect.x,
      frame->UvRect.y + frame->UvRect.height,
    });
    MAT3_Scale(&texmat, &(struct vec2){
      frame->UvRect.width,
      -frame->UvRect.height,
    });
  } else if (image) {
    struct image_info img;
    Image_GetInfo(image, &img);

    bbox = node->Rect;
    if (bbox.width == 0 && bbox.height == 0) {
      bbox.width = (float)img.bmWidth;
      bbox.height = (float)img.bmHeight;
    }
    bbox.x = -bbox.x;
    bbox.y = -bbox.y;

    struct rect uv = pSKSpriteNode->UvRect;
    if (uv.width == 0 && uv.height == 0) {
      uv = (struct rect){0, 0, 1, 1};
    }
    MAT3_Translate(&texmat, &(struct vec2){uv.x, uv.y + uv.height});
    MAT3_Scale(&texmat, &(struct vec2){uv.width, -uv.height});
  } else {
    bbox.width = 16;
    bbox.height = 16;
  }

  if (node->Anchor & kSKNodeAnchorRight) bbox.x += refW;
  if (node->Anchor & kSKNodeAnchorTop)   bbox.y += refH;

  enum blend_mode blendMode = (int)pSKSpriteNode->BlendMode >= 0
    ? (enum blend_mode)pSKSpriteNode->BlendMode
    : BLEND_MODE_ALPHA;

  struct ViewEntity entity = {
    .bbox = BOX3_FromRect(bbox),
    .matrix = node->Matrix,
    .material = {
      .opacity = node->_opacity,
      .color = {1, 1, 1, 1},
      .texture = image,
      .textureMatrix = texmat,
      .blendMode = blendMode,
    },
  };

//  printf("%f %f %f %f\n", pSKNode->Rect.x, pSKNode->Rect.y, pSKNode->Rect.width, pSKNode->Rect.height);
  
  R_DrawEntity(pRender, &entity);

  struct SpriteAnimation const *anim2 = pSKSpriteNode->Animation2;
  if (anim2 && anim2->NumFrames > 0) {
    uint32_t idx2 = get_frame_index(anim2->NumFrames, anim2->Framerate, -1);
    struct SpriteFrame const *frame2 = &anim2->Frames[idx2];
    struct mat3 texmat2 = MAT3_Identity();
    struct rect bbox2 = frame2->Rect;
    bbox2.x = -bbox2.x;
    bbox2.y = -bbox2.y;
    if (node->Anchor & kSKNodeAnchorRight) bbox2.x += refW;
    if (node->Anchor & kSKNodeAnchorTop)   bbox2.y += refH;
    MAT3_Translate(&texmat2, &(struct vec2){
      frame2->UvRect.x,
      frame2->UvRect.y + frame2->UvRect.height,
    });
    MAT3_Scale(&texmat2, &(struct vec2){
      frame2->UvRect.width,
      -frame2->UvRect.height,
    });
    struct ViewEntity entity2 = {
      .bbox = BOX3_FromRect(bbox2),
      .matrix = node->Matrix,
      .material = {
        .opacity = node->_opacity,
        .color = {1, 1, 1, 1},
        .texture = anim2->Image,
        .textureMatrix = texmat2,
        .blendMode = blendMode,
      },
    };
    R_DrawEntity(pRender, &entity2);
  }

  return TRUE;
}
