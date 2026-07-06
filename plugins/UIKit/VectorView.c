#include <UIKit/UIKit.h>

#include <include/api.h>
#include <include/renderer.h>
#include <filesystem/filesystem.h>

#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#define SVG_ERROR(...) Con_Error("VectorView: " __VA_ARGS__)

#include "nanosvg.h"
#include "nanosvgrast.h"

static void
_VectorView_ReleaseTexture(struct VectorView *pVectorView)
{
  if (pVectorView->_texture) {
    Texture_Release(pVectorView->_texture);
    pVectorView->_texture = NULL;
  }
  pVectorView->_width = 0;
  pVectorView->_height = 0;
}

static void
_VectorView_LoadSvg(struct VectorView *pVectorView)
{
  if (!pVectorView->Source || !*pVectorView->Source) return;

  struct file *pFile = FS_LoadFile(pVectorView->Source);
  if (!pFile) {
    SVG_ERROR("can't load '%s'", pVectorView->Source);
    return;
  }

  char *svg_data = malloc(pFile->size + 1);
  memcpy(svg_data, pFile->data, pFile->size);
  svg_data[pFile->size] = '\0';

  struct NSVGimage *image = nsvgParse(svg_data, "px", 96.0f);
  free(svg_data);

  if (!image) {
    SVG_ERROR("failed to parse '%s'", pVectorView->Source);
    FS_FreeFile(pFile);
    return;
  }

  uint32_t scale = axGetScaling();
  int width  = (int)image->width  * scale;
  int height = (int)image->height * scale;

  uint32_t *rgba = malloc(width * height * 4);
  if (!rgba) {
    nsvgDelete(image);
    FS_FreeFile(pFile);
    SVG_ERROR("out of memory");
    return;
  }

  struct NSVGrasterizer *rast = nsvgCreateRasterizer();
  if (!rast) {
    free(rgba);
    nsvgDelete(image);
    FS_FreeFile(pFile);
    SVG_ERROR("rasterizer creation failed");
    return;
  }

  nsvgRasterize(rast, image, 0, 0,
                (float)width / image->width,
                (byte_t *)rgba, width, height, width * 4);
  nsvgDeleteRasterizer(rast);
  nsvgDelete(image);

  // Flip vertically for OpenGL.
  for (int y = 0; y < height / 2; y++) {
    int other = height - y - 1;
    for (int x = 0; x < width; x++) {
      uint32_t tmp = rgba[y * width + x];
      rgba[y * width + x] = rgba[other * width + x];
      rgba[other * width + x] = tmp;
    }
  }

  // Convert to alpha8 mask: extract alpha channel only.
  byte_t *alpha = malloc(width * height);
  for (int i = 0; i < width * height; i++) {
    alpha[i] = rgba[i] >> 24;
  }
  free(rgba);

  // Upload as alpha8 texture.
  CREATEIMGSTRUCT cis = {
    .Width     = width,
    .Height    = height,
    .Format    = kTextureFormatAlpha8,
    .MinFilter = kTextureFilterLinear,
    .MagFilter = kTextureFilterLinear,
    .ImageData = alpha,
  };

  HRESULT hr = Texture_Create(&cis, &pVectorView->_texture);
  free(alpha);

  if (FAILED(hr)) {
    SVG_ERROR("texture creation failed for '%s'", pVectorView->Source);
    FS_FreeFile(pFile);
    return;
  }

  pVectorView->_texture->Scale  = scale;
  pVectorView->_texture->Width  = width  / scale;
  pVectorView->_texture->Height = height / scale;
  pVectorView->_width  = (float)(width  / scale);
  pVectorView->_height = (float)(height / scale);

  FS_FreeFile(pFile);
}

HANDLER(VectorView, Object, Create)
{
  return FALSE;
}

HANDLER(VectorView, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property) return FALSE;
  if (PROP_GetLongIdentifier(pPropertyChanged->Property) == ID_VectorView_Source) {
    _VectorView_ReleaseTexture(pVectorView);
    _VectorView_LoadSvg(pVectorView);
    OBJ_SetDirty(hObject);
  }
  return FALSE;
}

HANDLER(VectorView, Object, Destroy)
{
  _VectorView_ReleaseTexture(pVectorView);
  return FALSE;
}

HANDLER(VectorView, Node2D, MeasureOverride)
{
  if (pVectorView->_texture) {
    struct vec2 size = { pVectorView->_width, pVectorView->_height };
    if (pVectorView->Stretch == kStretchNone || pVectorView->Stretch == kStretchUniform) {
      float avail_w = isinf(pMeasureOverride->Width)  ? size.x : pMeasureOverride->Width;
      float avail_h = isinf(pMeasureOverride->Height) ? size.y : pMeasureOverride->Height;
      struct rect avail = {0, 0, avail_w, avail_h};
      struct rect final = RECT_Fit(&avail, &size);
      return MAKEDWORD(final.width, final.height);
    } else {
      float avail_w = isinf(pMeasureOverride->Width)  ? 0 : pMeasureOverride->Width;
      float avail_h = isinf(pMeasureOverride->Height) ? 0 : pMeasureOverride->Height;
      return MAKEDWORD(avail_w, avail_h);
    }
  } else {
    float w = isinf(pMeasureOverride->Width)  ? 0 : pMeasureOverride->Width;
    float h = isinf(pMeasureOverride->Height) ? 0 : pMeasureOverride->Height;
    return MAKEDWORD(w, h);
  }
}

HANDLER(VectorView, Node2D, ArrangeOverride)
{
  return _SendMessage(hObject, Node2D, MeasureOverride,
                      .Width = pArrangeOverride->Width,
                      .Height = pArrangeOverride->Height);
}

HANDLER(VectorView, Node2D, ForegroundContent)
{
  return (intptr_t)pVectorView->_texture;
}

HANDLER(VectorView, Node2D, DrawForeground)
{
  if (!pVectorView->_texture) return FALSE;

  if (!memcmp(&pDrawForeground->brush,
              &(struct BrushShorthand){0},
              sizeof(struct BrushShorthand)))
    return FALSE;

  struct Node2D *pNode2D = GetNode2D(hObject);
  struct ViewEntity entity;
  struct vec2 imgsize = { pVectorView->_width, pVectorView->_height };
  uint32_t width  = Node2D_GetFrame(pNode2D, kBox3FieldWidth);
  uint32_t height = Node2D_GetFrame(pNode2D, kBox3FieldHeight);

  Node2D_GetViewEntity(pNode2D, &entity, pVectorView->_texture, &pDrawForeground->brush);

  if (pVectorView->Stretch == kStretchUniform) {
    struct rect temp_rect = {
      entity.bbox.min.x,
      entity.bbox.min.y,
      entity.bbox.max.x - entity.bbox.min.x,
      entity.bbox.max.y - entity.bbox.min.y
    };
    temp_rect = RECT_Fit(&temp_rect, &imgsize);
    entity.bbox = BOX3_FromRect(temp_rect);
    entity.mesh = BOX_PTR(Mesh, MD_RECTANGLE);
  } else if (pVectorView->Stretch == kStretchUniformToFill && imgsize.x > 0 && imgsize.y > 0) {
    float scaleX = (float)width  / imgsize.x;
    float scaleY = (float)height / imgsize.y;
    float scale = scaleX > scaleY ? scaleX : scaleY;
    float uvScaleX = scaleX / scale;
    float uvScaleY = scaleY / scale;
    float uvOffsetX = (1.0f - uvScaleX) / 2.0f;
    float uvOffsetY = (1.0f - uvScaleY) / 2.0f;
    entity.material.textureMatrix.v[0] = uvScaleX;
    entity.material.textureMatrix.v[4] = uvScaleY;
    entity.material.textureMatrix.v[6] = uvOffsetX;
    entity.material.textureMatrix.v[7] = uvOffsetY;
    entity.mesh = BOX_PTR(Mesh, MD_RECTANGLE);
  } else {
    entity.mesh = BOX_PTR(Mesh, MD_RECTANGLE);
  }

  entity.bbox = BOX3_FromRect(GetNode2D(hObject)->_rect);

  R_DrawEntity(pDrawForeground->viewdef, &entity);

  return TRUE;
}
