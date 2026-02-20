#include <source/UIKit/UIKit.h>

static lpObject_t
_GetActivePage(lpObject_t hObject, PageHostPtr pPageHost)
{
  if (pPageHost->ActivePage) {
    return CMP_GetObject(pPageHost->ActivePage);
  }
  FOR_EACH_OBJECT(hChild, hObject) {
    if (GetPage(hChild)) {
      return hChild;
    }
  }
  return NULL;
}

HANDLER(PageHost, NavigateToPage) {
  lpObject_t hTarget = CMP_GetObject(pNavigateToPage->TargetPage);
  if (!hTarget) return FALSE;

  if (pPageHost->ActivePage) {
    pPageHost->ActivePage->IsActive = FALSE;
  }

  pPageHost->ActivePage = pNavigateToPage->TargetPage;
  pNavigateToPage->TargetPage->IsActive = TRUE;

  return TRUE;
}

HANDLER(PageHost, UpdateLayout) {
  Node2DPtr pNode2D = GetNode2D(hObject);
  if (!pNode2D) return FALSE;

  FOR_LOOP(i, 2) {
    Node2D_Measure(pNode2D, i, pUpdateLayout->Size[i], TRUE);
  }

  lpObject_t hActivePage = _GetActivePage(hObject, pPageHost);
  if (hActivePage && GetNode2D(hActivePage)) {
    OBJ_SendMessageW(hActivePage, kEventUpdateLayout, 0,
      &(UPDATELAYOUTSTRUCT){
        .Width = Node2D_GetSize(pNode2D, kDirectionHorizontal, kSizingMinusPadding) - TOTAL_MARGIN(GetNode2D(hActivePage), 0),
        .Height = Node2D_GetSize(pNode2D, kDirectionVertical, kSizingMinusPadding) - TOTAL_MARGIN(GetNode2D(hActivePage), 1),
        .Force = pUpdateLayout->Force,
      });
    FOR_LOOP(i, 2) {
      Node2D_Arrange(GetNode2D(hActivePage), Node2D_GetBounds(pNode2D, i), i);
    }
  }

  return TRUE;
}

HANDLER(PageHost, HitTest) {
  Node2DPtr pNode2D = GetNode2D(hObject);
  if (!pNode2D) return FALSE;

  if (OBJ_IsHidden(hObject) || pNode2D->IgnoreHitTest) {
    return FALSE;
  }

  int16_t x = LOWORD(wParam);
  int16_t y = HIWORD(wParam);
  int16_t lx = x - pNode2D->ContentOffset.x;
  int16_t ly = y - pNode2D->ContentOffset.y;

  lpObject_t hActivePage = _GetActivePage(hObject, pPageHost);
  if (hActivePage) {
    lpObject_t hittest = NULL;
    if (OBJ_SendMessageW(hActivePage, kEventHitTest, MAKEDWORD(lx, ly), &hittest)) {
      *pHitTest = hittest;
      return TRUE;
    }
  }

  struct mat4 inv = MAT4_Inverse(&pNode2D->Matrix);
  struct vec3 point = { x, y, 0 };
  float w = Node2D_GetFrame(pNode2D, kBox3FieldWidth);
  float h = Node2D_GetFrame(pNode2D, kBox3FieldHeight);
  struct vec3 out = MAT4_MultiplyVector3D(&inv, &point);
  if (RECT_Contains(&(struct rect){ 0, 0, w, h }, (struct vec2 const*)&out)) {
    *pHitTest = hObject;
    return TRUE;
  }

  return FALSE;
}
