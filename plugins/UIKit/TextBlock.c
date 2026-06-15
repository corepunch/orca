#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <UIKit/UIKit.h>

#define PLACEHOLDER_OPACITY 0.5f

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

static struct rect
text_texture_rect(struct Node2D *pNode2D,
                  struct TextBlockConcept *frame,
                  struct text_info *pTextInfo)
{
  struct rect const rect = mesh_rect(pNode2D, frame, pTextInfo);
  struct edges insets = pTextInfo->txInsets;
  return (struct rect){
    .x = floorf(rect.x - insets.left),
    .y = floorf(rect.y - insets.top),
    .width = floorf(rect.width + insets.right + insets.left),
    .height = floorf(rect.height + insets.bottom + insets.top),
  };
}

// TextBlock_MeasureOverride
HANDLER(TextBlock, Node2D, MeasureOverride)
{
  struct TextRun *output = GetTextRun(hObject);
  struct TextBlockConcept *textblock = GetTextBlockConcept(hObject);
  _SendMessage(hObject, TextBlockConcept, MakeText,
                   .availableSpace = pMeasureOverride->Width);
  TextBlockText_GetInfo(textblock->_text, &output->_textinfo);
  return MAKEDWORD(output->_textinfo.txWidth, output->_textinfo.txHeight);
}

// TextBlock_ForegroundContent
HANDLER(TextBlock, Node2D, ForegroundContent)
{
  struct TextBlockConcept *text = GetTextBlockConcept(hObject);
  return (intptr_t)TextBlockText_GetTexture(text->_text);
}

// TextBlock_UpdateGeometry
HANDLER(TextBlock, Node2D, UpdateGeometry)
{
  struct TextRun *run = GetTextRun(hObject);
  struct TextBlockConcept *text = GetTextBlockConcept(hObject);
  float availableWidth = Node2D_GetFrame(pTextBlock->_node2D, kBox3FieldWidth) -
                         TOTAL_PADDING(pTextBlock->_node2D, 0);
  _SendMessage(hObject, TextBlockConcept, MakeText,
               .availableSpace = MAX(0, availableWidth));
  TextBlockText_GetInfo(text->_text, &run->_textinfo);
  pTextBlock->_node2D->_rect = text_texture_rect(pTextBlock->_node2D, text, &run->_textinfo);
  return TRUE;
}

// TextBlock_DrawForeground
HANDLER(TextBlock, Node2D, DrawForeground)
{
  if (!memcmp(&pDrawForeground->brush, &(struct BrushShorthand){0}, sizeof(struct BrushShorthand))) return FALSE;

  struct ViewEntity entity;
  struct TextBlockConcept *text = GetTextBlockConcept(hObject);
  float modopacity = 1.f;
  if (text->_text->placeholder) {
    static struct BrushShorthand zero = { 0 };
    if (memcmp(&text->Placeholder, &zero, sizeof(struct BrushShorthand))) {
      pDrawForeground->brush = text->Placeholder;
    } else {
      modopacity = PLACEHOLDER_OPACITY;
    }
  }

  Node2D_GetViewEntity(GetNode2D(hObject), &entity, pDrawForeground->image, &pDrawForeground->brush);

  entity.material.opacity *= modopacity;
  entity.radius = (struct vec4){0};
  entity.bbox = BOX3_FromRect(pTextBlock->_node2D->_rect);
  struct Property *hProp = TextRun_GetProperty(hObject, kTextRunText);
  if (text->TextResourceID && *text->TextResourceID && !PROP_HasProgram(hProp)) {
    Loc_GetString(text->TextResourceID, LOC_TEXT);
  }

  entity.borderWidth = pDrawForeground->borderWidth;
  entity.borderOffset = pDrawForeground->borderOffset;

  float bbox_width = entity.bbox.max.x - entity.bbox.min.x;
  float bbox_height = entity.bbox.max.y - entity.bbox.min.y;
  if (bbox_width == 0 || bbox_height == 0) {
    return TRUE;
  }

  R_DrawEntity(pDrawForeground->viewdef, &entity);

  return TRUE;
}

// TextBlock_Create
HANDLER(TextBlock, Object, Create)
{
  OBJ_FindLongProperty(hObject, ID_TextRun_Text); // Initialize text
  pTextBlock->_node2D = GetNode2D(hObject);
  return FALSE;
}
