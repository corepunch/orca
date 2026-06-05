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
  
  struct ViewEntity entity = {
    .radius = (struct vec4){0},
    .bbox = BOX3_FromRect(((struct rect){-w/2,-h/2,w,h})),
    .material = (struct ViewMaterial) {
      .opacity = GetNode3D(hObject)->_opacity,
      .color = {1,1,1,1},
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
  
//  struct Property *hProp =
//  TextBlockConcept_GetProperty(hObject, kTextBlockConceptText);
//  if (*output->TextResourceID && !PROP_HasProgram(hProp)) {
//    Loc_GetString(
//                  OBJ_GetLocalization(hObject), output->TextResourceID, LOC_TEXT);
//  }
  
  R_DrawEntity(pRender->ViewDef, &entity);
  
  return TRUE;
}

HANDLER(TextBlock3D, Object, Create)
{
  pTextBlock3D->_node3D = GetNode3D(hObject);
  return FALSE;
}
