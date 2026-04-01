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
  if (property->pdesc->FullIdentifier != property->pdesc->ShortIdentifier) {
    return strrchr(property->pdesc->Name, '.') + 1;
  } else {
    return property->pdesc->Name;
  }
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
      SM_HandleControllerChange(object, property);
    }
    if (property->flags & PF_HASCHANGECALLBACK) {
      static path_t str;
      sprintf(str, ON_CHANGED_CALLBACK, PROP_GetShortName(property));
      luaX_pushProperty(L, property);
      luaX_executecallback(L, object, str, 1);
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
