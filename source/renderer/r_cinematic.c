#include "r_local.h"

#define MAX_FILENAME 256
#define ANIM_RATE 25
#define NUM_FRAMES 60
#define RESET_FRAME_TO_ZERO
#define FADE_OUT_DURATION 500
// #define LOOP_CINEMATIC

extern bool_t FS_FileExists(lpcString_t path);

typedef struct _CINEMATICS
{
  char filename[MAX_FILENAME];
  byte_t* pic;
  struct color32 palette[256];
  uint32_t width;
  uint32_t height;
} CINEMATICS, *PCINEMATICS;

static CINEMATICS cin = { 0 };

typedef struct
{
  char manufacturer;
  char version;
  char encoding;
  char bits_per_pixel;
  unsigned short xmin, ymin, xmax, ymax;
  unsigned short hres, vres;
  unsigned char palette[48];
  char reserved;
  char color_planes;
  unsigned short bytes_per_line;
  unsigned short palette_type;
  char filler[58];
  unsigned char data; // unbounded
} pcx_t;

static void
_LoadPCX(lpcString_t filename, PCINEMATICS out)
{
  struct file* file = FS_LoadFile(filename);

  if (!file)
    return; // Com_Printf ("Bad pcx file %s\n", filename);

  pcx_t* pcx = (pcx_t*)file->data;

  if (pcx->manufacturer != 0x0a || pcx->version != 5 || pcx->encoding != 1) {
    Con_Error("Bad pcx file %s\n", filename);
    return;
  }

  if (out->width != pcx->xmax + 1 || out->height != pcx->ymax + 1) {
    out->pic = realloc(out->pic, (pcx->ymax + 1) * (pcx->xmax + 1));
    out->width = pcx->xmax + 1;
    out->height = pcx->ymax + 1;
  }

  if (pcx->bits_per_pixel == 1) {
    memset(out->palette, 0, 768);
    memset(out->pic, 0, (pcx->ymax + 1) * (pcx->xmax + 1));
    FS_FreeFile(file);
    return;
  }

  if (pcx->bits_per_pixel != 8) {
    Con_Error("Bad pcx file %s\n", filename);
    SafeFree(out->pic);
    FS_FreeFile(file);
    return;
  }

  byte_t* palette = (byte_t*)pcx + file->size - 768;
  byte_t* pix = out->pic;
  byte_t* end = out->pic + (pcx->ymax + 1) * (pcx->xmax + 1);
  byte_t* raw = &pcx->data;

  FOR_LOOP(i, 256)
  {
    out->palette[i].r = palette[i * 3 + 0];
    out->palette[i].g = palette[i * 3 + 1];
    out->palette[i].b = palette[i * 3 + 2];
    out->palette[i].a = -1;
  }

  // optimization, reset to zero and then skip all zeroes
#ifdef RESET_FRAME_TO_ZERO
  memset(pix, 0, end - pix);
#endif

  while (pix < end) {
    byte_t dataByte = *raw++;
    if ((dataByte & 0xC0) == 0xC0) {
      uint32_t runLength = dataByte & 0x3F;
      dataByte = *raw++;
#ifdef RESET_FRAME_TO_ZERO
      if (dataByte)
#endif
        memset(pix, dataByte, runLength);
      pix += runLength;
    } else {
      *(pix++) = dataByte;
    }
  }

  if (raw - (byte_t*)pcx > file->size) {
    Con_Error("PCX file %s was malformed", filename);
    SafeFree(out->pic);
  }

  FS_FreeFile(file);
}

static void
_BindPBO(GLsizei size)
{
  if (tr.cinbuf.pbo == 0) {
    R_Call(glGenBuffers, 1, &tr.cinbuf.pbo);
  }
  R_Call(glBindBuffer, GL_PIXEL_UNPACK_BUFFER, tr.cinbuf.pbo);
  if (tr.cinbuf.size != size) {
    R_Call(glBufferData, GL_PIXEL_UNPACK_BUFFER, size, NULL, GL_STREAM_DRAW);
    tr.cinbuf.size = size;
  }
}

HRESULT
Cin_Load(lpcString_t filename, uint32_t frame)
{
  char name[MAX_FILENAME];

  snprintf(name, MAX_FILENAME, "%s_%05d.pcx", filename, frame);

  if (!FS_FileExists(name)) {
    Con_Error("Can't find cinematic %s", name);
    return E_ITEMNOTFOUND;
  }

  _LoadPCX(name, &cin);

  GLsizei w = cin.width;
  GLsizei h = cin.height;
  GLenum fmt = GL_RED;
  GLenum datatype = GL_UNSIGNED_BYTE;

  struct Texture* tex = tr.textures[TX_CINEMATIC];
  struct Texture* pal = tr.textures[TX_CINEMATICPALETTE];

  R_Call(glBindTexture, GL_TEXTURE_2D, tex->texnum);

  if (!strcmp(name, cin.filename)) {
    // skip
  } else if (tex->loaded && tex->Width == w && tex->Height == h) {
#ifdef GL_WRITE_ONLY
    _BindPBO(w * h);
    memcpy(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY), cin.pic, w * h);
    R_Call(glUnmapBuffer, GL_PIXEL_UNPACK_BUFFER);
    R_Call(glTexSubImage2D, GL_TEXTURE_2D, 0, 0, 0, w, h, fmt, datatype, NULL);
    R_Call(glBindBuffer, GL_PIXEL_UNPACK_BUFFER, 0);
#else
    R_Call(glTexSubImage2D, GL_TEXTURE_2D, 0, 0, 0, w, h, fmt, datatype, cin.pic);
#endif
  } else {
    R_Call(glTexImage2D, GL_TEXTURE_2D, 0, GL_R8, w, h, 0, fmt, datatype, cin.pic);
    tex->loaded = TRUE;
    tex->Width = w;
    tex->Height = h;
  }

  R_SetPointFiltering();

  R_Call(glBindTexture, GL_TEXTURE_2D, pal->texnum);
  R_Call(glTexSubImage2D, GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGBA, datatype, cin.palette);

  R_SetPointFiltering();

  //    R_DrawStretchRaw(&(DRAWSTRETCHRAWSTRUCT) {
  //        .rect = { (1920.f-cin.width)/2, (720.f-cin.height)/2, cin.width,
  //        cin.height}, .size = { cin.width, cin.height }, .palette =
  //        cin.palette, .data = cin.pic, .opacity = opacity, .refresh =
  //        refresh,
  //    });

  strncpy(cin.filename, name, MAX_FILENAME);

  return NOERROR;
}
