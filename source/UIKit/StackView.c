#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <source/UIKit/UIKit.h>

typedef struct _ARRANGESTRUCT
{
  /*enum Direction*/ uint32_t axis;
  struct bounds bounds;
} ARRANGESTRUCT, *PARRANGESTRUCT;

float
Node2D_Align(Node2DPtr, struct bounds const*, enum Direction, uint32_t);

static void
_ReverseArrangeInAxis(StackViewPtr pStackView,
                      lpObject_t child,
                      enum Direction axis,
                      float* counter)
{
  Node2DPtr subview = GetNode2D(child);

  if (!subview)
    return;

  _ReverseArrangeInAxis(pStackView, OBJ_GetNext(subview->_object), axis, counter);

  struct transform2* t = &subview->LayoutTransform;
  PROP_Update(Node2D_GetProperty(subview->_object, kNode2DLayoutTransform));
  Node2D_SetFrame(subview, kBox3FieldX + axis, ((float*)t)[axis] + *counter);

  if (Node2D_IsFrameSet(subview, kBox3FieldWidth + axis)) {
    float frame = Node2D_GetFrame(subview, kBox3FieldWidth + axis);
    float spacing = pStackView->Spacing;
    float margin = TOTAL_MARGIN(subview, axis);
    *counter += frame + margin + spacing;
  }
}

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
      value +=
        Node2D_GetFrame(subview, kBox3FieldWidth + axis) + pStackView->Spacing;
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

static void
_SetPosition(Node2DPtr subview, enum Direction axis, float counter)
{
  struct transform2* t = &subview->LayoutTransform;
  PROP_Update(Node2D_GetProperty(subview->_object, kNode2DLayoutTransform));
  float translation = ((float*)t)[axis] + counter + MARGIN_TOP(subview, axis);
  Node2D_SetFrame(subview, kBox3FieldX + axis, translation);
}

static float
_GetNodeSize(Node2DPtr subview, enum Direction axis, float spacing)
{
  if (Node2D_IsFrameSet(subview, kBox3FieldWidth + axis)) {
    float frame = Node2D_GetFrame(subview, kBox3FieldWidth + axis);
    float margin = TOTAL_MARGIN(subview, axis);
    return frame + margin + spacing;
  } else {
    return 0;
  }
}

static void
_ArrangeInAxis(lpObject_t hObject,
               StackViewPtr pStackView,
               PARRANGESTRUCT pArrange)
{
  uint32_t dwSizeAttr = kBox3FieldWidth + pArrange->axis;
  float counter = 0;
  float content_size = _GetContentSize(hObject, pStackView, pArrange->axis);
  switch (pStackView->JustifyContent) {
    case kJustifyContentCenter:
      counter =
        (pArrange->bounds.max + pArrange->bounds.min - content_size) * 0.5f;
      break;
    case kJustifyContentEnd:
      counter = pArrange->bounds.max - content_size;
      break;
    case kJustifyContentStart:
      counter = pArrange->bounds.min;
      break;
    case kJustifyContentSpaceBetween:
    case kJustifyContentSpaceAround:
    case kJustifyContentSpaceEvenly: {
      float counter = pArrange->bounds.min;
      float fLeftOver = pArrange->bounds.max - pArrange->bounds.min;
      uint32_t dwNumChildren = 0;
      FOR_EACH_LAYOUTABLE(child, hObject)
      {
        fLeftOver -= Node2D_GetFrame(GetNode2D(child), dwSizeAttr);
        dwNumChildren++;
      }
      if (!dwNumChildren)
        return;
      float fSize = 0;
      switch (pStackView->JustifyContent) {
        case kJustifyContentSpaceEvenly:
          fSize = fLeftOver / (dwNumChildren + 1);
          counter += fSize;
          break;
        case kJustifyContentSpaceAround:
          fSize = fLeftOver / dwNumChildren;
          counter += fSize * 0.5f;
          break;
        case kJustifyContentSpaceBetween:
          if (dwNumChildren > 1) {
            fSize = fLeftOver / (dwNumChildren - 1);
          }
          break;
        default:
          break;
      }
      FOR_EACH_LAYOUTABLE(child, hObject)
      {
        Node2DPtr subview = GetNode2D(child);
        _SetPosition(subview, pArrange->axis, counter);
        counter += _GetNodeSize(subview, pArrange->axis, pStackView->Spacing);
        counter += fSize;
      }
      return;
    }
  }
  if (pStackView->Reversed) {
    lpObject_t hChildObject = OBJ_GetFirstChild(hObject);
    _ReverseArrangeInAxis(pStackView, hChildObject, pArrange->axis, &counter);
  } else
    FOR_EACH_LAYOUTABLE(child, hObject)
    {
      Node2DPtr subview = GetNode2D(child);
      _SetPosition(subview, pArrange->axis, counter);
      counter += _GetNodeSize(subview, pArrange->axis, pStackView->Spacing);
    }
}

static enum NodeProperties alignment[] = {
  kNodeHorizontalAlignment,
  kNodeVerticalAlignment,
  kNodeDepthAlignment,
};

static void
_ArrangeOppositeAxis(lpObject_t hObject,
                     StackViewPtr pStackView,
                     PARRANGESTRUCT pArrange)
{
  //	Node2DPtr pNode2D = GetNode2D(hObject);
  FOR_EACH_LAYOUTABLE(child, hObject)
  {
    Node2DPtr subview = GetNode2D(child);
    //        uint32_t *pnt = &NODE2D_FRAME(pNode2D, Alignment,
    //        pArrange->axis);
    lpProperty_t p =
      Node_GetProperty(subview->_object, alignment[pArrange->axis]);
    int* chd = &NODE2D_FRAME(subview, Alignment, pArrange->axis);
    uint32_t align = p ? *chd : pStackView->AlignItems;
    float x = Node2D_Align(subview, &pArrange->bounds, pArrange->axis, align);
    Node2D_SetFrame(subview, kBox3FieldX + pArrange->axis, x);
  }
}

static bool_t
_WillStretch(Node2DPtr node, StackViewCPtr pStackView)
{
  return NODE2D_FRAME(node, Alignment, pStackView->Direction) == kHorizontalAlignmentStretch;
}

HANDLER(StackView, MeasureOverride)
{
  Size_t data = *pMeasureOverride;
  float *datasize = (float*)&data;
  Node2DPtr pNode2D = GetNode2D(hObject);
  float size[2] = { 0, 0 }, fsize[2];
  uint32_t dwNumAutos = 0;
  uint32_t const primary = pStackView->Direction;
  float const gap = pStackView->Spacing;

  size[primary] = -gap;

  data.width -= TOTAL_PADDING(pNode2D, 0);
  data.height -= TOTAL_PADDING(pNode2D, 1);

  FOR_EACH_LAYOUTABLE(hChild, pNode2D->_object)
  {
    if (_WillStretch(GetNode2D(hChild), pStackView)) {
      dwNumAutos++;
      continue;
    }
    uint32_t s = OBJ_SendMessageW(hChild, kEventMeasure, 0, &data);
    fsize[0] = LOWORD(s);
    fsize[1] = HIWORD(s);
    datasize[primary] -= fsize[primary] + gap;
    size[primary] += fsize[primary] + gap;
    // workaround for Kanzi stacks
#ifdef KANZI_SUPPORT
    if (primary == 1 || fsize[!primary] != data.Size[!primary] ||
        OBJ_GetFirstChild(hChild))
#endif
      size[!primary] = MAX(fsize[!primary], size[!primary]);
  }

  if (dwNumAutos > 0)
  {
    float spacing = gap * (dwNumAutos - 1);
    datasize[primary] = (datasize[primary] - spacing) / dwNumAutos;
    FOR_EACH_LAYOUTABLE(hChild, pNode2D->_object)
    {
      if (!_WillStretch(GetNode2D(hChild), pStackView))
        continue;
      uint32_t s = OBJ_SendMessageW(hChild, kEventMeasure, 0, &data);
      fsize[0] = LOWORD(s);
      fsize[1] = HIWORD(s);
      size[!primary] = MAX(fsize[!primary], size[!primary]);
    }
    size[primary] = (&pMeasureOverride->width)[primary] - TOTAL_PADDING(pNode2D, primary);
  }
  
  return MAKEDWORD((MAX(0, size[0]) ? MAX(0, size[0]) : datasize[0]) + TOTAL_PADDING(pNode2D, 0),
                   (MAX(0, size[1]) ? MAX(0, size[1]) : datasize[1]) + TOTAL_PADDING(pNode2D, 1));
}

HANDLER(StackView, ArrangeOverride)
{
  Node2DPtr pNode2D = GetNode2D(hObject);
  uint32_t const primary = pStackView->Direction;
  _ArrangeInAxis(hObject,
                 pStackView,
                 &(ARRANGESTRUCT){
    .axis = primary,
    .bounds = Node2D_GetBounds(pNode2D, primary),
  });
  
  _ArrangeOppositeAxis(hObject,
                       pStackView,
                       &(ARRANGESTRUCT){
    .axis = !primary,
    .bounds = Node2D_GetBounds(pNode2D, !primary),
  });
  
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
