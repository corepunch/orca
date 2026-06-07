#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <UIKit/UIKit.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_IMAGE_H
#include FT_OUTLINE_H

#define FT_SCALE(x) ((x) >> 6)
#define CARET_WIDTH 2
#define FT_Pixel uint8_t
#define WRAP_FUZZ 1

#define SCALE_CEIL(value, scale) (((value) + MAX(1, (scale)) - 1) / MAX(1, (scale)))

static uint32_t s_text_measure_count;
static uint32_t s_text_render_count;

static FT_UInt const offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL,
                                            0x000E2080UL, 0x03C82080UL,
                                            0xFA082080UL, 0x82082080UL };

static char const trailingBytesForUTF8[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
};

static FT_UInt
u8_readchar(lpcString_t* text)
{
  lpcString_t src = *text;
  FT_UInt ch = 0;
  int const nb = trailingBytesForUTF8[(unsigned char)*src];
  switch (nb) {
      /* these fall through deliberately */
    case 3:
      ch += (unsigned char)*src++;
      ch <<= 6;
    case 2:
      ch += (unsigned char)*src++;
      ch <<= 6;
    case 1:
      ch += (unsigned char)*src++;
      ch <<= 6;
      //        case 0:
    default:
      ch += (unsigned char)*src++;
  }
  ch -= offsetsFromUTF8[nb];
  *text = src;
  return ch;
}

static FT_Face
T_GetFontFace(struct TextBlockTextRun const* run)
{
  if (run->fontFamily) {
    FT_Face face = FontFamily_GetFace(run->fontFamily, run->fontStyle);
    if (face) return face;
  }
  return FontFamily_GetFace(Font_GetDefaultFamily(), run->fontStyle);
}

static bool_t
FT_Load_CharGlyph(FT_Face face, FT_ULong charcode)
{
  FT_UInt gi = FT_Get_Char_Index(face, charcode);
  if (gi == 0) {
    return FALSE;
  }
  FT_Load_Glyph(face, gi, FT_LOAD_DEFAULT);
  return TRUE;
}

static struct AXsize
T_GetSize(struct TextBlockText const* text,
          struct rect* rcursor)
{
  s_text_measure_count++;

  struct AXsize textSize = { 0 };
  FT_Int textwidth = 0;
  FT_Int wordwidth = 0;
  FT_Int cursor = 0;
  FT_Int prev_glyph_index = 0;
  FT_Pos spaceWidth = 0;
  FT_Pos lineheight = 0;

  for (struct TextBlockTextRun const *run = text->run;
       run - text->run < text->numTextRuns;
       run++)
  {
    if (!run->string)
      continue;

    FT_Face const face = T_GetFontFace(run);
    if (!face) {
      return textSize;
    }
    if (FT_Set_Pixel_Sizes(face, 0, run->fontSize * text->scale)) {
      return textSize;
    }
    if (FT_Load_CharGlyph(face, ' ')) {
      spaceWidth = (uint32_t)FT_SCALE(face->glyph->metrics.horiAdvance);
    }
    prev_glyph_index = 0;

    for (lpcString_t str = run->string;; cursor++) {
      if (!strncmp(str, "<u>", 3)) {str+=3;continue;}
      if (!strncmp(str, "</u>", 4)) {str+=4;continue;}
      FT_Bool const eos = !*str;
      FT_UInt const charcode = *str ? u8_readchar(&str) : ' ';

      lineheight = MAX(lineheight, FT_MulFix(face->height, face->size->metrics.y_scale));

      if (cursor == text->cursor && rcursor) {
        rcursor->x = textwidth + wordwidth;
        rcursor->y = textSize.height;
        rcursor->width = 0;
        rcursor->height = FT_SCALE(lineheight);
      }

      if (isspace(charcode)) {
        if (eos && (run+1) - text->run >= text->numTextRuns) spaceWidth = 0;
        if (textwidth == 0) {
            textwidth += spaceWidth;
        } else if (text->availableWidth > 0 && text->textWrapping != TEXT_WRAP_NO_WRAP && textwidth + wordwidth > text->availableWidth * text->scale + WRAP_FUZZ) {
          textSize.height += FT_SCALE(lineheight);
          textSize.width = MAX(textSize.width, textwidth);
          textwidth = 0;
          lineheight = FT_MulFix(face->height, face->size->metrics.y_scale);
          prev_glyph_index = 0;
        } else {
          textwidth += spaceWidth;
        }
        textwidth += wordwidth;
        wordwidth = 0;
        textSize.width = MAX(textSize.width, textwidth);
        if (eos) {
          break;
        } else if (charcode == '\n') {
          textwidth = 0;
          prev_glyph_index = 0;
          textSize.height += FT_SCALE(lineheight);
        }
      } else if (FT_Load_CharGlyph(face, charcode)) {
        FT_UInt glyph_index = FT_Get_Char_Index(face, charcode);
        FT_Pos advance = FT_SCALE(face->glyph->metrics.horiAdvance);

        if (prev_glyph_index && glyph_index) {
          FT_Vector kerning;
          if (FT_Get_Kerning(face, prev_glyph_index, glyph_index,
                             FT_KERNING_DEFAULT, &kerning) == 0) {
            wordwidth += FT_SCALE(kerning.x);
          }
        }

        wordwidth += advance;
        prev_glyph_index = glyph_index;
      }
    }
  }
  struct AXsize result = {MAX(0,(int)textSize.width), textSize.height + (int)FT_SCALE(lineheight)};
  if (text->textOverflow == TEXT_OVERFLOW_ELLIPSIS && text->availableWidth > 0) {
    result.width = MIN(result.width, (int)(text->availableWidth * text->scale));
  }
  return result;
}

static FT_Pos
T_GetEllipsisWidth(FT_Face face)
{
  FT_Pos width = 0;
  for (int i = 0; i < 3; i++) {
    if (FT_Load_CharGlyph(face, '.')) {
      width += FT_SCALE(face->glyph->metrics.horiAdvance);
    }
  }
  return width;
}

static void
write_char(FT_Bitmap *bitmap,
           uint8_t *image_data,
           const struct AXsize *textSize,
           FT_Pos x,
           FT_Pos y)
{
  for (FT_Pos i = 0, row = y; i < bitmap->rows; i++, row++) {
    for (FT_Pos j = 0, column = x; j < (int)bitmap->width; j++, column++) {
      uint8_t p = bitmap->buffer[i * bitmap->pitch + j];
      if (row >= textSize->height || row < 0) continue;
      if (column >= textSize->width || column < 0) continue;
      FT_Pixel *pix = image_data + column + (textSize->height - row - 1) * textSize->width;
      *pix = MAX(*pix, p);
    }
  }
}

static HRESULT
TextBlockText_Print(struct TextBlockText* pViewText,
                    struct Texture** pTexture,
                    bool_t bReuseTexture)
{
  s_text_render_count++;

  struct text_info info;
  TextBlockText_GetInfo(pViewText, &info);
  struct AXsize textSize = {
    .width = info.txWidth * pViewText->scale,
    .height = info.txHeight * pViewText->scale,
  };

  if (textSize.width == 0)
    return E_INVALIDARG;

  textSize.height /= pViewText->scale;
  textSize.height *= pViewText->scale;

  textSize.width /= pViewText->scale;
  textSize.width *= pViewText->scale;

  FT_Int spaceWidth = 0;
  FT_Int textwidth = 0;
  FT_Int wordwidth = 0;
  FT_Int prevchar = 0;
  FT_Int x = 0, y = 0;
  FT_UInt prev_glyph_index = 0;
  FT_Pos lineheight = 0;
  FT_Pos baseline = 0;
  FT_Int ul = 0;

  bool_t bDone = FALSE;
  FT_Int ellipsis_x = 0, ellipsis_y = 0;
  FT_Pos ellipsis_baseline = 0;
  FT_Face ellipsis_face = NULL;

  FT_Pixel* image_data = ZeroAlloc(textSize.width * textSize.height * sizeof(FT_Pixel));

  if (image_data == NULL)
    return E_OUTOFMEMORY;

  for (struct TextBlockTextRun const *run = pViewText->run;
       run - pViewText->run < pViewText->numTextRuns && !bDone;
       run++)
  {
    if (!run->string)
      continue;
    FT_Face const face = T_GetFontFace(run);
    if (FT_Set_Pixel_Sizes(face, 0, run->fontSize * pViewText->scale)) {
      free(image_data);
      return E_UNEXPECTED;
    }
    FT_Pos const ascender = FT_MulFix(face->ascender, face->size->metrics.y_scale);
    FT_Pos const underline = FT_MulFix(face->underline_position, face->size->metrics.y_scale);
    if (FT_Load_CharGlyph(face, ' ')) {
      spaceWidth = (FT_Int)FT_SCALE(face->glyph->metrics.horiAdvance);
    }

    FT_Pos const ellipsisWidth = (pViewText->textOverflow == TEXT_OVERFLOW_ELLIPSIS
                                  && pViewText->availableWidth > 0
                                  && textSize.width >= (FT_Int)(pViewText->availableWidth * pViewText->scale))
                                 ? T_GetEllipsisWidth(face) : 0;
    FT_Pos const cutX = textSize.width - ellipsisWidth;

    prev_glyph_index = 0;
    x -= spaceWidth;

    for (lpcString_t str = run->string, print = str, last = str;; last = str) {
      if (!strncmp(str, "<u>", 3)) {str+=3;continue;}
      if (!strncmp(str, "</u>", 4)) {str+=4;continue;}
      FT_Bool const eos = !*str;
      FT_UInt const charcode = *str ? u8_readchar(&str) : ' ';
      if (x > 0 && FT_SCALE(ascender) > baseline) {
        FT_Pos baseline_shift = FT_SCALE(ascender) - baseline;
        FT_Pos src_inv = textSize.height - y - FT_SCALE(lineheight) - 1;
        FT_Pos dst_inv = src_inv - baseline_shift;
        FT_Pos num_rows = FT_SCALE(lineheight) + 1;
        if (src_inv >= 0 && dst_inv >= 0 && src_inv + num_rows <= textSize.height) {
          memmove(&image_data[dst_inv * textSize.width],
                  &image_data[src_inv * textSize.width],
                  num_rows * textSize.width * sizeof(FT_Pixel));
          memset(&image_data[(dst_inv + num_rows) * textSize.width], 0,
                 baseline_shift * textSize.width * sizeof(FT_Pixel));
        }
      }
      lineheight = MAX(lineheight, FT_MulFix(face->height, face->size->metrics.y_scale));
      baseline = MAX(baseline, FT_SCALE(ascender));
      if (isspace(charcode)) {
        if (textwidth == 0) {
          x += spaceWidth;
          textwidth += spaceWidth;
        } else if (pViewText->availableWidth > 0 && pViewText->textWrapping != TEXT_WRAP_NO_WRAP && textwidth + wordwidth > pViewText->availableWidth * pViewText->scale + WRAP_FUZZ) {
          textwidth = 0;
          y += FT_SCALE(lineheight);
          x = 0;
          lineheight = FT_MulFix(face->height, face->size->metrics.y_scale);
          baseline = FT_SCALE(ascender);
          prevchar = 0;
          prev_glyph_index = 0;
        } else {
          x += spaceWidth;
          textwidth += spaceWidth;
        }
        while (print < last) {
          if (!strncmp(print, "<u>", 3)) {print+=3;ul++;continue;}
          if (!strncmp(print, "</u>", 4)) {print+=4;ul--;continue;}
          int ch = u8_readchar(&print);
          if (!FT_Load_CharGlyph(face, ch))
            continue;
          FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
          FT_Glyph_Metrics* metrics = &face->glyph->metrics;
          FT_Bitmap* bitmap = &face->glyph->bitmap;
          FT_Pos advance = FT_SCALE(metrics->horiAdvance);
          FT_Pos x_off = FT_SCALE(metrics->horiBearingX);
          FT_Pos y_off = baseline - FT_SCALE(metrics->horiBearingY);
          FT_UInt glyph_index = FT_Get_Char_Index(face, ch);
          if (prev_glyph_index && glyph_index) {
            FT_Vector kerning;
            if (FT_Get_Kerning(face, prev_glyph_index, glyph_index, FT_KERNING_DEFAULT, &kerning) == 0) {
              x += FT_SCALE(kerning.x);
            }
          }
          prev_glyph_index = glyph_index;

          if (ellipsisWidth > 0 && x + advance > cutX) {
            ellipsis_x = x;
            ellipsis_y = y;
            ellipsis_baseline = baseline;
            ellipsis_face = face;
            bDone = TRUE;
            break;
          }

          write_char(bitmap, image_data, &textSize, x + x_off, y + y_off);

          for (long i = 0; i < MAX(ul?1:0, run->underlineWidth) * pViewText->scale; i++) {
            for (long j = prevchar; j < x + x_off + bitmap->width; j++) {
              long row = baseline - FT_SCALE(underline) + i + y;
              long inv = textSize.height - row - 1;
              image_data[j + inv * textSize.width] = 255;
            }
          }
          prevchar = (int)(x + x_off + bitmap->width);
          x += advance;
        }
        if (bDone) {
          break;
        } else if (eos) {
          break;
        } else if (charcode == '\n') {
          textwidth = 0;
          y += FT_SCALE(lineheight);
          x = -spaceWidth;
          prevchar = 0;
          prev_glyph_index = 0;
          print = str;
        } else {
          print = str;
          textwidth += wordwidth;
          wordwidth = 0;
        }
      } else if (FT_Load_CharGlyph(face, charcode)) {
        FT_UInt glyph_index = FT_Get_Char_Index(face, charcode);
        FT_Pos advance = FT_SCALE(face->glyph->metrics.horiAdvance);

        if (ellipsisWidth > 0 && x + spaceWidth + wordwidth + advance > cutX) {
          lpcString_t p = print;
          FT_UInt kern_prev = prev_glyph_index;
          x += spaceWidth;
          while (p < last) {
            if (!strncmp(p, "<u>", 3)) { p += 3; continue; }
            if (!strncmp(p, "</u>", 4)) { p += 4; continue; }
            int ch = u8_readchar(&p);
            if (!FT_Load_CharGlyph(face, ch))
              continue;
            FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
            FT_Glyph_Metrics* wmetrics = &face->glyph->metrics;
            FT_Bitmap* wbitmap = &face->glyph->bitmap;
            FT_Pos wadv = FT_SCALE(wmetrics->horiAdvance);
            FT_Pos wx_off = FT_SCALE(wmetrics->horiBearingX);
            FT_Pos wy_off = baseline - FT_SCALE(wmetrics->horiBearingY);
            FT_UInt wgi = FT_Get_Char_Index(face, ch);
            if (kern_prev && wgi) {
              FT_Vector kerning;
              if (FT_Get_Kerning(face, kern_prev, wgi, FT_KERNING_DEFAULT, &kerning) == 0)
                x += FT_SCALE(kerning.x);
            }
            kern_prev = wgi;
            if (x + wadv > cutX) break;
            write_char(wbitmap, image_data, &textSize, x + wx_off, y + wy_off);
            x += wadv;
          }
          ellipsis_x = x;
          ellipsis_y = y;
          ellipsis_baseline = baseline;
          ellipsis_face = face;
          bDone = TRUE;
          break;
        }

        if (prev_glyph_index && glyph_index) {
          FT_Vector kerning;
          if (FT_Get_Kerning(face, prev_glyph_index, glyph_index,
                             FT_KERNING_DEFAULT, &kerning) == 0) {
            wordwidth += FT_SCALE(kerning.x);
          }
        }

        wordwidth += advance;
        prev_glyph_index = glyph_index;
      }
    }
  }

  if (ellipsis_face) {
    FT_Int ex = ellipsis_x;
    for (int i = 0; i < 3; i++) {
      if (FT_Load_CharGlyph(ellipsis_face, '.')) {
        FT_Render_Glyph(ellipsis_face->glyph, FT_RENDER_MODE_NORMAL);
        FT_Glyph_Metrics* m = &ellipsis_face->glyph->metrics;
        write_char(&ellipsis_face->glyph->bitmap, image_data, &textSize,
                   ex + FT_SCALE(m->horiBearingX),
                   ellipsis_y + ellipsis_baseline - FT_SCALE(m->horiBearingY));
        ex += (FT_Int)FT_SCALE(m->horiAdvance);
      }
    }
  }

  if (bReuseTexture && *pTexture) {
    Texture_Reallocate(*pTexture,
                       &(CREATEIMGSTRUCT){ .Width = textSize.width,
                                           .Height = textSize.height,
                                           .Format = kTextureFormatAlpha8,
                                           .MinFilter = kTextureFilterLinear,
                                           .MagFilter = kTextureFilterLinear,
                                           .ImageData = image_data });
    (*pTexture)->Scale = pViewText->scale;
    (*pTexture)->Width = textSize.width / MAX(1, (*pTexture)->Scale);
    (*pTexture)->Height = textSize.height / MAX(1, (*pTexture)->Scale);
    free(image_data);
    return S_OK;
  }

  HRESULT hr = Texture_Create(
    &(CREATEIMGSTRUCT){
      .Width = (int)textSize.width,
      .Height = (int)textSize.height,
      .MinFilter = kTextureFilterLinear,
      .MagFilter = kTextureFilterLinear,
      .Format = kTextureFormatAlpha8,
      .ImageData = image_data,
    },
    pTexture);

  free(image_data);

  if (FAILED(hr)) {
    return hr;
  } else {
    (*pTexture)->Scale = pViewText->scale;
    (*pTexture)->Width = textSize.width / MAX(1, (*pTexture)->Scale);
    (*pTexture)->Height = textSize.height / MAX(1, (*pTexture)->Scale);
    return S_OK;
  }
}

ORCA_API uint32_t
TextBlockText_GetHash(struct TextBlockText *text)
{
  uint32_t size = sizeof(struct TextBlockTextRun) - sizeof(lpcString_t);
  uint32_t text_hash = 0;
  uint32_t format_hash = fnv1a32_range((char*)text, (char*)&text->textureHash);
  for (struct TextBlockTextRun *run = text->run; run - text->run < text->numTextRuns; run++) {
    if (!run->string) {
      continue;
    }
    text_hash ^= fnv1a32(run->string);
    format_hash ^= fnv1a32_range((char*)&run->fontFamily, ((char*)&run->fontFamily) + size);
  }
  return text_hash ^ format_hash;
}

ORCA_API struct Texture *
TextBlockText_GetTexture(struct TextBlockText *text)
{
  uint32_t hash = TextBlockText_GetHash(text);
  if (text->texture && text->textureHash == hash) {
    return text->texture;
  }
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

ORCA_API uint32_t
TextBlockText_GetMeasureCount(void)
{
  return s_text_measure_count;
}

ORCA_API uint32_t
TextBlockText_GetRenderCount(void)
{
  return s_text_render_count;
}

ORCA_API void
TextBlockText_ResetStats(void)
{
  s_text_measure_count = 0;
  s_text_render_count = 0;
}

static HRESULT
TextBlockText_GetInsets(struct TextBlockText const* text,
                        struct edges* edges)
{
  for (struct TextBlockTextRun const *run = text->run;
       run - text->run < text->numTextRuns;
       run++)
  {
    FT_Face const face = T_GetFontFace(run);
    if (!face ||
        FT_Set_Pixel_Sizes(face, 0, run->fontSize * text->scale) ||
        (text->flags & UI_TEXT_USE_FONT_HEIGHT))
    {
      *edges = (struct edges){ 0 };
      return S_OK;
    }
    FT_Pos const ascender = FT_MulFix(face->ascender, face->size->metrics.y_scale);
    FT_Pos const descender = FT_MulFix(face->descender, face->size->metrics.y_scale);
    *edges = (struct edges){
      .left = 0,
      .right = 0,
      .top = ceil(FT_SCALE(ascender - descender) / text->scale - run->fontSize * text->scale),
      .bottom = -ceil(FT_SCALE(descender) / text->scale),
    };
  }
  return S_OK;
}

ORCA_API HRESULT
TextBlockText_GetInfo(struct TextBlockText* pViewText,
                      struct text_info* info)
{
  uint32_t hash = TextBlockText_GetHash(pViewText);
  if (pViewText->infoHash && pViewText->infoHash == hash) {
    *info = pViewText->info;
    return NOERROR;
  }

  struct AXsize textSize = T_GetSize(pViewText, &info->cursor);
  info->txWidth = SCALE_CEIL(textSize.width, pViewText->scale);
  info->txHeight = SCALE_CEIL(textSize.height, pViewText->scale);
  info->cursor.x /= pViewText->scale;
  info->cursor.y /= pViewText->scale;
  info->cursor.width /= pViewText->scale;
  info->cursor.height /= pViewText->scale;
  info->cursor.width += CARET_WIDTH;
  TextBlockText_GetInsets(pViewText, &info->txInsets);
  pViewText->info = *info;
  pViewText->infoHash = hash;
  return NOERROR;
}

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

HANDLER(TextBlockConcept, TextBlockConcept, GetInfo)
{
  struct text_info *info = (struct text_info*)pGetInfo;
  if (!info || !pTextBlockConcept->_text) {
    return FALSE;
  }
  return SUCCEEDED(TextBlockText_GetInfo(pTextBlockConcept->_text, info));
}

HANDLER(TextBlockConcept, TextBlockConcept, GetTexture)
{
  if (!pTextBlockConcept->_text) {
    return 0;
  }
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
