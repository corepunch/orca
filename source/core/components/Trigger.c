#include <ctype.h>
#include <source/core/core_local.h>
#include <filesystem/filesystem.h>

#define kMsgTriggered 0x3b1c3ae2

extern int parse_property(const char* str,
                          struct PropertyType const* prop,
                          void* valueptr);

static struct Object *
_TriggerSender(struct Object *hObject, struct Trigger_TriggeredEventArgs const* triggered)
{
  return (triggered && triggered->Sender) ? triggered->Sender : hObject;
}

static struct Object *
_ActionSender(struct Object *hObject, struct Action_DispatchEventArgs const* dispatch)
{
  return (dispatch && dispatch->Sender) ? dispatch->Sender : hObject;
}

static bool_t
_SetTargetVisible(struct Object *target, bool_t visible)
{
  if (!target) return FALSE;
  return SUCCEEDED(OBJ_SetPropertyValue(target, "Visible", &visible));
}

static lpcString_t
_BindingSkipSpace(lpcString_t s)
{
  while (s && *s && isspace((unsigned char)*s)) s++;
  return s;
}


static lpcString_t
_BindingGetStringProperty(struct Object *obj, lpcString_t name)
{
  struct Property *prop = NULL;
  if (FAILED(OBJ_FindShortProperty(obj, name, &prop)) || !prop) {
    return NULL;
  }
  if (PROP_GetType(prop) != kDataTypeString) {
    return NULL;
  }
  return *(lpcString_t*)PROP_GetValue(prop);
}

static int
_BindingGetIntProperty(struct Object *obj, lpcString_t name, int fallback)
{
  struct Property *prop = NULL;
  if (FAILED(OBJ_FindShortProperty(obj, name, &prop)) || !prop) {
    return fallback;
  }
  if (PROP_GetType(prop) != kDataTypeEnum &&
      PROP_GetType(prop) != kDataTypeInt &&
      PROP_GetType(prop) != kDataTypeBool) {
    return fallback;
  }
  return *(int*)PROP_GetValue(prop);
}


static LRESULT
_BindingCompileToProperty(struct Object *hObject,
                         struct Binding *binding,
                         struct Property *property,
                         bool_t normalize_markup)
{
  if (!property) {
    Con_Error("Binding.Compile requires a target property in lParam");
    return TRUE;
  }

  lpcString_t expr = _BindingGetStringProperty(hObject, "Expression");
  if (!expr || !*expr) {
    expr = OBJ_GetTextContent(hObject);
  }
  expr = _BindingSkipSpace(expr);
  if (!expr || !*expr) {
    Con_Error("Binding has no expression for property '%s'", PROP_GetName(property));
    return TRUE;
  }

  enum BindingMode mode = (enum BindingMode)
    _BindingGetIntProperty(hObject, "Mode", kBindingModeExpression);
  bool_t enabled = _BindingGetIntProperty(hObject, "Enabled", TRUE) ? TRUE : FALSE;

  lpcString_t final_expr = expr;
  fixedString_t normalized_expr = {0};
  if (normalize_markup) {
    size_t len = strlen(expr);
    if (len > 9 && !strncmp(expr, "{Binding ", 9) && expr[len - 1] == '}') {
      /* {Binding X} → {X}; runtime default resolves X relative to template root */
      snprintf(normalized_expr, sizeof(normalized_expr), "{%.*s}", (int)(len - 10), expr + 9);
      final_expr = normalized_expr;
    } else if (*expr != '{') {
      /* Bare path X → {X}; runtime default resolves relative to template root */
      snprintf(normalized_expr, sizeof(normalized_expr), "{%s}", expr);
      final_expr = normalized_expr;
    }
  }

  binding->property = property;
  if (!PROP_SetBinding(property, final_expr, mode, enabled)) {
    Con_Error("Binding failed for property '%s' on '%s'",
              PROP_GetName(property),
              OBJ_GetClassName(PROP_GetObject(property)));
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

HANDLER(Binding, Binding, Compile)
{
  return _BindingCompileToProperty(hObject,
                                   pBinding,
                                   (struct Property *)pCompile,
                                   TRUE);
}

HANDLER(BindingExpression, Binding, Compile)
{
  return _BindingCompileToProperty(hObject,
                                   (struct Binding *)pBindingExpression,
                                   (struct Property *)pCompile,
                                   FALSE);
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
        _SendMessage(hObject, Trigger, Triggered, .Sender = hObject);
      }
      return FALSE;
    case kDataTypeInt:
    case kDataTypeBool:
    case kDataTypeEnum:
      if (*(int*)PROP_GetValue(pPropertyChanged->Property) ==
          pTrigger->Value) {
        _SendMessage(hObject, Trigger, Triggered, .Sender = hObject);
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
               .Sender = sender ? sender : hObject);
  return FALSE;
}

HANDLER(Trigger, Trigger, Triggered)
{
  struct Object *sender = _TriggerSender(hObject, pTriggered);
  
  for (struct Object *child = OBJ_GetFirstChild(hObject); child; child = OBJ_GetNext(child)) {
    LRESULT result = _SendMessage(child, Action, Dispatch,
                 .Sender = sender);
    if (result) return result;
  }
  return FALSE;
}

HANDLER(Setter, Action, Dispatch)
{
  if (!pSetter->Property || !pSetter->Value) {
    Con_Error("Setter missing Property or Value");
    return FALSE;
  }

  struct Object *sender = _ActionSender(hObject, pDispatch);
  struct Property *p = NULL;
  if (FAILED(OBJ_FindShortProperty(sender, pSetter->Property, &p))) {
    if (OBJ_SetShorthandValueFromString(sender, pSetter->Property, pSetter->Value)) {
      return FALSE;
    }
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
                .Sender = sender ? sender : hObject);
}

HANDLER(HideAction, Action, Dispatch)
{
  if (!pHideAction->Path || !*pHideAction->Path) {
    Con_Error("HideAction missing Path");
    return FALSE;
  }

  struct Object *sender = _ActionSender(hObject, pDispatch);
  struct Object *target = OBJ_FindByPath(sender, pHideAction->Path);
  if (!target) {
    Con_Error("HideAction could not resolve path '%s'", pHideAction->Path);
    return FALSE;
  }

  _SetTargetVisible(target, FALSE);
  return TRUE;
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
