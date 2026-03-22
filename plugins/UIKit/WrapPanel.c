#include <math.h>
#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <plugins/UIKit/UIKit.h>

/*
 * WrapPanel — a layout container that arranges children along a primary axis
 * and wraps onto a new line/column when the available space is exhausted.
 *
 * Direction == Horizontal (default):
 *   Children flow left-to-right; wrap into new rows.
 *   main axis  = X (width),  cross axis = Y (height).
 *
 * Direction == Vertical:
 *   Children flow top-to-bottom; wrap into new columns.
 *   main axis  = Y (height), cross axis = X (width).
 *
 * Properties:
 *   Direction   – Horizontal or Vertical (enum Direction).
 *   Spacing     – Gap between items within a line and between lines.
 *   ItemWidth   – Fixed width for every item (0 = use measured width).
 *   ItemHeight  – Fixed height for every item (0 = use measured height).
 *
 * Line-breaking algorithm — non-greedy:
 *   WrapPanel tries to fit all remaining children on the current line by
 *   giving each an equal share of the available main-axis space.  A child
 *   wraps to the next line only when its MinWidth (or MinHeight for a
 *   Vertical panel) would be violated by the equal-share allocation.
 *
 *   Consequence: elements with no MinWidth set are never forced to wrap;
 *   they shrink to share the line.  Set MinWidth (or MinHeight) to control
 *   the minimum acceptable size before wrapping.
 *
 *   When ItemWidth/ItemHeight is set the override width is used for every
 *   item (traditional fixed-size behaviour).  When avail_main is INFINITY
 *   (e.g. inside a StackView) all children share one line and are measured
 *   with INFINITY so their natural sizes are reported.
 */

#define MAX_WRAP_CHILDREN 256
#define MAX_WRAP_LINES    128

/* One "line" (row for Horizontal, column for Vertical). */
typedef struct {
  int   start;      /* index of first child in this line */
  int   count;      /* number of children in this line   */
  float cross_size; /* max child cross-axis size in this line */
  float share;      /* main-axis allocation per item on this line */
} WrapLine;

/* -----------------------------------------------------------------------
 * _CollectChildren – fill children[] with all layoutable child objects.
 * Returns the count of collected children.
 * ----------------------------------------------------------------------- */
static int
_CollectChildren(lpObject_t hObject, lpObject_t *children, int max_count)
{
  int n = 0;
  FOR_EACH_LAYOUTABLE(hChild, hObject)
  {
    if (n < max_count)
      children[n++] = hChild;
  }
  return n;
}

/* -----------------------------------------------------------------------
 * _ChildCrossSize – effective cross-axis size of a child after measuring.
 * ----------------------------------------------------------------------- */
static float
_ChildCrossSize(Node2DPtr subview, enum Direction direction)
{
  if (direction == kDirectionHorizontal)
    return NODE2D_FRAME(subview, Size, 1).Desired + TOTAL_MARGIN(subview, 1);
  else
    return NODE2D_FRAME(subview, Size, 0).Desired + TOTAL_MARGIN(subview, 0);
}

/* -----------------------------------------------------------------------
 * _ChildMinMain – MinWidth (horizontal) or MinHeight (vertical) of a child.
 * ----------------------------------------------------------------------- */
static float
_ChildMinMain(Node2DPtr subview, enum Direction direction)
{
  float min = (direction == kDirectionHorizontal)
              ? NODE2D_FRAME(subview, Size, 0).Min
              : NODE2D_FRAME(subview, Size, 1).Min;
  return isnan(min) ? 0.0f : min;
}

/* -----------------------------------------------------------------------
 * _LineShare – main-axis allocation per item for a line with 'count' items.
 * When item_override is set (not NaN), every item gets that fixed size.
 * When avail_main is INFINITY, returns INFINITY (natural-size pass).
 * ----------------------------------------------------------------------- */
static float
_LineShare(int count, float avail_main, float spacing, float item_override)
{
  if (!isnan(item_override))
    return item_override;
  if (count == 0 || isinf(avail_main))
    return INFINITY;
  return (avail_main - (float)(count - 1) * spacing) / (float)count;
}

/* -----------------------------------------------------------------------
 * _ComputeLines – non-greedy line-break algorithm.
 *
 * For each line the algorithm:
 *   1. Starts by attempting to fit all remaining children on one line.
 *   2. Computes each child's equal share of the available main-axis space.
 *   3. Reduces the candidate count by 1 whenever any child's MinMain
 *      exceeds its equal share, then retries.
 *   4. Measures each line's children with their allocated share.
 *
 * Special cases:
 *   • ItemWidth/ItemHeight set  → fixed share, all remaining fit (greedy).
 *   • avail_main == INFINITY    → all children on one line, share = INFINITY.
 * ----------------------------------------------------------------------- */
static int
_ComputeLines(lpObject_t      *children,
              int              n,
              enum Direction   direction,
              float            avail_main,
              float            cross_avail,
              float            spacing,
              float            item_override_main,
              float            item_override_cross,
              WrapLine        *out_lines,
              int              max_lines)
{
  int num_lines  = 0;
  int line_start = 0;

  while (line_start < n && num_lines < max_lines) {
    int remaining = n - line_start;

    /* --- Determine how many items fit on this line. --- */
    int count;
    if (!isnan(item_override_main) || isinf(avail_main)) {
      /* Fixed ItemWidth or unlimited space: put all remaining on one line. */
      count = remaining;
    } else {
      /* Non-greedy: try to fit all remaining; reduce if MinMain violated. */
      count = remaining;
      while (count > 1) {
        float share = _LineShare(count, avail_main, spacing, item_override_main);
        bool_t all_fit = TRUE;
        for (int i = line_start; i < line_start + count; i++) {
          if (_ChildMinMain(GetNode2D(children[i]), direction) > share) {
            all_fit = FALSE;
            break;
          }
        }
        if (all_fit) break;
        count--;
      }
    }

    /* --- Measure items on this line with their allocated share. --- */
    float share      = _LineShare(count, avail_main, spacing, item_override_main);
    float line_cross = 0.0f;

    for (int i = line_start; i < line_start + count; i++) {
      Size_t avail;
      if (direction == kDirectionHorizontal) {
        avail.width  = share;
        avail.height = isfinite(item_override_cross) ? item_override_cross : cross_avail;
      } else {
        avail.width  = isfinite(item_override_cross) ? item_override_cross : cross_avail;
        avail.height = share;
      }
      OBJ_SendMessageW(children[i], kEventMeasure, 0, &avail);
      line_cross = fmaxf(line_cross, _ChildCrossSize(GetNode2D(children[i]), direction));
    }

    out_lines[num_lines].start      = line_start;
    out_lines[num_lines].count      = count;
    out_lines[num_lines].cross_size = line_cross;
    out_lines[num_lines].share      = share;
    num_lines++;
    line_start += count;
  }

  return num_lines;
}

/* -----------------------------------------------------------------------
 * MeasureOverride
 * ----------------------------------------------------------------------- */
HANDLER(WrapPanel, MeasureOverride)
{
  lpObject_t children[MAX_WRAP_CHILDREN];
  int n = _CollectChildren(hObject, children, MAX_WRAP_CHILDREN);

  float item_main  = pWrapPanel->Direction == kDirectionHorizontal
                       ? pWrapPanel->ItemWidth  : pWrapPanel->ItemHeight;
  float item_cross = pWrapPanel->Direction == kDirectionHorizontal
                       ? pWrapPanel->ItemHeight : pWrapPanel->ItemWidth;

  float avail_main  = pWrapPanel->Direction == kDirectionHorizontal
                        ? pMeasureOverride->width  : pMeasureOverride->height;
  float cross_avail = pWrapPanel->Direction == kDirectionHorizontal
                        ? pMeasureOverride->height : pMeasureOverride->width;

  WrapLine lines[MAX_WRAP_LINES];
  int num_lines = _ComputeLines(children, n, pWrapPanel->Direction,
                                avail_main, cross_avail, pWrapPanel->Spacing,
                                item_main, item_cross, lines, MAX_WRAP_LINES);

  /* Accumulate desired size across lines. */
  float max_main    = 0.0f;
  float total_cross = 0.0f;

  for (int li = 0; li < num_lines; li++) {
    float line_main;
    if (isinf(lines[li].share)) {
      /* Natural-size pass: sum individual desired widths. */
      line_main = 0.0f;
      for (int ci = 0; ci < lines[li].count; ci++) {
        Node2DPtr sv = GetNode2D(children[lines[li].start + ci]);
        int axis = pWrapPanel->Direction == kDirectionHorizontal ? 0 : 1;
        if (ci > 0) line_main += pWrapPanel->Spacing;
        line_main += NODE2D_FRAME(sv, Size, axis).Desired + TOTAL_MARGIN(sv, axis);
      }
    } else {
      /* Equal-share pass: line width = count*share + (count-1)*spacing. */
      line_main = lines[li].share * (float)lines[li].count
                  + (float)(lines[li].count - 1) * pWrapPanel->Spacing;
    }
    max_main = fmaxf(max_main, line_main);
    if (li > 0) total_cross += pWrapPanel->Spacing;
    total_cross += lines[li].cross_size;
  }

  if (pWrapPanel->Direction == kDirectionHorizontal)
    return MAKEDWORD(max_main, total_cross);
  else
    return MAKEDWORD(total_cross, max_main);
}

/* -----------------------------------------------------------------------
 * ArrangeOverride
 * ----------------------------------------------------------------------- */
HANDLER(WrapPanel, ArrangeOverride)
{
  lpObject_t children[MAX_WRAP_CHILDREN];
  int n = _CollectChildren(hObject, children, MAX_WRAP_CHILDREN);

  float item_main  = pWrapPanel->Direction == kDirectionHorizontal
                       ? pWrapPanel->ItemWidth  : pWrapPanel->ItemHeight;
  float item_cross = pWrapPanel->Direction == kDirectionHorizontal
                       ? pWrapPanel->ItemHeight : pWrapPanel->ItemWidth;

  float avail_main  = pWrapPanel->Direction == kDirectionHorizontal
                        ? pArrangeOverride->width  : pArrangeOverride->height;
  float cross_avail = pWrapPanel->Direction == kDirectionHorizontal
                        ? pArrangeOverride->height : pArrangeOverride->width;

  WrapLine lines[MAX_WRAP_LINES];
  int num_lines = _ComputeLines(children, n, pWrapPanel->Direction,
                                avail_main, cross_avail, pWrapPanel->Spacing,
                                item_main, item_cross, lines, MAX_WRAP_LINES);

  /* Lay out lines along the cross axis. */
  float cross_pos = pWrapPanel->Direction == kDirectionHorizontal
                      ? pArrangeOverride->y : pArrangeOverride->x;

  for (int li = 0; li < num_lines; li++) {
    float line_cross = isfinite(item_cross) ? item_cross : lines[li].cross_size;
    float main_pos   = pWrapPanel->Direction == kDirectionHorizontal
                         ? pArrangeOverride->x : pArrangeOverride->y;
    float share = lines[li].share;

    for (int ci = 0; ci < lines[li].count; ci++) {
      int idx = lines[li].start + ci;

      if (pWrapPanel->Direction == kDirectionHorizontal) {
        OBJ_SendMessageW(children[idx], kEventArrange, 0, &(struct rect) {
          .x      = main_pos,
          .y      = cross_pos,
          .width  = share,
          .height = line_cross,
        });
      } else {
        OBJ_SendMessageW(children[idx], kEventArrange, 0, &(struct rect) {
          .x      = cross_pos,
          .y      = main_pos,
          .width  = line_cross,
          .height = share,
        });
      }
      main_pos += share + pWrapPanel->Spacing;
    }

    cross_pos += line_cross + pWrapPanel->Spacing;
  }

  return MAKEDWORD(pArrangeOverride->width, pArrangeOverride->height);
}

