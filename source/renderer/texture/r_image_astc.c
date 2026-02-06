#include <stdio.h>

#include "../r_local.h"

#define ASTC_ERROR(...) Con_Error("ASTC: " __VA_ARGS__)

#ifndef GL_KHR_texture_compression_astc_hdr
#define GL_KHR_texture_compression_astc_hdr 1
#define GL_COMPRESSED_RGBA_ASTC_4x4_KHR 0x93B0
#define GL_COMPRESSED_RGBA_ASTC_5x4_KHR 0x93B1
#define GL_COMPRESSED_RGBA_ASTC_5x5_KHR 0x93B2
#define GL_COMPRESSED_RGBA_ASTC_6x5_KHR 0x93B3
#define GL_COMPRESSED_RGBA_ASTC_6x6_KHR 0x93B4
#define GL_COMPRESSED_RGBA_ASTC_8x5_KHR 0x93B5
#define GL_COMPRESSED_RGBA_ASTC_8x6_KHR 0x93B6
#define GL_COMPRESSED_RGBA_ASTC_8x8_KHR 0x93B7
#define GL_COMPRESSED_RGBA_ASTC_10x5_KHR 0x93B8
#define GL_COMPRESSED_RGBA_ASTC_10x6_KHR 0x93B9
#define GL_COMPRESSED_RGBA_ASTC_10x8_KHR 0x93BA
#define GL_COMPRESSED_RGBA_ASTC_10x10_KHR 0x93BB
#define GL_COMPRESSED_RGBA_ASTC_12x10_KHR 0x93BC
#define GL_COMPRESSED_RGBA_ASTC_12x12_KHR 0x93BD
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR 0x93D0
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR 0x93D1
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR 0x93D2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR 0x93D3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR 0x93D4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR 0x93D5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR 0x93D6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR 0x93D7
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR 0x93D8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR 0x93D9
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR 0x93DA
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR 0x93DB
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR 0x93DC
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR 0x93DD
#endif /* GL_KHR_texture_compression_astc_hdr */

struct astc_header
{
  uint8_t magic[4];
  uint8_t blockdim_x;
  uint8_t blockdim_y;
  uint8_t blockdim_z;
  uint8_t xsize[3];
  uint8_t ysize[3];
  uint8_t zsize[3];
};

struct WI_Size
R_TexImageASTC(GLenum target, struct WI_Buffer* sb)
{
  struct astc_header const* header = (void*)sb->data;
  if (*(int*)sb->data != ASTC_HEADER) {
    ASTC_ERROR("Not an ASTC file\n");
    return MAKE_TEX_SIZE(0, 0);
  }
  int xsize =
    (header->xsize[0]) + (header->xsize[1] << 8) + (header->xsize[2] << 16);
  int ysize =
    (header->ysize[0]) + (header->ysize[1] << 8) + (header->ysize[2] << 16);
  int readsize = sb->cursize - sizeof(struct astc_header);
  void const* ptr = sb->data + sizeof(struct astc_header);

  R_Call(glCompressedTexImage2D,
         target,
         0,
         GL_COMPRESSED_RGBA_ASTC_4x4_KHR,
         xsize,
         ysize,
         0,
         readsize,
         ptr);

  return MAKE_TEX_SIZE(xsize, ysize);
}
