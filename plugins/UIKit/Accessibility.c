#include <include/orca.h>

#include <UIKit/UIKit.h>
#include "Accessibility.h"

// What assistive technologies read (W11): the label of an element and the
// elements of a drawn screen. Platform bridges (source/ios_accessibility.m)
// turn the elements into native ones; nothing here depends on a platform.

#define A11Y_LABEL_MAX 1024
#define A11Y_DESCRIBE_MAX 65536

struct a11y_text {
  char text[A11Y_LABEL_MAX];
  size_t length;
};

static bool_t
a11y_shown(struct Object *object, struct Node2D *node)
{
  return node && !OBJ_IsHidden(object) && node->_opacity > 0 && !node->AccessibilityHidden;
}

static bool_t
a11y_has(lpcString_t text)
{
  return text && *text;
}

// The text a TextBlock draws, as TextBlockConcept resolves it, without the
// placeholder (an Input's placeholder is its label instead).
static lpcString_t
a11y_own_text(struct Object *object)
{
  struct TextBlockConcept *concept = GetTextBlockConcept(object);
  struct TextRun *run = GetTextRun(object);
  if (!concept || !run) return NULL;
  struct Property *text = OBJ_FindLongProperty(object, ID_TextRun_Text);
  if (a11y_has(concept->TextResourceID) && !(text && PROP_HasProgram(text)))
    return core_GetLocalizedString(object, concept->TextResourceID);
  if (a11y_has(run->Text)) return run->Text;
  lpcString_t content = OBJ_GetTextContent(object);
  return a11y_has(content) ? content : NULL;
}

// Appends with `separator` before it, folding runs of white space to one
// space; stops quietly when full.
static void
a11y_append(struct a11y_text *out, lpcString_t text, lpcString_t separator)
{
  if (!a11y_has(text)) return;
  while (*text == ' ' || *text == '\n' || *text == '\t' || *text == '\r') text++;
  if (!*text) return;
  if (out->length && separator)
    for (; *separator && out->length + 1 < sizeof(out->text); separator++) out->text[out->length++] = *separator;
  bool_t space = FALSE;
  for (; *text && out->length + 1 < sizeof(out->text); text++) {
    bool_t blank = *text == ' ' || *text == '\n' || *text == '\t' || *text == '\r';
    if (blank) { space = TRUE; continue; }
    if (space) out->text[out->length++] = ' ';
    space = FALSE;
    if (out->length + 1 < sizeof(out->text)) out->text[out->length++] = *text;
  }
  out->text[out->length] = 0;
}

static void
a11y_gather(struct Object *object, struct a11y_text *out)
{
  struct Node2D *node = GetNode2D(object);
  if (!a11y_shown(object, node)) return;
  if (a11y_has(node->AccessibilityLabel)) {
    a11y_append(out, node->AccessibilityLabel, ", ");
    return;
  }
  struct Input *input = GetInput(object);
  if (input) {
    // A field reads as its placeholder; a password never reads its text.
    struct TextBlockConcept *concept = GetTextBlockConcept(object);
    a11y_append(out, concept ? concept->PlaceholderText : NULL, ", ");
    if (input->Type != kInputTypePassword) a11y_append(out, a11y_own_text(object), ", ");
    return;
  }
  lpcString_t own = a11y_own_text(object);
  if (own) {
    a11y_append(out, own, ", ");
    // Inline Runs continue the same sentence.
    FOR_EACH_OBJECT(child, object) {
      struct TextRun *run = GetNode2D(child) ? NULL : GetTextRun(child);
      if (run) a11y_append(out, a11y_has(run->Text) ? run->Text : OBJ_GetTextContent(child), " ");
    }
  }
  FOR_EACH_OBJECT(child, object) if (GetNode2D(child)) a11y_gather(child, out);
}

static void
a11y_label(struct Object *object, struct a11y_text *out)
{
  out->length = 0;
  out->text[0] = 0;
  a11y_gather(object, out);
}

lpcString_t
UIKit_GetAccessibilityLabel(struct Object *object)
{
  static struct a11y_text label;
  a11y_label(object, &label);
  return label.text;
}

// The trait an element has, or Auto when it is not an element of its own
// and is read through its children.
static enum AccessibilityTraits
a11y_traits(struct Object *object, struct Node2D *node)
{
  if (node->AccessibilityTraits != kAccessibilityTraitsAuto) return node->AccessibilityTraits;
  if (GetButton(object)) return kAccessibilityTraitsButton;
  if (GetInput(object)) return kAccessibilityTraitsNone;
  if (a11y_has(node->AccessibilityLabel))
    return GetVectorView(object) || GetImageView(object) ? kAccessibilityTraitsImage : kAccessibilityTraitsNone;
  if (a11y_own_text(object)) return kAccessibilityTraitsStaticText;
  return kAccessibilityTraitsAuto;
}

struct a11y_rect {
  float x0, y0, x1, y1;
};

static struct a11y_rect
a11y_frame(struct Node2D *node)
{
  float x = node->Matrix.v[12], y = node->Matrix.v[13];
  return (struct a11y_rect) { x, y, x + Node2D_GetFrame(node, kBox3FieldWidth) * node->Matrix.v[0],
                              y + Node2D_GetFrame(node, kBox3FieldHeight) * node->Matrix.v[5] };
}

static struct a11y_rect
a11y_intersect(struct a11y_rect a, struct a11y_rect b)
{
  return (struct a11y_rect) { MAX(a.x0, b.x0), MAX(a.y0, b.y0), MIN(a.x1, b.x1), MIN(a.y1, b.y1) };
}

static bool_t
a11y_empty(struct a11y_rect r)
{
  return !(r.x1 - r.x0 >= 1 && r.y1 - r.y0 >= 1);
}

struct a11y_walk {
  AccessibilityVisitor visit;
  void *context;
  uint32_t count;
  struct a11y_text label;
};

static void a11y_children(struct Object *object, struct a11y_rect clip, struct a11y_walk *walk);

static void
a11y_walk(struct Object *object, struct a11y_rect clip, struct a11y_walk *walk)
{
  struct Node2D *node = GetNode2D(object);
  if (!a11y_shown(object, node)) return;
  struct a11y_rect frame = a11y_frame(node);
  enum AccessibilityTraits traits = a11y_traits(object, node);
  if (traits != kAccessibilityTraitsAuto) {
    struct a11y_rect visible = a11y_intersect(frame, clip);
    if (a11y_empty(visible)) return;
    a11y_label(object, &walk->label);
    struct AccessibilityElement element = {
      .object = object, .x = visible.x0, .y = visible.y0,
      .width = visible.x1 - visible.x0, .height = visible.y1 - visible.y0,
      .label = walk->label.text, .hint = a11y_has(node->AccessibilityHint) ? node->AccessibilityHint : "",
      .traits = (int)traits,
    };
    walk->visit(&element, walk->context);
    walk->count++;
    return;
  }
  if (node->Overflow.x != kOverflowVisible || node->Overflow.y != kOverflowVisible) {
    clip = a11y_intersect(clip, frame);
    if (a11y_empty(clip)) return;
  }
  a11y_children(object, clip, walk);
}

// Children in reading order. A Panorama and a Pivot add their title and
// headers after the items they label, so the title comes first, then a
// Panorama section's header before its section, a Pivot's headers before
// its items.
static void
a11y_children(struct Object *object, struct a11y_rect clip, struct a11y_walk *walk)
{
  struct Panorama *panorama = GetPanorama(object);
  if (panorama) {
    if (panorama->_title) a11y_walk(panorama->_title, clip, walk);
    FOR_EACH_OBJECT(child, object) {
      struct PanoramaItem *item = GetPanoramaItem(child);
      if (!item) continue;
      if (item->_header) a11y_walk(item->_header, clip, walk);
      a11y_walk(child, clip, walk);
    }
    return;
  }
  if (GetPivot(object)) {
    FOR_EACH_OBJECT(child, object) if (!GetPivotItem(child)) a11y_walk(child, clip, walk);
    FOR_EACH_OBJECT(child, object) if (GetPivotItem(child)) a11y_walk(child, clip, walk);
    return;
  }
  FOR_EACH_OBJECT(child, object) a11y_walk(child, clip, walk);
}

struct Object *
UIKit_AccessibilityScreen(struct Object *screen)
{
  if (!screen) return NULL;
  // As Screen's renderer: later visible modal siblings draw over it.
  for (struct Object *next = OBJ_GetNext(screen); next; next = OBJ_GetNext(next))
    if (GetScreen(next) && GetScreen(next)->Visible) screen = next;
  return screen;
}

uint32_t
UIKit_CollectAccessibility(struct Object *screen, float width, float height, AccessibilityVisitor visit,
                           void *context)
{
  screen = UIKit_AccessibilityScreen(screen);
  if (!screen || !visit) return 0;
  static struct a11y_walk walk;
  walk.visit = visit;
  walk.context = context;
  walk.count = 0;
  struct a11y_rect clip = { 0, 0, width, height };
  if (GetNode2D(screen)) a11y_walk(screen, clip, &walk);
  else FOR_EACH_OBJECT(child, screen) a11y_walk(child, clip, &walk);
  return walk.count;
}

struct a11y_describe {
  char *text;
  size_t length;
};

static void
a11y_describe_one(struct AccessibilityElement const *element, void *context)
{
  struct a11y_describe *out = context;
  size_t room = A11Y_DESCRIBE_MAX - out->length;
  int n = snprintf(out->text + out->length, room, "%s\t%s%s%s\n",
                   AccessibilityTraitsToString((enum AccessibilityTraits)element->traits), element->label,
                   *element->hint ? "\t" : "", element->hint);
  if (n > 0) out->length += MIN((size_t)n, room ? room - 1 : 0);
}

lpcString_t
UIKit_DescribeAccessibility(struct Object *screen)
{
  static char text[A11Y_DESCRIBE_MAX];
  struct a11y_describe out = { text, 0 };
  text[0] = 0;
  struct Object *shown = UIKit_AccessibilityScreen(screen);
  if (!shown) return text;
  // The screen's size: its top-level children fill it.
  float width = 0, height = 0;
  FOR_EACH_OBJECT(child, shown) {
    struct Node2D *node = GetNode2D(child);
    if (!node) continue;
    struct a11y_rect frame = a11y_frame(node);
    width = MAX(width, frame.x1);
    height = MAX(height, frame.y1);
  }
  UIKit_CollectAccessibility(screen, width, height, a11y_describe_one, &out);
  return text;
}
