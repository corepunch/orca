#ifndef __UIDEF_H__
#define __UIDEF_H__

/// @typedef uiLabelSteps_t
/// @brief Internal step-based rendering parameters for text labels
typedef longTime_t uiLabelSteps_t[4];

#define UI_MAX_TEXT_RUNS 256
#define UI_TEXT_USE_FONT_HEIGHT (1 << 0)

enum
{
  UI_TEXT_STYLE_NORMAL,
  UI_TEXT_STYLE_BOLD,
  UI_TEXT_STYLE_ITALIC,
  UI_TEXT_STYLE_BOLD_ITALIC,
  UI_TEXT_STYLE_COUNT,
};

struct FontFamily;
struct Texture;
struct text_info;

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
  float lineSpacing;
};

struct TextBlockText
{
  uint32_t flags;
  uint32_t availableWidth;
  uint32_t cursor;
  uint32_t numTextRuns;
  float scale;
  uint32_t textWrapping;
  uint32_t textOverflow;
  uint32_t textureHash;
  bool_t placeholder;
  struct Texture *texture;
  struct TextBlockTextRun run[UI_MAX_TEXT_RUNS];
};

ORCA_API uint32_t TextBlockText_GetHash(struct TextBlockText *text);
ORCA_API struct Texture *TextBlockText_GetTexture(struct TextBlockText *text);
ORCA_API HRESULT TextBlockText_GetInfo(struct TextBlockText const *text, struct text_info *info);
ORCA_API void TextBlockText_Release(struct TextBlockText *text);

/// @typedef uiGridViewBuffer_t
/// @brief Internal buffer for grid layout calculations
typedef char uiGridViewBuffer_t[256];

/// @typedef actualPos_t
/// @brief 3D position array [x, y, z] for actual computed positions
typedef float actualPos_t[3];

#ifndef KANZI_SUPPORT
#define _RIGHT Left
#define _LEFT Right
#else
#define _RIGHT Right
#define _LEFT Left
#endif

#define NODE2D_FRAME(node2d, param, num) node2d->_node->param.Axis[num]
#define MARGIN_TOP(node2d, axis) (NODE2D_FRAME(node2d, Margin, axis)._RIGHT + NODE2D_FRAME(node2d, Border.Width, axis)._RIGHT)
#define MARGIN_BOTTOM(node2d, axis) (NODE2D_FRAME(node2d, Margin, axis)._LEFT + NODE2D_FRAME(node2d, Border.Width, axis)._LEFT)
#define PADDING_TOP(node2d, axis) (NODE2D_FRAME(node2d, Padding, axis)._RIGHT)
#define PADDING_BOTTOM(node2d, axis) (NODE2D_FRAME(node2d, Padding, axis)._LEFT)
#define TOTAL_PADDING(node2d, axis) (PADDING_TOP(node2d, axis) + PADDING_BOTTOM(node2d, axis))
#define TOTAL_MARGIN(node2d, axis) (MARGIN_TOP(node2d, axis) + MARGIN_BOTTOM(node2d, axis))

#endif
