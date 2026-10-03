#include "../r_local.h"

#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wcast-qual"
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif
#include "stb_image.h"
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

static bool_t
flip_vertical(byte_t* pixels, int width, int height, int channels)
{
  int stride = width * channels;
  if (height < 2 || stride <= 0) return TRUE;
  byte_t* row = malloc((size_t)stride);
  if (!row) return FALSE;
  for (int y = 0; y < height / 2; y++) {
    byte_t* top = pixels + (size_t)y * stride;
    byte_t* bot = pixels + (size_t)(height - 1 - y) * stride;
    memcpy(row, top, (size_t)stride);
    memcpy(top, bot, (size_t)stride);
    memcpy(bot, row, (size_t)stride);
  }
  free(row);
  return TRUE;
}

static void
premultiply_rgba(byte_t* pixels, int count)
{
  FOR_LOOP(index, count)
  {
    struct color32* color = &((struct color32*)pixels)[index];
    color->r = color->r * color->a / 255;
    color->g = color->g * color->a / 255;
    color->b = color->b * color->a / 255;
  }
}

static void
upload_tex(GLenum target, GLenum format, int width, int height, void* pixels)
{
  R_Call(glPixelStorei, GL_UNPACK_ALIGNMENT, format == GL_RGBA ? 4 : 1);
  R_Call(glTexImage2D,
         target,
         0,
#ifdef R_USE_SRGB
         format == GL_RGBA ? GL_SRGB8_ALPHA8 : GL_SRGB8,
#else
         format,
#endif
         width,
         height,
         0,
         format,
         GL_UNSIGNED_BYTE,
         pixels);
}

static byte_t*
decode_image(struct AXbuffer* sb, int req, int* width, int* height)
{
  int n = 0;
  return stbi_load_from_memory(sb->data, sb->cursize, width, height, &n, req);
}

static struct AXsize
decode_failed(char const* kind)
{
  char const* reason = stbi_failure_reason();
  return Con_Error("%s: %s", kind, reason ? reason : "decode failed"),
         MAKE_TEX_SIZE(0, 0);
}

struct AXsize
R_TexImagePNG(GLenum target, struct AXbuffer* sb, bool_t premultiply_alpha)
{
  int width = 0, height = 0;
  byte_t* image = decode_image(sb, 4, &width, &height);
  if (!image) return decode_failed("PNG");
  if (target == GL_TEXTURE_2D && !flip_vertical(image, width, height, 4)) {
    stbi_image_free(image);
    return Con_Error("PNG: out of memory"), MAKE_TEX_SIZE(0, 0);
  }

  if (premultiply_alpha) premultiply_rgba(image, width * height);
  upload_tex(target, GL_RGBA, width, height, image);
  stbi_image_free(image);
  return MAKE_TEX_SIZE(width, height);
}

struct AXsize
R_TexImageJPEG(GLenum target, struct AXbuffer* rgb)
{
  int width = 0, height = 0;
  byte_t* image = decode_image(rgb, 3, &width, &height);
  if (!image) return decode_failed("JPEG");
  if (!flip_vertical(image, width, height, 3)) {
    stbi_image_free(image);
    return Con_Error("JPEG: out of memory"), MAKE_TEX_SIZE(0, 0);
  }

  upload_tex(target, GL_RGB, width, height, image);
  stbi_image_free(image);
  return MAKE_TEX_SIZE(width, height);
}

struct AXsize
R_TexImageJPEGwithAlpha(GLenum target,
                        struct AXbuffer* buf_rgb,
                        struct AXbuffer* buf_alpha,
                        bool_t premultiply_alpha)
{
  int width = 0, height = 0, aw = 0, ah = 0;
  byte_t* rgb = decode_image(buf_rgb, 3, &width, &height);
  byte_t* alpha = decode_image(buf_alpha, 1, &aw, &ah);
  if (!rgb || !alpha) {
    stbi_image_free(rgb);
    stbi_image_free(alpha);
    return decode_failed("JPEG");
  }
  if (!flip_vertical(rgb, width, height, 3) || !flip_vertical(alpha, aw, ah, 1)) {
    stbi_image_free(rgb);
    stbi_image_free(alpha);
    return Con_Error("JPEG: out of memory"), MAKE_TEX_SIZE(0, 0);
  }

  if (width == aw && height == ah) {
    struct color32* data = ZeroAlloc((size_t)width * height * sizeof *data);
    FOR_LOOP(index, width * height)
    {
      byte_t* pix = rgb + index * 3;
      byte_t a = alpha[index];
      if (premultiply_alpha) {
        data[index].r = pix[0] * a / 255;
        data[index].g = pix[1] * a / 255;
        data[index].b = pix[2] * a / 255;
      } else {
        data[index].r = pix[0];
        data[index].g = pix[1];
        data[index].b = pix[2];
      }
      data[index].a = a;
    }
    upload_tex(target, GL_RGBA, width, height, data);
    free(data);
  } else {
    upload_tex(target, GL_RGB, width, height, rgb);
  }

  stbi_image_free(rgb);
  stbi_image_free(alpha);
  return MAKE_TEX_SIZE(width, height);
}
