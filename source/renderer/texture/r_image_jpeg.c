#include <stdio.h>
#include <setjmp.h>
#include <limits.h>
#include <jpeglib.h>

#include "../r_local.h"

struct jpeg_imageinfo
{
  int width, height;
  byte_t* data;
};

struct orca_jpeg_error
{
  struct jpeg_error_mgr base;
  jmp_buf jump;
};

struct orca_jpeg_decoder
{
  struct jpeg_decompress_struct codec;
  struct orca_jpeg_error error;
  byte_t* pixels;
  bool_t created;
};

static void
orca_jpeg_error_exit(j_common_ptr codec)
{
  struct orca_jpeg_error* error = (struct orca_jpeg_error*)codec->err;
  longjmp(error->jump, 1);
}

static bool_t
orca_jpeg_dimensions(unsigned int width, unsigned int height)
{
  return width && height && width <= INT_MAX && height <= INT_MAX &&
         width <= (unsigned int)(INT_MAX / 4) / height;
}

/* The heap-owned state remains valid across a libjpeg longjmp. */
static struct jpeg_imageinfo
jpeg_readimage(struct AXbuffer* buf, bool_t size_only)
{
  struct jpeg_imageinfo image = {0};
  if (!buf || !buf->data || buf->cursize <= 0) return image;
  struct orca_jpeg_decoder* decoder = calloc(1, sizeof(*decoder));
  if (!decoder) return image;
  struct jpeg_decompress_struct* codec = &decoder->codec;
  codec->err = jpeg_std_error(&decoder->error.base);
  decoder->error.base.error_exit = orca_jpeg_error_exit;
  if (setjmp(decoder->error.jump)) goto done;
  jpeg_create_decompress(codec);
  decoder->created = TRUE;
  jpeg_mem_src(codec, buf->data, (unsigned long)buf->cursize);
  if (jpeg_read_header(codec, TRUE) != JPEG_HEADER_OK ||
      !orca_jpeg_dimensions(codec->image_width, codec->image_height)) goto done;
  if (size_only) {
    image.width = codec->image_width;
    image.height = codec->image_height;
    goto done;
  }

  bool_t cmyk = codec->jpeg_color_space == JCS_CMYK || codec->jpeg_color_space == JCS_YCCK;
  codec->out_color_space = cmyk ? JCS_CMYK : JCS_RGB;
  jpeg_start_decompress(codec);
  if (!orca_jpeg_dimensions(codec->output_width, codec->output_height)) goto done;
  size_t stride = (size_t)codec->output_width * 3;
  decoder->pixels = malloc(stride * codec->output_height);
  if (!decoder->pixels) goto done;
  JSAMPARRAY row = (*codec->mem->alloc_sarray)((j_common_ptr)codec, JPOOL_IMAGE,
                                               codec->output_width * codec->output_components, 1);
  while (codec->output_scanline < codec->output_height) {
    unsigned int y = codec->output_scanline;
    if (jpeg_read_scanlines(codec, row, 1) != 1) goto done;
    byte_t* dest = decoder->pixels + (codec->output_height - 1 - y) * stride;
    if (!cmyk) memcpy(dest, row[0], stride);
    else for (unsigned int x = 0; x < codec->output_width; x++) {
      byte_t* src = row[0] + x * 4;
      unsigned int k = codec->saw_Adobe_marker ? src[3] : 255 - src[3];
      for (int c = 0; c < 3; c++)
        dest[x * 3 + c] = (codec->saw_Adobe_marker ? src[c] : 255 - src[c]) * k / 255;
    }
  }
  jpeg_finish_decompress(codec);
  if (decoder->error.base.num_warnings) goto done;
  image.width = codec->output_width;
  image.height = codec->output_height;
  image.data = decoder->pixels;
  decoder->pixels = NULL;

done:
  if (decoder->created) jpeg_destroy_decompress(codec);
  free(decoder->pixels);
  free(decoder);
  return image;
}

/* Bottom-up RGB pixels for texture upload; caller frees the result. */
byte_t*
R_DecodeJPEG_RGB(struct AXbuffer* buf, int* width, int* height)
{
  struct jpeg_imageinfo image = jpeg_readimage(buf, FALSE);
  *width = image.width;
  *height = image.height;
  return image.data;
}

static void
jpeg_upload(GLenum target, struct jpeg_imageinfo const* image)
{
  R_Call(glPixelStorei, GL_UNPACK_ALIGNMENT, 1);
  R_Call(glTexImage2D, target, 0,
#ifdef R_USE_SRGB
         GL_SRGB8,
#else
         GL_RGB,
#endif
         image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
}

struct AXsize
R_TexImageJPEG(GLenum target, struct AXbuffer* rgb)
{
  struct jpeg_imageinfo image = {0};
  image.data = R_DecodeJPEG_RGB(rgb, &image.width, &image.height);
  if (!image.data) return MAKE_TEX_SIZE(0, 0);
  jpeg_upload(target, &image);
  free(image.data);
  return MAKE_TEX_SIZE(image.width, image.height);
}

struct AXsize
R_TexImageJPEGwithAlpha(GLenum target, struct AXbuffer* buf_rgb,
                        struct AXbuffer* buf_alpha, bool_t premultiply_alpha)
{
  struct jpeg_imageinfo rgb = jpeg_readimage(buf_rgb, FALSE);
  struct jpeg_imageinfo alpha = jpeg_readimage(buf_alpha, FALSE);
  if (!rgb.data || !alpha.data) {
    free(rgb.data);
    free(alpha.data);
    return MAKE_TEX_SIZE(0, 0);
  }
  if (rgb.width == alpha.width && rgb.height == alpha.height) {
    size_t count = (size_t)rgb.width * rgb.height;
    struct color32* data = malloc(count * sizeof(*data));
    if (!data) {
      free(rgb.data);
      free(alpha.data);
      return MAKE_TEX_SIZE(0, 0);
    }
    for (size_t i = 0; i < count; i++) {
      byte_t a = alpha.data[i * 3];
      data[i].r = premultiply_alpha ? rgb.data[i * 3] * a / 255 : rgb.data[i * 3];
      data[i].g = premultiply_alpha ? rgb.data[i * 3 + 1] * a / 255 : rgb.data[i * 3 + 1];
      data[i].b = premultiply_alpha ? rgb.data[i * 3 + 2] * a / 255 : rgb.data[i * 3 + 2];
      data[i].a = a;
    }
    R_Call(glPixelStorei, GL_UNPACK_ALIGNMENT, 4);
    R_Call(glTexImage2D, target, 0,
#ifdef R_USE_SRGB
           GL_SRGB8_ALPHA8,
#else
           GL_RGBA,
#endif
           rgb.width, rgb.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    free(data);
  } else jpeg_upload(target, &rgb);
  free(rgb.data);
  free(alpha.data);
  return MAKE_TEX_SIZE(rgb.width, rgb.height);
}

struct AXsize
R_JPEGSize(struct AXbuffer* buf)
{
  struct jpeg_imageinfo image = jpeg_readimage(buf, TRUE);
  return MAKE_TEX_SIZE(image.width, image.height);
}
