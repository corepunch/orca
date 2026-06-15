#include <ctype.h> // isspace()
#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <UIKit/UIKit.h>


#define MAX_COLUMNS 16

enum column_type
{
  column_type_invalid,
  column_type_auto,
  column_type_pixel,
  column_type_fr,
};

struct column
{
  enum column_type type;
  float width;
  float position;
};

struct columns
{
  int count;
  struct column columns[MAX_COLUMNS];
};

struct grid_layout
{
  struct columns axes[2];
};

typedef struct column* PCOLUMN;
typedef struct columns* PCOLUMNS;
typedef struct Grid* PGRIDVIEW;
typedef struct grid_layout* PGRIDLAYOUT;

static struct column
column_parse(lpcString_t a, lpcString_t *end)
{
  *end = a;
  if (!strncmp(a, "auto", 4)) {
    *end = a + 4;
    return (struct column){ column_type_auto, 1, 0 };
  } else if (!isdigit((unsigned char)*a)) {
    return (struct column){ column_type_invalid, 0 };
  } else {
    LPSTR tail = NULL;
    float value = strtof(a, &tail);
    if (!strncmp(tail, "fr", 2)) {
      *end = tail + 2;
      return (struct column){ column_type_fr, value > 0 ? value : 1, 0 };
    } else if (!strncmp(tail, "px", 2)) {
      *end = tail + 2;
      return (struct column){ column_type_pixel, value };
    } else {
      return (struct column){ column_type_invalid, 0 };
    }
  }
}

static void
columns_parse(lpcString_t a, struct columns* out)
{
  out->count = 0;
  while (*a) {
    while (isspace((unsigned char)*a)) a++;
    if (!*a)
      break;
    assert(out->count < MAX_COLUMNS);
    out->columns[out->count++] = column_parse(a, &a);
    while (*a && !isspace((unsigned char)*a)) a++;
  }
}

static PCOLUMNS
columns_at_axis(PGRIDLAYOUT layout, PGRIDVIEW pGrid, enum Direction axis, bool_t parse)
{
  switch (axis) {
    case kDirectionHorizontal:
      if (parse)
        layout->axes[axis] = (struct columns){0};
      if (parse && pGrid->Columns)
        columns_parse(pGrid->Columns, &layout->axes[axis]);
      return &layout->axes[axis];
    case kDirectionVertical:
      if (parse)
        layout->axes[axis] = (struct columns){0};
      if (parse && pGrid->Rows)
        columns_parse(pGrid->Rows, &layout->axes[axis]);
      return &layout->axes[axis];
    default:
      return NULL;
  }
}

static struct column*
column_at_cellindex(PGRIDLAYOUT layout, PGRIDVIEW pGrid, enum Direction axis, int cellindex)
{
  PCOLUMNS axis_columns = columns_at_axis(layout, pGrid, axis, FALSE);
  PCOLUMNS dir_columns = columns_at_axis(layout, pGrid, pGrid->Direction, FALSE);
  int dir_count = dir_columns ? dir_columns->count : 0;
  if (!axis_columns || axis_columns->count == 0) {
    return NULL;
  } else if (axis == pGrid->Direction) {
    return dir_count ? &dir_columns->columns[cellindex % dir_count] : NULL;
  } else {
    return &axis_columns->columns[(cellindex / MAX(1, dir_count)) %
                                  axis_columns->count];
  }
}

static bool_t
_column_is_flexible(struct column const *col)
{
  return col->type == column_type_auto || col->type == column_type_fr;
}

/*
 * When the primary axis (Columns for H-grids, Rows for V-grids) is defined
 * but the secondary axis has no explicit definition, auto-generate one "auto"
 * slot per implicit row (or column) so that children wrap and stack correctly
 * instead of all landing at position 0 on the secondary axis.
 */
static void
_EnsureImplicitSecondaryAxis(PGRIDLAYOUT layout, PGRIDVIEW pGrid, struct Object *hObject)
{
  enum Direction secondary = (pGrid->Direction == kDirectionHorizontal)
    ? kDirectionVertical : kDirectionHorizontal;
  bool_t secondary_unset = (secondary == kDirectionVertical)
    ? !pGrid->Rows : !pGrid->Columns;
  if (!secondary_unset)
    return;
  PCOLUMNS primary_cols   = columns_at_axis(layout, pGrid, pGrid->Direction, FALSE);
  PCOLUMNS secondary_cols = columns_at_axis(layout, pGrid, secondary, FALSE);
  if (!primary_cols || !secondary_cols || primary_cols->count == 0)
    return;
  int child_count = 0;
  FOR_EACH_LAYOUTABLE(hChild, hObject) child_count++;
  if (child_count == 0)
    return;
  int n = (child_count + (int)primary_cols->count - 1) / (int)primary_cols->count;
  secondary_cols->count = n < MAX_COLUMNS ? n : MAX_COLUMNS;
  FOR_LOOP(i, secondary_cols->count)
    secondary_cols->columns[i] = (struct column){ column_type_auto, 1, 0 };
}

static void
_CalculateAutos(float spacing, float avl, PCOLUMNS columns)
{
  float total_weight = 0;
  if (columns->count > 0) {
    avl -= spacing * (columns->count - 1);
  }
  FOR_LOOP(i, columns->count)
  {
    if (_column_is_flexible(&columns->columns[i])) {
      total_weight += columns->columns[i].width; /* width holds the fr weight */
    } else {
      avl -= columns->columns[i].width;
    }
  }
  float cursor = 0;
  FOR_LOOP(i, columns->count)
  {
    if (_column_is_flexible(&columns->columns[i])) {
      if (!isinf(avl) && total_weight > 0) {
        /* distribute proportionally by weight (fr or auto=1fr) */
        columns->columns[i].width = avl * columns->columns[i].width / total_weight;
      } else {
        /* unconstrained: start at 0; will be content-sized in a first pass */
        columns->columns[i].width = 0;
      }
    }
    columns->columns[i].position = cursor;
    cursor += columns->columns[i].width + spacing;
  }
}

// Grid_MeasureOverride
HANDLER(Grid, Node2D, MeasureOverride)
{
  uint32_t cellindex = 0;
  struct grid_layout layout = {0};
  Node2D_MeasureOverrideMsg_t size = *pMeasureOverride;
  struct Size desired = {0};

  PCOLUMNS hcols = columns_at_axis(&layout, pGrid, kDirectionHorizontal, TRUE);
  PCOLUMNS vcols = columns_at_axis(&layout, pGrid, kDirectionVertical, TRUE);

  _EnsureImplicitSecondaryAxis(&layout, pGrid, hObject);

  _CalculateAutos(pGrid->Spacing, size.Width, hcols);
  _CalculateAutos(pGrid->Spacing, size.Height, vcols);

  /*
   * When the available height is unconstrained (INFINITY) and we have
   * flexible (auto/fr) rows, do a content-sizing first pass: measure each
   * child and expand the owning row to fit, then recompute row positions.
   * Without this, row positions stay at 0 causing rows beyond the first to
   * overlap or produce incorrect (infinite) accumulated heights.
   */
  if (isinf(size.Height) && vcols->count > 0) {
    cellindex = 0;
    FOR_EACH_LAYOUTABLE(hChild, hObject) {
      struct column* w = column_at_cellindex(&layout, pGrid, kDirectionHorizontal, cellindex);
      struct column* h = column_at_cellindex(&layout, pGrid, kDirectionVertical, cellindex);
      if (h && _column_is_flexible(h)) {
        LRESULT s = _SendMessage(hChild, Node2D, Measure,
          .Width  = (w ? w->width : size.Width),
          .Height = INFINITY,
        );
        float child_h = (float)HIWORD(s);
        if (h->width < child_h) {
          h->width = child_h;
        }
      }
      cellindex++;
    }
    /* Recompute vertical positions with the content-sized row heights. */
    float cursor = 0;
    FOR_LOOP(i, vcols->count) {
      vcols->columns[i].position = cursor;
      cursor += vcols->columns[i].width + pGrid->Spacing;
    }
  }

  /* Symmetric first pass for unconstrained width with flexible columns. */
  if (isinf(size.Width) && hcols->count > 0) {
    cellindex = 0;
    FOR_EACH_LAYOUTABLE(hChild, hObject) {
      struct column* w = column_at_cellindex(&layout, pGrid, kDirectionHorizontal, cellindex);
      struct column* h = column_at_cellindex(&layout, pGrid, kDirectionVertical, cellindex);
      if (w && _column_is_flexible(w)) {
        LRESULT s = _SendMessage(hChild, Node2D, Measure,
          .Width  = INFINITY,
          .Height = (h ? h->width : size.Height),
        );
        float child_w = (float)LOWORD(s);
        if (w->width < child_w) {
          w->width = child_w;
        }
      }
      cellindex++;
    }
    float cursor = 0;
    FOR_LOOP(i, hcols->count) {
      hcols->columns[i].position = cursor;
      cursor += hcols->columns[i].width + pGrid->Spacing;
    }
  }

  cellindex = 0;
  FOR_EACH_LAYOUTABLE(hChild, hObject)
  {
    struct column* w = column_at_cellindex(&layout, pGrid, kDirectionHorizontal, cellindex);
    struct column* h = column_at_cellindex(&layout, pGrid, kDirectionVertical, cellindex);

    LRESULT s = _SendMessage(hChild, Node2D, Measure,
      .Width  = (w ? w->width : size.Width),
      .Height = (h ? h->width : size.Height),
    );

    if(LOWORD(s)<0xffff)desired.width = fmax((w?w->position:0) + LOWORD(s), desired.width);
    if(HIWORD(s)<0xffff)desired.height = fmax((h?h->position:0) + HIWORD(s), desired.height);
    cellindex++;
  }

  return MAKEDWORD(desired.width?desired.width:pMeasureOverride->Width,
                   desired.height?desired.height:pMeasureOverride->Height);
}

// Grid_ArrangeOverride
HANDLER(Grid, Node2D, ArrangeOverride)
{
  uint32_t cellindex = 0;
  struct grid_layout layout = {0};
  PCOLUMNS hcols = columns_at_axis(&layout, pGrid, kDirectionHorizontal, TRUE);
  PCOLUMNS vcols = columns_at_axis(&layout, pGrid, kDirectionVertical, TRUE);
  _EnsureImplicitSecondaryAxis(&layout, pGrid, hObject);
  _CalculateAutos(pGrid->Spacing, pArrangeOverride->Width, hcols);
  _CalculateAutos(pGrid->Spacing, pArrangeOverride->Height, vcols);
  /* Implicit auto tracks (generated by _EnsureImplicitSecondaryAxis because no
   * explicit Rows/Columns string was given) must size to their content rather
   * than taking equal shares of the available space like fr units.  Reset them
   * to 0 so the content-sizing pass below measures them correctly.  Explicit
   * auto tracks in a user-supplied Rows/Columns string keep their
   * _CalculateAutos-distributed width and only grow when content exceeds it. */
  if (pGrid->Rows == NULL)
    FOR_LOOP(_i, vcols->count)
      if (vcols->columns[_i].type == column_type_auto) vcols->columns[_i].width = 0;
  if (pGrid->Columns == NULL)
    FOR_LOOP(_i, hcols->count)
      if (hcols->columns[_i].type == column_type_auto) hcols->columns[_i].width = 0;
  if ((pArrangeOverride->Height == 0 || pGrid->Rows == NULL) && vcols->count > 0) {
    cellindex = 0;
    FOR_EACH_LAYOUTABLE(hChild, hObject) {
      struct column* w = column_at_cellindex(&layout, pGrid, kDirectionHorizontal, cellindex);
      struct column* h = column_at_cellindex(&layout, pGrid, kDirectionVertical, cellindex);
      if (h && _column_is_flexible(h)) {
        LRESULT s = _SendMessage(hChild, Node2D, Measure,
          .Width  = (w ? w->width : pArrangeOverride->Width),
          .Height = INFINITY,
        );
        float child_h = (float)HIWORD(s);
        if (h->width < child_h) {
          h->width = child_h;
        }
      }
      cellindex++;
    }
    float cursor = 0;
    FOR_LOOP(i, vcols->count) {
      vcols->columns[i].position = cursor;
      cursor += vcols->columns[i].width + pGrid->Spacing;
    }
    cellindex = 0;
  }
  if ((pArrangeOverride->Width == 0 || pGrid->Columns == NULL) && hcols->count > 0) {
    cellindex = 0;
    FOR_EACH_LAYOUTABLE(hChild, hObject) {
      struct column* w = column_at_cellindex(&layout, pGrid, kDirectionHorizontal, cellindex);
      struct column* h = column_at_cellindex(&layout, pGrid, kDirectionVertical, cellindex);
      if (w && _column_is_flexible(w)) {
        LRESULT s = _SendMessage(hChild, Node2D, Measure,
          .Width  = INFINITY,
          .Height = (h ? h->width : pArrangeOverride->Height),
        );
        float child_w = (float)LOWORD(s);
        if (w->width < child_w) {
          w->width = child_w;
        }
      }
      cellindex++;
    }
    float cursor = 0;
    FOR_LOOP(i, hcols->count) {
      hcols->columns[i].position = cursor;
      cursor += hcols->columns[i].width + pGrid->Spacing;
    }
    cellindex = 0;
  }
  float actual_height = pArrangeOverride->Height;
  float actual_width  = pArrangeOverride->Width;
  FOR_EACH_LAYOUTABLE(hChild, hObject)
  {
    struct Node2D *subview = GetNode2D(hChild);
    struct column* w = column_at_cellindex(&layout, pGrid, kDirectionHorizontal, cellindex);
    struct column* h = column_at_cellindex(&layout, pGrid, kDirectionVertical, cellindex);
    float y = pArrangeOverride->Y + (h ? h->position : 0);
    float height = h ? h->width : pArrangeOverride->Height;
    if (h && pGrid->Rows == NULL && vcols->count == 1 && pArrangeOverride->Height > height)
      height = pArrangeOverride->Height;
    float occupied = NODE2D_FRAME(subview, Size, kDirectionVertical).Desired + TOTAL_MARGIN(subview, kDirectionVertical);
    switch (pGrid->AlignItems) {
      case kAlignItemsEnd:
        y += height - occupied; height = occupied;
        break;
      case kAlignItemsCenter:
        y += (height - occupied) * 0.5f; height = occupied;
        break;
      case kAlignItemsStart:
      case kAlignItemsBaseline:
        height = occupied;
        break;
      case kAlignItemsStretch:
        break;
    }
    _SendMessage(hChild, Node2D, Arrange,
      .X = pArrangeOverride->X + (w ? w->position : 0),
      .Y = y,
      .Width = (w ? w->width : pArrangeOverride->Width),
      .Height = height,
    );
    if (h) actual_height = fmax(actual_height, h->position + h->width);
    if (w) actual_width  = fmax(actual_width,  w->position + w->width);
    cellindex++;
  }
  return MAKEDWORD(actual_width, actual_height);
}
