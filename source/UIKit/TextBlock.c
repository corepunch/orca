#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <source/UIKit/UIKit.h>

#define PLACEHOLDER_OPACITY 0.5f

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

float
text_pos(EdgeShorthand_t padding, uint32_t align, float size, float space)
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
mesh_rect(Node2DPtr pNode2D,
          TextBlockConceptPtr frame,
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
_MakeViewTextRun(lpObject_t hObject, TextRun_t text, lpcString_t szText)
{
  for (lpObject_t node = hObject; node; node = OBJ_GetParent(node)) {
    lpProperty_t plist = OBJ_GetProperties(node);
    lpProperty_t p = PROP_FindByLongID(plist, ID_TextRun_FontSize);
    if (p) {
      text.Font.Size = *(float const *)PROP_GetValue(p);
      break;
    }
//    if (Node_GetProperty(node, kNodeFontSize)) {
//      font.Size = GetNode(node)->Font.Size;
//      break;
//    }
  }
  struct ViewTextRun view = {
    .string = szText,
    .fontFamily = text.Font.Family,
    .fontSize = text.Font.Size,
    .letterSpacing = text.LetterSpacing,
    .fixedCharacterWidth = text.FixedCharacterWidth,
    .underlineWidth = text.Underline.Width,
    .underlineOffset = text.Underline.Offset,
    .fontStyle = 0,
  };
  if (text.Font.Weight == kFontWeightBold) {
    view.fontStyle += FS_BOLD;
  }
  if (text.Font.Style == kFontStyleItalic) {
    view.fontStyle += FS_ITALIC;
  }
  return view;
}

HANDLER(TextBlockConcept, MakeText)
{
  TextRunPtr pTextRun = GetTextRun(hObject);
  struct ViewText* pViewText = pMakeText->text;
//  lpcString_t szTextContent = OBJ_GetTextContent(hObject);
  pViewText->run[0] = _MakeViewTextRun(hObject, *pTextRun, _GetTextBlockText(hObject, pTextBlockConcept, pTextRun));
  pViewText->numTextRuns = 1;
  FOR_EACH_OBJECT(run, hObject) {
    TextRunPtr tr = GetTextRun(run);
    if (tr && pViewText->numTextRuns < MAX_TEXT_RUNS) {
      lpcString_t str = *tr->Text ? tr->Text : OBJ_GetTextContent(run);
      TextRun_t base = *pTextRun;
      if (TextRun_GetProperty(run, kTextRunFont)) base.Font = tr->Font;
      if (TextRun_GetProperty(run, kTextRunFontWeight)) base.Font.Weight = tr->Font.Weight;
      if (TextRun_GetProperty(run, kTextRunFontStyle)) base.Font.Style = tr->Font.Style;
      if (TextRun_GetProperty(run, kTextRunFontSize)) base.Font.Size = tr->Font.Size;
      if (TextRun_GetProperty(run, kTextRunFontFamily)) base.Font.Family = tr->Font.Family;
      if (TextRun_GetProperty(run, kTextRunUnderline)) base.Underline = tr->Underline;
      if (TextRun_GetProperty(run, kTextRunUnderlineOffset)) base.Underline.Offset = tr->Underline.Offset;
      if (TextRun_GetProperty(run, kTextRunUnderlineWidth)) base.Underline.Width = tr->Underline.Width;
      if (TextRun_GetProperty(run, kTextRunUnderlineColor)) base.Underline.Color = tr->Underline.Color;
      if (TextRun_GetProperty(run, kTextRunLetterSpacing)) base.LetterSpacing = tr->LetterSpacing;
      if (TextRun_GetProperty(run, kTextRunLineHeight)) base.LineHeight = tr->LineHeight;
      if (TextRun_GetProperty(run, kTextRunCharacterSpacing)) base.CharacterSpacing = tr->CharacterSpacing;
      if (TextRun_GetProperty(run, kTextRunFixedCharacterWidth)) base.FixedCharacterWidth = tr->FixedCharacterWidth;
      if (TextRun_GetProperty(run, kTextRunRemoveSideBearingsProperty)) base.RemoveSideBearingsProperty = tr->RemoveSideBearingsProperty;
      pViewText->run[pViewText->numTextRuns++] = _MakeViewTextRun(run, base, str);
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
  TextBlockConceptPtr textblock = GetTextBlockConcept(hObject);

  //	if (!is_updated(hObject, STEP_AXIS_X + pContentSize->axis) && *target
  //!=
  // 0) { 		Node2D_SetFrame(pTextBlock->_node2D, kBox3FieldWidth +
  // pContentSize->axis, *target); 		return *target;
  //	}

  float padding = TOTAL_PADDING(GetNode2D(hObject), kDirectionHorizontal);
  OBJ_SendMessageW(hObject, kEventMakeText, 0, &(MAKETEXTSTRUCT){
                     .text = textblock->_text,
                     .availableSpace = pUpdateLayout->Width - padding
                   });
  Text_GetInfo(textblock->_text, &output->_textinfo);

  output->TextureWidth = (float)output->_textinfo.txWidth;
  output->TextureHeight = (float)output->_textinfo.txHeight;
  output->_size[0] = output->TextureWidth + TOTAL_PADDING(pTextBlock->_node2D, 0);
  output->_size[1] = output->TextureHeight + TOTAL_PADDING(pTextBlock->_node2D, 1);

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
  float modopacity = 1.f;
  if (text->PlaceholderText == text->_text->run[0].string && pDrawBrush->foreground) {
    static struct BrushShorthand zero = { 0 };
    if (memcmp(&text->Placeholder, &zero, sizeof(struct BrushShorthand))) {
      pDrawBrush->brush = &text->Placeholder;
    } else {
      modopacity = PLACEHOLDER_OPACITY;
    }
  }

  Node2D_GetViewEntity(hObject, &entity, pDrawBrush->image, pDrawBrush->brush);

  if (pDrawBrush->foreground) {
    entity.material.texture = 0;
    entity.material.opacity *= modopacity;
    entity.radius = (struct vec4){0};
    entity.bbox = BOX3_FromRect(pTextBlock->_node2D->_rect);
//    TextBlockConceptPtr label = GetTextBlockConcept(hObject);
//    entity.bbox = BOX3_FromRect(mesh_rect(pTextBlock->_node2D, label, &label->_textinfo));
    entity.text = text->_text;
    lpProperty_t hProp = TextRun_GetProperty(hObject, kTextRunText);
    if (*text->TextResourceID && !PROP_HasProgram(hProp)) {
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
  pTextBlockConcept->_text = ZeroAlloc(sizeof(struct ViewText) + sizeof(struct ViewTextRun) * MAX_TEXT_RUNS);
  return FALSE;
}

HANDLER(TextBlockConcept, Destroy)
{
  SafeDelete(pTextBlockConcept->_text, free);
  return FALSE;
}
