#include <ctype.h> // isspace()
#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <plugins/UIKit/UIKit.h>


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

typedef struct column* PCOLUMN;
typedef struct columns* PCOLUMNS;
typedef struct Grid* PGRIDVIEW;

static struct column
column_parse(lpcString_t a)
{
  if (!strncmp(a, "auto", 4)) {
    return (struct column){ column_type_auto, 1, 0 };
  } else if (!isdigit(*a)) {
    return (struct column){ column_type_invalid, 0 };
  } else {
    float value = strtof(a, (LPSTR*)&a);
    if (!strncmp(a, "fr", 2)) {
      return (struct column){ column_type_fr, value > 0 ? value : 1, 0 };
    } else if (!strncmp(a, "px", 2)) {
      return (struct column){ column_type_pixel, value };
    } else {
      return (struct column){ column_type_invalid, 0 };
    }
  }
}

static void
columns_parse(lpcString_t start, struct columns* out)
{
  static char buffer[256];
  strncpy(buffer, start, sizeof(buffer));
  out->count = 0;
  for (lpcString_t tok = strtok(buffer, " "); tok; tok = strtok(NULL, " ")) {
    assert(out->count < MAX_COLUMNS);
    out->columns[out->count++] = column_parse(tok);
  }
}

static PCOLUMNS
columns_at_axis(PGRIDVIEW pGrid, enum Direction axis, bool_t parse)
{
  struct columns* columns = (struct columns*)pGrid->_buffer;
  switch (axis) {
    case kDirectionHorizontal:
      if (parse && pGrid->Columns)
        columns_parse(pGrid->Columns, &columns[axis]);
      return &columns[axis];
    case kDirectionVertical:
      if (parse && pGrid->Rows)
        columns_parse(pGrid->Rows, &columns[axis]);
      return &columns[axis];
    default:
      return NULL;
  }
}

static struct column*
column_at_cellindex(PGRIDVIEW pGrid, enum Direction axis, int cellindex)
{
  PCOLUMNS axis_columns = columns_at_axis(pGrid, axis, FALSE);
  PCOLUMNS dir_columns = columns_at_axis(pGrid, pGrid->Direction, FALSE);
  if (axis_columns->count == 0) {
    return NULL;
  } else if (axis == pGrid->Direction) {
    return &dir_columns->columns[cellindex % dir_columns->count];
  } else {
    return &axis_columns->columns[(cellindex / MAX(1, dir_columns->count)) %
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
_EnsureImplicitSecondaryAxis(PGRIDVIEW pGrid, lpObject_t hObject)
{
  enum Direction secondary = (pGrid->Direction == kDirectionHorizontal)
    ? kDirectionVertical : kDirectionHorizontal;
  bool_t secondary_unset = (secondary == kDirectionVertical)
    ? !pGrid->Rows : !pGrid->Columns;
  if (!secondary_unset)
    return;
  PCOLUMNS primary_cols   = columns_at_axis(pGrid, pGrid->Direction, FALSE);
  PCOLUMNS secondary_cols = columns_at_axis(pGrid, secondary, FALSE);
  if (primary_cols->count == 0)
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

HANDLER(Grid, MeasureOverride)
{
  uint32_t cellindex = 0;
  Size_t size = *pMeasureOverride;
  Size_t desired = {0};

  PCOLUMNS hcols = columns_at_axis(pGrid, 0, TRUE);
  PCOLUMNS vcols = columns_at_axis(pGrid, 1, TRUE);

  _EnsureImplicitSecondaryAxis(pGrid, hObject);

  _CalculateAutos(pGrid->Spacing, size.width, hcols);
  _CalculateAutos(pGrid->Spacing, size.height, vcols);
  
  /*
   * When the available height is unconstrained (INFINITY) and we have
   * flexible (auto/fr) rows, do a content-sizing first pass: measure each
   * child and expand the owning row to fit, then recompute row positions.
   * Without this, row positions stay at 0 causing rows beyond the first to
   * overlap or produce incorrect (infinite) accumulated heights.
   */
  if (isinf(size.height) && vcols->count > 0) {
    cellindex = 0;
    FOR_EACH_LAYOUTABLE(hChild, hObject) {
      struct column* w = column_at_cellindex(pGrid, kDirectionHorizontal, cellindex);
      struct column* h = column_at_cellindex(pGrid, kDirectionVertical, cellindex);
      if (h && _column_is_flexible(h)) {
        LRESULT s = _SendMessage(hChild, Measure,
          .Width  = (w ? w->width : size.width),
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
  if (isinf(size.width) && hcols->count > 0) {
    cellindex = 0;
    FOR_EACH_LAYOUTABLE(hChild, hObject) {
      struct column* w = column_at_cellindex(pGrid, kDirectionHorizontal, cellindex);
      struct column* h = column_at_cellindex(pGrid, kDirectionVertical, cellindex);
      if (w && _column_is_flexible(w)) {
        LRESULT s = _SendMessage(hChild, Measure,
          .Width  = INFINITY,
          .Height = (h ? h->width : size.height),
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
    struct column* w = column_at_cellindex(pGrid, kDirectionHorizontal, cellindex);
    struct column* h = column_at_cellindex(pGrid, kDirectionVertical, cellindex);

    LRESULT s = _SendMessage(hChild, Measure,
      .Width  = (w ? w->width : size.width),
      .Height = (h ? h->width : size.height),
    );
    
    if(LOWORD(s)<0xffff)desired.width = fmax((w?w->position:0) + LOWORD(s), desired.width);
    if(HIWORD(s)<0xffff)desired.height = fmax((h?h->position:0) + HIWORD(s), desired.height);
    cellindex++;
  }
  
  return MAKEDWORD(desired.width?desired.width:pMeasureOverride->Width,
                   desired.height?desired.height:pMeasureOverride->Height);
}

HANDLER(Grid, ArrangeOverride)
{
  uint32_t cellindex = 0;
  PCOLUMNS hcols = columns_at_axis(pGrid, 0, TRUE);
  PCOLUMNS vcols = columns_at_axis(pGrid, 1, TRUE);
  _EnsureImplicitSecondaryAxis(pGrid, hObject);
  _CalculateAutos(pGrid->Spacing, pArrangeOverride->Width, hcols);
  _CalculateAutos(pGrid->Spacing, pArrangeOverride->Height, vcols);
  /*
   * When the arranged height (or width) is 0 and flexible (auto/fr) rows (or
   * columns) exist, their widths are all 0 after _CalculateAutos. Run the
   * same content-sizing pass used in MeasureOverride so each flexible track
   * expands to fit its children, then recompute positions. This mirrors the
   * isinf(height/width) path in MeasureOverride and prevents children from
   * being arranged with 0 size when the grid has not been given any space.
   */
  if (pArrangeOverride->height == 0 && vcols->count > 0) {
    cellindex = 0;
    FOR_EACH_LAYOUTABLE(hChild, hObject) {
      struct column* w = column_at_cellindex(pGrid, kDirectionHorizontal, cellindex);
      struct column* h = column_at_cellindex(pGrid, kDirectionVertical, cellindex);
      if (h && _column_is_flexible(h)) {
        LRESULT s = _SendMessage(hChild, Measure,
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
  if (pArrangeOverride->Width == 0 && hcols->count > 0) {
    cellindex = 0;
    FOR_EACH_LAYOUTABLE(hChild, hObject) {
      struct column* w = column_at_cellindex(pGrid, kDirectionHorizontal, cellindex);
      struct column* h = column_at_cellindex(pGrid, kDirectionVertical, cellindex);
      if (w && _column_is_flexible(w)) {
        LRESULT s = _SendMessage(hChild, Measure,
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
    struct column* w = column_at_cellindex(pGrid, kDirectionHorizontal, cellindex);
    struct column* h = column_at_cellindex(pGrid, kDirectionVertical, cellindex);
    _SendMessage(hChild, Arrange,
      .X = pArrangeOverride->X + (w ? w->position : 0),
      .Y = pArrangeOverride->Y + (h ? h->position : 0),
      .Width = (w ? w->width : pArrangeOverride->Width),
      .Height = (h ? h->width : pArrangeOverride->Height),
    );
    if (h) actual_height = fmax(actual_height, h->position + h->width);
    if (w) actual_width  = fmax(actual_width,  w->position + w->width);
    cellindex++;
  }
  return MAKEDWORD(actual_width, actual_height);
}
