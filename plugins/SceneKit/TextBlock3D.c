#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <SceneKit/SceneKit.h>

HANDLER(TextBlock3D, Node3D, Render)
{
  struct TextRun *pTextRun = GetTextRun(hObject);
  _SendMessage(hObject, TextBlockConcept, MakeText,
                   .availableSpace = 512);
  OBJ_SendMessageW(hObject, ID_TextBlockConcept_GetInfo, 0, &pTextRun->_textinfo);

  float w = pTextRun->_textinfo.txWidth;
  float h = pTextRun->_textinfo.txHeight;
  float measuredWidth = w * 0.1f;
  if (pTextBlock3D->MeasuredWidth != measuredWidth)
    PROP_SetValue(OBJ_FindLongProperty(hObject, ID_TextBlock3D_MeasuredWidth), &measuredWidth);

  float y = -h / 2;
  if (pTextBlock3D->BaselineOrigin) {
    float baseline;
    if (OBJ_SendMessageW(hObject, ID_TextBlockConcept_GetFirstVisibleBaseline, 0, &baseline))
      y = baseline - h;
  }

  struct ViewEntity entity = {
    .radius = (struct vec4){0},
    .bbox = BOX3_FromRect(((struct rect){-w/2,y,w,h})),
    .material = (struct ViewMaterial) {
      .opacity = GetNode3D(hObject)->_opacity,
      .color = pTextBlock3D->Color,   // all zero (unset) draws white
      .texture = (struct Texture*)_SendMessage(hObject, TextBlockConcept, GetTexture),
      .blendMode = BLEND_MODE_PREMULTIPLIED_ALPHA,
    },
    .matrix = GetNode3D(hObject)->Matrix,
  };

  struct mat4 scale;
  scale = MAT4_Identity();
  MAT4_Scale(&scale, &(struct vec3) { 0.1, 0.1, 0.1 });
  entity.matrix = MAT4_Multiply(&GetNode3D(hObject)->Matrix, &scale);
  entity.material.textureMatrix = MAT3_Identity();
  entity.material.textureMatrix.v[4] = -1;
  entity.material.textureMatrix.v[7] =  1;
  struct LinearGradientBrush const *g = pTextBlock3D->Foreground.Gradient;
  if (pTextBlock3D->Foreground.Material) {
    static struct uniform uniforms[MAX_UNIFORMS];
    struct Material *material = pTextBlock3D->Foreground.Material;
    entity.shader = material->Shader;
    entity.material.uniforms = uniforms;
    entity.material.numUniforms = OBJ_GetUniforms(hObject, uniforms);
    entity.material.numUniforms += OBJ_GetUniforms(CMP_GetObject(material),
                                                  uniforms + entity.material.numUniforms);
  } else if (pTextBlock3D->Foreground.Image) {
    static bool_t warned;
    if (!warned) {
      Con_Error("TextBlock3D Foreground.Image is not supported");
      warned = TRUE;
    }
    return FALSE;
  } else if (g) {
    entity.material.gradientEnabled = TRUE;
    entity.material.gradientStartColor = g->StartColor;
    entity.material.gradientEndColor = g->EndColor;
    entity.material.gradientStartPoint = g->StartPoint;
    entity.material.gradientEndPoint = g->EndPoint;
    entity.material.gradientSmooth = g->Interpolation == kGradientInterpolationSmoothStep;
    entity.material.gradientLinearColor = g->ColorSpace == kGradientColorSpaceLinear;
    entity.material.gradientSmoothStart = g->SmoothStart;
    entity.material.gradientSmoothEnd = g->SmoothEnd;
    entity.material.gradientCoordTransform = (struct vec4){1, -1, 0, 1};
    entity.material.gradientCoordTransformSet = TRUE;
  } else if (!PROP_IsNull(OBJ_FindLongProperty(hObject, ID_TextBlock3D_ForegroundColor))) {
    entity.material.gradientEnabled = TRUE;
    entity.material.gradientStartColor = pTextBlock3D->Foreground.Color;
    entity.material.gradientEndColor = pTextBlock3D->Foreground.Color;
  }

//  struct Property *hProp =
//  TextBlockConcept_GetProperty(hObject, kTextBlockConceptText);
//  if (*output->TextResourceID && !PROP_HasProgram(hProp)) {
//    Loc_GetString(
//                  OBJ_GetLocalization(hObject), output->TextResourceID, LOC_TEXT);
//  }

  if (g && entity.material.gradientEnabled && g->Mapping == kGradientMappingTextLine) {
    struct TextBlockConcept_DrawGradientLinesEventArgs args = {
      .view = pRender->ViewDef, .entity = &entity, .yUp = TRUE,
    };
    if (!OBJ_SendMessageW(hObject, ID_TextBlockConcept_DrawGradientLines, 0, &args))
      Con_Error("could not draw TextBlock3D gradient lines");
  } else {
    R_DrawEntity(pRender->ViewDef, &entity);
  }

  return TRUE;
}

HANDLER(TextBlock3D, Object, Create)
{
  pTextBlock3D->_node3D = GetNode3D(hObject);
  return FALSE;
}
