#include <UIKit/UIKit.h>
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
  struct TextBlock *output = GetTextBlock(hObject);
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
          struct TextBlock *frame,
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
_GetTextBlockText(struct Object *hObject,
                  struct TextBlock *pTextBlock)
{
  struct Property *hProp = TextBlock_GetProperty(hObject, Text);
  if (pTextBlock->Text && *pTextBlock->Text)
  {
    return pTextBlock->Text;
  }
  else if (pTextBlock->TextResourceID && *pTextBlock->TextResourceID && !PROP_HasProgram(hProp))
  {
    return Loc_GetString(pTextBlock->TextResourceID, LOC_TEXT);
  }
  else if (OBJ_GetTextContent(hObject) && *OBJ_GetTextContent(hObject))
  {
    return OBJ_GetTextContent(hObject);
  }
  else
  {
    return pTextBlock->PlaceholderText;
  }
}

static struct ViewTextRun
_MakeViewTextRun(struct Object *hObject,
                 struct TextBlock text,
                 lpcString_t szText)
{
  (void)hObject;
  struct FontShorthand font = { .Size = DEFAULT_FONT_SIZE };
  font.Weight = IVALUE(text.Font.Weight, font.Weight);
  font.Style = IVALUE(text.Font.Style, font.Style);
  font.Size = IVALUE(text.Font.Size, font.Size ? font.Size : DEFAULT_FONT_SIZE);
  font.Family = IVALUE(text.Font.Family, font.Family);
  struct ViewTextRun view = {
    .string = szText,
    .fontFamily = font.Family,
    .fontSize = font.Size,
    .letterSpacing = text.LetterSpacing,
    .fixedCharacterWidth = text.FixedCharacterWidth,
    .underlineWidth = text.Underline.Width,
    .underlineOffset = text.Underline.Offset,
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

HANDLER(TextBlock, TextBlock, MakeText)
{
  struct ViewText* pViewText = pMakeText->text;
  pViewText->run[0] = _MakeViewTextRun(hObject, *pTextBlock, _GetTextBlockText(hObject, pTextBlock));
  pViewText->numTextRuns = 1;
  FOR_EACH_OBJECT(run, hObject) {
    struct TextRun *tr = GetTextRun(run);
    if (tr && pViewText->numTextRuns < MAX_TEXT_RUNS) {
      lpcString_t str = (tr->Text && *tr->Text) ? tr->Text : OBJ_GetTextContent(run);
      struct TextBlock base = *pTextBlock;
      if (tr->Font.Weight) base.Font.Weight = tr->Font.Weight;
      if (tr->Font.Style) base.Font.Style = tr->Font.Style;
      if (tr->Font.Size) base.Font.Size = tr->Font.Size;
      if (tr->Font.Family) base.Font.Family = tr->Font.Family;
      if (TextRun_GetProperty(run, Underline.Offset)) base.Underline.Offset = tr->Underline.Offset;
      if (TextRun_GetProperty(run, Underline.Width)) base.Underline.Width = tr->Underline.Width;
      if (TextRun_GetProperty(run, Underline.Color)) base.Underline.Color = tr->Underline.Color;
      if (TextRun_GetProperty(run, LetterSpacing)) base.LetterSpacing = tr->LetterSpacing;
      if (TextRun_GetProperty(run, LineHeight)) base.LineHeight = tr->LineHeight;
      if (TextRun_GetProperty(run, CharacterSpacing)) base.CharacterSpacing = tr->CharacterSpacing;
      if (TextRun_GetProperty(run, FixedCharacterWidth)) base.FixedCharacterWidth = tr->FixedCharacterWidth;
      if (TextRun_GetProperty(run, RemoveSideBearingsProperty)) base.RemoveSideBearingsProperty = tr->RemoveSideBearingsProperty;
      pViewText->run[pViewText->numTextRuns++] = _MakeViewTextRun(run, base, str);
    }
  }
  pViewText->flags = pTextBlock->UseFullFontHeight ? RF_USE_FONT_HEIGHT : 0;
  pViewText->availableWidth = pMakeText->availableSpace;
  pViewText->textWrapping = (enum text_wrap)pTextBlock->TextWrapping;
  pViewText->textOverflow = (enum text_overflow)pTextBlock->TextOverflow;
  pViewText->scale = axGetScaling();
  return TRUE;
}

HANDLER(TextBlock, Node2D, MeasureOverride)
{
  _SendMessage(hObject, TextBlock, MakeText,
                   .text = pTextBlock->_text,
                   .availableSpace = pMeasureOverride->Width);
  Text_GetInfo(pTextBlock->_text, &pTextBlock->_textinfo);
  return MAKEDWORD(pTextBlock->_textinfo.txWidth, pTextBlock->_textinfo.txHeight);
}

HANDLER(TextBlock, Node2D, ForegroundContent)
{
  return FALSE;
}

HANDLER(TextBlock, Node2D, UpdateGeometry)
{
  if (is_updated(hObject, STEP_GEOMETRY)) {
    struct rect const rect = mesh_rect(pTextBlock->_node2D, pTextBlock, &pTextBlock->_textinfo);
    struct edges insets = pTextBlock->_textinfo.txInsets;
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
  struct TextBlock *text = GetTextBlock(hObject);
  float modopacity = 1.f;
  if (text->PlaceholderText == text->_text->run[0].string && pDrawBrush->foreground) {
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
//    struct TextBlock *label = GetTextBlock(hObject);
//    entity.bbox = BOX3_FromRect(mesh_rect(pTextBlock->_node2D, label, &label->_textinfo));
    entity.text = text->_text;
    struct Property *hProp = TextBlock_GetProperty(hObject, Text);
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
  pTextBlock->_node = GetNode(hObject);
  pTextBlock->_text = ZeroAlloc(sizeof(struct ViewText) + sizeof(struct ViewTextRun) * MAX_TEXT_RUNS);
  return FALSE;
}

HANDLER(TextBlock, Object, Destroy)
{
  SafeDelete(pTextBlock->_text, free);
  return FALSE;
}
