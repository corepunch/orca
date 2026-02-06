#include <libpng16/png.h>

#include "../r_local.h"

#define PNG_ERROR(...) Con_Error("PNG: " __VA_ARGS__)

#define PNGU_SIG_BYTES 8

static void
png_readcallback(png_struct* ps, png_byte* data, png_size_t length)
{
  struct WI_Buffer* sb = png_get_io_ptr(ps);
  if (sb->readcount + length > sb->cursize) {
    png_error(ps, "Read Error!");
  } else {
    memcpy(data, sb->data + sb->readcount, length);
    sb->readcount += length;
  }
}

//bool_t print_png_color_info(png_structp png_ptr, png_infop info_ptr) {
//  // sRGB chunk
//  int srgb_intent;
//  if (png_get_sRGB(png_ptr, info_ptr, &srgb_intent)) {
//    lpcString_t intents[] = {"Perceptual", "Relative colorimetric", "Saturation", "Absolute colorimetric"};
//    Con_Error("sRGB chunk found. Rendering intent: %s (%d)",
//           srgb_intent >= 0 && srgb_intent < 4 ? intents[srgb_intent] : "Unknown",
//           srgb_intent);
//  }
//  
//  // gAMA chunk (gamma)
//  double gamma;
//  if (png_get_gAMA(png_ptr, info_ptr, &gamma)) {
//    Con_Error("gAMA chunk found. Gamma: %f (display exponent: %f)", gamma, 1.0 / gamma);
//  }
//  
//  // cHRM chunk (chromaticities)
//  double wx, wy, rx, ry, gx, gy, bx, by;
//  if (png_get_cHRM(png_ptr, info_ptr, &wx, &wy, &rx, &ry, &gx, &gy, &bx, &by)) {
//    Con_Error("cHRM chunk found.");
//    Con_Error("  White point:    x = %f, y = %f", wx, wy);
//    Con_Error("  Red chromaticity:   x = %f, y = %f", rx, ry);
//    Con_Error("  Green chromaticity: x = %f, y = %f", gx, gy);
//    Con_Error("  Blue chromaticity:  x = %f, y = %f", bx, by);
//  }
//  
//  // iCCP chunk (ICC profile)
//  png_charp name;
//  int compression_type;
//  png_bytep profile;
//  png_uint_32 proflen;
//  if (png_get_iCCP(png_ptr, info_ptr, &name, &compression_type, &profile, &proflen)) {
//    Con_Error("iCCP chunk found.");
//    Con_Error("  Profile name: %s", name);
//    Con_Error("  Compression type: %d", compression_type);
//    Con_Error("  Profile size: %u bytes", proflen);
//    
//    // (Optional) Dump beginning of ICC profile
//    Con_Error("  Profile data (first 16 bytes): ");
//    for (png_uint_32 i = 0; i < proflen && i < 16; ++i) {
//      Con_Error("%02X ", profile[i]);
//    }
//    Con_Error("\n");
//  }
//  
//  png_textp text_ptr;
//  int num_text;
//  if (png_get_text(png_ptr, info_ptr, &text_ptr, &num_text)) {
//    for (int i = 0; i < num_text; i++) {
//      if (strstr(text_ptr[i].text, "exif:ColorSpace=\"65535\"")) {
////        Con_Error("tEXt chunk [%s] = %s", text_ptr[i].key, text_ptr[i].text);
//        return FALSE;
//      }
//    }
//  }
//  return TRUE;
//}

struct WI_Size
R_TexImagePNG(GLenum target, struct WI_Buffer* sb, bool_t premultiply_alpha)
{
  if (png_sig_cmp(sb->data, 0, 8)) {
    PNG_ERROR("File is not a PNG.");
    return MAKE_TEX_SIZE(0, 0);
  }

  png_structp png_ptr =
    png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr);
  png_infop end_info = png_create_info_struct(png_ptr);

  setjmp(png_jmpbuf(png_ptr));

  png_set_read_fn(png_ptr, sb, png_readcallback);
  png_read_info(png_ptr, info_ptr);
  
//  bool_t is_sRGB = print_png_color_info(png_ptr, info_ptr);

  // get info about png
  uint32_t width = png_get_image_width(png_ptr, info_ptr);
  uint32_t height = png_get_image_height(png_ptr, info_ptr);
  int color_type = png_get_color_type(png_ptr, info_ptr);
  int bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  double file_gamma = 0.0;

  if (bit_depth == 16) {
    png_set_strip_16(png_ptr);
  }

  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_gAMA)) {
    png_get_gAMA(png_ptr, info_ptr, &file_gamma);
  }

  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
    png_set_tRNS_to_alpha(png_ptr);
  }

  switch (color_type) {
    case PNG_COLOR_TYPE_PALETTE:
      png_set_palette_to_rgb(png_ptr);
      png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
      break;
    case PNG_COLOR_TYPE_GRAY:
      png_set_expand_gray_1_2_4_to_8(png_ptr);
      png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
      png_set_gray_to_rgb(png_ptr);
      break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      png_set_gray_to_rgb(png_ptr);
      break;
    case PNG_COLOR_TYPE_RGB:
      png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
      break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
      break;
  }

//  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_sRGB)) {
//    png_get_sRGB(png_ptr, info_ptr, &srgb_intent);
//    is_srgb = true;
//    puts("sRGB");
//  } else {
//    puts("no sRGB");
//  }
  
  png_set_option(png_ptr, PNG_SKIP_sRGB_CHECK_PROFILE, PNG_OPTION_ON);
  //    png_set_gAMA(png_ptr, info_ptr, 0.5f);
  //    png_set_sRGB(png_ptr, info_ptr, 0);
  //    if (premultiply_alpha) {
  //        png_set_alpha_mode(png_ptr, PNG_ALPHA_PREMULTIPLIED,
  //        PNG_DEFAULT_sRGB);
  //    }

  // Update the png info struct.
  png_read_update_info(png_ptr, info_ptr);

  size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  size_t image_data_size = rowbytes * height * sizeof(png_byte) + 15;

  png_byte* image_data = ZeroAlloc(image_data_size);
  png_bytep* row_pointers = ZeroAlloc(height * sizeof(png_bytep));

  // set the individual row_pointers to point at the correct offsets of
  // image_data
  if (target == GL_TEXTURE_2D) {
    for (size_t i = 0, j = height - 1; i < height; i++, j--) {
      row_pointers[j] = image_data + i * rowbytes;
    }
  } else {
    for (size_t i = 0; i < height; i++) {
      row_pointers[i] = image_data + i * rowbytes;
    }
  }
  // read the png into image_data through row_pointers
  png_read_image(png_ptr, row_pointers);

  if (premultiply_alpha) {
    FOR_LOOP(index, width * height)
    {
      struct color32* color = &((struct color32*)image_data)[index];
      color->r = color->r * color->a / 255;
      color->g = color->g * color->a / 255;
      color->b = color->b * color->a / 255;
    }
  }

  R_Call(glTexImage2D,target,0,GL_SRGB8_ALPHA8,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,image_data);

  // clean up
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

  free(image_data);
  free(row_pointers);

  return MAKE_TEX_SIZE(width, height);
}
