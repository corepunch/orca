#ifndef __TEXTBLOCKTEXT_H__
#define __TEXTBLOCKTEXT_H__

#define UI_MAX_TEXT_RUNS 256

struct FontFamily;
struct Texture;

struct TextBlockTextRun
{
  lpcString_t string;
  struct FontFamily const *fontFamily; // NOTE: must stay second for TextBlockText_GetHash()
  uint32_t fontStyle;
  uint32_t fontSize;
  uint32_t fixedCharacterWidth;
  uint32_t underlineWidth;
  uint32_t underlineOffset;
  float letterSpacing;
  float lineHeight;
};

struct TextBlockText
{
  uint32_t flags;
  float availableWidth;
  uint32_t cursor;
  uint32_t numTextRuns;
  float scale;
  uint32_t textWrapping;
  uint32_t textOverflow;
  uint32_t textureHash;
  uint32_t infoHash;
  struct text_info info;
  bool_t placeholder;
  struct Texture *texture;
  struct TextBlockTextRun run[UI_MAX_TEXT_RUNS];
};

enum
{
  UI_TEXT_STYLE_NORMAL,
  UI_TEXT_STYLE_BOLD,
  UI_TEXT_STYLE_ITALIC,
  UI_TEXT_STYLE_BOLD_ITALIC,
  UI_TEXT_STYLE_COUNT,
};

ORCA_API uint32_t TextBlockText_GetHash(struct TextBlockText *text);
ORCA_API struct Texture *TextBlockText_GetTexture(struct TextBlockText *text);
ORCA_API HRESULT TextBlockText_GetInfo(struct TextBlockText *text, struct text_info *info);
ORCA_API void TextBlockText_Release(struct TextBlockText *text);
ORCA_API uint32_t TextBlockText_GetMeasureCount(void);
ORCA_API uint32_t TextBlockText_GetRenderCount(void);
ORCA_API void TextBlockText_ResetStats(void);

#endif /* __TEXTBLOCKTEXT_H__ */
