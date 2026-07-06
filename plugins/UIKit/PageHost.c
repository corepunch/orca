#include <UIKit/UIKit.h>
#include <filesystem/filesystem.h>

#define PAGE_HISTORY_MAX 32
#define NAVIGATION_STACK_MAX 32

static void
_SetActivePage(struct Object *hObject, struct PageHost *pPageHost, struct Page *pPage)
{
  pPageHost->_activePage = pPage;

  FOR_EACH_OBJECT(hChild, hObject) {
    struct Page *childPage = GetPage(hChild);
    struct Node *childNode = GetNode(hChild);
    if (!childPage || !childNode) continue;

    childNode->Visible = (childPage == pPage);
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
  if (pTarget) {
    _SetActivePage(hObject, pPageHost, pTarget);
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
  if (pNavigateToPage->DataContext) {
    (GetNode(CMP_GetObject(pTarget))->DataContext = pNavigateToPage->DataContext);
  }
  if (pPageHost->_activePage && pPageHost->_historySize < PAGE_HISTORY_MAX) {
    pPageHost->_historyStack[pPageHost->_historySize++] = pPageHost->_activePage;
  }
  _SetActivePage(hObject, pPageHost, pTarget);
  return TRUE;
}

// PageHost_NavigateBack
HANDLER(PageHost, PageHost, NavigateBack) {
  if (pPageHost->_historySize <= 0) return FALSE;

  struct Page *pPrev = pPageHost->_historyStack[--pPageHost->_historySize];
  _SetActivePage(hObject, pPageHost, pPrev);

  return TRUE;
}

static void
NavigationHost_UpdateCanGoBack(struct Object *hObject, struct NavigationHost *pNavigationHost)
{
  bool_t canGoBack = pNavigationHost->_stackSize > 1;
  struct Property *prop = OBJ_FindLongProperty(hObject, ID_NavigationHost_CanGoBack);
  if (prop) PROP_SetValue(prop, &canGoBack);
}

static void
NavigationHost_UpdateTitle(struct Object *hObject, struct NavigationHost *pNavigationHost)
{
  struct Page *active = pNavigationHost->_stackSize > 0
    ? pNavigationHost->_stack[pNavigationHost->_stackSize - 1] : NULL;
  const char *title = (active && active->Title) ? active->Title : "";
  struct Property *prop = OBJ_FindLongProperty(hObject, ID_NavigationHost_CurrentPageTitle);
  if (prop) PROP_SetValue(prop, &title);
}

static void
NavigationHost_InitRoot(struct Object *hObject, struct NavigationHost *pNavigationHost)
{
  if (pNavigationHost->_stackSize) return;
  FOR_EACH_OBJECT(child, hObject) {
    struct Page *page = GetPage(child);
    if (!page) continue;
    pNavigationHost->_stack[pNavigationHost->_stackSize++] = page;
    break;
  }
  NavigationHost_UpdateCanGoBack(hObject, pNavigationHost);
  NavigationHost_UpdateTitle(hObject, pNavigationHost);
}

HANDLER(NavigationHost, Node, ViewDidLoad) {
  NavigationHost_InitRoot(hObject, pNavigationHost);
  return FALSE;
}

HANDLER(NavigationHost, NavigationHost, Push) {
  Con_Printf("NavigationHost.Push: path=%s", pPush->Path ? pPush->Path : "(null)");
  if (!pPush->Path || !*pPush->Path) return Con_Error("NavigationHost.Push missing Path"), FALSE;
  NavigationHost_InitRoot(hObject, pNavigationHost);
  if (pNavigationHost->_stackSize >= NAVIGATION_STACK_MAX)
    return Con_Error("NavigationHost stack is full"), FALSE;

  struct Object *target = FS_LoadObject(pPush->Path);
  if (!target) return Con_Error("NavigationHost.Push failed to load: %s", pPush->Path), FALSE;
  struct Page *page = GetPage(target);
  if (!page) {
    OBJ_ReleaseRef(target);
    return Con_Error("NavigationHost.Push template is not a Page: %s", pPush->Path), FALSE;
  }
  pPush->DataContext && (GetNode(target)->DataContext = pPush->DataContext);
  OBJ_AddChild(hObject, target);
  OBJ_ReleaseRef(target);
  pNavigationHost->_stack[pNavigationHost->_stackSize++] = page;
  _SetActivePage(hObject, GetPageHost(hObject), page);
  NavigationHost_UpdateCanGoBack(hObject, pNavigationHost);
  NavigationHost_UpdateTitle(hObject, pNavigationHost);
  Con_Printf("NavigationHost.Push: pushed %s (stack size=%d)", pPush->Path, pNavigationHost->_stackSize);
  return TRUE;
}

HANDLER(NavigationHost, NavigationHost, Pop) {
  NavigationHost_InitRoot(hObject, pNavigationHost);
  if (pNavigationHost->_stackSize <= 1) return FALSE;

  struct Page *current = pNavigationHost->_stack[--pNavigationHost->_stackSize];
  struct Page *previous = pNavigationHost->_stack[pNavigationHost->_stackSize - 1];
  pNavigationHost->_stack[pNavigationHost->_stackSize] = NULL;
  _SetActivePage(hObject, GetPageHost(hObject), previous);
  OBJ_RemoveFromParent(CMP_GetObject(current));
  NavigationHost_UpdateCanGoBack(hObject, pNavigationHost);
  NavigationHost_UpdateTitle(hObject, pNavigationHost);
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
