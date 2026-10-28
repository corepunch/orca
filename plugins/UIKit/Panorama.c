#include "metro.h"

// Panorama layout, after the Windows Phone 8 Panorama template.
#define PANORAMA_TITLE_SIZE    110.0f
#define PANORAMA_TITLE_TOP     -14.0f  // the title is allowed to crowd the top edge
#define PANORAMA_HEADER_SIZE   36.0f
#define PANORAMA_HEADER_TOP    -4.0f
#define PANORAMA_CONTENT_TOP   12.0f
#define PANORAMA_ITEM_WIDTH    0.875f  // of the panorama width; the next section peeks in
#define PANORAMA_PARALLAX      0.6f    // title travel relative to its overflow
#define PANORAMA_MAX_ITEMS     32

static int
Panorama_Items(struct Object *hObject, struct Object **items)
{
  int count = 0;
  FOR_EACH_OBJECT(child, hObject) {
    if (GetPanoramaItem(child) && count < PANORAMA_MAX_ITEMS) items[count++] = child;
  }
  return count;
}

static bool_t
Panorama_IsHeader(struct Object *hObject, struct Object *child, struct Object **items, int count)
{
  for (int i = 0; i < count; i++) {
    if (GetPanoramaItem(items[i])->_header == child) return TRUE;
  }
  return FALSE;
}

static int
Panorama_Sync(struct Object *hObject, struct Panorama *pPanorama, struct Object **items)
{
  int count = Panorama_Items(hObject, items);
  if (!pPanorama->_title) {
    pPanorama->_title = Metro_CreateText(hObject, ID_TextBlock, "panorama-title", PANORAMA_TITLE_SIZE, FALSE);
    GetNode2D(pPanorama->_title)->IgnoreHitTest = TRUE;
  }
  Metro_SetText(pPanorama->_title, pPanorama->Title, tolower);

  // Drop headers whose section was removed.
  struct Object *stale[PANORAMA_MAX_ITEMS];
  int staleCount = 0;
  FOR_EACH_OBJECT(child, hObject) {
    if (child == pPanorama->_title || GetPanoramaItem(child) || !GetTextRun(child)) continue;
    if (!Panorama_IsHeader(hObject, child, items, count) && staleCount < PANORAMA_MAX_ITEMS)
      stale[staleCount++] = child;
  }
  for (int i = 0; i < staleCount; i++) OBJ_RemoveFromParent(stale[i]);

  if (count && (pPanorama->SelectedIndex < 0 || pPanorama->SelectedIndex >= count)) {
    pPanorama->SelectedIndex = MAX(0, MIN(pPanorama->SelectedIndex, count - 1));
  }
  pPanorama->_current = pPanorama->SelectedIndex;
  for (int i = 0; i < count; i++) {
    struct PanoramaItem *item = GetPanoramaItem(items[i]);
    if (!item->_header) {
      item->_header = Metro_CreateText(hObject, ID_TextBlock, "panorama-header", PANORAMA_HEADER_SIZE, FALSE);
      GetNode2D(item->_header)->IgnoreHitTest = TRUE;
    }
    Metro_SetText(item->_header, item->Header, tolower);
  }
  return count;
}

// Horizontal slot of section i relative to the one in view: 0 is in view,
// 1 is the peeking next section and -1 the previous one.
static int
Panorama_Slot(struct Panorama *pPanorama, int i, int count, float offset)
{
  int slot = (i - pPanorama->SelectedIndex + count) % count;
  if (slot == count - 1 && slot > 0 && (count > 2 || offset > 0)) slot = -1;
  return slot;
}

HANDLER(Panorama, Object, Create)
{
  pPanorama->_progress = 1;
  return FALSE;
}

HANDLER(Panorama, Object, Destroy)
{
  OBJ_ReleaseRef(pPanorama->_title);
  return FALSE;
}

// The header stays a child of the panorama, which drops it once stale.
HANDLER(PanoramaItem, Object, Destroy)
{
  OBJ_ReleaseRef(pPanoramaItem->_header);
  return FALSE;
}

HANDLER(Panorama, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property) return FALSE;
  uint32_t id = PROP_GetLongIdentifier(pPropertyChanged->Property);
  if (id == ID_Panorama_SelectedIndex) {
    struct Object *items[PANORAMA_MAX_ITEMS];
    int count = Panorama_Items(hObject, items), old = pPanorama->_current;
    int sel = pPanorama->SelectedIndex;
    float width = Node2D_GetRect(GetNode2D(hObject)).width * PANORAMA_ITEM_WIDTH;
    if (width <= 0) pPanorama->_current = sel;  // not laid out yet: nothing to animate
    else if (count && old != sel && old >= 0 && old < count && sel >= 0 && sel < count) {
      bool_t forward = (sel - old + count) % count <= count / 2;
      // Keep the swipe position when a drag committed this change.
      pPanorama->_offsetFrom = pPanorama->_dragX + (forward ? width : -width);
      pPanorama->_animStart = (int32_t)ANIM_GetTime();
      pPanorama->_progress = 0;
      pPanorama->_current = sel;
      OBJ_RequestAnimate(hObject);
      axPostMessageDataW(hObject, ID_Panorama_SelectionChanged, 0, &(struct Panorama_SelectionChangedEventArgs) {
        .SelectedIndex = sel, .OldIndex = old,
      }, sizeof(struct Panorama_SelectionChangedEventArgs));
    }
  }
  OBJ_SetDirty(hObject);
  return FALSE;
}

HANDLER(Panorama, Object, Animate)
{
  pPanorama->_progress = MIN(Metro_Elapsed(pPanorama->_animStart), 1);
  OBJ_SetDirty(hObject);
  if (pPanorama->_progress < 1) OBJ_RequestAnimate(hObject);
  return TRUE;
}

HANDLER(Panorama, Node2D, MeasureOverride)
{
  struct Object *items[PANORAMA_MAX_ITEMS];
  int count = Panorama_Sync(hObject, pPanorama, items);
  float itemWidth = pMeasureOverride->Width * PANORAMA_ITEM_WIDTH;
  float top = HIWORD(Metro_Measure(pPanorama->_title, 4096, 4096)) + PANORAMA_TITLE_TOP, height = 0;
  for (int i = 0; i < count; i++) {
    float header = HIWORD(Metro_Measure(GetPanoramaItem(items[i])->_header, 4096, 4096));
    LRESULT size = Metro_Measure(items[i], MAX(itemWidth - METRO_INSET, 0),
      MAX(pMeasureOverride->Height - top - header, 0));
    height = MAX(height, header + PANORAMA_HEADER_TOP + PANORAMA_CONTENT_TOP + HIWORD(size));
  }
  return MAKEDWORD(pMeasureOverride->Width, top + height);
}

HANDLER(Panorama, Node2D, ArrangeOverride)
{
  struct Object *items[PANORAMA_MAX_ITEMS];
  int count = Panorama_Sync(hObject, pPanorama, items);
  float x = pArrangeOverride->X, y = pArrangeOverride->Y;
  float w = pArrangeOverride->Width, h = pArrangeOverride->Height;
  float itemWidth = w * PANORAMA_ITEM_WIDTH;
  float offset = pPanorama->_offsetFrom * (1 - Metro_Ease(pPanorama->_progress)) + pPanorama->_dragX;

  // The title scrolls through its overflow more slowly than the sections.
  LRESULT title = Metro_Measure(pPanorama->_title, 4096, 4096);
  float travel = MAX(LOWORD(title) - w * PANORAMA_PARALLAX, 0);
  float position = count > 1 ? (pPanorama->SelectedIndex - offset / itemWidth) / (count - 1) : 0;
  Metro_Arrange(pPanorama->_title, x + METRO_INSET * 0.5f - travel * position, y + PANORAMA_TITLE_TOP,
    LOWORD(title), HIWORD(title));

  float top = y + PANORAMA_TITLE_TOP + HIWORD(title);
  for (int i = 0; i < count; i++) {
    struct Object *header = GetPanoramaItem(items[i])->_header;
    int slot = Panorama_Slot(pPanorama, i, count, offset);
    bool_t near = slot >= -1 && slot <= 1;
    Metro_Show(header, near, 1);
    Metro_Show(items[i], near, 1);
    if (!near) continue;
    float left = x + slot * itemWidth + offset + METRO_INSET;
    LRESULT size = Metro_Measure(header, 4096, 4096);
    Metro_Arrange(header, left, top + PANORAMA_HEADER_TOP, LOWORD(size), HIWORD(size));
    float contentY = top + PANORAMA_HEADER_TOP + HIWORD(size) + PANORAMA_CONTENT_TOP;
    Metro_Arrange(items[i], left, contentY, MAX(itemWidth - METRO_INSET, 0), MAX(y + h - contentY, 0));
  }
  return MAKEDWORD(w, h);
}

HANDLER(Panorama, Node, LeftButtonDown)
{
  pPanorama->_tracking = TRUE;
  pPanorama->_dragging = FALSE;
  pPanorama->_pressX = pLeftButtonDown->x;
  pPanorama->_pressY = pLeftButtonDown->y;
  return FALSE;
}

HANDLER(Panorama, Node, LeftButtonDragged)
{
  struct Object *items[PANORAMA_MAX_ITEMS];
  int count = Panorama_Items(hObject, items);
  if (!pPanorama->_tracking || count < 2) return FALSE;
  float dx = pLeftButtonDragged->x - pPanorama->_pressX;
  float dy = pLeftButtonDragged->y - pPanorama->_pressY;
  if (!pPanorama->_dragging) {
    if (fabsf(dx) < METRO_DRAG_START || fabsf(dx) < fabsf(dy) * 1.5f) return FALSE;
    pPanorama->_dragging = TRUE;
    pPanorama->_progress = 1;
    for (int i = 0; i < count; i++) GetNode2D(items[i])->IgnoreHitTest = TRUE;
  }
  pPanorama->_dragX = dx;
  OBJ_SetDirty(hObject);
  return TRUE;
}

HANDLER(Panorama, Node, LeftButtonUp)
{
  bool_t dragging = pPanorama->_dragging;
  pPanorama->_tracking = pPanorama->_dragging = FALSE;
  if (!dragging) return FALSE;

  struct Object *items[PANORAMA_MAX_ITEMS];
  int count = Panorama_Items(hObject, items);
  for (int i = 0; i < count; i++) GetNode2D(items[i])->IgnoreHitTest = FALSE;
  float dx = pPanorama->_dragX;
  if (fabsf(dx) >= METRO_SWIPE && count > 1) {
    int32_t next = (pPanorama->SelectedIndex + (dx < 0 ? 1 : count - 1)) % count;
    Metro_SetProperty(hObject, ID_Panorama_SelectedIndex, &next);
  } else {
    pPanorama->_offsetFrom = dx;
    pPanorama->_animStart = (int32_t)ANIM_GetTime();
    pPanorama->_progress = 0;
    OBJ_RequestAnimate(hObject);
  }
  pPanorama->_dragX = 0;
  OBJ_SetDirty(hObject);
  return TRUE;
}
