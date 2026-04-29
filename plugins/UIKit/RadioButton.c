#include <include/orca.h>

#include <plugins/UIKit/UIKit.h>

HANDLER(RadioButton, Object, Create)
{
  OBJ_SetStyle(hObject, OBJ_GetStyle(hObject) | OF_TABSTOP);
  return FALSE;
}

/* Ring buffer for SelectionChanged event args.  Using a small ring ensures
   the pointer is stable across the async dispatch while still being safe if
   several clicks are queued before any SelectionChanged is processed. */
#define SELCHG_RING_SIZE 8
static struct RadioGroup_SelectionChangedEventArgs _selchg_ring[SELCHG_RING_SIZE];
static uint8_t _selchg_head = 0;

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

  /* Fire RadioGroup.SelectionChanged.  Args are copied into a ring slot so
     the pointer remains valid until the async message is dispatched, even
     if another click is queued before dispatch. */
  struct RadioGroup_SelectionChangedEventArgs *args =
    &_selchg_ring[_selchg_head++ % SELCHG_RING_SIZE];
  args->SelectedValue = rg->SelectedValue;
  args->OldValue      = oldValue;
  axPostMessageW(group, ID_RadioGroup_SelectionChanged, 0, args);
  return TRUE;
}
