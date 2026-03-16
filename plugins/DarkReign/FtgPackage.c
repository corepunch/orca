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

/* ---- .spr / .spt loading ---- */

#define SPR_DEFAULT_FRAMERATE 10.0f  /* used when the SPR stores framerate=0 */

/*
 * Dark Reign sprite format (best-effort reverse-engineering of the 1997 Auran
 * engine, cross-referenced with the Volcano Lua conversion data):
 *
 * SPR file — animation descriptor, no pixel data:
 *   Header (32 bytes):
 *     char     texture_name[28]  null-terminated name of the .spt texture
 *                                file that holds the actual pixels (no ext)
 *     uint16_t num_frames        number of animation frames
 *     uint16_t framerate         frames per second (0 → use SPR_DEFAULT_FRAMERATE)
 *
 *   Per-frame descriptors (12 bytes × num_frames):
 *     uint16_t x, y   pixel position of the frame in the .spt texture
 *     uint16_t w, h   frame dimensions in pixels
 *     int16_t  dx, dy display hotspot offset
 *
 * SPT file — the palette-indexed texture atlas:
 *   uint16_t width
 *   uint16_t height
 *   [optional 256-entry palette: 256 × 3 bytes RGB, skipped here]
 *   uint8_t  pixels[width × height]   palette indices, treated as GL_RED
 *
 * Because multiple SPR animations can reference the same SPT atlas, the atlas
 * texture is loaded once per SPT filename (deduplication is left to the GPU
 * driver's texture cache).
 */
#define SPT_HEADER_SIZE  4           /* uint16 width + uint16 height */
#define SPT_PALETTE_SIZE (256 * 3)   /* 256-entry RGB palette        */

#pragma pack(push, 1)
struct _spr_header {
  char     texture_name[28]; /* null-terminated SPT name (no extension)     */
  uint16_t num_frames;       /* number of animation frames                  */
  uint16_t framerate;        /* frames per second; 0 → SPR_DEFAULT_FRAMERATE */
};
struct _spr_frame_desc {
  uint16_t x;       /* pixel x in SPT atlas  */
  uint16_t y;       /* pixel y in SPT atlas  */
  uint16_t w;       /* frame width in pixels */
  uint16_t h;       /* frame height in pixels*/
  int16_t  dx;      /* hotspot x offset      */
  int16_t  dy;      /* hotspot y offset      */
};
#pragma pack(pop)

/*
 * _SptFile_Load
 *
 * Read the SPT texture file named <texture_name>.spt from the FTG archive,
 * upload it as a kTextureFormatAlpha8 texture and return it.  Writes the
 * texture dimensions into *out_w / *out_h.  Returns NULL on failure.
 *
 * The SPT layout is: uint16 width, uint16 height, then either
 *   a) width × height raw bytes                (no palette)
 *   b) 768 bytes palette, then width × height  (with palette, palette skipped)
 * We distinguish the two cases by comparing the file size.
 */
static struct Texture *
_SptFile_Load(PFTG ftg, char const *texture_name, int32_t *out_w, int32_t *out_h)
{
  char spt_name[FTG_MAX_FILENAME] = {0};
  snprintf(spt_name, sizeof(spt_name) - 1, "%s.spt", texture_name);

  struct _FTGFILE *entry = _FindFtgEntry(ftg, spt_name);
  if (!entry)
    return NULL;

  struct file *f = _ReadFtgEntry(ftg, entry);
  if (!f)
    return NULL;

  if (f->size < SPT_HEADER_SIZE) {
    free(f);
    return NULL;
  }

  /* Use memcpy to read uint16 fields safely regardless of alignment */
  uint16_t w, h;
  memcpy(&w, f->data + 0, sizeof(w));
  memcpy(&h, f->data + 2, sizeof(h));

  if (w == 0 || h == 0) {
    free(f);
    return NULL;
  }

  uint32_t const expected_plain   = (uint32_t)SPT_HEADER_SIZE + (uint32_t)w * h;
  uint32_t const expected_palette = (uint32_t)SPT_HEADER_SIZE + SPT_PALETTE_SIZE
                                    + (uint32_t)w * h;

  uint8_t const *pixels = NULL;
  if (f->size >= expected_palette) {
    pixels = f->data + SPT_HEADER_SIZE + SPT_PALETTE_SIZE; /* skip header + palette */
  } else if (f->size >= expected_plain) {
    pixels = f->data + SPT_HEADER_SIZE;                    /* no palette */
  } else {
    Con_Error("DarkReign: '%s' size %u too small for %ux%u image",
              spt_name, f->size, w, h);
    free(f);
    return NULL;
  }

  struct Texture *tex = NULL;
  HRESULT hr = Texture_Create(
      &(CREATEIMGSTRUCT){
        .Width     = w,
        .Height    = h,
        .Scale     = 1,
        .Format    = kTextureFormatAlpha8,
        .ImageData = pixels,
        .MinFilter = kTextureFilterNearest,
        .MagFilter = kTextureFilterNearest,
        .WrapMode  = kTextureWrapClamp,
      },
      &tex);
  free(f);

  if (FAILED(hr) || !tex)
    return NULL;

  if (out_w) *out_w = w;
  if (out_h) *out_h = h;
  return tex;
}

/*
 * _SprFile_Load
 *
 * Parse a raw .spr descriptor file, load the referenced .spt texture from
 * the same FTG archive, and return a heap-allocated SpriteAnimation Object.
 * Returns NULL on any parse or allocation failure.
 *
 * On format mismatch the first 32 file bytes are printed to stderr so the
 * caller can diagnose unexpected file layouts.
 */
static lpObject_t
_SprFile_Load(PFTG ftg, uint8_t const *data, uint32_t size, lpcString_t name)
{
  /* ---- validate SPR header ---- */
  if (size < sizeof(struct _spr_header)) {
    Con_Error("DarkReign: '%s' too small for SPR header (%u bytes)", name, size);
    return NULL;
  }

  struct _spr_header const *hdr = (struct _spr_header const *)data;

  /* texture_name must be a non-empty printable ASCII string */
  if ((unsigned char)hdr->texture_name[0] < 0x20) {
    /* Dump the first bytes to help diagnose unexpected formats */
    uint32_t const dump_len = size < 31 ? size : 31; /* 31 × 3 + '\0' = 94 bytes */
    char hex[100] = {0};
    for (uint32_t k = 0; k < dump_len; k++)
      snprintf(hex + k*3, sizeof(hex) - k*3, "%02x ", data[k]);
    Con_Error("DarkReign: '%s' has unexpected header (size=%u bytes=%s...)",
              name, size, hex);
    return NULL;
  }

  uint16_t const num_frames = hdr->num_frames;
  if (num_frames == 0)
    return NULL;

  uint32_t const frame_bytes = (uint32_t)num_frames * sizeof(struct _spr_frame_desc);
  if (size < sizeof(struct _spr_header) + frame_bytes)
    return NULL;

  struct _spr_frame_desc const *descs =
      (struct _spr_frame_desc const *)(data + sizeof(struct _spr_header));

  /* ---- load the referenced SPT texture ---- */
  int32_t tex_w = 0, tex_h = 0;
  struct Texture *tex = _SptFile_Load(ftg, hdr->texture_name, &tex_w, &tex_h);
  if (!tex) {
    Con_Error("DarkReign: '%s' references texture '%s' which could not be loaded",
              name, hdr->texture_name);
    return NULL;
  }

  /* ---- build per-frame UV descriptors ---- */
  struct SpriteFrame *frames =
      (struct SpriteFrame *)malloc((size_t)num_frames * sizeof(struct SpriteFrame));
  if (!frames) {
    Texture_Release(tex);
    return NULL;
  }

  float const ftw = (float)tex_w;
  float const fth = (float)tex_h;
  for (uint16_t i = 0; i < num_frames; i++) {
    /* Rect: place sprite so its hotspot falls at the node's origin */
    frames[i].Rect.x      = -(float)descs[i].dx;
    frames[i].Rect.y      = -(float)descs[i].dy;
    frames[i].Rect.width  =  (float)descs[i].w;
    frames[i].Rect.height =  (float)descs[i].h;

    /* UvRect: normalized atlas coordinates derived from pixel positions */
    frames[i].UvRect.x      = (float)descs[i].x / ftw;
    frames[i].UvRect.y      = (float)descs[i].y / fth;
    frames[i].UvRect.width  = (float)descs[i].w / ftw;
    frames[i].UvRect.height = (float)descs[i].h / fth;
  }

  /* ---- create SpriteAnimation Object ---- */
  lpObject_t obj = OBJ_MakeNativeObject(ID_SpriteAnimation);
  if (!obj) {
    free(frames);
    Texture_Release(tex);
    return NULL;
  }
  OBJ_SetName(obj, name);

  struct SpriteAnimation *anim = GetSpriteAnimation(obj);
  anim->Image     = tex;
  anim->Framerate = hdr->framerate > 0 ? (float)hdr->framerate
                                       : SPR_DEFAULT_FRAMERATE;
  anim->NumFrames = (int32_t)num_frames;
  anim->Frames    = frames;

  return obj;
}

/*
 * _LoadSprAnimations
 *
 * Walk every entry in the FTG archive.  For each file whose name ends with
 * ".spr" (case-insensitive), parse it with _SprFile_Load() and add the
 * resulting SpriteAnimation Object as a direct child of |project|.
 * Successfully loaded animations are accessible by their basename (no ext).
 */
static void
_LoadSprAnimations(PFTG ftg, lpObject_t project)
{
  int loaded = 0, skipped = 0;

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
      skipped++;
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

    lpObject_t anim_obj = _SprFile_Load(ftg, f->data, f->size, anim_name);
    free(f);

    if (!anim_obj) {
      skipped++;
      continue;
    }

    OBJ_AddChild(project, anim_obj, FALSE);
    loaded++;
  }

  if (skipped > 0)
    Con_Error("DarkReign: loaded %d sprite animations, skipped %d "
              "(missing .spt or unexpected format)", loaded, skipped);
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

  /* Load all .spr sprite files from the archive and attach them to the project
   * as SpriteAnimation children so they are immediately accessible by name. */
  _LoadSprAnimations(ftg, project);

  return (intptr_t)project;
}
