#include <source/core/core_local.h>
#include <include/api.h>
#include <include/codegen.h>

HANDLER(DataContext, DataContext, GetData) {
  struct Object *data = OBJ_FindByPath(hObject, pGetData->Path ? pGetData->Path : "");
  return (LRESULT)GetDataObject(data);
}

HANDLER(DataObject, Object, Start) {
  return FALSE;
}
