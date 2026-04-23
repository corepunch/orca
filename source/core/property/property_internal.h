#ifndef __PROPERTY_INTERNAL_H__
#define __PROPERTY_INTERNAL_H__

#include <source/core/core_local.h>

#define kMsgPropertyChanged 0x6d47e0cc

#define ID_ContentOffset 0x35a57c45

enum uniform_precision { None, Low, Mid, High };

static size_t psize[] = {
  0,                  // kDataTypeNone
  sizeof(bool_t),     // kDataTypeBool
  sizeof(int),        // kDataTypeInt
  sizeof(int),        // kDataTypeEnum
  sizeof(float),      // kDataTypeFloat
  sizeof(char*),      // kDataTypeString
  sizeof(void*),      // kDataTypeEvent
  0,                  // kDataTypeStruct
  sizeof(struct color), // kDataTypeColor
  sizeof(lpObject_t), // kDataTypeObject
};

struct Property
{
  eDataType_t            type;
  enum uniform_precision precision;
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

static inline bool_t
PROP_HasHandler(lpcProperty_t property)
{
  if (property->flags &
      (PF_HASCHANGECALLBACK | PF_USED_IN_STATE_MANAGER))
    return TRUE;
  if (property->callbackMsg)
    return TRUE;
  return FALSE;
}

bool_t _AssignCallback(lua_State* L, lpProperty_t property);
void PROP_FireNotification(lua_State* L, lpProperty_t property, lpObject_t object);

#endif /* __PROPERTY_INTERNAL_H__ */
