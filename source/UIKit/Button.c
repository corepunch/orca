#include <include/orca.h>

#include <source/UIKit/UIKit.h>

lpObject_t
_NextTabStop(lpObject_t hObject);

HANDLER(Button, DrawBrush) {
  if (pDrawBrush->foreground) return FALSE;
  
  struct ViewEntity entity;
  Node2D_GetViewEntity(hObject,&entity,pDrawBrush->image,pDrawBrush->brush);
  entity.type = ET_CAPSULE;
  entity.color = (color_t){0.5,0.75,1,1};
  
  entity.rect.x -= PADDING_TOP(GetNode2D(hObject), 0);
  entity.rect.y -= PADDING_TOP(GetNode2D(hObject), 1);
  entity.rect.width += TOTAL_PADDING(GetNode2D(hObject), 0);
  entity.rect.height += TOTAL_PADDING(GetNode2D(hObject), 1);
    
  R_DrawEntity(pDrawBrush->viewdef, &entity);

  return FALSE;
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
