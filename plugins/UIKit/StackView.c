#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <plugins/UIKit/UIKit.h>

static float
_GetContentSize(lpObject_t hObject,
                StackViewPtr pStackView,
                enum Direction axis)
{
  float value = 0;
  FOR_EACH_LAYOUTABLE(child, hObject)
  {
    value += NODE2D_FRAME(GetNode2D(child), Size, axis).Desired + pStackView->Spacing;
  }
  return fmax(value - pStackView->Spacing, 0);
}

static uint32_t
_Arrange(lpObject_t hObject,
         StackViewPtr pStackView,
         struct bounds primaryBounds,
         struct bounds oppositeBounds)
{
  float content_size = _GetContentSize(hObject, pStackView, pStackView->Direction);
  float span = primaryBounds.max - primaryBounds.min;
  float counter, distributed = 0;
  float maxsize = 0;
  
  // Count children and leftover space (needed for space-* modes)
  uint32_t n = 0;
  float leftover = span;
  FOR_EACH_LAYOUTABLE(child, hObject) {
    leftover -= NODE2D_FRAME(GetNode2D(child), Size, pStackView->Direction).Desired;
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
      if (!n) return 0;
      distributed = leftover / (n + 1);
      counter = primaryBounds.min + distributed;
      break;
    case kJustifyContentSpaceAround:
      if (!n) return 0;
      distributed = leftover / n;
      counter = primaryBounds.min + distributed * 0.5f;
      break;
    case kJustifyContentSpaceBetween:
      if (!n) return 0;
      distributed = n > 1 ? leftover / (n - 1) : 0;
      counter = primaryBounds.min;
      break;
  }
  
  FOR_EACH_LAYOUTABLE(child, hObject)
  {
    Node2DPtr subview = GetNode2D(child);
    lpProperty_t p = StackView_GetProperty(hObject, kStackViewAlignItems);
    int *alignment = &subview->_node->Alignment.Axis[!pStackView->Direction];
    LRESULT s;
    switch (p ? pStackView->AlignItems : -1) {
      case kAlignItemsStart: *alignment = kHorizontalAlignmentLeft; break;
      case kAlignItemsBaseline: *alignment = kHorizontalAlignmentLeft; break;
      case kAlignItemsEnd: *alignment = kHorizontalAlignmentRight; break;
      case kAlignItemsCenter: *alignment = kHorizontalAlignmentCenter; break;
      case kAlignItemsStretch: *alignment = kHorizontalAlignmentStretch; break;
    }
    switch (pStackView->Direction) {
      case kDirectionHorizontal:
        s = OBJ_SendMessageW(child, kMsgArrange, 0, &(struct rect) {
          counter,
          oppositeBounds.min,
          NODE2D_FRAME(subview, Size, 0).Desired + TOTAL_MARGIN(subview, 0),
          oppositeBounds.max - oppositeBounds.min,
        });
        counter += LOWORD(s) + pStackView->Spacing + distributed;
        maxsize = fmax(maxsize, HIWORD(s));
        break;
      case kDirectionVertical:
        s = OBJ_SendMessageW(child, kMsgArrange, 0, &(struct rect) {
          oppositeBounds.min,
          counter,
          oppositeBounds.max - oppositeBounds.min,
          NODE2D_FRAME(subview, Size, 1).Desired + TOTAL_MARGIN(subview, 1),
        });
        counter += HIWORD(s) + pStackView->Spacing + distributed;
        maxsize = fmax(maxsize, LOWORD(s));
        break;
      case kDirectionDepth:
        // not implemented
        break;
    }
  }
  
  switch (pStackView->Direction) {
    case kDirectionHorizontal:
      return MAKEDWORD(counter, maxsize);
    case kDirectionVertical:
      return MAKEDWORD(maxsize, counter);
    case kDirectionDepth:
    default:
      // not implemented
      return 0;
  }
}

HANDLER(StackView, MeasureOverride)
{
  Node2DPtr pNode2D = GetNode2D(hObject);
  Size_t size = {
    pStackView->Direction == kDirectionHorizontal ? -pStackView->Spacing : 0,
    pStackView->Direction == kDirectionVertical ? -pStackView->Spacing : 0,
  };
  FOR_EACH_LAYOUTABLE(hChild, pNode2D->_object) {
    LRESULT s;
    switch (pStackView->Direction) {
      case kDirectionHorizontal:
        s = OBJ_SendMessageW(hChild, kMsgMeasure, 0, &(struct Size) {
          INFINITY, pMeasureOverride->Height
        });
        size.width += LOWORD(s) + pStackView->Spacing;
        size.height = MAX(size.height, HIWORD(s));
        break;
      case kDirectionVertical:
        s = OBJ_SendMessageW(hChild, kMsgMeasure, 0, &(struct Size) {
          pMeasureOverride->Width, INFINITY
        });
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

  switch (pStackView->Direction) {
    case kDirectionHorizontal:
      _Arrange(hObject, pStackView,
               (struct bounds) { r.x, r.x + r.width },
               (struct bounds) { r.y, r.y + r.height });
      break;
    case kDirectionVertical:
      _Arrange(hObject, pStackView,
               (struct bounds) { r.y, r.y + r.height },
               (struct bounds) { r.x, r.x + r.width });
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
  /*
   * Return the actual content size (scroll extent minus the start offset)
   * rather than echoing back the allocated rect.  When both components are 0
   * (e.g. the StackView was given a 0×0 rect), MAKEDWORD would return 0 and
   * OBJ_SendMessageW would fall through to Node2D_ArrangeOverride, which
   * re-arranges all children a second time with the same 0-sized rect —
   * triggering unnecessary extra Grid_ArrangeOverride calls.  Using the
   * content size ensures a non-zero return whenever there are children.
   */
  return MAKEDWORD(scrollSize[0] - pArrangeOverride->x,
                   scrollSize[1] - pArrangeOverride->y);
}
