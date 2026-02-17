#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include "SceneKit.h"

#if 0
static void
text_from_label(lpObject_t  hObject, struct TextBlock3D *label, struct ViewText* text)
{
	text->font          = label->Font;
	text->fontSize      = label->FontSize;
	text->flags         = label->UseFullFontHeight ? RF_USE_FONT_HEIGHT : 0;
	text->lineSpacing   = label->LineHeight;
	text->letterSpacing = label->LetterSpacing;
	text->fixedCharacterWidth = label->FixedCharacterWidth;
	
	memcpy(text->string, label->Text, MAX_PROPERTY_STRING);
}

static struct rect
mesh_rect(lpObject_t  hObject, struct TextBlock3D *frame)
{
	struct ViewText     text = {0};
	struct text_info info;
	text_from_label(hObject, frame, &text);
	Text_GetInfo(&text, info);
//	struct rect const rect = {
//		.width  = Node2D_GetFrame(hObject, kBox3FieldWidth),
//		.height = Node2D_GetFrame(hObject, kBox3FieldHeight),
//	};
	return (struct rect){
		.x      = 0,//text_x(frame, info.txWidth, rect.width),
		.y      = 0,//text_y(frame, info.txHeight, rect.height),
		.width  = info.txWidth,
		.height = info.txHeight,
	};
}
#endif

HANDLER(TextBlock3D, Render)
{
#if 0
	struct ViewText text;
	text_from_label(hObject, pTextBlock3D, &text);
	struct ViewEntity entity = {
		.debugName   = OBJ_GetName(hObject),
		.type        = ET_PLANE,
		.opacity     = GetNode3D(hObject)->_opacity,
		.matrix      = GetNode3D(hObject)->Matrix,
		.rect        = mesh_rect(hObject, pTextBlock3D),
		.text 		 = &text,
	};
	
	R_DrawEntity(parm, &entity);
#endif
  
  TextRunPtr pTextRun = GetTextRun(hObject);
  TextBlockConceptPtr pTextBlock = GetTextBlockConcept(hObject);
  OBJ_SendMessageW(hObject, kEventMakeText, 0, &(MAKETEXTSTRUCT){
                     .text = pTextBlock->_text,
                     .availableSpace = 512
                   });
  Text_GetInfo(pTextBlock->_text, &pTextRun->_textinfo);
  
  float w = pTextRun->_textinfo.txWidth;
  float h = pTextRun->_textinfo.txHeight;
  
  struct ViewEntity entity = {
    .debugName = OBJ_GetName(hObject),
    .texture = 0,
    .radius = (struct vec4){0},
    .rect = (struct rect){-w/2,-h/2,w,h},
    .text = pTextBlock->_text,
    .material = (struct ViewMaterial) {
      .opacity = GetNode3D(hObject)->_opacity,
      .color = {1,1,1,1},
    },
    .matrix = GetNode3D(hObject)->Matrix,
    .blendMode = BLEND_MODE_PREMULTIPLIED_ALPHA,
  };
  
  struct mat4 scale;
  scale = MAT4_Identity();
  MAT4_Scale(&scale, &(struct vec3) { 0.1, 0.1, 0.1 });
  entity.matrix = MAT4_Multiply(&GetNode3D(hObject)->Matrix, &scale);
  entity.textureMatrix = MAT3_Identity();
  entity.textureMatrix.v[4] = -1;
  entity.textureMatrix.v[7] =  1;
  
//  lpProperty_t hProp =
//  TextBlockConcept_GetProperty(hObject, kTextBlockConceptText);
//  if (*output->TextResourceID && !PROP_HasProgram(hProp)) {
//    Loc_GetString(
//                  OBJ_GetLocalization(hObject), output->TextResourceID, LOC_TEXT);
//  }
  
  R_DrawEntity(pRender, &entity);
  
  return TRUE;
}

HANDLER(TextBlock3D, Create)
{
  pTextBlock3D->_node3D = GetNode3D(hObject);
  return FALSE;
}
