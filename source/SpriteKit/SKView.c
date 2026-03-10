#include "SpriteKit.h"

static lpObject_t
find_viewport(lpObject_t node) {
  if (!node || OBJ_GetComponent(node, ID_SKView))
    return node;
  return find_viewport(OBJ_GetParent(node));
}

struct vec2
SKNode_GetReferenceSize(lpObject_t node)
{
  lpObject_t viewport = find_viewport(node);
  struct vec2 value = { 0, 0 };
  if (viewport) {
    struct SKView const *view = GetSKView(viewport);
    float const w = view->ReferenceWidth;
    float const h = view->ReferenceHeight;
    value.x = Node2D_GetFrame(GetNode2D(viewport), kBox3FieldWidth) - w;
    value.y = Node2D_GetFrame(GetNode2D(viewport), kBox3FieldHeight) - h;
    return value;
  } else {
    return value;
  }
}

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
                   &(struct UpdateMatrixEventArgs){
    .parent = MAT4_Identity(),
    .opacity = 1.0f
  });

  struct ViewDef viewdef = {0};
  viewdef.projectionMatrix = MAT4_Ortho(0, w, 0, h, -1, 1);
  viewdef.viewMatrix = MAT4_Identity();
  viewdef.viewport = hObject;

  SKNode_RenderTree(scene, &viewdef);

  return TRUE;
}
