#include "SpriteKit.h"

static void
SKNode_RenderTree(lpObject_t hObject, struct ViewDef *viewdef)
{
  if (OBJ_IsHidden(hObject)) return;
  OBJ_SendMessageW(hObject, kEventRender, 0, viewdef);
  FOR_EACH_OBJECT(child, hObject) {
    SKNode_RenderTree(child, viewdef);
  }
}

HANDLER(SKView, ForegroundContent)
{
  lpObject_t scene = NULL;

  FOR_EACH_OBJECT(child, hObject) {
    if (GetSKScene(child)) {
      scene = child;
      break;
    }
  }

  if (!scene) return FALSE;

  float w = Node2D_GetFrame(GetNode2D(hObject), kBox3FieldWidth);
  float h = Node2D_GetFrame(GetNode2D(hObject), kBox3FieldHeight);
  if (w <= 0 || h <= 0) return FALSE;

  OBJ_SendMessageW(scene, kEventUpdateMatrix, 0,
                   &(UPDATEMATRIXSTRUCT){ .opacity = 1.0f });

  struct ViewDef viewdef = {0};
  viewdef.projectionMatrix = MAT4_Ortho(0, w, h, 0, -1, 1);
  viewdef.viewMatrix = MAT4_Identity();
  viewdef.viewport = hObject;

  SKNode_RenderTree(scene, &viewdef);

  return TRUE;
}
