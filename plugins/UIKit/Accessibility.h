#ifndef __UIKIT_ACCESSIBILITY_H__
#define __UIKIT_ACCESSIBILITY_H__

#include <include/orca.h>

// One element an assistive technology visits: the rules are in
// Node2D.AccessibilityLabel and AccessibilityTraits (UIKit.cgen). The frame
// is in the screen's points, clipped by the screen and by every clipping
// ancestor; `traits` is an AccessibilityTraits value, never Auto. Include
// the generated UIKit header first (Apple's <UIKit/UIKit.h> shares its name). Strings live until the visitor returns.
struct AccessibilityElement {
  struct Object *object;
  float x, y, width, height;
  lpcString_t label, hint;
  int traits;
};

typedef void (*AccessibilityVisitor)(struct AccessibilityElement const *element, void *context);

// The screen that is read: `screen`, or the last visible modal sibling over it.
ORCA_API struct Object *UIKit_AccessibilityScreen(struct Object *screen);

// Visits the elements of a drawn screen in reading order (children in
// order, top to bottom as laid out). Returns how many were visited.
ORCA_API uint32_t UIKit_CollectAccessibility(struct Object *screen, float width, float height,
                                             AccessibilityVisitor visit, void *context);

#endif
