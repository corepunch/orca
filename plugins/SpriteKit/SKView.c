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
    value.x = NODE2D_FRAME(GetNode2D(viewport), Size, 0).Actual - w;
    value.y = NODE2D_FRAME(GetNode2D(viewport), Size, 1).Actual - h;
    return value;
  } else {
    return value;
  }
}

static void
SKNode_RenderTree(lpObject_t hObject, struct ViewDef *viewdef)
{
  if (OBJ_IsHidden(hObject)) return;
  _SendMessage(hObject, Render, .ViewDef = viewdef);
//  OBJ_SendMessageW(hObject, kMsgRender, 0, &(struct RenderMsgArgs) {viewdef});
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

  float w = NODE2D_FRAME(GetNode2D(hObject), Size, 0).Actual;
  float h = NODE2D_FRAME(GetNode2D(hObject), Size, 1).Actual;
  if (w <= 0 || h <= 0) return FALSE;

  OBJ_SendMessageW(scene, kMsgUpdateMatrix, 0,
                   &(struct UpdateMatrixMsgArgs){
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
