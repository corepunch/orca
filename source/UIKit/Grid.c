#include <ctype.h> // isspace()
#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <source/UIKit/UIKit.h>


#define MAX_COLUMNS 16

enum column_type
{
  column_type_invalid,
  column_type_auto,
  column_type_pixel,
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
    return (struct column){ column_type_auto, 0 };
  } else if (!isdigit(*a)) {
    return (struct column){ column_type_invalid, 0 };
  } else {
    int value = (int)strtol(a, (LPSTR*)&a, 10);
    if (!strncmp(a, "px", 2)) {
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
      if (parse)
        columns_parse(pGrid->Columns, &columns[axis]);
      return &columns[axis];
    case kDirectionVertical:
      if (parse)
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

static void
_CalculateAutos(float spacing, float avl, PCOLUMNS columns)
{
  uint32_t num_autos = 0;
  if (columns->count > 0) {
    avl -= spacing * (columns->count - 1);
  }
  FOR_LOOP(i, columns->count)
  {
    if (columns->columns[i].type == column_type_auto) {
      num_autos++;
    } else {
      avl -= columns->columns[i].width;
    }
  }
  float cursor = 0;
  FOR_LOOP(i, columns->count)
  {
    if (columns->columns[i].type == column_type_auto) {
      columns->columns[i].width = avl / num_autos;
    }
    columns->columns[i].position = cursor;
    cursor += columns->columns[i].width + spacing;
  }
}

HANDLER(Grid, MeasureOverride)
{
  Node2DPtr pNode2D = GetNode2D(hObject);
  uint32_t cellindex = 0;
  Size_t size = *pMeasureOverride;

  _CalculateAutos(pGrid->Spacing, size.width, columns_at_axis(pGrid, 0, TRUE));
  _CalculateAutos(pGrid->Spacing, size.height, columns_at_axis(pGrid, 1, TRUE));
	
  FOR_EACH_LAYOUTABLE(hChild, hObject)
  {
    struct column* w = column_at_cellindex(pGrid, kDirectionHorizontal, cellindex);
    struct column* h = column_at_cellindex(pGrid, kDirectionVertical, cellindex);

    OBJ_SendMessageW(hChild, kEventMeasure, 0, &(struct Size) {
      .width  = (w ? w->width : size.width),
      .height = (h ? h->width : size.height),
    });
    cellindex++;
  }
  
  return MAKEDWORD(size.width, size.height);
}

HANDLER(Grid, ArrangeOverride)
{
  Node2DPtr pNode2D = GetNode2D(hObject);
  uint32_t cellindex = 0;

  struct rect rect = {
    pArrangeOverride->x - PADDING_TOP(pNode2D, 0),
    pArrangeOverride->y - PADDING_TOP(pNode2D, 1),
    pArrangeOverride->width - TOTAL_PADDING(pNode2D, 0),
    pArrangeOverride->height - TOTAL_PADDING(pNode2D, 1)
  };
  
  FOR_EACH_LAYOUTABLE(hChild, hObject)
  {
    struct column* w = column_at_cellindex(pGrid, kDirectionHorizontal, cellindex);
    struct column* h = column_at_cellindex(pGrid, kDirectionVertical, cellindex);
    OBJ_SendMessageW(hChild, kEventArrange, 0, &(struct rect) {
      .x = rect.x + (w ? w->position : 0),
      .y = rect.y + (h ? h->position : 0),
      .width = (w ? w->width : rect.width),
      .height = (h ? h->width : rect.height),
//      .width = NODE2D_FRAME(GetNode2D(hChild), Size, 0).Desired,
//      .height = NODE2D_FRAME(GetNode2D(hChild), Size, 1).Desired,
  });
    cellindex++;
  }
  return MAKEDWORD(pArrangeOverride->width, pArrangeOverride->height);
}
