#include "metro.h"

// Pivot layout, after the Windows Phone 8 Pivot template.
#define PIVOT_TITLE_TOP      14.0f
#define PIVOT_TITLE_SIZE     15.0f
#define PIVOT_HEADER_TOP     2.0f    // gap between title and header strip
#define PIVOT_HEADER_SIZE    44.0f
#define PIVOT_HEADER_SPACING 18.0f
#define PIVOT_CONTENT_TOP    22.0f   // gap between header strip and content (28 px on WP)
#define PIVOT_SLIDE          80.0f   // distance the incoming content slides
#define PIVOT_MAX_ITEMS      32

static int
Pivot_Items(struct Object *hObject, struct Object **items)
{
  int count = 0;
  FOR_EACH_OBJECT(child, hObject) {
    if (GetPivotItem(child) && count < PIVOT_MAX_ITEMS) items[count++] = child;
  }
  return count;
}

static struct Object *
Pivot_HeaderFor(struct Object *hObject, struct Object *item)
{
  FOR_EACH_OBJECT(child, hObject) {
    struct PivotHeaderItem *header = GetPivotHeaderItem(child);
    if (header && !header->_repeat && header->_item == item) return child;
  }
  return NULL;
}

static float
Pivot_Width(struct Object *o)
{
  return LOWORD(Metro_Measure(o, 4096, 4096));
}

// Creates the title and one header per PivotItem, drops stale headers and
// reflects the selection on headers and items.
static int
Pivot_Sync(struct Object *hObject, struct Pivot *pPivot, struct Object **items)
{
  int count = Pivot_Items(hObject, items);
  if (!pPivot->_title) {
    pPivot->_title = Metro_CreateText(hObject, ID_TextBlock, "pivot-title", PIVOT_TITLE_SIZE, TRUE);
    GetNode2D(pPivot->_title)->IgnoreHitTest = TRUE;
  }
  Metro_SetText(pPivot->_title, pPivot->Title, toupper);
  Metro_Show(pPivot->_title, pPivot->Title && *pPivot->Title, 1);

  struct Object *stale[PIVOT_MAX_ITEMS];
  int staleCount = 0;
  FOR_EACH_OBJECT(child, hObject) {
    struct PivotHeaderItem *header = GetPivotHeaderItem(child);
    if (!header) continue;
    bool_t found = FALSE;
    for (int i = 0; i < count && !found; i++) found = items[i] == header->_item;
    if (!found && staleCount < PIVOT_MAX_ITEMS) stale[staleCount++] = child;
  }
  for (int i = 0; i < staleCount; i++) OBJ_RemoveFromParent(stale[i]);

  if (count && (pPivot->SelectedIndex < 0 || pPivot->SelectedIndex >= count)) {
    pPivot->SelectedIndex = MAX(0, MIN(pPivot->SelectedIndex, count - 1));
  }
  for (int i = 0; i < count; i++) {
    struct Object *header = Pivot_HeaderFor(hObject, items[i]);
    if (!header) {
      header = Metro_CreateText(hObject, ID_PivotHeaderItem, "pivot-header", PIVOT_HEADER_SIZE, FALSE);
      GetPivotHeaderItem(header)->_item = items[i];
    }
    bool_t selected = i == pPivot->SelectedIndex;
    GetPivotHeaderItem(header)->IsSelected = selected;
    Metro_SetText(header, GetPivotItem(items[i])->Header, tolower);
    Metro_Show(header, selected || !pPivot->IsLocked, selected ? 1 : METRO_DIM);
    Metro_Show(items[i], selected, 1);
  }
  return count;
}

static void
Pivot_StartAnimation(struct Object *hObject, struct Pivot *pPivot)
{
  pPivot->_animStart = (int32_t)axGetMilliseconds();
  pPivot->_progress = 0;
  OBJ_SetDirty(hObject);
  OBJ_RequestAnimate(hObject);
}

// Moves from `old` to the current SelectedIndex: the new header slides into
// the leading slot and the new content slides in from the side it came from.
static void
Pivot_Transition(struct Object *hObject, struct Pivot *pPivot, int old)
{
  struct Object *items[PIVOT_MAX_ITEMS];
  int count = Pivot_Items(hObject, items), sel = pPivot->SelectedIndex;
  if (old < 0 || old >= count || old == sel) return;
  int forward = (sel - old + count) % count;
  if (forward <= count / 2) {
    float shift = 0;
    for (int i = old; i != sel; i = (i + 1) % count) {
      struct Object *header = Pivot_HeaderFor(hObject, items[i]);
      if (header) shift += Pivot_Width(header) + PIVOT_HEADER_SPACING;
    }
    pPivot->_headerFrom = shift;
    pPivot->_contentFrom = PIVOT_SLIDE;
  } else {
    struct Object *header = Pivot_HeaderFor(hObject, items[sel]);
    pPivot->_headerFrom = -(header ? Pivot_Width(header) + PIVOT_HEADER_SPACING : 0);
    pPivot->_contentFrom = -PIVOT_SLIDE;
  }
  Pivot_StartAnimation(hObject, pPivot);
  axPostMessageDataW(hObject, ID_Pivot_SelectionChanged, 0, &(struct Pivot_SelectionChangedEventArgs) {
    .SelectedIndex = sel, .OldIndex = old,
  }, sizeof(struct Pivot_SelectionChangedEventArgs));
}

static int
Pivot_SelectedHeaderIndex(struct Object *hObject)
{
  struct Object *items[PIVOT_MAX_ITEMS];
  int count = Pivot_Items(hObject, items);
  for (int i = 0; i < count; i++) {
    struct Object *header = Pivot_HeaderFor(hObject, items[i]);
    if (header && GetPivotHeaderItem(header)->IsSelected) return i;
  }
  return -1;
}

static void
Pivot_Select(struct Object *hObject, int index)
{
  int32_t value = index;
  Metro_SetProperty(hObject, ID_Pivot_SelectedIndex, &value);
}

HANDLER(Pivot, Object, Create)
{
  pPivot->_progress = 1;
  return FALSE;
}

HANDLER(Pivot, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property) return FALSE;
  uint32_t id = PROP_GetLongIdentifier(pPropertyChanged->Property);
  if (id == ID_Pivot_SelectedIndex) {
    Pivot_Transition(hObject, pPivot, Pivot_SelectedHeaderIndex(hObject));
  }
  if (id == ID_Pivot_SelectedIndex || id == ID_Pivot_Title || id == ID_Pivot_IsLocked) {
    OBJ_SetDirty(hObject);
  }
  return FALSE;
}

HANDLER(Pivot, Object, Animate)
{
  pPivot->_progress = MIN(Metro_Elapsed(pPivot->_animStart), 1);
  OBJ_SetDirty(hObject);
  if (pPivot->_progress < 1) OBJ_RequestAnimate(hObject);
  return TRUE;
}

HANDLER(Pivot, Node2D, MeasureOverride)
{
  struct Object *items[PIVOT_MAX_ITEMS];
  int count = Pivot_Sync(hObject, pPivot, items);
  LRESULT title = Metro_Measure(pPivot->_title, pMeasureOverride->Width, pMeasureOverride->Height);
  float top = PIVOT_TITLE_TOP + HIWORD(title) + PIVOT_HEADER_TOP, strip = 0, width = 0, height = 0;
  for (int i = 0; i < count; i++) {
    LRESULT size = Metro_Measure(Pivot_HeaderFor(hObject, items[i]), 4096, 4096);
    strip = MAX(strip, HIWORD(size));
  }
  top += strip + PIVOT_CONTENT_TOP;
  if (count) {
    LRESULT size = Metro_Measure(items[pPivot->SelectedIndex],
      MAX(pMeasureOverride->Width - 2 * METRO_INSET, 0), MAX(pMeasureOverride->Height - top, 0));
    width = LOWORD(size) + 2 * METRO_INSET;
    height = HIWORD(size);
  }
  return MAKEDWORD(width, top + height);
}

HANDLER(Pivot, Node2D, ArrangeOverride)
{
  struct Object *items[PIVOT_MAX_ITEMS];
  int count = Pivot_Sync(hObject, pPivot, items);
  float x = pArrangeOverride->X, y = pArrangeOverride->Y;
  float w = pArrangeOverride->Width, h = pArrangeOverride->Height;
  float settle = 1 - Metro_Ease(pPivot->_progress);
  struct Object *headers[PIVOT_MAX_ITEMS], *copies[PIVOT_MAX_ITEMS];
  LRESULT sizes[PIVOT_MAX_ITEMS];
  int copyCount = 0;
  // Keep metrics local to this pass so text, style and viewport changes still
  // remeasure, while repeated labels reuse their source header's metrics.
  for (int i = 0; i < count; i++) {
    headers[i] = Pivot_HeaderFor(hObject, items[i]);
    sizes[i] = Metro_Measure(headers[i], 4096, 4096);
  }
  FOR_EACH_OBJECT(child, hObject) {
    struct PivotHeaderItem *header = GetPivotHeaderItem(child);
    if (header && header->_repeat && copyCount < PIVOT_MAX_ITEMS) copies[copyCount++] = child;
  }

  LRESULT title = Metro_Measure(pPivot->_title, w, h);
  Metro_Arrange(pPivot->_title, x + METRO_INSET, y + PIVOT_TITLE_TOP, LOWORD(title), HIWORD(title));
  float stripY = y + PIVOT_TITLE_TOP + HIWORD(title) + PIVOT_HEADER_TOP, strip = 0;

  // The selected header leads; the others follow in order and wrap around.
  float hx = x + METRO_INSET + pPivot->_headerFrom * settle + pPivot->_dragX * 0.3f;
  for (int k = 0; k < count; k++) {
    int index = (pPivot->SelectedIndex + k) % count;
    LRESULT size = sizes[index];
    Metro_Arrange(headers[index], hx, stripY, LOWORD(size), HIWORD(size));
    hx += LOWORD(size) + PIVOT_HEADER_SPACING;
    strip = MAX(strip, HIWORD(size));
  }

  // Short strips keep cycling, dimmed, until they reach the right edge. Copies
  // take the size of the header they repeat, which is already measured.
  int repeats = 0;
  for (int k = 0; !pPivot->IsLocked && count && hx < x + w && repeats < PIVOT_MAX_ITEMS; k++) {
    int index = (pPivot->SelectedIndex + k) % count;
    struct Object *item = items[index];
    LRESULT size = sizes[index];
    if (LOWORD(size) == 0) break;
    struct Object *copy = repeats < copyCount ? copies[repeats] :
      Metro_CreateText(hObject, ID_PivotHeaderItem, "pivot-header", PIVOT_HEADER_SIZE, FALSE);
    if (!copy) break;
    GetPivotHeaderItem(copy)->_repeat = TRUE;
    if (repeats == copyCount) copies[copyCount++] = copy;
    repeats++;
    GetPivotHeaderItem(copy)->_item = item;
    Metro_SetText(copy, GetPivotItem(item)->Header, tolower);
    Metro_Show(copy, TRUE, METRO_DIM);
    Metro_Measure(copy, 4096, 4096);
    Metro_Arrange(copy, hx, stripY, LOWORD(size), HIWORD(size));
    hx += LOWORD(size) + PIVOT_HEADER_SPACING;
  }
  // Keep spare copies as the moving strip crosses the edge; locking drops them.
  for (int i = repeats; i < copyCount; i++) {
    if (pPivot->IsLocked) OBJ_RemoveFromParent(copies[i]);
    else Metro_Show(copies[i], FALSE, METRO_DIM);
  }

  float contentY = stripY + strip + PIVOT_CONTENT_TOP;
  if (count) {
    Metro_Arrange(items[pPivot->SelectedIndex],
      x + METRO_INSET + pPivot->_contentFrom * settle + pPivot->_dragX, contentY,
      MAX(w - 2 * METRO_INSET, 0), MAX(y + h - contentY, 0));
  }
  return MAKEDWORD(w, h);
}

HANDLER(Pivot, Node, LeftButtonDown)
{
  pPivot->_tracking = TRUE;
  pPivot->_dragging = FALSE;
  pPivot->_pressX = pLeftButtonDown->x;
  pPivot->_pressY = pLeftButtonDown->y;
  return FALSE;
}

HANDLER(Pivot, Node, LeftButtonDragged)
{
  struct Object *items[PIVOT_MAX_ITEMS];
  int count = Pivot_Items(hObject, items);
  if (!pPivot->_tracking || pPivot->IsLocked || count < 2) return FALSE;
  float dx = pLeftButtonDragged->x - pPivot->_pressX;
  float dy = pLeftButtonDragged->y - pPivot->_pressY;
  if (!pPivot->_dragging) {
    if (fabsf(dx) < METRO_DRAG_START || fabsf(dx) < fabsf(dy) * 1.5f) return FALSE;
    pPivot->_dragging = TRUE;
    pPivot->_progress = 1;
    // Hide the content from hit testing so releasing the swipe is not a tap.
    GetNode2D(items[pPivot->SelectedIndex])->IgnoreHitTest = TRUE;
  }
  pPivot->_dragX = dx;
  OBJ_SetDirty(hObject);
  return TRUE;
}

HANDLER(Pivot, Node, LeftButtonUp)
{
  bool_t dragging = pPivot->_dragging;
  float dx = pPivot->_dragX;
  pPivot->_tracking = pPivot->_dragging = FALSE;
  pPivot->_dragX = 0;
  if (!dragging) return FALSE;

  struct Object *items[PIVOT_MAX_ITEMS];
  int count = Pivot_Items(hObject, items);
  if (!count) return TRUE;
  GetNode2D(items[pPivot->SelectedIndex])->IgnoreHitTest = FALSE;
  if (fabsf(dx) >= METRO_SWIPE) {
    Pivot_Select(hObject, (pPivot->SelectedIndex + (dx < 0 ? 1 : count - 1)) % count);
  } else {
    // Not far enough: settle back into place.
    pPivot->_headerFrom = dx * 0.3f;
    pPivot->_contentFrom = dx;
    Pivot_StartAnimation(hObject, pPivot);
  }
  return TRUE;
}

HANDLER(PivotHeaderItem, Node, LeftButtonUp)
{
  struct Object *pivot = OBJ_GetParent(hObject);
  struct Pivot *pPivot = GetPivot(pivot);
  if (!pPivot || pPivot->_dragging) return FALSE;
  struct Object *items[PIVOT_MAX_ITEMS];
  int count = Pivot_Items(pivot, items);
  for (int i = 0; i < count; i++) {
    if (items[i] == pPivotHeaderItem->_item) Pivot_Select(pivot, i);
  }
  pPivot->_tracking = FALSE;
  return TRUE;
}
