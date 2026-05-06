#include <include/orca.h>
#include <source/core/core_local.h>

#include <plugins/UIKit/UIKit.h>
#include <plugins/UIKit/UIKit_message.h>

static struct Object *
TabView_GetActiveContent(struct Object *hObject, struct TabView *pTabView)
{
  struct Object *hFallback = NULL;
  struct Object *hVisible = NULL;
  struct Object *hSelected = NULL;

  FOR_EACH_OBJECT(hChild, hObject) {
    if (GetTabBar(hChild)) continue;

    if (!hFallback) {
      hFallback = hChild;
    }

    if (!hSelected && pTabView->SelectedValue) {
      const char *name = OBJ_GetName(hChild);
      if (name && strcmp(name, pTabView->SelectedValue) == 0) {
        hSelected = hChild;
      }
    }

    if (!hVisible) {
      struct Node *node = GetNode(hChild);
      if (node && node->Visible) {
        hVisible = hChild;
      }
    }
  }

  return hSelected ? hSelected : (hVisible ? hVisible : hFallback);
}

HANDLER(TabView, Node2D, MeasureOverride)
{
  uint16_t width = 0;
  uint16_t headerHeight = 0;
  uint16_t contentWidth = 0;
  uint16_t contentHeight = 0;
  struct Object *hContent = TabView_GetActiveContent(hObject, pTabView);

  FOR_EACH_OBJECT(hChild, hObject) {
    if (GetTabBar(hChild)) {
      LRESULT size = _SendMessage(hChild, Node2D, Measure,
        .Width = pMeasureOverride->Width,
        .Height = pMeasureOverride->Height);

      width = MAX(width, LOWORD(size));
      headerHeight = MAX(headerHeight, HIWORD(size));
      continue;
    }

    if (hContent && hChild == hContent) {
      LRESULT size = _SendMessage(hChild, Node2D, Measure,
        .Width = pMeasureOverride->Width,
        .Height = pMeasureOverride->Height);

      contentWidth = LOWORD(size);
      contentHeight = HIWORD(size);
      width = MAX(width, contentWidth);
    }
  }

  return MAKEDWORD(width, headerHeight + contentHeight);
}

HANDLER(TabView, Node2D, ArrangeOverride)
{
  float headerHeight = 0;
  struct Object *hContent = TabView_GetActiveContent(hObject, pTabView);

  FOR_EACH_OBJECT(hChild, hObject) {
    if (!GetTabBar(hChild)) continue;

    LRESULT size = _SendMessage(hChild, Node2D, Arrange,
      .X = pArrangeOverride->X,
      .Y = pArrangeOverride->Y,
      .Width = pArrangeOverride->Width,
      .Height = pArrangeOverride->Height);
    headerHeight = MAX(headerHeight, HIWORD(size));
  }

  float contentY = pArrangeOverride->Y + headerHeight;
  float contentHeight = MAX(pArrangeOverride->Height - headerHeight, 0);

  if (hContent) {
    _SendMessage(hContent, Node2D, Arrange,
      .X = pArrangeOverride->X,
      .Y = contentY,
      .Width = pArrangeOverride->Width,
      .Height = contentHeight);
  }

  return MAKEDWORD(pArrangeOverride->Width, pArrangeOverride->Height);
}

static void
TabView_ShowContent(struct Object *hObject, const char *value);

static void
TabView_SyncInitialSelection(struct Object *hObject, struct TabView *pTabView)
{
  struct Object *hTabBar = NULL;
  FOR_EACH_OBJECT(hChild, hObject) {
    if (GetTabBar(hChild)) { hTabBar = hChild; break; }
  }

  if (!hTabBar) return;

  struct Object *hFirstTab = NULL;
  struct Object *hSelectedTab = NULL;

  FOR_EACH_OBJECT(hChild, hTabBar) {
    struct Tab *t = GetTab(hChild);
    if (!t) continue;

    if (!hFirstTab) {
      hFirstTab = hChild;
    }
    if (!hSelectedTab && t->IsSelected) {
      hSelectedTab = hChild;
    }
  }

  if (!hFirstTab) return;

  const char *value = pTabView->SelectedValue;
  if ((!value || !*value) && hSelectedTab) {
    struct Tab *selected = GetTab(hSelectedTab);
    if (selected && selected->Value) {
      value = selected->Value;
    }
  }
  if (!value || !*value) {
    struct Tab *first = GetTab(hFirstTab);
    if (first && first->Value) {
      value = first->Value;
    }
  }
  if (!value || !*value) return;

  struct Property *tb_prop = NULL;
  OBJ_FindLongProperty(hTabBar, ID_TabBar_SelectedValue, &tb_prop);
  if (tb_prop) PROP_SetStringValue(tb_prop, value);

  struct Property *tv_prop = NULL;
  OBJ_FindLongProperty(hObject, ID_TabView_SelectedValue, &tv_prop);
  if (tv_prop) PROP_SetStringValue(tv_prop, value);

  struct Object *hActiveTab = NULL;
  FOR_EACH_OBJECT(hChild, hTabBar) {
    struct Tab *t = GetTab(hChild);
    if (!t) continue;

    bool_t selected = t->Value && strcmp(t->Value, value) == 0;
    if (selected) {
      hActiveTab = hChild;
    }
    t->IsSelected = selected;
    uint32_t flags = OBJ_GetFlags(hChild);
    flags = selected ? (flags | OF_SELECTED) : (flags & ~OF_SELECTED);
    OBJ_SetFlags(hChild, flags);
    OBJ_SetDirty(hChild);
    _SendMessage(hChild, StyleController, ThemeChanged, .recursive = FALSE);
  }

  if (hActiveTab) {
    OBJ_SetFocus(hActiveTab);
  }

  TabView_ShowContent(hObject, value);
}

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
HANDLER(TabView, Object, Start)
{
  TabView_SyncInitialSelection(hObject, pTabView);
  return FALSE;
}

HANDLER(TabView, Object, Attached)
{
  TabView_SyncInitialSelection(hObject, pTabView);
  return FALSE;
}

HANDLER(TabView, Node, ViewDidLoad)
{
  TabView_SyncInitialSelection(hObject, pTabView);
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
