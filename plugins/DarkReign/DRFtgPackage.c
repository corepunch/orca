/*
 * FtgPackage.c - FTG package loader for Orca (DarkReign plugin)
 *
 * FTG is the archive format used by the Dark Reign game.
 * Based on the drExplorer reference: https://github.com/btigi/drExplorer
 *
 * FTG File Format:
 *   Header (12 bytes):
 *     uint32_t identifier;        // Magic identifier bytes
 *     int32_t  directoryOffset;   // Byte offset of the file directory
 *     int32_t  fileCount;         // Number of files in the archive
 *
 *   Directory (at directoryOffset, each entry is 36 bytes):
 *     char    name[28];           // Null-terminated filename (up to 27 chars + NUL)
 *     int32_t offset;             // Byte offset of the file data
 *     int32_t size;               // Size of the file data in bytes
 *
 *   File data is stored uncompressed at the given offsets.
 *
 * SPR sprite file format (palette-indexed, 8 bits per pixel):
 *
 *   Header (8 bytes):
 *     uint16_t num_frames   – total number of animation frames
 *     uint16_t max_width    – maximum frame width  (informational; may be 0)
 *     uint16_t max_height   – maximum frame height (informational; may be 0)
 *     uint16_t reserved     – unused / padding
 *
 *   Per-frame descriptors (8 bytes × num_frames):
 *     uint16_t width        – pixel width of this frame
 *     uint16_t height       – pixel height of this frame
 *     int16_t  offset_x     – display hotspot X: sprite origin is offset_x pixels
 *                             to the right of the top-left corner
 *     int16_t  offset_y     – display hotspot Y: sprite origin is offset_y pixels
 *                             below the top-left corner
 *
 *   Raw pixel data (1 byte per pixel, palette index):
 *     For each frame in order: width × height bytes, rows top-to-bottom.
 *     Palette is not stored; each byte is uploaded as-is to a 1-channel
 *     GL_RED (kTextureFormatAlpha8) texture.
 *
 * Texture atlas strategy:
 *   Frames are packed into a horizontal strip:
 *     atlas_width  = sum of all frame widths
 *     atlas_height = maximum frame height
 *   Frame i starts at x = sum of widths of frames 0 … i-1, y = 0.
 *   Short frames are not padded; their UvRect.height is set proportionally.
 */

#include <include/orca.h>
#include <include/renderer.h>
#include <source/core/core.h>
#include <plugins/SpriteKit/SpriteKit.h>
#include "DarkReign.h"

#define FTG_MAX_FILENAME 28     /* 27 chars + 1 null terminator */
#define FTG_MAX_FILES    65536  /* sanity cap to prevent DoS via huge fileCount */

struct _FTGFILE
{
  char     name[FTG_MAX_FILENAME]; /* null-terminated filename (up to 27 chars) */
  int32_t  offset;                 /* byte offset of the file data              */
  int32_t  size;                   /* size of the file data in bytes            */
};

typedef struct _FTG
{
  char            filename[MAX_OSPATH];
  FILE           *handle;
  int             numfiles;
  struct _FTGFILE *files;
} *PFTG;

/* ---- internal helpers ---- */

static PFTG
_LoadFtgFile(lpcString_t szFtgfile)
{
  uint32_t identifier;
  int32_t  dirofs, numfiles;

  FILE *handle = fopen(szFtgfile, "rb");
  if (!handle)
    return NULL;

  if (fread(&identifier, sizeof(identifier), 1, handle) != 1 ||
      fread(&dirofs,     sizeof(dirofs),     1, handle) != 1 ||
      fread(&numfiles,   sizeof(numfiles),   1, handle) != 1) {
    fclose(handle);
    return NULL;
  }

  if (dirofs < (int32_t)sizeof(uint32_t) * 3 || numfiles < 0 ||
      numfiles > FTG_MAX_FILES) {
    Con_Error("%s is not a valid FTG file", szFtgfile);
    fclose(handle);
    return NULL;
  }

  PFTG ftg     = ZeroAlloc(sizeof(struct _FTG));
  ftg->handle  = handle;
  ftg->numfiles = numfiles;
  ftg->files   = ZeroAlloc((size_t)numfiles * sizeof(struct _FTGFILE));
  strncpy(ftg->filename, szFtgfile, sizeof(ftg->filename) - 1);

  fseek(handle, dirofs, SEEK_SET);
  if (fread(ftg->files, sizeof(struct _FTGFILE), (size_t)numfiles, handle) !=
      (size_t)numfiles) {
    Con_Error("Failed to read directory from %s", szFtgfile);
    free(ftg->files);
    fclose(handle);
    free(ftg);
    return NULL;
  }

  return ftg;
}

static void
_FreeFtg(PFTG ftg)
{
  if (!ftg)
    return;
  fclose(ftg->handle);
  free(ftg->files);
  free(ftg);
}

/* Return a pointer to the entry whose name matches, or NULL. */
static struct _FTGFILE *
_FindFtgEntry(PFTG ftg, lpcString_t basename)
{
  for (int i = 0; i < ftg->numfiles; i++) {
    /* Compare case-insensitively; FTG names may be upper-case on disk. */
    if (!strcasecmp(ftg->files[i].name, basename))
      return &ftg->files[i];
  }
  return NULL;
}

/* Read a file from the FTG archive and return a heap-allocated struct file. */
static struct file *
_ReadFtgEntry(PFTG ftg, struct _FTGFILE const *entry)
{
  if (entry->size < 0 || (size_t)entry->size > SIZE_MAX - sizeof(struct file) - 1) {
    Con_Error("Invalid file size %d in FTG archive %s", entry->size, ftg->filename);
    return NULL;
  }
  /* add 1 so callers can safely append a '\0' terminator */
  struct file *f = ZeroAlloc(sizeof(struct file) + (size_t)entry->size + 1);
  if (!f)
    return NULL;
  f->size = (uint32_t)entry->size;
  fseek(ftg->handle, entry->offset, SEEK_SET);
  if (fread(f->data, 1, (size_t)entry->size, ftg->handle) != (size_t)entry->size) {
    Con_Error("Failed to read %s from FTG archive %s",
              entry->name, ftg->filename);
    free(f);
    return NULL;
  }
  return f;
}

/* ---- event handlers ---- */

HANDLER(FtgPackage, OpenFile) {
  struct _FTGFILE *entry = _FindFtgEntry(pFtgPackage->_ftg, pOpenFile->FileName);
  if (!entry)
    return 0;
//  fprintf(stderr, "FtgPackage: opening file '%s' from archive '%s'\n", entry->name, pFtgPackage->_ftg->filename);
  return (LRESULT)_ReadFtgEntry(pFtgPackage->_ftg, entry);
}

HANDLER(FtgPackage, FileExists) {
  return _FindFtgEntry(pFtgPackage->_ftg, pFileExists->FileName) != NULL;
}

HANDLER(FtgPackage, HasChangedFiles) {
  return FALSE;
}

HANDLER(FtgPackage, Destroy) {
  _FreeFtg(pFtgPackage->_ftg);
  return FALSE;
}

/*
 * _LoadSprAnimations
 *
 * Walk every entry in the FTG archive.  For each file whose name ends with
 * ".spr" (case-insensitive), parse it with _SprFile_Load() and add the
 * resulting SpriteAnimation Object as a direct child of |project|.
 */

#if 0
lpObject_t
_SprFile_Load(lua_State* L, uint8_t const *data, uint32_t size, lpcString_t name);

static void
_LoadSprAnimations(lua_State* L, PFTG ftg, lpObject_t project)
{
  for (int i = 0; i < ftg->numfiles; i++) {
    char const *fname = ftg->files[i].name;
    size_t      flen  = strnlen(fname, FTG_MAX_FILENAME);

    /* need at least "x.spr" (5 chars) */
    if (flen < 5)
      continue;
    if (strcasecmp(fname + flen - 4, ".spr") != 0)
      continue;

    struct file *f = _ReadFtgEntry(ftg, &ftg->files[i]);
    if (!f) {
      Con_Error("DarkReign: could not read '%s'", fname);
      continue;
    }

    /* derive the animation name by stripping the ".spr" suffix */
    char anim_name[FTG_MAX_FILENAME] = {0};
    strncpy(anim_name, fname, sizeof(anim_name) - 1);
    anim_name[sizeof(anim_name) - 1] = '\0';
    {
      char *dot = strrchr(anim_name, '.');
      if (dot)
        *dot = '\0';
    }

    lpObject_t anim_obj = _SprFile_Load(L, f->data, f->size, anim_name);
    free(f);

    if (!anim_obj) {
      Con_Error("DarkReign: failed to parse sprite '%s'", fname);
      continue;
    }

    OBJ_AddChild(project, anim_obj, FALSE);
  }
}
#endif

/*
 * FtgPackage_LoadProject
 *
 * The Bundle iteration in filesystem.c calls this handler for every Bundle
 * class and passes the bare directory path (without extension).  We try to
 * open <path>.ftg; if the file does not exist we return 0 so the next Bundle
 * class gets a chance.  On success we create a default empty Project and
 * attach the FtgPackage component to it.
 */

#include <include/api.h>

HANDLER(FtgPackage, LoadProject) {
  path_t tmp = { 0 };
  /*
   * When ID_LoadProject is dispatched from _TryLoadBundle() in filesystem.c,
   * the object does not yet have a component attached.  The engine passes the
   * lua_State pointer as the second ObjProc argument instead of a real
   * component pointer – the same convention used by Package.c.
   */
  lua_State *L = (lua_State *)pFtgPackage;

  snprintf(tmp, sizeof(tmp), "%s.ftg", pLoadProject->Path);

  /* Peek at the file – if it cannot be opened this is not an FTG bundle. */
  FILE *fp = fopen(tmp, "rb");
  if (!fp)
    return 0;
  fclose(fp);

  PFTG ftg = _LoadFtgFile(tmp);
  if (!ftg)
    return 0;

  /*
   * _LoadProject will first try require("<path>"); because there is no
   * package.lua embedded in an FTG archive this will fail, and it will fall
   * back to creating a bare Project object – which is exactly what we want.
   */
  lua_pcall(L, (luaX_import(L, "orca.DarkReign", "FtgPackage"), 0), 1, 0);
  lpObject_t project = luaX_checkObject(L, -1);
  GetFtgPackage(project)->_ftg = ftg;

  // Debug: list all files in the archive
  // for (int i = 0; i < ftg->numfiles; i++) {
  //   char const *fname = ftg->files[i].name;
  //   fprintf(stderr, "FtgPackage: found file '%s' in archive '%s' size %u\n", fname, ftg->filename, ftg->files[i].size);
  // }

  /* Load all .spr sprite files from the archive and attach them to the project
   * as SpriteAnimation children so they are immediately accessible by name. */
  // _LoadSprAnimations(L, ftg, project);

  return (intptr_t)project;
}
