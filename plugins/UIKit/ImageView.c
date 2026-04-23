#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>
#include <source/filesystem/filesystem.h>
#include <source/network/network.h>

#include <plugins/UIKit/UIKit.h>

/* Cancel and release an in-flight fetch stored in the ImageView.
   Sets both _fetch and _src_object to NULL. */
static void
_ImageView_CancelFetch(lpImageView_t pImageView)
{
  if (pImageView->_fetch) {
    NET_FetchCancel((fetch_handle_t)pImageView->_fetch);
    pImageView->_fetch = NULL;
  }
  if (pImageView->_src_object) {
    OBJ_Release(NULL, (lpObject_t)pImageView->_src_object);
    pImageView->_src_object = NULL;
    pImageView->Source = NULL;
  }
}

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
    _ImageView_CancelFetch(pImageView);
    axPostMessageW(hObject, ID_Node_LoadView, 0, NULL);
  }
  return FALSE;
}

HANDLER(ImageView, Object, Destroy)
{
  _ImageView_CancelFetch(pImageView);
  return FALSE;
}

/* Node.LoadView handler — resolves pImageView->Src:
 *
 *  HTTP/HTTPS URLs
 *  ---------------
 *  First invocation (_fetch == NULL):
 *    Start an async fetch, store the handle in _fetch, then re-post
 *    Node.LoadView so the next pump iteration can poll the result.
 *
 *  Subsequent invocations (_fetch != NULL):
 *    Poll the in-flight request.
 *    - Still running  → re-post Node.LoadView and return.
 *    - Completed      → build a Texture object from the downloaded bytes,
 *                        store it in _src_object (so it stays alive), expose
 *                        the Texture component via pImageView->Source, then
 *                        invalidate layout.
 *
 *  Local file paths
 *  ----------------
 *  Load synchronously via FS_LoadObject. The returned Object already holds
 *  a fully initialised Texture component; we point Source at it and keep
 *  the Object alive in _src_object.
 */
HANDLER(ImageView, Node, LoadView)
{
  const char *src = pImageView->Src;
  if (!src || !*src) return TRUE;

  bool_t is_http = (strncasecmp(src, "http://",  7) == 0 ||
                    strncasecmp(src, "https://", 8) == 0);

  if (is_http) {
    if (!pImageView->_fetch) {
      /* First call: start the async download. */
      fetch_handle_t h = NET_FetchBegin(src);
      if (!h) {
        Con_Printf("ImageView: failed to start fetch for '%s'", src);
        return TRUE;
      }
      pImageView->_fetch = h;
      /* Re-post so we poll next frame. */
      axPostMessageW(hObject, ID_Node_LoadView, 0, NULL);
      return TRUE;
    }

    /* Subsequent calls: poll. */
    if (NET_FetchPoll((fetch_handle_t)pImageView->_fetch)) {
      /* Still downloading — check again next frame. */
      axPostMessageW(hObject, ID_Node_LoadView, 0, NULL);
      return TRUE;
    }

    /* Download finished — retrieve the data. */
    void   *data = NULL;
    size_t  size = 0;
    long    code = NET_FetchFinish((fetch_handle_t)pImageView->_fetch,
                                   &data, &size);
    pImageView->_fetch = NULL;

    if (code != 200 || !data || !size) {
      Con_Printf("ImageView: HTTP fetch for '%s' failed (code %ld)", src, code);
      free(data);
      return TRUE;
    }

    if (size > UINT32_MAX) {
      Con_Printf("ImageView: HTTP response too large for '%s' (%zu bytes)", src, size);
      free(data);
      return TRUE;
    }

    lpObject_t tex_obj = R_LoadImageFromMemoryNative(data, (uint32_t)size);
    free(data);

    if (!tex_obj) {
      Con_Printf("ImageView: failed to decode image from '%s'", src);
      return TRUE;
    }

    /* Release any previous downloaded texture before storing the new one. */
    if (pImageView->_src_object) {
      OBJ_Release(NULL, (lpObject_t)pImageView->_src_object);
    }
    pImageView->_src_object = tex_obj;
    pImageView->Source = GetTexture(tex_obj);

    /* Trigger layout/render invalidation. */
    OBJ_SetDirty(hObject);

    return TRUE;
  }

  /* Local file path: load synchronously. */
  lpObject_t file_obj = FS_LoadObject(src);
  if (!file_obj) {
    Con_Printf("ImageView: failed to load '%s'", src);
    return TRUE;
  }

  if (pImageView->_src_object) {
    OBJ_Release(NULL, (lpObject_t)pImageView->_src_object);
  }
  pImageView->_src_object = file_obj;
  pImageView->Source = GetTexture(file_obj);

  OBJ_SetDirty(hObject);

  return TRUE;
}

