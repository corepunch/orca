#include <source/UIKit/UIKit.h>


#define kEventTriggered 0x3b1c3ae2

HANDLER(Trigger, Attached)
{
  lpProperty_t prop;
  if (SUCCEEDED(OBJ_FindShortProperty(hObject, pTrigger->Property, &prop))) {
    PROP_SetFlag(prop, PF_USED_IN_TRIGGER);
  }
  return FALSE;
}

HANDLER(Trigger, PropertyChanged)
{
  if (strcmp(PROP_GetName(pPropertyChanged->hProperty), pTrigger->Property))
    return FALSE;
  switch (PROP_GetType(pPropertyChanged->hProperty)) {
    case kDataTypeFloat:
      if (fabs(*(float*)PROP_GetValue(pPropertyChanged->hProperty) -
               pTrigger->Value) < 0.001f) {
        OBJ_SendMessageW(hObject, kEventTriggered, 0, &(TRIGGEREDSTRUCT){
          pTrigger
        });
      }
      return FALSE;
    case kDataTypeInt:
    case kDataTypeBool:
    case kDataTypeEnum:
      if (*(int*)PROP_GetValue(pPropertyChanged->hProperty) ==
          pTrigger->Value) {
        OBJ_SendMessageW(hObject, kEventTriggered, 0, &(TRIGGEREDSTRUCT){
          pTrigger
        });
      }
      return FALSE;
    default:
      return FALSE;
  }
}

HANDLER(OnAttachedTrigger, Attached)
{
  OBJ_SendMessageW(hObject, kEventTriggered, 0, &(TRIGGEREDSTRUCT){
    pOnAttachedTrigger
  });
  return FALSE;
}

HANDLER(Setter, Triggered)
{
  if (pTriggered->Trigger ==
      CMP_GetUserData((struct component*)pSetter->Trigger)) {
    lpProperty_t p;
    if (SUCCEEDED(OBJ_FindShortProperty(hObject, pSetter->Property, &p))) {
      PROP_Parse(p, pSetter->Value);
    }
    Con_Error("setting %s to %s", pSetter->Property, pSetter->Value);
  }
  return FALSE;
}

#include <include/api.h>

HANDLER(Handler, Triggered)
{
  EVENT_PTR(HandleMessage) msg = pTriggered->message;
  if (pTriggered->Trigger ==
        CMP_GetUserData((struct component*)pHandler->Trigger) &&
      msg) {
    lpObject_t pTarget = pHandler->Target ? pHandler->Target : hObject;
    lua_State* L = msg->L;
    lua_geti(L, LUA_REGISTRYINDEX, OBJ_GetLuaObject(pTarget));
    lua_getfield(L, -1, pHandler->Function);
    if (lua_type(L, -1) == LUA_TFUNCTION) {
      lua_pop(L, 2);
      lua_geti(L, LUA_REGISTRYINDEX, OBJ_GetLuaObject(hObject));
      for (int i = 0; i < msg->NumArgs; i++) {
        lua_pushvalue(L, -(1 + msg->NumArgs));
      }
      return luaX_executecallback(
        L, pTarget, pHandler->Function, msg->NumArgs + 1);
    } else {
      lua_pop(L, 2);
    }
  }
  return FALSE;
}

HANDLER(EventTrigger, HandleMessage)
{
  if (!strcmp(pHandleMessage->EventName, pEventTrigger->RoutedEvent)) {
    TRIGGEREDSTRUCT parm = { pEventTrigger, pHandleMessage };
    return OBJ_SendMessageW(hObject, kEventTriggered, 0, &parm);
  }
  return FALSE;
}

HANDLER(OnPropertyChangedTrigger, PropertyChanged)
{
  if (strcmp(PROP_GetName(pPropertyChanged->hProperty),
             pOnPropertyChangedTrigger->Property))
    return FALSE;
  lua_State* L = pPropertyChanged->L;
  luaX_pushProperty(L, pPropertyChanged->hProperty);
  TRIGGEREDSTRUCT parm = {
    pOnPropertyChangedTrigger,
    &(HANDLEMESSAGESTRUCT){ .NumArgs = 1 },
  };
  return OBJ_SendMessageW(hObject, kEventTriggered, 0, &parm);
}

HANDLER(OnPropertyChangedTrigger, Attached)
{
  lpProperty_t pProp;
  lpcString_t szName = pOnPropertyChangedTrigger->Property;
  if (SUCCEEDED(OBJ_FindShortProperty(hObject, szName, &pProp))) {
    PROP_SetFlag(pProp, PF_USED_IN_TRIGGER);
  }
  return FALSE;
}
