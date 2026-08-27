/*
 * text.c — TrueType text layout & rendering for Orca
 *
 * Architecture (SDL_ttf / greedy-wrap style):
 *
 *   T_LayoutText()          — single pass for both measure and render
 *     T_BeginRun()          — set face + pixel size for one TextRun
 *     T_MeasureWord()       — pixel width of one whitespace-delimited token
 *     T_CommitLine()        — flush pending words onto the line
 *     T_BreakLine()         — advance Y, reset X state
 *     T_BlitWord()          — render one word's glyphs into the alpha bitmap
 *     T_BlitEllipsis()      — render trailing "..."
 *     T_BlitUnderline()     — fill underline spans
 *
 * The key insight (lifted from SDL_ttf's greedy wrap):
 *   We collect complete *words* (sequences of non-space chars) before
 *   committing them to the current line.  Only once we know a word fits
 *   (or doesn't) do we decide to wrap.  This eliminates the "slip past
 *   right border" class of bugs that plagued the old interleaved approach.
 *
 * Measure-only mode (bRender == FALSE) skips all bitmap writes and the
 * ZeroAlloc, matching the WinAPI DrawText / DT_CALCRECT pattern.
 */

#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>
#include <include/renderer_font.h>

#include <UIKit/UIKit.h>
#include "TextBlockText.h"

/* ── Tunables ─────────────────────────────────────────────────── */
#define FONT_SCALE_16_16(x) ((x) >> 16)
#define CARET_WIDTH     2
#define FontPixel       uint8_t

static int
T_LogicalPixels(int device_pixels, float device_scale)
{
  float const scale = device_scale > 0 ? device_scale : 1.0f;
  return (int)ceilf(device_pixels / scale);
}

static int
T_AlignedDevicePixels(int device_pixels, float device_scale)
{
  float const scale = device_scale > 0 ? device_scale : 1.0f;
  return (int)ceilf(T_LogicalPixels(device_pixels, scale) * scale);
}

/* ── Stats ────────────────────────────────────────────────────── */
static uint32_t s_text_measure_count;
static uint32_t s_text_render_count;

/* ── UTF-8 decoder (unchanged) ────────────────────────────────── */
static uint32_t const offsetsFromUTF8[6] = {
  0x00000000UL, 0x00003080UL, 0x000E2080UL,
  0x03C82080UL, 0xFA082080UL, 0x82082080UL
};
static char const trailingBytesForUTF8[256] = {
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
  1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3, 4,4,4,4,5,5,5,5
};

static uint32_t
u8_readchar(lpcString_t *text)
{
  lpcString_t src = *text;
  uint32_t ch = 0;
  int const nb = trailingBytesForUTF8[(unsigned char)*src];
  switch (nb) {
    case 3: ch += (unsigned char)*src++; ch <<= 6; /* fall */
    case 2: ch += (unsigned char)*src++; ch <<= 6; /* fall */
    case 1: ch += (unsigned char)*src++; ch <<= 6; /* fall */
    default: ch += (unsigned char)*src++;
  }
  ch -= offsetsFromUTF8[nb];
  *text = src;
  return ch;
}

/* Skip inline markup tags, return TRUE if one was consumed. */
static bool_t
u8_skip_tag(lpcString_t *p, int *ul_depth)
{
  if (!strncmp(*p, "<u>",  3)) { *p += 3; if (ul_depth) (*ul_depth)++; return TRUE; }
  if (!strncmp(*p, "</u>", 4)) { *p += 4; if (ul_depth) (*ul_depth)--; return TRUE; }
  return FALSE;
}

/* ── Face helpers ─────────────────────────────────────────────── */
static struct fontface*
T_GetFontFace(struct TextBlockTextRun const *run)
{
  if (run->fontFamily) {
    struct fontface* face = FontFamily_GetFace(run->fontFamily, run->fontStyle);
    if (face) return face;
  }
  return FontFamily_GetFace(Font_GetDefaultFamily(), run->fontStyle);
}

static uint32_t
T_LoadChar(struct fontface* face, uint32_t code)
{
  return FontFace_GetGlyphIndex(face, code);
}

/* ── Per-run setup ────────────────────────────────────────────── */
typedef struct {
  struct fontface* face;
  float fontScale;
  int ascender;
  int descender;
  int height;
  int underlinePos;
  int spaceAdv;
} RunMetrics;

static bool_t
T_BeginRun(struct TextBlockTextRun const *run, float scale, RunMetrics *m)
{
  m->face = T_GetFontFace(run);
  if (!m->face) return FALSE;
  struct font_metrics metrics;
  if (!FontFace_GetMetrics(m->face, run->fontSize, scale, &metrics)) return FALSE;
  m->fontScale = metrics.raster_scale;
  m->ascender = metrics.ascender;
  m->descender = metrics.descender;
  m->height = metrics.height;
  m->underlinePos = metrics.underline_position;

  /* line-height: if set to a value > 1 it is an absolute pixel height override */
  if (run->lineHeight > 1.0f) {
    int const lh = (int)(run->lineHeight * scale);
    if (lh > m->height) m->height = lh;
  }

  m->spaceAdv = FONT_SCALE_16_16(metrics.space_advance);
  return TRUE;
}

/* ── Word measurement ─────────────────────────────────────────── */

/*
 * Measure one "word" — a run of non-whitespace codepoints starting at *src.
 * On return *src points past the last character of the word.
 * Returns pixel advance width (kerning-aware).
 * The word is bounded by whitespace or end-of-string.
 * Inline markup tags (<u> / </u>) are consumed but not counted.
 */
static int
T_MeasureWord(struct fontface* face,
              float font_scale,
              lpcString_t *src,
              uint32_t *last_glyph_index_inout)
{
  int32_t width16 = 0;
  uint32_t prev = *last_glyph_index_inout;

  lpcString_t p = *src;
  while (*p) {
    /* skip markup */
    if (!strncmp(p, "<u>", 3))  { p += 3; continue; }
    if (!strncmp(p, "</u>", 4)) { p += 4; continue; }

    lpcString_t before = p;
    uint32_t ch = u8_readchar(&p);
    if (isspace(ch)) { p = before; break; }  /* stop before space */

    uint32_t const gi = T_LoadChar(face, ch);
    if (!gi) { prev = 0; continue; }

    width16 += FontFace_GetGlyphKerning(face, prev, gi, font_scale);
    width16 += FontFace_GetGlyphAdvance(face, gi, font_scale);
    prev = gi;
  }

  *last_glyph_index_inout = prev;
  *src = p;
  return FONT_SCALE_16_16(width16);
}

/* ── Bitmap helpers ───────────────────────────────────────────── */

static void
T_BlitGlyph(struct font_glyph const *bm,
            FontPixel      *image_data,
            struct AXsize const *sz,
            int             x,
            int             y)
{
  for (int i = 0; i < bm->height; i++) {
    int const row = y + i;
    if (row < 0 || row >= sz->height) continue;
    for (int j = 0; j < bm->width; j++) {
      int const col = x + j;
      if (col < 0 || col >= sz->width) continue;
      uint8_t const p = bm->bitmap[i * bm->width + j];
      FontPixel *pix = image_data + col + (sz->height - row - 1) * sz->width;
      *pix = MAX(*pix, p);
    }
  }
}

static void
T_BlitUnderline(FontPixel      *image_data,
                struct AXsize const *sz,
                int             x0,
                int             x1,
                int             y_baseline,
                int             ul_pos,
                int             thickness)
{
  for (int t = 0; t < thickness; t++) {
    int const row = y_baseline - ul_pos + t;
    if (row < 0 || row >= sz->height) continue;
    int const inv = sz->height - row - 1;
    for (int x = x0; x < x1 && x < sz->width; x++) {
      if (x >= 0)
        image_data[x + inv * sz->width] = 255;
    }
  }
}

/* Render one word starting at *src into the bitmap.
 * Advances *x by the total pixel advance.
 * Returns x_end (for underline extent tracking). */
static int
T_BlitWord(struct fontface* face,
           float            font_scale,
           lpcString_t      word_start,
           lpcString_t      word_end,
           FontPixel       *image_data,
           struct AXsize const *sz,
           int             *x,
           int              y,
           int              baseline,
           int              ul_pos,
           int              ul_thickness,   /* 0 = no underline */
           uint32_t        *prev_gi_inout)
{
  int const x_start = *x;
  lpcString_t p  = word_start;
  uint32_t prev = *prev_gi_inout;

  int32_t pen16 = (int32_t)(*x) << 16;
  while (p < word_end) {
    if (!strncmp(p, "<u>",  3)) { p += 3; continue; }
    if (!strncmp(p, "</u>", 4)) { p += 4; continue; }

    lpcString_t before = p;
    uint32_t ch = u8_readchar(&p);
    if (isspace(ch)) { p = before; break; }

    uint32_t const gi = T_LoadChar(face, ch);
    if (!gi) { prev = 0; continue; }
    pen16 += FontFace_GetGlyphKerning(face, prev, gi, font_scale);
    struct font_glyph glyph;
    if (FontFace_RenderGlyph(face, gi, font_scale, &glyph)) {
      int const px = FONT_SCALE_16_16(pen16);
      T_BlitGlyph(&glyph, image_data, sz, px + glyph.xoff,
                  y + baseline + glyph.yoff);
      FontFace_FreeGlyph(&glyph);
    }
    pen16 += FontFace_GetGlyphAdvance(face, gi, font_scale);
    prev = gi;
  }
  *x = FONT_SCALE_16_16(pen16);

  if (ul_thickness > 0) {
    T_BlitUnderline(image_data, sz,
                    x_start, *x,
                    y + baseline, ul_pos, ul_thickness);
  }

  *prev_gi_inout = prev;
  return *x;
}

static void
T_BlitEllipsis(struct fontface* face,
               float            font_scale,
               FontPixel       *image_data,
               struct AXsize const *sz,
               int              x,
               int              y,
               int              baseline)
{
  int32_t pen16 = (int32_t)x << 16;
  for (int i = 0; i < 3; i++) {
    uint32_t const glyph_index = T_LoadChar(face, '.');
    if (!glyph_index) continue;
    struct font_glyph glyph;
    if (FontFace_RenderGlyph(face, glyph_index, font_scale, &glyph)) {
      int const px = FONT_SCALE_16_16(pen16);
      T_BlitGlyph(&glyph, image_data, sz, px + glyph.xoff,
                  y + baseline + glyph.yoff);
      FontFace_FreeGlyph(&glyph);
    }
    pen16 += FontFace_GetGlyphAdvance(face, glyph_index, font_scale);
  }
}

static int
T_EllipsisWidth(struct fontface* face, float font_scale)
{
  int32_t width16 = 0;
  uint32_t const glyph = T_LoadChar(face, '.');
  if (glyph) width16 = 3 * FontFace_GetGlyphAdvance(face, glyph, font_scale);
  return FONT_SCALE_16_16(width16);
}

/* ═══════════════════════════════════════════════════════════════
 * T_LayoutText — unified measure + render pass
 *
 * When bRender == FALSE  → pure measurement, no bitmap allocation.
 * When bRender == TRUE   → allocates bitmap, renders glyphs.
 *
 * Greedy word-wrap algorithm (same as SDL_ttf / every browser engine):
 *   For each word token:
 *     if (lineX + spaceIfNeeded + wordWidth <= wrapWidth)
 *         append to current line
 *     else
 *         flush line, start new line with word
 *
 * "Words" are non-space character runs.  Spaces are only accounted for
 * as inter-word gap *after* we know both the previous and next word fit.
 * This gives a clean, correct boundary with zero fuzz hacks.
 * ═══════════════════════════════════════════════════════════════ */

typedef struct {
  /* inputs */
  struct TextBlockText const *text;
  bool_t    bRender;

  /* outputs */
  struct AXsize  size;         /* final pixel size (scaled) */
  struct rect    cursor;       /* cursor rect in unscaled pixel space */
  bool_t         cursorSet;    /* TRUE once cursor rect has been written */

  /* bitmap (only when bRender == TRUE) */
  FontPixel     *image_data;
} LayoutCtx;

/* Layout state shared across runs */
typedef struct {
  int     lineX;          /* current X pen position on this line */
  int     lineY;          /* top of current line */
  int     lineWidth;      /* max width reached on current line */
  int     lineHeight;     /* max face->height on current line */
  int     baseline;       /* max ascender on current line */
  int     charCount;      /* codepoint counter for cursor tracking */
  bool_t  lineHasContent; /* any non-space glyph committed on this line? */
} LayoutState;

static void
LS_Init(LayoutState *ls)
{
  memset(ls, 0, sizeof(*ls));
}

static void
LS_NewLine(LayoutState *ls, RunMetrics const *m)
{
  ls->lineY      += ls->lineHeight;
  ls->lineX       = 0;
  ls->lineWidth   = 0;
  ls->lineHeight  = m->height;
  ls->baseline    = m->ascender;
  ls->lineHasContent = FALSE;
}

/*
 * T_LayoutText: core function.
 * Returns S_OK on success; populates ctx->size, ctx->cursor,
 * and (if bRender) ctx->image_data.
 */
static HRESULT
T_LayoutText(LayoutCtx *ctx)
{
  struct TextBlockText const *text = ctx->text;
  bool_t const bRender = ctx->bRender;
  float  const scale   = text->scale;
  int const wrapW = (text->availableWidth > 0 && text->textWrapping != TEXT_WRAP_NO_WRAP)
  ? (int)(text->availableWidth * scale)
  : 0;

  /* ── First pass: compute pixel dimensions ── */
  /* We always need to know the final size before allocating the bitmap,
   so we do a lightweight pre-pass to get width/height.              */
  struct AXsize measuredSize = { 0 };
  {
    LayoutState ls;
    LS_Init(&ls);
    if (text->textIndent > 0)
      ls.lineX = (int)(text->textIndent * scale);
    int current_char = 0; /* true per-character index, parallel to pInput->Cursor */
    int spaceOffset  = 0; /* accumulated space advance since last committed word */

    for (struct TextBlockTextRun const *run = text->run;
         run - text->run < text->numTextRuns; run++)
    {
      if (!run->string) continue;

      RunMetrics m;
      if (!T_BeginRun(run, scale, &m)) return E_UNEXPECTED;

      ls.lineHeight = MAX(ls.lineHeight, m.height);
      ls.baseline   = MAX(ls.baseline, m.ascender);

      lpcString_t p = run->string;
      while (*p) {
        /* skip tags */
        if (!strncmp(p, "<u>", 3))  { p += 3; continue; }
        if (!strncmp(p, "</u>", 4)) { p += 4; continue; }

        uint32_t ch = (unsigned char)*p;
        lpcString_t before = p;
        ch = u8_readchar(&p);

        ls.charCount++;

        if (ch == '\n') {
          if (!ctx->cursorSet && current_char == text->cursor) {
            ctx->cursor.x      = (int)(ls.lineX / scale);
            ctx->cursor.y      = (int)(ls.lineY / scale);
            ctx->cursor.width  = CARET_WIDTH;
            ctx->cursor.height = (int)(m.height / scale);
            ctx->cursorSet     = TRUE;
          }
          current_char++;
          measuredSize.width = MAX(measuredSize.width, (uint32_t)ls.lineX);
          LS_NewLine(&ls, &m);
          if (text->textIndent > 0)
            ls.lineX = (int)(text->textIndent * scale);
          spaceOffset = 0;
          continue;
        }

        if (isspace(ch)) {
          /* A plain space: just advance, no measurement needed here;
           word tokens already carry their own space prefix below. */
          if (!ctx->cursorSet && current_char == text->cursor) {
            ctx->cursor.x      = (int)((ls.lineX + spaceOffset) / scale);
            ctx->cursor.y      = (int)(ls.lineY / scale);
            ctx->cursor.width  = CARET_WIDTH;
            ctx->cursor.height = (int)(m.height / scale);
            ctx->cursorSet     = TRUE;
          }
          spaceOffset += m.spaceAdv;
          current_char++;
          continue;
        }

        /* --- Non-space: we have a word token --- */
        /* Back up and measure the whole word */
        lpcString_t word_start = before;
        uint32_t dummy_gi = 0;
        lpcString_t word_p = word_start;
        int const wordW = T_MeasureWord(m.face, m.fontScale, &word_p, &dummy_gi);
        /* word_p now points past the word */

        /* Decide whether to wrap before this word */
        if (wrapW > 0 && ls.lineHasContent) {
          int const needed = ls.lineX + m.spaceAdv + wordW;
          if (needed > wrapW) {
            measuredSize.width = MAX(measuredSize.width, (uint32_t)ls.lineX);
            LS_NewLine(&ls, &m);
          } else {
            ls.lineX += m.spaceAdv;
          }
        } else if (ls.lineHasContent) {
          ls.lineX += m.spaceAdv;
        }

        /* cursor: walk the word char-by-char to count and find x if needed */
        {
          int32_t cx16 = (int32_t)ls.lineX << 16;
          uint32_t prev_gi2 = 0;
          lpcString_t wp = word_start;
          while (wp < word_p) {
            if (!strncmp(wp, "<u>",  3)) { wp += 3; continue; }
            if (!strncmp(wp, "</u>", 4)) { wp += 4; continue; }
            if (!ctx->cursorSet && current_char == text->cursor) {
              ctx->cursor.x      = (int)(FONT_SCALE_16_16(cx16) / scale);
              ctx->cursor.y      = (int)(ls.lineY / scale);
              ctx->cursor.width  = CARET_WIDTH;
              ctx->cursor.height = (int)(m.height / scale);
              ctx->cursorSet     = TRUE;
            }
            uint32_t const wch = u8_readchar(&wp);
            current_char++;
            uint32_t const gi = T_LoadChar(m.face, wch);
            if (!gi) { prev_gi2 = 0; continue; }
            cx16 += FontFace_GetGlyphKerning(m.face, prev_gi2, gi, m.fontScale);
            cx16 += FontFace_GetGlyphAdvance(m.face, gi, m.fontScale);
            prev_gi2 = gi;
          }
        }

        ls.lineX += wordW;
        ls.lineHasContent = TRUE;
        ls.lineHeight = MAX(ls.lineHeight, m.height);
        ls.baseline   = MAX(ls.baseline, m.ascender);
        spaceOffset = 0;

        /* skip past the word we just measured */
        p = word_p;
      }
    }

    /* cursor at end-of-text */
    if (!ctx->cursorSet && current_char == text->cursor) {
      ctx->cursor.x      = (int)((ls.lineX + spaceOffset) / scale);
      ctx->cursor.y      = (int)(ls.lineY / scale);
      ctx->cursor.width  = CARET_WIDTH;
      ctx->cursor.height = (int)(ls.lineHeight / scale);
      ctx->cursorSet     = TRUE;
    }

    measuredSize.width  = MAX(measuredSize.width, (uint32_t)ls.lineX);
    measuredSize.height = (uint32_t)ls.lineY + (uint32_t)ls.lineHeight;

    /* Apply availableWidth clamp for ellipsis */
    if (text->textOverflow == TEXT_OVERFLOW_ELLIPSIS && text->availableWidth > 0)
      measuredSize.width = MIN(measuredSize.width, (int)(text->availableWidth * scale));

    ctx->size = measuredSize;
    s_text_measure_count++;
  }

  if (!bRender) return S_OK;

  /* ── Allocate bitmap ── */
  struct AXsize const sz = {
    .width  = T_AlignedDevicePixels(measuredSize.width, scale),
    .height = T_AlignedDevicePixels(measuredSize.height, scale),
  };
  if (sz.width == 0 || sz.height == 0) return E_INVALIDARG;

  FontPixel *image_data = (FontPixel*)ZeroAlloc(sz.width * sz.height * sizeof(FontPixel));
  if (!image_data) return E_OUTOFMEMORY;

  /* ── Second pass: render ── */
  s_text_render_count++;

  LayoutState ls;
  LS_Init(&ls);
  if (text->textIndent > 0)
    ls.lineX = (int)(text->textIndent * scale);
  int  ul_depth     = 0;       /* nesting depth for <u> tags */
  uint32_t prev_gi  = 0;

  bool_t    bDone           = FALSE;
  int       ellipsis_x      = 0;
  int       ellipsis_y      = 0;
  int       ellipsis_base   = 0;
  float     ellipsis_scale  = 0;
  struct fontface* ellipsis_face = NULL;

  for (struct TextBlockTextRun const *run = text->run;
       run - text->run < text->numTextRuns && !bDone; run++)
  {
    if (!run->string) continue;

    RunMetrics m;
    if (!T_BeginRun(run, scale, &m)) { free(image_data); return E_UNEXPECTED; }

    ls.lineHeight = MAX(ls.lineHeight, m.height);
    ls.baseline   = MAX(ls.baseline, m.ascender);

    /* Ellipsis threshold for this run */
    int ellipsisW = 0;
    if (text->textOverflow == TEXT_OVERFLOW_ELLIPSIS && text->availableWidth > 0
        && sz.width >= (int)(text->availableWidth * scale))
    {
      ellipsisW = T_EllipsisWidth(m.face, m.fontScale);
    }

    lpcString_t p = run->string;
    while (*p && !bDone) {
      /* consume markup tags at any position */
      if (!strncmp(p, "<u>",  3)) { p += 3; ul_depth++; continue; }
      if (!strncmp(p, "</u>", 4)) { p += 4; ul_depth--; continue; }

      lpcString_t before = p;
      uint32_t const ch = u8_readchar(&p);

      if (ch == '\n') {
        ls.lineY  += ls.lineHeight;
        ls.lineX   = text->textIndent > 0 ? (int)(text->textIndent * scale) : 0;
        ls.lineHeight = m.height;
        ls.baseline  = m.ascender;
        ls.lineHasContent = FALSE;
        prev_gi = 0;
        continue;
      }

      if (isspace(ch)) continue;  /* spacing handled at word boundary */

      /* ── We have a word ── */
      lpcString_t word_start = before;

      /* Measure word without side-effects */
      uint32_t measure_gi = prev_gi;
      lpcString_t word_end = word_start;
      int const wordW = T_MeasureWord(m.face, m.fontScale, &word_end, &measure_gi);

      /* Determine inter-word space */
      int spacePrefix = ls.lineHasContent ? m.spaceAdv : 0;

      /* Wrap decision */
      if (wrapW > 0 && ls.lineHasContent) {
        if (ls.lineX + spacePrefix + wordW > wrapW) {
          ls.lineY      += ls.lineHeight;
          ls.lineX       = 0;
          ls.lineHeight  = m.height;
          ls.baseline    = m.ascender;
          ls.lineHasContent = FALSE;
          prev_gi = 0;
          spacePrefix = 0;
        }
      }

      /* Ellipsis: would even one char of this word push past the cut? */
      if (ellipsisW > 0) {
        int const cutX = sz.width - ellipsisW;
        if (ls.lineX + spacePrefix > cutX) {
          /* No room at all — write ellipsis where we are */
          ellipsis_x    = ls.lineX;
          ellipsis_y    = ls.lineY;
          ellipsis_base = ls.baseline;
          ellipsis_face = m.face;
          ellipsis_scale = m.fontScale;
          bDone = TRUE;
          break;
        }

        /* Render word characters until we hit the cut */
        int ul_thick = MAX(ul_depth ? 1 : 0, (int)run->underlineWidth) * (int)scale;
        if (ul_thick > 0 && spacePrefix > 0)
          T_BlitUnderline(image_data, &sz,
                          ls.lineX, ls.lineX + spacePrefix,
                          ls.lineY + ls.baseline, m.underlinePos, ul_thick);
        ls.lineX += spacePrefix;
        int32_t epen16 = (int32_t)ls.lineX << 16;
        lpcString_t q = word_start;
        while (q < word_end) {
          if (!strncmp(q, "<u>",  3)) { q += 3; continue; }
          if (!strncmp(q, "</u>", 4)) { q += 4; continue; }
          lpcString_t qb = q;
          uint32_t const qch = u8_readchar(&q);
          if (isspace(qch)) break;
          uint32_t const qgi = T_LoadChar(m.face, qch);
          if (!qgi) { prev_gi = 0; continue; }
          epen16 += FontFace_GetGlyphKerning(m.face, prev_gi, qgi, m.fontScale);
          int const px = FONT_SCALE_16_16(epen16);
          int32_t const nextPen = epen16 + FontFace_GetGlyphAdvance(m.face, qgi, m.fontScale);
          if (FONT_SCALE_16_16(nextPen) > cutX) {
            ellipsis_x    = px;
            ellipsis_y    = ls.lineY;
            ellipsis_base = ls.baseline;
            ellipsis_face = m.face;
            ellipsis_scale = m.fontScale;
            bDone = TRUE;
            break;
          }
          struct font_glyph glyph;
          if (FontFace_RenderGlyph(m.face, qgi, m.fontScale, &glyph)) {
            T_BlitGlyph(&glyph, image_data, &sz,
                        px + glyph.xoff, ls.lineY + ls.baseline + glyph.yoff);
            FontFace_FreeGlyph(&glyph);
          }
          epen16 = nextPen;
          prev_gi   = qgi;
        }
        ls.lineX = FONT_SCALE_16_16(epen16);
        if (!bDone) {
          /* full word fit under cut — finish normally below */
        }
        p = word_end;
        ls.lineHasContent = TRUE;
        continue;
      }

      /* ── Normal render: blit the whole word ── */
      int ul_thick = MAX(ul_depth ? 1 : 0, (int)run->underlineWidth) * (int)scale;
      if (ul_thick > 0 && spacePrefix > 0)
        T_BlitUnderline(image_data, &sz,
                        ls.lineX, ls.lineX + spacePrefix,
                        ls.lineY + ls.baseline, m.underlinePos, ul_thick);
      ls.lineX += spacePrefix;
      T_BlitWord(m.face, m.fontScale, word_start, word_end,
                 image_data, &sz,
                 &ls.lineX, ls.lineY,
                 ls.baseline, m.underlinePos,
                 ul_thick, &prev_gi);

      ls.lineHasContent = TRUE;
      ls.lineHeight = MAX(ls.lineHeight, m.height);
      p = word_end;
    }
  }

  /* ── Draw ellipsis if needed ── */
  if (ellipsis_face) {
    T_BlitEllipsis(ellipsis_face, ellipsis_scale, image_data, &sz,
                   ellipsis_x, ellipsis_y, ellipsis_base);
  }

  ctx->image_data = image_data;
  ctx->size       = (struct AXsize){ sz.width, sz.height };
  return S_OK;
}

/* ═══════════════════════════════════════════════════════════════
 * Public API — thin wrappers around T_LayoutText
 * ═══════════════════════════════════════════════════════════════ */

static struct AXsize
T_GetSize(struct TextBlockText const *text, struct rect *rcursor)
{
  LayoutCtx ctx = { .text = text, .bRender = FALSE };
  if (FAILED(T_LayoutText(&ctx)))
    return (struct AXsize){ 0 };

  if (rcursor && ctx.cursorSet)
    *rcursor = ctx.cursor;

  return ctx.size;
}

static HRESULT
TextBlockText_Print(struct TextBlockText *pViewText,
                    struct Texture      **pTexture,
                    bool_t               bReuseTexture)
{
  LayoutCtx ctx = { .text = pViewText, .bRender = TRUE };
  HRESULT hr = T_LayoutText(&ctx);
  if (FAILED(hr)) return hr;

  struct AXsize const sz = ctx.size;

  CREATEIMGSTRUCT cis = {
    .Width     = sz.width,
    .Height    = sz.height,
    .Format    = kTextureFormatAlpha8,
    .MinFilter = kTextureFilterLinear,
    .MagFilter = kTextureFilterLinear,
    .ImageData = ctx.image_data,
  };

  if (bReuseTexture && *pTexture) {
    Texture_Reallocate(*pTexture, &cis);
    (*pTexture)->Scale  = pViewText->scale;
    (*pTexture)->Width  = T_LogicalPixels(sz.width, pViewText->scale);
    (*pTexture)->Height = T_LogicalPixels(sz.height, pViewText->scale);
    free(ctx.image_data);
    return S_OK;
  }

  hr = Texture_Create(&cis, pTexture);
  free(ctx.image_data);
  if (FAILED(hr)) return hr;

  (*pTexture)->Scale  = pViewText->scale;
  (*pTexture)->Width  = T_LogicalPixels(sz.width, pViewText->scale);
  (*pTexture)->Height = T_LogicalPixels(sz.height, pViewText->scale);
  return S_OK;
}

/* ═══════════════════════════════════════════════════════════════
 * Hash, cache, and release helpers (unchanged logic, cleaner form)
 * ═══════════════════════════════════════════════════════════════ */

ORCA_API uint32_t
TextBlockText_GetHash(struct TextBlockText *text)
{
  uint32_t format_hash = fnv1a32_range((char*)text, (char*)&text->textureHash);
  uint32_t text_hash   = 0;
  size_t   run_fmt_sz  = sizeof(struct TextBlockTextRun) - sizeof(lpcString_t);

  for (struct TextBlockTextRun *run = text->run;
       run - text->run < text->numTextRuns; run++)
  {
    if (!run->string) continue;
    text_hash   ^= fnv1a32(run->string);
    format_hash ^= fnv1a32_range((char*)&run->fontFamily,
                                 (char*)&run->fontFamily + run_fmt_sz);
  }
  return text_hash ^ format_hash;
}

ORCA_API struct Texture *
TextBlockText_GetTexture(struct TextBlockText *text)
{
  uint32_t hash = TextBlockText_GetHash(text);
  if (text->texture && text->textureHash == hash)
    return text->texture;

  if (FAILED(TextBlockText_Print(text, &text->texture, text->texture != NULL))) {
    SafeDelete(text->texture, Texture_Release);
    text->textureHash = 0;
    return NULL;
  }
  text->textureHash = hash;
  return text->texture;
}

ORCA_API void
TextBlockText_Release(struct TextBlockText *text)
{
  if (!text) return;
  SafeDelete(text->texture, Texture_Release);
  free(text);
}

/* ── Stats ────────────────────────────────────────────────────── */

ORCA_API uint32_t TextBlockText_GetMeasureCount(void) { return s_text_measure_count; }
ORCA_API uint32_t TextBlockText_GetRenderCount(void)  { return s_text_render_count;  }
ORCA_API void     TextBlockText_ResetStats(void)       { s_text_measure_count = s_text_render_count = 0; }

/* ── Insets ───────────────────────────────────────────────────── */

static HRESULT
TextBlockText_GetInsets(struct TextBlockText const *text,
                        struct edges               *edges)
{
  for (struct TextBlockTextRun const *run = text->run;
       run - text->run < text->numTextRuns; run++)
  {
    struct fontface* face = T_GetFontFace(run);
    struct font_metrics metrics;
    if (!face
      || !FontFace_GetMetrics(face, run->fontSize, text->scale, &metrics)
      || (text->flags & UI_TEXT_USE_FONT_HEIGHT))
    {
      *edges = (struct edges){ 0 };
      return S_OK;
    }
    *edges = (struct edges){
      .left   = 0,
      .right  = 0,
      .top    = (int)ceil(metrics.height / text->scale - run->fontSize),
        .bottom = -(int)ceil(metrics.descender / text->scale),
    };
  }
  return S_OK;
}

/* ── GetInfo ─────────────────────────────────────────────────── */

ORCA_API HRESULT
TextBlockText_GetInfo(struct TextBlockText *pViewText,
                      struct text_info    *info)
{
  uint32_t hash = TextBlockText_GetHash(pViewText);
  if (pViewText->infoHash && pViewText->infoHash == hash) {
    *info = pViewText->info;
    return NOERROR;
  }

  struct AXsize textSize = T_GetSize(pViewText, &info->cursor);
  info->txWidth  = T_LogicalPixels(textSize.width, pViewText->scale);
  info->txHeight = T_LogicalPixels(textSize.height, pViewText->scale);
  /* cursor rect is already in unscaled logical pixels from T_LayoutText */
  TextBlockText_GetInsets(pViewText, &info->txInsets);

  pViewText->info     = *info;
  pViewText->infoHash = hash;
  return NOERROR;
}

/* ═══════════════════════════════════════════════════════════════
 * Object / concept glue (unchanged)
 * ═══════════════════════════════════════════════════════════════ */

static lpcString_t
_GetTextBlockText(struct Object           *hObject,
                  struct TextBlockConcept *pTextBlockConcept,
                  struct TextRun          *pTextRun)
{
  struct Property *hProp = TextRun_GetProperty(hObject, kTextRunText);
  if (pTextRun->Text && *pTextRun->Text)
    return pTextRun->Text;
  if (pTextBlockConcept->TextResourceID && *pTextBlockConcept->TextResourceID
      && !PROP_HasProgram(hProp))
    return Loc_GetString(pTextBlockConcept->TextResourceID, LOC_TEXT);
  if (OBJ_GetTextContent(hObject) && *OBJ_GetTextContent(hObject))
    return OBJ_GetTextContent(hObject);
  return pTextBlockConcept->PlaceholderText;
}

static struct TextBlockTextRun
_MakeTextBlockTextRun(struct Object  *hObject,
                      struct TextRun  text,
                      lpcString_t     szText)
{
  struct FontShorthand font = { .Size = DEFAULT_FONT_SIZE };
  TextRun_ReadProperty(hObject, Font.Weight, &font.Weight);
  TextRun_ReadProperty(hObject, Font.Style,  &font.Style);
  TextRun_ReadProperty(hObject, Font.Size,   &font.Size);
  TextRun_ReadProperty(hObject, Font.Family, &font.Family);

  struct TextBlockTextRun view = {
    .string             = szText,
    .fontFamily         = font.Family,
    .fontSize           = font.Size,
    .letterSpacing      = text.LetterSpacing,
    .lineHeight         = text.LineHeight,
    .fixedCharacterWidth= text.FixedCharacterWidth,
    .underlineWidth     = text.TextDecoration.Type == kTextDecorationUnderline ? MAX(1, text.TextDecoration.Width) : 0,
    .underlineOffset    = text.TextDecoration.Offset,
    .fontStyle          = 0,
  };
  if (font.Weight == kFontWeightBold)  view.fontStyle += UI_TEXT_STYLE_BOLD;
  if (font.Style  == kFontStyleItalic) view.fontStyle += UI_TEXT_STYLE_ITALIC;
  return view;
}

HANDLER(TextBlockConcept, TextBlockConcept, MakeText)
{
  struct TextRun *pTextRun = GetTextRun(hObject);
  struct TextBlockText *pViewText = pTextBlockConcept->_text;

  pViewText->run[0] = _MakeTextBlockTextRun(
                                            hObject, *pTextRun,
                                            _GetTextBlockText(hObject, pTextBlockConcept, pTextRun));
  pViewText->placeholder  = (pTextBlockConcept->PlaceholderText == pViewText->run[0].string);
  pViewText->numTextRuns  = 1;

  FOR_EACH_OBJECT(run, hObject) {
    struct TextRun *tr = GetTextRun(run);
    if (tr && pViewText->numTextRuns < UI_MAX_TEXT_RUNS) {
      lpcString_t str = (tr->Text && *tr->Text) ? tr->Text : OBJ_GetTextContent(run);
      struct TextRun base = *pTextRun;
      if (tr->Font.Weight)  base.Font.Weight = tr->Font.Weight;
      if (tr->Font.Style)   base.Font.Style  = tr->Font.Style;
      if (tr->Font.Size)    base.Font.Size   = tr->Font.Size;
      if (tr->Font.Family)  base.Font.Family = tr->Font.Family;
      if (TextRun_GetProperty(run, kTextRunTextDecorationType))  base.TextDecoration.Type   = tr->TextDecoration.Type;
      if (TextRun_GetProperty(run, kTextRunTextDecorationWidth)) base.TextDecoration.Width  = tr->TextDecoration.Width;
      if (TextRun_GetProperty(run, kTextRunTextDecorationOffset))base.TextDecoration.Offset = tr->TextDecoration.Offset;
      if (TextRun_GetProperty(run, kTextRunTextDecorationColor)) base.TextDecoration.Color  = tr->TextDecoration.Color;
      if (TextRun_GetProperty(run, kTextRunLetterSpacing))       base.LetterSpacing         = tr->LetterSpacing;
      if (TextRun_GetProperty(run, kTextRunLineHeight))          base.LineHeight            = tr->LineHeight;
      if (TextRun_GetProperty(run, kTextRunCharacterSpacing))    base.CharacterSpacing      = tr->CharacterSpacing;
      if (TextRun_GetProperty(run, kTextRunFixedCharacterWidth)) base.FixedCharacterWidth   = tr->FixedCharacterWidth;
      if (TextRun_GetProperty(run, kTextRunRemoveSideBearingsProperty)) base.RemoveSideBearingsProperty = tr->RemoveSideBearingsProperty;
      pViewText->run[pViewText->numTextRuns++] = _MakeTextBlockTextRun(run, base, str);
    }
  }

  pViewText->flags          = pTextBlockConcept->UseFullFontHeight ? UI_TEXT_USE_FONT_HEIGHT : 0;
  pViewText->availableWidth = pMakeText->availableSpace;
  pViewText->textWrapping   = (uint32_t)pTextBlockConcept->TextWrapping;
  pViewText->textOverflow   = (uint32_t)pTextBlockConcept->TextOverflow;
  pViewText->textIndent     = pTextBlockConcept->TextIndent;
  pViewText->scale          = axGetScaling();
  return TRUE;
}

HANDLER(TextBlockConcept, TextBlockConcept, GetInfo)
{
  struct text_info *info = (struct text_info*)pGetInfo;
  if (!info || !pTextBlockConcept->_text) return FALSE;
  return SUCCEEDED(TextBlockText_GetInfo(pTextBlockConcept->_text, info));
}

HANDLER(TextBlockConcept, TextBlockConcept, GetTexture)
{
  if (!pTextBlockConcept->_text) return 0;
  return (intptr_t)TextBlockText_GetTexture(pTextBlockConcept->_text);
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
