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
  PROP_SetValue(prop, &trigger);
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

  // Resolve target object
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

  // Parse message name to get ID
  uint32_t msg_id = fnv1a32(pSendMessageAction->Message);

  // Get message field property types
  uint32_t field_count = 0;
  struct PropertyType const* fields = OBJ_FindMessagePropertyTypes(pSendMessageAction->Message, &field_count);
  
  if (!fields || field_count == 0) {
    // Message has no arguments, send it directly
    OBJ_SendMessageW(target, msg_id, 0, 0);
    return FALSE;
  }

  // Build message payload from attached properties
  char payload[MAX_MESSAGE_SIZE];
  memset(payload, 0, sizeof(payload));
  
  for (uint32_t i = 0; i < field_count; i++) {
    struct PropertyType const* field = &fields[i];
    
    // Look for attached property with the field name
    struct Property *prop = NULL;
    if (SUCCEEDED(OBJ_FindShortProperty(hObject, field->Name, &prop)) && prop) {
      void const* value = PROP_GetValue(prop);
      if (value) {
        // Copy value to payload at the correct offset
        size_t copy_size = field->DataSize;
        if (copy_size > 0 && field->Offset + copy_size <= MAX_MESSAGE_SIZE) {
          memcpy(payload + field->Offset, value, copy_size);
        }
      }
    }
  }

  // Send the message with the constructed payload
  OBJ_SendMessageW(target, msg_id, 0, (lParam_t)payload);
  
  return FALSE;
}
