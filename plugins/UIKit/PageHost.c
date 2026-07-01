#include <UIKit/UIKit.h>

#define PAGE_HISTORY_MAX 32

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

static struct Page *
PageHost_FindPageByKey(struct Object *hObject, const char* key)
{
  FOR_EACH_OBJECT(hChild, hObject) {
    struct Page *pPage = GetPage(hChild);
    if (pPage && pPage->Key && strcmp(pPage->Key, key) == 0) {
      return pPage;
    }
  }
  return NULL;
}

static void
PageHost_SyncToCurrentPage(struct Object *hObject, struct PageHost *pPageHost)
{
  if (!pPageHost->CurrentPage || !pPageHost->CurrentPage[0]) {
    return;
  }
  struct Page *pTarget = PageHost_FindPageByKey(hObject, pPageHost->CurrentPage);
  if (pTarget && pTarget != pPageHost->ActivePage) {
    _SetActivePage(pPageHost, pTarget);
  }
}

// PageHost_NavigateToPage
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

// PageHost_NavigateBack
HANDLER(PageHost, PageHost, NavigateBack) {
  if (pPageHost->_historySize <= 0) return FALSE;

  struct Page *pPrev = pPageHost->_historyStack[--pPageHost->_historySize];
  _SetActivePage(pPageHost, pPrev);

  return TRUE;
}

// PageHost_Start — sync initial CurrentPage if set before ViewDidLoad
HANDLER(PageHost, Object, Start) {
  PageHost_SyncToCurrentPage(hObject, pPageHost);
  return FALSE;
}

// PageHost_PropertyChanged — watch CurrentPage for binding-driven navigation
HANDLER(PageHost, Object, PropertyChanged) {
  if (!pPropertyChanged->Property)
    return FALSE;

  if (PROP_GetLongIdentifier(pPropertyChanged->Property) == ID_PageHost_CurrentPage) {
    PageHost_SyncToCurrentPage(hObject, pPageHost);
  }

  return FALSE;
}

// PageHost_ViewDidLoad
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
  // If CurrentPage was set (e.g. via binding) but ActivePage wasn't resolved yet
  PageHost_SyncToCurrentPage(hObject, pPageHost);
  return TRUE;
}

// Page_Create
HANDLER(Page, Object, Create) {
  pPage->_node = GetNode(hObject);
  return FALSE;
}
