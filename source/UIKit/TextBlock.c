#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <source/UIKit/UIKit.h>


enum label_step
{
  STEP_GEOMETRY,
  STEP_IMAGE,
  STEP_COUNT
};

static bool_t
is_updated(lpObject_t hObject,
           enum label_step label_step)
{
  TextBlockConceptPtr output = GetTextBlockConcept(hObject);
  longTime_t dirty = OBJ_GetTimestamp(hObject);
  if (output->_steps[label_step] != dirty) {
    output->_steps[label_step] = dirty;
    return TRUE;
  } else {
    return FALSE;
  }
}

static float
text_pos(struct vec2 const* padding,
         uint32_t align,
         float size,
         float space)
{
  switch (align) {
    case kTextHorizontalAlignmentRight:
      return space - size + padding->y;
    case kTextHorizontalAlignmentCenter:
      return padding->y + (space - padding->x - padding->y - size) / 2;
    case kTextHorizontalAlignmentLeft:
    default:
      return padding->y;
  }
}

static struct rect
mesh_rect(Node2DPtr pNode2D,
          TextBlockConceptPtr frame,
          struct text_info* pTextInfo)
{
  struct rect const rect = {
    .width = Node2D_GetFrame(pNode2D, kBox3FieldWidth),
    .height = Node2D_GetFrame(pNode2D, kBox3FieldHeight),
  };
  return (struct rect){
    .x = text_pos((struct vec2*)&NODE2D_FRAME(pNode2D, Padding, 0),
                  frame->TextHorizontalAlignment,
                  pTextInfo->txWidth,
                  rect.width),
    .y = text_pos((struct vec2*)&NODE2D_FRAME(pNode2D, Padding, 1),
                  frame->TextVerticalAlignment,
                  pTextInfo->txHeight,
                  rect.height),
    .width = pTextInfo->txWidth,
    .height = pTextInfo->txHeight,
  };
}

HANDLER(TextBlockConcept, MakeText)
{
  TextBlockConceptPtr label = pTextBlockConcept;
  struct FontShorthand font = *label->_font;
  for (lpObject_t node = hObject; node; node = OBJ_GetParent(node)) {
    if (Node_GetProperty(node, kNodeFontSize)) {
      font.Size = GetNode(node)->Font.Size;
      break;
    }
  }
  struct view_text* text = pMakeText->text;
  lpcString_t szTextContent = OBJ_GetTextContent(hObject);
  text->string = label->PlaceholderText;
  text->font = font.Family?font.Family->font:NULL;
  text->fontSize = font.Size;
  text->flags = label->UseFullFontHeight ? RF_USE_FONT_HEIGHT : 0;
  text->lineSpacing = label->LineHeight;
  text->letterSpacing = label->LetterSpacing;
  text->underlineWidth = label->Underline.Width;
  text->underlineOffset = label->Underline.Offset;
  text->fixedCharacterWidth = label->FixedCharacterWidth;
  text->availableWidth = pMakeText->availableSpace;
  text->fontStyle = 0;
  text->backingScale = WI_GetScaling();

  lpProperty_t hProp =
    TextBlockConcept_GetProperty(hObject, kTextBlockConceptText);

  if (label->_font->Weight == kFontWeightBold) {
    text->fontStyle += FS_BOLD;
  }
  if (label->_font->Style == kFontStyleItalic) {
    text->fontStyle += FS_ITALIC;
  }

  if (*label->Text) {
    text->string = label->Text;
  } else if (*label->TextResourceID && !PROP_HasProgram(hProp)) {
    text->string = Loc_GetString(label->TextResourceID, LOC_TEXT);
  } else if (szTextContent && *szTextContent) {
    text->string = szTextContent = OBJ_GetTextContent(hObject);
  }

  return TRUE;
}

HANDLER(TextBlock, UpdateLayout)
{
  TextBlockConceptPtr output = GetTextBlockConcept(hObject);

  //	if (!is_updated(hObject, STEP_AXIS_X + pContentSize->axis) && *target
  //!=
  // 0) { 		Node2D_SetFrame(pTextBlock->_node2D, kBox3FieldWidth +
  // pContentSize->axis, *target); 		return *target;
  //	}

  float padding = TOTAL_PADDING(GetNode2D(hObject), kDirectionHorizontal);
  OBJ_SendMessageW(hObject, kEventMakeText, 0, &(MAKETEXTSTRUCT){
                     .text = &output->_text,
                     .availableSpace = pUpdateLayout->Width - padding
                   });
  Text_GetInfo(&output->_text, &output->_textinfo);

  output->_size[0] = output->_textinfo.txWidth + TOTAL_PADDING(pTextBlock->_node2D, 0);
  output->_size[1] = output->_textinfo.txHeight + TOTAL_PADDING(pTextBlock->_node2D, 1);

  FOR_LOOP(i, 2)
  {
    Node2D_SetFrame(pTextBlock->_node2D, kBox3FieldWidth + i, output->_size[i]);
    Node2D_Measure(pTextBlock->_node2D, i, pUpdateLayout->Size[i], FALSE);
  }

  return TRUE;
}

HANDLER(TextBlock, ForegroundContent)
{
  //    TextBlockConceptPtr label = GetTextBlockConcept(hObject);
  //    if (is_updated(hObject, STEP_IMAGE))
  //    {
  //        struct view_text text = text_from_label(hObject, label);
  //        Text_Print(&text, &label->_image, TRUE);
  //    }
  //    pForegroundContent->result = label->_image;
  return TRUE;
}

HANDLER(TextBlock, UpdateGeometry)
{
  if (is_updated(hObject, STEP_GEOMETRY)) {
    TextBlockConceptPtr label = GetTextBlockConcept(hObject);
    struct rect const rect = mesh_rect(pTextBlock->_node2D, label, &label->_textinfo);
    struct edges insets = label->_textinfo.txInsets;
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

HANDLER(TextBlock, DrawBrush)
{
	if (!memcmp(pDrawBrush->brush,
							&(struct BrushShorthand){0},
							sizeof(struct BrushShorthand)) &&
			!pDrawBrush->foreground)
    return FALSE;

  struct view_entity entity;
  TextBlockConceptPtr output = GetTextBlockConcept(hObject);

  if (output->PlaceholderText == output->_text.string && pDrawBrush->foreground) {
    static struct BrushShorthand zero = { 0 };
    if (memcmp(&output->Placeholder, &zero, sizeof(struct BrushShorthand))) {
      pDrawBrush->brush = &output->Placeholder;
    }
  }

  Node2D_GetViewEntity(hObject, &entity, pDrawBrush->image, pDrawBrush->brush);

  if (pDrawBrush->foreground) {
    entity.texture = 0;
    entity.radius = (struct vec4){0};
    entity.rect = pTextBlock->_node2D->_rect;
//    TextBlockConceptPtr label = GetTextBlockConcept(hObject);
//    entity.rect = mesh_rect(pTextBlock->_node2D, label, &label->_textinfo);
    entity.text = &output->_text;
    lpProperty_t hProp = TextBlockConcept_GetProperty(hObject, kTextBlockConceptText);
    if (*output->TextResourceID && !PROP_HasProgram(hProp)) {
      Loc_GetString(output->TextResourceID, LOC_TEXT);
    }
  } else {
    entity.rect = Node2D_GetBackgroundRect(pTextBlock->_node2D);
  }

  entity.borderWidth = pDrawBrush->borderWidth;
  entity.borderOffset = pDrawBrush->borderOffset;

  if (entity.rect.width == 0 || entity.rect.height == 0) {
    return TRUE;
  }
  
  R_DrawEntity(pDrawBrush->viewdef, &entity);

  return TRUE;
}

HANDLER(TextBlock, Create)
{
  lpProperty_t p;
  OBJ_FindShortProperty(hObject, "Text", &p);
  pTextBlock->_node2D = GetNode2D(hObject);
  return FALSE;
}

HANDLER(TextBlockConcept, Create)
{
  pTextBlockConcept->_node = GetNode(hObject);
  pTextBlockConcept->_font = &GetNode(hObject)->Font;
  return FALSE;
}
