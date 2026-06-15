#include <source/core/core_local.h>

#include <math.h>

static void Node_SetFloatProperty(struct Object *object, uint32_t id, float value)
{
  struct Property *property = OBJ_FindLongProperty(object, id);
  if (property) PROP_SetValue(property, &value);
}

// Node_GetSize
HANDLER(Node, Node, GetSize)
{
  return MAKEDWORD(pNode->Size.Axis[0].Actual, pNode->Size.Axis[1].Actual);
}

// Node_IsVisible
HANDLER(Node, Node, IsVisible)
{
  struct Property *visible = OBJ_FindLongProperty(hObject, ID_Node_Visible);
  if (visible) {
    PROP_Update(visible);
  }
  return pNode->Visible && !GetNode(hObject)->QuickHide;
}

// Node_Start
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

// Node_PropertyChanged
HANDLER(Node, Object, PropertyChanged)
{
  if (!pPropertyChanged || !pPropertyChanged->Property) return FALSE;

  /* Compatibility helper: old *Alignment props translate to Width/Height/Depth
   * and Margin* NaN semantics. Not canonical layout input; may be removed. */
  static uint32_t const ids[3][4] = {
    {ID_Node_HorizontalAlignment, ID_Node_Width,  ID_Node_MarginLeft,  ID_Node_MarginRight},
    {ID_Node_VerticalAlignment,   ID_Node_Height, ID_Node_MarginTop,   ID_Node_MarginBottom},
    {ID_Node_DepthAlignment,      ID_Node_Depth,  ID_Node_MarginFront, ID_Node_MarginBack},
  };
  uint32_t const changed = PROP_GetLongIdentifier(pPropertyChanged->Property);
  for (int axis = 0; axis < 3; axis++) {
    if (changed != ids[axis][0]) continue;
    int alignment = *(int *)PROP_GetValue(pPropertyChanged->Property);
    if (alignment < 0 || alignment > 3) alignment = 0;
    if (alignment == 3) Node_SetFloatProperty(hObject, ids[axis][1], NAN);
    Node_SetFloatProperty(hObject, ids[axis][2], (alignment == 1 || alignment == 2) ? NAN : 0);
    Node_SetFloatProperty(hObject, ids[axis][3], (alignment == 0 || alignment == 1) ? NAN : 0);
    return FALSE;
  }
  return FALSE;
}

// Node_Attached
HANDLER(Node, Object, Attached)
{
  return FALSE;
}
