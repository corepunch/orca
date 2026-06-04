#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <UIKit/UIKit.h>

#define PLACEHOLDER_OPACITY 0.5f

enum label_step
{
  STEP_GEOMETRY,
  STEP_IMAGE,
  STEP_COUNT
};

static bool_t
is_updated(struct Object *hObject,
           enum label_step label_step)
{
  struct TextBlockConcept *output = GetTextBlockConcept(hObject);
  longTime_t dirty = OBJ_GetTimestamp(hObject);
  if (output->_steps[label_step] != dirty) {
    output->_steps[label_step] = dirty;
    return TRUE;
  } else {
    return FALSE;
  }
}

float
text_pos(struct EdgeShorthand padding, uint32_t align, float size, float space)
{
  switch (align) {
    case kTextHorizontalAlignmentRight:
      return space - size + padding.Left;
    case kTextHorizontalAlignmentCenter:
      return padding.Left + (space - padding.Left - padding.Right - size) / 2;
    case kTextHorizontalAlignmentLeft:
    default:
      return padding.Left;
  }
}

static struct rect
mesh_rect(struct Node2D *pNode2D,
          struct TextBlockConcept *frame,
          struct text_info* pTextInfo)
{
  return (struct rect){
    .x = text_pos(NODE2D_FRAME(pNode2D, Padding, 0),
                  frame->TextHorizontalAlignment,
                  pTextInfo->txWidth,
                  Node2D_GetFrame(pNode2D, kBox3FieldWidth)),
    .y = text_pos(NODE2D_FRAME(pNode2D, Padding, 1),
                  frame->TextVerticalAlignment,
                  pTextInfo->txHeight,
                  Node2D_GetFrame(pNode2D, kBox3FieldHeight)),
    .width = pTextInfo->txWidth,
    .height = pTextInfo->txHeight,
  };
}

HANDLER(TextBlock, Node2D, MeasureOverride)
{
  struct TextRun *output = GetTextRun(hObject);
  struct TextBlockConcept *textblock = GetTextBlockConcept(hObject);
  _SendMessage(hObject, TextBlockConcept, MakeText,
                   .availableSpace = pMeasureOverride->Width);
  TextBlockText_GetInfo(textblock->_text, &output->_textinfo);
  return MAKEDWORD(output->_textinfo.txWidth, output->_textinfo.txHeight);
}

HANDLER(TextBlock, Node2D, ForegroundContent)
{
  struct TextBlockConcept *text = GetTextBlockConcept(hObject);
  return (intptr_t)TextBlockText_GetTexture(text->_text);
}

HANDLER(TextBlock, Node2D, UpdateGeometry)
{
  if (is_updated(hObject, STEP_GEOMETRY)) {
    struct TextRun *run = GetTextRun(hObject);
    struct rect const rect = mesh_rect(pTextBlock->_node2D, GetTextBlockConcept(hObject), &run->_textinfo);
    struct edges insets = run->_textinfo.txInsets;
    struct rect const geom = {
      .x = floorf(rect.x - insets.left),
      .y = floorf(rect.y - insets.top),
      .width = floorf(rect.width + insets.right + insets.left),
      .height = floorf(rect.height + insets.bottom + insets.top),
    };
    pTextBlock->_node2D->_rect = geom;
  }
  return TRUE;
}

HANDLER(TextBlock, Node2D, DrawBrush)
{
	if (!memcmp(&pDrawBrush->brush,
							&(struct BrushShorthand){0},
							sizeof(struct BrushShorthand)) &&
			!pDrawBrush->foreground)
    return FALSE;

  struct ViewEntity entity;
  struct TextBlockConcept *text = GetTextBlockConcept(hObject);
  float modopacity = 1.f;
  if (text->_text->placeholder && pDrawBrush->foreground) {
    static struct BrushShorthand zero = { 0 };
    if (memcmp(&text->Placeholder, &zero, sizeof(struct BrushShorthand))) {
      pDrawBrush->brush = text->Placeholder;
    } else {
      modopacity = PLACEHOLDER_OPACITY;
    }
  }

  Node2D_GetViewEntity(GetNode2D(hObject), &entity, pDrawBrush->image, &pDrawBrush->brush);

  if (pDrawBrush->foreground) {
    entity.material.texture = 0;
    entity.material.opacity *= modopacity;
    entity.radius = (struct vec4){0};
    entity.bbox = BOX3_FromRect(pTextBlock->_node2D->_rect);
//    struct TextBlockConcept *label = GetTextBlockConcept(hObject);
//    entity.bbox = BOX3_FromRect(mesh_rect(pTextBlock->_node2D, label, &label->_textinfo));
    struct Property *hProp = TextRun_GetProperty(hObject, kTextRunText);
    if (text->TextResourceID && *text->TextResourceID && !PROP_HasProgram(hProp)) {
      Loc_GetString(text->TextResourceID, LOC_TEXT);
    }
  } else {
    entity.bbox = BOX3_FromRect(Node2D_GetBackgroundRect(pTextBlock->_node2D));
  }

  entity.borderWidth = pDrawBrush->borderWidth;
  entity.borderOffset = pDrawBrush->borderOffset;

  float bbox_width = entity.bbox.max.x - entity.bbox.min.x;
  float bbox_height = entity.bbox.max.y - entity.bbox.min.y;
  if (bbox_width == 0 || bbox_height == 0) {
    return TRUE;
  }

  R_DrawEntity(pDrawBrush->viewdef, &entity);

  return TRUE;
}

HANDLER(TextBlock, Object, Create)
{
  struct Property *p;
  OBJ_FindShortProperty(hObject, "Text", &p);
  pTextBlock->_node2D = GetNode2D(hObject);
  return FALSE;
}
