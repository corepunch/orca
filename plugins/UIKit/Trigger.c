#include <plugins/UIKit/UIKit.h>


#define kMsgTriggered 0x3b1c3ae2

HANDLER(Trigger, Object, Attached)
{
  lpProperty_t prop;
  if (pTrigger->Property && SUCCEEDED(OBJ_FindShortProperty(hObject, pTrigger->Property, &prop))) {
    PROP_SetFlag(prop, PF_USED_IN_TRIGGER);
  }
  return FALSE;
}

HANDLER(Trigger, Object, PropertyChanged)
{
  if (!pTrigger->Property || strcmp(PROP_GetName(pPropertyChanged->Property), pTrigger->Property))
    return FALSE;
  switch (PROP_GetType(pPropertyChanged->Property)) {
    case kDataTypeFloat:
      if (fabs(*(float*)PROP_GetValue(pPropertyChanged->Property) -
               pTrigger->Value) < 0.001f) {
        _SendMessage(hObject, Trigger, Triggered, pTrigger);
      }
      return FALSE;
    case kDataTypeInt:
    case kDataTypeBool:
    case kDataTypeEnum:
      if (*(int*)PROP_GetValue(pPropertyChanged->Property) ==
          pTrigger->Value) {
        _SendMessage(hObject, Trigger, Triggered, pTrigger);
      }
      return FALSE;
    default:
      return FALSE;
  }
}

HANDLER(OnAttachedTrigger, Object, Attached)
{
  _SendMessage(hObject, Trigger, Triggered, GetTrigger(CMP_GetObject(pOnAttachedTrigger)));
  return FALSE;
}

HANDLER(Setter, Trigger, Triggered)
{
  if (pTriggered->Trigger ==
      CMP_GetUserData((struct component*)pSetter->Trigger)) {
    lpProperty_t p;
    if (pSetter->Property && SUCCEEDED(OBJ_FindShortProperty(hObject, pSetter->Property, &p))) {
//      PROP_Parse(p, pSetter->Value);
      assert(!"Not implemented");
    }
    Con_Error("setting %s to %s", pSetter->Property ? pSetter->Property : "", pSetter->Value ? pSetter->Value : "");
  }
  return FALSE;
}

#include <include/api.h>

HANDLER(Handler, Trigger, Triggered)
{
//  HandleMessageMsgPtr msg = &pTriggered->message;
//  if (pTriggered->Trigger ==
//        CMP_GetUserData((struct component*)pHandler->Trigger) &&
//      msg) {
//    lpObject_t pTarget = pHandler->Target ? CMP_GetObject(pHandler->Target) : hObject;
//    lua_State* L = OBJ_GetDomain(hObject);
//    lua_geti(L, LUA_REGISTRYINDEX, OBJ_GetLuaObject(pTarget));
//    lua_getfield(L, -1, pHandler->Function ? pHandler->Function : "");
//    if (lua_type(L, -1) == LUA_TFUNCTION) {
//      lua_pop(L, 2);
//      lua_geti(L, LUA_REGISTRYINDEX, OBJ_GetLuaObject(hObject));
//      for (int i = 0; i < msg->NumArgs; i++) {
//        lua_pushvalue(L, -(1 + msg->NumArgs));
//      }
//      return luaX_executecallback(L, pTarget, pHandler->Function ? pHandler->Function : "", msg->NumArgs + 1);
//    } else {
//      lua_pop(L, 2);
//    }
//  }
  return FALSE;
}

HANDLER(EventTrigger, Node, HandleMessage)
{
  if (pEventTrigger->RoutedEvent && !strcmp(pHandleMessage->EventName, pEventTrigger->RoutedEvent)) {
//    return _SendMessage(hObject, Trigger, Triggered,
//      .Trigger = GetTrigger(CMP_GetObject(pEventTrigger)),
//      .message = *pHandleMessage);
  }
  return FALSE;
}

HANDLER(OnPropertyChangedTrigger, Object, PropertyChanged)
{
  if (!pOnPropertyChangedTrigger->Property ||
      strcmp(PROP_GetName(pPropertyChanged->Property),
             pOnPropertyChangedTrigger->Property))
    return FALSE;
  lua_State* L = OBJ_GetDomain(hObject);
  luaX_pushProperty(L, pPropertyChanged->Property);
  return _SendMessage(hObject, Trigger, Triggered,
    .Trigger = GetTrigger(CMP_GetObject(pOnPropertyChangedTrigger)),
    /*.message = { .NumArgs = 1 }*/);
}

HANDLER(OnPropertyChangedTrigger, Object, Attached)
{
  lpProperty_t pProp;
  lpcString_t szName = pOnPropertyChangedTrigger->Property;
  if (szName && SUCCEEDED(OBJ_FindShortProperty(hObject, szName, &pProp))) {
    PROP_SetFlag(pProp, PF_USED_IN_TRIGGER);
  }
  return FALSE;
}
