#include "Layout.h"
#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <UIKit/UIKit.h>

static float
_GetContentSize(struct Object *hObject,
                struct StackView *pStackView,
                enum Direction axis)
{
  float value = 0;
  FOR_EACH_LAYOUTABLE(child, hObject)
  {
    value += NODE2D_FRAME(GetNode2D(child), Size, axis).Desired + pStackView->Spacing;
  }
  return fmax(value - pStackView->Spacing, 0);
}

static void
_Arrange(struct Object *hObject,
         struct StackView *pStackView,
         struct bounds primaryBounds,
         struct bounds oppositeBounds)
{
  float content_size = _GetContentSize(hObject, pStackView, pStackView->Direction);
  float span = primaryBounds.max - primaryBounds.min;
  float counter, distributed = 0;

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

  enum Direction crossAxis = pStackView->Direction == kDirectionHorizontal
    ? kDirectionVertical
    : kDirectionHorizontal;

  FOR_EACH_LAYOUTABLE(child, hObject)
  {
    struct Node2D *subview = GetNode2D(child);
    float crossMin = oppositeBounds.min;
    float crossSize = oppositeBounds.max - oppositeBounds.min;
    if (pStackView->AlignItems != kAlignItemsStretch) {
      float occupied = NODE2D_FRAME(subview, Size, crossAxis).Desired + TOTAL_MARGIN(subview, crossAxis);
      switch (pStackView->AlignItems) {
        case kAlignItemsEnd:
          crossMin = oppositeBounds.max - occupied;
          crossSize = occupied;
          break;
        case kAlignItemsCenter:
          crossMin = (oppositeBounds.min + oppositeBounds.max - occupied) * 0.5f;
          crossSize = occupied;
          break;
        case kAlignItemsStart:
        case kAlignItemsBaseline:
          crossSize = occupied;
          break;
        case kAlignItemsStretch:
          break;
      }
    }
    struct Size s;
    switch (pStackView->Direction) {
      case kDirectionHorizontal:
        s = Layout_Arrange(child,
          counter,
          crossMin,
          NODE2D_FRAME(subview, Size, 0).Desired + TOTAL_MARGIN(subview, 0),
          crossSize,
        );
        counter += s.width + pStackView->Spacing + distributed;
        break;
      case kDirectionVertical:
        s = Layout_Arrange(child,
          crossMin,
          counter,
          crossSize,
          NODE2D_FRAME(subview, Size, 1).Desired + TOTAL_MARGIN(subview, 1),
        );
        counter += s.height + pStackView->Spacing + distributed;
        break;
      case kDirectionDepth:
        // not implemented
        break;
    }
  }
}

HANDLER(StackView, Node2D, MeasureOverride)
{
  struct Node2D *pNode2D = GetNode2D(hObject);
  struct Size size = {
    pStackView->Direction == kDirectionHorizontal ? -pStackView->Spacing : 0,
    pStackView->Direction == kDirectionVertical ? -pStackView->Spacing : 0,
  };
  FOR_EACH_LAYOUTABLE(hChild, pNode2D->_object) {
    struct Size s;
    switch (pStackView->Direction) {
      case kDirectionHorizontal:
        s = Layout_Measure(hChild, INFINITY, pMeasureOverride->Height);
        size.width += s.width + pStackView->Spacing;
        size.height = MAX(size.height, s.height);
        break;
      case kDirectionVertical:
        s = Layout_Measure(hChild, pMeasureOverride->Width, INFINITY);
        size.width = MAX(size.width, s.width);
        size.height += s.height + pStackView->Spacing;
        break;
      case kDirectionDepth:
        // Not supported
        break;
    }
  }
  return Layout_Return(pMeasureOverride, fmax(0,size.width), fmax(0,size.height));
}

HANDLER(StackView, Node2D, ArrangeOverride)
{
  struct Node2D *pNode2D = GetNode2D(hObject);
  Node2D_ArrangeOverrideMsg_t r = *pArrangeOverride;

  switch (pStackView->Direction) {
    case kDirectionHorizontal:
      _Arrange(hObject, pStackView,
               (struct bounds) { r.X, r.X + r.Width },
               (struct bounds) { r.Y, r.Y + r.Height });
      break;
    case kDirectionVertical:
      _Arrange(hObject, pStackView,
               (struct bounds) { r.Y, r.Y + r.Height },
               (struct bounds) { r.X, r.X + r.Width });
      break;
    case kDirectionDepth:
      // Not supported
      break;
  }

  float scrollSize[2] = { PADDING_TOP(pNode2D, 0), PADDING_TOP(pNode2D, 1) };

  FOR_LOOP(i, 2)
  {
    FOR_EACH_LAYOUTABLE(hChild, pNode2D->_object)
    {
      struct Node2D *subview = GetNode2D(hChild);
      float pos = Node2D_GetFrame(subview, kBox3FieldX + i);
      float size = Node2D_GetFrame(subview, kBox3FieldWidth + i);
      scrollSize[i] = MAX(scrollSize[i], pos + size);
    }
    scrollSize[i] += PADDING_BOTTOM(pNode2D, i);
    uint32_t const scrollPropertyIds[] = { ID_Node_ScrollWidth, ID_Node_ScrollHeight };
    struct Property *scrollProperty = OBJ_FindLongProperty(hObject, scrollPropertyIds[i]);
    if (scrollProperty) {
      PROP_SetValue(scrollProperty, &scrollSize[i]);
    } else {
      NODE2D_FRAME(pNode2D, Size, i).Scroll = scrollSize[i];
    }
  }

  return Layout_Return(pArrangeOverride, pArrangeOverride->Width, pArrangeOverride->Height);
}
