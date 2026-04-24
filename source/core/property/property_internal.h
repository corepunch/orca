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
  [kDataTypeObject] = sizeof(struct Object *),
};

struct property_program {
  struct Property*        property;
  struct token*           token;
  char*                   code;       // source code (for editor / XML serialization)
  enum PropertyAttribute  attr;
  uint32_t                updateFrame;
  struct property_program* next;
};

struct Property
{
  struct Property*           next;
  struct Object*             object;
  struct PropertyType const *pdesc;
  void*                      value;
  uint32_t                   flags;
  uint32_t                   changeCallback; // Lua registry reference for the change callback function
  uint32_t                   updateFrame;
};

static inline bool_t
PROP_HasHandler(struct Property const *property)
{
  return (property->flags &
      (PF_HASCHANGECALLBACK | PF_USED_IN_STATE_MANAGER | PF_USED_IN_TRIGGER)) != 0;
}

bool_t _AssignCallback(lua_State* L, struct Property *property);
void PROP_FireNotification(lua_State* L, struct Property *property, struct Object *object);

#endif /* __PROPERTY_INTERNAL_H__ */
