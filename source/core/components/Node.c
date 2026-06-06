#include <source/core/core_local.h>

#include <math.h>

HANDLER(Node, Node, GetSize)
{
  return MAKEDWORD(pNode->Size.Axis[0].Actual, pNode->Size.Axis[1].Actual);
}

HANDLER(Node, Node, IsVisible)
{
  struct Property *visible = NULL;
  if (SUCCEEDED(OBJ_FindLongProperty(hObject, ID_Node_Visible, &visible))) {
    PROP_Update(visible);
  }
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
  if (!pPropertyChanged || !pPropertyChanged->Property) {
    return FALSE;
  }

  /* Compatibility helper layer for old Horizontal/Vertical/DepthAlignment
   * properties. These are not canonical layout inputs and may be removed once
   * callers migrate to Width/Height/Depth = NaN and Margin* = NaN alignment. */
  static uint32_t const size_ids[] = {
    ID_Node_Width,
    ID_Node_Height,
    ID_Node_Depth,
  };
  static uint32_t const leading_margin_ids[] = {
    ID_Node_MarginLeft,
    ID_Node_MarginTop,
    ID_Node_MarginFront,
  };
  static uint32_t const trailing_margin_ids[] = {
    ID_Node_MarginRight,
    ID_Node_MarginBottom,
    ID_Node_MarginBack,
  };
  static uint32_t const alignment_ids[] = {
    ID_Node_HorizontalAlignment,
    ID_Node_VerticalAlignment,
    ID_Node_DepthAlignment,
  };

  uint32_t changed = PROP_GetLongIdentifier(pPropertyChanged->Property);
  for (int axis = 0; axis < 3; axis++) {
    if (changed != alignment_ids[axis]) {
      continue;
    }

    int alignment = *(int *)PROP_GetValue(pPropertyChanged->Property);
    float zero = 0;
    float auto_value = NAN;
    struct Property *property = NULL;

    switch (alignment) {
      case 1: /* Center */
        if (SUCCEEDED(OBJ_FindLongProperty(hObject, leading_margin_ids[axis], &property))) {
          PROP_SetValue(property, &auto_value);
        }
        if (SUCCEEDED(OBJ_FindLongProperty(hObject, trailing_margin_ids[axis], &property))) {
          PROP_SetValue(property, &auto_value);
        }
        break;
      case 2: /* Right, Bottom, Back */
        if (SUCCEEDED(OBJ_FindLongProperty(hObject, leading_margin_ids[axis], &property))) {
          PROP_SetValue(property, &auto_value);
        }
        if (SUCCEEDED(OBJ_FindLongProperty(hObject, trailing_margin_ids[axis], &property))) {
          PROP_SetValue(property, &zero);
        }
        break;
      case 3: /* Stretch */
        if (SUCCEEDED(OBJ_FindLongProperty(hObject, size_ids[axis], &property))) {
          PROP_SetValue(property, &auto_value);
        }
        if (SUCCEEDED(OBJ_FindLongProperty(hObject, leading_margin_ids[axis], &property))) {
          PROP_SetValue(property, &zero);
        }
        if (SUCCEEDED(OBJ_FindLongProperty(hObject, trailing_margin_ids[axis], &property))) {
          PROP_SetValue(property, &zero);
        }
        break;
      case 0: /* Left, Top, Front */
      default:
        if (SUCCEEDED(OBJ_FindLongProperty(hObject, leading_margin_ids[axis], &property))) {
          PROP_SetValue(property, &zero);
        }
        if (SUCCEEDED(OBJ_FindLongProperty(hObject, trailing_margin_ids[axis], &property))) {
          PROP_SetValue(property, &auto_value);
        }
        break;
    }

    return FALSE;
  }
  return FALSE;
}

HANDLER(Node, Object, Attached)
{
  return FALSE;
}
