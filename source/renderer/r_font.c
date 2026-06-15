#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_IMAGE_H
#include FT_OUTLINE_H
#include FT_SYSTEM_H
#include FT_TRUETYPE_TABLES_H

#include "r_local.h"
#include <filesystem/filesystem.h>

extern struct Object *FS_LoadObject(lpcString_t path);

#define FT_ERROR(...) Con_Error("FONT: " __VA_ARGS__)

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

struct fontface
{
  FT_Face face;
  void* mem;
};

static struct _FONTGLOBALS
{
  struct Object* defaultFontObject;
  struct FontFamily defaultFontStorage;
  struct FontFamily* defaultFont;
  bool_t defaultFontTried;
  void* ft;
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
    return (&family->regular)[fontStyle]->face;
  }
  FOR_LOOP(i, FS_COUNT) {
    if ((&family->regular)[i]) {
      return (&family->regular)[i]->face;
    }
  }
  return NULL;
}

HRESULT
Font_Release(struct fontface* font)
{
  FT_Done_Face(font->face);
  free(font->mem);
  free(font);
  return S_OK;
}

static struct fontface*
Font_LoadFromMemory(void* buffer, int fileSize, struct FontFamily* family)
{
  FT_Face face;
  if (!fg.ft)
    return NULL;

  void* mem = ZeroAlloc(fileSize);
  memcpy(mem, buffer, fileSize);

  // allocate on the stack first in case we fail
  if (FT_New_Memory_Face(fg.ft, mem, fileSize, 0, &face)) {
    FT_ERROR("FreeType2, unable to allocate new face");
    free(mem);
    return NULL;
  }

  FontStyle fs = FS_NORMAL;

  TT_OS2* os2 = (TT_OS2*)FT_Get_Sfnt_Table(face, ft_sfnt_os2);
  TT_Postscript* post = (TT_Postscript*)FT_Get_Sfnt_Table(face, ft_sfnt_post);

  if (os2 && os2->usWeightClass > 400)
    fs += FS_BOLD;
  if (post && post->italicAngle != 0)
    fs += FS_ITALIC;

  struct fontface *fontface = malloc(sizeof(struct fontface));
  fontface->face = face;
  fontface->mem = mem;

  (&family->regular)[fs] = fontface;

  return fontface;
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

  Font_Load(regular, &fg.defaultFontStorage);
  if (FS_FileExists(bold)) Font_Load(bold, &fg.defaultFontStorage);
  if (FS_FileExists(italic)) Font_Load(italic, &fg.defaultFontStorage);
  if (FS_FileExists(bolditalic)) Font_Load(bolditalic, &fg.defaultFontStorage);

  if (fg.defaultFontStorage.regular) {
    fg.defaultFont = &fg.defaultFontStorage;
    return TRUE;
  }
  FontFamily_ReleaseFaces(&fg.defaultFontStorage);
  return FALSE;
}

void
FT_Init(void)
{
  if (FT_Init_FreeType((FT_Library*)&fg.ft)) {
    FT_ERROR("Unable to initialize FreeType.");
  }
}

void
FT_Shutdown(void)
{
  if (fg.defaultFontObject) {
    OBJ_ReleaseRef(fg.defaultFontObject);
    fg.defaultFontObject = NULL;
  }
  FontFamily_ReleaseFaces(&fg.defaultFontStorage);
  fg.defaultFont = NULL;
  fg.defaultFontTried = FALSE;

  FT_Done_FreeType(fg.ft);
  fg.ft = NULL;
}

struct fontface*
Font_Load(lpcString_t szFileName, struct FontFamily *pFontFamily)
{
  struct file* pFile = FS_LoadFile(szFileName);
  if (pFile) {
    struct fontface *face = Font_LoadFromMemory(pFile->data, pFile->size, pFontFamily);
    FS_FreeFile(pFile);
    return face;
  } else {
    return NULL;
  }
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
    Font_Load(Font_ResolvePath(hObject, pFontFamily->Regular, resolved), pFontFamily);
  }
  if (pFontFamily->Bold && *pFontFamily->Bold) {
    path_t resolved = {0};
    Font_Load(Font_ResolvePath(hObject, pFontFamily->Bold, resolved), pFontFamily);
  }
  if (pFontFamily->Italic && *pFontFamily->Italic) {
    path_t resolved = {0};
    Font_Load(Font_ResolvePath(hObject, pFontFamily->Italic, resolved), pFontFamily);
  }
  if (pFontFamily->BoldItalic && *pFontFamily->BoldItalic) {
    path_t resolved = {0};
    Font_Load(Font_ResolvePath(hObject, pFontFamily->BoldItalic, resolved), pFontFamily);
  }
  return TRUE;
}

// FontFamily_Destroy
HANDLER(FontFamily, Object, Destroy) {
  FontFamily_ReleaseFaces(pFontFamily);
  return TRUE;
}
