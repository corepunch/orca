#define STBI_FAILURE_USERMSG
#define STBI_NO_STDIO
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <limits.h>

#include "../r_local.h"

static byte_t*
image_decode(struct AXbuffer* buffer,
             int channels,
             int* width,
             int* height)
{
  if (buffer->cursize > INT_MAX) {
    fprintf(stderr, "Could not decode image: input is too large\n");
    return NULL;
  }
  byte_t* data = stbi_load_from_memory(buffer->data,
                                      (int)buffer->cursize,
                                      width,
                                      height,
                                      NULL,
                                      channels);
  if (!data) {
    fprintf(stderr, "Could not decode image: %s\n", stbi_failure_reason());
  }
  return data;
}

static void
image_flip_rows(byte_t* data, int width, int height, int channels)
{
  size_t const row_size = (size_t)width * channels;
  for (int top = 0, bottom = height - 1; top < bottom; ++top, --bottom) {
    byte_t* top_row = data + (size_t)top * row_size;
    byte_t* bottom_row = data + (size_t)bottom * row_size;
    for (size_t offset = 0; offset < row_size; ++offset) {
      byte_t const value = top_row[offset];
      top_row[offset] = bottom_row[offset];
      bottom_row[offset] = value;
    }
  }
}

struct AXsize
R_TexImagePNG(GLenum target, struct AXbuffer* buffer, bool_t premultiply_alpha)
{
  int width;
  int height;
  byte_t* data = image_decode(buffer, STBI_rgb_alpha, &width, &height);
  if (!data) {
    return MAKE_TEX_SIZE(0, 0);
  }

  if (target == GL_TEXTURE_2D) {
    image_flip_rows(data, width, height, STBI_rgb_alpha);
  }
  if (premultiply_alpha) {
    FOR_LOOP(index, (size_t)width * height)
    {
      struct color32* color = &((struct color32*)data)[index];
      color->r = color->r * color->a / 255;
      color->g = color->g * color->a / 255;
      color->b = color->b * color->a / 255;
    }
  }

  R_Call(glPixelStorei, GL_UNPACK_ALIGNMENT, 4);
  R_Call(glTexImage2D,
         target,
         0,
#ifdef R_USE_SRGB
         GL_SRGB8_ALPHA8,
#else
         GL_RGBA,
#endif
         width,
         height,
         0,
         GL_RGBA,
         GL_UNSIGNED_BYTE,
         data);

  stbi_image_free(data);
  return MAKE_TEX_SIZE(width, height);
}

struct AXsize
R_TexImageJPEG(GLenum target, struct AXbuffer* buffer)
{
  int width;
  int height;
  byte_t* data = image_decode(buffer, STBI_rgb, &width, &height);
  if (!data) {
    return MAKE_TEX_SIZE(0, 0);
  }

  image_flip_rows(data, width, height, STBI_rgb);
  R_Call(glPixelStorei, GL_UNPACK_ALIGNMENT, 1);
  R_Call(glTexImage2D,
         target,
         0,
#ifdef R_USE_SRGB
         GL_SRGB8,
#else
         GL_RGB,
#endif
         width,
         height,
         0,
         GL_RGB,
         GL_UNSIGNED_BYTE,
         data);

  stbi_image_free(data);
  return MAKE_TEX_SIZE(width, height);
}