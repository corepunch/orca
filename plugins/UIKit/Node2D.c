#include <source/core/animation.h>
#include <include/api.h>
#include <include/orca.h>
#include <include/plugapi.h>
#include <include/renderer.h>

#include <UIKit/UIKit.h>

#include <float.h>
#include <string.h>
#ifdef AX_PLATFORM_IOS
#include <source/ios_scroll.h>
#endif

#define STENCIL_VISIBLE_OVERFLOW_EXTENT 100000.0f
// Normal UIScrollView retention is 0.998 per millisecond: -1000 * log(0.998).
#define SCROLL_DECELERATION_RATE 2.00200267f

// Node2D_Draw2DContent
HANDLER(Node2D, Node2D, Draw2DContent);

// Node2D_UpdateGeometry
HANDLER(Node2D, Node2D, UpdateGeometry) {
  float const w = Node2D_GetFrame(pNode2D, kBox3FieldWidth);
  float const h = Node2D_GetFrame(pNode2D, kBox3FieldHeight);
  int const data = ((int)w << 16) | (int)h;
  pNode2D->_userdata = data;
  pNode2D->_rect = (struct rect){0,0,w,h};
  return TRUE;
}

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

void
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

// Finds the point on the node's plane (z = 0) that projects to (x, y). This
// stays exact when the matrix carries perspective, as during a turnstile.
static struct vec3 _Node2DToLocalPoint(struct Node2D *pNode2D, float x, float y) {
  float const *m = pNode2D->Matrix.v;
  float a = m[0] - x * m[3], b = m[4] - x * m[7], e = x * m[15] - m[12];
  float c = m[1] - y * m[3], d = m[5] - y * m[7], f = y * m[15] - m[13];
  float det = a * d - b * c;
  if (fabsf(det) < 1e-12f) return (struct vec3){ -FLT_MAX, -FLT_MAX, 0 };
  return (struct vec3){ (e * d - b * f) / det, (a * f - e * c) / det, 0 };
}

static bool_t _ContainsPoint(struct Node2D *pNode2D, float x, float y) {
  float w = Node2D_GetFrame(pNode2D, kBox3FieldWidth);
  float h = Node2D_GetFrame(pNode2D, kBox3FieldHeight);
  struct vec3 out = _Node2DToLocalPoint(pNode2D, x, y);
  return RECT_Contains(&(struct rect){0,0,w,h}, (struct vec2 const*)&out);
}

static bool_t
_Node2DShouldClipPointByOverflow(struct Node2D *pNode2D, float x, float y)
{
  struct vec3 local = _Node2DToLocalPoint(pNode2D, x, y);
  float const w = Node2D_GetFrame(pNode2D, kBox3FieldWidth);
  float const h = Node2D_GetFrame(pNode2D, kBox3FieldHeight);
  if (pNode2D->Overflow.x != kOverflowVisible &&
      (local.x < 0 || local.x > w)) {
    return TRUE;
  }
  if (pNode2D->Overflow.y != kOverflowVisible &&
      (local.y < 0 || local.y > h)) {
    return TRUE;
  }
  return FALSE;
}

// Node2D_HitTest
HANDLER(Node2D, Node, HitTest) {
  if (OBJ_IsHidden(hObject) || pNode2D->IgnoreHitTest) {
    return FALSE;
  }
  int16_t x = (int16_t)pHitTest->x;
  int16_t y = (int16_t)pHitTest->y;
  bool_t contains = _ContainsPoint(pNode2D, x, y);
  if (_Node2DShouldClipPointByOverflow(pNode2D, x, y)) {
    return FALSE;
  }
  struct Object *result = NULL;
  FOR_EACH_OBJECT(hChild, hObject) {
    struct Object *childHit = (struct Object *)_SendMessage(hChild, Node, HitTest, .x = x, .y = y);
    if (childHit) result = childHit;
  }
  if (result) {
    return (intptr_t)result;
  } else if (contains) {
    return (intptr_t)hObject;
  } else {
    return FALSE;
  }
}


// Node2D_Create
HANDLER(Node2D, Object, Create) {
  pNode2D->_object = hObject;
  pNode2D->_node = GetNode(hObject);
  pNode2D->_node->Size.Axis[0].Requested = NAN;
  pNode2D->_node->Size.Axis[1].Requested = NAN;
  pNode2D->_node->Size.Axis[2].Requested = NAN;
  return FALSE;
}

// int stereoSeparation = 0;

ORCA_API struct rect
Node2D_GetRect(struct Node2D *node)
{
  return (struct rect){
    .x = Node2D_GetFrame(node, kBox3FieldX),
    .y = Node2D_GetFrame(node, kBox3FieldY),
    .width = Node2D_GetFrame(node, kBox3FieldWidth),
    .height = Node2D_GetFrame(node, kBox3FieldHeight),
  };
}

float
Node2D_GetFrame(struct Node2D *pNode2D, enum Box3Field parm)
{
  switch (parm) {
    case kBox3FieldWidth:
    case kBox3FieldHeight:
    case kBox3FieldDepth:
      return MAX(NODE2D_FRAME(pNode2D, Size, parm - kBox3FieldWidth).Actual, 0);
    default:
      return pNode2D->_actual_pos[parm];
  }
}

void
Node2D_SetFrame(struct Node2D *pNode2D, enum Box3Field parm, float value)
{
  switch (parm) {
    case kBox3FieldWidth:
    case kBox3FieldHeight:
    case kBox3FieldDepth:
      NODE2D_FRAME(pNode2D, Size, parm - kBox3FieldWidth).Actual =
      MAX(NODE2D_FRAME(pNode2D, Size, parm - kBox3FieldWidth).Min, value);
      break;
    default:
      pNode2D->_actual_pos[parm] = value;
      break;
  }
  pNode2D->_bbox_flags |= 1 << parm;
}

static struct edges
_Node2DBorderImageSliceEdges(struct Node2D const *node2d)
{
  return (struct edges) {
    .left = node2d->BorderImage.Slice.Axis[0].Left,
    .top = node2d->BorderImage.Slice.Axis[1].Left,
    .right = node2d->BorderImage.Slice.Axis[0].Right,
    .bottom = node2d->BorderImage.Slice.Axis[1].Right,
  };
}

static bool_t
Node2D_HasBorderImageSlice(struct Node2D const *node2d)
{
  struct edges const slice = _Node2DBorderImageSliceEdges(node2d);
  return memcmp(&slice, &(struct edges){ 0 }, sizeof(struct edges)) != 0;
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
    .blendMode = BLEND_MODE_ALPHA,
    .uniforms = uniforms,
    .numUniforms = OBJ_GetUniforms(node2d->_object, uniforms),
  };
  if (!brush) {
  } else if (brush->Material) {
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
}

HANDLER(Node2D, Node2D, DrawBackground)
{
  if (!memcmp(&pDrawBackground->brush,
              &(struct BrushShorthand){0},
              sizeof(struct BrushShorthand)))
    return FALSE;

  struct ViewEntity entity;

  Node2D_GetViewEntity(GetNode2D(hObject), &entity, NULL, &pDrawBackground->brush);
  // A transparent color or opacity would draw nothing.
  if (!entity.material.texture && !entity.shader &&
      (entity.material.color.a <= 0 || entity.material.opacity <= 0))
    return FALSE;

  entity.bbox = BOX3_FromRect(Node2D_GetBackgroundRect(pNode2D));

  if (entity.material.texture) {
    /* Stretch image to view width, maintain aspect ratio, tile vertically. */
    float tw = (float)entity.material.texture->Width;
    float th = (float)entity.material.texture->Height;
    if (tw > 0 && th > 0) {
      float w = Node2D_GetFrame(pNode2D, kBox3FieldWidth);
      float h = Node2D_GetFrame(pNode2D, kBox3FieldHeight);
      float tile_h = th * (w / tw);
      float ox = pNode2D->ContentOffset.x;
      float oy = pNode2D->ContentOffset.y;
      entity.material.textureMatrix = MAT3_Identity();
      entity.material.textureMatrix.v[0] = 1.0f;
      entity.material.textureMatrix.v[4] = h / tile_h;
      entity.material.textureMatrix.v[6] = ox / w;
      entity.material.textureMatrix.v[7] = oy / tile_h;
      entity.material.textureWrap = kTextureWrapRepeat;
    }
  }

  entity.borderWidth = pDrawBackground->borderWidth;
  entity.borderOffset = pDrawBackground->borderOffset;

  R_DrawEntity(pDrawBackground->viewdef, &entity);

  return TRUE;
}

HANDLER(Node2D, Node2D, DrawForeground)
{
  if (!memcmp(&pDrawForeground->brush,
              &(struct BrushShorthand){0},
              sizeof(struct BrushShorthand)))
    return FALSE;

  if (!pDrawForeground->image &&
      !Node2D_GetProperty(hObject, kNode2DForegroundMaterial)) {
    return FALSE;
  }

  struct ViewEntity entity;

  Node2D_GetViewEntity(GetNode2D(hObject), &entity, pDrawForeground->image, &pDrawForeground->brush);

  entity.borderWidth = pDrawForeground->borderWidth;
  entity.borderOffset = pDrawForeground->borderOffset;

  R_DrawEntity(pDrawForeground->viewdef, &entity);

  return TRUE;
}

// static uint32_t _IndexInParent(struct Object * hObject) {
//	if (!OBJ_GetParent(hObject)) {
//		return 0;
//	}
//	uint32_t dwCounter = 0;
//	struct Object * p = OBJ_GetParent(hObject);
//	FOR_EACH_OBJECT(hChild, p) {
//		if (hChild == hObject) {
//			return dwCounter;
//		}
//		dwCounter++;
//	}
//	return dwCounter;
// }

HANDLER(Node2D, Node, UpdateMatrix)
{
  struct mat4 Matrix;
  struct vec2 ContentOffset = pNode2D->ContentOffset;
  bool_t bInvalidate =
    pUpdateMatrix->force || (OBJ_GetFlags(hObject) & OF_DIRTY);

  if (1) // bInvalidate)
  {
    struct mat4 renderMatrix;
    struct mat4 layoutMatrix;
    struct transform2 const* RenderTransform = &pNode2D->RenderTransform;
    struct transform2 const* LayoutTransform = &pNode2D->LayoutTransform;
    struct rect frame = Node2D_GetRect(pNode2D);
    struct vec2 size = { frame.width, frame.height };
    struct vec2 pivot = VEC2_Mul(&size, &pNode2D->RenderTransformOrigin);
    struct vec2 center = VEC2_Scale(&size, 0.5f);
    struct Property* matprop = Node2D_GetProperty(hObject, kNode2DMatrix);

    layoutMatrix = transform2_ToMatrix3D(
      &(struct transform2){ .translation = { frame.x, frame.y },
                            .rotation = LayoutTransform->rotation,
                            .scale = LayoutTransform->scale },
      &center);

    renderMatrix = transform2_ToMatrix3D(RenderTransform, &pivot);

    Matrix = MAT4_Multiply(&layoutMatrix, &renderMatrix);

    if (matprop && (PROP_GetFlags(matprop) & PF_MODIFIED)) {
      // skip if set from outside
    } else {
      pNode2D->Matrix = MAT4_Multiply(&pUpdateMatrix->parent, &Matrix);
    }

    pNode2D->_opacity = GetNode(hObject)->Opacity * pUpdateMatrix->opacity;
  }

  Matrix = pNode2D->Matrix;

  if (!pNode2D->RenderTarget) {
    MAT4_Translate(&Matrix, &(struct vec3){
      .x = ContentOffset.x,
      .y = ContentOffset.y,
      .z = 0
    });
  }

  FOR_EACH_CHILD(hObject, _SendMessage, Node, UpdateMatrix,
                   .parent = Matrix,
                   .opacity = pNode2D->_opacity,
                   .force = bInvalidate,
                 );

  return TRUE;
}

// struct vec3 UI_GetActualSize(struct Object * hObject) {
//     struct Node2D *view = GetNode2D(hObject);
//     if (view) {
//         struct vec3 v = {
//             NODE2D_FRAME(hObject, ActualSize, 0),
//             NODE2D_FRAME(hObject, ActualSize, 1),
//             view->_actual_pos[1],
//         };
//         return v;
//     } else {
//         return (struct vec3){0};
//     }
// }

HANDLER(Node2D, Object, Destroy)
{
#ifdef AX_PLATFORM_IOS
  ORCA_IOS_ForgetScrollView(hObject);
#endif
  SafeDelete(pNode2D->_shadowTexture, Texture_Release);
  SafeDelete(pNode2D->_backdrop, R_ReleaseBackdrop);
  if (!pNode2D->OffscreenRendering) {
    SafeDelete(pNode2D->RenderTarget, Texture_Release);
  }
  return FALSE;
}

// Scrolling uses ContentOffset's property path, so frames reuse layout and
// still invalidate descendant transforms and painting.
static void
Node2D_ScrollOffset(struct Object *object, struct Node2D *view, float y)
{
  struct vec2 offset = view->ContentOffset;
  offset.y = y;
  struct Property *property = OBJ_FindLongProperty(object, ID_Node2D_ContentOffset);
  view->_scrollWriting = TRUE;
  if (property) PROP_SetValue(property, &offset);
  view->_scrollWriting = FALSE;
}

static float
Node2D_ScrollLimit(struct Node2D *view)
{
  return MIN(0, view->_node->Size.Axis[1].Actual - view->_node->Size.Axis[1].Scroll);
}

static float
Scroll_Clamp(float value, float minimum) { return MAX(minimum, MIN(0, value)); }

// Bounded edge resistance with a matching inverse: grabbing a bouncing list
// preserves its current visual position rather than snapping it to an edge.
static float
Scroll_Rubber(float value, float minimum, float extent)
{
  float edge = Scroll_Clamp(value, minimum), distance = value - edge;
  return edge + distance * 0.55f / (1 + fabsf(distance) * 0.55f / MAX(extent, 1));
}

static float
Scroll_Unrubber(float value, float minimum, float extent)
{
  float edge = Scroll_Clamp(value, minimum), distance = value - edge;
  float ratio = MIN(fabsf(distance) / MAX(extent, 1), 0.95f);
  return edge + distance / (0.55f * (1 - ratio));
}

static bool_t
Node2D_ScrollVisible(struct Object *object)
{
  for (struct Object *node = object; node; node = OBJ_GetParent(node)) {
    struct Screen *screen = GetScreen(node);
    if (screen) return screen->Visible;
    if (GetNode(node) && OBJ_IsHidden(node)) return FALSE;
  }
  return FALSE;
}

static void
Node2D_ScrollRelease(struct Object *object, struct Node2D *view, bool_t cancelled, uint32_t timestamp)
{
  view->_scrollTracking = FALSE;
  if (cancelled || !view->InertiaScrolling || timestamp - view->_scrollSampleTime > 100)
    view->_scrollVelocity.y = 0;
  float y = view->ContentOffset.y, minimum = Node2D_ScrollLimit(view);
  view->_scrollAnimating = fabsf(view->_scrollVelocity.y) > 5 || fabsf(y - Scroll_Clamp(y, minimum)) > 0.25f;
  view->_scrollAnimTime = ANIM_GetTime();
  if (view->_scrollAnimating) OBJ_RequestAnimate(object);
}

HANDLER(Node2D, Node, ScrollWheel)
{
  float minimum = Node2D_ScrollLimit(pNode2D);
  if (pScrollWheel->scrollPhase == kScrollPhaseBegin) {
    if (pNode2D->Overflow.y != kOverflowScroll || minimum >= 0) return FALSE;
    pNode2D->_scrollAnimating = FALSE;
    pNode2D->_scrollTracking = TRUE;
    pNode2D->_scrollVelocity.y = 0;
    pNode2D->_scrollRawOffset.y = Scroll_Unrubber(pNode2D->ContentOffset.y, minimum,
                                                pNode2D->_node->Size.Axis[1].Actual);
    pNode2D->_scrollSampleTime = pScrollWheel->timestamp;
    return TRUE;
  }
  if (pScrollWheel->scrollPhase != kScrollPhaseNone) {
    if (!pNode2D->_scrollTracking) return FALSE;
    if (pNode2D->Overflow.y != kOverflowScroll || !Node2D_ScrollVisible(hObject)) {
      pNode2D->_scrollTracking = pNode2D->_scrollAnimating = FALSE;
      pNode2D->_scrollVelocity.y = 0;
      Node2D_ScrollOffset(hObject, pNode2D, Scroll_Clamp(pNode2D->ContentOffset.y, minimum));
      return FALSE;
    }
    if (pScrollWheel->scrollPhase == kScrollPhaseEnd || pScrollWheel->scrollPhase == kScrollPhaseCancel) {
      Node2D_ScrollRelease(hObject, pNode2D, pScrollWheel->scrollPhase == kScrollPhaseCancel,
                           pScrollWheel->timestamp);
      return TRUE;
    }
    if (pScrollWheel->scrollPhase != kScrollPhaseUpdate) return FALSE;
    uint32_t elapsed = pScrollWheel->timestamp - pNode2D->_scrollSampleTime;
    pNode2D->_scrollRawOffset.y += pScrollWheel->deltaY;
    float raw = pNode2D->_scrollRawOffset.y;
    float y = pNode2D->RubberBandScrolling ? Scroll_Rubber(raw, minimum, pNode2D->_node->Size.Axis[1].Actual)
                                        : Scroll_Clamp(raw, minimum);
    // Sample visual movement, not uncompressed overdrag, to avoid an edge fling.
    if (elapsed > 0 && elapsed <= 100) {
      float velocity = (y - pNode2D->ContentOffset.y) * 1000 / elapsed;
      float weight = 1 - expf(-(float)elapsed / 35);
      pNode2D->_scrollVelocity.y += (CLAMP(velocity, -6000, 6000) - pNode2D->_scrollVelocity.y) * weight;
    } else if (elapsed > 100) pNode2D->_scrollVelocity.y = 0;
    pNode2D->_scrollSampleTime = pScrollWheel->timestamp;
    if (!pNode2D->RubberBandScrolling) pNode2D->_scrollRawOffset.y = y;
    Node2D_ScrollOffset(hObject, pNode2D, y);
    return TRUE;
  }
  if (pNode2D->Overflow.y != kOverflowScroll || minimum >= 0) return FALSE;
  pNode2D->_scrollTracking = pNode2D->_scrollAnimating = FALSE;
  pNode2D->_scrollVelocity.y = 0;
  Node2D_ScrollOffset(hObject, pNode2D, Scroll_Clamp(pNode2D->ContentOffset.y + pScrollWheel->deltaY, minimum));
  return TRUE;
}

HANDLER(Node2D, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property || pNode2D->_scrollWriting) return FALSE;
  uint32_t id = PROP_GetLongIdentifier(pPropertyChanged->Property);
  if (id == ID_Node2D_InertiaScrolling && !pNode2D->InertiaScrolling)
    pNode2D->_scrollVelocity.y = 0;
  if (id == ID_Node2D_ContentOffset) {
    pNode2D->_scrollTracking = pNode2D->_scrollAnimating = FALSE;
#ifdef AX_PLATFORM_IOS
    ORCA_IOS_NativeScrollOffsetChanged(hObject);
#endif
    pNode2D->_scrollVelocity.y = 0;
  }
  return FALSE;
}

HANDLER(Node2D, Object, Animate)
{
#ifdef AX_PLATFORM_IOS
  if (ORCA_IOS_ApplyNativeScroll(hObject)) return TRUE;
#endif
  if (!pNode2D->_scrollAnimating) return FALSE;
  uint32_t now = ANIM_GetTime();
  float dt = MIN((uint32_t)(now - pNode2D->_scrollAnimTime), 64u) / 1000.0f;
  pNode2D->_scrollAnimTime = now;
  float minimum = Node2D_ScrollLimit(pNode2D), y = pNode2D->ContentOffset.y;
  float velocity = pNode2D->_scrollVelocity.y;
  if (!Node2D_ScrollVisible(hObject) || pNode2D->Overflow.y != kOverflowScroll) {
    pNode2D->_scrollAnimating = FALSE;
    pNode2D->_scrollVelocity.y = 0;
    Node2D_ScrollOffset(hObject, pNode2D, Scroll_Clamp(y, minimum));
    return FALSE;
  }
  // Short integration steps keep the spring stable at 30/60/120 Hz and after
  // delayed frames. Critically damped return settles without repeated bounces.
  while (dt > 0) {
    float step = MIN(dt, 1.0f / 240), edge = Scroll_Clamp(y, minimum);
    if (y != edge && pNode2D->RubberBandScrolling) {
      velocity += (-180 * (y - edge) - 26.833f * velocity) * step;
      y += velocity * step;
    } else {
      float loss = -expm1f(-SCROLL_DECELERATION_RATE * step);
      y += velocity * loss / SCROLL_DECELERATION_RATE;
      velocity *= 1 - loss;
    }
    if (!pNode2D->RubberBandScrolling) {
      float clamped = Scroll_Clamp(y, minimum);
      if (clamped != y) velocity = 0;
      y = clamped;
    }
    dt -= step;
  }
  if (fabsf(velocity) < 5 && fabsf(y - Scroll_Clamp(y, minimum)) < 0.25f) {
    y = Scroll_Clamp(y, minimum);
    velocity = 0;
    pNode2D->_scrollAnimating = FALSE;
  }
  pNode2D->_scrollVelocity.y = velocity;
  Node2D_ScrollOffset(hObject, pNode2D, y);
  if (pNode2D->_scrollAnimating) OBJ_RequestAnimate(hObject);
  return TRUE;
}

HANDLER(Node2D, Node, MouseMoved)
{
  if (OBJ_GetFlags(hObject) & OF_HOVERABLE) {
    OBJ_SetHover(hObject);
    return TRUE;
  }
  return FALSE;
}

float Node2D_GetSize(struct Node2D *pNode2D, enum Direction axis, enum Sizing sizing) {
  float const size = Node2D_GetFrame(pNode2D, kBox3FieldWidth + (int)axis);
  switch (sizing) {
    case kSizingPlusMargin:
      return size + TOTAL_MARGIN(pNode2D, axis);
    case kSizingSizeOnly:
      return size;
    case kSizingMinusPadding:
      return size - TOTAL_PADDING(pNode2D, axis);
  }
  return 0;
}

static bool_t
_MarginIsAuto(struct Node2D *pNode2D, enum Direction axis, bool_t trailing)
{
  float const value = trailing
    ? NODE2D_FRAME(pNode2D, Margin, axis)._LEFT
    : NODE2D_FRAME(pNode2D, Margin, axis)._RIGHT;
  return isnan(value);
}

static bool_t
_AxisHasAutoMargin(struct Node2D *pNode2D, enum Direction axis)
{
  return _MarginIsAuto(pNode2D, axis, FALSE) || _MarginIsAuto(pNode2D, axis, TRUE);
}

static float
_AlignAxis(struct Node2D *pNode2D, struct rect const *rect, enum Direction axis, float length)
{
  struct transform2 const *transform = &pNode2D->LayoutTransform;
  float const transformOffset = ((float const*)&transform->translation)[axis];
  float const *r = &rect->x;
  float const available = r[axis + 2];
  float const leftover = fmax(available - length, 0);
  float offset = 0;

  bool_t const leadingAuto = _MarginIsAuto(pNode2D, axis, FALSE);
  bool_t const trailingAuto = _MarginIsAuto(pNode2D, axis, TRUE);
  if (leadingAuto && trailingAuto) {
    offset = leftover * 0.5f;
  } else if (leadingAuto) {
    offset = leftover;
  }

  return r[axis] + offset + transformOffset;
}

static float _MeasureAxis(struct Node2D *n, float space, int axis)
{
  if (!isnan(NODE2D_FRAME(n, Size, axis).Requested)) {
    return NODE2D_FRAME(n, Size, axis).Requested;
  } else if (n->RenderTarget) {
    struct image_info image;
    Image_GetInfo(n->RenderTarget, &image);
    int const size[] = { image.bmWidth, image.bmHeight, 0 };
    return size[axis] + TOTAL_PADDING(n, axis);
  } else if (_AxisHasAutoMargin(n, axis)) {
    return INFINITY;
  } else {
    return space;
  }
}

// A positive AspectRatio derives an auto height from a finite width.
static bool_t
_HasAspectRatio(struct Node2D *n, float width)
{
  return n->AspectRatio > 0 && isnan(NODE2D_FRAME(n, Size, 1).Requested) && !isinf(width);
}

static float
_ArrangeAxisSize(struct Node2D *n, float available, enum Direction axis)
{
  float const requested = NODE2D_FRAME(n, Size, axis).Requested;
  if (!isnan(requested)) return requested;
  if (isinf(available)) return NODE2D_FRAME(n, Size, axis).Desired;

  if (_AxisHasAutoMargin(n, axis)) {
    return fmin(NODE2D_FRAME(n, Size, axis).Desired, available - TOTAL_MARGIN(n, axis));
  }

  return available - TOTAL_MARGIN(n, axis);
}

// Clamped distances that a parent's top and bottom insets reach into a child
// at y with height h. Zero insets never reach, even past the parent's bounds.
static struct vec2
_InsetOverlap(struct vec2 inset, float parentHeight, float y, float h)
{
  return (struct vec2){
    inset.x > 0 ? CLAMP(inset.x - y, 0, h) : 0,
    inset.y > 0 ? CLAMP(y + h - (parentHeight - inset.y), 0, h) : 0,
  };
}

// The safe area reaches a node through its parent: the bars docked by its
// ancestors and, at the root, the system insets of a Screen that lets its
// content run edge to edge.
static void
Node2D_InheritSafeArea(struct Object *object, struct Node2D *node)
{
  struct Object *parent = OBJ_GetParent(object);
  struct Node2D *p = parent ? GetNode2D(parent) : NULL;
  struct Screen *screen = parent && !p ? GetScreen(parent) : NULL;
  struct vec2 safeArea = { 0 }, bars = { 0 };
  float height = 0;
  if (p) {
    safeArea = p->_safeArea, bars = p->_barInset;
    height = Node2D_GetFrame(p, kBox3FieldHeight);
  } else if (screen && !screen->RespectSafeArea) {
    safeArea.x = screen->SafeAreaTop;
    safeArea.y = MAX(screen->SafeAreaBottom, screen->KeyboardInsetBottom);
    height = screen->Height;
  }
  float y = Node2D_GetFrame(node, kBox3FieldY), h = Node2D_GetFrame(node, kBox3FieldHeight);
  node->_safeArea = _InsetOverlap(safeArea, height, y, h);
  node->_barInset = _InsetOverlap(bars, height, y, h);
}

// A node that insets its content keeps its children clear of the safe area,
// so none of it reaches them. The inset is published like ScrollHeight, so
// applications can find the visible part of the content.
static struct vec2
Node2D_AdjustContentInset(struct Object *object, struct Node2D *node)
{
  bool_t adjust = node->ContentInsetAdjustment == kContentInsetAdjustmentAlways ||
    (node->ContentInsetAdjustment == kContentInsetAdjustmentAutomatic && node->Overflow.y == kOverflowScroll);
  struct vec2 inset = adjust ? node->_safeArea : (struct vec2){ 0 };
  if (adjust) node->_safeArea = node->_barInset = (struct vec2){ 0 };
  float values[2] = { inset.x, inset.y };
  uint32_t const ids[2] = { ID_Node2D_AdjustedContentInsetTop, ID_Node2D_AdjustedContentInsetBottom };
  float *edges[2] = { &NODE2D_INSET(node)._RIGHT, &NODE2D_INSET(node)._LEFT };
  FOR_LOOP(i, 2) {
    if (*edges[i] == values[i]) continue;
    struct Property *property = OBJ_FindLongProperty(object, ids[i]);
    if (property) PROP_SetValue(property, &values[i]);
    else *edges[i] = values[i];
  }
  return inset;
}

// After UIKit scrolling the first responder into view: when the keyboard or a
// bar raises this view's visible bottom, or focus moves to a field inside it,
// a field they cover scrolls into view with a margin. A field already in view
// stays put, and scrolling away from it afterwards is left alone.
#define REVEAL_MARGIN 16
static void
Node2D_RevealFocus(struct Object *object, struct Node2D *view, float previousBottom)
{
  static struct Object *revealed;
  struct Object *focus = core_GetFocus(), *node = focus;
  float bottom = Node2D_GetFrame(view, kBox3FieldHeight) - NODE2D_INSET(view)._LEFT;
  if (!focus || (focus == revealed && bottom >= previousBottom - 0.5f)) return;
  // The field's top in this view's content; a nearer scrolling view owns it.
  float y = 0;
  for (; node && node != object; node = OBJ_GetParent(node)) {
    struct Node2D *n = GetNode2D(node);
    if (!n || (node != focus && n->Overflow.y == kOverflowScroll)) return;
    y += Node2D_GetFrame(n, kBox3FieldY);
  }
  if (!node) return;
  revealed = focus;
  float height = Node2D_GetFrame(GetNode2D(focus), kBox3FieldHeight);
  float offset = view->ContentOffset.y, top = y + offset, target = offset;
  float visibleTop = NODE2D_INSET(view)._RIGHT;
  if (top + height > bottom) target -= top + height - (bottom - REVEAL_MARGIN);
  else if (top < visibleTop) target += visibleTop + REVEAL_MARGIN - top;
  target = Scroll_Clamp(target, Node2D_ScrollLimit(view));
  if (target != offset) Node2D_ScrollOffset(object, view, target);
}

HANDLER(Node2D, Node2D, Measure)
{
  struct Node2D *n = pNode2D;
  float space[2] = { pMeasure->Width - TOTAL_MARGIN(n, 0), pMeasure->Height - TOTAL_MARGIN(n, 1) };
  float avail[2] = { _MeasureAxis(n, space[0], 0), _MeasureAxis(n, space[1], 1) };
  bool_t const aspect = _HasAspectRatio(n, avail[0]);
  if (aspect) avail[1] = avail[0] / n->AspectRatio;
  LRESULT size = _SendMessage(hObject, Node2D, MeasureOverride,
    .Width  = avail[0] - TOTAL_PADDING(n, 0),
    .Height = avail[1] - TOTAL_PADDING(n, 1),
  );
  // Auto margins measure at max-content; content wider than the real space is
  // measured again within it, so wrapping text reports its wrapped height.
  bool_t clamp = FALSE;
  FOR_LOOP(i, 2) {
    float content = (i ? HIWORD(size) : LOWORD(size)) + TOTAL_PADDING(n, i);
    if (isinf(avail[i]) && !isinf(space[i]) && content > space[i]) {
      avail[i] = space[i];
      clamp = TRUE;
    }
  }
  if (clamp) {
    size = _SendMessage(hObject, Node2D, MeasureOverride,
      .Width  = avail[0] - TOTAL_PADDING(n, 0),
      .Height = avail[1] - TOTAL_PADDING(n, 1),
    );
  }
  if (isnan(NODE2D_FRAME(n, Size, 0).Requested)) {
    NODE2D_FRAME(n, Size, 0).Desired = LOWORD(size) + TOTAL_PADDING(n, 0);
  } else {
    NODE2D_FRAME(n, Size, 0).Desired = NODE2D_FRAME(n, Size, 0).Requested;
  }
  if (aspect) {
    NODE2D_FRAME(n, Size, 1).Desired = avail[1];
  } else if (isnan(NODE2D_FRAME(n, Size, 1).Requested)) {
    NODE2D_FRAME(n, Size, 1).Desired = HIWORD(size) + TOTAL_PADDING(n, 1);
  } else {
    NODE2D_FRAME(n, Size, 1).Desired = NODE2D_FRAME(n, Size, 1).Requested;
  }
  return MAKEDWORD(NODE2D_FRAME(n, Size, 0).Desired + TOTAL_MARGIN(n, 0),
                   NODE2D_FRAME(n, Size, 1).Desired + TOTAL_MARGIN(n, 1));
}

HANDLER(Node2D, Node2D, Arrange)
{
  struct Node2D *n = pNode2D;
  struct Size s = {
    _ArrangeAxisSize(n, pArrange->Width, 0),
    _ArrangeAxisSize(n, pArrange->Height, 1),
  };
  if (_HasAspectRatio(n, s.width)) s.height = s.width / n->AspectRatio;

  struct rect m = {
    pArrange->X + MARGIN_TOP(n, 0),
    pArrange->Y + MARGIN_TOP(n, 1),
    pArrange->Width - TOTAL_MARGIN(n, 0),
    pArrange->Height - TOTAL_MARGIN(n, 1),
  };

  struct rect rect = {
    .x = _AlignAxis(n, &m, 0, s.width),
    .y = _AlignAxis(n, &m, 1, s.height),
    .width  = s.width,
    .height = s.height,
  };

  // Read before the frame changes. A bottom-anchored node at its end stays
  // there; elsewhere it keeps the content at its visible bottom edge in place,
  // as the keyboard or a bottom bar grows. A touch in progress is left alone.
  bool_t anchored = n->ScrollAnchor == kScrollAnchorBottom && n->Overflow.y == kOverflowScroll &&
    !n->_scrollTracking && !n->_scrollAnimating;
#ifdef AX_PLATFORM_IOS
  if (ORCA_IOS_IsNativeScrollActive(hObject)) anchored = FALSE;
#endif
  bool_t atEnd = anchored && fabsf(n->ContentOffset.y - Node2D_ScrollLimit(n)) < 1;
  float visibleBottom = Node2D_GetFrame(n, kBox3FieldHeight) - NODE2D_INSET(n)._LEFT;

  // Children read the frame and safe area of their ancestors while arranging.
  Node2D_SetFrame(n, kBox3FieldX,      rect.x);
  Node2D_SetFrame(n, kBox3FieldY,      rect.y);
  Node2D_SetFrame(n, kBox3FieldWidth,  rect.width);
  Node2D_SetFrame(n, kBox3FieldHeight, rect.height);
  Node2D_InheritSafeArea(hObject, n);
  struct vec2 inset = Node2D_AdjustContentInset(hObject, n);

  LRESULT size = _SendMessage(hObject, Node2D, ArrangeOverride,
    .X      = PADDING_TOP(n, 0),
    .Y      = PADDING_TOP(n, 1) + inset.x,
    .Width  = rect.width  - TOTAL_PADDING(n, 0),
    .Height = rect.height - TOTAL_PADDING(n, 1) - inset.x - inset.y,
  );

  // Final frame is the outer rect (including padding, excluding margin)
  Node2D_SetFrame(n, kBox3FieldWidth,  LOWORD(size) + TOTAL_PADDING(n, 0));
  Node2D_SetFrame(n, kBox3FieldHeight, HIWORD(size) + TOTAL_PADDING(n, 1) + inset.x + inset.y);
  if (inset.y > 0) {
    float scroll = n->_node->Size.Axis[1].Scroll + inset.y;
    struct Property *property = OBJ_FindLongProperty(hObject, ID_Node_ScrollHeight);
    if (property) PROP_SetValue(property, &scroll);
    else n->_node->Size.Axis[1].Scroll = scroll;
  }
  if (anchored) {
    float limit = Node2D_ScrollLimit(n);
    float moved = Node2D_GetFrame(n, kBox3FieldHeight) - NODE2D_INSET(n)._LEFT - visibleBottom;
    float y = atEnd ? limit : Scroll_Clamp(n->ContentOffset.y + moved, limit);
    if (y != n->ContentOffset.y) Node2D_ScrollOffset(hObject, n, y);
  }
  if (n->Overflow.y == kOverflowScroll && !n->_scrollTracking) Node2D_RevealFocus(hObject, n, visibleBottom);

  return MAKEDWORD(rect.width + TOTAL_MARGIN(n, 0), rect.height + TOTAL_MARGIN(n, 1));
}

HANDLER(Node2D, Node2D, MeasureOverride)
{
  uint16_t width = 0, height = 0;
  FOR_EACH_OBJECT(hChild, hObject) {
    // Docked bars take their content's height, like an auto row.
    struct Node2D *child = GetNode2D(hChild);
    bool_t docked = child && child->SafeAreaEdge != kSafeAreaEdgeNone;
    uint32_t size = (uint32_t)_SendMessage(hChild, Node2D, Measure,
                                           .Width = pMeasureOverride->Width,
                                           .Height = docked ? INFINITY : pMeasureOverride->Height);
    width  = MAX(width,  LOWORD(size));
    height = MAX(height, HIWORD(size));
  }
  return MAKEDWORD(isinf(pMeasureOverride->Width)  ? width  : 0,
                   isinf(pMeasureOverride->Height) ? height : 0);
}

// Docked bars stack at their edge below the bars of the ancestors. The
// other children are laid out under them, and the area the bars cover joins
// the safe area that reaches those children. A bar that always insets its
// content extends through the system inset at its edge, like an iOS toolbar
// under the home indicator, while its content stays clear of it.
static void
Node2D_ArrangeBars(struct Object *hObject, struct Node2D *pNode2D,
                   Node2D_ArrangeOverrideMsgPtr pArrangeOverride)
{
  float height = Node2D_GetFrame(pNode2D, kBox3FieldHeight);
  float top = MAX(pArrangeOverride->Y, pNode2D->_barInset.x);
  float bottom = MIN(pArrangeOverride->Y + pArrangeOverride->Height, height - pNode2D->_barInset.y);
  struct vec2 system = {
    MAX(pNode2D->_safeArea.x - top, 0),
    MAX(pNode2D->_safeArea.y - (height - bottom), 0),
  };
  bool_t docked[2] = { FALSE, FALSE };
  FOR_EACH_LAYOUTABLE(hChild, hObject) {
    struct Node2D *child = GetNode2D(hChild);
    if (child->SafeAreaEdge == kSafeAreaEdgeNone) continue;
    bool_t atTop = child->SafeAreaEdge == kSafeAreaEdgeTop;
    float size = NODE2D_FRAME(child, Size, 1).Desired + TOTAL_MARGIN(child, 1);
    if (child->ContentInsetAdjustment == kContentInsetAdjustmentAlways && !docked[!atTop])
      size += atTop ? system.x : system.y;
    _SendMessage(hChild, Node2D, Arrange,
                 .X = pArrangeOverride->X,
                 .Y = atTop ? top : bottom - size,
                 .Width = pArrangeOverride->Width,
                 .Height = size);
    if (atTop) top += size; else bottom -= size;
    docked[!atTop] = TRUE;
  }
  if (docked[0]) {
    pNode2D->_barInset.x = MAX(pNode2D->_barInset.x, top);
    pNode2D->_safeArea.x = MAX(pNode2D->_safeArea.x, top);
  }
  if (docked[1]) {
    pNode2D->_barInset.y = MAX(pNode2D->_barInset.y, height - bottom);
    pNode2D->_safeArea.y = MAX(pNode2D->_safeArea.y, height - bottom);
  }
}

HANDLER(Node2D, Node2D, ArrangeOverride)
{
  Node2D_ArrangeBars(hObject, pNode2D, pArrangeOverride);
  FOR_EACH_OBJECT(hChild, hObject) {
    struct Node2D *child = GetNode2D(hChild);
    if (child && child->SafeAreaEdge != kSafeAreaEdgeNone && !OBJ_IsHidden(hChild)) continue;
    _SendMessage(hChild, Node2D, Arrange,
                 .X = pArrangeOverride->X,
                 .Y = pArrangeOverride->Y,
                 .Width = pArrangeOverride->Width,
                 .Height = pArrangeOverride->Height);
  }

  float scroll[2] = { 0, 0 };
  FOR_EACH_LAYOUTABLE(hChild, hObject) {
    struct Node2D *child = GetNode2D(hChild);
    FOR_LOOP(i, 2)
      scroll[i] = MAX(scroll[i], Node2D_GetFrame(child, kBox3FieldX + i)
                               + Node2D_GetFrame(child, kBox3FieldWidth + i));
  }
  pNode2D->_node->Size.Axis[0].Scroll = scroll[0];
  pNode2D->_node->Size.Axis[1].Scroll = scroll[1];

  return MAKEDWORD(pArrangeOverride->Width, pArrangeOverride->Height);
}

HANDLER(Node2D, Node2D, SetScrollTop)
{
  struct vec2 offset = pNode2D->ContentOffset;
  offset.y = fmin(0, Node2D_GetFrame(pNode2D, kBox3FieldHeight) - pSetScrollTop->Value);
  struct Property *property = OBJ_FindLongProperty(hObject, ID_Node2D_ContentOffset);
  if (property) PROP_SetValue(property, &offset);
  return TRUE;
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

// Blurs and saturates what is already drawn behind the background box. The
// background brush, drawn next, tints the result.
static void
_DrawBackdrop(struct Node2D *pNode2D, struct ViewDef *viewdef)
{
  struct BackdropFilterShorthand const *filter = &pNode2D->BackdropFilter;
  if (filter->BlurRadius <= 0 && filter->Saturation == 1) {
    SafeDelete(pNode2D->_backdrop, R_ReleaseBackdrop);
    return;
  }
  struct ViewEntity entity;
  Node2D_GetViewEntity(pNode2D, &entity, NULL, NULL);
  entity.bbox = BOX3_FromRect(Node2D_GetBackgroundRect(pNode2D));
  float sigma = filter->BlurRadius * (float)axGetScaling();
  if (SUCCEEDED(R_FilterBackdrop(&pNode2D->_backdrop, viewdef, &entity, sigma, filter->Saturation)))
    R_DrawEntity(viewdef, &entity);
}

static void
_DrawBorderImage(struct Node2D *pNode2D, struct ViewDef *viewdef)
{
  if (!pNode2D->BorderImage.Source || !Node2D_HasBorderImageSlice(pNode2D))
    return;
  struct image_info img = {0};
  if (!SUCCEEDED(Image_GetInfo(pNode2D->BorderImage.Source, &img)) ||
      img.bmWidth <= 0 || img.bmHeight <= 0)
    return;
  struct ViewEntity entity;
  Node2D_GetViewEntity(pNode2D, &entity, pNode2D->BorderImage.Source, NULL);
  struct rect bg = Node2D_GetBackgroundRect(pNode2D);
  struct edges slice = _Node2DBorderImageSliceEdges(pNode2D);
  entity.bbox = BOX3_FromRect(bg);
  calculate_ninepatch(&(struct vec2){ bg.width, bg.height },
                      &(struct vec2){ img.bmWidth, img.bmHeight },
                      &(struct edges){ 0 },
                      &slice,
                      &(struct rect){ 0, 0, 1, 1 },
                      &entity.ninepatch);
  entity.mesh = BOX_PTR(Mesh, MD_NINEPATCH);
  R_DrawEntity(viewdef, &entity);
}

HANDLER(Node2D, Node2D, Draw2DContent)
{
  if (OBJ_IsHidden(hObject))
    return FALSE;

  if (_IsOutOfBounds(pNode2D, pDraw2DContent))
    return FALSE;

  LRESULT foregroundContent = FALSE;
  struct Texture* foreground = NULL;

#define kMsgDrawBackground 0x0875c1d1
#define kMsgUpdateGeometry 0x12c1a314
#define kMsgForegroundContent 0x9a7735e5

  _SendMessage(hObject, Node2D, UpdateGeometry);
  foregroundContent = _SendMessage(hObject, Node2D, ForegroundContent);
  foreground = foregroundContent == TRUE ? NULL : (struct Texture*)foregroundContent;

  uint32_t flags = OBJ_GetFlags(hObject);
  struct ViewDef viewdef;

  Init_ViewDef(&viewdef, pDraw2DContent);

  if (pNode2D->BoxShadow.Color.a && foreground) {
    float blur = pNode2D->BoxShadow.BlurRadius;
    struct Texture *shadowTex = NULL;

    if (blur > 0.f) {
      // Descendant content changes propagate OF_DIRTY without updating this
      // object's timestamp, so the dirty flag must also invalidate the blur.
      long ts = OBJ_GetTimestamp(hObject);
      if (!pNode2D->_shadowTexture || (flags & OF_DIRTY) ||
          pNode2D->_shadowRevision != (int32_t)ts) {
        uint32_t pw = foreground->Width  * MAX(foreground->Scale, 1u);
        uint32_t ph = foreground->Height * MAX(foreground->Scale, 1u);
        struct Texture *scratch = NULL, *newShadow = NULL;
        CREATERTSTRUCT rtdesc = { .Width = pw, .Height = ph, .Scale = 1 };
        if (pw > 0 && ph > 0 &&
            SUCCEEDED(RenderTexture_Create(&rtdesc, &scratch)) &&
            SUCCEEDED(RenderTexture_Create(&rtdesc, &newShadow))) {
          float sigma = blur * (float)axGetScaling() / 3.f;
          R_BlurTexture(foreground, newShadow, scratch, sigma);
          // Render targets are allocated in physical pixels for the blur
          // passes, but are composed in the source texture's logical space.
          newShadow->Width = foreground->Width;
          newShadow->Height = foreground->Height;
          newShadow->Scale = foreground->Scale;
          if (pNode2D->_shadowTexture)
            Texture_Release(pNode2D->_shadowTexture);
          pNode2D->_shadowTexture = newShadow;
          newShadow = NULL;
          pNode2D->_shadowRevision = (int32_t)ts;
        }
        if (scratch)   Texture_Release(scratch);
        if (newShadow) Texture_Release(newShadow);
      }
      shadowTex = pNode2D->_shadowTexture;
    }

    // Draw shadow: same bbox as foreground, matrix translated by shadow offset,
    // tinted with BoxShadow.Color. Uses blurred texture when BlurRadius > 0.
    struct ViewEntity shadow;
    Node2D_GetViewEntity(pNode2D, &shadow, shadowTex ? shadowTex : foreground, NULL);
    shadow.material.color = pNode2D->BoxShadow.Color;
    shadow.material.blendMode = BLEND_MODE_ALPHA;
    MAT4_Translate(&shadow.matrix, &(struct vec3){
      pNode2D->BoxShadow.Offset.x,
      pNode2D->BoxShadow.Offset.y,
      0
    });
    R_DrawEntity(&viewdef, &shadow);
  }

  if (!(flags & OF_ACTIVATED)) {
    OBJ_SetFlags(hObject, flags | OF_ACTIVATED);
  }

  struct BrushShorthand foregroundBrush = _Node2DGetForegroundBrush(hObject);

  if (!pDraw2DContent->OnlyDecorations) {
    _DrawBackdrop(pNode2D, &viewdef);
    _SendMessage(hObject, Node2D, DrawBackground,
     .projection = pDraw2DContent->ProjectionMatrix,
     .brush = pNode2D->Background,
     .viewdef = &viewdef);
  }

  if (pNode2D->Ring.Width > 0) {
    _SendMessage(hObject, Node2D, DrawBackground,
      .projection = pDraw2DContent->ProjectionMatrix,
      .borderOffset = pNode2D->Ring.Offset,
      .borderWidth = {
        pNode2D->Ring.Width,
        pNode2D->Ring.Width,
        pNode2D->Ring.Width,
        pNode2D->Ring.Width
      },
      .viewdef = &viewdef,
      .brush = {
        .Color = pNode2D->Ring.Color
      });
  }

  _DrawBorderImage(pNode2D, &viewdef);

  struct vec4 BorderWidth = {
    NODE2D_FRAME(pNode2D, Border.Width, 0).Left,
    NODE2D_FRAME(pNode2D, Border.Width, 0).Right,
    NODE2D_FRAME(pNode2D, Border.Width, 1).Left,
    NODE2D_FRAME(pNode2D, Border.Width, 1).Right,
  }, Zero = {0};

  if (memcmp(&BorderWidth, &Zero, sizeof(struct vec4))) {
    _SendMessage(hObject, Node2D, DrawBackground,
      .projection = pDraw2DContent->ProjectionMatrix,
      .borderWidth = BorderWidth,
      .viewdef = &viewdef,
      .brush = {
        .Color = pNode2D->_node->Border.Color
      });
  }

  if (!pDraw2DContent->OnlyDecorations) {

    if (foregroundContent) {
      _SendMessage(hObject, Node2D, DrawForeground,
        .projection = pDraw2DContent->ProjectionMatrix,
        .image = foreground,
        .brush = foregroundBrush,
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
