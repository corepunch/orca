#include "Layout.h"
#include <include/orca.h>
#include <include/renderer.h>
#include <UIKit/UIKit.h>

// After UIPageControl: 8-point dots 8 points apart, the current page a
// 24-point pill, in a 26-point-tall row.
#define DOT 8.0f
#define CURRENT 24.0f
#define GAP 8.0f
#define HEIGHT 26.0f
#define INACTIVE_OPACITY 0.3f

static bool_t
PageControl_IsShown(struct PageControl const *control)
{
  return control->NumberOfPages > (control->HidesForSinglePage ? 1 : 0);
}

static float
PageControl_RowWidth(int pages)
{
  return pages > 0 ? (pages - 1) * (DOT + GAP) + CURRENT : 0;
}

static int
PageControl_Clamp(struct PageControl const *control, int page)
{
  return MAX(0, MIN(page, control->NumberOfPages - 1));
}

// Writes go through the property, so bindings and on...Changed observers see
// taps and keys as they see any other change.
static bool_t
PageControl_SetPage(struct Object *object, struct PageControl *control, int page)
{
  page = PageControl_Clamp(control, page);
  if (page == control->CurrentPage) return FALSE;
  struct Property *property = OBJ_FindLongProperty(object, ID_PageControl_CurrentPage);
  if (property) PROP_SetValue(property, &page);
  else control->CurrentPage = page;
  return TRUE;
}

HANDLER(PageControl, Object, Create)
{
  OBJ_SetStyle(hObject, OBJ_GetStyle(hObject) | OF_TABSTOP);
  return FALSE;
}

HANDLER(PageControl, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property) return FALSE;
  uint32_t id = PROP_GetLongIdentifier(pPropertyChanged->Property);
  if (id == ID_PageControl_NumberOfPages && pPageControl->NumberOfPages < 0)
    pPageControl->NumberOfPages = 0;
  if (id == ID_PageControl_NumberOfPages || id == ID_PageControl_CurrentPage)
    PageControl_SetPage(hObject, pPageControl, pPageControl->CurrentPage);
  OBJ_SetDirty(hObject);
  return FALSE;
}

HANDLER(PageControl, Node2D, MeasureOverride)
{
  if (!PageControl_IsShown(pPageControl)) return Layout_Return(pMeasureOverride, 0, 0);
  return Layout_Return(pMeasureOverride, PageControl_RowWidth(pPageControl->NumberOfPages), HEIGHT);
}

HANDLER(PageControl, Node, LeftButtonUp)
{
  if (!PageControl_IsShown(pPageControl)) return FALSE;
  float width = Node2D_GetFrame(GetNode2D(hObject), kBox3FieldWidth);
  int step = pLeftButtonUp->x < width / 2 ? -1 : 1;
  PageControl_SetPage(hObject, pPageControl, pPageControl->CurrentPage + step);
  return TRUE;
}

HANDLER(PageControl, Node, KeyDown)
{
  switch (pKeyDown->keyCode) {
    case AX_KEY_LEFTARROW:
      return PageControl_SetPage(hObject, pPageControl, pPageControl->CurrentPage - 1), TRUE;
    case AX_KEY_RIGHTARROW:
      return PageControl_SetPage(hObject, pPageControl, pPageControl->CurrentPage + 1), TRUE;
    default:
      return FALSE;
  }
}

HANDLER(PageControl, Node2D, ForegroundContent)
{
  return PageControl_IsShown(pPageControl);
}

HANDLER(PageControl, Node2D, DrawForeground)
{
  if (!PageControl_IsShown(pPageControl)) return FALSE;
  struct Node2D *view = GetNode2D(hObject);
  struct rect bounds = view->_rect;
  int pages = pPageControl->NumberOfPages;
  float x = bounds.x + (bounds.width - PageControl_RowWidth(pages)) / 2;
  float y = bounds.y + (bounds.height - DOT) / 2;
  struct BrushShorthand const other = { .Color = pPageControl->PageIndicatorColor };
  bool_t tinted = pPageControl->PageIndicatorColor.a > 0;
  struct ViewEntity current, rest;
  Node2D_GetViewEntity(view, &current, NULL, &pDrawForeground->brush);
  Node2D_GetViewEntity(view, &rest, NULL, tinted ? &other : &pDrawForeground->brush);
  if (!tinted) rest.material.opacity *= INACTIVE_OPACITY;
  current.radius = rest.radius = (struct vec4){ DOT / 2, DOT / 2, DOT / 2, DOT / 2 };
  for (int i = 0; i < pages; i++) {
    bool_t on = i == pPageControl->CurrentPage;
    struct ViewEntity *entity = on ? &current : &rest;
    float width = on ? CURRENT : DOT;
    struct rect dot = { x, y, width, DOT };
    entity->bbox = BOX3_FromRect(dot);
    R_DrawEntity(pDrawForeground->viewdef, entity);
    x += width + GAP;
  }
  return TRUE;
}
