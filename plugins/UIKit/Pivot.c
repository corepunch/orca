#include "metro.h"

// Pivot layout, after the Windows Phone 8 Pivot template.
#define PIVOT_TITLE_TOP      14.0f
#define PIVOT_TITLE_SIZE     15.0f
#define PIVOT_HEADER_TOP     2.0f    // gap between title and header strip
#define PIVOT_HEADER_SIZE    44.0f
#define PIVOT_HEADER_SPACING 18.0f
#define PIVOT_CONTENT_TOP    8.0f    // gap between header strip and content; the header line already ends below its descenders
#define PIVOT_SLIDE          80.0f   // distance the incoming content slides
#define PIVOT_BACK_ICON      16.0f   // back glyph, centred in its ring
#define PIVOT_BACK_RING      30.0f   // round back button before the title
#define PIVOT_BACK_GAP       10.0f
#define PIVOT_BACK_MIN       44.0f   // smallest back tap target
#define PIVOT_EDGE           24.0f   // width of the strip a back swipe starts in
#define PIVOT_BACK_SWIPE     96.0f   // distance that commits a back swipe
#define PIVOT_MAX_ITEMS      32

HANDLER(Node2D, Node, UpdateMatrix);
HANDLER(Node2D, Node, HitTest);

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
  // The bar precedes the title so that it draws over the items and under the
  // title and headers.
  if (!pPivot->_bar) {
    pPivot->_bar = OBJ_Create(ID_Node2D);
    _SendMessage(pPivot->_bar, StyleController, AddClass, .ClassName = "pivot-bar");
    OBJ_AddChild(hObject, pPivot->_bar);
  }
  Metro_Show(pPivot->_bar, pPivot->ExtendBodyBehindHeader, 1);
  if (!pPivot->_title) {
    pPivot->_title = Metro_CreateText(hObject, ID_TextBlock, "pivot-title", PIVOT_TITLE_SIZE, TRUE);
    GetNode2D(pPivot->_title)->IgnoreHitTest = TRUE;
  }
  Metro_SetText(pPivot->_title, pPivot->Title, toupper);
  Metro_Show(pPivot->_title, pPivot->Title && *pPivot->Title, 1);
  // The back tap area follows the title so it takes the taps over the bar.
  bool_t back = pPivot->BackIcon && *pPivot->BackIcon;
  if (back && !pPivot->_back) {
    pPivot->_back = OBJ_Create(ID_Node2D);
    OBJ_AddChild(hObject, pPivot->_back);
    // Antialiased vector artwork tinted like the glyph; CSS supplies its SVG.
    pPivot->_backRing = OBJ_Create(ID_VectorView);
    GetNode2D(pPivot->_backRing)->IgnoreHitTest = TRUE;
    Metro_SetFloat(pPivot->_backRing, ID_Node_Width, PIVOT_BACK_RING);
    Metro_SetFloat(pPivot->_backRing, ID_Node_Height, PIVOT_BACK_RING);
    _SendMessage(pPivot->_backRing, StyleController, AddClass, .ClassName = "pivot-back");
    OBJ_AddChild(pPivot->_back, pPivot->_backRing);
    pPivot->_backIcon = OBJ_Create(ID_VectorView);
    GetNode2D(pPivot->_backIcon)->IgnoreHitTest = TRUE;
    Metro_SetFloat(pPivot->_backIcon, ID_Node_Width, PIVOT_BACK_ICON);
    Metro_SetFloat(pPivot->_backIcon, ID_Node_Height, PIVOT_BACK_ICON);
    OBJ_AddChild(pPivot->_back, pPivot->_backIcon);
  }
  if (pPivot->_back) {
    struct VectorView *icon = GetVectorView(pPivot->_backIcon);
    if (back && (!icon->Source || strcmp(icon->Source, pPivot->BackIcon)))
      PROP_SetStringValue(OBJ_FindLongProperty(pPivot->_backIcon, ID_VectorView_Source), pPivot->BackIcon);
    Metro_Show(pPivot->_back, back, 1);
  }

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
      OBJ_ReleaseRef(header);  // found again among the children
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
  pPivot->_animStart = (int32_t)ANIM_GetTime();
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

// The parts were cleared off the pivot already; drop the references it kept.
HANDLER(Pivot, Object, Destroy)
{
  OBJ_ReleaseRef(pPivot->_bar);
  OBJ_ReleaseRef(pPivot->_title);
  OBJ_ReleaseRef(pPivot->_back);
  OBJ_ReleaseRef(pPivot->_backRing);
  OBJ_ReleaseRef(pPivot->_backIcon);
  return FALSE;
}

HANDLER(Pivot, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property) return FALSE;
  uint32_t id = PROP_GetLongIdentifier(pPropertyChanged->Property);
  if (id == ID_Pivot_SelectedIndex) {
    Pivot_Transition(hObject, pPivot, Pivot_SelectedHeaderIndex(hObject));
  }
  if (id == ID_Pivot_TransitionMode) {
    // The clock starts with the first frame, after the page is built.
    pPivot->_transitionElapsed = -1;
    if (pPivot->TransitionMode != kTransitionModeNone) OBJ_RequestAnimate(hObject);
  }
  if (id == ID_Pivot_SelectedIndex || id == ID_Pivot_Title || id == ID_Pivot_IsLocked ||
      id == ID_Pivot_ExtendBodyBehindHeader || id == ID_Pivot_BackIcon || id == ID_Pivot_TransitionMode) {
    OBJ_SetDirty(hObject);
  }
  return FALSE;
}

HANDLER(Pivot, Object, Animate)
{
  pPivot->_progress = MIN(Metro_Elapsed(pPivot->_animStart), 1);
  bool_t moving = FALSE;
  enum TransitionMode mode = pPivot->TransitionMode;
  if (mode != kTransitionModeNone) {
    float total = PageTransition_Duration(hObject, pPivot->NavigationTransition, mode);
    bool_t done = pPivot->_transitionElapsed >= total;
    if (pPivot->_transitionElapsed < 0) pPivot->_transitionStart = (int32_t)ANIM_GetTime();
    pPivot->_transitionElapsed = MIN((float)(uint32_t)(ANIM_GetTime() - (uint32_t)pPivot->_transitionStart), total);
    moving = pPivot->_transitionElapsed < total;
    if (!moving && !done) {
      // A page that went out stays away until the next mode.
      if (mode == kTransitionModeForwardIn || mode == kTransitionModeBackwardIn) {
        enum TransitionMode none = kTransitionModeNone;
        Metro_SetProperty(hObject, ID_Pivot_TransitionMode, &none);
      }
      axPostMessageW(hObject, ID_Pivot_TransitionCompleted, 0, 0);
    }
  }
  OBJ_SetDirty(hObject);
  if (pPivot->_progress < 1 || moving) OBJ_RequestAnimate(hObject);
  return TRUE;
}

// Lays the page out in place, then moves its parts along the transition.
// Until the first frame starts the clock, every part waits at its start.
HANDLER(Pivot, Node, UpdateMatrix)
{
  if (pPivot->TransitionMode == kTransitionModeNone) return FALSE;
  Node2D_UpdateMatrix(hObject, GetNode2D(hObject), wParam, pUpdateMatrix);
  PageTransition_Apply(hObject, pPivot->NavigationTransition, pPivot->TransitionMode,
    MAX(pPivot->_transitionElapsed, 0));
  return TRUE;
}

// A page in transition takes no input: the pivot catches what would reach
// its content, and ignores it.
HANDLER(Pivot, Node, HitTest)
{
  if (pPivot->TransitionMode == kTransitionModeNone) return FALSE;
  return Node2D_HitTest(hObject, GetNode2D(hObject), wParam, pHitTest) ? (LRESULT)(intptr_t)hObject : FALSE;
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
  // The title keeps clear of the top safe area, like a navigation bar.
  struct Node2D *node = GetNode2D(hObject);
  float top = MAX(y, node->_safeArea.x);
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
  bool_t back = pPivot->_back && GetNode(pPivot->_back)->Visible;
  float titleX = x + METRO_INSET + (back ? PIVOT_BACK_RING + PIVOT_BACK_GAP : 0);
  Metro_Arrange(pPivot->_title, titleX, top + PIVOT_TITLE_TOP, LOWORD(title), HIWORD(title));
  float stripY = top + PIVOT_TITLE_TOP + HIWORD(title) + PIVOT_HEADER_TOP, strip = 0;
  if (back) {
    // The whole title line, up through the status bar, is the back button.
    // The ring is centred on the title line and may reach into the gaps
    // above and below it, so the page's metrics stay the same.
    float right = MAX(titleX + LOWORD(title) + METRO_INSET, x + PIVOT_BACK_MIN);
    float bottom = MAX(stripY, top + PIVOT_BACK_MIN);
    float ringX = METRO_INSET, ringY = top - y + PIVOT_TITLE_TOP + (HIWORD(title) - PIVOT_BACK_RING) / 2;
    Metro_Arrange(pPivot->_back, x, y, right - x, bottom - y);
    Metro_Measure(pPivot->_backRing, PIVOT_BACK_RING, PIVOT_BACK_RING);
    Metro_Arrange(pPivot->_backRing, ringX, ringY, PIVOT_BACK_RING, PIVOT_BACK_RING);
    Metro_Measure(pPivot->_backIcon, PIVOT_BACK_ICON, PIVOT_BACK_ICON);
    Metro_Arrange(pPivot->_backIcon, ringX + (PIVOT_BACK_RING - PIVOT_BACK_ICON) / 2,
      ringY + (PIVOT_BACK_RING - PIVOT_BACK_ICON) / 2, PIVOT_BACK_ICON, PIVOT_BACK_ICON);
  }

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
    if (repeats == copyCount) {
      copies[copyCount++] = copy;
      OBJ_ReleaseRef(copy);  // found again among the children
    }
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

  // A body extending behind the header starts at the top and takes the
  // header as its top safe area; otherwise the header consumes it.
  float contentY = stripY + strip + PIVOT_CONTENT_TOP;
  bool_t extend = pPivot->ExtendBodyBehindHeader;
  node->_safeArea.x = node->_barInset.x = extend ? contentY : 0;
  Metro_Measure(pPivot->_bar, Node2D_GetFrame(node, kBox3FieldWidth), contentY);
  Metro_Arrange(pPivot->_bar, 0, 0, Node2D_GetFrame(node, kBox3FieldWidth), contentY);
  if (count) {
    float bodyY = extend ? y : contentY;
    Metro_Arrange(items[pPivot->SelectedIndex],
      x + METRO_INSET + pPivot->_contentFrom * settle + pPivot->_dragX, bodyY,
      MAX(w - 2 * METRO_INSET, 0), MAX(y + h - bodyY, 0));
  }
  return MAKEDWORD(w, h);
}

static bool_t
Pivot_HitsBack(struct Pivot *pPivot, float x, float y)
{
  return pPivot->_back && _SendMessage(pPivot->_back, Node, HitTest, .x = x, .y = y);
}

HANDLER(Pivot, Node, LeftButtonDown)
{
  if (pPivot->TransitionMode != kTransitionModeNone) return TRUE;
  bool_t back = pPivot->_back && GetNode(pPivot->_back)->Visible;
  pPivot->_tracking = TRUE;
  pPivot->_dragging = FALSE;
  pPivot->_pressX = pLeftButtonDown->x;
  pPivot->_pressY = pLeftButtonDown->y;
  pPivot->_backPressed = back && Pivot_HitsBack(pPivot, pLeftButtonDown->x, pLeftButtonDown->y);
  if (back) {
    struct mat4 inv = MAT4_Inverse(&GetNode2D(hObject)->Matrix);
    struct vec3 local = MAT4_MultiplyVector3D(&inv, &(struct vec3){ pLeftButtonDown->x, pLeftButtonDown->y, 0 });
    pPivot->_edge = local.x < PIVOT_EDGE;
  } else pPivot->_edge = FALSE;
  return FALSE;
}

HANDLER(Pivot, Node, LeftButtonDragged)
{
  if (pPivot->TransitionMode != kTransitionModeNone) return TRUE;
  struct Object *items[PIVOT_MAX_ITEMS];
  int count = Pivot_Items(hObject, items);
  bool_t edge = pPivot->_edge;
  if (!pPivot->_tracking || (!edge && (pPivot->IsLocked || count < 2)) || !count) return FALSE;
  float dx = pLeftButtonDragged->x - pPivot->_pressX;
  float dy = pLeftButtonDragged->y - pPivot->_pressY;
  if (!pPivot->_dragging) {
    if (fabsf(dx) < METRO_DRAG_START || fabsf(dx) < fabsf(dy) * 1.5f) return FALSE;
    // A back swipe moves right only; a leftward one may still change items.
    if (edge && dx < 0) {
      pPivot->_edge = edge = FALSE;
      if (pPivot->IsLocked || count < 2) return FALSE;
    }
    pPivot->_dragging = TRUE;
    pPivot->_backPressed = FALSE;
    pPivot->_progress = 1;
    // Hide the content from hit testing so releasing the swipe is not a tap.
    GetNode2D(items[pPivot->SelectedIndex])->IgnoreHitTest = TRUE;
  }
  pPivot->_dragX = edge ? MAX(dx, 0) : dx;
  OBJ_SetDirty(hObject);
  return TRUE;
}

HANDLER(Pivot, Node, LeftButtonUp)
{
  if (pPivot->TransitionMode != kTransitionModeNone) return TRUE;
  bool_t dragging = pPivot->_dragging, edge = pPivot->_edge;
  bool_t back = pPivot->_backPressed && Pivot_HitsBack(pPivot, pLeftButtonUp->x, pLeftButtonUp->y);
  float dx = pPivot->_dragX;
  pPivot->_tracking = pPivot->_dragging = pPivot->_backPressed = pPivot->_edge = FALSE;
  pPivot->_dragX = 0;
  if (back && !dragging) {
    axPostMessageW(hObject, ID_Pivot_BackRequested, 0, 0);
    return TRUE;
  }
  if (!dragging) return FALSE;

  struct Object *items[PIVOT_MAX_ITEMS];
  int count = Pivot_Items(hObject, items);
  if (!count) return TRUE;
  GetNode2D(items[pPivot->SelectedIndex])->IgnoreHitTest = FALSE;
  if (edge && dx >= PIVOT_BACK_SWIPE) {
    axPostMessageW(hObject, ID_Pivot_BackRequested, 0, 0);
  } else if (!edge && fabsf(dx) >= METRO_SWIPE) {
    Pivot_Select(hObject, (pPivot->SelectedIndex + (dx < 0 ? 1 : count - 1)) % count);
    return TRUE;
  }
  // Not far enough, or the page stays while the app goes back: settle into place.
  pPivot->_headerFrom = dx * 0.3f;
  pPivot->_contentFrom = dx;
  Pivot_StartAnimation(hObject, pPivot);
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
    if (items[i] != pPivotHeaderItem->_item) continue;
    if (i == pPivot->SelectedIndex) axPostMessageW(pivot, ID_Pivot_HeaderClick, 0, 0);
    else Pivot_Select(pivot, i);
  }
  pPivot->_tracking = FALSE;
  return TRUE;
}
