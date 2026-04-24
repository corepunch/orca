#include "SpriteKit.h"

static struct Object *
find_viewport(struct Object *node) {
  if (!node || OBJ_GetComponent(node, ID_SKView))
    return node;
  return find_viewport(OBJ_GetParent(node));
}

struct vec2
SKNode_GetReferenceSize(struct Object *node)
{
  struct Object *viewport = find_viewport(node);
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
SKNode_RenderTree(struct Object *hObject, struct ViewDef *viewdef)
{
  if (OBJ_IsHidden(hObject)) return;
  _SendMessage(hObject, SKNode, Render, .ViewDef = viewdef);
  FOR_EACH_OBJECT(child, hObject) {
    SKNode_RenderTree(child, viewdef);
  }
}

HANDLER(SKView, Node2D, ForegroundContent)
{
  struct Object *scene = NULL;

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

  _SendMessage(scene, Node, UpdateMatrix,
    .parent = MAT4_Identity(),
    .opacity = 1.0f
  );

  struct ViewDef viewdef = {0};
  viewdef.projectionMatrix = MAT4_Ortho(0, w, 0, h, -1, 1);
  viewdef.viewMatrix = MAT4_Identity();
  viewdef.viewport = hObject;

  SKNode_RenderTree(scene, &viewdef);

  return FALSE;
}
