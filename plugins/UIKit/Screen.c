#include <include/renderer.h>

#include <UIKit/UIKit.h>
#include <filesystem/filesystem.h>
#include <source/core/object/object_internal.h>
#include <source/core/property/property_internal.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define STENCIL_VISIBLE_OVERFLOW_EXTENT 100000.0f

static bool_t
_PopupSetDialogResult(struct Object *modal, float value);

typedef struct _DRAW2DCONTENTSTRUCT
{
  struct mat4 ProjectionMatrix;
  struct mat4 ViewMatrix;
  struct mat4 BoundsMatrix;
  bool_t ForceRender;
  bool_t OnlyDecorations;
  uint8_t StencilRef;
} DRAW2DCONTENTSTRUCT, *Node2D_Draw2DContentMsgPtr;

HANDLER(Node2D, Node2D, Draw2DContent);

static bool_t
_ScreenShouldClipByOverflow(struct Node2D *pNode2D)
{
  return pNode2D->Overflow.x != kOverflowVisible ||
         pNode2D->Overflow.y != kOverflowVisible;
}

static struct ViewDef*
Init_ViewDef(struct ViewDef* view, Node2D_Draw2DContentMsgPtr parms)
{
  memset(view, 0, sizeof(struct ViewDef));

  view->viewMatrix = MAT4_Identity();

  view->projectionMatrix = parms->ProjectionMatrix;
  view->viewMatrix = parms->ViewMatrix;

  return view;
}

static struct BrushShorthand
_Node2DGetForegroundBrush(struct Object *object)
{
  struct BrushShorthand brush = { 0 };

  brush.Color = (struct color){ 1, 1, 1, 1 };

  Node2D_ReadProperty(object, Foreground.Color, &brush.Color);
  Node2D_ReadProperty(object, Foreground.Image, &brush.Image);
  Node2D_ReadProperty(object, Foreground.Material, &brush.Material);

  return brush;
}

static struct _PIPELINESTATE
_Pipeline2D(int width, int height)
{
  return (struct _PIPELINESTATE) {
		.blend_mode = BLEND_MODE_PREMULTIPLIED_ALPHA,
		.color_write_mode = COLOR_WRITE_MODE_RGBA,
		.cull_mode = CULL_MODE_NONE,
		.depth = {
			.write = FALSE,
			.func = COMPARE_FUNC_ALWAYS,
		},
			.stencil = {
				.write = FALSE,
				.func = COMPARE_FUNC_ALWAYS,
			},
      .viewport = { 0, 0, width, height },
      .scissor = { 0, 0, width, height },
	};
}

static bool_t
_PopupSetDialogResult(struct Object *modal, float value)
{
  struct Property *dialog_result = NULL;
  if (FAILED(OBJ_FindShortProperty(modal, "DialogResult", &dialog_result)) || !dialog_result) {
    return FALSE;
  }
  if (PROP_GetType(dialog_result) != kDataTypeFloat) {
    return FALSE;
  }
  PROP_SetValue(dialog_result, &value);
  return TRUE;
}

static void
_GetContentsMatrix(struct Node2D *pNode2D,
                    struct mat4* projection,
                    struct mat4* view)
{
  float width = Node2D_GetFrame(pNode2D, kBox3FieldWidth);
  float height = Node2D_GetFrame(pNode2D, kBox3FieldHeight);
  *projection = MAT4_Ortho(0, width, height, 0, -1, 1);
  *view = MAT4_Inverse(&pNode2D->Matrix);
  MAT4_Translate(view,
                 &(struct vec3){
                   .x = pNode2D->ContentOffset.x,
                   .y = pNode2D->ContentOffset.y,
                 });
}

static bool_t
_IsOutOfBounds(struct Node2D *node, Node2D_Draw2DContentMsgPtr param)
{
  if (param->ForceRender)
    return FALSE;
  struct mat4 mvp;
  mvp = MAT4_Multiply(&param->BoundsMatrix, &node->Matrix);
  struct vec3 _min = MAT4_MultiplyVector3D(&mvp, &(struct vec3){ 0, 0, 0 });
  struct vec3 _max =
    MAT4_MultiplyVector3D(&mvp,
                          &(struct vec3){ Node2D_GetFrame(node, kBox3FieldWidth),
                                          Node2D_GetFrame(node, kBox3FieldHeight),
                                          0 });
  return _max.x < -1.0f || _min.x > 1.0f || _min.y < -1.0f || _max.y > 1.0f; /* NDC bounds */
}

static void
_DrawStencilClip(struct Node2D *pNode2D, struct ViewDef* viewdef)
{
  struct ViewEntity clip = { 0 };
  struct rect clipRect = Node2D_GetBackgroundRect(pNode2D);

  if (pNode2D->Overflow.x == kOverflowVisible) {
    clipRect.x = -STENCIL_VISIBLE_OVERFLOW_EXTENT;
    clipRect.width = STENCIL_VISIBLE_OVERFLOW_EXTENT * 2.0f;
  }
  if (pNode2D->Overflow.y == kOverflowVisible) {
    clipRect.y = -STENCIL_VISIBLE_OVERFLOW_EXTENT;
    clipRect.height = STENCIL_VISIBLE_OVERFLOW_EXTENT * 2.0f;
  }

  clip.bbox = BOX3_FromRect(clipRect);
  clip.matrix = pNode2D->Matrix;
  clip.material.opacity = 1;
  R_DrawEntity(viewdef, &clip);
}

static void
_EnterStencilClip(struct Node2D *pNode2D,
                  struct ViewDef* viewdef,
                  uint8_t parentRef,
                  uint8_t clipRef)
{
  PIPELINESTATE ps;
  R_GetPipelineState(&ps);

  ps.color_write_mode = COLOR_WRITE_MODE_NONE;
  ps.stencil = (struct stencil_state){
    .mask = 0xFF,
    .ref = parentRef > 0 ? parentRef : clipRef,
    .write = TRUE,
    .func = parentRef > 0 ? COMPARE_FUNC_EQUAL : COMPARE_FUNC_ALWAYS,
    .fail = STENCIL_OP_KEEP,
    .zfail = STENCIL_OP_KEEP,
    .zpass = parentRef > 0 ? STENCIL_OP_INCREMENT : STENCIL_OP_REPLACE,
  };
  R_SetPipelineState(&ps);

  _DrawStencilClip(pNode2D, viewdef);

  ps.color_write_mode = COLOR_WRITE_MODE_RGBA;
  ps.stencil = (struct stencil_state){
    .mask = 0xFF,
    .ref = clipRef,
    .write = FALSE,
    .func = COMPARE_FUNC_EQUAL,
    .fail = STENCIL_OP_KEEP,
    .zfail = STENCIL_OP_KEEP,
    .zpass = STENCIL_OP_KEEP,
  };
  R_SetPipelineState(&ps);
}

static void
_ExitStencilClip(struct Node2D *pNode2D,
                 struct ViewDef* viewdef,
                 uint8_t parentRef,
                 uint8_t clipRef)
{
  PIPELINESTATE ps;
  R_GetPipelineState(&ps);

  ps.color_write_mode = COLOR_WRITE_MODE_NONE;
  ps.stencil = (struct stencil_state){
    .mask = 0xFF,
    .ref = clipRef,
    .write = TRUE,
    .func = COMPARE_FUNC_EQUAL,
    .fail = STENCIL_OP_KEEP,
    .zfail = STENCIL_OP_KEEP,
    .zpass = STENCIL_OP_DECREMENT,
  };
  R_SetPipelineState(&ps);

  _DrawStencilClip(pNode2D, viewdef);

  ps.color_write_mode = COLOR_WRITE_MODE_RGBA;

  if (parentRef > 0) {
    ps.stencil = (struct stencil_state){
      .mask = 0xFF,
      .ref = parentRef,
      .write = FALSE,
      .func = COMPARE_FUNC_EQUAL,
      .fail = STENCIL_OP_KEEP,
      .zfail = STENCIL_OP_KEEP,
      .zpass = STENCIL_OP_KEEP,
    };
  } else {
    ps.stencil = (struct stencil_state){ 0 };
  }
  R_SetPipelineState(&ps);
}

static void
draw_children(struct Object *hObject,
              Node2D_Draw2DContentMsgPtr pDraw2DContent)
{
  struct Node2D *node2D = GetNode2D(hObject);
  DRAW2DCONTENTSTRUCT tmp = *pDraw2DContent;

  if (!node2D || OBJ_IsHidden(hObject)) {
    return;
  } else if (!node2D->RenderTarget) {
    Node2D_Draw2DContent(hObject, node2D, 0, &tmp);
  } else if (!node2D->OffscreenRendering) {
    struct ViewEntity entity;
    struct ViewDef viewdef;
    handle_t rt = node2D->RenderTarget;
    Node2D_GetViewEntity(node2D, &entity, rt, 0);
    Init_ViewDef(&viewdef, pDraw2DContent);
    entity.material.opacity = 1;
    tmp.OnlyDecorations = TRUE;
    Node2D_Draw2DContent(hObject, node2D, 0, &tmp);
    //		struct rect frame = Node2D_GetRect(node2D);
    //		struct vec2 offset = VEC2_Div(&node2D->ContentOffset,
    //&frame.size); 		MAT3_Translate(&entity.textureMatrix, &offset);
    R_DrawEntity(&viewdef, &entity);
  }
}

HANDLER(Node2D, Node2D, DrawBrush)
{
  // #define ID_Material_Texture 0x0789ca7d
	if (!memcmp(&pDrawBrush->brush,
							&(struct BrushShorthand){0},
							sizeof(struct BrushShorthand)))
    return FALSE;

  if (pDrawBrush->foreground && !pDrawBrush->image &&
      !Node2D_GetProperty(hObject, kNode2DForegroundMaterial)
      // && !PROP_FindByLongID(OBJ_GetProperties(hObject), ID_Material_Texture)
  ) {
    return FALSE;
  }

  struct ViewEntity entity;

  Node2D_GetViewEntity(GetNode2D(hObject),&entity,pDrawBrush->image,&pDrawBrush->brush);

  if (!pDrawBrush->foreground) {
    entity.bbox = BOX3_FromRect(Node2D_GetBackgroundRect(pNode2D));
  }

  entity.borderWidth = pDrawBrush->borderWidth;
  entity.borderOffset = pDrawBrush->borderOffset;

  R_DrawEntity(pDrawBrush->viewdef, &entity);

  return TRUE;
}

static void _RenderSubViews(struct Object *hObject) {
  FOR_EACH_CHILD(hObject, _RenderSubViews);

  struct image_info img;
  struct Node2D *node2D = GetNode2D(hObject);
  uint32_t dwFlags = OBJ_GetFlags(hObject);

  if (!node2D || !node2D->RenderTarget)
    return;

  if (!(dwFlags & (OF_DIRTY | OF_SCROLL)) && (dwFlags & OF_RENDERED))
    return;

  if (FAILED(Image_GetInfo(node2D->RenderTarget, &img)))
    return;

	PIPELINESTATE ps = _Pipeline2D(img.bmWidth, img.bmHeight);
  struct mat4 projection, view;

  R_BindFramebuffer(node2D->RenderTarget);
  R_SetPipelineState(&ps);

  _GetContentsMatrix(GetNode2D(hObject), &projection, &view);

  Node2D_Draw2DContent(hObject, node2D, 0, &(DRAW2DCONTENTSTRUCT){
      .ProjectionMatrix = projection,
      .ViewMatrix = view,
      .BoundsMatrix = projection,
      .ForceRender = TRUE,
    });

  OBJ_SetFlags(hObject, (dwFlags & (~OF_SCROLL)) | OF_RENDERED);
}

#define UI_FOV (M_PI / 10)

// float __angle = 0;
//
static bool_t
_FallThrough(struct Screen const* s, Screen_RenderScreenMsgPtr r)
{
  if (s->ResizeMode==kResizeModeCanResize)
    return TRUE;
  if (isnan(s->Width) || isnan(s->Height))
    return TRUE;
  if (s->Width != r->width || s->Height != r->height)
    return FALSE;
  return TRUE;
}

HANDLER(Screen, Screen, RenderScreen) {
  float width = pRenderScreen->width;
  float height = pRenderScreen->height;
  struct Texture *rt = pRenderScreen->target;
  if (!_FallThrough(pScreen, pRenderScreen)) {
    width = pScreen->Width;
    height = pScreen->Height;
    if (!pScreen->_rt) {
      RenderTexture_Create(&(CREATERTSTRUCT) {
        .Width = width * axGetScaling(),
        .Height = height * axGetScaling(),
      }, &pScreen->_rt);
    }
    rt = pScreen->_rt;
  } else if (pScreen->ResizeMode == kResizeModeCanResize ||
             isnan(pScreen->Width) ||
             isnan(pScreen->Height)) {
    pScreen->Width = width;
    pScreen->Height = height;
  }
  
  // setup pipeline
  PIPELINESTATE ps = _Pipeline2D(width, height);
  DRAW2DCONTENTSTRUCT params = { 0 };

  float dist = fmax(width, height) / (2.0 * tan(UI_FOV));
  struct vec3 p = { width / 2, height / 2, -dist }, d = { 0, 0, 1 };

  struct mat4 r;
  r = MAT4_Identity();
  MAT4_Rotate(&r, &(struct vec3){ 0, pRenderScreen->angle, 0 }, kRotationOrderXYZ);
  p = MAT4_MultiplyVector3D(&r, &p);
  d = MAT4_MultiplyVector3D(&r, &d);

  //	__angle = pRenderScreen->angle;

  params.ViewMatrix = MAT4_LookAt(&p, &d, &(struct vec3){ 0, -1, 0 });
  params.BoundsMatrix = MAT4_Ortho(0, width, height, 0, -1000, 1000);
  //	MAT4_Perspective(&params.ProjectionMatrix, UI_FOV, width / height, dist
  //* 0.01, dist * 100);
  //
  //	static int i = 0;
  //	i++;
  //	if ((i/200) % 2) {
  params.ProjectionMatrix = MAT4_Ortho(0, width, height, 0, -1000, 1000);
  params.ViewMatrix = MAT4_Identity();
  //	}

  _RenderSubViews(hObject);

  R_BindFramebuffer(rt);
  R_SetPipelineState(&ps);

  FOR_EACH_CHILD(hObject, draw_children, &params);

  if (pRenderScreen->target != rt) {
    R_BindFramebuffer(pRenderScreen->target);
    R_DrawImage(&(DRAWIMAGESTRUCT) {
      .img = pScreen->_rt,
      .rect = RECT_Fit(&(struct rect) {
        0, 0, pRenderScreen->width, pRenderScreen->height
      }, &(struct vec2) { width, height }),
      .uv = { 0, 0, 1, 1 },
    });
  }
  R_BindFramebuffer(0);
  return FALSE;
}

void
Node2D_GetViewEntity(struct Node2D* node2d,
                     struct ViewEntity* entity,
										 struct Texture const* image,
										 struct BrushShorthand const* brush)
{
  static struct uniform uniforms[MAX_UNIFORMS];

  memset(entity, 0, sizeof(struct ViewEntity));

  entity->bbox = BOX3_FromRect(node2d->_rect);
  entity->matrix = node2d->Matrix;
  entity->radius = *(struct vec4*)&node2d->_node->Border.Radius;
  entity->material = (struct ViewMaterial) {
    .opacity = node2d->_opacity,
    .texture = image,
    .textureMatrix = MAT3_Identity(),
    // Use explicit alpha blending for UI brushes to avoid inherited blend state.
    .blendMode = BLEND_MODE_ALPHA,
    .uniforms = uniforms,
    .numUniforms = OBJ_GetUniforms(node2d->_object, uniforms),
  };
	if (!brush) {
	} else if (brush->Material) {
    // if (material->blendMode != BLEND_MODE_INHERIT)
    // entity->blendMode = (enum blend_mode)material->blendMode;
    entity->shader = brush->Material->Shader;
    entity->material.numUniforms +=
    OBJ_GetUniforms(CMP_GetObject(brush->Material),
                    uniforms + entity->material.numUniforms);
	} else if (brush->Image) {
		entity->material.texture = brush->Image;
	} else {
		entity->material.color = brush->Color;
	}
}

struct rect
Node2D_GetBackgroundRect(struct Node2D *pNode2D)
{
  return (struct rect){
    .width = Node2D_GetFrame(pNode2D, kBox3FieldWidth),
    .height = Node2D_GetFrame(pNode2D, kBox3FieldHeight),
  };
  //	return (struct rect) {
  //		.x = -MARGIN_TOP(pNode2D, kDirectionHorizontal),
  //		.y = -MARGIN_TOP(pNode2D, kDirectionVertical),
  //		.width = Node2D_GetFrame(pNode2D, kBox3FieldWidth) +
  // TOTAL_MARGIN(pNode2D, kDirectionHorizontal), 		.height =
  // Node2D_GetFrame(pNode2D, kBox3FieldHeight) + TOTAL_MARGIN(pNode2D,
  // kDirectionVertical),
  //	};
}

HANDLER(Node2D, Node2D, Draw2DContent)
{
  if (OBJ_IsHidden(hObject))
    return FALSE;
  
  if (_IsOutOfBounds(pNode2D, pDraw2DContent))
    return FALSE;

  LRESULT foregroundContent = FALSE;
  struct Texture* foreground = NULL;

#define kMsgDrawBrush 0x0875c1d1
#define kMsgUpdateGeometry 0x12c1a314
#define kMsgForegroundContent 0x9a7735e5

  _SendMessage(hObject, Node2D, UpdateGeometry);
  foregroundContent = _SendMessage(hObject, Node2D, ForegroundContent);
  foreground = foregroundContent == TRUE ? NULL : (struct Texture*)foregroundContent;
  
  if (pNode2D->BoxShadow.Color.a) {
    //		struct mat4 mat, offset;
    //		offset = MAT4_Identity();
    //		MAT4_Translate(&offset, &(struct
    // vec3){pNode2D->Shadow.Offset.x,
    // pNode2D->Shadow.Offset.y}); 		MAT4_Multiply(pDraw2DContent,
    // &offset, &mat); 		OBJ_SendMessage(hObject, ID_Node2D_DrawBrush, 0,
    //&(DRAWBRUSHSTRUCT){ 			.projection = &mat,
    //.image      = foreground.result, 			.brush      =
    // pNode2D->Shadow.Color, 			.foreground = FALSE });
  }

  uint32_t flags = OBJ_GetFlags(hObject);
  struct ViewDef viewdef;

  Init_ViewDef(&viewdef, pDraw2DContent);

  if (!(flags & OF_ACTIVATED)) {
    OBJ_SetFlags(hObject, flags | OF_ACTIVATED);
  }

  struct BrushShorthand foregroundBrush = _Node2DGetForegroundBrush(hObject);

  if (pNode2D->Ring.Width > 0) {
    _SendMessage(hObject, Node2D, DrawBrush,
      .projection = pDraw2DContent->ProjectionMatrix,
      .borderOffset = pNode2D->Ring.Offset,
      .borderWidth = {
        pNode2D->Ring.Width,
        pNode2D->Ring.Width,
        pNode2D->Ring.Width,
        pNode2D->Ring.Width
      },
      .foreground = FALSE,
      .viewdef = &viewdef,
      .brush = {
        .Color = pNode2D->Ring.Color
      });
  }
  
  struct vec4 BorderWidth = {
    NODE2D_FRAME(pNode2D, Border.Width, 0).Left,
    NODE2D_FRAME(pNode2D, Border.Width, 0).Right,
    NODE2D_FRAME(pNode2D, Border.Width, 1).Left,
    NODE2D_FRAME(pNode2D, Border.Width, 1).Right,
  }, Zero = {0};

  if (memcmp(&BorderWidth, &Zero, sizeof(struct vec4))) {
    _SendMessage(hObject, Node2D, DrawBrush,
      .projection = pDraw2DContent->ProjectionMatrix,
      .borderWidth = BorderWidth,
      .foreground = FALSE,
      .viewdef = &viewdef,
      .brush = {
        .Color = pNode2D->_node->Border.Color
      });
  }

  if (!pDraw2DContent->OnlyDecorations) {
    _SendMessage(hObject, Node2D, DrawBrush,
     .projection = pDraw2DContent->ProjectionMatrix,
     .brush = pNode2D->Background,
     .foreground = FALSE,
     .viewdef = &viewdef);

    if (foregroundContent) {
      _SendMessage(hObject, Node2D, DrawBrush,
        .projection = pDraw2DContent->ProjectionMatrix,
        .image = foreground,
        .brush = foregroundBrush,
        .foreground = TRUE,
        .viewdef = &viewdef);
    }

    if (_ScreenShouldClipByOverflow(pNode2D) && pDraw2DContent->StencilRef < 255) {
      uint8_t parentStencilRef = pDraw2DContent->StencilRef;
      uint8_t clipRef = parentStencilRef + 1;
      _EnterStencilClip(pNode2D, &viewdef, parentStencilRef, clipRef);
      pDraw2DContent->StencilRef = clipRef;
      FOR_EACH_CHILD(hObject, draw_children, pDraw2DContent);
      pDraw2DContent->StencilRef = parentStencilRef;
      _ExitStencilClip(pNode2D, &viewdef, parentStencilRef, clipRef);
    } else {
      FOR_EACH_CHILD(hObject, draw_children, pDraw2DContent);
    }
  }
  return TRUE;
}

//HANDLER(Screen, Object, Create) {
//  extern bool_t is_server;
//  pScreen->_size = axGetSize(NULL);
//  R_Init(LOWORD(pScreen->_size), HIWORD(pScreen->_size), is_server);
//  axPostMessageW(hObject, ID_Window_Paint, pScreen->_size, NULL);
//  return FALSE;
//}


HANDLER(Screen, Object, Create) {
//  struct AXsize size;
//  if (pScreen->ResizeMode == kResizeModeCanResize) {
//    if (axGetSize(&size)) {
//      struct Node *node = GetNode(hObject);
//      node->Size.Axis[0].Requested = (float)size.width;
//      node->Size.Axis[1].Requested = (float)size.height;
//    }
//  }
  return FALSE;
}

HANDLER(Screen, Object, Destroy) {
  SafeDelete(pScreen->_rt, Texture_Release);
  return FALSE;
}

static void
draw_screen(struct Object* hObject,
            struct Screen* pScreen,
            uint32_t WindowWidth,
            uint32_t WindowHeight)
{
  if (!pScreen || !pScreen->Visible) {
    return;
  }

  uint32_t const _size = pScreen->_size;

  _SendMessage(hObject, Screen, UpdateLayout, WindowWidth, WindowHeight);
  
  // If screen size has changed, we need to make sure all properties
  // are recalculated with the new size
  if (pScreen->_size != _size) {
    ORCA_API void CORE_AdvanceFrame(void);
    CORE_AdvanceFrame();
    PROP_RunAllPrograms(hObject);
    _SendMessage(hObject, Screen, UpdateLayout, WindowWidth, WindowHeight);
  }
  
  _SendMessage(hObject, Screen, RenderScreen,
               .width = WindowWidth,
               .height = WindowHeight,
               .stereo = 0,
               .target = 0,
               .angle = 0);
  
  //  int tmp = 0;
  //  FOR_LOOP(i, MAX_FPS_CACHE) { tmp += _fps[i]; }
  //  void DEBUG_Draw(float fps, int bindings);
  //  DEBUG_Draw(MAX_FPS_CACHE*1000.f/tmp);
  
  OBJ_ClearDirtyFlags(hObject);
  
  if (OBJ_GetNext(hObject)) { // Render modal screens
    draw_screen(OBJ_GetNext(hObject), GetScreen(OBJ_GetNext(hObject)), WindowWidth, WindowHeight);
  }

}

HANDLER(Screen, Window, Paint) {
  PROP_RunAllPrograms(hObject);

  R_BeginFrame(pScreen->ClearColor);

  draw_screen(hObject, pScreen, pPaint->WindowWidth, pPaint->WindowHeight);
  
  R_EndFrame();

  return TRUE;
}

HANDLER(Screen, Node, HitTest) {
  if (!pScreen->Visible) {
    return FALSE;
  }

  struct Object *result = NULL;
  FOR_EACH_OBJECT(child, hObject) {
    struct Object *childHit = (struct Object *)_SendMessage(child, Node, HitTest,
      .x = pHitTest->x,
      .y = pHitTest->y);
    if (childHit) result = childHit;
  }
  return (intptr_t)result;
}

static void OBJ_SetTreeDirty(struct Object *obj) {
  OBJ_SetDirty(obj);
  FOR_EACH_CHILD(obj, OBJ_SetTreeDirty);
}

static bool_t
_RemoveFromModalChain(struct Object *hObject)
{
  struct Object *parent = OBJ_GetParent(hObject);
  if (!parent) {
    return FALSE;
  }

  for (struct Object **link = &parent->next; *link; link = &(*link)->next) {
    if (*link != hObject) {
      continue;
    }

    *link = hObject->next;
    hObject->next = NULL;
    hObject->parent = NULL;
    return TRUE;
  }

  return FALSE;
}

static bool_t
_AttachModalObject(struct Object *hObject, struct Object *target)
{
  if (!hObject || !target) {
    Con_Error("Invalid arguments to modal attachment");
    return FALSE;
  }

  if (target->parent) {
    REMOVE_FROM_LIST(struct Object, target, target->parent->children);
    REMOVE_FROM_LIST(struct Object, target, target->parent);
  }

  struct Object **next = &hObject->next;
  while (*next) next = &(*next)->next;
  *next = target;
  OBJ_AddRef(target);
  target->parent = hObject;
  target->flags |= OF_NOACTIVATE;
  _SendMessage(target, StyleController, ThemeChanged, .recursive = TRUE);
  if (!_PopupSetDialogResult(target, NAN)) {
    Con_Error("Modal popup missing DialogResult property");
  }
  return TRUE;
}

static void
_CloseModalPopup(struct Object *hObject, float result)
{
  struct Popup *popup = GetPopup(hObject);
  struct Property *dialog_result = NULL;
  bool_t has_dialog_result_handler = FALSE;
  if (!popup) {
    return;
  }

  if (SUCCEEDED(OBJ_FindShortProperty(hObject, "DialogResult", &dialog_result)) &&
      dialog_result && PROP_GetType(dialog_result) == kDataTypeFloat) {
    has_dialog_result_handler = PROP_HasHandler(dialog_result);
    PROP_SetValue(dialog_result, &result);
  } else {
    popup->DialogResult = result;
  }
  {
    bool_t visible = FALSE;
    OBJ_SetPropertyValue(hObject, "Visible", &visible);
  }

  if (!has_dialog_result_handler && _RemoveFromModalChain(hObject)) {
    OBJ_RemoveFromParent(hObject);
  }
}

HANDLER(Screen, Screen, ShowModal) {
  if (!pShowModal || !pShowModal->Path || !*pShowModal->Path) {
    Con_Error("Screen.ShowModal missing Path");
    return FALSE;
  }

  struct Object *target = FS_LoadObject(pShowModal->Path);
  if (!target) {
    Con_Error("Screen.ShowModal could not load template '%s'", pShowModal->Path);
    return FALSE;
  }

  if (!GetPopup(target)) {
    Con_Error("Screen.ShowModal template '%s' is not a Popup", pShowModal->Path);
    OBJ_ReleaseRef(target); // Release the loaded object since we're not going to use it
    return FALSE;
  }

  if (_AttachModalObject(hObject, target)) {
    bool_t visible = TRUE;
    OBJ_SetPropertyValue(target, "Visible", &visible);
    return TRUE;
  }
  return FALSE;
}

HANDLER(Screen, Screen, SetModalObject) {
  if (_AttachModalObject(hObject, pSetModalObject->Target)) {
    bool_t visible = TRUE;
    OBJ_SetPropertyValue(pSetModalObject->Target, "Visible", &visible);
    return TRUE;
  }
  return FALSE;
}

HANDLER(Popup, Popup, ClosePopup) {
  _CloseModalPopup(hObject, pClosePopup->ReturnValue);
  return TRUE;
}

HANDLER(Screen, Window, Resized) {
  if (pScreen->ResizeMode == kResizeModeCanResize ||
      isnan(pScreen->Width) ||
      isnan(pScreen->Height)) {
    pScreen->Width = pResized->WindowWidth;
    pScreen->Height = pResized->WindowHeight;
  }
  OBJ_SetTreeDirty(hObject);
  OBJ_SendMessageW(hObject, ID_Window_Paint, wParam, pResized);
  return FALSE;
}


HANDLER(Screen, Screen, UpdateLayout) {
  float width = pUpdateLayout->Width;
  float height = pUpdateLayout->Height;

  if (pScreen->ResizeMode == kResizeModeCanResize ||
      isnan(pScreen->Width) ||
      isnan(pScreen->Height)) {
    pScreen->Width = width;
    pScreen->Height = height;
  } else {
    width = pScreen->Width;
    height = pScreen->Height;
  }

  pScreen->_size = MAKEDWORD(width, height);

  FOR_EACH_OBJECT(child, hObject) {
    if (!GetNode2D(child)) continue;
    _SendMessage(child, Node2D, Measure, .Width = width, .Height = height);
    _SendMessage(child, Node2D, Arrange, .Width = width, .Height = height);
    _SendMessage(child, Node, UpdateMatrix, .parent = MAT4_Identity(), .opacity = 1);
  }
  PROP_RunAllPrograms(hObject);
  return TRUE;
}
