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

static float
Panorama_Width(struct Object *item, float viewport)
{
  return MAX(GetPanoramaItem(item)->_width, viewport * PANORAMA_ITEM_WIDTH);
}

static float
Panorama_Position(struct Object **items, int count, int index, float viewport)
{
  float position = 0;
  for (int i = 0; i < MIN(index, count); i++) position += Panorama_Width(items[i], viewport);
  return position;
}

static float
Panorama_End(struct Object *item, float viewport)
{
  return MAX(Panorama_Width(item, viewport) - viewport, 0);
}

static void
Panorama_AnimateTo(struct Object *hObject, struct Panorama *pPanorama, float offset)
{
  pPanorama->_offsetFrom = offset;
  pPanorama->_animStart = (int32_t)ANIM_GetTime();
  pPanorama->_progress = 0;
  OBJ_RequestAnimate(hObject);
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
    float viewport = Node2D_GetRect(GetNode2D(hObject)).width;
    float width = viewport * PANORAMA_ITEM_WIDTH;
    if (width <= 0) pPanorama->_current = sel;  // not laid out yet: nothing to animate
    else if (count && old != sel && old >= 0 && old < count && sel >= 0 && sel < count) {
      bool_t forward = pPanorama->_dragX ? pPanorama->_dragX < 0 : (sel - old + count) % count <= count / 2;
      // Keep the swipe position when a drag committed this change.
      float from = Panorama_Position(items, count, old, viewport) + pPanorama->_sectionOffset;
      float to = Panorama_Position(items, count, sel, viewport);
      float total = Panorama_Position(items, count, count, viewport);
      if (forward && to < from) to += total;
      if (!forward && to > from) to -= total;
      pPanorama->_sectionOffset = 0;
      Panorama_AnimateTo(hObject, pPanorama, pPanorama->_dragX + to - from);
      pPanorama->_current = sel;
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
    struct PanoramaItem *item = GetPanoramaItem(items[i]);
    LRESULT size = Metro_Measure(items[i], item->Orientation == kDirectionHorizontal ? INFINITY : MAX(itemWidth - METRO_INSET, 0),
      MAX(pMeasureOverride->Height - top - header, 0));
    item->_width = item->Orientation == kDirectionHorizontal ? MAX(itemWidth, LOWORD(size) + METRO_INSET) : itemWidth;
    // Once its natural width is known, wrapping and aspect ratios use that width.
    if (item->Orientation == kDirectionHorizontal)
      size = Metro_Measure(items[i], MAX(item->_width - METRO_INSET, 0),
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
  float offset = pPanorama->_offsetFrom * (1 - Metro_Ease(pPanorama->_progress)) + pPanorama->_dragX;
  float selected = Panorama_Position(items, count, pPanorama->SelectedIndex, w);
  float total = Panorama_Position(items, count, count, w);
  if (count) pPanorama->_sectionOffset = MIN(pPanorama->_sectionOffset,
    Panorama_End(items[pPanorama->SelectedIndex], w));

  // The title scrolls through its overflow more slowly than the sections.
  LRESULT title = Metro_Measure(pPanorama->_title, 4096, 4096);
  float travel = MAX(LOWORD(title) - w * PANORAMA_PARALLAX, 0);
  float range = count ? MAX(total - MIN(Panorama_Width(items[count - 1], w), w), 0) : 0;
  float position = range > 0 ? (selected + pPanorama->_sectionOffset - offset) / range : 0;
  Metro_Arrange(pPanorama->_title, x + METRO_INSET * 0.5f - travel * position, y + PANORAMA_TITLE_TOP,
    LOWORD(title), HIWORD(title));

  float top = y + PANORAMA_TITLE_TOP + HIWORD(title);
  for (int i = 0; i < count; i++) {
    struct Object *header = GetPanoramaItem(items[i])->_header;
    float distance = Panorama_Position(items, count, i, w) - selected;
    float width = Panorama_Width(items[i], w);
    float contentWidth = MAX(width - METRO_INSET, 0);
    float left = distance - pPanorama->_sectionOffset + offset + METRO_INSET;
    if (total > 0) {
      // Choose the circular copy at the animated viewport, including skipped sections.
      left -= floorf((left + contentWidth) / total) * total;
      if (left + contentWidth <= 0) left += total;
    }
    bool_t near = left < w && left + contentWidth > 0;
    Metro_Show(header, near, 1);
    Metro_Show(items[i], near, 1);
    if (!near) continue;
    LRESULT size = Metro_Measure(header, 4096, 4096);
    Metro_Arrange(header, x + left, top + PANORAMA_HEADER_TOP, LOWORD(size), HIWORD(size));
    float contentY = top + PANORAMA_HEADER_TOP + HIWORD(size) + PANORAMA_CONTENT_TOP;
    Metro_Arrange(items[i], x + left, contentY, contentWidth, MAX(y + h - contentY, 0));
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
  if (!pPanorama->_tracking || !count) return FALSE;
  float viewport = Node2D_GetRect(GetNode2D(hObject)).width;
  if (count < 2 && Panorama_Width(items[0], viewport) <= viewport) return FALSE;
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
  float viewport = Node2D_GetRect(GetNode2D(hObject)).width;
  float step = viewport;
  float end = count ? Panorama_End(items[pPanorama->SelectedIndex], viewport) : 0;
  float oldOffset = pPanorama->_sectionOffset;
  if (fabsf(dx) >= METRO_SWIPE && (dx < 0 ? oldOffset < end : oldOffset > 0)) {
    pPanorama->_sectionOffset = dx < 0 ? MIN(oldOffset + step, end) : MAX(oldOffset - step, 0);
    Panorama_AnimateTo(hObject, pPanorama, dx + pPanorama->_sectionOffset - oldOffset);
  } else if (fabsf(dx) >= METRO_SWIPE && count > 1) {
    int32_t next = (pPanorama->SelectedIndex + (dx < 0 ? 1 : count - 1)) % count;
    Metro_SetProperty(hObject, ID_Panorama_SelectedIndex, &next);
    if (dx > 0) {
      pPanorama->_sectionOffset = Panorama_End(items[next], viewport);
      pPanorama->_offsetFrom += pPanorama->_sectionOffset;
    }
  } else {
    Panorama_AnimateTo(hObject, pPanorama, dx);
  }
  pPanorama->_dragX = 0;
  OBJ_SetDirty(hObject);
  return TRUE;
}
