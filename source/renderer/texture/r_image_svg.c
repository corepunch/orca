#include "../r_local.h"

#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#define SVG_ERROR(...) Con_Error("SVG: " __VA_ARGS__)

#include "nanosvg.h"
#include "nanosvgrast.h"

struct WI_Size
R_TexImageSVG(GLenum target,
              struct WI_Buffer* sb,
              uint32_t backingScale,
              struct Texture* pImage,
              bool_t is_mask)
{
  // Create a null-terminated string from the SVG data.
  LPSTR svg_data = malloc(sb->cursize + 1);
  memcpy(svg_data, sb->data, sb->cursize);
  svg_data[sb->cursize] = '\0';

  // Parse the SVG data using NanoSVG.
  struct NSVGimage* image = nsvgParse(svg_data, "px", 96.0f);
  free(svg_data);

  if (!image) {
    SVG_ERROR("Failed to parse SVG.");
    return MAKE_TEX_SIZE(0, 0);
  }

  // Get the dimensions of the SVG.
  int width = (int)image->width;
  int height = (int)image->height;

  if (pImage && pImage->Width > 1) {
    float aspect = image->height / image->width;
    pImage->Scale = backingScale;
    width = pImage->Width * pImage->Scale;
    height = pImage->Width * pImage->Scale * aspect;
  }

  // Allocate a buffer for the rasterized image.
  uint32_t* data = malloc(width * height * 4); // RGBA
  if (!data) {
    nsvgDelete(image);
    SVG_ERROR("Memory allocation failed.");
    return MAKE_TEX_SIZE(0, 0);
  }

  // Rasterize the SVG to the buffer.
  struct NSVGrasterizer* rasterizer = nsvgCreateRasterizer();
  if (!rasterizer) {
    free(data);
    nsvgDelete(image);
    SVG_ERROR("Failed to create rasterizer.");
    return MAKE_TEX_SIZE(0, 0);
  }

  nsvgRasterize(rasterizer,image,0,0,width / image->width,(byte_t*)data,width,height,width * 4);
  nsvgDeleteRasterizer(rasterizer);

  // Flip the image vertically.
  for (int y = 0; y < height / 2; y++) {
    int const other = height - y - 1;
    for (int x = 0; x < width; x++) {
      uint32_t temp = data[y * width + x];
      data[y * width + x] = data[other * width + x];
      data[other * width + x] = temp;
    }
  }

  if (is_mask) {
    for (int i = 0; i < width * height; i++) {
      data[i] |= 0x00ffffff;
    }
  }

  // Upload the texture to OpenGL.
  R_Call(glTexImage2D,target,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,data);

  // Clean up.
  free(data);
  nsvgDelete(image);

  return MAKE_TEX_SIZE(width, height);
}
