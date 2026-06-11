/*
 * text.c — FreeType2 text layout & rendering for Orca
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

#include <UIKit/UIKit.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_IMAGE_H
#include FT_OUTLINE_H

/* ── Tunables ─────────────────────────────────────────────────── */
#define FT_SCALE(x)     ((x) >> 6)
#define CARET_WIDTH     2
#define FT_Pixel        uint8_t
#define SCALE_CEIL(v,s) (((v) + MAX(1,(s)) - 1) / MAX(1,(s)))

/* ── Stats ────────────────────────────────────────────────────── */
static uint32_t s_text_measure_count;
static uint32_t s_text_render_count;

/* ── UTF-8 decoder (unchanged) ────────────────────────────────── */
static FT_UInt const offsetsFromUTF8[6] = {
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

static FT_UInt
u8_readchar(lpcString_t *text)
{
  lpcString_t src = *text;
  FT_UInt ch = 0;
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
static FT_Face
T_GetFontFace(struct TextBlockTextRun const *run)
{
  if (run->fontFamily) {
    FT_Face face = FontFamily_GetFace(run->fontFamily, run->fontStyle);
    if (face) return face;
  }
  return FontFamily_GetFace(Font_GetDefaultFamily(), run->fontStyle);
}

static bool_t
T_LoadChar(FT_Face face, FT_ULong code)
{
  FT_UInt gi = FT_Get_Char_Index(face, code);
  if (!gi) return FALSE;
  FT_Load_Glyph(face, gi, FT_LOAD_DEFAULT);
  return TRUE;
}

/* ── Per-run setup ────────────────────────────────────────────── */
typedef struct {
  FT_Face  face;
  FT_Pos   ascender;      /* 26.6 fixed, NOT yet shifted */
  FT_Pos   descender;     /* 26.6 fixed, always negative */
  FT_Pos   height;        /* 26.6 fixed */
  FT_Pos   underlinePos;  /* 26.6 fixed */
  FT_Int   spaceAdv;      /* pixels */
} RunMetrics;

static bool_t
T_BeginRun(struct TextBlockTextRun const *run, float scale, RunMetrics *m)
{
  m->face = T_GetFontFace(run);
  if (!m->face) return FALSE;
  if (FT_Set_Pixel_Sizes(m->face, 0, (FT_UInt)(run->fontSize * scale))) return FALSE;
  
  m->ascender    = FT_MulFix(m->face->ascender,         m->face->size->metrics.y_scale);
  m->descender   = FT_MulFix(m->face->descender,        m->face->size->metrics.y_scale);
  m->height      = FT_MulFix(m->face->height,           m->face->size->metrics.y_scale);
  m->underlinePos= FT_MulFix(m->face->underline_position,m->face->size->metrics.y_scale);
  
  m->spaceAdv = T_LoadChar(m->face, ' ')
  ? (FT_Int)FT_SCALE(m->face->glyph->metrics.horiAdvance) : 0;
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
static FT_Pos
T_MeasureWord(FT_Face face, lpcString_t *src, FT_UInt *last_glyph_index_inout)
{
  FT_Pos    width = 0;
  FT_UInt   prev  = *last_glyph_index_inout;
  
  lpcString_t p = *src;
  while (*p) {
    /* skip markup */
    if (!strncmp(p, "<u>", 3))  { p += 3; continue; }
    if (!strncmp(p, "</u>", 4)) { p += 4; continue; }
    
    lpcString_t before = p;
    FT_UInt ch = u8_readchar(&p);
    if (isspace(ch)) { p = before; break; }  /* stop before space */
    
    if (!T_LoadChar(face, ch)) { prev = 0; continue; }
    FT_UInt gi = FT_Get_Char_Index(face, ch);
    
    if (prev && gi) {
      FT_Vector kern;
      if (FT_Get_Kerning(face, prev, gi, FT_KERNING_DEFAULT, &kern) == 0)
        width += FT_SCALE(kern.x);
    }
    width += FT_SCALE(face->glyph->metrics.horiAdvance);
    prev = gi;
  }
  
  *last_glyph_index_inout = prev;
  *src = p;
  return width;
}

/* ── Bitmap helpers ───────────────────────────────────────────── */

static void
T_BlitGlyph(FT_Bitmap const *bm,
            FT_Pixel       *image_data,
            struct AXsize const *sz,
            FT_Pos          x,
            FT_Pos          y)
{
  for (FT_Pos i = 0; i < (FT_Pos)bm->rows; i++) {
    FT_Pos row = y + i;
    if (row < 0 || row >= sz->height) continue;
    for (FT_Pos j = 0; j < (FT_Pos)bm->width; j++) {
      FT_Pos col = x + j;
      if (col < 0 || col >= sz->width) continue;
      uint8_t p = bm->buffer[i * bm->pitch + j];
      FT_Pixel *pix = image_data + col + (sz->height - row - 1) * sz->width;
      *pix = MAX(*pix, p);
    }
  }
}

static void
T_BlitUnderline(FT_Pixel       *image_data,
                struct AXsize const *sz,
                FT_Pos          x0,
                FT_Pos          x1,
                FT_Pos          y_baseline,
                FT_Pos          ul_pos,
                int             thickness)
{
  for (int t = 0; t < thickness; t++) {
    FT_Pos row = y_baseline - ul_pos + t;
    if (row < 0 || row >= sz->height) continue;
    FT_Pos inv = sz->height - row - 1;
    for (FT_Pos x = x0; x < x1 && x < sz->width; x++) {
      if (x >= 0)
        image_data[x + inv * sz->width] = 255;
    }
  }
}

/* Render one word starting at *src into the bitmap.
 * Advances *x by the total pixel advance.
 * Returns x_end (for underline extent tracking). */
static FT_Pos
T_BlitWord(FT_Face          face,
           lpcString_t      word_start,
           lpcString_t      word_end,
           FT_Pixel        *image_data,
           struct AXsize const *sz,
           FT_Pos          *x,
           FT_Pos           y,
           FT_Pos           baseline,
           FT_Pos           ul_pos,
           int              ul_thickness,   /* 0 = no underline */
           FT_UInt         *prev_gi_inout)
{
  FT_Pos x_start = *x;
  lpcString_t p  = word_start;
  FT_UInt prev   = *prev_gi_inout;
  
  while (p < word_end) {
    if (!strncmp(p, "<u>",  3)) { p += 3; continue; }
    if (!strncmp(p, "</u>", 4)) { p += 4; continue; }
    
    lpcString_t before = p;
    FT_UInt ch = u8_readchar(&p);
    if (isspace(ch)) { p = before; break; }
    
    if (!T_LoadChar(face, ch)) { prev = 0; continue; }
    FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
    
    FT_UInt   gi    = FT_Get_Char_Index(face, ch);
    FT_Pos    adv   = FT_SCALE(face->glyph->metrics.horiAdvance);
    FT_Pos    xoff  = FT_SCALE(face->glyph->metrics.horiBearingX);
    FT_Pos    yoff  = baseline - FT_SCALE(face->glyph->metrics.horiBearingY);
    
    if (prev && gi) {
      FT_Vector kern;
      if (FT_Get_Kerning(face, prev, gi, FT_KERNING_DEFAULT, &kern) == 0)
        *x += FT_SCALE(kern.x);
    }
    
    T_BlitGlyph(&face->glyph->bitmap, image_data, sz, *x + xoff, y + yoff);
    *x  += adv;
    prev = gi;
  }
  
  if (ul_thickness > 0) {
    T_BlitUnderline(image_data, sz,
                    x_start, *x,
                    y + baseline, ul_pos, ul_thickness);
  }
  
  *prev_gi_inout = prev;
  return *x;
}

static void
T_BlitEllipsis(FT_Face          face,
               FT_Pixel        *image_data,
               struct AXsize const *sz,
               FT_Pos           x,
               FT_Pos           y,
               FT_Pos           baseline)
{
  for (int i = 0; i < 3; i++) {
    if (!T_LoadChar(face, '.')) continue;
    FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
    FT_Pos xoff = FT_SCALE(face->glyph->metrics.horiBearingX);
    FT_Pos yoff = baseline - FT_SCALE(face->glyph->metrics.horiBearingY);
    T_BlitGlyph(&face->glyph->bitmap, image_data, sz, x + xoff, y + yoff);
    x += FT_SCALE(face->glyph->metrics.horiAdvance);
  }
}

static FT_Pos
T_EllipsisWidth(FT_Face face)
{
  FT_Pos w = 0;
  for (int i = 0; i < 3; i++)
    if (T_LoadChar(face, '.'))
      w += FT_SCALE(face->glyph->metrics.horiAdvance);
  return w;
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
  FT_Pixel      *image_data;
} LayoutCtx;

/* Layout state shared across runs */
typedef struct {
  FT_Pos  lineX;          /* current X pen position on this line */
  FT_Pos  lineY;          /* top of current line */
  FT_Pos  lineWidth;      /* max width reached on current line */
  FT_Pos  lineHeight;     /* max face->height on current line */
  FT_Pos  baseline;       /* max ascender on current line */
  FT_Int  charCount;      /* codepoint counter for cursor tracking */
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
  ls->lineY      += FT_SCALE(ls->lineHeight);
  ls->lineX       = 0;
  ls->lineWidth   = 0;
  ls->lineHeight  = m->height;
  ls->baseline    = FT_SCALE(m->ascender);
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
  FT_Pos const wrapW   = (text->availableWidth > 0 && text->textWrapping != TEXT_WRAP_NO_WRAP)
  ? (FT_Pos)(text->availableWidth * scale)
  : 0;
  
  /* ── First pass: compute pixel dimensions ── */
  /* We always need to know the final size before allocating the bitmap,
   so we do a lightweight pre-pass to get width/height.              */
  struct AXsize measuredSize = { 0 };
  {
    LayoutState ls;
    LS_Init(&ls);
    FT_Int current_char = 0; /* true per-character index, parallel to pInput->Cursor */
    FT_Pos spaceOffset  = 0; /* accumulated space advance since last committed word */

    for (struct TextBlockTextRun const *run = text->run;
         run - text->run < text->numTextRuns; run++)
    {
      if (!run->string) continue;

      RunMetrics m;
      if (!T_BeginRun(run, scale, &m)) return E_UNEXPECTED;

      ls.lineHeight = MAX(ls.lineHeight, m.height);
      ls.baseline   = MAX(ls.baseline,   FT_SCALE(m.ascender));

      lpcString_t p = run->string;
      while (*p) {
        /* skip tags */
        if (!strncmp(p, "<u>", 3))  { p += 3; continue; }
        if (!strncmp(p, "</u>", 4)) { p += 4; continue; }

        FT_UInt ch = (unsigned char)*p;
        lpcString_t before = p;
        ch = u8_readchar(&p);

        ls.charCount++;

        if (ch == '\n') {
          if (!ctx->cursorSet && current_char == text->cursor) {
            ctx->cursor.x      = (int)(ls.lineX / scale);
            ctx->cursor.y      = (int)(ls.lineY / scale);
            ctx->cursor.width  = CARET_WIDTH;
            ctx->cursor.height = (int)(FT_SCALE(m.height) / scale);
            ctx->cursorSet     = TRUE;
          }
          current_char++;
          measuredSize.width = MAX(measuredSize.width, (uint32_t)ls.lineX);
          LS_NewLine(&ls, &m);
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
            ctx->cursor.height = (int)(FT_SCALE(m.height) / scale);
            ctx->cursorSet     = TRUE;
          }
          spaceOffset += m.spaceAdv;
          current_char++;
          continue;
        }

        /* --- Non-space: we have a word token --- */
        /* Back up and measure the whole word */
        lpcString_t word_start = before;
        FT_UInt dummy_gi = 0;
        lpcString_t word_p = word_start;
        FT_Pos wordW = T_MeasureWord(m.face, &word_p, &dummy_gi);
        /* word_p now points past the word */

        /* Decide whether to wrap before this word */
        if (wrapW > 0 && ls.lineHasContent) {
          FT_Pos needed = ls.lineX + m.spaceAdv + wordW;
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
          FT_Pos cx = ls.lineX;
          FT_UInt prev_gi2 = 0;
          lpcString_t wp = word_start;
          while (wp < word_p) {
            if (!strncmp(wp, "<u>",  3)) { wp += 3; continue; }
            if (!strncmp(wp, "</u>", 4)) { wp += 4; continue; }
            if (!ctx->cursorSet && current_char == text->cursor) {
              ctx->cursor.x      = (int)(cx / scale);
              ctx->cursor.y      = (int)(ls.lineY / scale);
              ctx->cursor.width  = CARET_WIDTH;
              ctx->cursor.height = (int)(FT_SCALE(m.height) / scale);
              ctx->cursorSet     = TRUE;
            }
            FT_UInt wch = u8_readchar(&wp);
            current_char++;
            if (!T_LoadChar(m.face, wch)) { prev_gi2 = 0; continue; }
            FT_UInt gi = FT_Get_Char_Index(m.face, wch);
            if (prev_gi2 && gi) {
              FT_Vector kern;
              if (FT_Get_Kerning(m.face, prev_gi2, gi, FT_KERNING_DEFAULT, &kern) == 0)
                cx += FT_SCALE(kern.x);
            }
            cx += FT_SCALE(m.face->glyph->metrics.horiAdvance);
            prev_gi2 = gi;
          }
        }

        ls.lineX += wordW;
        ls.lineHasContent = TRUE;
        ls.lineHeight = MAX(ls.lineHeight, m.height);
        ls.baseline   = MAX(ls.baseline, FT_SCALE(m.ascender));
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
      ctx->cursor.height = (int)(FT_SCALE(ls.lineHeight) / scale);
      ctx->cursorSet     = TRUE;
    }

    measuredSize.width  = MAX(measuredSize.width, (uint32_t)ls.lineX);
    measuredSize.height = (uint32_t)ls.lineY + (uint32_t)FT_SCALE(ls.lineHeight);

    /* Apply availableWidth clamp for ellipsis */
    if (text->textOverflow == TEXT_OVERFLOW_ELLIPSIS && text->availableWidth > 0)
      measuredSize.width = MIN(measuredSize.width, (int)(text->availableWidth * scale));

    ctx->size = measuredSize;
    s_text_measure_count++;
  }
  
  if (!bRender) return S_OK;
  
  /* ── Allocate bitmap ── */
  struct AXsize const sz = {
    .width  = SCALE_CEIL(measuredSize.width,  (int)scale) * (int)scale,
    .height = SCALE_CEIL(measuredSize.height, (int)scale) * (int)scale,
  };
  if (sz.width == 0 || sz.height == 0) return E_INVALIDARG;
  
  FT_Pixel *image_data = (FT_Pixel*)ZeroAlloc(sz.width * sz.height * sizeof(FT_Pixel));
  if (!image_data) return E_OUTOFMEMORY;
  
  /* ── Second pass: render ── */
  s_text_render_count++;
  
  LayoutState ls;
  LS_Init(&ls);
  int  ul_depth     = 0;       /* nesting depth for <u> tags */
  FT_UInt prev_gi   = 0;
  
  bool_t    bDone           = FALSE;
  FT_Pos    ellipsis_x      = 0;
  FT_Pos    ellipsis_y      = 0;
  FT_Pos    ellipsis_base   = 0;
  FT_Face   ellipsis_face   = NULL;
  
  for (struct TextBlockTextRun const *run = text->run;
       run - text->run < text->numTextRuns && !bDone; run++)
  {
    if (!run->string) continue;
    
    RunMetrics m;
    if (!T_BeginRun(run, scale, &m)) { free(image_data); return E_UNEXPECTED; }
    
    ls.lineHeight = MAX(ls.lineHeight, m.height);
    ls.baseline   = MAX(ls.baseline,   FT_SCALE(m.ascender));
    
    /* Ellipsis threshold for this run */
    FT_Pos ellipsisW = 0;
    if (text->textOverflow == TEXT_OVERFLOW_ELLIPSIS && text->availableWidth > 0
        && sz.width >= (FT_Pos)(text->availableWidth * scale))
    {
      ellipsisW = T_EllipsisWidth(m.face);
    }
    
    lpcString_t p = run->string;
    while (*p && !bDone) {
      /* consume markup tags at any position */
      if (!strncmp(p, "<u>",  3)) { p += 3; ul_depth++; continue; }
      if (!strncmp(p, "</u>", 4)) { p += 4; ul_depth--; continue; }
      
      lpcString_t before = p;
      FT_UInt ch = u8_readchar(&p);
      
      if (ch == '\n') {
        ls.lineY  += FT_SCALE(ls.lineHeight);
        ls.lineX   = 0;
        ls.lineHeight = m.height;
        ls.baseline  = FT_SCALE(m.ascender);
        ls.lineHasContent = FALSE;
        prev_gi = 0;
        continue;
      }
      
      if (isspace(ch)) continue;  /* spacing handled at word boundary */
      
      /* ── We have a word ── */
      lpcString_t word_start = before;
      
      /* Measure word without side-effects */
      FT_UInt measure_gi = prev_gi;
      lpcString_t word_end = word_start;
      FT_Pos wordW = T_MeasureWord(m.face, &word_end, &measure_gi);
      
      /* Determine inter-word space */
      FT_Pos spacePrefix = ls.lineHasContent ? m.spaceAdv : 0;
      
      /* Wrap decision */
      if (wrapW > 0 && ls.lineHasContent) {
        if (ls.lineX + spacePrefix + wordW > wrapW) {
          ls.lineY      += FT_SCALE(ls.lineHeight);
          ls.lineX       = 0;
          ls.lineHeight  = m.height;
          ls.baseline    = FT_SCALE(m.ascender);
          ls.lineHasContent = FALSE;
          prev_gi = 0;
          spacePrefix = 0;
        }
      }
      
      /* Ellipsis: would even one char of this word push past the cut? */
      if (ellipsisW > 0) {
        FT_Pos cutX = sz.width - ellipsisW;
        if (ls.lineX + spacePrefix > cutX) {
          /* No room at all — write ellipsis where we are */
          ellipsis_x    = ls.lineX;
          ellipsis_y    = ls.lineY;
          ellipsis_base = ls.baseline;
          ellipsis_face = m.face;
          bDone = TRUE;
          break;
        }
        
        /* Render word characters until we hit the cut */
        int ul_thick = MAX(ul_depth ? 1 : 0, (int)run->underlineWidth) * (int)scale;
        if (ul_thick > 0 && spacePrefix > 0)
          T_BlitUnderline(image_data, &sz,
                          ls.lineX, ls.lineX + spacePrefix,
                          ls.lineY + ls.baseline, FT_SCALE(m.underlinePos), ul_thick);
        ls.lineX += spacePrefix;
        lpcString_t q = word_start;
        while (q < word_end) {
          if (!strncmp(q, "<u>",  3)) { q += 3; continue; }
          if (!strncmp(q, "</u>", 4)) { q += 4; continue; }
          lpcString_t qb = q;
          FT_UInt qch = u8_readchar(&q);
          if (isspace(qch)) break;
          if (!T_LoadChar(m.face, qch)) { prev_gi = 0; continue; }
          FT_Render_Glyph(m.face->glyph, FT_RENDER_MODE_NORMAL);
          FT_UInt  qgi  = FT_Get_Char_Index(m.face, qch);
          FT_Pos   qadv = FT_SCALE(m.face->glyph->metrics.horiAdvance);
          if (prev_gi && qgi) {
            FT_Vector kern;
            if (FT_Get_Kerning(m.face, prev_gi, qgi, FT_KERNING_DEFAULT, &kern) == 0)
              ls.lineX += FT_SCALE(kern.x);
          }
          if (ls.lineX + qadv > cutX) {
            /* This glyph would exceed the cut — stop and place ellipsis */
            ellipsis_x    = ls.lineX;
            ellipsis_y    = ls.lineY;
            ellipsis_base = ls.baseline;
            ellipsis_face = m.face;
            bDone = TRUE;
            break;
          }
          FT_Pos xoff = FT_SCALE(m.face->glyph->metrics.horiBearingX);
          FT_Pos yoff = ls.baseline - FT_SCALE(m.face->glyph->metrics.horiBearingY);
          T_BlitGlyph(&m.face->glyph->bitmap, image_data, &sz,
                      ls.lineX + xoff, ls.lineY + yoff);
          ls.lineX += qadv;
          prev_gi   = qgi;
        }
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
                        ls.lineY + ls.baseline, FT_SCALE(m.underlinePos), ul_thick);
      ls.lineX += spacePrefix;
      T_BlitWord(m.face, word_start, word_end,
                 image_data, &sz,
                 &ls.lineX, ls.lineY,
                 ls.baseline, FT_SCALE(m.underlinePos),
                 ul_thick, &prev_gi);
      
      ls.lineHasContent = TRUE;
      ls.lineHeight = MAX(ls.lineHeight, m.height);
      p = word_end;
    }
  }
  
  /* ── Draw ellipsis if needed ── */
  if (ellipsis_face) {
    T_BlitEllipsis(ellipsis_face, image_data, &sz,
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
    (*pTexture)->Width  = sz.width  / MAX(1, (int)(*pTexture)->Scale);
    (*pTexture)->Height = sz.height / MAX(1, (int)(*pTexture)->Scale);
    free(ctx.image_data);
    return S_OK;
  }
  
  hr = Texture_Create(&cis, pTexture);
  free(ctx.image_data);
  if (FAILED(hr)) return hr;
  
  (*pTexture)->Scale  = pViewText->scale;
  (*pTexture)->Width  = sz.width  / MAX(1, (int)pViewText->scale);
  (*pTexture)->Height = sz.height / MAX(1, (int)pViewText->scale);
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
    FT_Face face = T_GetFontFace(run);
    if (!face
        || FT_Set_Pixel_Sizes(face, 0, (FT_UInt)(run->fontSize * text->scale))
        || (text->flags & UI_TEXT_USE_FONT_HEIGHT))
    {
      *edges = (struct edges){ 0 };
      return S_OK;
    }
    FT_Pos asc  = FT_MulFix(face->ascender,  face->size->metrics.y_scale);
    FT_Pos desc = FT_MulFix(face->descender, face->size->metrics.y_scale);
    *edges = (struct edges){
      .left   = 0,
      .right  = 0,
      .top    = (int)ceil(FT_SCALE(asc - desc) / text->scale
                          - run->fontSize * text->scale),
        .bottom = -(int)ceil(FT_SCALE(desc) / text->scale),
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
  info->txWidth  = SCALE_CEIL(textSize.width,  pViewText->scale);
  info->txHeight = SCALE_CEIL(textSize.height, pViewText->scale);
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
