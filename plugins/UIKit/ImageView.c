#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <plugins/UIKit/UIKit.h>


static struct vec2
_GetImageSize(lpObject_t hObject, ImageViewPtr imageView)
{
  struct vec2 size = { 0 };
  if (imageView->Source) {
    struct image_info img;
    if (SUCCEEDED(Image_GetInfo(imageView->Source, &img))) {
      size.x = img.bmWidth * imageView->Viewbox.z;
      size.y = img.bmHeight * imageView->Viewbox.w;
    }
  }
  return size;
}

HANDLER(ImageView, Node2D, MeasureOverride)
{
  if (pImageView->Source) {
    vec2_t size = _GetImageSize(hObject, pImageView);
    lpcedges_t e = (struct edges const*)&pImageView->Insets;
//    vec2_t calcsize = {
//      fmin(size.x - e->left - e->right, pMeasureOverride->Width),
//      fmin(size.y - e->top - e->bottom, pMeasureOverride->Height),
//    };
    if (pImageView->Stretch == kStretchNone) {
      return MAKEDWORD(size.x - e->left - e->right, size.y - e->top - e->bottom);
    } else if (pImageView->Stretch == kStretchUniform) {
      rect_t avail = {0, 0, pMeasureOverride->Width, pMeasureOverride->Height};
      rect_t final = RECT_Fit(&avail, &size);
      return MAKEDWORD(final.width, final.height);
    } else {
      return MAKEDWORD(pMeasureOverride->Width, pMeasureOverride->Height);
    }
  } else {
    return MAKEDWORD(pMeasureOverride->Width, pMeasureOverride->Height);
  }
}

HANDLER(ImageView, Node2D, ArrangeOverride) {
  return _SendMessage(hObject, Node2D, MeasureOverride,
                      .Width = pArrangeOverride->Width,
                      .Height = pArrangeOverride->Height);
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

HANDLER(ImageView, Node2D, DrawBrush)
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
  if (!pImageView->Source) {
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

  lpTexture_t img = pDrawBrush->foreground ? pImageView->Source : NULL;
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
  } else if (pImageView->Stretch == kStretchUniformToFill && imgsize.x > 0 && imgsize.y > 0) {
    float scaleX = (float)width / imgsize.x;
    float scaleY = (float)height / imgsize.y;
    float scale = scaleX > scaleY ? scaleX : scaleY;
    float uvScaleX = scaleX / scale;
    float uvScaleY = scaleY / scale;
    float uvOffsetX = (1.0f - uvScaleX) / 2.0f;
    float uvOffsetY = (1.0f - uvScaleY) / 2.0f;
    entity.material.textureMatrix.v[0] = uvScaleX * pImageView->Viewbox.z;
    entity.material.textureMatrix.v[4] = uvScaleY * pImageView->Viewbox.w;
    entity.material.textureMatrix.v[6] = pImageView->Viewbox.x + uvOffsetX * pImageView->Viewbox.z;
    entity.material.textureMatrix.v[7] = pImageView->Viewbox.y + uvOffsetY * pImageView->Viewbox.w;
    entity.mesh = BOX_PTR(Mesh, MD_RECTANGLE);
  }
  
  R_DrawEntity(pDrawBrush->viewdef, &entity);

  return TRUE;
}

HANDLER(ImageView, Node2D, ForegroundContent)
{
  return (intptr_t)pImageView->Source;
}

HANDLER(ImageView, Object, Start)
{
  lpProperty_t p = PROP_FindByLongID(OBJ_GetProperties(hObject), ID_ImageView_Src);
  if (p) PROP_SetFlag(p, PF_USED_IN_TRIGGER);
  if (pImageView->Src && *pImageView->Src) {
    axPostMessageW(hObject, ID_Node_LoadView, 0, NULL);
  }
  return FALSE;
}

HANDLER(ImageView, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property) return FALSE;
  if (PROP_GetLongIdentifier(pPropertyChanged->Property) == ID_ImageView_Src) {
    axPostMessageW(hObject, ID_Node_LoadView, 0, NULL);
  }
  return FALSE;
}

HANDLER(ImageView, Node, LoadView)
{
  if (pImageView->Src && *pImageView->Src &&
      pImageView->_loadedImage != fnv1a32(pImageView->Src)) {
    pImageView->_loadedImage = fnv1a32(pImageView->Src);
  }
  return TRUE;
}
