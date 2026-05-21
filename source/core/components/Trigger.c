#include <source/core/core_local.h>
#include <source/filesystem/filesystem.h>

#define kMsgTriggered 0x3b1c3ae2

extern int parse_property(const char* str,
                          struct PropertyType const* prop,
                          void* valueptr);

static bool_t
_TriggerMatches(struct Trigger const* expected, struct Trigger_TriggeredEventArgs const* triggered)
{
  return !expected ||
         (triggered && triggered->Trigger == expected);
}

static struct Object *
_TriggerSender(struct Object *hObject, struct Trigger_TriggeredEventArgs const* triggered)
{
  return (triggered && triggered->Sender) ? triggered->Sender : hObject;
}

static bool_t
_SetTargetVisible(struct Object *target, bool_t visible)
{
  if (!target) return FALSE;
  return SUCCEEDED(OBJ_SetPropertyValue(target, "Visible", &visible));
}

static bool_t
_BindActionTrigger(struct Object *hObject, struct Property **outProp)
{
  struct Property *prop = NULL;
  struct Trigger *trigger = GetTrigger(hObject);
  if (!trigger) {
    struct Object *owner = OBJ_GetParent(hObject);
    if (owner) {
      trigger = GetTrigger(owner);
      if (!trigger) {
        struct Object *trigger_object = OBJ_FindChildOfClass(owner, ID_Trigger);
        trigger = GetTrigger(trigger_object);
      }
    }
  }
  if (FAILED(OBJ_FindShortProperty(hObject, "Trigger", &prop))) {
    return FALSE;
  }
  if (!trigger) {
    return FALSE;
  }
  PROP_SetValue(prop, &trigger);
  if (outProp) {
    *outProp = prop;
  }
  return TRUE;
}

HANDLER(Trigger, Object, Attached)
{
  struct Property *prop = NULL;
  if (pTrigger->Property && SUCCEEDED(OBJ_FindShortProperty(hObject, pTrigger->Property, &prop)) && prop) {
    PROP_SetFlag(prop, PF_USED_IN_TRIGGER);
  }
  return FALSE;
}

HANDLER(Trigger, Object, PropertyChanged)
{
  if (!pPropertyChanged || !pPropertyChanged->Property ||
      !pTrigger->Property ||
      strcmp(PROP_GetName(pPropertyChanged->Property), pTrigger->Property))
    return FALSE;
  switch (PROP_GetType(pPropertyChanged->Property)) {
    case kDataTypeFloat:
      if (fabs(*(float*)PROP_GetValue(pPropertyChanged->Property) -
               pTrigger->Value) < 0.001f) {
        _SendMessage(hObject, Trigger, Triggered, .Trigger = pTrigger, .Sender = hObject);
      }
      return FALSE;
    case kDataTypeInt:
    case kDataTypeBool:
    case kDataTypeEnum:
      if (*(int*)PROP_GetValue(pPropertyChanged->Property) ==
          pTrigger->Value) {
        _SendMessage(hObject, Trigger, Triggered, .Trigger = pTrigger, .Sender = hObject);
      }
      return FALSE;
    default:
      return FALSE;
  }
}

HANDLER(OnAttachedTrigger, Object, Attached)
{
  struct Object *sender = OBJ_GetParent(hObject);
  _SendMessage(hObject, Trigger, Triggered,
               .Trigger = GetTrigger(CMP_GetObject(pOnAttachedTrigger)),
               .Sender = sender ? sender : hObject);
  return FALSE;
}

HANDLER(Trigger, Trigger, Triggered)
{
  struct Object *sender = _TriggerSender(hObject, pTriggered);
  
  for (struct Object *child = OBJ_GetFirstChild(hObject); child; child = OBJ_GetNext(child)) {
    LRESULT result = _SendMessage(child, Trigger, Triggered,
                 .Trigger = pTriggered->Trigger,
                 .Sender = sender);
    if (result) return result;
  }
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

  struct Object *sender = _TriggerSender(hObject, pTriggered);
  struct Property *p = NULL;
  if (FAILED(OBJ_FindShortProperty(sender, pSetter->Property, &p))) {
    Con_Error("Setter could not find property %s on %s",
              pSetter->Property, OBJ_GetClassName(sender));
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

static LRESULT
_EventTrigger_Fire(struct Object *hObject, struct EventTrigger const *pEventTrigger, struct Object *sender, lpcString_t routed_event)
{
  if (!pEventTrigger->RoutedEvent || !*pEventTrigger->RoutedEvent)
    return FALSE;
  if (strcmp(pEventTrigger->RoutedEvent, routed_event))
    return FALSE;
  return _SendMessage(hObject, Trigger, Triggered,
                .Trigger = GetTrigger(CMP_GetObject(pEventTrigger)),
                .Sender = sender ? sender : hObject);
}

HANDLER(ShowModalAction, Trigger, Triggered)
{
  if (!pShowModalAction->Path || !*pShowModalAction->Path) {
    Con_Error("ShowModalAction missing Path");
    return FALSE;
  }

  struct Object *sender = _TriggerSender(hObject, pTriggered);
  struct Object *target = FS_LoadObject(pShowModalAction->Path);
  if (!target) {
    Con_Error("ShowModalAction could not load template '%s'", pShowModalAction->Path);
    return FALSE;
  }

  if (!OBJ_GetComponent(target, fnv1a32("Popup"))) {
    Con_Error("ShowModalAction template '%s' is not a Popup", pShowModalAction->Path);
    OBJ_RemoveFromParent(target);
    return FALSE;
  }
  if (OBJ_ShowModalObject(sender, target)) {
    bool_t visible = TRUE;
    OBJ_SetPropertyValue(target, "Visible", &visible);
    return TRUE; // This will tell the system to refresh the screen
  }
  return FALSE;
}

HANDLER(ShowModalAction, Object, Attached)
{
  return FALSE;
}

HANDLER(HideAction, Trigger, Triggered)
{
  if (!pHideAction->Path || !*pHideAction->Path) {
    Con_Error("HideAction missing Path");
    return FALSE;
  }

  struct Object *sender = _TriggerSender(hObject, pTriggered);
  struct Object *target = OBJ_FindByPath(sender, pHideAction->Path);
  if (!target) {
    Con_Error("HideAction could not resolve path '%s'", pHideAction->Path);
    return FALSE;
  }

  _SetTargetVisible(target, FALSE);
  return FALSE;
}

HANDLER(HideAction, Object, Attached)
{
  return FALSE;
}

HANDLER(OnPropertyChangedTrigger, Object, PropertyChanged)
{
  if (!pPropertyChanged || !pPropertyChanged->Property ||
      !pOnPropertyChangedTrigger->Property ||
      strcmp(PROP_GetName(pPropertyChanged->Property),
             pOnPropertyChangedTrigger->Property))
    return FALSE;
  _SendMessage(hObject, Trigger, Triggered,
               .Trigger = GetTrigger(CMP_GetObject(pOnPropertyChangedTrigger)),
               .Sender = hObject);
  return FALSE;
}

HANDLER(OnPropertyChangedTrigger, Object, Attached)
{
  struct Property *pProp;
  lpcString_t szName = pOnPropertyChangedTrigger->Property;
  struct Object *source = hObject;
  if (pOnPropertyChangedTrigger->SourceNode && *pOnPropertyChangedTrigger->SourceNode) {
    source = OBJ_FindByPath(hObject, pOnPropertyChangedTrigger->SourceNode);
  }
  if (szName && source && SUCCEEDED(OBJ_FindShortProperty(source, szName, &pProp))) {
    PROP_SetFlag(pProp, PF_USED_IN_TRIGGER);
  }
  return FALSE;
}
