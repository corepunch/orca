// clang-format off
#include <stdio.h>
#include <jpeglib.h>
// clang-format on

#include "../r_local.h"

struct jpeg_imageinfo
{
  uint32_t width;
  uint32_t height;
  int channels;
  int size;
  int type;
  byte_t* data;
};

static struct jpeg_imageinfo
jpeg_readimage(struct WI_Buffer* buf)
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_mem_src(&cinfo, buf->data, buf->cursize);
  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);
  struct jpeg_imageinfo image = {
    .width = cinfo.output_width,
    .height = cinfo.output_height,
    .channels = cinfo.num_components,
    .size = cinfo.output_width * cinfo.output_height * cinfo.num_components,
    .type = cinfo.num_components == 4 ? GL_RGBA : GL_RGB,
    .data = ZeroAlloc(cinfo.output_width * cinfo.output_height *
                      cinfo.num_components),
  };
  byte_t* p1 = image.data + (image.height - 1) * image.channels * image.width;
  byte_t** p2 = &p1;
  while (cinfo.output_scanline < image.height) {
    unsigned long const numlines = jpeg_read_scanlines(&cinfo, p2, 1);
    *p2 -= numlines * image.channels * image.width;
  }
  jpeg_finish_decompress(&cinfo);
  return image;
}

static void
jpeg_releaseimage(struct jpeg_imageinfo const* image)
{
  free(image->data);
}

struct WI_Size
R_TexImageJPEG(GLenum target, struct WI_Buffer* rgb)
{
  struct jpeg_imageinfo const image = jpeg_readimage(rgb);

  R_Call(glPixelStorei, GL_UNPACK_ALIGNMENT, image.type != GL_RGBA ? 1 : 4);
  R_Call(glTexImage2D,
         target,
         0,
         GL_RGBA,
         image.width,
         image.height,
         0,
         image.type,
         GL_UNSIGNED_BYTE,
         image.data);

  jpeg_releaseimage(&image);

  return MAKE_TEX_SIZE(image.width, image.height);
}

struct WI_Size
R_TexImageJPEGwithAlpha(GLenum target,
                        struct WI_Buffer* buf_rgb,
                        struct WI_Buffer* buf_alpha,
                        bool_t premultiply_alpha)
{
  struct jpeg_imageinfo const rgb = jpeg_readimage(buf_rgb);
  struct jpeg_imageinfo const alpha = jpeg_readimage(buf_alpha);

  if (rgb.width == alpha.width && rgb.height == alpha.height) {
    struct color32* data = ZeroAlloc(rgb.width * rgb.height);

    FOR_LOOP(index, rgb.width * rgb.height)
    {
      byte_t* _rgb = rgb.data + index * rgb.channels;
      byte_t _alpha = *(alpha.data + index * alpha.channels);
      if (premultiply_alpha) {
        data[index].r = _rgb[0] * _alpha / 255;
        data[index].g = _rgb[1] * _alpha / 255;
        data[index].b = _rgb[2] * _alpha / 255;
        data[index].a = _alpha;
      } else {
        data[index].r = _rgb[0];
        data[index].g = _rgb[1];
        data[index].b = _rgb[2];
        data[index].a = _alpha;
      }
    }

    R_Call(glPixelStorei, GL_UNPACK_ALIGNMENT, 4);
    R_Call(glTexImage2D,
           target,
           0,
           GL_RGBA,
           rgb.width,
           rgb.height,
           0,
           GL_RGBA,
           GL_UNSIGNED_BYTE,
           data);

    free(data);
  } else {
    R_Call(glPixelStorei, GL_UNPACK_ALIGNMENT, rgb.type != GL_RGBA ? 1 : 4);
    R_Call(glTexImage2D,
           target,
           0,
           GL_RGBA,
           rgb.width,
           rgb.height,
           0,
           rgb.type,
           GL_UNSIGNED_BYTE,
           rgb.data);
  }

  jpeg_releaseimage(&rgb);
  jpeg_releaseimage(&alpha);

  return MAKE_TEX_SIZE(rgb.width, rgb.height);
}
