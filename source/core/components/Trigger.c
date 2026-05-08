#include <source/core/core_local.h>

#define kMsgTriggered 0x3b1c3ae2

extern int parse_property(const char* str,
                          struct PropertyType const* prop,
                          void* valueptr);

static bool_t
_TriggerMatches(struct Trigger const* expected, struct Trigger_TriggeredEventArgs const* triggered)
{
  return !expected ||
         (triggered && triggered->Trigger ==
            CMP_GetUserData((struct component*)expected));
}

static bool_t
_SetTargetVisible(struct Object *target, bool_t visible)
{
  if (!target) return FALSE;
  return SUCCEEDED(OBJ_SetPropertyValue(target, "Visible", &visible));
}

HANDLER(Trigger, Object, Attached)
{
  struct Property *prop;
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
  if (!_TriggerMatches(pSetter->Trigger, pTriggered)) {
    return FALSE;
  }

  if (!pSetter->Property || !pSetter->Value) {
    Con_Error("Setter missing Property or Value");
    return FALSE;
  }

  struct Property *p = NULL;
  if (FAILED(OBJ_FindShortProperty(hObject, pSetter->Property, &p))) {
    Con_Error("Setter could not find property %s on %s",
              pSetter->Property, OBJ_GetClassName(hObject));
    return FALSE;
  }

  char buf[MAX_PROPERTY_STRING] = {0};
  if (!parse_property(pSetter->Value, PROP_GetDesc(p), buf)) {
    Con_Error("Setter could not parse value '%s' for property %s",
              pSetter->Value, pSetter->Property);
    return FALSE;
  }

  PROP_SetValue(p, buf);
  if (PROP_GetType(p) == kDataTypeString) {
    free(*(char**)buf);
  }
  return FALSE;
}

HANDLER(Handler, Trigger, Triggered)
{
  if (!_TriggerMatches(pHandler->Trigger, pTriggered)) {
    return FALSE;
  }

  if (!pHandler->Function || !*pHandler->Function) {
    Con_Error("Handler missing Function");
    return FALSE;
  }

  struct Object *target = hObject;
  if (pHandler->TargetPath && *pHandler->TargetPath) {
    target = OBJ_FindByPath(hObject, pHandler->TargetPath);
  } else if (pHandler->Target) {
    target = CMP_GetObject((struct component*)pHandler->Target);
  }
  if (!target) {
    Con_Error("Handler target not found for function %s", pHandler->Function);
    return FALSE;
  }

  if (!strcmp(pHandler->Function, "Show")) {
    _SetTargetVisible(target, TRUE);
  } else if (!strcmp(pHandler->Function, "Hide")) {
    _SetTargetVisible(target, FALSE);
  } else if (!strcmp(pHandler->Function, "Toggle")) {
    struct Property *p = NULL;
    if (SUCCEEDED(OBJ_FindShortProperty(target, "Visible", &p))) {
      bool_t visible = !*(bool_t*)PROP_GetValue(p);
      PROP_SetValue(p, &visible);
    } else {
      Con_Error("Handler could not find Visible property on %s",
                OBJ_GetClassName(target));
    }
  } else if (!strcmp(pHandler->Function, "ShowModal")) {
    if (OBJ_ShowModalObject(hObject, target)) {
      _SetTargetVisible(target, TRUE);
    }
  } else {
    Con_Error("Unknown handler function '%s'", pHandler->Function);
  }

  return FALSE;
}

static bool_t
_EventTrigger_Fire(struct Object *hObject, struct EventTrigger const *pEventTrigger, lpcString_t routed_event)
{
  if (pEventTrigger->RoutedEvent &&
      strcmp(pEventTrigger->RoutedEvent, routed_event))
    return FALSE;
  _SendMessage(hObject, Trigger, Triggered, GetTrigger(CMP_GetObject(pEventTrigger)));
  return FALSE;
}

HANDLER(EventTrigger, Node, LeftButtonUp)
{
  return _EventTrigger_Fire(hObject, pEventTrigger, "Node.LeftButtonUp");
}

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
  struct Property *pProp;
  lpcString_t szName = pOnPropertyChangedTrigger->Property;
  if (szName && SUCCEEDED(OBJ_FindShortProperty(hObject, szName, &pProp))) {
    PROP_SetFlag(pProp, PF_USED_IN_TRIGGER);
  }
  return FALSE;
}
