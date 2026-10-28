#include <include/orca.h>
#include <include/renderer.h>
#include <source/core/core_local.h>
#include <UIKit/UIKit.h>

static bool_t
ActivityIndicator_IsVisible(struct Object *object)
{
  for (struct Object *node = object; node; node = OBJ_GetParent(node)) {
    struct Screen *screen = GetScreen(node);
    if (screen) return screen->Visible;
    if (GetNode(node) && OBJ_IsHidden(node)) return FALSE;
  }
  return FALSE;
}

HANDLER(ActivityIndicator, Object, Create)
{
  struct Node *node = GetNode(hObject);
  for (int axis = 0; axis < 2; axis++) {
    node->Margin.Axis[axis].Left = node->Margin.Axis[axis].Right = NAN;
  }
  GetNode2D(hObject)->IgnoreHitTest = TRUE;
  return FALSE;
}

HANDLER(ActivityIndicator, Node2D, MeasureOverride)
{
  float side = fmaxf(0, fminf(64, fminf(pMeasureOverride->Width, pMeasureOverride->Height)));
  return MAKEDWORD(side, side);
}

HANDLER(ActivityIndicator, Object, PropertyChanged)
{
  if (pPropertyChanged->Property &&
      PROP_GetLongIdentifier(pPropertyChanged->Property) == ID_ActivityIndicator_IsActive)
    OBJ_SetDirty(hObject);
  return FALSE;
}

HANDLER(ActivityIndicator, Object, Animate)
{
  // A retained page behind the current one is not hidden, yet its indicator
  // is never drawn; once a paint skips it the steps stop until it draws again.
  if (!pActivityIndicator->IsActive || !ActivityIndicator_IsVisible(hObject) ||
      core.frame - pActivityIndicator->_drawnFrame > 1)
    return FALSE;
  OBJ_RequestAnimate(hObject);
  uint32_t step = axGetMilliseconds() / 80;
  if (step == pActivityIndicator->_step) return FALSE;
  pActivityIndicator->_step = step;
  OBJ_SetDirty(hObject);
  return TRUE;
}

HANDLER(ActivityIndicator, Node2D, ForegroundContent)
{
  return pActivityIndicator->IsActive;
}

HANDLER(ActivityIndicator, Node2D, DrawForeground)
{
  if (!pActivityIndicator->IsActive) return FALSE;
  struct Node2D *view = GetNode2D(hObject);
  struct rect bounds = view->_rect;
  float side = MIN(bounds.width, bounds.height), diameter = side * 0.12f;
  if (side <= 0) return FALSE;
  struct ViewEntity entity;
  Node2D_GetViewEntity(view, &entity, NULL, &pDrawForeground->brush);
  entity.radius = (struct vec4){diameter / 2, diameter / 2, diameter / 2, diameter / 2};
  uint32_t step = axGetMilliseconds() / 80;
  pActivityIndicator->_step = step;
  pActivityIndicator->_drawnFrame = core.frame;
  for (int i = 0; i < 12; i++) {
    float angle = (float)i * 6.2831853f / 12 - 1.5707963f;
    struct rect dot = {
      bounds.x + bounds.width / 2 + cosf(angle) * side * 0.36f - diameter / 2,
      bounds.y + bounds.height / 2 + sinf(angle) * side * 0.36f - diameter / 2,
      diameter, diameter,
    };
    entity.bbox = BOX3_FromRect(dot);
    entity.material.opacity = view->_opacity * (1 - ((step + 12 - i) % 12) * 0.07f);
    R_DrawEntity(pDrawForeground->viewdef, &entity);
  }
  // Drawing restarts the animation when a retained, previously hidden page returns.
  OBJ_RequestAnimate(hObject);
  return TRUE;
}
