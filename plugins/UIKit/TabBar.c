#include <include/orca.h>
#include <source/filesystem/filesystem.h>

#include <plugins/UIKit/UIKit.h>

HANDLER(TabBar, Object, Create)
{
  struct Node *node = GetNode(hObject);
  struct StackView *stack = GetStackView(hObject);
  if (stack) {
    stack->Direction = kDirectionHorizontal;
  }

  const char *accent = FS_GetThemeValue("$accent");
  if (accent) {
    node->Border.Color = COLOR_Parse(accent);
  }
  node->Border.Width.Axis[1].Right = 1.0f;

  return FALSE;
}

HANDLER(TabBar, Node2D, DrawBrush)
{
  if (!pDrawBrush->foreground) {
    return FALSE;
  }

  struct Node2D *pNode2D = GetNode2D(hObject);
  struct Node *node = GetNode(hObject);
  if (!pNode2D || !node || node->Border.Width.Axis[1].Right <= 0.0f) {
    return FALSE;
  }

  struct ViewEntity entity;
  Node2D_GetViewEntity(pNode2D, &entity, NULL, &pDrawBrush->brush);

  struct rect bounds = Node2D_GetBackgroundRect(pNode2D);
  float borderH = node->Border.Width.Axis[1].Right;
  entity.bbox = BOX3_FromRect(bounds);
  entity.bbox.min.y = entity.bbox.max.y - borderH;
  entity.radius = (struct vec4){0, 0, 0, 0};
  entity.material = (struct ViewMaterial){
    .color   = node->Border.Color,
    .opacity = pNode2D->_opacity,
    .blendMode = BLEND_MODE_ALPHA,
  };

  R_DrawEntity(pDrawBrush->viewdef, &entity);
  return TRUE;
}

HANDLER(TabBar, TabBar, SelectionChanged)
{
  /* All state mutation is done in Tab_Select before the message is
     posted here.  Return FALSE so the event continues to bubble up
     the tree — allowing a parent TabView to intercept it via its own
     HANDLER(TabView, TabBar, SelectionChanged) component proc. */
  return FALSE;
}
