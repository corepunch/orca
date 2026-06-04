#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <UIKit/UIKit.h>

static lpcString_t
_GetTextBlockText(struct Object *hObject,
                  struct TextBlockConcept *pTextBlockConcept,
                  struct TextRun *pTextRun)
{
  struct Property *hProp = TextRun_GetProperty(hObject, kTextRunText);
  if (pTextRun->Text && *pTextRun->Text)
  {
    return pTextRun->Text;
  }
  else if (pTextBlockConcept->TextResourceID && *pTextBlockConcept->TextResourceID && !PROP_HasProgram(hProp))
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

static struct TextBlockTextRun
_MakeTextBlockTextRun(struct Object *hObject,
                      struct TextRun text,
                      lpcString_t szText)
{
  struct FontShorthand font = { .Size = DEFAULT_FONT_SIZE };

  TextRun_ReadProperty(hObject, Font.Weight, &font.Weight);
  TextRun_ReadProperty(hObject, Font.Style, &font.Style);
  TextRun_ReadProperty(hObject, Font.Size, &font.Size);
  TextRun_ReadProperty(hObject, Font.Family, &font.Family);

  struct TextBlockTextRun view = {
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
    view.fontStyle += UI_TEXT_STYLE_BOLD;
  }
  if (font.Style == kFontStyleItalic) {
    view.fontStyle += UI_TEXT_STYLE_ITALIC;
  }
  return view;
}

HANDLER(TextBlockConcept, TextBlockConcept, MakeText)
{
  struct TextRun *pTextRun = GetTextRun(hObject);
  struct TextBlockText* pViewText = pTextBlockConcept->_text;
//  lpcString_t szTextContent = OBJ_GetTextContent(hObject);
  pViewText->run[0] = _MakeTextBlockTextRun(hObject, *pTextRun, _GetTextBlockText(hObject, pTextBlockConcept, pTextRun));
  pViewText->placeholder = pTextBlockConcept->PlaceholderText == pViewText->run[0].string;
  pViewText->numTextRuns = 1;
  FOR_EACH_OBJECT(run, hObject) {
    struct TextRun *tr = GetTextRun(run);
    if (tr && pViewText->numTextRuns < UI_MAX_TEXT_RUNS) {
      lpcString_t str = (tr->Text && *tr->Text) ? tr->Text : OBJ_GetTextContent(run);
      struct TextRun base = *pTextRun;
      if (tr->Font.Weight) base.Font.Weight = tr->Font.Weight;
      if (tr->Font.Style) base.Font.Style = tr->Font.Style;
      if (tr->Font.Size) base.Font.Size = tr->Font.Size;
      if (tr->Font.Family) base.Font.Family = tr->Font.Family;
      if (TextRun_GetProperty(run, kTextRunUnderlineOffset)) base.Underline.Offset = tr->Underline.Offset;
      if (TextRun_GetProperty(run, kTextRunUnderlineWidth)) base.Underline.Width = tr->Underline.Width;
      if (TextRun_GetProperty(run, kTextRunUnderlineColor)) base.Underline.Color = tr->Underline.Color;
      if (TextRun_GetProperty(run, kTextRunLetterSpacing)) base.LetterSpacing = tr->LetterSpacing;
      if (TextRun_GetProperty(run, kTextRunLineHeight)) base.LineHeight = tr->LineHeight;
      if (TextRun_GetProperty(run, kTextRunCharacterSpacing)) base.CharacterSpacing = tr->CharacterSpacing;
      if (TextRun_GetProperty(run, kTextRunFixedCharacterWidth)) base.FixedCharacterWidth = tr->FixedCharacterWidth;
      if (TextRun_GetProperty(run, kTextRunRemoveSideBearingsProperty)) base.RemoveSideBearingsProperty = tr->RemoveSideBearingsProperty;
      pViewText->run[pViewText->numTextRuns++] = _MakeTextBlockTextRun(run, base, str);
    }
  }
  pViewText->flags = pTextBlockConcept->UseFullFontHeight ? UI_TEXT_USE_FONT_HEIGHT : 0;
//  pViewText->lineSpacing = pTextRun->LineHeight;
  pViewText->availableWidth = pMakeText->availableSpace;
  pViewText->textWrapping = (uint32_t)pTextBlockConcept->TextWrapping;
  pViewText->textOverflow = (uint32_t)pTextBlockConcept->TextOverflow;
  pViewText->scale = axGetScaling();
  return TRUE;
}

HANDLER(TextBlockConcept, Object, Create)
{
  pTextBlockConcept->_node = GetNode(hObject);
  pTextBlockConcept->_text = ZeroAlloc(sizeof(struct TextBlockText));
  return FALSE;
}

HANDLER(TextBlockConcept, Object, Destroy)
{
  TextBlockText_Release(pTextBlockConcept->_text);
  return FALSE;
}
