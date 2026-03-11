#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <source/UIKit/UIKit.h>


static struct vec2
_GetImageSize(lpObject_t hObject, ImageViewPtr imageView)
{
  struct vec2 size = { 0 };
  if (imageView->Image) {
    struct image_info img;
    if (SUCCEEDED(Image_GetInfo(imageView->Image, &img))) {
      size.x = img.bmWidth * imageView->Viewbox.z;
      size.y = img.bmHeight * imageView->Viewbox.w;
    }
  }
  return size;
}

HANDLER(ImageView, MeasureOverride)
{
  if (pImageView->Image) {
    vec2_t size = _GetImageSize(hObject, pImageView);
    lpcedges_t e = (struct edges const*)&pImageView->Insets;
//    vec2_t calcsize = {
//      fmin(size.x - e->left - e->right, pMeasureOverride->width),
//      fmin(size.y - e->top - e->bottom, pMeasureOverride->height),
//    };
    if (pImageView->Stretch == kStretchNone) {
      return MAKEDWORD(size.x - e->left - e->right, size.y - e->top - e->bottom);
    } else if (pImageView->Stretch == kStretchUniform) {
      rect_t avail = {0, 0, pMeasureOverride->width, pMeasureOverride->height};
      rect_t final = RECT_Fit(&avail, &size);
      return MAKEDWORD(final.width, final.height);
    } else {
      return MAKEDWORD(pMeasureOverride->width, pMeasureOverride->height);
    }
  } else {
    return MAKEDWORD(pMeasureOverride->width, pMeasureOverride->height);
  }
}

HANDLER(ImageView, ArrangeOverride) {
  return OBJ_SendMessageW(hObject, kEventMeasureOverride, 0, &(Size_t){
    pArrangeOverride->width, pArrangeOverride->height
  });
}

#if 0
typedef struct
{
  bool_t value;
  bool_t anyImagesNeeded;
  lpProperty_t prop;
} checkimg;

static void
check_images(struct _SHADERCONST *u, void* pVoid)
{
  checkimg* pParm = pVoid;
  if (u->type != UT_SAMPLER_2D)
    return;
  pParm->anyImagesNeeded = TRUE;
  lpProperty_t p = PROP_FindByLongID(pParm->prop, u->identifier);
  if (!p)
    return;
  pParm->value = TRUE;
}
#endif

HANDLER(ImageView, DrawBrush)
{
  Node2DPtr pNode2D = GetNode2D(hObject);
  uint32_t width = Node2D_GetFrame(pNode2D, kBox3FieldWidth);
  uint32_t height = Node2D_GetFrame(pNode2D, kBox3FieldHeight);
  struct vec2 imgsize = _GetImageSize(hObject, pImageView);
  struct ViewEntity entity;

	if (!memcmp(&pDrawBrush->brush,
							&(struct BrushShorthand){0},
							sizeof(struct BrushShorthand)) &&
			!pDrawBrush->foreground)
		return FALSE;

#if 0
  if (!pImageView->Image) {
    checkimg hasImages = {
      .value = FALSE,
      .prop = OBJ_GetProperties(hObject),
    };
    struct Material* pMaterial = GetMaterial(pDrawBrush->brush.Material);
    if (!pMaterial) {
      return FALSE;
    }
    Shader_EnumUniforms(pMaterial->Shader, check_images, &hasImages);
    if (!hasImages.value && hasImages.anyImagesNeeded) {
      return FALSE;
    }
    imgsize.x = width;
    imgsize.y = height;
  }
#endif

  lpTexture_t img = pDrawBrush->foreground ? pImageView->Image : NULL;
  Node2D_GetViewEntity(pNode2D, &entity, img, &pDrawBrush->brush);
  
  calculate_ninepatch(&(vec2_t){ width, height },
                      &imgsize,
                      (lpcedges_t)&pImageView->Insets,
                      (lpcedges_t)&pImageView->Edges,
                      (lpcrect_t)&pImageView->Viewbox,
                      &entity.ninepatch);

  entity.mesh = BOX_PTR(Mesh, MD_NINEPATCH);
  entity.bbox = BOX3_FromRect(GetNode2D(hObject)->_rect);
  entity.material.blendMode = OBJ_GetInteger(hObject, ID_Material_BlendMode, kBlendModeAlpha);

  if (pImageView->Stretch == kStretchUniform) {
    struct rect temp_rect = {
      entity.bbox.min.x,
      entity.bbox.min.y,
      entity.bbox.max.x - entity.bbox.min.x,
      entity.bbox.max.y - entity.bbox.min.y
    };
    temp_rect = RECT_Fit(&temp_rect, &imgsize);
    entity.bbox = BOX3_FromRect(temp_rect);
    entity.mesh = BOX_PTR(Mesh, MD_RECTANGLE);
  }
  
  R_DrawEntity(pDrawBrush->viewdef, &entity);

  return TRUE;
}

HANDLER(ImageView, ForegroundContent)
{
//    if (imageView->image) {
//        pForegroundContent->result = imageView->image;
//        return TRUE;
//    } else {
//        pForegroundContent->result = imageView->rendition;
//        return TRUE;
//    }
  pForegroundContent->result = pImageView->Image;
  return pForegroundContent->result != 0;
}

HANDLER(ImageView, LoadView)
{
  lua_State* L = pLoadView->lua_state;
  if (!(OBJ_GetFlags(hObject) & OF_ACTIVATED)) {
    return TRUE;
  }
  if (*pImageView->Src &&
      pImageView->_loadedImage != fnv1a32(pImageView->Src)) {
    lua_pushstring(L, pImageView->Src);
    luaX_executecallback(L, hObject, "loadImageAsync", 1);
    pImageView->_loadedImage = fnv1a32(pImageView->Src);
  }
  return TRUE;
}
