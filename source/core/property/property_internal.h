#ifndef __PROPERTY_INTERNAL_H__
#define __PROPERTY_INTERNAL_H__

#include <source/core/core_local.h>

#define ID_ContentOffset 0x35a57c45

static size_t psize[] = {
  [kDataTypeNone] = 0,
  [kDataTypeBool] = sizeof(bool_t),
  [kDataTypeInt] = sizeof(int),
  [kDataTypeEnum] = sizeof(int),
  [kDataTypeFloat] = sizeof(float),
  [kDataTypeString] = sizeof(char*),
  [kDataTypeEvent] = sizeof(void*),
  [kDataTypeStruct] = 0,
  [kDataTypeColor] = sizeof(struct color),
  [kDataTypeObject] = sizeof(lpObject_t),
};

struct Property
{
  eDataType_t            type;
  struct token*          programs[PropertyAttribute_Count];
  LPSTR                  programSources[PropertyAttribute_Count];
  uint32_t               flags;
  void*                  value;
  lpObject_t             object;
  lpcPropertyType_t      pdesc;
  uint32_t               updateFrame;
  uint32_t               stateflags;
  lpProperty_t           callbackMsg;
  lpProperty_t           next;
  char                   states[];
};

static inline void*
PROP_GetState(lpProperty_t p, enum PropertyState state)
{
  return p->states + state * PROP_GetSize(p);
}

static inline bool_t
PROP_HasChanged(lpProperty_t property)
{
  if (memcmp(PROP_GetState(property, kPropertyStateOldValue), property->value, PROP_GetSize(property))) {
    memcpy((void*)PROP_GetState(property, kPropertyStateOldValue), property->value, PROP_GetSize(property));
    return TRUE;
  }
  return FALSE;
}

bool_t _AssignCallback(lua_State* L, lpProperty_t property);

#endif /* __PROPERTY_INTERNAL_H__ */
