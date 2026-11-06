#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_ADVANCES_H

#include "r_local.h"
#include <filesystem/filesystem.h>
#include <include/renderer_font.h>

extern struct Object *FS_LoadObject(lpcString_t path);

#define FONT_ERROR(...) Con_Error("FONT: " __VA_ARGS__)

typedef enum
{
  FS_NORMAL,
  FS_BOLD,
  FS_ITALIC,
  FS_BOLD_ITALIC,
  FS_COUNT,
} FontStyle;

static bool_t T_LoadDefaultFontBase(lpcString_t base);
static void FontFamily_ReleaseFaces(struct FontFamily *family);
static struct fontface* Font_LoadStyle(lpcString_t, struct FontFamily*, FontStyle);

struct fontface
{
  FT_Face face;
  byte_t* mem;
};

static struct _FONTGLOBALS
{
  struct Object* defaultFontObject;
  struct FontFamily defaultFontStorage;
  struct FontFamily* defaultFont;
  bool_t defaultFontTried;
  FT_Library ft;
} fg = { 0 };

static struct FontFamily const*
T_GetDefaultFontFamily(void)
{
  if (fg.defaultFont) return fg.defaultFont;

  lpcString_t path = CORE_FindFontFamily("default");
  if (path && T_LoadDefaultFontBase(path)) return fg.defaultFont;
  if (T_LoadDefaultFontBase("build/share/fonts/NotoSans/NotoSans")) return fg.defaultFont;
  if (T_LoadDefaultFontBase("share/fonts/NotoSans/NotoSans")) return fg.defaultFont;

  if (fg.defaultFontTried || !path) return NULL;
  fg.defaultFontTried = TRUE;
  fg.defaultFontObject = FS_LoadObject(path);
  fg.defaultFont = GetFontFamily(fg.defaultFontObject);
  return fg.defaultFont;
}

struct FontFamily const *
Font_GetDefaultFamily(void)
{
  return T_GetDefaultFontFamily();
}

void *
FontFamily_GetFace(struct FontFamily const *family, uint32_t fontStyle)
{
  if (!family) return NULL;
  if ((&family->regular)[fontStyle]) {
    return (&family->regular)[fontStyle];
  }
  FOR_LOOP(i, FS_COUNT) {
    if ((&family->regular)[i]) {
      return (&family->regular)[i];
    }
  }
  return NULL;
}

HRESULT
Font_Release(struct fontface* font)
{
  // Faces die with the library: after FT_Done_FreeType only the memory is left.
  if (font->face && fg.ft) FT_Done_Face(font->face);
  free(font->mem);
  free(font);
  return S_OK;
}

static struct fontface*
Font_LoadFromMemory(void* buffer,
                    int fileSize,
                    struct FontFamily* family,
                    FontStyle style)
{
  if (!fg.ft && FT_Init_FreeType(&fg.ft)) {
    FONT_ERROR("Unable to initialize FreeType.");
    return NULL;
  }

  byte_t* mem = ZeroAlloc(fileSize);
  memcpy(mem, buffer, fileSize);

  struct fontface* fontface = ZeroAlloc(sizeof(*fontface));
  if (FT_New_Memory_Face(fg.ft, mem, fileSize, 0, &fontface->face)) {
    FONT_ERROR("Unable to initialize FreeType font");
    free(fontface);
    free(mem);
    return NULL;
  }

  fontface->mem = mem;
  SafeDelete((&family->regular)[style], Font_Release);
  (&family->regular)[style] = fontface;

  return fontface;
}

// Glyph rasterization sizes the face in pixels: `scale` is pixels per font unit.
static void
Face_SetScale(struct fontface* face, float scale)
{
  FT_Set_Char_Size(face->face, 0,
                   (FT_F26Dot6)lroundf(scale * face->face->units_per_EM * 64.0f),
                   72, 72);
}

int
FontFace_GetMetrics(struct fontface* face,
                    float logical_height,
                    float device_scale,
                    struct font_metrics* metrics)
{
  if (!face || !metrics || logical_height <= 0 || device_scale <= 0) return FALSE;
  FT_Face const f = face->face;
  // FontSize specifies the em, not ascent minus descent.
  float const device_height = logical_height * device_scale;
  metrics->raster_scale = device_height / f->units_per_EM;
  metrics->ascender = (int)ceilf(f->ascender * metrics->raster_scale);
  metrics->descender = (int)floorf(f->descender * metrics->raster_scale);
  metrics->height = metrics->ascender - metrics->descender;
  metrics->underline_position = (int)floorf(f->descender * metrics->raster_scale * 0.5f);
  FT_Fixed space_advance = 0;
  FT_Get_Advance(f, FT_Get_Char_Index(f, ' '), FT_LOAD_NO_SCALE, &space_advance);
  metrics->space_advance = (int32_t)lroundf(space_advance * metrics->raster_scale * 65536.0f);
  return TRUE;
}

uint32_t
FontFace_GetGlyphIndex(struct fontface* face, uint32_t codepoint)
{
  return face ? (uint32_t)FT_Get_Char_Index(face->face, codepoint) : 0;
}

int32_t
FontFace_GetGlyphAdvance(struct fontface* face, uint32_t glyph, float scale)
{
  FT_Fixed advance = 0;
  if (!face || !glyph) return 0;
  if (FT_Get_Advance(face->face, glyph, FT_LOAD_NO_SCALE, &advance)) return 0;
  return (int32_t)lroundf(advance * scale * 65536.0f);
}

int32_t
FontFace_GetGlyphKerning(struct fontface* face,
                         uint32_t left,
                         uint32_t right,
                         float scale)
{
  FT_Vector kern = { 0, 0 };
  if (!face || !left || !right) return 0;
  if (FT_Get_Kerning(face->face, left, right, FT_KERNING_UNSCALED, &kern)) return 0;
  return (int32_t)lroundf(kern.x * scale * 65536.0f);
}

int
FontFace_GetGlyphBounds(struct fontface* face,
                        uint32_t glyph,
                        float scale,
                        struct font_glyph* output)
{
  if (!face || !glyph || !output) return FALSE;
  memset(output, 0, sizeof(*output));
  Face_SetScale(face, scale);
  if (FT_Load_Glyph(face->face, glyph, FT_LOAD_DEFAULT)) return FALSE;
  FT_Glyph image;
  if (FT_Get_Glyph(face->face->glyph, &image)) return FALSE;
  FT_BBox box;
  FT_Glyph_Get_CBox(image, FT_GLYPH_BBOX_PIXELS, &box);
  FT_Done_Glyph(image);
  output->xoff = (int)box.xMin;
  output->yoff = -(int)box.yMax;
  output->width = (int)(box.xMax - box.xMin);
  output->height = (int)(box.yMax - box.yMin);
  return output->width > 0 && output->height > 0;
}

int
FontFace_RenderGlyph(struct fontface* face,
                     uint32_t glyph,
                     float scale,
                     struct font_glyph* output)
{
  if (!face || !glyph || !output) return FALSE;
  memset(output, 0, sizeof(*output));
  Face_SetScale(face, scale);
  if (FT_Load_Glyph(face->face, glyph, FT_LOAD_DEFAULT) ||
      FT_Render_Glyph(face->face->glyph, FT_RENDER_MODE_NORMAL)) {
    return FALSE;
  }
  FT_Bitmap const* bitmap = &face->face->glyph->bitmap;
  if (!bitmap->width || !bitmap->rows || bitmap->pitch < 0) return FALSE;
  output->bitmap = malloc((size_t)bitmap->width * bitmap->rows);
  if (!output->bitmap) return FALSE;
  for (unsigned row = 0; row < bitmap->rows; row++) {
    memcpy(output->bitmap + (size_t)row * bitmap->width,
           bitmap->buffer + (size_t)row * bitmap->pitch,
           bitmap->width);
  }
  output->width = (int)bitmap->width;
  output->height = (int)bitmap->rows;
  output->xoff = face->face->glyph->bitmap_left;
  output->yoff = -face->face->glyph->bitmap_top;
  return TRUE;
}

void
FontFace_FreeGlyph(struct font_glyph* glyph)
{
  if (!glyph) return;
  free(glyph->bitmap);
  glyph->bitmap = NULL;
}

static void
FontFamily_ReleaseFaces(struct FontFamily *family)
{
  SafeDelete(family->regular, Font_Release);
  SafeDelete(family->bold, Font_Release);
  SafeDelete(family->italic, Font_Release);
  SafeDelete(family->bolditalic, Font_Release);
}

static bool_t
T_LoadDefaultFontBase(lpcString_t base)
{
  path_t regular = {0};
  path_t bold = {0};
  path_t italic = {0};
  path_t bolditalic = {0};
  snprintf(regular, sizeof(regular), "%s-Regular.ttf", base);
  if (!FS_FileExists(regular)) return FALSE;

  snprintf(bold, sizeof(bold), "%s-Bold.ttf", base);
  snprintf(italic, sizeof(italic), "%s-Italic.ttf", base);
  snprintf(bolditalic, sizeof(bolditalic), "%s-BoldItalic.ttf", base);

  Font_LoadStyle(regular, &fg.defaultFontStorage, FS_NORMAL);
  if (FS_FileExists(bold)) Font_LoadStyle(bold, &fg.defaultFontStorage, FS_BOLD);
  if (FS_FileExists(italic)) Font_LoadStyle(italic, &fg.defaultFontStorage, FS_ITALIC);
  if (FS_FileExists(bolditalic)) Font_LoadStyle(bolditalic, &fg.defaultFontStorage, FS_BOLD_ITALIC);

  if (fg.defaultFontStorage.regular) {
    fg.defaultFont = &fg.defaultFontStorage;
    return TRUE;
  }
  FontFamily_ReleaseFaces(&fg.defaultFontStorage);
  return FALSE;
}

void
Font_Shutdown(void)
{
  extern void Emoji_Shutdown(void);
  Emoji_Shutdown();
  if (fg.defaultFontObject) {
    OBJ_ReleaseRef(fg.defaultFontObject);
    fg.defaultFontObject = NULL;
  }
  FontFamily_ReleaseFaces(&fg.defaultFontStorage);
  fg.defaultFont = NULL;
  fg.defaultFontTried = FALSE;

  if (fg.ft) {
    FT_Done_FreeType(fg.ft);
    fg.ft = NULL;
  }
}

static struct fontface*
Font_LoadStyle(lpcString_t szFileName,
               struct FontFamily* pFontFamily,
               FontStyle style)
{
  struct file* pFile = FS_LoadFile(szFileName);
  if (pFile) {
    struct fontface *face = Font_LoadFromMemory(pFile->data, pFile->size,
                                                pFontFamily, style);
    FS_FreeFile(pFile);
    if (!face) Con_Error("FONT: Failed to load font face from '%s'", szFileName);
    return face;
  }
  Con_Error("FONT: Failed to load font file '%s'", szFileName);
  return NULL;
}

struct fontface*
Font_Load(lpcString_t szFileName, struct FontFamily* pFontFamily)
{
  return Font_LoadStyle(szFileName, pFontFamily, FS_NORMAL);
}

static lpcString_t
Font_ResolvePath(struct Object *object, lpcString_t path, path_t resolved)
{
  lpcString_t source = OBJ_GetSourceFile(object);
  lpcString_t slash;
  if (!path || !*path) return path;
  if (FS_FileExists(path)) return path;
  if (!source || !*source) return path;
  slash = strrchr(source, '/');
  if (!slash) return path;
  size_t dir_len = (size_t)(slash - source);
  if (dir_len + 1 + strlen(path) >= sizeof(path_t)) return path;
  memcpy(resolved, source, dir_len);
  resolved[dir_len] = '/';
  strcpy(resolved + dir_len + 1, path);
  return FS_FileExists(resolved) ? resolved : path;
}

// FontFamily_Start
HANDLER(FontFamily, Object, Start) {
  if (pFontFamily->Regular && *pFontFamily->Regular) {
    path_t resolved = {0};
    Font_LoadStyle(Font_ResolvePath(hObject, pFontFamily->Regular, resolved), pFontFamily, FS_NORMAL);
  }
  if (pFontFamily->Bold && *pFontFamily->Bold) {
    path_t resolved = {0};
    Font_LoadStyle(Font_ResolvePath(hObject, pFontFamily->Bold, resolved), pFontFamily, FS_BOLD);
  }
  if (pFontFamily->Italic && *pFontFamily->Italic) {
    path_t resolved = {0};
    Font_LoadStyle(Font_ResolvePath(hObject, pFontFamily->Italic, resolved), pFontFamily, FS_ITALIC);
  }
  if (pFontFamily->BoldItalic && *pFontFamily->BoldItalic) {
    path_t resolved = {0};
    Font_LoadStyle(Font_ResolvePath(hObject, pFontFamily->BoldItalic, resolved), pFontFamily, FS_BOLD_ITALIC);
  }
  return TRUE;
}

// FontFamily_Destroy
HANDLER(FontFamily, Object, Destroy) {
  FontFamily_ReleaseFaces(pFontFamily);
  return TRUE;
}
