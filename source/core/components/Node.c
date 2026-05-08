#include <source/core/core_local.h>

static void
_DispatchTriggers(struct Object *sender, uint32_t message, wParam_t wParam, lParam_t lParam)
{
  struct Node *node = GetNode(sender);
  if (!node || !node->Triggers || node->NumTriggers <= 0) {
    return;
  }
  FOR_LOOP(i, node->NumTriggers) {
    struct Object *trigger = node->Triggers[i];
    if (trigger) {
      OBJ_SendMessageW(trigger, message, wParam, lParam);
    }
  }
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

HANDLER(Node, Node, LeftButtonUp)
{
  struct Node_MouseMessageEventArgs args = *pLeftButtonUp;
  args.Sender = hObject;
  _DispatchTriggers(hObject, ID_Node_LeftButtonUp, 0, &args);
  return FALSE;
}
