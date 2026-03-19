#include <include/renderer.h>

#include <plugins/UIKit/UIKit.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

typedef struct _DRAW2DCONTENTSTRUCT
{
  struct color ForegroundColor;
  struct mat4 ProjectionMatrix;
  struct mat4 ViewMatrix;
  struct mat4 BoundsMatrix;
  bool_t ForceRender;
  bool_t OnlyDecorations;
  float FontSize;
} DRAW2DCONTENTSTRUCT, *EVENT_PTR(Draw2DContent);

HANDLER(Node2D, Draw2DContent);

static struct ViewDef*
Init_ViewDef(struct ViewDef* view, Draw2DContentEventPtr parms)
{
  memset(view, 0, sizeof(struct ViewDef));

  view->viewMatrix = MAT4_Identity();

  view->projectionMatrix = parms->ProjectionMatrix;
  view->viewMatrix = parms->ViewMatrix;

  return view;
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

static void
_GetContentsMatrix(Node2DPtr pNode2D,
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
_IsOutOfBounds(Node2DPtr node, Draw2DContentEventPtr param)
{
//  if (param->ForceRender)
    return FALSE;
//  struct mat4 mvp;
//  mvp = MAT4_Multiply(&param->BoundsMatrix, &node->Matrix);
//  struct vec3 _min = MAT4_MultiplyVector3D(&mvp, &(struct vec3){ 0, 0, 0 });
//  struct vec3 _max =
//    MAT4_MultiplyVector3D(&mvp,
//                          &(struct vec3){ Node2D_GetFrame(node, kBox3FieldWidth),
//                                          Node2D_GetFrame(node, kBox3FieldHeight),
//                                          0 });
//  return _min.y < -1 || _min.x < -1 || _max.y > 1 || _max.x > 1;
}

static void
draw_children(lpObject_t hObject,
              Draw2DContentEventPtr pDraw2DContent,
              struct color fg)
{
  Node2DPtr node2D = GetNode2D(hObject);
  DRAW2DCONTENTSTRUCT tmp = *pDraw2DContent;
  tmp.ForegroundColor = fg;

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

HANDLER(Node2D, DrawBrush)
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
		if (pNode2D->ClipChildren) {
			float new_width = MAX(pNode2D->_node->Size.Axis[0].Scroll, entity.bbox.max.x - entity.bbox.min.x);
			float new_height = MAX(pNode2D->_node->Size.Axis[1].Scroll, entity.bbox.max.y - entity.bbox.min.y);
			entity.bbox.max.x = entity.bbox.min.x + new_width;
			entity.bbox.max.y = entity.bbox.min.y + new_height;
		}
  }

  entity.borderWidth = pDrawBrush->borderWidth;
  entity.borderOffset = pDrawBrush->borderOffset;

  R_DrawEntity(pDrawBrush->viewdef, &entity);

  return TRUE;
}

static void _RenderSubViews(lpObject_t hObject) {
  FOR_EACH_CHILD(hObject, _RenderSubViews);

  struct image_info img;
  Node2DPtr node2D = GetNode2D(hObject);
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
      .ForegroundColor = GetNode2D(hObject)->Foreground.Color,
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
_FallThrough(ScreenCPtr s, NodeCPtr n, RenderScreenEventPtr r)
{
  if (s->ResizeMode==kResizeModeCanResize)
    return TRUE;
  if (isnan(n->Size.Axis[0].Requested) || isnan(n->Size.Axis[1].Requested))
    return TRUE;
  if (n->Size.Axis[0].Requested != r->width || n->Size.Axis[1].Requested != r->height)
    return FALSE;
  return TRUE;
}

HANDLER(Screen, RenderScreen) {
  NodePtr node = GetNode(hObject);
  float width = pRenderScreen->width;
  float height = pRenderScreen->height;
  struct Texture *rt = pRenderScreen->target;
  if (!_FallThrough(pScreen, node, pRenderScreen)) {
    width = node->Size.Axis[0].Requested;
    height = node->Size.Axis[1].Requested;
    if (!pScreen->_rt) {
      RenderTexture_Create(&(CREATERTSTRUCT) {
        .Width = width * WI_GetScaling(),
        .Height = height * WI_GetScaling(),
      }, &pScreen->_rt);
    }
    rt = pScreen->_rt;
  } else if (!isnan(node->Size.Axis[0].Requested) &&
             !isnan(node->Size.Axis[1].Requested))
  {
    node->Size.Axis[0].Requested = width;
    node->Size.Axis[1].Requested = height;
  }
  
  // setup pipeline
  PIPELINESTATE ps = _Pipeline2D(width, height);
  DRAW2DCONTENTSTRUCT params = {
    .ForegroundColor = { 1, 1, 1, 1 },
  };

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

  Node2D_Draw2DContent(hObject, GetNode2D(hObject), 0, &params);

  if (pRenderScreen->target != rt) {
    struct rect r = RECT_Fit(&(struct rect) {
      0, 0, pRenderScreen->width, pRenderScreen->height
    }, &(struct vec2) { width, height });
    Con_Printf("Rect %f %f %f %f", r.x, r.y, r.width, r.height);
    Con_Printf("R_DrawImage %f %f in %d %d", width, height, pRenderScreen->width, pRenderScreen->height);
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
Node2D_GetBackgroundRect(Node2DPtr pNode2D)
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

HANDLER(Node2D, Draw2DContent)
{
  if (OBJ_IsHidden(hObject))
    return FALSE;
  
  if (_IsOutOfBounds(pNode2D, pDraw2DContent))
    return FALSE;
  
  if (pNode2D->ClipChildren && !pNode2D->RenderTarget) {
    uint32_t dwWidth = Node2D_GetFrame(pNode2D, kBox3FieldWidth);
    uint32_t dwHeight = Node2D_GetFrame(pNode2D, kBox3FieldHeight);
    RenderTexture_Create(
      &(CREATERTSTRUCT){ .Width = dwWidth,
                         .Height = dwHeight,
                         .Scale = WI_GetScaling() },
      &pNode2D->RenderTarget);
  }

  struct ForegroundContentEventArgs foreground = { 0 };

#define kEventDrawBrush 0x0875c1d1
#define kEventUpdateGeometry 0x12c1a314
#define kEventForegroundContent 0x9a7735e5

  OBJ_SendMessageW(hObject, kEventUpdateGeometry, 0, NULL);
  OBJ_SendMessageW(hObject, kEventForegroundContent, 0, &foreground);
  
  if (pNode2D->BoxShadow.Color.a) {
    //		struct mat4 mat, offset;
    //		offset = MAT4_Identity();
    //		MAT4_Translate(&offset, &(struct
    // vec3){pNode2D->Shadow.Offset.x,
    // pNode2D->Shadow.Offset.y}); 		MAT4_Multiply(pDraw2DContent,
    // &offset, &mat); 		OBJ_SendMessage(hObject, kEventDrawBrush, 0,
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

  if (!Node2D_GetProperty(hObject, kNode2DForegroundColor)) {
    pNode2D->Foreground.Color = pDraw2DContent->ForegroundColor;
  }

  if (pNode2D->Ring.Width > 0) {
    OBJ_SendMessageW(hObject, kEventDrawBrush, 0, &(struct DrawBrushEventArgs){
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
      }});
  }
  
  struct vec4 BorderWidth = {
    NODE2D_FRAME(pNode2D, Border.Width, 0).Left,
    NODE2D_FRAME(pNode2D, Border.Width, 0).Right,
    NODE2D_FRAME(pNode2D, Border.Width, 1).Left,
    NODE2D_FRAME(pNode2D, Border.Width, 1).Right,
  }, Zero = {0};

  if (memcmp(&BorderWidth, &Zero, sizeof(struct vec4))) {
    OBJ_SendMessageW(hObject, kEventDrawBrush, 0, &(struct DrawBrushEventArgs){
      .projection = pDraw2DContent->ProjectionMatrix,
      .borderWidth = BorderWidth,
      .foreground = FALSE,
      .viewdef = &viewdef,
      .brush = {
        .Color = pNode2D->_node->Border.Color
      }
    });
  }

  if (!pDraw2DContent->OnlyDecorations) {
    OBJ_SendMessageW(hObject, kEventDrawBrush, 0, &(struct DrawBrushEventArgs){
     .projection = pDraw2DContent->ProjectionMatrix,
     .brush = pNode2D->Background,
     .foreground = FALSE,
     .viewdef = &viewdef,
    });

    OBJ_SendMessageW(hObject, kEventDrawBrush, 0, &(struct DrawBrushEventArgs){
      .projection = pDraw2DContent->ProjectionMatrix,
      .image = foreground.result,
      .brush = pNode2D->Foreground,
      .foreground = TRUE,
      .viewdef = &viewdef,
    });
    
    FOR_EACH_CHILD(hObject, draw_children, pDraw2DContent, pNode2D->Foreground.Color);
  }
  return TRUE;
}

HANDLER(Screen, MeasureOverride) {
  NodeCPtr n = GetNode(hObject);
  if (!isnan(n->Size.Axis[0].Requested)) {
    pMeasureOverride->width = n->Size.Axis[0].Requested;
  }
  if (!isnan(n->Size.Axis[1].Requested)) {
    pMeasureOverride->height = n->Size.Axis[1].Requested;
  }
  uint32_t newsize = MAKEDWORD(pMeasureOverride->width, pMeasureOverride->height);
  if (pScreen->_size != newsize) {
//    WI_SetSize(pUpdateLayout->Width, pUpdateLayout->Height, TRUE);
    pScreen->_size = newsize;
  }
  FOR_EACH_CHILD(hObject, OBJ_SendMessageW, kEventMeasure, 0, pMeasureOverride);
  return MAKEDWORD(pMeasureOverride->width, pMeasureOverride->height);
}

//HANDLER(Screen, Create) {
//  extern bool_t is_server;
//  pScreen->_size = WI_GetSize(NULL);
//  R_Init(LOWORD(pScreen->_size), HIWORD(pScreen->_size), is_server);
//  WI_PostMessageW(hObject, kEventWindowPaint, pScreen->_size, NULL);
//  return FALSE;
//}


HANDLER(Screen, Create) {
//  extern bool_t is_server;
//  extern int ScreenWidth, ScreenHeight;
////  uint16_t width = MAX(640, GetNode(hObject)->Size.Axis[0].Requested);
////  uint16_t height = MAX(480, GetNode(hObject)->Size.Axis[1].Requested);
//  uint16_t width = MAX(256, ScreenWidth);
//  uint16_t height = MAX(256, ScreenHeight);
//  GetNode(hObject)->Size.Axis[0].Requested = width;
//  GetNode(hObject)->Size.Axis[1].Requested = height;
//  pScreen->_size = MAKEDWORD(width, height);  
  return FALSE;
}

HANDLER(Screen, Destroy) {
  SafeDelete(pScreen->_rt, Texture_Release);
  return FALSE;
}

HANDLER(Screen, WindowPaint) {
  lua_State *L = OBJ_GetDomain(hObject);
  
  if (!pWindowPaint) {
    R_BeginFrame(pScreen->ClearColor);
  }
  
  OBJ_Awake(L, hObject);
  OBJ_Animate(L, hObject);
  OBJ_LoadPrefabs(L, hObject);
  OBJ_EmitPropertyChangedEvents(L, hObject);
  OBJ_UpdateProperties(hObject);
  OBJ_UpdateLayout(hObject, LOWORD(wParam), HIWORD(wParam));
  
  OBJ_SendMessageW(hObject, kEventUpdateMatrix, 0, &(struct UpdateMatrixEventArgs){
    .parent = MAT4_Identity(),
    .opacity = 1,
  });
  
  OBJ_SendMessageW(hObject, kEventRenderScreen, 0, &(struct RenderScreenEventArgs) {
    .width = LOWORD(wParam),
    .height = HIWORD(wParam),
    .stereo = 0,
    .target = 0,
    .angle = 0,
  });
    
  //  int tmp = 0;
  //  FOR_LOOP(i, MAX_FPS_CACHE) { tmp += _fps[i]; }
  //  void DEBUG_Draw(float fps, int bindings);
  //  DEBUG_Draw(MAX_FPS_CACHE*1000.f/tmp);
  
  OBJ_ClearDirtyFlags(hObject);
  
  if (OBJ_GetNext(hObject)) { // Render modal screens
    OBJ_SendMessageW(OBJ_GetNext(hObject), kEventWindowPaint, wParam, hObject);
  }
  
  if (!pWindowPaint) {
    R_EndFrame();
  }

  return TRUE;
}

HANDLER(Screen, WindowResized) {
  NodePtr node = GetNode(hObject);
  if (pScreen->ResizeMode == kResizeModeCanResize ||
      isnan(node->Size.Axis[0].Requested) ||
      isnan(node->Size.Axis[1].Requested)) {
    node->Size.Axis[0].Requested = LOWORD(wParam);
    node->Size.Axis[1].Requested = HIWORD(wParam);
  }
  OBJ_SendMessageW(hObject, kEventWindowPaint, wParam, NULL);
  return FALSE;
}
