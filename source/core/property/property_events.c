#include "property_internal.h"

#define kMsgPropertyChanged 0x6d47e0cc

INLINE bool_t
PROP_HasHandler(lpProperty_t property)
{
  if (property->flags &
      (PF_HASCHANGECALLBACK | PF_USED_IN_STATE_MANAGER | PF_USED_IN_TRIGGER))
    return TRUE;
  if (property->callbackMsg)
    return TRUE;
  return FALSE;
}

static lpcString_t
PROP_GetShortName(lpcProperty_t property)
{
  return property->pdesc->Name;
}

static bool_t
invoke_event_property_from_property_events(lua_State* L, lpObject_t object, lpProperty_t callback, int num_args)
{
  if (!callback || PROP_GetType(callback) != kDataTypeEvent) {
    lua_pop(L, num_args);
    return FALSE;
  }
  event_t ref = *(event_t*)PROP_GetValue(callback);
  if (!ref) {
    lua_pop(L, num_args);
    return FALSE;
  }

  lua_geti(L, LUA_REGISTRYINDEX, ref);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1 + num_args);
    return FALSE;
  }

  lua_insert(L, -(num_args + 1)); /* func below args */
  luaX_pushObject(L, object);
  lua_insert(L, -(num_args + 1)); /* self below args */

  if (lua_pcall(L, num_args + 1, 1, 0) != LUA_OK) {
    Con_Error("property callback: %s", lua_tostring(L, -1));
    lua_pop(L, 1);
    return FALSE;
  }
  bool_t ret = lua_toboolean(L, -1);
  lua_pop(L, 1);
  return ret;
}

void
PROP_ProcessEvents(lua_State* L,
                   lpProperty_t property,
                   lpObject_t object)
{
  for (; property; property = property->next) {
    if (!PROP_HasHandler(property) || !PROP_HasChanged(property))
      continue;
    PROP_Update(property);
    if (property->flags & PF_USED_IN_STATE_MANAGER) {
      _SendMessage(object, StateManagerController, ControllerChanged, .Property = property);
    }
    if (property->flags & PF_HASCHANGECALLBACK) {
      static path_t str;
      sprintf(str, ON_CHANGED_CALLBACK, PROP_GetShortName(property));
      lpProperty_t callback = NULL;
      if (SUCCEEDED(OBJ_FindShortProperty(object, str, &callback))) {
        luaX_pushProperty(L, property);
        invoke_event_property_from_property_events(L, object, callback, 1);
      }
    }
    if (property->flags & PF_USED_IN_TRIGGER) {
      _SendMessage(object, Object, PropertyChanged, .Property = property);
    }
    if (property->callbackMsg) {
      PROP_ExecuteChangedCallback(L, object, property);
    }
  }
}

void
PROP_AttachProgram(lpProperty_t p,
                   enum PropertyAttribute a,
                   struct token* program,
                   lpcString_t code)
{
  SafeSet(p->programSources[a], strdup(code), free);
  SafeSet(p->programs[a], program, Token_Release);
}

bool_t
PROP_HasProgram(lpProperty_t p)
{
  FOR_LOOP(i, PropertyAttribute_Count)
  {
    if (p->programs[i]) {
      return TRUE;
    }
  }
  return FALSE;
}
