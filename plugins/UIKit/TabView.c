#include <include/orca.h>
#include <source/core/core_local.h>

#include <plugins/UIKit/UIKit.h>

/* Show the content child whose name matches `value`; hide all others.
   Content children are direct children that are NOT a TabBar. */
static void
TabView_ShowContent(struct Object *hObject, const char *value)
{
  FOR_EACH_OBJECT(hChild, hObject) {
    if (GetTabBar(hChild)) continue;

    struct Node *pNode = GetNode(hChild);
    if (!pNode) continue;

    const char *name = OBJ_GetName(hChild);
    bool_t show = (value && name && strcmp(name, value) == 0);
    pNode->Visible = show;
    OBJ_SetDirty(hChild);
  }
}

/* On initial load: select first tab in the first TabBar child and
   show its matching content panel. */
HANDLER(TabView, Node, ViewDidLoad)
{
  struct Object *hTabBar = NULL;
  FOR_EACH_OBJECT(hChild, hObject) {
    if (GetTabBar(hChild)) { hTabBar = hChild; break; }
  }

  if (!hTabBar) return FALSE;

  /* Find the first Tab child of the TabBar */
  struct Object *hFirstTab = NULL;
  FOR_EACH_OBJECT(hChild, hTabBar) {
    if (GetTab(hChild)) { hFirstTab = hChild; break; }
  }

  if (!hFirstTab) return FALSE;

  struct Tab *pFirstTab = GetTab(hFirstTab);
  struct TabBar *pTabBar = GetTabBar(hTabBar);

  /* Set SelectedValue via the property system so each component owns
     its own strdup'd copy and we don't create cross-property aliases. */
  struct Property *tb_prop = NULL;
  OBJ_FindLongProperty(hTabBar, ID_TabBar_SelectedValue, &tb_prop);
  if (tb_prop) PROP_SetStringValue(tb_prop, pFirstTab->Value);
  struct Property *tv_prop = NULL;
  OBJ_FindLongProperty(hObject, ID_TabView_SelectedValue, &tv_prop);
  if (tv_prop) PROP_SetStringValue(tv_prop, pFirstTab->Value);

  /* Mark first tab selected, all others deselected */
  bool_t first = TRUE;
  FOR_EACH_OBJECT(hChild, hTabBar) {
    struct Tab *t = GetTab(hChild);
    if (!t) continue;
    t->IsSelected = first;
    uint32_t flags = OBJ_GetFlags(hChild);
    flags = first ? (flags | OF_SELECTED) : (flags & ~OF_SELECTED);
    OBJ_SetFlags(hChild, flags);
    OBJ_SetDirty(hChild);
    first = FALSE;
  }

  TabView_ShowContent(hObject, pTabView->SelectedValue);
  return FALSE;
}

/* Intercept SelectionChanged bubbling up from the TabBar child. */
HANDLER(TabView, TabBar, SelectionChanged)
{
  /* Capture a copy of the old value before overwriting it via the property
     system.  PROP_SetStringValue will free the old string, so we must save
     our own copy first to keep the event-args pointer valid for the call. */
  char *savedOld = pTabView->SelectedValue ? strdup(pTabView->SelectedValue) : NULL;

  struct Property *tv_prop = NULL;
  OBJ_FindLongProperty(hObject, ID_TabView_SelectedValue, &tv_prop);
  if (tv_prop) PROP_SetStringValue(tv_prop, pSelectionChanged->SelectedValue);

  TabView_ShowContent(hObject, pTabView->SelectedValue);
  OBJ_SetDirty(hObject);

  struct TabView_SelectionChangedEventArgs args = {
    .SelectedValue = pTabView->SelectedValue,  /* strdup'd by PROP_SetStringValue */
    .OldValue      = savedOld,                 /* our own strdup copy */
  };
  CORE_HandleObjectMessage(core.L, &(struct AXmessage){
    .target  = hObject,
    .message = ID_TabView_SelectionChanged,
    .lParam  = &args,
  });
  /* savedOld is intentionally not freed here: the Lua event-args userdata
     may hold a reference across async boundaries (production event-loop).
     This is a small, bounded per-selection allocation. */

  return TRUE;
}
