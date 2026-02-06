#include "core_local.h"

struct script_callback
{
  uint32_t value;
  shortStr_t name;
  struct script_callback* next;
};

void
OBJ_ProcessFunctions2(lpObject_t object, lpcString_t name)
{
  if (name[0] != 'o' || name[1] != 'n')
    return;
  struct script_callback* cb = ZeroAlloc(sizeof(struct script_callback));
  cb->value = fnv1a32(name + 2);
  strncpy(cb->name, name, sizeof(cb->name));
  ADD_TO_LIST(cb, _GetCallbacks(object));
}

lpcString_t
OBJ_FindCallbackForID(lpObject_t object, uint32_t id)
{
  FOR_EACH_LIST(struct script_callback, cb, _GetCallbacks(object))
  {
    if (cb->value == id) {
      return cb->name;
    }
  }
  return NULL;
}

