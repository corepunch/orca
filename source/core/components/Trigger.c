#include <source/core/core_local.h>

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
      // TODO: implement property parsing via PROP_Parse when available
      (void)p;
    }
    // TODO: remove this debug log once Setter is fully implemented
    Con_Error("setting %s to %s", pSetter->Property ? pSetter->Property : "", pSetter->Value ? pSetter->Value : "");
  }
  return FALSE;
}

// TODO: implement Handler_Triggered — needs lua callback support wired to target object
HANDLER(Handler, Trigger, Triggered)
{
  return FALSE;
}

// TODO: implement EventTrigger handler — waiting for Node.HandleMessage routing support

HANDLER(OnPropertyChangedTrigger, Object, PropertyChanged)
{
  if (!pOnPropertyChangedTrigger->Property ||
      strcmp(PROP_GetName(pPropertyChanged->Property),
             pOnPropertyChangedTrigger->Property))
    return FALSE;
  // lua_State* L = OBJ_GetDomain(hObject);
  // luaX_pushProperty(L, pPropertyChanged->Property);
  // return _SendMessage(hObject, Trigger, Triggered,
  //   .Trigger = GetTrigger(CMP_GetObject(pOnPropertyChangedTrigger)),
    /*.message = { .NumArgs = 1 }*/
    // );
    return FALSE;
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
