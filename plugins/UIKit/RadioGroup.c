#include <include/orca.h>

#include <UIKit/UIKit.h>

/* Use the property system so bindings and CSS parts observe group changes.
   RadioButton skips group resynchronization when SelectedValue already matches. */
static void
RadioGroup_SetButtonChecked(struct Object *object,
                             struct RadioButton *button,
                             bool_t checked)
{
  if (!button)
    return;

  struct Property *property = OBJ_FindLongProperty(object, ID_RadioButton_IsChecked);
  if (property) PROP_SetValue(property, &checked);
}

/* Walk RadioButton children and check the one whose Value matches
   SelectedValue; uncheck all others. */
static void
RadioGroup_SyncToSelectedValue(struct Object *object,
                                struct RadioGroup *group)
{
  FOR_EACH_OBJECT(child, object) {
    struct RadioButton *rb = GetRadioButton(child);
    if (!rb) continue;

    bool_t should_be_checked = group->SelectedValue && rb->Value &&
                               strcmp(rb->Value, group->SelectedValue) == 0;
    if (rb->IsChecked != should_be_checked)
      RadioGroup_SetButtonChecked(child, rb, should_be_checked);
  }
}

HANDLER(RadioGroup, Object, Start)
{
  /* Sync children to SelectedValue once all children are loaded. */
  if (pRadioGroup->SelectedValue)
    RadioGroup_SyncToSelectedValue(hObject, pRadioGroup);
  return FALSE;
}

HANDLER(RadioGroup, Object, Attached)
{
  /* Re-apply the current selection when the group is attached so rebuilt
     screens restore the checked radio button even if Start ran too early. */
  if (pRadioGroup->SelectedValue)
    RadioGroup_SyncToSelectedValue(hObject, pRadioGroup);
  return FALSE;
}

HANDLER(RadioGroup, Object, PropertyChanged)
{
  /* When SelectedValue changes at runtime, keep child buttons in sync. */
  if (pPropertyChanged->Property &&
      PROP_GetLongIdentifier(pPropertyChanged->Property) == ID_RadioGroup_SelectedValue)
    RadioGroup_SyncToSelectedValue(hObject, pRadioGroup);
  return FALSE;
}

HANDLER(RadioGroup, RadioGroup, SelectionChanged)
{
  /* State is already updated by the child RadioButton before this fires.
     Return TRUE to stop the message from bubbling further up the tree. */
  return TRUE;
}
