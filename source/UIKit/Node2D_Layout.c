#include <source/UIKit/UIKit.h>

enum ui_align
{
  kUIAlignLeft,
  kUIAlignCenter,
  kUIAlignRight,
  kUIAlignStretch,
  kUIAlignTop = kUIAlignLeft,
  kUIAlignBottom = kUIAlignRight,
};

float
Node2D_Align(Node2DPtr, struct bounds const*, enum Direction, uint32_t);

// static void init_size(enum Direction axis, Node2DPtr pNode2D) {
//	if (Node2D_IsFrameSet(pNode2D, kBox3FieldWidth + axis))
//		return;
//	Node2D_SetFrame(pNode2D, kBox3FieldWidth + axis, 0);
// }
//
// static void init_pos(enum Direction axis, struct bounds const* bounds, Node2DPtr
// pNode2D) { 	if (Node2D_IsFrameSet(pNode2D, kBox3FieldX + axis))
// return; if
//(OBJ_GetObject(pNode2D->_object, kGetObjectPartParent)) { 		uint32_t align =
// NODE2D_FRAME(pNode2D, Alignment, axis); 		float coord =
// Node2D_Align(pNode2D, bounds, axis, align); 		Node2D_SetFrame(pNode2D,
// kBox3FieldX + axis, coord); 	} else { 		Node2D_SetFrame(pNode2D,
// kBox3FieldX + axis, 0);
//	}
// }
//
// static void
// Node2D_SetRootFrame(Node2DPtr pNode2D, enum Direction axis, float space)
//{
//	float margin = MARGIN_TOP(pNode2D, axis) + MARGIN_BOTTOM(pNode2D, axis);
//	Node2D_SetFrame(pNode2D, kBox3FieldX + axis, MARGIN_TOP(pNode2D, axis));
//	Node2D_SetFrame(pNode2D, kBox3FieldWidth + axis, space - margin);
// }
//
// static void
// Node2D_SizeToContent(Node2DPtr pNode2D, enum Direction axis, float
// avl_space)
//{
//	int component = kBox3FieldWidth + axis, value = 0;
//	if (pNode2D->SizeToContent || GetPrefabView2D(pNode2D->_object))
//	{
//		FOR_EACH_LAYOUTABLE(child, pNode2D->_object)
//		{
//			Node2DPtr subview = GetNode2D(child);
//			if (!Node2D_IsFrameSet(subview, component))
//				return;
//			value = MAX(value, Node2D_GetFrame(subview, component) +
// TOTAL_MARGIN(subview, axis));
//		}
//		Node2D_SetFrame(pNode2D, component, value +
// TOTAL_PADDING(pNode2D, axis));
//	}
//	else if (OBJ_GetObject(pNode2D->_object, kGetObjectPartParent))
//	{
//		Node2D_SetFrame(pNode2D, component, avl_space);
//	}
//	else
//	{
//		Node2D_SetFrame(pNode2D, component, 1);
//	}
// }

float
Node2D_Align(Node2DPtr pNode2D,
             struct bounds const* bounds,
             enum Direction axis,
             uint32_t align)
{
  struct transform2 const* transform = &(pNode2D->LayoutTransform);
  float const length = Node2D_GetFrame(pNode2D, kBox3FieldWidth + axis);
  float const value = ((float const*)&transform->translation)[axis];

  switch (align) {
    case kUIAlignLeft:
      return bounds->min + value + MARGIN_TOP(pNode2D, axis);
    case kUIAlignRight:
      return bounds->max - length + value - MARGIN_BOTTOM(pNode2D, axis);
    case kUIAlignCenter:
      return (bounds->max - MARGIN_BOTTOM(pNode2D, axis) + bounds->min +
              MARGIN_TOP(pNode2D, axis) - length) *
               0.5f +
             value;
    case kUIAlignStretch:
      return bounds->min + MARGIN_TOP(pNode2D, axis);
    default:
      return 0;
  }
}

struct bounds
Node2D_GetBounds(Node2DPtr pNode2D, enum Direction axis)
{
  float width = Node2D_GetFrame(pNode2D, kBox3FieldWidth + axis);
  struct bounds nodebounds = {
    PADDING_TOP(pNode2D, axis),
    width - PADDING_BOTTOM(pNode2D, axis),
  };
  //	OBJ_SendMessage(pNode2D->_object,
  //					  kEventArrange,
  //					  &(ARRANGESTRUCT){
  //		.axis   = axis,
  //		.bounds = &nodebounds,
  //	});
  return nodebounds;
}

struct rect
Node2D_GetContentRect(Node2DPtr pNode2D)
{
  return (struct rect)  {
    PADDING_TOP(pNode2D, 0),
    PADDING_TOP(pNode2D, 1),
    Node2D_GetFrame(pNode2D, kBox3FieldWidth) - TOTAL_PADDING(pNode2D, 0),
    Node2D_GetFrame(pNode2D, kBox3FieldHeight) - TOTAL_PADDING(pNode2D, 1),
  };
}

HANDLER(Node2D, Measure)
{
  int desired[2] = {0,0};
  FOR_LOOP(axis, 2) {
    //  float const padding = TOTAL_PADDING(pNode2D, axis);
    //  float size = Node2D_GetFrame(pNode2D, kBox3FieldWidth + axis) - padding;
    static enum NodeProperties _props[] = { kNodeWidth, kNodeHeight, kNodeDepth };
    float const requested = NODE2D_FRAME(pNode2D, Size, axis).Requested;
    //  float  padding = TOTAL_PADDING(pNode2D, axis);
    bool_t const stretch = NODE2D_FRAME(pNode2D, Alignment, axis) == kUIAlignStretch;
    bool_t const isRoot = OBJ_GetParent(pNode2D->_object) == NULL;
    lpProperty_t sizeProperty = Node_GetProperty(pNode2D->_object, _props[axis]);
    /* Process normally */
    if (stretch || isRoot) {
      desired[axis] = (&pMeasure->width)[axis];
    } else if (!PROP_IsNull(sizeProperty)) {
      if (PROP_HasProgram(sizeProperty)) {
        // recalculate size
        PROP_Update(sizeProperty);
        float const* sizevalue = PROP_GetValue(sizeProperty);
        desired[axis] = *sizevalue;
      } else {
        desired[axis] = requested ? requested : (&pMeasure->width)[axis];
      }
    } else if (pNode2D->RenderTarget) {
      struct image_info image;
      Image_GetInfo(pNode2D->RenderTarget, &image);
      int const size[] = { image.bmWidth, image.bmHeight, 0 };
      desired[axis] = size[axis];
    } else {
      desired[axis] = (&pMeasure->width)[axis];
    }
  }
  Node2D_SetFrame(pNode2D, kBox3FieldWidth, desired[0]);
  Node2D_SetFrame(pNode2D, kBox3FieldHeight, desired[1]);
  return MAKEDWORD(desired[0], desired[1]);
}

HANDLER(Node2D, Arrange)
{
  //  uint32_t align = NODE2D_FRAME(subview, Alignment, axis);
  //  float pos = Node2D_Align(subview, &bounds, axis, align);
  //  Node2D_SetFrame(subview, kBox3FieldX + axis, pos);
  return MAKEDWORD(pArrange->width, pArrange->height);
}
