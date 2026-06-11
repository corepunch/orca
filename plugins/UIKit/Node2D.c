#include <include/api.h>
#include <include/orca.h>
#include <include/plugapi.h>

#include <UIKit/UIKit.h>

HANDLER(Node2D, Node2D, UpdateGeometry) {
  float const w = Node2D_GetFrame(pNode2D, kBox3FieldWidth);
  float const h = Node2D_GetFrame(pNode2D, kBox3FieldHeight);
  int const data = ((int)w << 16) | (int)h;
  pNode2D->_userdata = data;
  pNode2D->_rect = (struct rect){0,0,w,h};
  return TRUE;
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
