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

HANDLER(Grid, UpdateLayout)
{
  Node2DPtr pNode2D = GetNode2D(hObject);
  uint32_t cellindex = 0;

  FOR_LOOP(i, 2)
  {
    float size = pUpdateLayout->Size[i] - TOTAL_PADDING(pNode2D, i);
    _CalculateAutos(pGrid->Spacing, size, columns_at_axis(pGrid, i, TRUE));
    Node2D_Measure(pNode2D, i, pUpdateLayout->Size[i], TRUE);
  }
	
	// clang-format off

#ifdef CALCULATE_ROWS
  PCOLUMNS axis_columns = columns_at_axis(pGrid, kDirectionVertical, FALSE);
  PCOLUMNS dir_columns = columns_at_axis(pGrid, pGrid->Direction, FALSE);
  float max_pos_size[2] = { 0, 0 };  // Track maximum pos+size per axis
  int prev_row_index = -1;

  FOR_EACH_LAYOUTABLE(hChild, hObject)
  {
    Node2DPtr subview = GetNode2D(hChild);
    struct column* cell[2] = {
      column_at_cellindex(pGrid, kDirectionHorizontal, cellindex),
      column_at_cellindex(pGrid, kDirectionVertical, cellindex)
    };
    float child_width = cell[0] ? cell[0]->width : pUpdateLayout->Width;
    float child_height = cell[1] ? cell[1]->width : pUpdateLayout->Height;
    OBJ_SendMessageW(hChild, kEventUpdateLayout, 0,
      &(UPDATELAYOUTSTRUCT){
        .Width  = child_width,
        .Height = child_height,
      });
    
    // When Rows is not defined, use max pos+size from previous row
    float position_offset = 0;
    if (axis_columns->count == 0) {
      int cols_per_row = dir_columns->count > 0 ? dir_columns->count : 1;
      int row_index = cellindex / cols_per_row;
      if (row_index != prev_row_index && prev_row_index >= 0) {
        position_offset = max_pos_size[kDirectionVertical] + pGrid->Spacing;
      }
      prev_row_index = row_index;
    }
    
    FOR_LOOP(i, 2)
    {
      float p = PADDING_TOP(pNode2D, i);
      float position = cell[i] ? cell[i]->position : (i == kDirectionVertical && axis_columns->count == 0 ? position_offset : 0);
      float size = cell[i] ? cell[i]->width : (i == kDirectionHorizontal ? child_width : child_height);
      float end = position + size;
      struct bounds bounds = { p + position, p + end };
      Node2D_Arrange(subview, bounds, i);
      
      // Track maximum position + size for each axis
      if (!cell[i]) {
        max_pos_size[i] = MAX(max_pos_size[i], end);
      }
    }
    cellindex++;
  }
#else
  FOR_EACH_LAYOUTABLE(hChild, hObject)
  {
    Node2DPtr subview = GetNode2D(hChild);
    struct column* cell[2] = {
      column_at_cellindex(pGrid, kDirectionHorizontal, cellindex),
      column_at_cellindex(pGrid, kDirectionVertical, cellindex)
    };
    OBJ_SendMessageW(hChild, kEventUpdateLayout, 0,
      &(UPDATELAYOUTSTRUCT){
        .Width  = cell[0] ? cell[0]->width : pUpdateLayout->Width,
        .Height = cell[1] ? cell[1]->width : pUpdateLayout->Height,
      });
    FOR_LOOP(i, 2)
    {
      float p = PADDING_TOP(pNode2D, i);
      struct bounds bounds = {
				p + (cell[i] ? cell[i]->position : 0),
				p + (cell[i] ? (cell[i]->position + cell[i]->width) : pUpdateLayout->Size[i]) };
      Node2D_Arrange(subview, bounds, i);
    }
    cellindex++;
  }
#endif

	// clang-format on
  return TRUE;
}
