#ifndef ORCA_FS_XML_INLINE_H
#define ORCA_FS_XML_INLINE_H

#include <source/core/core_local.h>
#include <source/filesystem/fs_local.h>

struct PropertyType const *_FindStructField(struct StructDesc const *sdesc,
                                            lpcString_t name);
bool_t _SetStructFieldFromString(struct PropertyType const *pdesc,
                                 void *valueptr,
                                 lpcString_t value);
void _ClearStructValue(struct StructDesc const *sdesc, void *value);

bool_t _LoadStructFromXmlFragment(struct Property *prop,
                                  struct PropertyType const *pdesc,
                                  lpcString_t text);
bool_t _ParseStructXmlFragment(struct PropertyType const *pdesc,
                               void *dst,
                               lpcString_t text);
struct Object *_LoadObjectFromXmlFragment(lpcString_t text,
                                          int positional_start);
char *_ExpandXmlPositionalArgs(lpcString_t xml);

#endif
