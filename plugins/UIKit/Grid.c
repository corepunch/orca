#include <ctype.h> // isspace()
#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <UIKit/UIKit.h>


#define MAX_COLUMNS 16

// Track sizing follows WPF's RowDefinition/ColumnDefinition: a pixel track
// keeps its size ("48px" or a bare "48"), an Auto track ("auto") fits its
// largest child, and a star track ("1fr", "*", "2*") takes a weighted share
// of the space left after pixel and Auto tracks. With unbounded space star
// tracks fit their content too.
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
  float width;    /* resolved size; the declared size for pixel tracks */
  float position;
  float weight;   /* star weight */
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

static bool_t
column_is_auto(lpcString_t a)
{
  for (lpcString_t k = "auto"; *k; k++, a++)
    if (tolower((unsigned char)*a) != *k) return FALSE;
  return TRUE;
}

static struct column
column_parse(lpcString_t a, lpcString_t *end)
{
  *end = a;
  if (column_is_auto(a)) {
    *end = a + 4;
    return (struct column){ column_type_auto };
  } else if (*a == '*') {
    *end = a + 1;
    return (struct column){ column_type_fr, .weight = 1 };
  } else if (!isdigit((unsigned char)*a) && *a != '.') {
    return (struct column){ column_type_invalid };
  }
  LPSTR tail = NULL;
  float value = strtof(a, &tail);
  if (!strncmp(tail, "fr", 2) || *tail == '*') {
    *end = tail + (*tail == '*' ? 1 : 2);
    return (struct column){ column_type_fr, .weight = value > 0 ? value : 1 };
  } else if (!strncmp(tail, "px", 2) || !*tail || isspace((unsigned char)*tail)) {
    *end = tail + (*tail == 'p' ? 2 : 0);
    return (struct column){ column_type_pixel, .width = value };
  }
  return (struct column){ column_type_invalid };
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
    secondary_cols->columns[i] = (struct column){ column_type_auto };
}

static void
_PrepareLayout(PGRIDLAYOUT layout, PGRIDVIEW pGrid, struct Object *hObject)
{
  columns_at_axis(layout, pGrid, kDirectionHorizontal, TRUE);
  columns_at_axis(layout, pGrid, kDirectionVertical, TRUE);
  _EnsureImplicitSecondaryAxis(layout, pGrid, hObject);
}

/* Resolves one axis. Columns are resolved before rows, so Auto columns are
 * measured against pixel rows only, while rows see the final column widths. */
static void
_ResolveTracks(PGRIDLAYOUT layout, PGRIDVIEW pGrid, struct Object *hObject,
               enum Direction axis, float available)
{
  PCOLUMNS cols = &layout->axes[axis];
  enum Direction cross = axis == kDirectionHorizontal ? kDirectionVertical : kDirectionHorizontal;
  bool_t bounded = isfinite(available) && available > 0;
  float weights = 0, used = cols->count > 1 ? pGrid->Spacing * (cols->count - 1) : 0;
  FOR_LOOP(i, cols->count) {
    struct column *c = &cols->columns[i];
    if (c->type != column_type_pixel) c->width = 0;
  }

  uint32_t cellindex = 0;
  FOR_EACH_LAYOUTABLE(hChild, hObject) {
    struct column *c = column_at_cellindex(layout, pGrid, axis, cellindex);
    struct column *o = column_at_cellindex(layout, pGrid, cross, cellindex);
    cellindex++;
    if (!c || !(c->type == column_type_auto || (c->type == column_type_fr && !bounded))) continue;
    float other = o && (axis == kDirectionVertical || o->type == column_type_pixel) ? o->width : INFINITY;
    LRESULT size = axis == kDirectionHorizontal
      ? _SendMessage(hChild, Node2D, Measure, .Width = INFINITY, .Height = other)
      : _SendMessage(hChild, Node2D, Measure, .Width = other, .Height = INFINITY);
    uint16_t extent = axis == kDirectionHorizontal ? LOWORD(size) : HIWORD(size);
    if (extent < 0xffff) c->width = fmaxf(c->width, extent);
  }

  FOR_LOOP(i, cols->count) {
    struct column *c = &cols->columns[i];
    if (c->type == column_type_fr && bounded) weights += c->weight;
    else used += c->width;
  }
  float cursor = 0;
  FOR_LOOP(i, cols->count) {
    struct column *c = &cols->columns[i];
    if (c->type == column_type_fr && bounded && weights > 0)
      c->width = fmaxf(available - used, 0) * c->weight / weights;
    c->position = cursor;
    cursor += c->width + pGrid->Spacing;
  }
}

static float
_TracksExtent(PCOLUMNS cols)
{
  if (!cols->count) return 0;
  struct column *last = &cols->columns[cols->count - 1];
  return last->position + last->width;
}

HANDLER(Grid, Node2D, MeasureOverride)
{
  uint32_t cellindex = 0;
  struct grid_layout layout = {0};
  struct Size desired = {0};
  _PrepareLayout(&layout, pGrid, hObject);
  _ResolveTracks(&layout, pGrid, hObject, kDirectionHorizontal, pMeasureOverride->Width);
  _ResolveTracks(&layout, pGrid, hObject, kDirectionVertical, pMeasureOverride->Height);

  FOR_EACH_LAYOUTABLE(hChild, hObject)
  {
    struct column* w = column_at_cellindex(&layout, pGrid, kDirectionHorizontal, cellindex);
    struct column* h = column_at_cellindex(&layout, pGrid, kDirectionVertical, cellindex);
    LRESULT s = _SendMessage(hChild, Node2D, Measure,
      .Width  = (w ? w->width : pMeasureOverride->Width),
      .Height = (h ? h->width : pMeasureOverride->Height),
    );
    if (!w && LOWORD(s) < 0xffff) desired.width = fmax(LOWORD(s), desired.width);
    if (!h && HIWORD(s) < 0xffff) desired.height = fmax(HIWORD(s), desired.height);
    cellindex++;
  }
  /* A grid with tracks is as large as its tracks, as in WPF. */
  if (layout.axes[kDirectionHorizontal].count) desired.width = _TracksExtent(&layout.axes[kDirectionHorizontal]);
  if (layout.axes[kDirectionVertical].count) desired.height = _TracksExtent(&layout.axes[kDirectionVertical]);

  return MAKEDWORD(desired.width?desired.width:pMeasureOverride->Width,
                   desired.height?desired.height:pMeasureOverride->Height);
}

HANDLER(Grid, Node2D, ArrangeOverride)
{
  uint32_t cellindex = 0;
  struct grid_layout layout = {0};
  _PrepareLayout(&layout, pGrid, hObject);
  _ResolveTracks(&layout, pGrid, hObject, kDirectionHorizontal, pArrangeOverride->Width);
  _ResolveTracks(&layout, pGrid, hObject, kDirectionVertical, pArrangeOverride->Height);
  PCOLUMNS vcols = &layout.axes[kDirectionVertical];
  float actual_height = pArrangeOverride->Height;
  float actual_width  = pArrangeOverride->Width;
  FOR_EACH_LAYOUTABLE(hChild, hObject)
  {
    struct Node2D *subview = GetNode2D(hChild);
    struct column* w = column_at_cellindex(&layout, pGrid, kDirectionHorizontal, cellindex);
    struct column* h = column_at_cellindex(&layout, pGrid, kDirectionVertical, cellindex);
    float y = pArrangeOverride->Y + (h ? h->position : 0);
    float height = h ? h->width : pArrangeOverride->Height;
    /* A single generated row fills the grid, like WPF's default star row. */
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
