#include "../core_local.h"

#define SUFFIX "Changed"
#define SUFFIX_LEN 7

struct script_callback
{
  uint32_t value;
  shortStr_t name;
  struct script_callback* next;
};

void
OBJ_RegisterCallback(lpObject_t object, lpcString_t name)
{
  if (strncmp(name, "on", 2) || !isupper(name[2]))
    return;
  // Add regular callback
  struct script_callback* cb = ZeroAlloc(sizeof(struct script_callback));
  cb->value = fnv1a32(name + 2);
  strncpy(cb->name, name, sizeof(cb->name));
  ADD_TO_LIST(cb, _GetCallbacks(object));
  // Mark property as has-callback
  size_t str_len = strlen(name);
  if (str_len >= SUFFIX_LEN && strcmp(name + str_len - SUFFIX_LEN, SUFFIX) == 0) {
    size_t property_len = str_len - SUFFIX_LEN;
    shortStr_t pname = { 0 };
    lpProperty_t pProp = NULL;
    strncpy(pname, name + 2, property_len - 2);
    if (SUCCEEDED(OBJ_FindShortProperty(object, pname, &pProp))) {
      PROP_SetFlag(pProp, PF_HASCHANGECALLBACK);
    } else {
      Con_Error("Could not find property for %s", name);
    }
  }
}


lpcString_t
OBJ_FindCallbackForID(lpObject_t object, uint32_t id)
{
  FOR_EACH_LIST(struct script_callback, cb, _GetCallbacks(object))
  {
    if ((cb->value & MSG_DATA_MASK) == (id & MSG_DATA_MASK)) {
      return cb->name;
    }
  }
  return NULL;
}

ORCA_API int
luaX_getobjectcallback(lua_State* L, lpObject_t object, uint32_t id)
{
  FOR_EACH_LIST(struct script_callback, cb, _GetCallbacks(object)) {
    if ((cb->value & MSG_DATA_MASK) == (id & MSG_DATA_MASK)) {
      luaX_pushObject(L, object);
      lua_getfield(L, -1, cb->name);
      lua_remove(L, -2);
      return LUA_TFUNCTION;
    }
  }
  lpProperty_t event = PROP_FindByLongID(OBJ_GetProperties(object), id);
  if (event && PROP_GetType(event) == kDataTypeEvent) {
    lua_geti(L, LUA_REGISTRYINDEX, *(event_t*)PROP_GetValue(event));
    return LUA_TFUNCTION;
  }
  return LUA_TNIL;
}
