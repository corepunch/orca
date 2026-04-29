#include <include/orca.h>
#include <source/core/core_local.h>

#include <plugins/UIKit/UIKit.h>

HANDLER(RadioButton, Object, Create)
{
  OBJ_SetStyle(hObject, OBJ_GetStyle(hObject) | OF_TABSTOP);
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
  pRadioButton->IsChecked = TRUE;
  OBJ_SetDirty(hObject);

  /* Update SelectedValue on the group */
  rg->SelectedValue = pRadioButton->Value;
  OBJ_SetDirty(group);

  /* Clear IsChecked on every sibling RadioButton */
  FOR_EACH_OBJECT(child, group) {
    if (child == hObject) continue;
    struct RadioButton *rb = GetRadioButton(child);
    if (rb && rb->IsChecked) {
      rb->IsChecked = FALSE;
      OBJ_SetDirty(child);
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
