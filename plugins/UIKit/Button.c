#include <include/orca.h>
#include <math.h>
#include <source/filesystem/theme_palette.h>

#include <plugins/UIKit/UIKit.h>

struct Object *
_NextTabStop(struct Object *hObject);

static bool_t
Button_ColorMatches(struct color const *c, float r, float g, float b, float a)
{
  return fabsf(c->r - r) < 0.001f &&
         fabsf(c->g - g) < 0.001f &&
         fabsf(c->b - b) < 0.001f &&
         fabsf(c->a - a) < 0.001f;
}

HANDLER(Button, Node2D, DrawBrush) {
  if (pDrawBrush->foreground) return FALSE;

  struct Node2D *pNode2D = GetNode2D(hObject);
  if (!pNode2D) return FALSE;

  struct ViewEntity entity;
  Node2D_GetViewEntity(pNode2D, &entity, pDrawBrush->image, &pDrawBrush->brush);

  /* Use the brush color if one is explicitly set (non-zero alpha),
     otherwise fall back to the component's DiffuseColor default. */
  struct color faceColor =
      (pDrawBrush->brush.Color.a > 0.0f)
          ? pDrawBrush->brush.Color
          : pButton->DiffuseColor;

  float opacity = entity.material.opacity;
  float r_x = entity.bbox.min.x;
  float r_y = entity.bbox.min.y;
  float r_width  = entity.bbox.max.x - entity.bbox.min.x;
  float r_height = entity.bbox.max.y - entity.bbox.min.y;

  r_x     -= PADDING_TOP(pNode2D, 0);
  r_y     -= PADDING_TOP(pNode2D, 1);
  r_width  += TOTAL_PADDING(pNode2D, 0);
  r_height += TOTAL_PADDING(pNode2D, 1);

  entity.bbox = BOX3_FromRect(((struct rect){r_x, r_y, r_width, r_height}));
  entity.mesh = BOX_PTR(Mesh, MD_ROUNDED_BOX);
  entity.shader = BOX_PTR(Shader, SHADER_ROUNDEDBOX);
  entity.radius = (struct vec4){
    pButton->CornerRadius,
    pButton->CornerRadius,
    pButton->CornerRadius,
    pButton->CornerRadius,
  };
  entity.material = (struct ViewMaterial){
    .opacity   = opacity,
    .color     = faceColor,
    .blendMode = BLEND_MODE_ALPHA,
  };

  R_DrawEntity(pDrawBrush->viewdef, &entity);

  return TRUE;
}

HANDLER(Button, Node, LeftButtonUp)
{
  axPostMessageW(hObject, ID_Button_Click, 0, 0);
  if (pButton->Type == kButtonTypeSubmit) {
    axPostMessageW(hObject, ID_Form_Submit, 0, hObject);
  }
  return FALSE;
}

HANDLER(Button, Object, Create)
{
  if (Button_ColorMatches(&pButton->DiffuseColor, 0.3f, 0.55f, 0.85f, 1.0f)) {
    pButton->DiffuseColor = FS_GetThemeColorOr3(
      THEME_COLOR_CONTROL_BACKGROUND,
      THEME_COLOR_ACCENT_BACKGROUND,
      THEME_COLOR_ACCENT,
      pButton->DiffuseColor);
  }

  struct Node2D *node2d = GetNode2D(hObject);
  if (node2d && FS_IsZeroColor(&node2d->Foreground.Color)) {
    node2d->Foreground.Color = FS_GetThemeColorOr(
      THEME_COLOR_CONTROL_FOREGROUND,
      (struct color){ 1.0f, 1.0f, 1.0f, 1.0f });
  }

  OBJ_SetStyle(hObject, OBJ_GetStyle(hObject) | OF_TABSTOP);
  return FALSE;
}

HANDLER(Button, Node, KeyDown)
{
  if (pKeyDown->keyCode == AX_KEY_TAB) {
    _NextTabStop(hObject);
    return TRUE;
  } else {
    return FALSE;
  }
}
