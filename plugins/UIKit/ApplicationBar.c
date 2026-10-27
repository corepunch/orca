#include "metro.h"

// ApplicationBar layout, after the Windows Phone 8 application bar.
#define APPBAR_HEIGHT        58.0f
#define APPBAR_MINI_HEIGHT   24.0f
#define APPBAR_RING          38.0f
#define APPBAR_RING_BORDER   2.0f
#define APPBAR_ICON          18.0f
#define APPBAR_SLOT          78.0f
#define APPBAR_PAD           10.0f   // above the buttons when expanded
#define APPBAR_LABEL_SIZE    11.0f
#define APPBAR_LABEL_GAP     5.0f
#define APPBAR_LABEL_HEIGHT  15.0f
#define APPBAR_MENU_SIZE     20.0f
#define APPBAR_MENU_ITEM     46.0f
#define APPBAR_DOT           4.0f
#define APPBAR_DOTS          3

static struct color
AppBar_Foreground(struct Object *bar)
{
  struct Node2D *n = bar ? GetNode2D(bar) : NULL;
  return n ? n->Foreground.Color : (struct color){1, 1, 1, 1};
}

static void
AppBar_Round(struct Object *o, float radius)
{
  struct Node *n = GetNode(o);
  n->Border.Radius = (struct CornerRadius){ radius, radius, radius, radius };
}

static struct Object *
AppBar_Part(struct Object *parent, uint32_t class_id)
{
  struct Object *o = OBJ_Create(class_id);
  GetNode2D(o)->IgnoreHitTest = TRUE;
  OBJ_AddChild(parent, o);
  return o;
}

static void
AppBar_SetColor(struct Object *o, uint32_t id, struct color color)
{
  struct Property *p = OBJ_FindLongProperty(o, id);
  if (p && memcmp(PROP_GetValue(p), &color, sizeof(color))) PROP_SetValue(p, &color);
}

static void
AppBar_Collapse(struct Object *bar)
{
  struct ApplicationBar *pBar = GetApplicationBar(bar);
  if (!pBar || !pBar->IsExpanded) return;
  bool_t value = FALSE;
  Metro_SetProperty(bar, ID_ApplicationBar_IsExpanded, &value);
}

HANDLER(ApplicationBar, Object, Create)
{
  // PhoneChromeBrush and PhoneForegroundBrush for the current appearance.
  bool_t dark = axIsDarkTheme();
  struct color chrome = dark ? (struct color){0.122f, 0.122f, 0.122f, 1} : (struct color){0.867f, 0.867f, 0.867f, 1};
  struct color ink = dark ? (struct color){1, 1, 1, 1} : (struct color){0, 0, 0, 1};
  Metro_SetProperty(hObject, ID_Node2D_BackgroundColor, &chrome);
  Metro_SetProperty(hObject, ID_Node2D_ForegroundColor, &ink);

  pApplicationBar->_scrim = AppBar_Part(hObject, ID_Node2D);
  GetNode2D(pApplicationBar->_scrim)->IgnoreHitTest = FALSE;
  pApplicationBar->_panel = AppBar_Part(hObject, ID_Node2D);
  GetNode2D(pApplicationBar->_panel)->IgnoreHitTest = FALSE;
  pApplicationBar->_ellipsis = AppBar_Part(hObject, ID_Node2D);
  for (int i = 0; i < APPBAR_DOTS; i++) {
    AppBar_Round(AppBar_Part(pApplicationBar->_ellipsis, ID_Node2D), APPBAR_DOT / 2);
  }
  _SendMessage(pApplicationBar->_ellipsis, StyleController, AddClass, .ClassName = "appbar-ellipsis");
  return FALSE;
}

HANDLER(ApplicationBar, Object, PropertyChanged)
{
  OBJ_SetDirty(hObject);
  return FALSE;
}

HANDLER(ApplicationBar, Node2D, MeasureOverride)
{
  FOR_EACH_OBJECT(child, hObject) {
    if (GetApplicationBarIconButton(child) || GetApplicationBarMenuItem(child))
      Metro_Measure(child, pMeasureOverride->Width, APPBAR_MENU_ITEM);
  }
  float height = pApplicationBar->Mode == kApplicationBarModeMinimized ? APPBAR_MINI_HEIGHT : APPBAR_HEIGHT;
  // An auto track can leave width unbounded; keep the packed height intact.
  return MAKEDWORD(isfinite(pMeasureOverride->Width) ? pMeasureOverride->Width : 0, height);
}

HANDLER(ApplicationBar, Node2D, ArrangeOverride)
{
  float x = pArrangeOverride->X, y = pArrangeOverride->Y;
  float w = pArrangeOverride->Width, h = pArrangeOverride->Height;
  bool_t expanded = pApplicationBar->IsExpanded;
  bool_t minimized = pApplicationBar->Mode == kApplicationBarModeMinimized;
  int buttons = 0, menu = 0;
  // Buttons the app hides with Visible="false" leave the row.
  FOR_EACH_OBJECT(child, hObject) {
    buttons += GetApplicationBarIconButton(child) && GetNode(child)->Visible;
    menu += GetApplicationBarMenuItem(child) != NULL;
  }
  bool_t showMenu = expanded && pApplicationBar->IsMenuEnabled && menu > 0;
  float rows = APPBAR_PAD + APPBAR_RING + APPBAR_LABEL_GAP + APPBAR_LABEL_HEIGHT + APPBAR_PAD;
  float panel = expanded ? rows + (showMenu ? menu * APPBAR_MENU_ITEM + APPBAR_PAD : 0) : h;
  float top = expanded ? y + h - panel : y;

  // The expanded bar grows upwards over the page; taps above it collapse it.
  // The panel continues the bar's surface, translucent bars included.
  struct Node2D *bar = GetNode2D(hObject);
  AppBar_SetColor(pApplicationBar->_panel, ID_Node2D_BackgroundColor, bar->Background.Color);
  GetNode2D(pApplicationBar->_panel)->BackdropFilter = bar->BackdropFilter;
  Metro_Show(pApplicationBar->_panel, expanded, 1);
  Metro_Arrange(pApplicationBar->_panel, x, top, w, panel);
  Metro_Show(pApplicationBar->_scrim, expanded, 1);
  Metro_Arrange(pApplicationBar->_scrim, x, top - 4096, w, 4096);

  // The ellipsis lines up with the middle of the round buttons.
  float ringY = expanded ? top + APPBAR_PAD : y + (h - APPBAR_RING) / 2;
  float dotsWidth = APPBAR_DOTS * APPBAR_DOT + (APPBAR_DOTS - 1) * APPBAR_DOT;
  float dotsX = x + w - METRO_INSET - dotsWidth, dotsY = ringY + (APPBAR_RING - APPBAR_DOT) / 2;
  Metro_Arrange(pApplicationBar->_ellipsis, dotsX, dotsY, dotsWidth, APPBAR_DOT);
  int dot = 0;
  FOR_EACH_OBJECT(child, pApplicationBar->_ellipsis) {
    AppBar_SetColor(child, ID_Node2D_BackgroundColor, AppBar_Foreground(hObject));
    Metro_Arrange(child, dot++ * 2 * APPBAR_DOT, 0, APPBAR_DOT, APPBAR_DOT);
  }

  float slotX = x + (w - buttons * APPBAR_SLOT) / 2, menuY = top + rows;
  FOR_EACH_OBJECT(child, hObject) {
    struct ApplicationBarIconButton *button = GetApplicationBarIconButton(child);
    struct ApplicationBarMenuItem *item = GetApplicationBarMenuItem(child);
    if (button && GetNode(child)->Visible) {
      // A minimized bar fades its buttons rather than hiding them, so the
      // app's own Visible setting stays authoritative.
      bool_t shown = !minimized || expanded;
      GetNode2D(child)->IgnoreHitTest = !shown;
      Metro_Show(child, TRUE, shown ? (button->IsEnabled ? 1 : METRO_DIM) : 0);
      Metro_Arrange(child, slotX, ringY, APPBAR_SLOT, APPBAR_RING + APPBAR_LABEL_GAP + APPBAR_LABEL_HEIGHT);
      slotX += APPBAR_SLOT;
    } else if (item) {
      Metro_Show(child, showMenu, item->IsEnabled ? 1 : METRO_DIM);
      Metro_Arrange(child, x, menuY, w, APPBAR_MENU_ITEM);
      menuY += APPBAR_MENU_ITEM;
    }
  }
  return MAKEDWORD(w, h);
}

// Tapping the bar itself, the ellipsis or outside the expanded bar toggles it.
HANDLER(ApplicationBar, Node, LeftButtonUp)
{
  bool_t value = !pApplicationBar->IsExpanded;
  Metro_SetProperty(hObject, ID_ApplicationBar_IsExpanded, &value);
  return TRUE;
}

static void
AppBarButton_Sync(struct Object *hObject, struct ApplicationBarIconButton *pButton)
{
  if (!pButton->_ring) {
    pButton->_ring = AppBar_Part(hObject, ID_Node2D);
    AppBar_Round(pButton->_ring, APPBAR_RING / 2);
    struct Node *ring = GetNode(pButton->_ring);
    ring->Border.Width.Axis[0].Left = ring->Border.Width.Axis[0].Right = APPBAR_RING_BORDER;
    ring->Border.Width.Axis[1].Left = ring->Border.Width.Axis[1].Right = APPBAR_RING_BORDER;
    _SendMessage(pButton->_ring, StyleController, AddClass, .ClassName = "appbar-button-ring");
  }
  if (!pButton->_icon) {
    pButton->_icon = AppBar_Part(hObject, ID_VectorView);
    Metro_SetFloat(pButton->_icon, ID_Node_Width, APPBAR_ICON);
    Metro_SetFloat(pButton->_icon, ID_Node_Height, APPBAR_ICON);
  }
  if (!pButton->_label) {
    pButton->_label = Metro_CreateText(hObject, ID_TextBlock, "appbar-button-label", APPBAR_LABEL_SIZE, FALSE);
    GetNode2D(pButton->_label)->IgnoreHitTest = TRUE;
    GetTextBlockConcept(pButton->_label)->TextHorizontalAlignment = kTextHorizontalAlignmentCenter;
  }
  struct VectorView *icon = GetVectorView(pButton->_icon);
  if (pButton->IconUri && (!icon->Source || strcmp(icon->Source, pButton->IconUri))) {
    PROP_SetStringValue(OBJ_FindLongProperty(pButton->_icon, ID_VectorView_Source), pButton->IconUri);
  }
  Metro_SetText(pButton->_label, pButton->Text, tolower);
  AppBar_SetColor(pButton->_ring, ID_Node_BorderColor, AppBar_Foreground(OBJ_GetParent(hObject)));
}

HANDLER(ApplicationBarIconButton, Object, PropertyChanged)
{
  OBJ_SetDirty(hObject);
  return FALSE;
}

HANDLER(ApplicationBarIconButton, Node2D, MeasureOverride)
{
  AppBarButton_Sync(hObject, pApplicationBarIconButton);
  Metro_Measure(pApplicationBarIconButton->_label, APPBAR_SLOT, APPBAR_LABEL_HEIGHT);
  Metro_Measure(pApplicationBarIconButton->_icon, APPBAR_ICON, APPBAR_ICON);
  return MAKEDWORD(APPBAR_SLOT, APPBAR_RING + APPBAR_LABEL_GAP + APPBAR_LABEL_HEIGHT);
}

HANDLER(ApplicationBarIconButton, Node2D, ArrangeOverride)
{
  struct ApplicationBarIconButton *pButton = pApplicationBarIconButton;
  AppBarButton_Sync(hObject, pButton);
  float x = pArrangeOverride->X, y = pArrangeOverride->Y, w = pArrangeOverride->Width;
  float ringX = x + (w - APPBAR_RING) / 2;
  Metro_Arrange(pButton->_ring, ringX, y, APPBAR_RING, APPBAR_RING);
  Metro_Arrange(pButton->_icon, ringX + (APPBAR_RING - APPBAR_ICON) / 2, y + (APPBAR_RING - APPBAR_ICON) / 2,
    APPBAR_ICON, APPBAR_ICON);
  struct ApplicationBar *bar = GetApplicationBar(OBJ_GetParent(hObject));
  Metro_Show(pButton->_label, bar && bar->IsExpanded, 1);
  Metro_Arrange(pButton->_label, x, y + APPBAR_RING + APPBAR_LABEL_GAP, w, APPBAR_LABEL_HEIGHT);
  return MAKEDWORD(w, pArrangeOverride->Height);
}

HANDLER(ApplicationBarIconButton, Node, LeftButtonUp)
{
  if (pApplicationBarIconButton->IsEnabled) {
    AppBar_Collapse(OBJ_GetParent(hObject));
    axPostMessageW(hObject, ID_ApplicationBarIconButton_Click, 0, 0);
  }
  return TRUE;
}

HANDLER(ApplicationBarMenuItem, Object, PropertyChanged)
{
  OBJ_SetDirty(hObject);
  return FALSE;
}

HANDLER(ApplicationBarMenuItem, Node2D, MeasureOverride)
{
  if (!pApplicationBarMenuItem->_label) {
    pApplicationBarMenuItem->_label =
      Metro_CreateText(hObject, ID_TextBlock, "appbar-menu-item", APPBAR_MENU_SIZE, FALSE);
    GetNode2D(pApplicationBarMenuItem->_label)->IgnoreHitTest = TRUE;
  }
  Metro_SetText(pApplicationBarMenuItem->_label, pApplicationBarMenuItem->Text, tolower);
  LRESULT size = Metro_Measure(pApplicationBarMenuItem->_label, pMeasureOverride->Width, APPBAR_MENU_ITEM);
  return MAKEDWORD(LOWORD(size) + 2 * METRO_INSET, APPBAR_MENU_ITEM);
}

HANDLER(ApplicationBarMenuItem, Node2D, ArrangeOverride)
{
  struct Object *label = pApplicationBarMenuItem->_label;
  if (!label) return MAKEDWORD(pArrangeOverride->Width, pArrangeOverride->Height);
  LRESULT size = Metro_Measure(label, pArrangeOverride->Width, APPBAR_MENU_ITEM);
  Metro_Arrange(label, pArrangeOverride->X + METRO_INSET,
    pArrangeOverride->Y + (pArrangeOverride->Height - HIWORD(size)) / 2,
    MAX(pArrangeOverride->Width - 2 * METRO_INSET, 0), HIWORD(size));
  return MAKEDWORD(pArrangeOverride->Width, pArrangeOverride->Height);
}

HANDLER(ApplicationBarMenuItem, Node, LeftButtonUp)
{
  if (pApplicationBarMenuItem->IsEnabled) {
    AppBar_Collapse(OBJ_GetParent(hObject));
    axPostMessageW(hObject, ID_ApplicationBarMenuItem_Click, 0, 0);
  }
  return TRUE;
}
