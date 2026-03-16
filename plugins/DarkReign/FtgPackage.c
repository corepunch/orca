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

/* ---- .spr loading ---- */

/*
 * Dark Reign .spr format (Auran engine, 1997).
 * Reference: github.com/drogoganor/OpenDR — OpenRA.Mods.Dr/SpriteLoaders/DrSprLoader.cs
 *
 * Header (8 × int32 = 32 bytes, all little-endian):
 *   char[4]  magic     "RSPR" (regular) or "SSPR" (shadow — pixels set to index 47)
 *   int32    version   0x0210
 *   int32    nanims    total animation frame count (across all sections)
 *   int32    nrots     rotation directions (e.g. 8 for 8-directional unit)
 *   int32    szx       sprite cell width in pixels
 *   int32    szy       sprite cell height in pixels
 *   int32    npics     number of unique compressed pictures
 *   int32    nsects    number of animation sections
 *
 * Layout after header:
 *   int32[nanims × nrots]        pic-index table: pic# = table[anim*nrots + rot]
 *   SprSection[nsects]           each 16 bytes: firstanim, lastanim, fps, nhot
 *   int32[nanims]                (per-anim extra data, not used here)
 *   PicEntry[npics] + int32      each 8 bytes: {picoff, hotoff}; +4 end sentinel
 *   uint8[]                      RLE-compressed pixel data
 *
 * RLE encoding (per scanline, alternating transparent/opaque runs):
 *   step=0 (transparent): n = src[curr++]; skip n pixels (remain 0)
 *   step=1 (opaque):       n = src[curr++] & 0x7f; copy n bytes into output
 *   … repeat until currx == szx
 *
 * Atlas layout:  nrots columns × nanims rows, each cell szx × szy pixels.
 * Frame (anim a, rotation r) → cell at column r, row a.
 */

#define SPR_HEADER_SIZE  32
#define SPR_VERSION      0x0210

static bool
_spr_is_valid(uint8_t const *data, uint32_t size)
{
  if (size < SPR_HEADER_SIZE)
    return false;
  if (memcmp(data, "RSPR", 4) != 0 && memcmp(data, "SSPR", 4) != 0)
    return false;
  int32_t version;
  memcpy(&version, data + 4, sizeof(version));
  return version == SPR_VERSION;
}

/* Decompress one RLE-encoded picture into a szx×szy output buffer.
 * Shadow sprites fill every opaque pixel with palette index 47. */
static void
_spr_decompress(uint8_t *restrict out, uint32_t szx, uint32_t szy,
                uint8_t const *src, uint32_t src_len, bool is_shadow)
{
  memset(out, 0, szx * szy);
  uint32_t curr = 0;
  for (uint32_t l = 0; l < szy; l++) {
    uint32_t step = 0, currx = 0;
    while (currx < szx) {
      if (curr >= src_len) return;
      uint32_t cnt = src[curr++];
      bool const opaque = (step & 1u) != 0;
      if (opaque) cnt &= 0x7fu;
      if (opaque) {
        /* opaque run: copy pixel bytes into output */
        for (uint32_t i = 0; i < cnt; i++) {
          if (curr >= src_len) return;
          uint8_t px = src[curr++];
          uint32_t idx = l * szx + currx + i;
          if (idx < szx * szy)
            out[idx] = is_shadow ? 47u : px;
        }
      }
      /* transparent run: pixels remain 0 */
      currx += cnt;
      step++;
    }
  }
}

/*
 * _SprFile_Load
 *
 * Parse a Dark Reign .spr file from raw bytes, decompress all frames into a
 * texture atlas, and return a heap-allocated SpriteAnimation Object.
 * Returns NULL on any parse or allocation failure.
 *
 * The atlas is nrots×szx wide and nanims×szy tall.  Frame (a, r) occupies
 * the cell at pixel column r*szx and row a*szy.
 */
static lpObject_t
_SprFile_Load(uint8_t const *data, uint32_t size, lpcString_t name)
{
  if (!_spr_is_valid(data, size)) {
    Con_Error("DarkReign: '%s': not a valid RSPR/SSPR file (size=%u)", name, size);
    return NULL;
  }

  bool const is_shadow = (memcmp(data, "SSPR", 4) == 0);

  /* Read header fields with memcpy for alignment safety */
  int32_t nanims, nrots, szx, szy, npics, nsects;
  memcpy(&nanims,  data +  8, 4);
  memcpy(&nrots,   data + 12, 4);
  memcpy(&szx,     data + 16, 4);
  memcpy(&szy,     data + 20, 4);
  memcpy(&npics,   data + 24, 4);
  memcpy(&nsects,  data + 28, 4);

  if (nanims <= 0 || nrots <= 0 || szx <= 0 || szy <= 0 || npics <= 0 || nsects <= 0) {
    Con_Error("DarkReign: '%s': invalid SPR dimensions", name);
    return NULL;
  }

  /* Compute data-section offsets (all relative to start of file) */
  uint32_t const off_pic_table  = SPR_HEADER_SIZE;          /* int32 × nanims×nrots */
  uint32_t const off_sections   = off_pic_table
                                   + (uint32_t)nanims * (uint32_t)nrots * 4u;
  uint32_t const off_anims      = off_sections + (uint32_t)nsects * 16u;
  uint32_t const off_picoffs    = off_anims + (uint32_t)nanims * 4u;
  uint32_t const off_bits       = off_picoffs + (uint32_t)npics * 8u + 4u;

  if (size < off_bits) {
    Con_Error("DarkReign: '%s': file too small (%u bytes, need %u)", name, size, off_bits);
    return NULL;
  }

  /* Read framerate from the first section (firstanim, lastanim, fps, nhot) */
  int32_t fps = 0;
  if (size >= off_sections + 12u)
    memcpy(&fps, data + off_sections + 8u, 4);
  float const framerate = (fps > 0) ? (float)fps : 10.0f;

  /* Build atlas: nrots columns × nanims rows, each cell szx×szy */
  uint32_t const atlas_w = (uint32_t)nrots  * (uint32_t)szx;
  uint32_t const atlas_h = (uint32_t)nanims * (uint32_t)szy;

  /* Guard against implausibly large atlases (max 8192×8192) */
  if (atlas_w > 8192u || atlas_h > 8192u) {
    Con_Error("DarkReign: '%s': atlas too large (%ux%u)", name, atlas_w, atlas_h);
    return NULL;
  }

  uint8_t *atlas = (uint8_t *)calloc(1, (size_t)atlas_w * (size_t)atlas_h);
  if (!atlas)
    return NULL;

  /* Decompress every (anim, rot) pair into the atlas */
  for (int32_t a = 0; a < nanims; a++) {
    for (int32_t r = 0; r < nrots; r++) {
      /* Look up the compressed-picture index for this (anim, rot) */
      uint32_t const tbl_off = off_pic_table + (uint32_t)(a * nrots + r) * 4u;
      if (tbl_off + 4u > size) continue;
      int32_t picnr;
      memcpy(&picnr, data + tbl_off, 4);
      if (picnr < 0 || picnr >= npics) continue;

      /* Read the compressed-data offset pair for this picture */
      uint32_t const pe_off      = off_picoffs + (uint32_t)picnr * 8u;
      uint32_t const pe_next_off = off_picoffs + (uint32_t)(picnr + 1) * 8u;
      if (pe_next_off + 4u > size) continue;
      int32_t picoff, nextpicoff;
      memcpy(&picoff,     data + pe_off,      4);
      memcpy(&nextpicoff, data + pe_next_off, 4);
      if (picoff < 0 || nextpicoff <= picoff) continue;

      uint32_t const src_start = off_bits + (uint32_t)picoff;
      uint32_t const src_end   = off_bits + (uint32_t)nextpicoff;
      if (src_end > size) continue;

      /* Decompress into a temporary cell-sized buffer */
      uint8_t *cell = (uint8_t *)calloc(1, (size_t)szx * (size_t)szy);
      if (!cell) {
        Con_Warning("DarkReign: '%s': out of memory for frame (%d,%d)", name, a, r);
        continue;
      }
      _spr_decompress(cell, (uint32_t)szx, (uint32_t)szy,
                      data + src_start, src_end - src_start, is_shadow);

      /* Copy cell into the atlas at (col=r, row=a) */
      uint32_t const cell_x = (uint32_t)r * (uint32_t)szx;
      uint32_t const cell_y = (uint32_t)a * (uint32_t)szy;
      for (int32_t row = 0; row < szy; row++) {
        memcpy(atlas + (cell_y + (uint32_t)row) * atlas_w + cell_x,
               cell  + (uint32_t)row * (uint32_t)szx,
               (size_t)szx);
      }
      free(cell);
    }
  }

  /* Upload atlas as a single-channel texture */
  struct Texture *tex = NULL;
  HRESULT hr = Texture_Create(
      &(CREATEIMGSTRUCT){
        .Width     = (int32_t)atlas_w,
        .Height    = (int32_t)atlas_h,
        .Scale     = 1,
        .Format    = kTextureFormatAlpha8,
        .ImageData = atlas,
        .MinFilter = kTextureFilterNearest,
        .MagFilter = kTextureFilterNearest,
        .WrapMode  = kTextureWrapClamp,
      },
      &tex);
  free(atlas);
  if (FAILED(hr) || !tex)
    return NULL;

  /* Build per-frame SpriteFrame array (nanims × nrots frames) */
  int32_t const nframes = nanims * nrots;
  struct SpriteFrame *frames =
      (struct SpriteFrame *)malloc((size_t)nframes * sizeof(struct SpriteFrame));
  if (!frames) {
    Texture_Release(tex);
    return NULL;
  }

  float const inv_w = 1.0f / (float)atlas_w;
  float const inv_h = 1.0f / (float)atlas_h;
  float const uv_cw = (float)szx * inv_w;  /* normalized cell width  */
  float const uv_ch = (float)szy * inv_h;  /* normalized cell height */

  for (int32_t a = 0; a < nanims; a++) {
    for (int32_t r = 0; r < nrots; r++) {
      int32_t const fi = a * nrots + r;
      /* Rect: full cell size, origin at cell centre (hotspot not read here) */
      frames[fi].Rect.x      = -(float)szx * 0.5f;
      frames[fi].Rect.y      = -(float)szy * 0.5f;
      frames[fi].Rect.width  =  (float)szx;
      frames[fi].Rect.height =  (float)szy;
      /* UvRect: this (anim, rot) cell in the atlas */
      frames[fi].UvRect.x      = (float)r * uv_cw;
      frames[fi].UvRect.y      = (float)a * uv_ch;
      frames[fi].UvRect.width  = uv_cw;
      frames[fi].UvRect.height = uv_ch;
    }
  }

  /* Create SpriteAnimation Object */
  lpObject_t obj = OBJ_MakeNativeObject(ID_SpriteAnimation);
  if (!obj) {
    free(frames);
    Texture_Release(tex);
    return NULL;
  }
  OBJ_SetName(obj, name);

  struct SpriteAnimation *anim = GetSpriteAnimation(obj);
  anim->Image     = tex;
  anim->Framerate = framerate;
  anim->NumFrames = nframes;
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

    lpObject_t anim_obj = _SprFile_Load(f->data, f->size, anim_name);
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
              "(invalid RSPR/SSPR format or allocation failure)", loaded, skipped);
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
