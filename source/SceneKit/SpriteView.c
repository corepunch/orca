#include <include/orca.h>
#include <include/renderer.h>

#include "SceneKit.h"

HANDLER(SpriteView, Render)
{
  struct rect viewbox = {0};
  struct mat3 texmat;

  texmat = MAT3_Identity();

  if (!pSpriteView->Image) {
    viewbox.width = 64;
    viewbox.height = 64;
  } else {
    struct image_info img;
    Image_GetInfo(pSpriteView->Image, &img);
    if (memcmp(&viewbox, &pSpriteView->Bounds, sizeof(struct rect))) {
      memcpy(&viewbox, &pSpriteView->Bounds, sizeof(struct rect));
      MAT3_Translate(&texmat, &(struct vec2) {
        (float)viewbox.x / (float)img.bmWidth,
        (float)(viewbox.y+viewbox.height) / (float)img.bmHeight,
      });
      MAT3_Scale(&texmat, &(struct vec2) {
        (float)viewbox.width / (float)img.bmWidth,
        (float)-viewbox.height / (float)img.bmHeight,
      });
    } else {
      viewbox.width = img.bmWidth;
      viewbox.height = img.bmHeight;
    }
  }
  
  struct ViewEntity entity = {
    .debugName = OBJ_GetName(hObject),
    .radius = (struct vec4){0},
    .rect = (struct rect){
      -0.005f*viewbox.width, -0.005f*viewbox.height,
      0.01f*viewbox.width, 0.01f*viewbox.height
    },
    .texture = pSpriteView->Image,
    .textureMatrix = texmat,
    .opacity = GetNode3D(hObject)->_opacity,
    .matrix = GetNode3D(hObject)->Matrix,
    .color = {1,1,1,1},
    .blendMode = BLEND_MODE_ALPHA,
  };
  
  R_DrawEntity(pRender, &entity);
  
  return TRUE;
}
