#include <include/orca.h>

#include <source/UIKit/UIKit.h>

lpObject_t
_NextTabStop(lpObject_t hObject);

//#define USE_STANDARD_BUTTON

HANDLER(Button, DrawBrush) {
#ifdef USE_STANDARD_BUTTON
  if (pDrawBrush->foreground) return FALSE;
  
  struct ViewEntity entity;
  Node2D_GetViewEntity(hObject,&entity,pDrawBrush->image,pDrawBrush->brush);
  entity.mesh = BOX_PTR(Mesh, MD_ROUNDED_BOX);
  entity.shader = BOX_PTR(Shader, SHADER_ROUNDEDBOX);
  entity.material.color = pDrawBrush->brush->Color;// (color_t){0.4,0.6,0.8,1};
  
  float opacity = entity.material.opacity;
  float r_x = entity.bbox.min.x;
  float r_y = entity.bbox.min.y;
  float r_width = entity.bbox.max.x - entity.bbox.min.x;
  float r_height = entity.bbox.max.y - entity.bbox.min.y;
  
  r_x -= PADDING_TOP(GetNode2D(hObject), 0);
  r_y -= PADDING_TOP(GetNode2D(hObject), 1);
  r_width += TOTAL_PADDING(GetNode2D(hObject), 0);
  r_height += TOTAL_PADDING(GetNode2D(hObject), 1);
  
//  entity.rect = RECT_Expand(&r, 1);
//  entity.material.opacity *= 0.5;
//  entity.material.color = (color_t){0,0,0,1};
//
//  R_DrawEntity(pDrawBrush->viewdef, &entity);

  entity.bbox = BOX3_FromRect(((struct rect){r_x, r_y, r_width, r_height}));
  entity.material = (struct ViewMaterial) {
    .opacity = opacity,
    .color = pDrawBrush->brush->Color,// (color_t){0.4,0.6,0.8,1};
  };
  
  entity.radius.x = 8;
  entity.radius.y = 8;
  entity.radius.z = 8;

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
