#include <limits.h>
#include <png.h>

#include "../r_local.h"

#define PNG_ERROR(...) Con_Error("PNG: " __VA_ARGS__)

#define PNGU_SIG_BYTES 8

static void
png_readcallback(png_struct* ps, png_byte* data, png_size_t length)
{
  struct AXbuffer* sb = png_get_io_ptr(ps);
  if (sb->readcount < 0 || sb->readcount > sb->cursize ||
      length > (size_t)(sb->cursize - sb->readcount)) {
    png_error(ps, "Read Error!");
  } else {
    memcpy(data, sb->data + sb->readcount, length);
    sb->readcount += length;
  }
}

static bool_t
png_open(struct AXbuffer* sb, png_structp* ps, png_infop* info)
{
  if (!sb || !sb->data || sb->cursize < PNGU_SIG_BYTES ||
      png_sig_cmp(sb->data, 0, PNGU_SIG_BYTES)) {
    PNG_ERROR("File is not a PNG.");
    return FALSE;
  }
  *ps = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  *info = *ps ? png_create_info_struct(*ps) : NULL;
  if (!*info) {
    png_destroy_read_struct(ps, NULL, NULL);
    PNG_ERROR("Unable to allocate the decoder.");
    return FALSE;
  }
  sb->readcount = 0;
  png_set_read_fn(*ps, sb, png_readcallback);
  return TRUE;
}

/* Decodes to 8-bit RGBA. Rows are stored bottom-up when flip_rows is set,
 * which is the order GL_TEXTURE_2D expects. Caller frees with free(). */
static byte_t*
png_decode(struct AXbuffer* sb, bool_t flip_rows, int* width, int* height)
{
  png_structp ps = NULL;
  png_infop info = NULL;
  /* Modified after setjmp and read on the error path, so they must be volatile. */
  png_bytep* volatile rows = NULL;
  byte_t* volatile image = NULL;
  uint32_t w = 0, h = 0;

  *width = *height = 0;
  if (!png_open(sb, &ps, &info)) return NULL;
  if (setjmp(png_jmpbuf(ps))) {
    PNG_ERROR("Unable to decode image.");
    goto done;
  }

  png_read_info(ps, info);

  w = png_get_image_width(ps, info);
  h = png_get_image_height(ps, info);
  int color_type = png_get_color_type(ps, info);
  int bit_depth = png_get_bit_depth(ps, info);

  if (!w || !h || w > INT_MAX || h > INT_MAX || w > (uint32_t)(INT_MAX / 4) / h)
    goto done;
  if (bit_depth == 16) png_set_strip_16(ps);
  bool_t has_transparency = png_get_valid(ps, info, PNG_INFO_tRNS) != 0;
  if (has_transparency) png_set_tRNS_to_alpha(ps);

  switch (color_type) {
    case PNG_COLOR_TYPE_PALETTE:
      png_set_palette_to_rgb(ps);
      if (!has_transparency) png_set_filler(ps, 0xFF, PNG_FILLER_AFTER);
      break;
    case PNG_COLOR_TYPE_GRAY:
      png_set_expand_gray_1_2_4_to_8(ps);
      if (!has_transparency) png_set_filler(ps, 0xFF, PNG_FILLER_AFTER);
      png_set_gray_to_rgb(ps);
      break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      png_set_gray_to_rgb(ps);
      break;
    case PNG_COLOR_TYPE_RGB:
      if (!has_transparency) png_set_filler(ps, 0xFF, PNG_FILLER_AFTER);
      break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
      break;
  }

  png_set_option(ps, PNG_SKIP_sRGB_CHECK_PROFILE, PNG_OPTION_ON);
  png_read_update_info(ps, info);

  size_t rowbytes = png_get_rowbytes(ps, info);
  if (rowbytes != (size_t)w * 4 || rowbytes > SIZE_MAX / h ||
      (size_t)h > SIZE_MAX / sizeof(png_bytep)) goto done;
  image = malloc(rowbytes * h);
  rows = malloc(h * sizeof(png_bytep));
  if (!image || !rows) {
    PNG_ERROR("Out of memory.");
    goto done;
  }
  // rows[i] is where libpng puts file row i; flipping reverses the destination.
  for (uint32_t i = 0; i < h; i++) {
    rows[i] = image + (size_t)(flip_rows ? h - 1 - i : i) * rowbytes;
  }
  png_read_image(ps, (png_bytep*)rows);
  png_read_end(ps, NULL);
  *width = (int)w;
  *height = (int)h;

done:
  png_destroy_read_struct(&ps, &info, NULL);
  free((png_bytep*)rows);
  if (!*width) {
    free((byte_t*)image);
    return NULL;
  }
  return (byte_t*)image;
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

/* Premultiplied RGBA rows, top row first; release with R_FreeImage(). */
byte_t*
R_DecodeImageRGBA(void const* data, uint32_t size, int* width, int* height)
{
  struct AXbuffer sb = { .data = (void*)data, .cursize = size };
  byte_t* image = png_decode(&sb, FALSE, width, height);
  if (image) premultiply_rgba(image, *width * *height);
  return image;
}

void
R_FreeImage(byte_t* image)
{
  free(image);
}

struct AXsize
R_PNGSize(struct AXbuffer* sb)
{
  png_structp ps = NULL;
  png_infop info = NULL;
  if (!png_open(sb, &ps, &info)) return MAKE_TEX_SIZE(0, 0);
  struct AXsize size = MAKE_TEX_SIZE(0, 0);
  if (!setjmp(png_jmpbuf(ps))) {
    png_read_info(ps, info);
    uint32_t w = png_get_image_width(ps, info), h = png_get_image_height(ps, info);
    if (w && h && w <= (uint32_t)(INT_MAX / 4) / h)
      size = MAKE_TEX_SIZE(w, h);
  }
  png_destroy_read_struct(&ps, &info, NULL);
  return size;
}

struct AXsize
R_TexImagePNG(GLenum target, struct AXbuffer* sb, bool_t premultiply_alpha)
{
  int width = 0, height = 0;
  byte_t* image = png_decode(sb, target == GL_TEXTURE_2D, &width, &height);
  if (!image) return MAKE_TEX_SIZE(0, 0);

  if (premultiply_alpha) premultiply_rgba(image, width * height);

  R_Call(glPixelStorei, GL_UNPACK_ALIGNMENT, 4);
  R_Call(glTexImage2D,target,0,
#ifdef R_USE_SRGB
         GL_SRGB8_ALPHA8,
#else
         GL_RGBA,
#endif
         width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,image);

  free(image);
  return MAKE_TEX_SIZE(width, height);
}
