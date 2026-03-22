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
 */

#define MAX_WRAP_CHILDREN 256
#define MAX_WRAP_LINES    128

/* One "line" (row for Horizontal, column for Vertical). */
typedef struct {
  int   start;      /* index of first child in this line */
  int   count;      /* number of children in this line   */
  float cross_size; /* max child cross-axis size in this line */
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
 * _ChildMainSize – effective main-axis size of a child after measuring.
 * If ItemWidth/ItemHeight override is active it takes precedence.
 * ----------------------------------------------------------------------- */
static float
_ChildMainSize(Node2DPtr subview,
               enum Direction direction,
               float item_override)
{
  if (!isnan(item_override))
    return item_override;
  if (direction == kDirectionHorizontal)
    return NODE2D_FRAME(subview, Size, 0).Desired + TOTAL_MARGIN(subview, 0);
  else
    return NODE2D_FRAME(subview, Size, 1).Desired + TOTAL_MARGIN(subview, 1);
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
 * _ComputeLines – greedy line-break algorithm.
 *
 * Populates out_lines[] and returns the number of lines.
 * When avail_main is INFINITY no wrapping occurs (all children in one line).
 * ----------------------------------------------------------------------- */
static int
_ComputeLines(lpObject_t      *children,
              int              n,
              enum Direction   direction,
              float            avail_main,
              float            spacing,
              float            item_override_main,
              WrapLine        *out_lines,
              int              max_lines)
{
  int   num_lines   = 0;
  int   line_start  = 0;
  int   line_count  = 0;
  float line_main   = 0;
  float line_cross  = 0;
  bool_t first_in_line = TRUE;

  for (int i = 0; i < n; i++) {
    Node2DPtr subview   = GetNode2D(children[i]);
    float     child_m   = _ChildMainSize(subview, direction, item_override_main);
    float     child_c   = _ChildCrossSize(subview, direction);

    /* Does this child overflow the current line? */
    bool_t wraps = !first_in_line &&
                   !isinf(avail_main) &&
                   (line_main + spacing + child_m > avail_main);

    if (wraps) {
      if (num_lines < max_lines) {
        out_lines[num_lines].start      = line_start;
        out_lines[num_lines].count      = line_count;
        out_lines[num_lines].cross_size = line_cross;
        num_lines++;
      }
      line_start    = i;
      line_count    = 0;
      line_main     = 0;
      line_cross    = 0;
      first_in_line = TRUE;
    }

    if (!first_in_line)
      line_main += spacing;
    line_main   += child_m;
    line_cross   = fmax(line_cross, child_c);
    line_count++;
    first_in_line = FALSE;
  }

  /* Flush the last (possibly only) line. */
  if (line_count > 0 && num_lines < max_lines) {
    out_lines[num_lines].start      = line_start;
    out_lines[num_lines].count      = line_count;
    out_lines[num_lines].cross_size = line_cross;
    num_lines++;
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

  /* Measure all children. */
  for (int i = 0; i < n; i++) {
    Size_t avail;
    if (pWrapPanel->Direction == kDirectionHorizontal) {
      avail.width  = !isnan(item_main)  ? item_main  : INFINITY;
      avail.height = !isnan(item_cross) ? item_cross : pMeasureOverride->height;
    } else {
      avail.width  = !isnan(item_cross) ? item_cross : pMeasureOverride->width;
      avail.height = !isnan(item_main)  ? item_main  : INFINITY;
    }
    OBJ_SendMessageW(children[i], kEventMeasure, 0, &avail);
  }

  float avail_main = pWrapPanel->Direction == kDirectionHorizontal
                       ? pMeasureOverride->width
                       : pMeasureOverride->height;

  WrapLine lines[MAX_WRAP_LINES];
  int num_lines = _ComputeLines(children, n, pWrapPanel->Direction,
                                avail_main, pWrapPanel->Spacing,
                                item_main, lines, MAX_WRAP_LINES);

  /* Accumulate desired size across lines. */
  float max_main    = 0;
  float total_cross = 0;

  for (int li = 0; li < num_lines; li++) {
    float line_main = 0;
    for (int ci = 0; ci < lines[li].count; ci++) {
      Node2DPtr sv = GetNode2D(children[lines[li].start + ci]);
      if (ci > 0) line_main += pWrapPanel->Spacing;
      line_main += _ChildMainSize(sv, pWrapPanel->Direction, item_main);
    }
    max_main = fmax(max_main, line_main);
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

  /* Re-measure children with the final arrange constraints. */
  for (int i = 0; i < n; i++) {
    Size_t avail;
    if (pWrapPanel->Direction == kDirectionHorizontal) {
      avail.width  = !isnan(item_main)  ? item_main  : INFINITY;
      avail.height = !isnan(item_cross) ? item_cross : pArrangeOverride->height;
    } else {
      avail.width  = !isnan(item_cross) ? item_cross : pArrangeOverride->width;
      avail.height = !isnan(item_main)  ? item_main  : INFINITY;
    }
    OBJ_SendMessageW(children[i], kEventMeasure, 0, &avail);
  }

  float avail_main = pWrapPanel->Direction == kDirectionHorizontal
                       ? pArrangeOverride->width
                       : pArrangeOverride->height;

  WrapLine lines[MAX_WRAP_LINES];
  int num_lines = _ComputeLines(children, n, pWrapPanel->Direction,
                                avail_main, pWrapPanel->Spacing,
                                item_main, lines, MAX_WRAP_LINES);

  /* Lay out lines along the cross axis. */
  float cross_pos = pWrapPanel->Direction == kDirectionHorizontal
                      ? pArrangeOverride->y
                      : pArrangeOverride->x;

  for (int li = 0; li < num_lines; li++) {
    float line_cross = !isnan(item_cross) ? item_cross : lines[li].cross_size;
    float main_pos   = pWrapPanel->Direction == kDirectionHorizontal
                         ? pArrangeOverride->x
                         : pArrangeOverride->y;

    for (int ci = 0; ci < lines[li].count; ci++) {
      int       idx     = lines[li].start + ci;
      Node2DPtr subview = GetNode2D(children[idx]);
      float     child_m = _ChildMainSize(subview, pWrapPanel->Direction, item_main);

      if (pWrapPanel->Direction == kDirectionHorizontal) {
        OBJ_SendMessageW(children[idx], kEventArrange, 0, &(struct rect) {
          .x      = main_pos,
          .y      = cross_pos,
          .width  = child_m,
          .height = line_cross,
        });
      } else {
        OBJ_SendMessageW(children[idx], kEventArrange, 0, &(struct rect) {
          .x      = cross_pos,
          .y      = main_pos,
          .width  = line_cross,
          .height = child_m,
        });
      }
      main_pos += child_m + pWrapPanel->Spacing;
    }

    cross_pos += line_cross + pWrapPanel->Spacing;
  }

  return MAKEDWORD(pArrangeOverride->width, pArrangeOverride->height);
}
