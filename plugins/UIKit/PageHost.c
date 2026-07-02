#include <UIKit/UIKit.h>
#include <filesystem/filesystem.h>

#define PAGE_HISTORY_MAX 32

static void
_SetActivePage(struct PageHost *pPageHost, struct Page *pPage)
{
  if (pPageHost->_activePage) {
    pPageHost->_activePage->_node->Visible = FALSE;
  }
  if ((pPageHost->_activePage = pPage)) {
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
    if (pPage && OBJ_GetName(hChild) && strcmp(OBJ_GetName(hChild), key) == 0) {
      return pPage;
    }
  }
  return NULL;
}

static void
PageHost_SyncActivePage(struct Object *hObject, struct PageHost *pPageHost)
{
  if (!pPageHost->ActivePage || !pPageHost->ActivePage[0]) {
    return;
  }
  struct Page *pTarget = PageHost_FindPageByKey(hObject, pPageHost->ActivePage);
  if (pTarget && pTarget != pPageHost->_activePage) {
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
  if (pPageHost->_activePage && pPageHost->_historySize < PAGE_HISTORY_MAX) {
    pPageHost->_historyStack[pPageHost->_historySize++] = pPageHost->_activePage;
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

// PageHost_Start — sync initial ActivePage if set before ViewDidLoad
HANDLER(PageHost, Object, Start) {
  PageHost_SyncActivePage(hObject, pPageHost);
  return FALSE;
}

// PageHost_PropertyChanged — watch ActivePage for binding-driven navigation
HANDLER(PageHost, Object, PropertyChanged) {
  if (!pPropertyChanged->Property)
    return FALSE;

  if (PROP_GetLongIdentifier(pPropertyChanged->Property) == ID_PageHost_ActivePage) {
    PageHost_SyncActivePage(hObject, pPageHost);
  }

  return FALSE;
}

// PageHost_ViewDidLoad
HANDLER(PageHost, Node, ViewDidLoad) {
  FOR_EACH_OBJECT(hChild, hObject) {
    if (pPageHost->_activePage == NULL) {
      pPageHost->_activePage = GetPage(hChild);
      GetNode(hChild)->Visible = TRUE;
    } else if (pPageHost->_activePage == GetPage(hChild)) {
      GetNode(hChild)->Visible = TRUE;
    } else {
      GetNode(hChild)->Visible = FALSE;
    }
  }
  // If ActivePage was set (e.g. via binding) but _activePage wasn't resolved yet
  PageHost_SyncActivePage(hObject, pPageHost);
  return TRUE;
}

// Page_Create
HANDLER(Page, Object, Create) {
  pPage->_node = GetNode(hObject);
  return FALSE;
}

// Page_Start — load page content from Source if set
HANDLER(Page, Object, Start) {
  if (pPage->Source && pPage->Source[0]) {
    struct Object *content = FS_LoadObjectFromXml(pPage->Source);
    if (content) {
      OBJ_AddChild(hObject, content);
    }
  }
  return FALSE;
}
