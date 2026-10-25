#include "fs_struct.h"

extern int parse_property(const char *, struct PropertyType const *, void *);

struct PropertyType const *
_FindStructField(struct StructDesc const *sdesc, lpcString_t name)
{
  FOR_LOOP(i, sdesc->NumProperties) {
    if (!strcmp(sdesc->Properties[i].Name, name)) {
      return &sdesc->Properties[i];
    }
  }
  return NULL;
}

bool_t
_SetStructFieldFromString(struct PropertyType const *pdesc,
                          void *valueptr,
                          lpcString_t value)
{
  char tmp[MAX_PROPERTY_STRING] = {0};
  lpcString_t resolved = (value[0] == '$') ? FS_GetThemeValue(value) : value;
  if (!resolved || !parse_property(resolved, pdesc, tmp)) return FALSE;

  if (pdesc->DataType == kDataTypeString) {
    *(char**)valueptr = *(char**)tmp;
  } else {
    memcpy(valueptr, tmp, pdesc->DataSize);
  }
  return TRUE;
}

void
_ClearStructValue(struct StructDesc const *sdesc, void *value)
{
  FOR_LOOP(i, sdesc->NumProperties) {
    struct PropertyType const *field = &sdesc->Properties[i];
    if (field->DataType == kDataTypeString && field->DataSize == sizeof(char*)) {
      free(*(char **)((char *)value + field->Offset));
    }
  }
}
