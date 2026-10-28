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
