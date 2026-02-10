#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_IMAGE_H
#include FT_OUTLINE_H
#include FT_SYSTEM_H
#include FT_TRUETYPE_TABLES_H

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
T_GetFontFace(struct view_text const* tf)
{
  if (tf->font) {
    if (tf->font->faces[tf->fontStyle].face) {
      return tf->font->faces[tf->fontStyle].face;
    }
    FOR_LOOP(i, FS_COUNT)
    {
      if (tf->font->faces[i].face) {
        return tf->font->faces[i].face;
      }
    }
  }
  if (fg.font->faces[tf->fontStyle].face) {
    return fg.font->faces[tf->fontStyle].face;
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

static struct view_text*
T_Scale(struct view_text const* input, float scale, struct view_text* output)
{
  memcpy(output, input, sizeof(struct view_text));
  output->fontSize *= scale;
  output->fixedCharacterWidth *= scale;
  output->availableWidth *= scale;
  output->underlineWidth *= scale;
  return output;
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
T_GetSize(FT_Face face, struct view_text const* text, struct rect* rcursor)
{
  FT_Pos lineHeight = FT_MulFix(face->height, face->size->metrics.y_scale);
  struct WI_Size textSize = { 0, (int)FT_SCALE(lineHeight) };
  uint32_t textwidth = 0;
  uint32_t wordwidth = 0;
  FT_Pos spaceWidth = 0;
  uint32_t cursor = 0;
  FT_UInt prev_glyph_index = 0;

  //	if (!(text->flags & RF_USE_FONT_HEIGHT)) {
  //		textSize.height = (uint32_t)(text->fontSize +
  // FT_SCALE(descender));
  //	}

  if (FT_Set_Pixel_Sizes(face, 0, text->fontSize))
    return textSize;

  if (FT_Load_CharGlyph(face, ' ', FT_LOAD_DEFAULT)) {
    spaceWidth = (uint32_t)FT_SCALE(face->glyph->metrics.horiAdvance);
  }

  for (lpcString_t str = text->string;; cursor++) {
    bool_t const eos = !*str;
    uint32_t const charcode = *str ? u8_readchar(&str) : ' ';

    if (cursor == text->cursor && rcursor) {
      rcursor->x = textwidth + wordwidth;
      rcursor->y = textSize.height - FT_SCALE(lineHeight);
      rcursor->width = 0;
      rcursor->height = FT_SCALE(lineHeight);
    }
    if (isspace(charcode)) {
      // if (eos) spaceWidth = 0;
      if (textwidth == 0) {
        // first word print anyway
      } else if (textwidth + wordwidth + spaceWidth > text->availableWidth) {
        textSize.height += /*text->lineSpacing **/ FT_SCALE(lineHeight);
        textSize.width = MAX(textSize.width, textwidth);
        textwidth = 0;
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
        textSize.height += /*text->lineSpacing **/ FT_SCALE(lineHeight);
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
  return textSize;
}

HRESULT
Text_Print(struct view_text const* input, struct Texture** img, bool_t reuse)
{
  struct view_text text;

  FT_Face const face = T_GetFontFace(input);

  T_Scale(input, input->backingScale, &text);

  if (FT_Set_Pixel_Sizes(face, 0, text.fontSize))
    return E_UNEXPECTED;

  FT_Pos const ascender = FT_MulFix(face->ascender, face->size->metrics.y_scale);
  FT_Pos const underline = FT_MulFix(face->underline_position, face->size->metrics.y_scale);
  // FT_Pos const descender = FT_MulFix(face->descender, face->size->metrics.y_scale);
  FT_Pos const lineHeight = FT_MulFix(face->height, face->size->metrics.y_scale);
  FT_Pos const baseline = FT_SCALE(ascender);
  FT_Pos const underline_y = baseline - FT_SCALE(underline); // You can adjust this slightly if needed

  struct WI_Size textSize = T_GetSize(face, &text, NULL);

  if (textSize.width == 0)
    return E_INVALIDARG;

  textSize.height /= input->backingScale;
  textSize.height *= input->backingScale;

  textSize.width /= input->backingScale;
  textSize.width *= input->backingScale;

  //	if ((text.flags & RF_USE_FONT_HEIGHT) == 0) {
  //		textSize.height -= (uint32_t)(text.fontSize +
  // FT_SCALE(descender)); 		textSize.height += FT_SCALE(lineHeight);
  //	}

  byte_t* image_data = ZeroAlloc(textSize.width * textSize.height * sizeof(uint8_t));

  if (image_data == NULL)
    return E_OUTOFMEMORY;

  float spaceWidth = 0;
  if (FT_Load_CharGlyph(face, ' ', FT_LOAD_DEFAULT)) {
    spaceWidth = FT_SCALE(face->glyph->metrics.horiAdvance);
  }

  int textwidth = 0, wordwidth = 0, prevchar = 0, x = 0, y = 0;
  FT_UInt prev_glyph_index = 0;
  for (lpcString_t str = text.string, print = str, last = str;; last = str) {
    bool_t const eos = !*str;
    uint32_t const charcode = *str ? u8_readchar(&str) : ' ';
    if (isspace(charcode)) {
      // first word print anyway
      if (textwidth == 0) {
        x += spaceWidth;
        // first word print anyway
      } else if (textwidth + wordwidth + spaceWidth > text.availableWidth) {
        textwidth = 0;
        y += FT_SCALE(lineHeight);
        x = 0;
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
        
        // Apply kerning adjustment
        FT_UInt glyph_index = FT_Get_Char_Index(face, ch);
        if (prev_glyph_index && glyph_index) {
          FT_Vector kerning;
          if (FT_Get_Kerning(face, prev_glyph_index, glyph_index,
                             FT_KERNING_DEFAULT, &kerning) == 0) {
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
        
        for (long i = 0; i < text.underlineWidth; i++) {
          for (long j = prevchar; j < x + x_off + bitmap->width; j++) {
            long row = underline_y + i;
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
        y += FT_SCALE(lineHeight);
        x = 0;
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

  if (reuse && *img) {
    Texture_Reallocate(*img,
                       &(CREATEIMGSTRUCT){ .Width = textSize.width,
                                           .Height = textSize.height,
                                           .Format = kTextureFormatAlpha8,
                                           .MinFilter = kTextureFilterLinear,
                                           .MagFilter = kTextureFilterLinear,
                                           .ImageData = image_data });
    (*img)->Scale = input->backingScale;
    (*img)->Width = textSize.width / MAX(1, (*img)->Scale);
    (*img)->Height = textSize.height / MAX(1, (*img)->Scale);
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
    img);

  free(image_data);

  if (FAILED(hr)) {
    return hr;
  } else {
    (*img)->Scale = input->backingScale;
    (*img)->Width = textSize.width / MAX(1, (*img)->Scale);
    (*img)->Height = textSize.height / MAX(1, (*img)->Scale);
    return S_OK;
  }
}

struct font*
Font_Load(lpcString_t szFileName)
{
  struct file* pFile=FS_LoadFile(szFileName);
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
Text_GetInsets(struct view_text const* text, struct edges* edges)
{
  if (FT_Set_Pixel_Sizes(T_GetFontFace(text), 0,
                         text->fontSize * text->backingScale) ||
      (text->flags & RF_USE_FONT_HEIGHT)) {
    *edges = (struct edges){ 0 };
    return S_OK;
  }
  FT_Face const face = T_GetFontFace(text);
  FT_Pos const ascender =
    FT_MulFix(face->ascender, face->size->metrics.y_scale);
  FT_Pos const descender =
    FT_MulFix(face->descender, face->size->metrics.y_scale);
  *edges = (struct edges){
    .left = 0,
    .right = 0,
    .top = ceil(FT_SCALE(ascender - descender) / text->backingScale -
                text->fontSize),
    .bottom = -ceil(FT_SCALE(descender) / text->backingScale),
  };
  return S_OK;
}

HRESULT
Text_GetInfo(struct view_text const* input, struct text_info* info)
{
  struct view_text text;
  float scale = 1.0f / (float)input->backingScale;
  FT_Face const face = T_GetFontFace(input);
  T_Scale(input, input->backingScale, &text);
  if (FT_Set_Pixel_Sizes(face, 0, text.fontSize))
    return E_UNEXPECTED;
  struct WI_Size textSize = T_GetSize(face, &text, &info->cursor);
  info->txWidth = textSize.width * scale;
  info->txHeight = textSize.height * scale;
  info->cursor.x *= scale;
  info->cursor.y *= scale;
  info->cursor.width *= scale;
  info->cursor.height *= scale;
  info->cursor.width += CARET_WIDTH;
  Text_GetInsets(&text, &info->txInsets);
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
