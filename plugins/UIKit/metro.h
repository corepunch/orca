// Shared helpers for the Windows Phone controls (Pivot, Panorama, ApplicationBar).
// Metrics are Windows Phone 8 values (480-wide screen) scaled to ORCA's
// 390-point portrait viewport.
#ifndef ORCA_UIKIT_METRO_H
#define ORCA_UIKIT_METRO_H

#include <ctype.h>
#include <math.h>
#include <include/orca.h>
#include <source/core/core_local.h>
#include <UIKit/UIKit.h>

#define METRO_INSET          12.0f   // page margin: the outer 12 of Windows Phone's 12 + 12
#define METRO_ANIM_MS        300
#define METRO_SWIPE          48.0f   // distance that commits a swipe
#define METRO_DRAG_START     12.0f   // distance before a press becomes a swipe
#define METRO_DIM            0.4f    // opacity of inactive headers and disabled commands

static inline float
Metro_Ease(float t)
{
  t = t < 0 ? 0 : t > 1 ? 1 : t;
  return 1 - (1 - t) * (1 - t) * (1 - t);
}

static inline float
Metro_Elapsed(int32_t start)
{
  return (float)((int32_t)axGetMilliseconds() - start) / METRO_ANIM_MS;
}

static inline void
Metro_SetProperty(struct Object *o, uint32_t id, void const *value)
{
  struct Property *p = OBJ_FindLongProperty(o, id);
  if (p) PROP_SetValue(p, value);
}

static inline void
Metro_SetFloat(struct Object *o, uint32_t id, float value)
{
  Metro_SetProperty(o, id, &value);
}

// Sets a TextBlock's text, transformed to lower or upper case, when it differs.
static inline void
Metro_SetText(struct Object *o, lpcString_t text, int (*transform)(int))
{
  char buf[MAX_PROPERTY_STRING] = {0};
  size_t i = 0;
  for (; text && text[i] && i < sizeof(buf) - 1; i++)
    buf[i] = (char)((unsigned char)text[i] < 128 ? transform((unsigned char)text[i]) : text[i]);
  struct TextRun *run = GetTextRun(o);
  if (run && run->Text && !strcmp(run->Text, buf)) return;
  struct Property *p = OBJ_FindLongProperty(o, ID_TextRun_Text);
  if (p) PROP_SetStringValue(p, buf);
}

// Creates a non-interactive text part under parent. The caller keeps the
// creation reference: it releases it at once or when it drops the pointer.
static inline struct Object *
Metro_CreateText(struct Object *parent, uint32_t class_id, lpcString_t css, float size, bool_t bold)
{
  struct Object *o = OBJ_Create(class_id);
  if (!o) return NULL;
  Metro_SetFloat(o, ID_TextRun_FontSize, size);
  if (bold) {
    enum FontWeight weight = kFontWeightBold;
    Metro_SetProperty(o, ID_TextRun_FontWeight, &weight);
  }
  _SendMessage(o, StyleController, AddClass, .ClassName = css);
  OBJ_AddChild(parent, o);
  return o;
}

static inline LRESULT
Metro_Measure(struct Object *o, float width, float height)
{
  return _SendMessage(o, Node2D, Measure, .Width = width, .Height = height);
}

static inline void
Metro_Arrange(struct Object *o, float x, float y, float width, float height)
{
  _SendMessage(o, Node2D, Arrange, .X = x, .Y = y, .Width = width, .Height = height);
}

static inline void
Metro_Show(struct Object *o, bool_t visible, float opacity)
{
  struct Node *n = o ? GetNode(o) : NULL;
  if (!n || (n->Visible == visible && n->Opacity == opacity)) return;
  n->Visible = visible;
  n->Opacity = opacity;
  OBJ_SetDirty(o);
}

// Page transitions (PageTransition.c). Apply moves a laid-out page's parts
// `elapsed` milliseconds into the transition; Duration is when the last part
// finishes. A type without motion for a mode lasts 0 ms.
float PageTransition_Duration(struct Object *page, enum TransitionType type, enum TransitionMode mode);
void PageTransition_Apply(struct Object *page, enum TransitionType type, enum TransitionMode mode, float elapsed);
void PageTransition_NativeChanged(struct Object *page, enum TransitionType type, enum TransitionMode mode);
bool_t PageTransition_NativeAnimate(struct Object *page);
bool_t PageTransition_NativeActive(struct Object *page);
void PageTransition_NativePresent(struct Object *screen, uint32_t width, uint32_t height);

#endif
