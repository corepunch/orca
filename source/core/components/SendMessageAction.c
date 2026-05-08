#include <source/core/core_local.h>
#include <source/core/core_properties.h>

HANDLER(SendMessageAction, Object, Attached)
{
  _BindActionTrigger(hObject, NULL);
  return FALSE;
}

HANDLER(SendMessageAction, Trigger, Triggered)
{
  if (!_TriggerMatches(pSendMessageAction->Trigger, pTriggered)) {
    return FALSE;
  }

  if (!pSendMessageAction->Message || !*pSendMessageAction->Message) {
    Con_Error("SendMessageAction missing Message property");
    return FALSE;
  }

  // Resolve target object
  struct Object *sender = _TriggerSender(hObject, pTriggered);
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
  uint32_t msg_id = FNV1a(pSendMessageAction->Message);
  if (!msg_id) {
    Con_Error("SendMessageAction invalid message name '%s'", 
              pSendMessageAction->Message);
    return FALSE;
  }

  // Get message field property types
  uint32_t field_count = 0;
  struct PropertyType* fields = OBJ_FindMessagePropertyTypes(pSendMessageAction->Message, &field_count);
  
  if (!fields || field_count == 0) {
    // Message has no arguments, send it directly
    OBJ_SendMessage(target, msg_id, 0, 0);
    return FALSE;
  }

  // Build message payload from attached properties
  char payload[MAX_MESSAGE_SIZE];
  memset(payload, 0, sizeof(payload));
  
  for (uint32_t i = 0; i < field_count; i++) {
    struct PropertyType const* field = &fields[i];
    
    // Look for attached property with the field name
    struct Property* prop = OBJ_FindShortProperty(hObject, field->Name);
    if (prop) {
      void const* value = PROP_GetValue(prop);
      if (value) {
        // Copy value to payload at the correct offset
        size_t copy_size = field->Size;
        if (copy_size > 0 && field->Offset + copy_size <= MAX_MESSAGE_SIZE) {
          memcpy(payload + field->Offset, value, copy_size);
        }
      }
    }
  }

  // Send the message with the constructed payload
  OBJ_SendMessage(target, msg_id, 0, (lParam_t)payload);
  
  return FALSE;
}
