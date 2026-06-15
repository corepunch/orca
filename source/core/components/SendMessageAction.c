#include <source/core/core_local.h>
#include <source/core/object/object_internal.h>

static struct Object *
_SendMessageActionSender(struct Object *hObject,
                         struct Action_DispatchEventArgs const* dispatch)
{
  return (dispatch && dispatch->Sender) ? dispatch->Sender : hObject;
}

static bool_t
_ClassInherits(struct ClassDesc const *cls, uint32_t parent)
{
  if (!cls) return FALSE;
  if (!parent || cls->ClassID == parent) return TRUE;
  for (uint32_t const *id = cls->ParentClasses; *id; id++) {
    if (_ClassInherits(OBJ_FindClassW(*id), parent)) {
      return TRUE;
    }
  }
  return FALSE;
}

static struct component *
_FindMessageActionPayload(struct Object *action)
{
  FOR_EACH_LIST(struct component, cmp, action ? action->components : NULL) {
    if (cmp->pcls->ClassID == ID_SendMessageAction) {
      continue;
    }
    if (_ClassInherits(cmp->pcls, ID_SendMessageAction)) {
      return cmp;
    }
  }
  return NULL;
}

// SendMessageAction_Dispatch
HANDLER(SendMessageAction, Action, Dispatch)
{
  struct component *payload_cmp = _FindMessageActionPayload(hObject);
  if (!payload_cmp) {
    Con_Error("SendMessageAction could not find a generated message payload");
    return FALSE;
  }

  struct Object *sender = _SendMessageActionSender(hObject, pDispatch);
  struct Object *target = sender;
  bool_t explicit_target = FALSE;
  if (pSendMessageAction->Target && *pSendMessageAction->Target) {
    explicit_target = TRUE;
    target = OBJ_FindByPath(sender, pSendMessageAction->Target);
    if (!target) {
      Con_Error("%s could not resolve target path '%s'",
                payload_cmp->pcls->ClassName,
                pSendMessageAction->Target);
      return FALSE;
    }
  }

  uint32_t message = payload_cmp->pcls->ClassID;
  bool_t has_payload = payload_cmp->pcls->NumProperties > 0;
  lParam_t payload = has_payload ? payload_cmp->pUserData : NULL;

  if (pSendMessageAction->Mode == kDispatchModePost) {
    if (has_payload) {
      axPostMessageDataW(target, message, 0, payload, payload_cmp->pcls->ClassSize);
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
