#include <include/orca.h>
#include <source/core/core_local.h>

#include <plugins/UIKit/UIKit.h>

HANDLER(TabView, Node2D, MeasureOverride)
{
  uint16_t width = 0;
  uint16_t headerHeight = 0;
  uint16_t contentHeight = 0;

  FOR_EACH_OBJECT(hChild, hObject) {
    LRESULT size = _SendMessage(hChild, Node2D, Measure,
      .Width = pMeasureOverride->Width,
      .Height = pMeasureOverride->Height);

    width = MAX(width, LOWORD(size));
    if (GetTabBar(hChild)) {
      headerHeight = MAX(headerHeight, HIWORD(size));
    } else {
      contentHeight = MAX(contentHeight, HIWORD(size));
    }
  }

  return MAKEDWORD(width, headerHeight + contentHeight);
}

HANDLER(TabView, Node2D, ArrangeOverride)
{
  float headerHeight = 0;

  FOR_EACH_OBJECT(hChild, hObject) {
    if (!GetTabBar(hChild)) continue;

    struct Node2D *node = GetNode2D(hChild);
    float height = node ? NODE2D_FRAME(node, Size, 1).Desired : 0;
    _SendMessage(hChild, Node2D, Arrange,
      .X = pArrangeOverride->X,
      .Y = pArrangeOverride->Y,
      .Width = pArrangeOverride->Width,
      .Height = height);
    headerHeight = MAX(headerHeight, height);
  }

  float contentY = pArrangeOverride->Y + headerHeight;
  float contentHeight = MAX(pArrangeOverride->Height - headerHeight, 0);

  FOR_EACH_OBJECT(hChild, hObject) {
    if (GetTabBar(hChild)) continue;

    _SendMessage(hChild, Node2D, Arrange,
      .X = pArrangeOverride->X,
      .Y = contentY,
      .Width = pArrangeOverride->Width,
      .Height = contentHeight);
  }

  return MAKEDWORD(pArrangeOverride->Width, pArrangeOverride->Height);
}

/* Show the content child whose name matches `value`; hide all others.
   Content children are direct children that are NOT a TabBar. */
static void
TabView_ShowContent(struct Object *hObject, struct TabView *pTabView,
                    const char *value)
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

  pTabBar->SelectedValue = pFirstTab->Value;
  pTabView->SelectedValue = pFirstTab->Value;

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

  TabView_ShowContent(hObject, pTabView, pTabView->SelectedValue);
  return FALSE;
}

/* Intercept SelectionChanged bubbling up from the TabBar child. */
HANDLER(TabView, TabBar, SelectionChanged)
{
  const char *oldValue = pTabView->SelectedValue;
  pTabView->SelectedValue = pSelectionChanged->SelectedValue;

  TabView_ShowContent(hObject, pTabView, pTabView->SelectedValue);
  OBJ_SetDirty(hObject);

  struct TabView_SelectionChangedEventArgs args = {
    .SelectedValue = pTabView->SelectedValue,
    .OldValue      = oldValue,
  };
  CORE_HandleObjectMessage(core.L, &(struct AXmessage){
    .target  = hObject,
    .message = ID_TabView_SelectionChanged,
    .lParam  = &args,
  });

  return TRUE;
}
