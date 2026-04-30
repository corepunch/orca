#include <include/orca.h>
#include <source/core/core_local.h>

#include <plugins/UIKit/UIKit.h>

static void
RadioButton_SetChecked(struct Object *object,
                       struct RadioButton *button,
                       bool_t checked)
{
  if (!object || !button)
    return;

  button->IsChecked = checked;

  uint32_t flags = OBJ_GetFlags(object);
  flags = checked ? (flags | OF_SELECTED) : (flags & ~OF_SELECTED);
  OBJ_SetFlags(object, flags);

  OBJ_SetDirty(object);
  _SendMessage(object, StyleController, ThemeChanged, .recursive = FALSE);
}

HANDLER(RadioButton, Object, Create)
{
  OBJ_SetStyle(hObject, OBJ_GetStyle(hObject) | OF_TABSTOP);
  struct Property *prop = PROP_FindByLongID(OBJ_GetProperties(hObject), ID_RadioButton_IsChecked);
  if (prop) PROP_SetFlag(prop, PF_USED_IN_TRIGGER);

  RadioButton_SetChecked(hObject, pRadioButton, pRadioButton->IsChecked);
  return FALSE;
}

HANDLER(RadioButton, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property)
    return FALSE;

  if (PROP_GetLongIdentifier(pPropertyChanged->Property) == ID_RadioButton_IsChecked) {
    RadioButton_SetChecked(hObject, pRadioButton, pRadioButton->IsChecked);
  }

  return FALSE;
}

HANDLER(RadioButton, Node, LeftButtonUp)
{
  struct Object *group = OBJ_GetParent(hObject);
  if (!group || !GetRadioGroup(group))
    return TRUE;

  struct RadioGroup *rg = GetRadioGroup(group);

  /* No-op if this button is already selected */
  if (pRadioButton->IsChecked)
    return TRUE;

  const char *oldValue = rg->SelectedValue;

  /* Mark this button as selected */
  RadioButton_SetChecked(hObject, pRadioButton, TRUE);

  /* Update SelectedValue on the group */
  rg->SelectedValue = pRadioButton->Value;
  OBJ_SetDirty(group);

  /* Clear IsChecked on every sibling RadioButton */
  FOR_EACH_OBJECT(child, group) {
    if (child == hObject) continue;
    struct RadioButton *rb = GetRadioButton(child);
    if (rb && rb->IsChecked) {
      RadioButton_SetChecked(child, rb, FALSE);
    }
  }

  /* Fire RadioGroup.SelectionChanged synchronously so that Lua handlers
     registered on the group see the correct SelectedValue/OldValue within
     the same dispatch pass.  The args live on the stack; push_object_message_arg
     copies them into a Lua full-userdata before this call returns.
     Return value not checked: a FALSE result means no handler was registered,
     which is a valid no-op (SelectionChanged is optional). */
  struct RadioGroup_SelectionChangedEventArgs args = {
    .SelectedValue = rg->SelectedValue,
    .OldValue      = oldValue,
  };
  CORE_HandleObjectMessage(core.L, &(struct AXmessage){
    .target  = group,
    .message = ID_RadioGroup_SelectionChanged,
    .lParam  = &args,
  });
  return TRUE;
}
