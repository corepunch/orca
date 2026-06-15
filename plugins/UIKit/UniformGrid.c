#include <include/api.h>
#include <include/orca.h>

#include <stdio.h>
#include <string.h>

#include <UIKit/UIKit.h>

#define UNIFORM_GRID_MAX_COLUMNS 16

// Grid_MeasureOverride
HANDLER(Grid, Node2D, MeasureOverride);
// Grid_ArrangeOverride
HANDLER(Grid, Node2D, ArrangeOverride);

static void
_UniformGridSyncColumns(struct Object *hObject,
                        struct Grid *pGrid,
                        struct UniformGrid *pUniformGrid)
{
  if (!pUniformGrid || !pUniformGrid->_autoColumns || !pGrid) {
    return;
  }

  int child_count = 0;
  FOR_EACH_LAYOUTABLE(hChild, hObject) {
    if (child_count >= UNIFORM_GRID_MAX_COLUMNS) {
      break;
    }
    child_count++;
  }

  if (child_count <= 0) {
    return;
  }

  char columns[256] = { 0 };
  char *cursor = columns;
  size_t remaining = sizeof(columns);
  for (int i = 0; i < child_count; i++) {
    int written = snprintf(cursor, remaining, "%s1fr", i ? " " : "");
    if (written < 0 || (size_t)written >= remaining) {
      break;
    }
    cursor += written;
    remaining -= (size_t)written;
  }

  if (pGrid->Columns && !strcmp(pGrid->Columns, columns)) {
    return;
  }

  struct Property *prop = OBJ_FindLongProperty(hObject, ID_Grid_Columns);
  if (prop) {
    pUniformGrid->_syncingColumns = TRUE;
    PROP_SetStringValue(prop, columns);
    pUniformGrid->_syncingColumns = FALSE;
  }
}

// UniformGrid_Create
HANDLER(UniformGrid, Object, Create)
{
  pUniformGrid->_autoColumns = TRUE;
  pUniformGrid->_syncingColumns = FALSE;
  return FALSE;
}

// UniformGrid_PropertyChanged
HANDLER(UniformGrid, Object, PropertyChanged)
{
  struct Grid *pGrid = GetGrid(hObject);
  if (!pGrid || pUniformGrid->_syncingColumns || !pPropertyChanged->Property) {
    return FALSE;
  }

  if (PROP_GetLongIdentifier(pPropertyChanged->Property) == ID_Grid_Columns) {
    pUniformGrid->_autoColumns = !pGrid->Columns || !*pGrid->Columns;
  }
  return FALSE;
}

// UniformGrid_MeasureOverride
HANDLER(UniformGrid, Node2D, MeasureOverride)
{
  struct Grid *pGrid = GetGrid(hObject);
  if (!pGrid) {
    return FALSE;
  }
  _UniformGridSyncColumns(hObject, pGrid, pUniformGrid);
  return Grid_MeasureOverride(hObject, pGrid, wParam, pMeasureOverride);
}

// UniformGrid_ArrangeOverride
HANDLER(UniformGrid, Node2D, ArrangeOverride)
{
  struct Grid *pGrid = GetGrid(hObject);
  if (!pGrid) {
    return FALSE;
  }
  _UniformGridSyncColumns(hObject, pGrid, pUniformGrid);
  return Grid_ArrangeOverride(hObject, pGrid, wParam, pArrangeOverride);
}
