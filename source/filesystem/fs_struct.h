#ifndef ORCA_FS_STRUCT_H
#define ORCA_FS_STRUCT_H

#include <source/core/core_local.h>
#include <source/filesystem/fs_local.h>

struct PropertyType const *_FindStructField(struct StructDesc const *sdesc,
                                            lpcString_t name);
bool_t _SetStructFieldFromString(struct PropertyType const *pdesc,
                                 void *valueptr,
                                 lpcString_t value);
void _ClearStructValue(struct StructDesc const *sdesc, void *value);

#endif
