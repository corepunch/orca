#include <include/orca.h>

#include <source/UIKit/UIKit.h>

lpObject_t
_NextTabStop(lpObject_t hObject);

#define USE_STANDARD_BUTTON

HANDLER(Button, DrawBrush) {
#ifdef USE_STANDARD_BUTTON
  if (pDrawBrush->foreground) return FALSE;
  
  struct ViewEntity entity;
  Node2D_GetViewEntity(hObject,&entity,pDrawBrush->image,pDrawBrush->brush);
  entity.type = ET_CAPSULE;
  entity.material.color = pDrawBrush->brush->Color;// (color_t){0.4,0.6,0.8,1};
  
  float opacity = entity.material.opacity;
  rect_t r = entity.rect;
  
  r.x -= PADDING_TOP(GetNode2D(hObject), 0);
  r.y -= PADDING_TOP(GetNode2D(hObject), 1);
  r.width += TOTAL_PADDING(GetNode2D(hObject), 0);
  r.height += TOTAL_PADDING(GetNode2D(hObject), 1);
  
  entity.rect = RECT_Expand(&r, 1);
  entity.material.opacity *= 0.5;
  entity.material.color = (color_t){0,0,0,1};

  R_DrawEntity(pDrawBrush->viewdef, &entity);

  entity.rect = r;
  entity.material.opacity = opacity;
  entity.material.color = pDrawBrush->brush->Color;// (color_t){0.4,0.6,0.8,1};

  R_DrawEntity(pDrawBrush->viewdef, &entity);

  return TRUE;
#else
  return FALSE;
#endif
}

HANDLER(Button, LeftMouseUp)
{
  SV_PostMessage(hObject, "Click", 0, hObject);
  if (pButton->Type == kButtonTypeSubmit) {
    WI_PostMessageW(hObject, kEventSubmit, 0, hObject);
  }
  return FALSE;
}

HANDLER(Button, Create)
{
  OBJ_SetStyle(hObject, OBJ_GetStyle(hObject) | OF_TABSTOP);
  return FALSE;
}

HANDLER(Button, KeyDown)
{
  if (pKeyDown->keyCode == WI_KEY_TAB) {
    _NextTabStop(hObject);
    return TRUE;
  } else {
    return FALSE;
  }
}
