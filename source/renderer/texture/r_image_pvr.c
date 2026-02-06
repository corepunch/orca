#include "../r_local.h"

#define PVR_ERROR(...) Con_Error("PVR: " __VA_ARGS__)

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3

struct pvr_header
{
  int version;
  int flags;
  int format;
  int unused1;
  int colorSpace;
  int channelType;
  uint32_t height;
  uint32_t width;
  int depth;
  int numSufraces;
  int numFaces;
  int numMipMaps;
  int metadataSize;
};

struct WI_Size
R_TexImagePVR(GLenum target, struct WI_Buffer* sb)
{
  struct pvr_header const* header = (void const*)sb->data;
  if (header->version != PVR_HEADER) {
    PVR_ERROR("Not an PVR file\n");
    return MAKE_TEX_SIZE(0, 0);
  }

  int format, blockSize;

  switch (header->format) {
    case 7: // DXT1
      format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
      blockSize = 8;
      break;
    case 9: // DXT3
      format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
      blockSize = 16;
      break;
    case 11: // DXT5
      format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
      blockSize = 16;
      break;
    default:
      PVR_ERROR("PVR format %d unsupported\n", header->format);
      return MAKE_TEX_SIZE(0, 0);
  }

  R_Call(glTexParameteri, target, GL_TEXTURE_BASE_LEVEL, 0);
  R_Call(glTexParameteri,
         target,
         GL_TEXTURE_MAX_LEVEL,
         header->numMipMaps - 1); // opengl likes array length of mipmaps
  R_Call(glTexParameteri, target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  R_Call(glTexParameteri,
         target,
         GL_TEXTURE_MIN_FILTER,
         GL_LINEAR_MIPMAP_LINEAR); // don't forget to enable mipmaping
  R_Call(glTexParameteri, target, GL_TEXTURE_WRAP_S, GL_REPEAT);
  R_Call(glTexParameteri, target, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // prepare some variables
  uint32_t offset = 0;
  uint32_t size = 0;
  uint32_t w = header->width;
  uint32_t h = header->height;
  uint32_t mipMapCount = header->numMipMaps;

  // loop through sending block at a time with the magic formula
  // upload to opengl properly, note the offset transverses the pointer
  // assumes each mipmap is 1/2 the size of the previous mipmap
  for (unsigned int i = 0; i < mipMapCount; i++) {
    if (w == 0 || h == 0) { // discard any odd mipmaps 0x1 0x2 resolutions
      mipMapCount--;
      continue;
    }
    size = ((w + 3) / 4) * ((h + 3) / 4) * blockSize;
    R_Call(glCompressedTexImage2D,
           target,
           i,
           format,
           w,
           h,
           0,
           size,
           sb->data + sizeof(struct pvr_header) + header->metadataSize);
    offset += size;
    w /= 2;
    h /= 2;
  }
  // discard any odd mipmaps, ensure a complete texture
  R_Call(glTexParameteri, target, GL_TEXTURE_MAX_LEVEL, mipMapCount - 1);

  return MAKE_TEX_SIZE(header->width, header->height);
}
