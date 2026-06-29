#include <core/core.h>
#include <include/api.h>
#include <include/codegen.h>

HANDLER(DataObject, Object, Start) {
  return FALSE;
}

HANDLER(DataObjectString, Object, Start) {
  struct Property *dt = OBJ_FindLongProperty(hObject, ID_DataObject_DataType);
  if (dt) {
    int stringVal = 1;
    PROP_SetValue(dt, &stringVal);
  }
  return FALSE;
}

HANDLER(DataObjectInt, Object, Start) {
  struct Property *dt = OBJ_FindLongProperty(hObject, ID_DataObject_DataType);
  if (dt) {
    int intVal = 2;
    PROP_SetValue(dt, &intVal);
  }
  return FALSE;
}

HANDLER(DataObjectFloat, Object, Start) {
  struct Property *dt = OBJ_FindLongProperty(hObject, ID_DataObject_DataType);
  if (dt) {
    int floatVal = 3;
    PROP_SetValue(dt, &floatVal);
  }
  return FALSE;
}

HANDLER(DataObjectBool, Object, Start) {
  struct Property *dt = OBJ_FindLongProperty(hObject, ID_DataObject_DataType);
  if (dt) {
    int boolVal = 4;
    PROP_SetValue(dt, &boolVal);
  }
  return FALSE;
}
