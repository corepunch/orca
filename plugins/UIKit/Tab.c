#include <include/orca.h>
#include <source/core/core_local.h>
#include <filesystem/filesystem.h>

#include <UIKit/UIKit.h>
#include <plugins/UIKit/UIKit_message.h>

static void
Tab_SetSelected(struct Object *object, struct Tab *tab, bool_t selected)
{
  if (!object || !tab)
    return;

  tab->IsSelected = selected;

  uint32_t flags = OBJ_GetFlags(object);
  flags = selected ? (flags | OF_SELECTED) : (flags & ~OF_SELECTED);
  OBJ_SetFlags(object, flags);

  OBJ_SetDirty(object);
  _SendMessage(object, StyleController, ThemeChanged, .recursive = FALSE);
}

static bool_t
Tab_Select(struct Object *object, struct Tab *tab)
{
  struct Object *bar = OBJ_GetParent(object);
  if (!bar || !GetTabBar(bar))
    return TRUE;

  struct TabBar *tb = GetTabBar(bar);

  if (tab->IsSelected)
    return TRUE;

  const char *oldValue = tb->SelectedValue;

  Tab_SetSelected(object, tab, TRUE);

  /* Set SelectedValue via the property system to avoid cross-property string
     aliasing.  PROP_SetStringValue strdup's tab->Value into the TabBar's own
     property storage, so a later change to Tab.Value won't affect SelectedValue.
     We must capture a copy of oldValue first because PROP_SetStringValue frees
     the previous string. */
  char *savedOld = oldValue ? strdup(oldValue) : NULL;
  struct Property *prop = OBJ_FindLongProperty(bar, ID_TabBar_SelectedValue);
  if (prop) PROP_SetStringValue(prop, tab->Value);
  OBJ_SetDirty(bar);

  FOR_EACH_OBJECT(child, bar) {
    if (child == object) continue;
    struct Tab *t = GetTab(child);
    if (t && t->IsSelected) {
      Tab_SetSelected(child, t, FALSE);
    }
  }

  struct TabBar_SelectionChangedEventArgs args = {
    .SelectedValue = tb->SelectedValue,  /* strdup'd by PROP_SetStringValue */
    .OldValue      = savedOld,           /* our own strdup copy */
  };
  axPostMessageDataW(bar, ID_TabBar_SelectionChanged, 0, &args, sizeof(args));
  /* savedOld is intentionally not freed here: the Lua event-args userdata
     may hold a reference across async boundaries (production event-loop).
     This is a small, bounded per-selection allocation. */
  return TRUE;
}

HANDLER(Tab, Object, Create)
{
  OBJ_SetStyle(hObject, OBJ_GetStyle(hObject) | OF_TABSTOP);
  struct Property *prop = OBJ_FindLongProperty(hObject, ID_Tab_IsSelected);
  if (prop) PROP_SetFlag(prop, PF_USED_IN_TRIGGER);

  Tab_SetSelected(hObject, pTab, pTab->IsSelected);
  return FALSE;
}

HANDLER(Tab, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property)
    return FALSE;

  if (PROP_GetLongIdentifier(pPropertyChanged->Property) == ID_Tab_IsSelected) {
    Tab_SetSelected(hObject, pTab, pTab->IsSelected);
  }

  return FALSE;
}

HANDLER(Tab, Node, LeftButtonUp)
{
  return Tab_Select(hObject, pTab);
}

