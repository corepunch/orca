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
text_pos(lpcvec2_t padding,
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

static lpcString_t
_GetTextBlockText(lpObject_t hObject,
                  TextBlockConceptPtr pTextBlockConcept,
                  TextRunPtr pTextRun)
{
  lpProperty_t hProp = TextRun_GetProperty(hObject, kTextRunText);
  if (*pTextRun->Text)
  {
    return pTextRun->Text;
  }
  else if (*pTextBlockConcept->TextResourceID && !PROP_HasProgram(hProp))
  {
    return Loc_GetString(pTextBlockConcept->TextResourceID, LOC_TEXT);
  }
  else if (OBJ_GetTextContent(hObject) && *OBJ_GetTextContent(hObject))
  {
    return OBJ_GetTextContent(hObject);
  }
  else
  {
    return pTextBlockConcept->PlaceholderText;
  }
}

static struct ViewTextRun
_MakeViewTextRun(lpObject_t hObject, TextRunPtr pTextRun, lpcString_t szText)
{
  struct FontShorthand font = pTextRun->Font;
  for (lpObject_t node = hObject; node; node = OBJ_GetParent(node)) {
    lpProperty_t plist = OBJ_GetProperties(node);
    lpProperty_t p = PROP_FindByLongID(plist, ID_TextRun_FontSize);
    if (p) {
      font.Size = *(float const *)PROP_GetValue(p);
      break;
    }
//    if (Node_GetProperty(node, kNodeFontSize)) {
//      font.Size = GetNode(node)->Font.Size;
//      break;
//    }
  }
  struct ViewTextRun view = {
    .string = szText,
    .fontFamily = font.Family,
    .fontSize = font.Size,
    .letterSpacing = pTextRun->LetterSpacing,
    .fixedCharacterWidth = pTextRun->FixedCharacterWidth,
    .underlineWidth = pTextRun->Underline.Width,
    .underlineOffset = pTextRun->Underline.Offset,
    .fontStyle = 0,
  };
  if (font.Weight == kFontWeightBold) {
    view.fontStyle += FS_BOLD;
  }
  if (font.Style == kFontStyleItalic) {
    view.fontStyle += FS_ITALIC;
  }
  return view;
}

HANDLER(TextBlockConcept, MakeText)
{
  TextRunPtr pTextRun = GetTextRun(hObject);
  struct ViewText* pViewText = pMakeText->text;
//  lpcString_t szTextContent = OBJ_GetTextContent(hObject);
  pViewText->run[0] = _MakeViewTextRun(hObject, pTextRun, _GetTextBlockText(hObject, pTextBlockConcept, pTextRun));
  pViewText->numTextRuns = 1;
  FOR_EACH_OBJECT(run, hObject) {
    TextRunPtr tr = GetTextRun(run);
    if (tr && pViewText->numTextRuns < MAX_TEXT_RUNS) {
      lpcString_t str = *tr->Text ? tr->Text : OBJ_GetTextContent(run);
      pViewText->run[pViewText->numTextRuns++] = _MakeViewTextRun(run, tr, str);
    }
  }
  pViewText->flags = pTextBlockConcept->UseFullFontHeight ? RF_USE_FONT_HEIGHT : 0;
//  pViewText->lineSpacing = pTextRun->LineHeight;
  pViewText->availableWidth = pMakeText->availableSpace;
  pViewText->scale = WI_GetScaling();
  return TRUE;
}

HANDLER(TextBlock, UpdateLayout)
{
  TextRunPtr output = GetTextRun(hObject);

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
  //        struct ViewText text = text_from_label(hObject, label);
  //        Text_Print(&text, &label->_image, TRUE);
  //    }
  //    pForegroundContent->result = label->_image;
  return TRUE;
}

HANDLER(TextBlock, UpdateGeometry)
{
  if (is_updated(hObject, STEP_GEOMETRY)) {
    TextRunPtr run = GetTextRun(hObject);
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

HANDLER(TextBlock, DrawBrush)
{
	if (!memcmp(pDrawBrush->brush,
							&(struct BrushShorthand){0},
							sizeof(struct BrushShorthand)) &&
			!pDrawBrush->foreground)
    return FALSE;

  struct ViewEntity entity;
  TextBlockConceptPtr text = GetTextBlockConcept(hObject);
  TextRunPtr run = GetTextRun(hObject);

  if (text->PlaceholderText == run->_text.run[0].string && pDrawBrush->foreground) {
    static struct BrushShorthand zero = { 0 };
    if (memcmp(&text->Placeholder, &zero, sizeof(struct BrushShorthand))) {
      pDrawBrush->brush = &text->Placeholder;
    }
  }

  Node2D_GetViewEntity(hObject, &entity, pDrawBrush->image, pDrawBrush->brush);

  if (pDrawBrush->foreground) {
    entity.texture = 0;
    entity.radius = (struct vec4){0};
    entity.rect = pTextBlock->_node2D->_rect;
//    TextBlockConceptPtr label = GetTextBlockConcept(hObject);
//    entity.rect = mesh_rect(pTextBlock->_node2D, label, &label->_textinfo);
    entity.text = &run->_text;
    lpProperty_t hProp = TextRun_GetProperty(hObject, kTextRunText);
    if (*text->TextResourceID && !PROP_HasProgram(hProp)) {
      Loc_GetString(text->TextResourceID, LOC_TEXT);
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
  return FALSE;
}
