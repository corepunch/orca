#include <source/core/core_local.h>

static LRESULT
_DispatchTriggers(struct Object *sender, uint32_t message, wParam_t wParam, lParam_t lParam)
{
  struct Node *node = GetNode(sender);
  if (!node || !node->Triggers || node->NumTriggers <= 0) {
    return FALSE;
  }
  LRESULT r = FALSE;
  FOR_LOOP(i, node->NumTriggers) {
    struct Object *trigger = node->Triggers[i];
    if (trigger) {
      r |= OBJ_SendMessageW(trigger, message, wParam, lParam);
    }
  }
  return r;
}

HANDLER(Node, Node, GetSize)
{
  return MAKEDWORD(pNode->Size.Axis[0].Actual, pNode->Size.Axis[1].Actual);
}

HANDLER(Node, Node, IsVisible)
{
  PROP_Update(Node_GetProperty(hObject, kNodeVisible));
  return pNode->Visible && !GetNode(hObject)->QuickHide;
}

HANDLER(Node, Object, Start)
{
  FOR_LOOP(i, pNode->NumResources) {
    struct ResourceEntry* alias = &pNode->Resources[i];
    if (!alias->Value || !*alias->Value || !alias->Key || !*alias->Key)
      continue;
    struct Object *aliasedobj = OBJ_FindByPath(hObject, alias->Value);
    if (!aliasedobj)
      continue;
    if (OBJ_GetAlias(aliasedobj)) {
      Con_Error("Erasing alias to %s from object %s\n", OBJ_GetName(aliasedobj), OBJ_GetName(hObject));
      continue;
    }
    OBJ_SetAlias(aliasedobj, fnv1a32(alias->Key));
  }
  return FALSE;
}

HANDLER(Node, Object, PropertyChanged)
{
  return FALSE;
}

HANDLER(Node, Object, Attached)
{
  return FALSE;
}

#define NODE_TRIGGER_MOUSE_HANDLER(EVENT) \
HANDLER(Node, Node, EVENT) \
{ \
  struct Node_MouseMessageEventArgs local_args = {0}; \
  if (p##EVENT) local_args = *p##EVENT; \
  local_args.Sender = hObject; \
  return _DispatchTriggers(hObject, ID_Node_##EVENT, 0, &local_args); \
}

#define NODE_TRIGGER_KEY_HANDLER(EVENT) \
HANDLER(Node, Node, EVENT) \
{ \
  struct Node_KeyMessageEventArgs local_args = {0}; \
  if (p##EVENT) local_args = *p##EVENT; \
  return _DispatchTriggers(hObject, ID_Node_##EVENT, 0, &local_args); \
}

#define NODE_TRIGGER_HANDLER(EVENT) \
HANDLER(Node, Node, EVENT) \
{ \
  return _DispatchTriggers(hObject, ID_Node_##EVENT, 0, p##EVENT); \
}

NODE_TRIGGER_MOUSE_HANDLER(LeftButtonDown)
NODE_TRIGGER_MOUSE_HANDLER(RightButtonDown)
NODE_TRIGGER_MOUSE_HANDLER(OtherButtonDown)
NODE_TRIGGER_MOUSE_HANDLER(LeftButtonUp)
NODE_TRIGGER_MOUSE_HANDLER(RightButtonUp)
NODE_TRIGGER_MOUSE_HANDLER(OtherButtonUp)
NODE_TRIGGER_MOUSE_HANDLER(LeftButtonDragged)
NODE_TRIGGER_MOUSE_HANDLER(RightButtonDragged)
NODE_TRIGGER_MOUSE_HANDLER(OtherButtonDragged)
NODE_TRIGGER_MOUSE_HANDLER(LeftDoubleClick)
NODE_TRIGGER_MOUSE_HANDLER(RightDoubleClick)
NODE_TRIGGER_MOUSE_HANDLER(OtherDoubleClick)
NODE_TRIGGER_MOUSE_HANDLER(MouseMoved)
NODE_TRIGGER_MOUSE_HANDLER(ScrollWheel)
NODE_TRIGGER_MOUSE_HANDLER(DragDrop)
NODE_TRIGGER_MOUSE_HANDLER(DragEnter)
NODE_TRIGGER_HANDLER(SetFocus)
NODE_TRIGGER_HANDLER(KillFocus)
NODE_TRIGGER_KEY_HANDLER(KeyDown)
NODE_TRIGGER_KEY_HANDLER(KeyUp)
NODE_TRIGGER_KEY_HANDLER(TextInput)
