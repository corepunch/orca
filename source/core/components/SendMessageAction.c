#include <source/core/core_local.h>
#include <source/core/object/object_internal.h>

static struct Object *
_SendMessageActionSender(struct Object *hObject,
                         struct Action_DispatchEventArgs const* dispatch)
{
  return (dispatch && dispatch->Sender) ? dispatch->Sender : hObject;
}

HANDLER(SendMessageAction, Object, Create)
{
  /* Record the concrete class ID and payload size so Dispatch doesn't need
   * to walk the component list. The object's type IS the concrete action class
   * (e.g. Button_ClickAction); SendMessageAction is its parent. */
  struct ClassDesc const *concrete = hObject->type;
  if (concrete && concrete->ClassID != ID_SendMessageAction) {
    pSendMessageAction->_payload_class_id = concrete->ClassID;
    pSendMessageAction->_payload_size     = concrete->ClassSize;
  }
  return FALSE;
}

HANDLER(SendMessageAction, Action, Dispatch)
{
  uint32_t message   = pSendMessageAction->_payload_class_id;
  uint32_t psize     = pSendMessageAction->_payload_size;
  bool_t has_payload = psize > 0;

  if (!message) {
    Con_Error("SendMessageAction: payload class not set — was Object.Create sent?");
    return FALSE;
  }

  /* The payload data lives in the object's own typedata or component userdata
   * at offset 0 of the concrete class's slot. For Action-family objects that
   * aren't yet in a storage family we fall back to the component list. */
  lParam_t payload = NULL;
  if (has_payload) {
    if (OBJ_UsesTypedata(hObject)) {
      payload = (lParam_t)(hObject->typedata +
                           (hObject->type ? hObject->type->TypedataOffset : 0));
    } else {
      /* Fallback: find concrete data in component list */
      FOR_EACH_LIST(struct component, cmp, hObject->animations) {
        if (cmp->pcls->ClassID == message) {
          payload = (lParam_t)cmp->pUserData;
          break;
        }
      }
    }
  }

  struct Object *sender = _SendMessageActionSender(hObject, pDispatch);
  struct Object *target = sender;
  bool_t explicit_target = FALSE;

  if (pSendMessageAction->Target && *pSendMessageAction->Target) {
    explicit_target = TRUE;
    target = OBJ_FindByPath(sender, pSendMessageAction->Target);
    if (!target) {
      Con_Error("%s could not resolve target path '%s'",
                hObject->type ? hObject->type->ClassName : "SendMessageAction",
                pSendMessageAction->Target);
      return FALSE;
    }
  }

  if (pSendMessageAction->Mode == kDispatchModePost) {
    if (has_payload) {
      axPostMessageDataW(target, message, 0, payload, psize);
    } else {
      axPostMessageW(target, message, 0, NULL);
    }
    return TRUE;
  }

  if (!explicit_target) {
    for (struct Object *receiver = target; receiver; receiver = OBJ_GetParent(receiver)) {
      LRESULT result = OBJ_SendMessageW(receiver, message, 0, payload);
      if (result) return result;
    }
    return FALSE;
  }

  return OBJ_SendMessageW(target, message, 0, payload);
}
