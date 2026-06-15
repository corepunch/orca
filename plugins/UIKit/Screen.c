#include <include/renderer.h>

#include <UIKit/UIKit.h>
#include <filesystem/filesystem.h>
#include <source/core/object/object_internal.h>
#include <source/core/property/property_internal.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

// Node2D_Draw2DContent
HANDLER(Node2D, Node2D, Draw2DContent);

void
draw_children(struct Object *hObject,
              Node2D_Draw2DContentMsgPtr pDraw2DContent);

static bool_t
_PopupSetDialogResult(struct Object *modal, float value);

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

struct _PIPELINESTATE
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

// Screen_RenderScreen
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

// Screen_Create
//HANDLER(Screen, Object, Create) {
//  extern bool_t is_server;
//  pScreen->_size = axGetSize(NULL);
//  R_Init(LOWORD(pScreen->_size), HIWORD(pScreen->_size), is_server);
//  axPostMessageW(hObject, ID_Window_Paint, pScreen->_size, NULL);
//  return FALSE;
//}

// Screen_Create
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

// Screen_Destroy
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

// Screen_Paint
HANDLER(Screen, Window, Paint) {
  PROP_RunAllPrograms(hObject);

  R_BeginFrame(pScreen->ClearColor);

  draw_screen(hObject, pScreen, pPaint->WindowWidth, pPaint->WindowHeight);

  R_EndFrame();

  return TRUE;
}

// Screen_HitTest
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

// Screen_ShowModal
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

// Screen_SetModalObject
HANDLER(Screen, Screen, SetModalObject) {
  if (_AttachModalObject(hObject, pSetModalObject->Target)) {
    bool_t visible = TRUE;
    OBJ_SetPropertyValue(pSetModalObject->Target, "Visible", &visible);
    return TRUE;
  }
  return FALSE;
}

// Popup_ClosePopup
HANDLER(Popup, Popup, ClosePopup) {
  _CloseModalPopup(hObject, pClosePopup->ReturnValue);
  return TRUE;
}

// Screen_Resized
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


// Screen_UpdateLayout
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
