#include "property_internal.h"

static lpcString_t
PROP_GetShortName(lpcProperty_t property)
{
  return property->pdesc->Name;
}

void
PROP_FireNotification(lua_State* L,
                      lpProperty_t property,
                      lpObject_t object)
{
  if (!property || !PROP_HasHandler(property)) return;
  PROP_Update(property);
  if (property->flags & PF_USED_IN_STATE_MANAGER) {
    _SendMessage(object, StateManagerController, ControllerChanged, .Property = property);
  }
  if (property->flags & PF_HASCHANGECALLBACK) {
    static path_t str;
    sprintf(str, ON_CHANGED_CALLBACK, PROP_GetShortName(property));
    luaX_pushProperty(L, property);
    luaX_executecallback(L, object, str, 1);
  }
  if (property->callbackMsg) {
    PROP_ExecuteChangedCallback(L, object, property);
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
