#include <plugins/UIKit/UIKit.h>

#define PAGE_HISTORY_MAX 32

//static struct Object *
//_GetActivePage(struct Object *hObject, struct PageHost *pPageHost)
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
_SetActivePage(struct PageHost *pPageHost, struct Page *pPage)
{
  if (pPageHost->ActivePage) {
    pPageHost->ActivePage->_node->Visible = FALSE;
  }
  if ((pPageHost->ActivePage = pPage)) {
    pPage->_node->Visible = TRUE;
  }
}

static struct Page *
PageHost_FindPageByPath(struct Object *hObject, const char* path)
{
  FOR_EACH_OBJECT(hChild, hObject) {
    struct Page *pPage = GetPage(hChild);
    if (pPage && pPage->Path && strcmp(pPage->Path, path) == 0) {
      return pPage;
    }
    struct Page *pFound = PageHost_FindPageByPath(hChild, path);
    if (pFound) return pFound;
  }
  return NULL;
}

HANDLER(PageHost, PageHost, NavigateToPage) {
  struct Page *pTarget = PageHost_FindPageByPath(hObject, pNavigateToPage->URL);
  if (!pTarget) {
    Con_Error("Page not found: %s", pNavigateToPage->URL);
    return FALSE;
  }
  if (!CMP_GetObject(pTarget)) {
    Con_Error("Page object not found for path: %s", pNavigateToPage->URL);
    return FALSE;
  }
  if (pPageHost->ActivePage && pPageHost->_historySize < PAGE_HISTORY_MAX) {
    pPageHost->_historyStack[pPageHost->_historySize++] = pPageHost->ActivePage;
  }
  _SetActivePage(pPageHost, pTarget);
  return TRUE;
}

HANDLER(PageHost, PageHost, NavigateBack) {
  if (pPageHost->_historySize <= 0) return FALSE;

  struct Page *pPrev = pPageHost->_historyStack[--pPageHost->_historySize];
  _SetActivePage(pPageHost, pPrev);

  return TRUE;
}

HANDLER(PageHost, Node, ViewDidLoad) {
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

HANDLER(Page, Object, Create) {
  pPage->_node = GetNode(hObject);
  return FALSE;
}
