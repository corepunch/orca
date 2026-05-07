#include <include/orca.h>
#include <source/core/core_local.h>
#include <source/filesystem/filesystem.h>
#include <source/filesystem/theme_palette.h>

#include <plugins/UIKit/UIKit.h>
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
  struct Property *prop = NULL;
  OBJ_FindLongProperty(bar, ID_TabBar_SelectedValue, &prop);
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
  CORE_HandleObjectMessage(core.L, &(struct AXmessage){
    .target  = bar,
    .message = ID_TabBar_SelectionChanged,
    .lParam  = &args,
  });
  /* savedOld is intentionally not freed here: the Lua event-args userdata
     may hold a reference across async boundaries (production event-loop).
     This is a small, bounded per-selection allocation. */
  return TRUE;
}

static void
Tab_ApplyThemeDefaults(struct Tab *tab)
{
  tab->SelectedColor = FS_GetThemeColor(
    THEME_COLOR_ACCENT_BACKGROUND,
    FS_GetThemeColor(
      THEME_COLOR_ACCENT,
      (struct color){0.24f, 0.36f, 0.58f, 1.0f}));
  tab->UnselectedColor = FS_GetThemeColor(
    THEME_COLOR_CONTROL_BACKGROUND,
    FS_GetThemeColor(
      THEME_COLOR_CARD_BACKGROUND,
      (struct color){0.18f, 0.19f, 0.22f, 0.95f}));
}

HANDLER(Tab, Object, Create)
{
  struct TextBlockConcept *text = GetTextBlockConcept(hObject);
  OBJ_SetStyle(hObject, OBJ_GetStyle(hObject) | OF_TABSTOP);
  struct Property *prop = PROP_FindByLongID(OBJ_GetProperties(hObject), ID_Tab_IsSelected);
  if (prop) PROP_SetFlag(prop, PF_USED_IN_TRIGGER);

  if (text) {
    text->TextHorizontalAlignment = kTextHorizontalAlignmentCenter;
    text->TextVerticalAlignment = kTextVerticalAlignmentCenter;
  }

  Tab_ApplyThemeDefaults(pTab);
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

HANDLER(Tab, Node2D, DrawBrush)
{
  if (pDrawBrush->foreground) return FALSE;

  struct Node2D *pNode2D = GetNode2D(hObject);
  if (!pNode2D) return FALSE;

  struct ViewEntity entity;
  Node2D_GetViewEntity(pNode2D, &entity, NULL, &pDrawBrush->brush);
  /* Keep the tab background anchored to the control bounds. Text alignment
     only affects the text layout rect, not the tab surface itself. */
  entity.bbox = BOX3_FromRect(Node2D_GetBackgroundRect(pNode2D));

  float w = Node2D_GetFrame(pNode2D, kBox3FieldWidth);
  float h = Node2D_GetFrame(pNode2D, kBox3FieldHeight);
  entity.bbox.max.x = entity.bbox.min.x + w;
  entity.bbox.max.y = entity.bbox.min.y + h;

  entity.radius = (struct vec4){4, 4, 0, 0};
  entity.material = (struct ViewMaterial){
    .color   = pTab->IsSelected ? pTab->SelectedColor : pTab->UnselectedColor,
    .opacity = pNode2D->_opacity,
    .blendMode = BLEND_MODE_ALPHA,
  };

  R_DrawEntity(pDrawBrush->viewdef, &entity);

  if (pTab->IsSelected) {
    /* Draw a bottom highlight bar to visually connect the tab to the content */
    struct ViewEntity bar = entity;
    float barH = 3.0f;
    bar.bbox.min.y = entity.bbox.max.y - barH;
    bar.bbox.max.y = entity.bbox.max.y;
    bar.radius = (struct vec4){0, 0, 0, 0};
    bar.material.color = pTab->SelectedColor;
    R_DrawEntity(pDrawBrush->viewdef, &bar);
  }

  return TRUE;
}
