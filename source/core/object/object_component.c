#include "object_internal.h"

struct component*
OBJ_AddComponent(struct Object *pobj, uint32_t class_id)
{
  struct ClassDesc const *cls = OBJ_FindClassW(class_id);
  if (!cls) {
    Con_Error("Class ID 0x%08x not found\n", class_id);
    return NULL;
  }
  uint32_t clsSize = sizeof(struct component) + cls->ClassSize;
  uint32_t propsSize = cls->NumProperties * sizeof(void*);
  struct component* comp = ZeroAlloc(clsSize + propsSize);

  assert(comp);

  if (cls->Defaults) {
    memcpy(comp->pUserData, cls->Defaults, cls->ClassSize);
  }

  comp->pcls = cls;
  comp->pobj = pobj;
  
  ADD_TO_LIST_END(struct component, comp, pobj->components);

  for (uint32_t const* p = cls->ParentClasses; *p; OBJ_AddComponent(pobj, *(p++)));

  return comp;
}

struct Object *
CMP_GetOwner(struct component* hcmp)
{
  return hcmp->pobj;
}

struct Property *
_CreateClassProperty(struct Object *object, uint32_t ident)
{
  FOR_EACH_LIST(struct component, cmp, object->components)
  {
    FOR_LOOP(i, cmp->pcls->NumProperties)
    {
      struct PropertyType const *pdesc = &cmp->pcls->Properties[i];
      if (pdesc->ShortIdentifier == ident ||
          pdesc->FullIdentifier == ident)
      {
        return OBJ_AddComponentProperty(NULL, cmp, pdesc);
      }
    }
  }
  return NULL;
}

struct PropertyType const *
OBJ_FindImplicitProperty(struct Object *object, lpcString_t name)
{
  uint32_t identifier = fnv1a32(name);
//  return PROP_FindByShortID(object->properties, identifier);
  FOR_EACH_LIST(struct component, cmp, object->components) {
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      struct PropertyType const *pdesc = &cmp->pcls->Properties[i];
      if (pdesc->ShortIdentifier == identifier) {
        return (struct PropertyType const *)pdesc;
      }
    }
  }
  return NULL;
}

struct PropertyType const *
OBJ_FindExplicitProperty(struct Object *object, lpcString_t name)
{
  uint32_t identifier = fnv1a32(name);
  //  return PROP_FindByShortID(object->properties, identifier);
  FOR_EACH_LIST(struct component, cmp, object->components) {
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      struct PropertyType const *pdesc = &cmp->pcls->Properties[i];
      if (pdesc->FullIdentifier == identifier) {
        return (struct PropertyType const *)pdesc;
      }
    }
  }
  return OBJ_FindPropertyType(identifier);
}

void
OBJ_EnumClassProperties(struct Object *object,
                        void (*fnProc)(struct Object const *,
                                       struct PropertyType const *,
                                       struct ClassDesc const *cdesc,
                                       void const*,
                                       void*),
                        void* parm)
{
  FOR_EACH_LIST(struct component, cmp, object->components) {
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      struct PropertyType const *pdesc = &cmp->pcls->Properties[i];
      fnProc(object, pdesc, cmp->pcls, cmp->pUserData+pdesc->Offset, parm);
    }
  }
}

bool_t
CMP_SetProperty(struct component* comp, struct Property *property)
{
  FOR_LOOP(index, comp->pcls->NumProperties)
  {
    struct PropertyType const *pdesc = comp->pcls->Properties + index;
    if (PROP_GetLongIdentifier(property) == pdesc->FullIdentifier) {
      struct Property ** properties = (void*)(comp->pUserData + comp->pcls->ClassSize);
      properties[index] = property;
      PROP_SetFlag(property, PF_PROPERTY_TYPE);
      PROP_SetValuePtr(property, comp->pUserData + pdesc->Offset);
      return TRUE;
    }
  }
  return comp->next && CMP_SetProperty(comp->next, property);
}

lpcString_t
CMP_GetClassName(struct component* hcmp)
{
  return hcmp->pcls->ClassName;
}

static bool_t
_IsNodeTriggerMessage(uint32_t MsgID)
{
  switch (MsgID) {
    case ID_Node_LeftButtonDown:
    case ID_Node_RightButtonDown:
    case ID_Node_OtherButtonDown:
    case ID_Node_LeftButtonUp:
    case ID_Node_RightButtonUp:
    case ID_Node_OtherButtonUp:
    case ID_Node_LeftButtonDragged:
    case ID_Node_RightButtonDragged:
    case ID_Node_OtherButtonDragged:
    case ID_Node_LeftDoubleClick:
    case ID_Node_RightDoubleClick:
    case ID_Node_OtherDoubleClick:
    case ID_Node_MouseMoved:
    case ID_Node_ScrollWheel:
    case ID_Node_DragDrop:
    case ID_Node_DragEnter:
    case ID_Node_SetFocus:
    case ID_Node_KillFocus:
    case ID_Node_KeyDown:
    case ID_Node_KeyUp:
    case ID_Node_TextInput:
      return TRUE;
    default:
      return FALSE;
  }
}

static lpcString_t
_NodeTriggerMessageName(uint32_t MsgID)
{
  switch (MsgID) {
    case ID_Node_LeftButtonDown: return "Node.LeftButtonDown";
    case ID_Node_RightButtonDown: return "Node.RightButtonDown";
    case ID_Node_OtherButtonDown: return "Node.OtherButtonDown";
    case ID_Node_LeftButtonUp: return "Node.LeftButtonUp";
    case ID_Node_RightButtonUp: return "Node.RightButtonUp";
    case ID_Node_OtherButtonUp: return "Node.OtherButtonUp";
    case ID_Node_LeftButtonDragged: return "Node.LeftButtonDragged";
    case ID_Node_RightButtonDragged: return "Node.RightButtonDragged";
    case ID_Node_OtherButtonDragged: return "Node.OtherButtonDragged";
    case ID_Node_LeftDoubleClick: return "Node.LeftDoubleClick";
    case ID_Node_RightDoubleClick: return "Node.RightDoubleClick";
    case ID_Node_OtherDoubleClick: return "Node.OtherDoubleClick";
    case ID_Node_MouseMoved: return "Node.MouseMoved";
    case ID_Node_ScrollWheel: return "Node.ScrollWheel";
    case ID_Node_DragDrop: return "Node.DragDrop";
    case ID_Node_DragEnter: return "Node.DragEnter";
    case ID_Node_SetFocus: return "Node.SetFocus";
    case ID_Node_KillFocus: return "Node.KillFocus";
    case ID_Node_KeyDown: return "Node.KeyDown";
    case ID_Node_KeyUp: return "Node.KeyUp";
    case ID_Node_TextInput: return "Node.TextInput";
    default:
      return NULL;
  }
}

static struct Object *
_NodeTriggerSender(struct Object *receiver, uint32_t MsgID, wParam_t wParam, lParam_t lParam)
{
  (void)MsgID;
  (void)wParam;
  (void)lParam;
  return receiver;
}

static LRESULT
_FireRoutedTrigger(struct Object *hObject, lpcString_t configured_event, lpcString_t routed_event, struct Object *sender)
{
  if (!configured_event || !*configured_event || strcmp(configured_event, routed_event)) {
    return FALSE;
  }
  return _SendMessage(hObject, Trigger, Triggered,
                .Sender = sender ? sender : hObject);
}

static LRESULT
_DispatchEventTriggerSpecialCase(struct Object *receiver,
                                 uint32_t MsgID,
                                 wParam_t wParam,
                                 lParam_t lParam,
                                 struct Object *sender_override)
{
  lpcString_t routed_event = _NodeTriggerMessageName(MsgID);
  if (!routed_event) {
    return FALSE;
  }
  struct Object *sender = sender_override ? sender_override : _NodeTriggerSender(receiver, MsgID, wParam, lParam);

  struct EventTrigger const *event_trigger = GetEventTrigger(receiver);
  if (event_trigger && _FireRoutedTrigger(receiver, event_trigger->RoutedEvent, routed_event, sender)) {
    return TRUE;
  }
  return FALSE;
}

static LRESULT
_DispatchNodeTriggers(struct Object *node_object, uint32_t MsgID, wParam_t wParam, lParam_t lParam)
{
  // This may be called for any object receiving a Node.* message.
  // Only objects with an attached Node component can host Node.Triggers.
  struct Node *node = GetNode(node_object);
  if (!node || node->NumTriggers <= 0 || !node->Triggers) {
    return FALSE;
  }

  lParam_t trigger_param = lParam;
  struct Object **triggers = (struct Object **)node->Triggers;
  // Trigger arrays may contain NULL holes (e.g. sparse/partially initialized slots).
  FOR_LOOP(i, node->NumTriggers) {
    struct Object *trigger = triggers[i];
    if (!trigger) {
      continue;
    }
    LRESULT handled = _DispatchEventTriggerSpecialCase(trigger,
                                                       MsgID,
                                                       wParam,
                                                       trigger_param,
                                                       node_object);
    if (handled) {
      return handled;
    }
  }
  return FALSE;
}

LRESULT
OBJ_SendMessageW(struct Object *pobj, uint32_t MsgID, wParam_t wParam, lParam_t lParam)
{
  if (MsgID == ID_Object_Animate) {
    OBJ_SetFlags(pobj, OBJ_GetFlags(pobj) & ~OF_ANIMATE_QUEUED);
  }
//#ifndef KANZI_SUPPORT
//	if (MsgID == kMsgUpdateLayout && !(OBJ_GetFlags(pobj) & OF_DIRTY))
//		return FALSE;
//#endif
  for (struct component *cmp = pobj->components; cmp; ) {
    struct component *next = cmp->next;
    if (cmp->pcls->ObjProc) {
      LRESULT res = cmp->pcls->ObjProc(pobj, cmp->pUserData, MsgID, wParam, lParam);
      if (res) {
        return res;
      }
    }
    cmp = next;
  }
  if (_IsNodeTriggerMessage(MsgID)) {
    LRESULT handled = _DispatchEventTriggerSpecialCase(pobj, MsgID, wParam, lParam, NULL);
    if (handled) {
      return handled;
    }
    return _DispatchNodeTriggers(pobj, MsgID, wParam, lParam);
  }
  return FALSE;
}

LRESULT
OBJ_SendMessage(struct Object *pobj, lpcString_t Msg, wParam_t wParam, lParam_t lParam)
{
  return OBJ_SendMessageW(pobj, fnv1a32(Msg), wParam, lParam);
}

void
OBJ_RequestAnimate(struct Object *pobj)
{
  if (!pobj) return;
  uint32_t flags = OBJ_GetFlags(pobj);
  if (flags & OF_ANIMATE_QUEUED) return;
  OBJ_SetFlags(pobj, flags | OF_ANIMATE_QUEUED);
  axPostMessageW(pobj, ID_Object_Animate, 0, NULL);
}

void*
CMP_GetUserData(struct component* component)
{
  return component->pUserData;
}

void*
OBJ_GetComponent(struct Object *pobj, uint32_t id)
{
  FOR_EACH_LIST(struct component, cmp, pobj->components)
  {
    if (cmp->pcls->ClassID == id) {
      return cmp->pUserData;
    }
  }
  return NULL;
}

void
OBJ_ReleaseComponents(struct Object *pobj)
{
  FOR_EACH_LIST(struct component, cmp, pobj->components)
  {
    free(cmp);
  }
}

void
CMP_Detach(void* userdata)
{
  if (!userdata) return;
  // pUserData is a flexible array member at the end of struct component.
  // Subtract its offset to recover the containing struct component pointer.
  struct component *self = (struct component*)((char*)userdata - offsetof(struct component, pUserData));
  struct Object *pobj = self->pobj;
  REMOVE_FROM_LIST(struct component, self, pobj->components);
  free(self);
}

struct Object *
CMP_GetObject(void const* udata)
{
  if (udata == NULL) {
    return NULL;
  }
  struct component const *cmp = udata - offsetof(struct component, pUserData);
  return cmp->pobj;
}

void
OBJ_EnumObjectClasses(struct Object *pobj,
                      void (*fnProc)(struct ClassDesc const *, void*),
                      void* param)
{
  FOR_EACH_LIST(struct component, cmp, pobj->components) {
    fnProc(cmp->pcls, param);
  }
}

extern int write_property(lua_State *L, struct PropertyType const* prop, void const* valueptr);

bool_t
OBJ_PushClassProperty(lua_State *L, struct Object *object, uint32_t id)
{
  FOR_EACH_LIST(struct component, cmp, object->components) {
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      struct PropertyType const *pdesc = &cmp->pcls->Properties[i];
      if (pdesc->ShortIdentifier == id) {
        write_property(L, pdesc, cmp->pUserData + pdesc->Offset);
        return TRUE;
      }
    }
  }
  return FALSE;
}


struct PropertyType const *
MSG_FindByShortID(struct Object *obj, uint32_t ident)
{
  FOR_EACH_LIST(struct component, cmp, obj->components) {
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      struct PropertyType const *msg = &cmp->pcls->Properties[i];
      if (msg->DataType == kDataTypeEvent && msg->ShortIdentifier == ident) {
        return msg;
      }
    }
  }
  return 0;
}

bool_t
OBJ_IsPrefabView(struct Object const *object)
{
  FOR_EACH_LIST(struct component, cmp, object ? object->components : NULL) {
    switch (cmp->pcls->ClassID) {
      case 0xe741d328: /* ID_PrefabView2D */
      case 0x8d451711: /* ID_PrefabView3D */
        return TRUE;
    }
  }
  return FALSE;
}
