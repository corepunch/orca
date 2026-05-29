#include <source/core/core_local.h>
#include <source/core/core_properties.h>

static bool_t
_SendMessageActionPayload(struct Object *source,
                          lpcString_t message,
                          char payload[MAX_MESSAGE_SIZE],
                          size_t *payload_size)
{
  uint32_t count = 0;
  struct PropertyType const *fields = OBJ_FindMessagePropertyTypes(message, &count);
  if (!fields || count == 0) {
    *payload_size = 0;
    return FALSE;
  }

  memset(payload, 0, MAX_MESSAGE_SIZE);
  *payload_size = 0;
  FOR_LOOP(i, count) {
    struct PropertyType const *field = &fields[i];
    size_t field_end = field->Offset + field->DataSize;
    if (field->Offset >= MAX_MESSAGE_SIZE || field_end > MAX_MESSAGE_SIZE) {
      Con_Error("SendMessageAction field '%s' exceeds payload capacity for '%s'",
                field->Name ? field->Name : "<unnamed>",
                message);
      continue;
    }
    if (field_end > *payload_size) {
      *payload_size = field_end;
    }
    struct Property *prop = NULL;
    if (SUCCEEDED(OBJ_FindShortProperty(source, field->Name, &prop)) && prop) {
      void const *value = PROP_GetValue(prop);
      if (value && field->DataSize > 0) {
        memcpy(payload + field->Offset, value, field->DataSize);
      }
    }
  }
  return TRUE;
}

static bool_t
_SendMessageAction_Matches(struct Trigger const* expected, struct Trigger_TriggeredEventArgs const* triggered)
{
  (void)expected;
  return triggered != NULL;
}

static struct Object *
_SendMessageAction_Sender(struct Object *hObject, struct Trigger_TriggeredEventArgs const* triggered)
{
  return (triggered && triggered->Sender) ? triggered->Sender : hObject;
}

HANDLER(SendMessageAction, Trigger, Triggered)
{
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
  char payload[MAX_MESSAGE_SIZE];
  size_t payload_size = 0;

  if (!_SendMessageActionPayload(hObject,
                                 pSendMessageAction->Message,
                                 payload,
                                 &payload_size)) {
    axPostMessageW(target, msg_id, 0, NULL);
    return TRUE;
  }

  axPostMessageDataW(target, msg_id, 0, payload, payload_size);
  return TRUE;
}
