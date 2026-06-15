#ifndef __PROPERTY_INTERNAL_H__
#define __PROPERTY_INTERNAL_H__

#include <source/core/core_local.h>

/* struct Binding is defined in generated/core/core.h (the Binding component).
 * That same struct is used as the runtime binding node on struct Property.
 * Fields relevant here:
 *   token       — compiled VM token tree
 *   property    — back-pointer to the owning Property
 *   Expression  — original source text (const char*, set by PROP_SetBinding)
 *   updateFrame — last frame this binding ran
 */

#define ID_ContentOffset 0x35a57c45

static size_t psize[] = {
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

struct Property
{
  struct Property*           next;
  struct Binding*            binding;
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

struct Object *PROP_GetObjectValue(struct Property const *property);
void const *PROP_GetRawValueSlot(struct Property const *property);
bool_t _AssignCallback(lua_State* L, struct Property *property);
void PROP_FireNotification(lua_State* L, struct Property *property, struct Object *object);
void PROP_NotifyChanged(struct Property *property);

#endif /* __PROPERTY_INTERNAL_H__ */
