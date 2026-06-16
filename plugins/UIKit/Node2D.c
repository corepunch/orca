#include <include/api.h>
#include <include/orca.h>
#include <include/plugapi.h>
#include <include/renderer.h>

#include <UIKit/UIKit.h>

#include <string.h>

#define STENCIL_VISIBLE_OVERFLOW_EXTENT 100000.0f

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

static struct vec3 _Node2DToLocalPoint(struct Node2D *pNode2D, float x, float y) {
  struct mat4 inv = MAT4_Inverse(&pNode2D->Matrix);
	struct vec3 point = {x,y,0};
  return MAT4_MultiplyVector3D(&inv, &point);
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
  if (!pNode2D->OffscreenRendering) {
    SafeDelete(pNode2D->RenderTarget, Texture_Release);
  }
  return FALSE;
}

HANDLER(Node2D, Node, ScrollWheel)
{
  struct Node *node = pNode2D->_node;
  if (pNode2D->Overflow.y == kOverflowScroll) {
    float maxScroll = MIN(0, node->Size.Axis[1].Actual - node->Size.Axis[1].Scroll);
    if (maxScroll < 0) {
      struct vec2 Offset = pNode2D->ContentOffset;
      Offset.y = MAX(MIN(Offset.y + pScrollWheel->deltaY, 0), maxScroll);
      pNode2D->ContentOffset = Offset;
      OBJ_SetDirty(hObject);
      return TRUE;
    }
  }
  return FALSE;
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

HANDLER(Node2D, Node2D, Measure)
{
  struct Node2D *n = pNode2D;
  LRESULT size = _SendMessage(hObject, Node2D, MeasureOverride,
    .Width  = _MeasureAxis(n, pMeasure->Width  - TOTAL_MARGIN(n, 0), 0) - TOTAL_PADDING(n, 0),
    .Height = _MeasureAxis(n, pMeasure->Height - TOTAL_MARGIN(n, 1), 1) - TOTAL_PADDING(n, 1),
  );
  if (isnan(NODE2D_FRAME(n, Size, 0).Requested)) {
    NODE2D_FRAME(n, Size, 0).Desired = LOWORD(size) + TOTAL_PADDING(n, 0);
  } else {
    NODE2D_FRAME(n, Size, 0).Desired = NODE2D_FRAME(n, Size, 0).Requested;
  }
  if (isnan(NODE2D_FRAME(n, Size, 1).Requested)) {
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

  LRESULT size = _SendMessage(hObject, Node2D, ArrangeOverride,
    .X      = PADDING_TOP(n, 0),
    .Y      = PADDING_TOP(n, 1),
    .Width  = rect.width  - TOTAL_PADDING(n, 0),
    .Height = rect.height - TOTAL_PADDING(n, 1),
  );

  // Final frame is the outer rect (including padding, excluding margin)
  Node2D_SetFrame(n, kBox3FieldX,      rect.x);
  Node2D_SetFrame(n, kBox3FieldY,      rect.y);
  Node2D_SetFrame(n, kBox3FieldWidth,  LOWORD(size) + TOTAL_PADDING(n, 0));
  Node2D_SetFrame(n, kBox3FieldHeight, HIWORD(size) + TOTAL_PADDING(n, 1));

  return MAKEDWORD(rect.width + TOTAL_MARGIN(n, 0), rect.height + TOTAL_MARGIN(n, 1));
}

HANDLER(Node2D, Node2D, MeasureOverride)
{
  uint16_t width = 0, height = 0;
  FOR_EACH_OBJECT(hChild, hObject) {
    uint32_t size = (uint32_t)_SendMessage(hChild, Node2D, Measure,
                                           .Width = pMeasureOverride->Width,
                                           .Height = pMeasureOverride->Height);
    width  = MAX(width,  LOWORD(size));
    height = MAX(height, HIWORD(size));
  }
  return MAKEDWORD(isinf(pMeasureOverride->Width)  ? width  : 0,
                   isinf(pMeasureOverride->Height) ? height : 0);
}

HANDLER(Node2D, Node2D, ArrangeOverride)
{
  FOR_EACH_CHILD(hObject, _SendMessage, Node2D, Arrange,
                 .X = pArrangeOverride->X,
                 .Y = pArrangeOverride->Y,
                 .Width = pArrangeOverride->Width,
                 .Height = pArrangeOverride->Height);

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
  pNode2D->ContentOffset = offset;
  OBJ_SetDirty(hObject);
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

  if (!pDraw2DContent->OnlyDecorations) {
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
