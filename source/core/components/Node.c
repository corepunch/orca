#include <source/core/core_local.h>

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
    lpObject_t aliasedobj = OBJ_FindByPath(hObject, alias->Value);
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
