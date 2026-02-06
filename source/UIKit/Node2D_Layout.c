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

void
Node2D_Measure(Node2DPtr node,
							 enum Direction axis,
							 float space,
							 bool_t force)
{
  //	float const padding = TOTAL_PADDING(pNode2D, axis);
  //	float size = Node2D_GetFrame(pNode2D, kBox3FieldWidth + axis) - padding;
  static enum NodeProperties _props[] = { kNodeWidth, kNodeHeight, kNodeDepth };
  float* size = &NODE2D_FRAME(node, Size, axis).Requested;
  float margin = TOTAL_MARGIN(node, axis);
  //	float  padding = TOTAL_PADDING(pNode2D, axis);
  bool_t stretch = NODE2D_FRAME(node, Alignment, axis) == kUIAlignStretch;
  bool_t isRoot = OBJ_GetParent(node->_object) == NULL;
  lpProperty_t sizeProperty = Node_GetProperty(node->_object, _props[axis]);
  /* Process normally */
  if (stretch || isRoot) {
    Node2D_SetFrame(node, kBox3FieldWidth + axis, space - margin);
  } else if (!PROP_IsNull(sizeProperty)) {
    if (PROP_HasProgram(sizeProperty)) {
      // recalculate size
      PROP_Update(sizeProperty);
      float const* sizevalue = PROP_GetValue(sizeProperty);
      Node2D_SetFrame(node, kBox3FieldWidth + axis, *sizevalue);
    } else {
      Node2D_SetFrame(node, kBox3FieldWidth + axis, *size);
    }
  } else if (node->RenderTarget) {
    struct image_info image;
    Image_GetInfo(node->RenderTarget, &image);
    int const size[] = { image.bmWidth, image.bmHeight, 0 };
    Node2D_SetFrame(node, kBox3FieldWidth + axis, size[axis]);
  } else if (force) {
    Node2D_SetFrame(node, kBox3FieldWidth + axis, space - margin);
  }
}

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

HANDLER(Node2D, UpdateLayout)
{
  FOR_LOOP(i, 2)
  {
    Node2D_Measure(pNode2D, i, pUpdateLayout->Size[i], TRUE);
  }

//  FOR_EACH_LAYOUTABLE(hChild, hObject)
  FOR_EACH_OBJECT(hChild, hObject) if (GetNode2D(hChild))
  {
    OBJ_SendMessageW(hChild, kEventUpdateLayout, 0,
      &(UPDATELAYOUTSTRUCT){
        .Width = Node2D_GetSize(pNode2D, kDirectionHorizontal, kSizingMinusPadding),
        .Height = Node2D_GetSize(pNode2D, kDirectionVertical, kSizingMinusPadding),
        .Force = pUpdateLayout->Force,
      });
    FOR_LOOP(i, 2)
    {
      Node2D_Arrange(GetNode2D(hChild), Node2D_GetBounds(pNode2D, i), i);
    }
  }

  return FALSE;
}

void
Node2D_Arrange(Node2DPtr subview, struct bounds bounds, enum Direction axis)
{
  uint32_t align = NODE2D_FRAME(subview, Alignment, axis);
  float pos = Node2D_Align(subview, &bounds, axis, align);
  Node2D_SetFrame(subview, kBox3FieldX + axis, pos);
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
