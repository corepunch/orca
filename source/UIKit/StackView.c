#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <source/UIKit/UIKit.h>

//float
//Node2D_Align(Node2DPtr, float min, float max, enum Direction, int align);

//static void
//_ReverseArrangeInAxis(StackViewPtr pStackView,
//                      lpObject_t child,
//                      enum Direction axis,
//                      float* counter)
//{
//  Node2DPtr subview = GetNode2D(child);
//
//  if (!subview)
//    return;
//
//  _ReverseArrangeInAxis(pStackView, OBJ_GetNext(subview->_object), axis, counter);
//
//  struct transform2* t = &subview->LayoutTransform;
//  PROP_Update(Node2D_GetProperty(subview->_object, kNode2DLayoutTransform));
//  Node2D_SetFrame(subview, kBox3FieldX + axis, ((float*)t)[axis] + *counter);
//
//  if (Node2D_IsFrameSet(subview, kBox3FieldWidth + axis)) {
//    float frame = Node2D_GetFrame(subview, kBox3FieldWidth + axis);
//    float spacing = pStackView->Spacing;
//    float margin = TOTAL_MARGIN(subview, axis);
//    *counter += frame + margin + spacing;
//  }
//}

static float
_GetContentSize(lpObject_t hObject,
                StackViewPtr pStackView,
                enum Direction axis)
{
  float value = 0;
  FOR_EACH_LAYOUTABLE(child, hObject)
  {
    Node2DPtr subview = GetNode2D(child);
    if (Node2D_IsFrameSet(subview, kBox3FieldWidth + axis)) {
      value += Node2D_GetFrame(subview, kBox3FieldWidth + axis) + pStackView->Spacing;
      //        } else {
      //            return 0;
    }
  }
  return value > 0 ? value - pStackView->Spacing : value;
}

// static enum ui_align
//_GetContentAlignment(lpObject_t  hObject, enum Direction axis)
//{
//	lpObject_t  child = OBJ_GetFirstChild(hObject);
//	return child ? NODE2D_FRAME(GetNode2D(child), Alignment, axis) :
// kUIAlignLeft;
// }

//static void
//_SetPosition(Node2DPtr subview, enum Direction axis, float counter)
//{
//  struct transform2* t = &subview->LayoutTransform;
//  PROP_Update(Node2D_GetProperty(subview->_object, kNode2DLayoutTransform));
//  float translation = ((float*)t)[axis] + counter + MARGIN_TOP(subview, axis);
//  Node2D_SetFrame(subview, kBox3FieldX + axis, translation);
//}
//
//static float
//_GetNodeSize(Node2DPtr subview, enum Direction axis, float spacing)
//{
//  if (Node2D_IsFrameSet(subview, kBox3FieldWidth + axis)) {
//    float frame = Node2D_GetFrame(subview, kBox3FieldWidth + axis);
//    float margin = TOTAL_MARGIN(subview, axis);
//    return frame + margin + spacing;
//  } else {
//    return 0;
//  }
//}

static enum NodeProperties alignment[] = {
  kNodeHorizontalAlignment,
  kNodeVerticalAlignment,
  kNodeDepthAlignment,
};

static void
_Arrange(lpObject_t hObject,
         StackViewPtr pStackView,
         struct bounds primaryBounds,
         struct bounds oppositeBounds)
{
  uint32_t primary = pStackView->Direction;
  float content_size = _GetContentSize(hObject, pStackView, primary);
  float span = primaryBounds.max - primaryBounds.min;
  float counter, distributed = 0;
  
  // Count children and leftover space (needed for space-* modes)
  uint32_t n = 0;
  float leftover = span;
  FOR_EACH_LAYOUTABLE(child, hObject) {
    leftover -= NODE2D_FRAME(GetNode2D(child), Size, primary).Desired;
    n++;
  }
  
  switch (pStackView->JustifyContent) {
    case kJustifyContentCenter:
      counter = primaryBounds.min + (span - content_size) * 0.5f;
      break;
    case kJustifyContentEnd:
      counter = primaryBounds.max - content_size;
      break;
    case kJustifyContentStart:
      counter = primaryBounds.min;
      break;
    case kJustifyContentSpaceEvenly:
      if (!n) return;
      distributed = leftover / (n + 1);
      counter = primaryBounds.min + distributed;
      break;
    case kJustifyContentSpaceAround:
      if (!n) return;
      distributed = leftover / n;
      counter = primaryBounds.min + distributed * 0.5f;
      break;
    case kJustifyContentSpaceBetween:
      if (!n) return;
      distributed = n > 1 ? leftover / (n - 1) : 0;
      counter = primaryBounds.min;
      break;
  }
  
  FOR_EACH_LAYOUTABLE(child, hObject)
  {
    Node2DPtr subview = GetNode2D(child);
    lpProperty_t p = StackView_GetProperty(hObject, kStackViewAlignItems);
    int *alignment = &subview->_node->Alignment.Axis[!primary];
    switch (p ? pStackView->AlignItems : -1) {
      case kAlignItemsStart: *alignment = kHorizontalAlignmentLeft; break;
      case kAlignItemsBaseline: *alignment = kHorizontalAlignmentLeft; break;
      case kAlignItemsEnd: *alignment = kHorizontalAlignmentRight; break;
      case kAlignItemsCenter: *alignment = kHorizontalAlignmentCenter; break;
      case kAlignItemsStretch: *alignment = kHorizontalAlignmentStretch; break;
    }
    if (primary) {
      struct rect rect = {
        oppositeBounds.min,
        counter,
        oppositeBounds.max - oppositeBounds.min,
        NODE2D_FRAME(subview, Size, 1).Desired + TOTAL_MARGIN(subview, 1),
      };
      counter += rect.height + pStackView->Spacing + distributed;
      OBJ_SendMessageW(child, kEventArrange, 0, &rect);
    } else {
      struct rect rect = {
        counter,
        oppositeBounds.min,
        NODE2D_FRAME(subview, Size, 0).Desired + TOTAL_MARGIN(subview, 0),
        oppositeBounds.max - oppositeBounds.min,
      };
      counter += rect.width + pStackView->Spacing + distributed;
      OBJ_SendMessageW(child, kEventArrange, 0, &rect);
    }

//    lpProperty_t p = Node_GetProperty(subview->_object, alignment[opposite]);
//    int* chd = &NODE2D_FRAME(subview, Alignment, opposite);
//    uint32_t align = p ? *chd : pStackView->AlignItems;
//    float x = Node2D_Align(subview,
//                           oppositeBounds.min,
//                           oppositeBounds.max,
//                           opposite,
//                           align);
//    
//    Node2D_SetFrame(subview, kBox3FieldX + opposite, x);
  }
}
//
//static bool_t
//_WillStretch(Node2DPtr node, StackViewCPtr pStackView)
//{
//  return !NODE2D_FRAME(node, Alignment, pStackView->Direction);
//}
//
//HANDLER(StackView, MeasureOverride)
//{
//  Node2DPtr pNode2D = GetNode2D(hObject);
//  Size_t data = {
//    pMeasureOverride->width - TOTAL_PADDING(pNode2D, 0),
//    pMeasureOverride->height - TOTAL_PADDING(pNode2D, 1),
//  };
//  
//  float *datasize = (float*)&data;
//  float size[2] = {0}, fsize[2] = {0};
//  uint32_t dwNumAutos = 0;
//  uint32_t const primary = pStackView->Direction;
//  float const gap = pStackView->Spacing;
//
//  size[primary] = -gap;
//
//  FOR_EACH_LAYOUTABLE(hChild, pNode2D->_object)
//  {
//    if (_WillStretch(GetNode2D(hChild), pStackView)) {
//      dwNumAutos++;
//      continue;
//    }
//    uint32_t s = OBJ_SendMessageW(hChild, kEventMeasure, 0, &data);
//    fsize[0] = LOWORD(s);
//    fsize[1] = HIWORD(s);
//    datasize[primary] -= fsize[primary] + gap;
//    size[primary] += fsize[primary] + gap;
//    // workaround for Kanzi stacks
//#ifdef KANZI_SUPPORT
//    if (primary == 1 || fsize[!primary] != data.Size[!primary] ||
//        OBJ_GetFirstChild(hChild))
//#endif
//      size[!primary] = MAX(fsize[!primary], size[!primary]);
//  }
//
//  if (dwNumAutos > 0)
//  {
//    float spacing = gap * (dwNumAutos - 1);
//    datasize[primary] = (datasize[primary] - spacing) / dwNumAutos;
//    FOR_EACH_LAYOUTABLE(hChild, pNode2D->_object)
//    {
//      if (!_WillStretch(GetNode2D(hChild), pStackView))
//        continue;
//      uint32_t s = OBJ_SendMessageW(hChild, kEventMeasure, 0, &data);
//      fsize[0] = LOWORD(s);
//      fsize[1] = HIWORD(s);
//      size[!primary] = MAX(fsize[!primary], size[!primary]);
//    }
//    size[primary] = (&pMeasureOverride->width)[primary] - TOTAL_PADDING(pNode2D, primary);
//  }
//  
//  return MAKEDWORD((MAX(0, size[0]) ? MAX(0, size[0]) : datasize[0]) + TOTAL_PADDING(pNode2D, 0),
//                   (MAX(0, size[1]) ? MAX(0, size[1]) : datasize[1]) + TOTAL_PADDING(pNode2D, 1));
//}

HANDLER(StackView, MeasureOverride)
{
  Node2DPtr pNode2D = GetNode2D(hObject);
  Size_t size = {
    pStackView->Direction == kDirectionHorizontal ? -pStackView->Spacing : 0,
    pStackView->Direction == kDirectionVertical ? -pStackView->Spacing : 0,
  };
  FOR_EACH_LAYOUTABLE(hChild, pNode2D->_object) {
    uint32_t s = OBJ_SendMessageW(hChild, kEventMeasure, 0, pMeasureOverride);
    switch (pStackView->Direction) {
      case kDirectionHorizontal:
        size.width += LOWORD(s) + pStackView->Spacing;
        size.height = MAX(size.height, HIWORD(s));
        break;
      case kDirectionVertical:
        size.width = MAX(size.width, LOWORD(s));
        size.height += HIWORD(s) + pStackView->Spacing;
        break;
      case kDirectionDepth:
        // Not supported
        break;
    }
  }
  return MAKEDWORD(fmax(0,size.width), fmax(0,size.height));
}

HANDLER(StackView, ArrangeOverride)
{
  Node2DPtr pNode2D = GetNode2D(hObject);
  rect_t r = *pArrangeOverride;
//  uint32_t const primary = pStackView->Direction;

  switch (pStackView->Direction) {
    case kDirectionHorizontal:
      _Arrange(hObject, pStackView, (struct bounds) { r.x, r.x + r.width }, (struct bounds) { r.y, r.y + r.height });
      break;
    case kDirectionVertical:
      _Arrange(hObject, pStackView, (struct bounds) { r.y, r.y + r.height }, (struct bounds) { r.x, r.x + r.width });
      break;
    case kDirectionDepth:
      // Not supported
      break;
  }
  
  float scrollSize[2] = { 0, 0 };
  
  FOR_LOOP(i, 2)
  {
    FOR_EACH_LAYOUTABLE(hChild, pNode2D->_object)
    {
      Node2DPtr subview = GetNode2D(hChild);
      float pos = Node2D_GetFrame(subview, kBox3FieldX + i);
      float size = Node2D_GetFrame(subview, kBox3FieldWidth + i);
      scrollSize[i] = MAX(scrollSize[i], pos + size);
    }
    NODE2D_FRAME(pNode2D, Size, i).Scroll = scrollSize[i];
  }
  return MAKEDWORD(pArrangeOverride->width, pArrangeOverride->height);
}
