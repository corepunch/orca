#include "metro.h"

// Windows Phone page transitions. The motion follows WinJS, the web
// implementation of the Windows Phone 8.1 animations; the feather follows
// Windows Phone 8.1, which delays each element 33 ms after the one above it.
// docs/windows-phone-transitions.md records the sources and the values.
#define TRANSITION_HINGE     40.0f          // turnstile hinge, left of the page
#define TRANSITION_STAGGER   (33.0f / 80)   // ms per point: 33 ms per 80-point row

struct Bezier { float x1, y1, x2, y2; };

static struct Bezier const kLinear = { 0, 0, 1, 1 };
static struct Bezier const kTurnIn = { 0.01f, 0.975f, 0.4775f, 0.9775f };
static struct Bezier const kTurnOut = { 0.4925f, 0.01f, 0.7675f, -0.01f };
static struct Bezier const kSlideUp = { 0.17f, 0.79f, 0.215f, 1.0025f };
static struct Bezier const kSlideDown = { 0.3825f, 0.0025f, 0.8775f, -0.1075f };
static struct Bezier const kShow = { 0, 2, 0, 2 };                  // opacity steps to 1 at once
static struct Bezier const kHide = { 1, -0.42f, 0.995f, -0.425f };  // and to 0 at the end

// One phase of a transition: each part moves for `ms` from the first value
// of each pair to the second.
struct Motion {
  float ms;
  float turn[2];      // rotateY in degrees, as CSS
  float depth;        // CSS perspective in points
  float slide[2];     // vertical offset in points
  float opacity[2];
  struct Bezier const *move, *fade;
};

#define TRANSITION_MODES (kTransitionModeBackwardOut + 1)

static struct Motion const kMotions[][TRANSITION_MODES] = {
  [kTransitionTypeTurnstile] = {
    [kTransitionModeForwardIn] = { 300, { 80, 0 }, 600, { 0, 0 }, { 0, 1 }, &kTurnIn, &kShow },
    [kTransitionModeForwardOut] = { 128, { 0, -50 }, 600, { 0, 0 }, { 1, 0 }, &kTurnOut, &kHide },
    [kTransitionModeBackwardIn] = { 300, { -50, 0 }, 600, { 0, 0 }, { 0, 1 }, &kTurnIn, &kShow },
    [kTransitionModeBackwardOut] = { 128, { 0, 80 }, 800, { 0, 0 }, { 1, 0 }, &kTurnOut, &kHide },
  },
  // Forward, the new page slides up over an instant exit; back, the old
  // page slides down and the previous one returns at once.
  [kTransitionTypeSlide] = {
    [kTransitionModeForwardIn] = { 350, { 0, 0 }, 600, { 200, 0 }, { 0, 1 }, &kSlideUp, &kShow },
    [kTransitionModeBackwardOut] = { 250, { 0, 0 }, 600, { 0, 200 }, { 1, 0 }, &kSlideDown, &kHide },
  },
  [kTransitionTypeFade] = {
    [kTransitionModeForwardIn] = { 250, { 0, 0 }, 600, { 0, 0 }, { 0, 1 }, &kLinear, &kLinear },
    [kTransitionModeForwardOut] = { 167, { 0, 0 }, 600, { 0, 0 }, { 1, 0 }, &kLinear, &kLinear },
    [kTransitionModeBackwardIn] = { 250, { 0, 0 }, 600, { 0, 0 }, { 0, 1 }, &kLinear, &kLinear },
    [kTransitionModeBackwardOut] = { 167, { 0, 0 }, 600, { 0, 0 }, { 1, 0 }, &kLinear, &kLinear },
  },
};

struct Transition {
  struct Motion const *motion;
  bool_t feather;
  struct vec2 hinge;
  float top, elapsed;
};

static struct Motion const *
Transition_Motion(enum TransitionType type, enum TransitionMode mode)
{
  if (type >= sizeof(kMotions) / sizeof(*kMotions) || mode >= TRANSITION_MODES) return NULL;
  struct Motion const *motion = &kMotions[type][mode];
  return motion->move ? motion : NULL;
}

// CSS cubic-bezier(x1, y1, x2, y2) at time t.
static float
Transition_Ease(struct Bezier const *b, float t)
{
  float lo = 0, hi = 1, u = t;
  for (int i = 0; i < 20; i++) {
    u = (lo + hi) / 2;
    float x = 3 * (1 - u) * (1 - u) * u * b->x1 + 3 * (1 - u) * u * u * b->x2 + u * u * u;
    if (x < t) lo = u; else hi = u;
  }
  return 3 * (1 - u) * (1 - u) * u * b->y1 + 3 * (1 - u) * u * u * b->y2 + u * u * u;
}

static float
Transition_Lerp(float const range[2], float t)
{
  return range[0] + (range[1] - range[0]) * t;
}

// translateY(slide) perspective(depth) rotateY(degrees) about the hinge, in
// screen space, as CSS composes it.
static struct mat4
Transition_Matrix(struct vec2 hinge, float slide, float degrees, float depth)
{
  float a = degrees * (float)M_PI / 180, c = cosf(a), s = sinf(a);
  struct mat4 to = MAT4_Identity(), from = MAT4_Identity(), turn = MAT4_Identity();
  to.v[12] = hinge.x, to.v[13] = hinge.y + slide;
  from.v[12] = -hinge.x, from.v[13] = -hinge.y;
  // x' = x cos + z sin, z' = z cos - x sin, w = 1 - z' / depth
  turn.v[0] = c, turn.v[2] = -s, turn.v[3] = s / depth;
  turn.v[8] = s, turn.v[10] = c, turn.v[11] = -c / depth;
  struct mat4 m = MAT4_Multiply(&to, &turn);
  return MAT4_Multiply(&m, &from);
}

// Parts are what draws something of its own: a decorated node keeps its
// content with it, and a bare container is split into its children.
static bool_t
Transition_IsPart(struct Object *o)
{
  struct Node2D *n = GetNode2D(o);
  struct Node *node = GetNode(o);
  if (n->Background.Color.a > 0 || n->Background.Image || n->Background.Material ||
      n->BackdropFilter.BlurRadius > 0) return TRUE;
  for (int i = 0; i < 2; i++) {
    if (node->Border.Width.Axis[i].Left > 0 || node->Border.Width.Axis[i].Right > 0) return TRUE;
  }
  FOR_EACH_OBJECT(child, o) {
    if (GetNode2D(child)) return FALSE;
  }
  return TRUE;
}

static void
Transition_Transform(struct Object *o, struct mat4 const *m, float opacity)
{
  struct Node2D *n = GetNode2D(o);
  if (!n) return;
  n->Matrix = MAT4_Multiply(m, &n->Matrix);
  n->_opacity *= opacity;
  FOR_EACH_OBJECT(child, o) Transition_Transform(child, m, opacity);
}

// Moves a part by its own clock, which starts later the lower it begins.
static void
Transition_Part(struct Object *o, struct Transition const *t)
{
  struct Motion const *m = t->motion;
  float lag = t->feather ? MAX(GetNode2D(o)->Matrix.v[13] - t->top, 0) * TRANSITION_STAGGER : 0;
  float p = m->ms > 0 ? MIN(MAX((t->elapsed - lag) / m->ms, 0), 1) : 1;
  float move = Transition_Ease(m->move, p);
  float opacity = MIN(MAX(Transition_Lerp(m->opacity, Transition_Ease(m->fade, p)), 0), 1);
  struct mat4 matrix = Transition_Matrix(t->hinge, Transition_Lerp(m->slide, move),
    Transition_Lerp(m->turn, move), m->depth);
  Transition_Transform(o, &matrix, opacity);
}

static void
Transition_Feather(struct Object *o, struct Transition const *t)
{
  if (Transition_IsPart(o)) {
    Transition_Part(o, t);
    return;
  }
  FOR_EACH_OBJECT(child, o) {
    if (GetNode2D(child) && !OBJ_IsHidden(child)) Transition_Feather(child, t);
  }
}

float
PageTransition_Duration(struct Object *page, enum TransitionType type, enum TransitionMode mode)
{
  struct Motion const *motion = Transition_Motion(type, mode);
  if (!motion) return 0;
  float stagger = type == kTransitionTypeTurnstile ?
    Node2D_GetFrame(GetNode2D(page), kBox3FieldHeight) * TRANSITION_STAGGER : 0;
  return motion->ms + stagger;
}

void
PageTransition_Apply(struct Object *page, enum TransitionType type, enum TransitionMode mode, float elapsed)
{
  struct Motion const *motion = Transition_Motion(type, mode);
  if (!motion) return;
  struct Node2D *node = GetNode2D(page);
  float height = Node2D_GetFrame(node, kBox3FieldHeight);
  struct Transition t = {
    .motion = motion,
    .feather = type == kTransitionTypeTurnstile,
    .hinge = { node->Matrix.v[12] - TRANSITION_HINGE, node->Matrix.v[13] + height / 2 },
    .top = node->Matrix.v[13],
    .elapsed = elapsed,
  };
  if (t.feather) Transition_Feather(page, &t);
  else Transition_Part(page, &t);
}

#if defined(AX_PLATFORM_IOS) || defined(ORCA_TEST_NATIVE_TRANSITIONS)
#include <source/ios_page_transition.h>

enum NativeTransitionPhase { NativeTransitionWaiting, NativeTransitionPlaying, NativeTransitionFinished, NativeTransitionHeld };
static struct {
  struct Object *page;
  uint64_t generation;
  enum TransitionMode mode;
  enum NativeTransitionPhase phase;
} native_transition;
static uint64_t native_transition_generation;

static struct Object *
NativeTransition_Screen(struct Object *page)
{
  for (struct Object *o = page; o; o = OBJ_GetParent(o)) if (GetScreen(o)) return o;
  return NULL;
}

static void
NativeTransition_ClipBars(struct Object *object, struct Object *page, float *bottom)
{
  if (object == page) return;
  struct Node2D *node = GetNode2D(object);
  if (node && OBJ_IsHidden(object)) return;
  if (node && node->SafeAreaEdge == kSafeAreaEdgeBottom)
    *bottom = MIN(*bottom, node->Matrix.v[13]);
  FOR_EACH_OBJECT(child, object) NativeTransition_ClipBars(child, page, bottom);
}

static void
NativeTransition_Reset(bool_t clear_overlay)
{
  struct Object *page = native_transition.page;
  memset(&native_transition, 0, sizeof(native_transition));
  if (clear_overlay) ORCA_IOS_ClearPageTransition();
  OBJ_ReleaseRef(page);
}

static void
NativeTransition_Completed(uint64_t generation, bool_t finished)
{
  // The callback carries no engine pointer. Reset invalidates old generations
  // before releasing the page, including during scene teardown or hot reload.
  if (!native_transition.page || native_transition.generation != generation) return;
  struct Object *page = native_transition.page;
  struct Pivot *pivot = GetPivot(page);
  pivot->_transitionElapsed = PageTransition_Duration(page, kTransitionTypeTurnstile, native_transition.mode);
  native_transition.phase = NativeTransitionFinished;
  OBJ_RequestAnimate(page);
  if (!finished) {
    // Cancellation still resolves navigation, but teardown must not retain a
    // page until another display callback (which may never arrive).
    PageTransition_NativeAnimate(page);
    NativeTransition_Reset(FALSE);
  }
}

void ORCA_IOS_CancelPageTransitionRequests(void)
{
  if (native_transition.phase == NativeTransitionHeld) NativeTransition_Reset(FALSE);
  else if (native_transition.page) NativeTransition_Completed(native_transition.generation, FALSE);
}

void
PageTransition_NativeChanged(struct Object *page, enum TransitionType type, enum TransitionMode mode)
{
  bool_t start = type == kTransitionTypeTurnstile && mode != kTransitionModeNone && ORCA_IOS_CanPageTransition();
  // A retained or retiring page must not cancel another page's entrance when
  // its own transition mode is cleared.
  if (!start && native_transition.page != page) return;
  // A completed exit remains covered until the incoming capture is ready.
  NativeTransition_Reset(!start);
  if (!start) return;
  OBJ_AddRef(page);
  native_transition.page = page;
  native_transition.generation = ++native_transition_generation;
  native_transition.mode = mode;
  native_transition.phase = NativeTransitionWaiting;
}

bool_t PageTransition_NativeActive(struct Object *page) { return native_transition.page == page; }

bool_t
PageTransition_NativeAnimate(struct Object *page)
{
  if (native_transition.page != page) return FALSE;
  if (native_transition.phase == NativeTransitionFinished) {
    OBJ_AddRef(page);
    enum TransitionMode mode = native_transition.mode;
    uint64_t generation = native_transition.generation;
    native_transition.phase = NativeTransitionHeld;
    OBJ_SetDirty(page);
    if (mode == kTransitionModeForwardIn || mode == kTransitionModeBackwardIn) {
      enum TransitionMode none = kTransitionModeNone;
      Metro_SetProperty(page, ID_Pivot_TransitionMode, &none);
      // Also release ownership when a platform/test adapter doesn't deliver
      // the property-change notification synchronously.
      if (native_transition.page == page && native_transition.generation == generation)
        NativeTransition_Reset(TRUE);
    }
    axPostMessageW(page, ID_Pivot_TransitionCompleted, 0, NULL);
    OBJ_ReleaseRef(page);
  }
  return TRUE;
}

void
PageTransition_NativePresent(struct Object *screen, uint32_t width, uint32_t height)
{
  struct Object *page = native_transition.page;
  if (!page) return;
  if (NativeTransition_Screen(page) != screen) {
    // A route without an entrance releases the old cover after it is drawn.
    if (native_transition.phase == NativeTransitionHeld) NativeTransition_Reset(TRUE);
    return;
  }
  if (native_transition.phase == NativeTransitionPlaying) {
    // Network/content paints may arrive after the entrance's first capture.
    // Replace only its pixels; Core Animation keeps the original motion clock.
    if (native_transition.mode == kTransitionModeForwardIn || native_transition.mode == kTransitionModeBackwardIn)
      ORCA_IOS_UpdatePageTransition(native_transition.generation);
    return;
  }
  if (native_transition.phase != NativeTransitionWaiting) return;
  struct Node2D *node = GetNode2D(page);
  struct Motion const *motion = Transition_Motion(kTransitionTypeTurnstile, native_transition.mode);
  struct Screen *view = GetScreen(screen);
  float bottom = MIN(node->Matrix.v[13] + Node2D_GetFrame(node, kBox3FieldHeight), height);
  NativeTransition_ClipBars(screen, page, &bottom);
  struct OrcaIOSPageTransition spec = {
    .x = node->Matrix.v[12], .y = node->Matrix.v[13],
    .width = MIN(Node2D_GetFrame(node, kBox3FieldWidth), width),
    .height = MAX(bottom - node->Matrix.v[13], 0),
    .background = { view->ClearColor.r, view->ClearColor.g, view->ClearColor.b, view->ClearColor.a },
    .fromAngle = motion->turn[0], .toAngle = motion->turn[1],
    .perspective = motion->depth, .duration = motion->ms / 1000,
    .curve = { motion->move->x1, motion->move->y1, motion->move->x2, motion->move->y2 },
    .hold = native_transition.mode == kTransitionModeForwardOut || native_transition.mode == kTransitionModeBackwardOut,
  };
  native_transition.phase = NativeTransitionPlaying;
  if (!ORCA_IOS_StartPageTransition(&spec, native_transition.generation, NativeTransition_Completed)) {
    // Failed capture falls back to the renderer and keeps navigation live.
    OBJ_RequestAnimate(page);
    NativeTransition_Reset(TRUE);
  }
}
#else
void PageTransition_NativeChanged(struct Object *page, enum TransitionType type, enum TransitionMode mode)
{ (void)page; (void)type; (void)mode; }
bool_t PageTransition_NativeAnimate(struct Object *page) { (void)page; return FALSE; }
bool_t PageTransition_NativeActive(struct Object *page) { (void)page; return FALSE; }
void PageTransition_NativePresent(struct Object *screen, uint32_t width, uint32_t height)
{ (void)screen; (void)width; (void)height; }
#endif
