#include <source/UIKit/UIKit.h>

#define PAGE_HISTORY_MAX 32

//static lpObject_t
//_GetActivePage(lpObject_t hObject, PageHostPtr pPageHost)
//{
//  if (pPageHost->ActivePage) {
//    return CMP_GetObject(pPageHost->ActivePage);
//  }
//  FOR_EACH_OBJECT(hChild, hObject) {
//    if (GetPage(hChild)) {
//      return hChild;
//    }
//  }
//  return NULL;
//}

static void
_SetActivePage(PageHostPtr pPageHost, lpPage_t pPage)
{
  if (pPageHost->ActivePage) {
    pPageHost->ActivePage->_node->Visible = FALSE;
  }
  if ((pPageHost->ActivePage = pPage)) {
    pPage->_node->Visible = TRUE;
  }
}

HANDLER(PageHost, NavigateToPage) {
  lpPage_t pTarget = pNavigateToPage->TargetPage;
  if (!pTarget || !CMP_GetObject(pTarget)) return FALSE;

  if (pPageHost->ActivePage && pPageHost->_historySize < PAGE_HISTORY_MAX) {
    pPageHost->_historyStack[pPageHost->_historySize++] = pPageHost->ActivePage;
  }

  _SetActivePage(pPageHost, pTarget);

  return TRUE;
}

HANDLER(PageHost, NavigateBack) {
  if (pPageHost->_historySize <= 0) return FALSE;

  lpPage_t pPrev = pPageHost->_historyStack[--pPageHost->_historySize];
  _SetActivePage(pPageHost, pPrev);

  return TRUE;
}

HANDLER(PageHost, ViewDidLoad) {
  if (hObject != pViewDidLoad) {
    return FALSE;
  }
  FOR_EACH_OBJECT(hChild, hObject) {
    if (pPageHost->ActivePage == NULL) {
      pPageHost->ActivePage = GetPage(hChild);
      GetNode(hChild)->Visible = TRUE;
    } else if (pPageHost->ActivePage == GetPage(hChild)) {
      GetNode(hChild)->Visible = TRUE;
    } else {
      GetNode(hChild)->Visible = FALSE;
    }
  }
  return TRUE;
}

HANDLER(Page, Create) {
  pPage->_node = GetNode(hObject);
  return FALSE;
}
