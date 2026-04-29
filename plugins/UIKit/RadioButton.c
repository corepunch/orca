#include <include/orca.h>

#include <plugins/UIKit/UIKit.h>

HANDLER(RadioButton, Object, Create)
{
  OBJ_SetStyle(hObject, OBJ_GetStyle(hObject) | OF_TABSTOP);
  return FALSE;
}

HANDLER(RadioButton, Node, LeftButtonUp)
{
  pRadioButton->IsChecked = TRUE;
  OBJ_SetDirty(hObject);

  struct Object *group = OBJ_GetParent(hObject);
  if (!group || !GetRadioGroup(group))
    return TRUE;

  struct RadioGroup *rg = GetRadioGroup(group);
  const char *oldValue = rg->SelectedValue;

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

  /* Fire RadioGroup.SelectionChanged for Lua handlers.
     SelectedValue on the group is already updated above.
     Lua handlers read self.SelectedValue directly (like Button.Click). */
  axPostMessageW(group, ID_RadioGroup_SelectionChanged, 0, NULL);
  return TRUE;
}
