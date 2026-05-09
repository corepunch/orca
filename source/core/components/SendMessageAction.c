#include <source/core/core_local.h>
#include <source/core/core_properties.h>

static bool_t
_SendMessageAction_Matches(struct Trigger const* expected, struct Trigger_TriggeredEventArgs const* triggered)
{
  return !expected || (triggered && triggered->Trigger == expected);
}

static struct Object *
_SendMessageAction_Sender(struct Object *hObject, struct Trigger_TriggeredEventArgs const* triggered)
{
  return (triggered && triggered->Sender) ? triggered->Sender : hObject;
}

static void
_SendMessageAction_BindTrigger(struct Object *hObject)
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
  if (!trigger || FAILED(OBJ_FindShortProperty(hObject, "Trigger", &prop))) {
    return;
  }
  struct Object *trigger_object = CMP_GetObject(trigger);
  PROP_SetValue(prop, &trigger_object);
}

HANDLER(SendMessageAction, Object, Attached)
{
  _SendMessageAction_BindTrigger(hObject);
  return FALSE;
}

HANDLER(SendMessageAction, Trigger, Triggered)
{
  if (!_SendMessageAction_Matches(pSendMessageAction->Trigger, pTriggered)) {
    return FALSE;
  }

  if (!pSendMessageAction->Message || !*pSendMessageAction->Message) {
    Con_Error("SendMessageAction missing Message property");
    return FALSE;
  }

  struct Object *sender = _SendMessageAction_Sender(hObject, pTriggered);
  struct Object *target = sender;

  if (pSendMessageAction->Target && *pSendMessageAction->Target) {
    target = OBJ_FindByPath(sender, pSendMessageAction->Target);
    if (!target) {
      Con_Error("SendMessageAction could not resolve target path '%s'",
                pSendMessageAction->Target);
      return FALSE;
    }
  }

  uint32_t msg_id = fnv1a32(pSendMessageAction->Message);
  uint32_t field_count = 0;
  struct PropertyType const *fields = OBJ_FindMessagePropertyTypes(pSendMessageAction->Message, &field_count);

  if (!fields || field_count == 0) {
    axPostMessageW(target, msg_id, 0, NULL);
    return FALSE;
  }

  char payload[MAX_MESSAGE_SIZE];
  memset(payload, 0, sizeof(payload));
  size_t payload_size = 0;

  for (uint32_t i = 0; i < field_count; i++) {
    struct PropertyType const *field = &fields[i];
    if (field->Offset >= MAX_MESSAGE_SIZE) {
      Con_Error("SendMessageAction field '%s' has invalid message definition offset %zu (limit %u)",
                field->Name ? field->Name : "<unnamed>",
                (size_t)field->Offset,
                (unsigned)MAX_MESSAGE_SIZE);
      continue;
    }
    size_t max_copy_size = MAX_MESSAGE_SIZE - field->Offset;
    size_t copy_size = (size_t)field->DataSize;
    if (copy_size > max_copy_size) {
      Con_Error("SendMessageAction field '%s' size %zu exceeds payload capacity %zu; skipping field",
                field->Name ? field->Name : "<unnamed>",
                copy_size,
                max_copy_size);
      continue;
    }

    size_t field_end = field->Offset + copy_size;
    if (field_end > payload_size) {
      payload_size = field_end;
    }

    struct Property *prop = NULL;
    if (SUCCEEDED(OBJ_FindShortProperty(hObject, field->Name, &prop)) && prop) {
      void const *value = PROP_GetValue(prop);
      if (value && copy_size > 0) {
        memcpy(payload + field->Offset, value, copy_size);
      }
    }
  }

  if (payload_size > MAX_MESSAGE_SIZE) {
    Con_Error("SendMessageAction payload for '%s' exceeded limit (%zu > %u) and was clamped",
              pSendMessageAction->Message,
              payload_size,
              (unsigned)MAX_MESSAGE_SIZE);
    payload_size = MAX_MESSAGE_SIZE;
  }
  axPostMessageDataW(target, msg_id, 0, payload, payload_size);
  return FALSE;
}
