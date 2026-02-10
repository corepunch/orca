#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_IMAGE_H
#include FT_OUTLINE_H
#include FT_SYSTEM_H
#include FT_TRUETYPE_TABLES_H
#include <string.h>

#include "r_local.h"

#include "builtin/Gudea_Bold_ttf.h"
#include "builtin/Gudea_Regular_ttf.h"

#define RegisterDefaultFont(font)                                              \
  Font_LoadFromMemory((void*)Gudea_Regular_ttf, Gudea_Regular_ttf_len, font);  \
  Font_LoadFromMemory((void*)Gudea_Bold_ttf, Gudea_Bold_ttf_len, font);

#define FT_ERROR(...) Con_Error("FONT: " __VA_ARGS__)
#define FT_SCALE(x) ((x) >> 6)
#define FT_FONTSCALE(font)

#define CARET_WIDTH 2

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

typedef struct _FONTFACE
{
  FT_Face face;
  void* mem;
}* PFONTFACE;

struct font
{
  struct _FONTFACE faces[FS_COUNT];
};

static struct _FONTGLOBALS
{
  struct font* font;
  void* ft;
} fg = { 0 };

static FT_UInt u8_readchar(lpcString_t* text)
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
T_GetFontFace(struct ViewTextRun const* run)
{
  if (run->font) {
    if (run->font->faces[run->fontStyle].face) {
      return run->font->faces[run->fontStyle].face;
    }
    FOR_LOOP(i, FS_COUNT) {
      if (run->font->faces[i].face) {
        return run->font->faces[i].face;
      }
    }
  }
  if (fg.font->faces[run->fontStyle].face) {
    return fg.font->faces[run->fontStyle].face;
  }
  return fg.font->faces[FS_NORMAL].face;
}

HRESULT
Font_Release(struct font* font)
{
  if (!font) {
    return S_OK;
  }
  FOR_LOOP(i, FS_COUNT)
  {
    if (font->faces[i].face) {
      FT_Done_Face(font->faces[i].face);
      free(font->faces[i].mem);
    }
  }
  return S_OK;
}

static HRESULT
Font_LoadFromMemory(void* buffer, int fileSize, struct font* font)
{
  FT_Face face;
  if (!fg.ft)
    return E_NOINTERFACE;

  void* mem = ZeroAlloc(fileSize);
  memcpy(mem, buffer, fileSize);

  // allocate on the stack first in case we fail
  if (FT_New_Memory_Face(fg.ft, mem, fileSize, 0, &face)) {
    FT_ERROR("FreeType2, unable to allocate new face");
    free(mem);
    return E_INVALIDARG;
  }

  FONTSTYLE fs = FS_NORMAL;

  TT_OS2* os2 = (TT_OS2*)FT_Get_Sfnt_Table(face, ft_sfnt_os2);
  TT_Postscript* post = (TT_Postscript*)FT_Get_Sfnt_Table(face, ft_sfnt_post);

  if (os2 && os2->usWeightClass > 400)
    fs += FS_BOLD;
  if (post && post->italicAngle != 0)
    fs += FS_ITALIC;

  font->faces[fs].face = face;
  font->faces[fs].mem = mem;

  return NOERROR;
}

void
FT_Init(void)
{
  if (FT_Init_FreeType((FT_Library*)&fg.ft)) {
    FT_ERROR("Unable to initialize FreeType.");
  }
  //    fg.font = Font_LoadFromMemory((void *)Gudea_Bold_ttf,
  //    Gudea_Bold_ttf_len);
  fg.font = ZeroAlloc(sizeof(struct font));
  RegisterDefaultFont(fg.font);
}

void
FT_Shutdown(void)
{
  Font_Release(fg.font);
  FT_Done_FreeType(fg.ft);
  fg.ft = NULL;
}
bool_t
FT_Load_CharGlyph(FT_Face face, FT_ULong charcode, FT_Int32 load_flags)
{
  FT_UInt gi = FT_Get_Char_Index(face, charcode);
  if (gi == 0) {
    return FALSE;
  }
  FT_Load_Glyph(face, gi, FT_LOAD_DEFAULT);
  return TRUE;
}

static struct WI_Size
T_GetSize(struct ViewText const* text,
          struct rect* rcursor)
{
  struct WI_Size textSize = { 0 };
  FT_UInt textwidth = 0;
  FT_UInt wordwidth = 0;
  FT_UInt cursor = 0;
  FT_UInt prev_glyph_index = 0;
  FT_Pos spaceWidth = 0;
  FT_Pos lineheight = 0;

  //	if (!(text->flags & RF_USE_FONT_HEIGHT)) {
  //		textSize.height = (uint32_t)(text->run.fontSize * text->scale +
  // FT_SCALE(descender));
  //	}

  for (struct ViewTextRun const *run = text->run;
       run - text->run < text->numTextRuns;
       run++)
  {
    FT_Face const face = T_GetFontFace(run);
    if (FT_Set_Pixel_Sizes(face, 0, run->fontSize * text->scale)) {
      return textSize;
    }
    if (FT_Load_CharGlyph(face, ' ', FT_LOAD_DEFAULT)) {
      spaceWidth = (uint32_t)FT_SCALE(face->glyph->metrics.horiAdvance);
    }

    // Reset glyph index when starting a new run to prevent incorrect kerning
    // between glyphs from different fonts
    prev_glyph_index = 0;

    for (lpcString_t str = run->string;; cursor++) {
      FT_Bool const eos = !*str;
      FT_UInt const charcode = *str ? u8_readchar(&str) : ' ';

      lineheight = MAX(lineheight, FT_MulFix(face->height, face->size->metrics.y_scale));

      if (cursor == text->cursor && rcursor) {
        rcursor->x = textwidth + wordwidth;
        rcursor->y = textSize.height - FT_SCALE(lineheight);
        rcursor->width = 0;
        rcursor->height = FT_SCALE(lineheight);
      }
      if (isspace(charcode)) {
        // if (eos) spaceWidth = 0;
        if (textwidth == 0) {
          textwidth += spaceWidth;
          // first word print anyway
        } else if (textwidth + wordwidth + spaceWidth > text->availableWidth * text->scale) {
          textSize.height += /*text->lineSpacing **/ FT_SCALE(lineheight);
          textSize.width = MAX(textSize.width, textwidth);
          textwidth = 0;
          lineheight = FT_MulFix(face->height, face->size->metrics.y_scale);
          prev_glyph_index = 0;
        } else { // if (charcode != '\n') {
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
          textSize.height += /*text->lineSpacing **/ FT_SCALE(lineheight);
        }
      } else if (FT_Load_CharGlyph(face, charcode, FT_LOAD_DEFAULT)) {
        FT_UInt glyph_index = FT_Get_Char_Index(face, charcode);
        FT_Pos advance = FT_SCALE(face->glyph->metrics.horiAdvance);
        
        // Apply kerning adjustment
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
  return (struct WI_Size){textSize.width, textSize.height + (int)FT_SCALE(lineheight)};
}

HRESULT
Text_Print(struct ViewText const* pViewText,
           struct Texture** pTexture,
           bool_t bReuseTexture)
{
  struct WI_Size textSize = T_GetSize(pViewText, NULL);

  if (textSize.width == 0)
    return E_INVALIDARG;
  
  textSize.height /= pViewText->scale;
  textSize.height *= pViewText->scale;
  
  textSize.width /= pViewText->scale;
  textSize.width *= pViewText->scale;
  
  float spaceWidth = 0;
  FT_Int textwidth = 0;
  FT_Int wordwidth = 0;
  FT_Int prevchar = 0;
  FT_Int x = INT_MIN;
  FT_Int y = 0;
  FT_UInt prev_glyph_index = 0;
  FT_Pos lineheight = 0;
  FT_Pos baseline = 0;
  
  // NOTE: Currently using uint8_t for Alpha8 texture format.
  // If RGBA format (uint32_t) is needed in the future, add #ifdef here
  // and adjust memmove/memset calls in the baseline shifting code below.
  byte_t* image_data = ZeroAlloc(textSize.width * textSize.height * sizeof(uint8_t));
  
  if (image_data == NULL)
    return E_OUTOFMEMORY;

  for (struct ViewTextRun const *run = pViewText->run;
       run - pViewText->run < pViewText->numTextRuns;
       run++)
  {
    FT_Face const face = T_GetFontFace(run);
    if (FT_Set_Pixel_Sizes(face, 0, run->fontSize * pViewText->scale)) {
      free(image_data);
      return E_UNEXPECTED;
    }
    FT_Pos const ascender = FT_MulFix(face->ascender, face->size->metrics.y_scale);
    FT_Pos const underline = FT_MulFix(face->underline_position, face->size->metrics.y_scale);
    // FT_Pos const descender = FT_MulFix(face->descender, face->size->metrics.y_scale);
    //	if ((pViewText->flags & RF_USE_FONT_HEIGHT) == 0) {
    //		textSize.height -= (uint32_t)(pViewText->fontSize + FT_SCALE(descender));
    //    textSize.height += FT_SCALE(lineHeight);
    //	}
    if (FT_Load_CharGlyph(face, ' ', FT_LOAD_DEFAULT)) {
      spaceWidth = FT_SCALE(face->glyph->metrics.horiAdvance);
    }
    if (x == INT_MIN) {
      x = -spaceWidth;
    }

    // Reset glyph index when starting a new run to prevent incorrect kerning
    // between glyphs from different fonts
    prev_glyph_index = 0;

    for (lpcString_t str = run->string, print = str, last = str;; last = str) {
      bool_t const eos = !*str;
      uint32_t const charcode = *str ? u8_readchar(&str) : ' ';
      lineheight = MAX(lineheight, FT_MulFix(face->height, face->size->metrics.y_scale));
      FT_Pos new_baseline = FT_SCALE(ascender);
      if (new_baseline > baseline && x > 0) {
        // Baseline has increased, need to move already written pixels down
        // baseline_shift is guaranteed positive due to the condition above
        FT_Pos baseline_shift = new_baseline - baseline;
        
        // Move pixels in the current line down by baseline_shift
        // In the inverted coordinate system: higher inv values = lower on screen
        
        // Calculate the range of rows that might contain data for this line
        // lineheight is in 26.6 fixed-point format, so we need FT_SCALE to convert to pixels
        long max_row = y + FT_SCALE(lineheight);
        long min_row = y;
        
        // Optimize: calculate the actual range to move, considering boundaries
        long first_row_to_move = min_row;
        long last_row_to_move = max_row;
        
        // Adjust range to stay within image bounds
        if (last_row_to_move >= textSize.height) {
          last_row_to_move = textSize.height - 1;
        }
        if (first_row_to_move < 0) {
          first_row_to_move = 0;
        }
        
        // Check if we have valid range and destination
        if (first_row_to_move <= last_row_to_move && 
            last_row_to_move + baseline_shift < textSize.height) {
          
          // Calculate inverted coordinates for the entire block to move
          long first_inv = textSize.height - last_row_to_move - 1;
          long last_inv = textSize.height - first_row_to_move - 1;
          long dest_first_inv = textSize.height - (last_row_to_move + baseline_shift) - 1;
          
          // Move the entire block in one operation (from top to bottom in inverted coords)
          long num_rows = last_inv - first_inv + 1;
          if (num_rows > 0 && first_inv >= 0 && dest_first_inv >= 0 && 
              last_inv < textSize.height && dest_first_inv + num_rows <= textSize.height) {
            memmove(&image_data[dest_first_inv * textSize.width],
                    &image_data[first_inv * textSize.width],
                    num_rows * textSize.width * sizeof(uint8_t));
            
            // Clear the rows that are now empty at the top of the moved region
            for (long row = min_row; row < min_row + baseline_shift && row < textSize.height; row++) {
              if (row < 0) continue;
              long inv = textSize.height - row - 1;
              if (inv >= 0 && inv < textSize.height) {
                memset(&image_data[inv * textSize.width], 0, textSize.width * sizeof(uint8_t));
              }
            }
          }
        }
      }
      baseline = new_baseline;
      if (isspace(charcode)) {
        if (textwidth == 0) { // first word print anyway
          x += spaceWidth;
          textwidth += spaceWidth;
        } else if (textwidth + wordwidth + spaceWidth > pViewText->availableWidth * pViewText->scale) {
          textwidth = 0;
          y += FT_SCALE(lineheight);
          x = 0;
          lineheight = FT_MulFix(face->height, face->size->metrics.y_scale);
          baseline = FT_SCALE(ascender);
          prevchar = 0;
          prev_glyph_index = 0;
        } else { // if (charcode != '\n') {
          x += spaceWidth;
          textwidth += spaceWidth;
        }
        while (print < last) {
          int ch = u8_readchar(&print);
          if (!FT_Load_CharGlyph(face, ch, FT_LOAD_DEFAULT))
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
          
          for (int i = 0; i < (int)bitmap->rows; i++) {
            long row = y + i + y_off;
            for (int j = 0; j < (int)bitmap->width; j++) {
              float p = bitmap->buffer[i * bitmap->pitch + j];
              long column = x + j + x_off;
              if (row >= textSize.height || row < 0)
                continue;
              if (column >= textSize.width || column < 0)
                continue;
              long inv = textSize.height - row - 1;
              image_data[column + inv * textSize.width] = p;
            }
          }
          
          for (long i = 0; i < run->underlineWidth * pViewText->scale; i++) {
            for (long j = prevchar; j < x + x_off + bitmap->width; j++) {
              long row = baseline - FT_SCALE(underline) + i;
              long inv = textSize.height - row - 1;
              image_data[j + inv * textSize.width] = 255;
            }
          }
          prevchar = (int)(x + x_off + bitmap->width);
          x += advance;
        }
        if (eos) {
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
      } else if (FT_Load_CharGlyph(face, charcode, FT_LOAD_DEFAULT)) {
        FT_UInt glyph_index = FT_Get_Char_Index(face, charcode);
        FT_Pos advance = FT_SCALE(face->glyph->metrics.horiAdvance);
        
        // Apply kerning adjustment
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

struct font*
Font_Load(lpcString_t szFileName)
{
  struct file* pFile = FS_LoadFile(szFileName);
  if (pFile) {
    struct font* pFont = ZeroAlloc(sizeof(struct font));
    Font_LoadFromMemory(pFile->data, pFile->size, pFont);
    FS_FreeFile(pFile);
    return pFont;
  } else {
    return NULL;
  }
}

HRESULT
Text_GetInsets(struct ViewText const* text,
               struct edges* edges)
{
  for (struct ViewTextRun const *run = text->run;
       run - text->run < text->numTextRuns;
       run++)
  {
    FT_Face const face = T_GetFontFace(run);
    if (FT_Set_Pixel_Sizes(face, 0, run->fontSize * text->scale) ||
        (text->flags & RF_USE_FONT_HEIGHT))
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

HRESULT
Text_GetInfo(struct ViewText const* pViewText,
             struct text_info* info)
{
  struct WI_Size textSize = T_GetSize(pViewText, &info->cursor);
  info->txWidth = textSize.width / pViewText->scale;
  info->txHeight = textSize.height / pViewText->scale;
  info->cursor.x /= pViewText->scale;
  info->cursor.y /= pViewText->scale;
  info->cursor.width /= pViewText->scale;
  info->cursor.height /= pViewText->scale;
  info->cursor.width += CARET_WIDTH;
  Text_GetInsets(pViewText, &info->txInsets);
  return NOERROR;
}

HANDLER(FontFamily, Start) {
  pFontFamily->font = Font_Load(pFontFamily->Regular);
  return TRUE;
}

HANDLER(FontFamily, Destroy) {
  Font_Release(pFontFamily->font);
  return TRUE;
}
