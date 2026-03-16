/*
 * FtgPackage.c - FTG package loader for Orca
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
 *     char    name[28];           // Null-terminated filename (up to 27 chars)
 *     int32_t offset;             // Byte offset of the file data
 *     int32_t size;               // Size of the file data in bytes
 *
 *   File data is stored uncompressed at the given offsets.
 *
 * Loading sprites from an FTG package:
 *   1. Register the FTG archive as a bundle:
 *        FS_LoadBundle(L, "path/to/archive");   // opens "archive.ftg"
 *   2. Access files by path "packagename/filename":
 *        struct file *f = FS_LoadFile("archive/sprites/hero.raw");
 *   3. To use with SpriteKit, map the raw bitmap data to a SpriteAnimation:
 *        - Create an ImageLibrary entry pointing to the loaded image data
 *        - Create a SpriteAnimation referencing the image with frame rects
 *        - Assign the SpriteAnimation to an SKSpriteNode.Animation property
 */

#include <include/orca.h>
#include "fs_local.h"

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
 * FtgPackage_LoadProject
 *
 * The Bundle iteration in filesystem.c calls this handler for every Bundle
 * class and passes the bare directory path (without extension).  We try to
 * open <path>.ftg; if the file does not exist we return 0 so the next Bundle
 * class gets a chance.  On success we create a default empty Project and
 * attach the FtgPackage component to it.
 */
extern lpObject_t _LoadProject(lua_State *L, lpcString_t path, lpcString_t name);

HANDLER(FtgPackage, LoadProject) {
  path_t tmp = { 0 };
  /*
   * When kEventLoadProject is dispatched from _TryLoadBundle() in filesystem.c,
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
  lpObject_t project = _LoadProject(L, pLoadProject->Path,
                                    FS_GetBaseName(pLoadProject->Path));
  if (!project) {
    _FreeFtg(ftg);
    return 0;
  }

  OBJ_AddComponent(project, ID_FtgPackage);
  GetFtgPackage(project)->_ftg = ftg;

  return (intptr_t)project;
}
