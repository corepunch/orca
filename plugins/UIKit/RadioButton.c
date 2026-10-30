#include <include/orca.h>
#include <source/core/core_local.h>
#include "SelectionControl.h"
#include <source/filesystem/theme_palette.h>

#include <UIKit/UIKit.h>
#include <plugins/UIKit/UIKit_message.h>

struct Object *
_NextTabStop(struct Object *hObject);

static void
RadioButton_SetChecked(struct Object *object,
                       struct RadioButton *button,
                       bool_t checked)
{
  if (!object || !button)
    return;

  button->IsChecked = checked;

  SelectionControl_SetState(object, checked, FALSE);
}

/* Sync the parent RadioGroup after this button has been checked: updates
   SelectedValue via the property system (so ownership is correct and
   PropertyChanged fires to sync siblings), then fires SelectionChanged.
   Assumes the caller has already set button->IsChecked = TRUE via
   RadioButton_SetChecked. */
static void
RadioButton_SyncGroup(struct Object *object, struct RadioButton *button)
{
  struct Object *group = OBJ_GetParent(object);
  if (!group)
    return;

  struct RadioGroup *rg = GetRadioGroup(group);
  if (!rg || (rg->SelectedValue && button->Value && !strcmp(rg->SelectedValue, button->Value)))
    return;

  /* Capture old value before the property setter frees it. */
  char *oldValue = rg->SelectedValue ? strdup(rg->SelectedValue) : NULL;

  /* Update SelectedValue through the property API so the previous string is
     freed correctly, the new value is strdup'd, and Object.PropertyChanged
     fires on the group (which calls RadioGroup_SyncToSelectedValue to clear
     sibling buttons). */
  struct Property *prop = OBJ_FindLongProperty(group, ID_RadioGroup_SelectedValue);
  if (prop)
    PROP_SetStringValue(prop, button->Value);

  struct RadioGroup_SelectionChangedEventArgs args = {
    .SelectedValue = rg->SelectedValue,
    .OldValue      = oldValue,
  };
  axPostMessageDataW(group, ID_RadioGroup_SelectionChanged, 0, &args, sizeof(args));
  /* oldValue is intentionally not freed here: the queued event-args payload
     retains the pointer until the event pump delivers the message. */
}

static bool_t
RadioButton_Select(struct Object *object, struct RadioButton *button)
{
  if (!SelectionControl_IsEnabled(object) || button->IsChecked) return TRUE;
  bool_t checked = TRUE;
  struct Property *property = OBJ_FindLongProperty(object, ID_RadioButton_IsChecked);
  if (property) PROP_SetValue(property, &checked);
  return TRUE;
}

static struct Object *
RadioButton_FindSibling(struct Object *object, bool_t forward)
{
  struct Object *group = OBJ_GetParent(object);
  if (!group || !GetRadioGroup(group))
    return NULL;

  struct Object *first = NULL;
  struct Object *last = NULL;
  struct Object *previous = NULL;
  bool_t seenCurrent = FALSE;

  FOR_EACH_OBJECT(child, group) {
    if (!GetRadioButton(child) || !SelectionControl_IsEnabled(child)) continue;
    if (!first) first = child;

    if (forward && seenCurrent)
      return child;
    if (child == object) {
      if (!forward)
        return previous;
      seenCurrent = TRUE;
    }

    previous = child;
    last = child;
  }

  return forward ? first : last;
}

static void
RadioButton_SyncToGroupSelection(struct Object *hObject, struct RadioButton *pRadioButton)
{
  struct Object *group = OBJ_GetParent(hObject);
  if (!group) return;
  struct RadioGroup *rg = GetRadioGroup(group);
  if (!rg || !rg->SelectedValue || !pRadioButton->Value) return;

  bool_t should_be_checked = strcmp(pRadioButton->Value, rg->SelectedValue) == 0;
  if (pRadioButton->IsChecked != should_be_checked)
    RadioButton_SetChecked(hObject, pRadioButton, should_be_checked);
}

HANDLER(RadioButton, Object, Create)
{
  pRadioButton->AccentColor = FS_GetThemeColor(THEME_COLOR_ACCENT_BACKGROUND);
  SelectionControl_Build(hObject, "radio-button", FALSE);
  struct Property *prop = OBJ_FindLongProperty(hObject, ID_RadioButton_IsChecked);
  if (prop) PROP_SetFlag(prop, PF_USED_IN_TRIGGER);

  RadioButton_SetChecked(hObject, pRadioButton, pRadioButton->IsChecked);
  return FALSE;
}

HANDLER(RadioButton, Object, Attached)
{
  RadioButton_SyncToGroupSelection(hObject, pRadioButton);
  return FALSE;
}

HANDLER(RadioButton, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property)
    return FALSE;

  if (PROP_GetLongIdentifier(pPropertyChanged->Property) == ID_RadioButton_Value) {
    RadioButton_SyncToGroupSelection(hObject, pRadioButton);
    return FALSE;
  }

  if (PROP_GetLongIdentifier(pPropertyChanged->Property) == ID_RadioButton_IsChecked) {
    RadioButton_SetChecked(hObject, pRadioButton, pRadioButton->IsChecked);
    if (pRadioButton->IsChecked)
      RadioButton_SyncGroup(hObject, pRadioButton);
  }

  return FALSE;
}

HANDLER(RadioButton, Node, LeftButtonUp)
{
  OBJ_SetFlags(hObject, OBJ_GetFlags(hObject) & ~OF_SELECTED);
  return RadioButton_Select(hObject, pRadioButton);
}

HANDLER(RadioButton, Node, KeyDown)
{
  if (!SelectionControl_IsEnabled(hObject)) return TRUE;
  switch (pKeyDown->keyCode) {
    case AX_KEY_SPACE:
    case AX_KEY_ENTER:
    case AX_KEY_KP_ENTER:
      return RadioButton_Select(hObject, pRadioButton);
    case AX_KEY_RIGHTARROW:
    case AX_KEY_DOWNARROW: {
      struct Object *next = RadioButton_FindSibling(hObject, TRUE);
      if (next) {
        OBJ_SetFocus(next);
        return RadioButton_Select(next, GetRadioButton(next));
      }
      return TRUE;
    }
    case AX_KEY_LEFTARROW:
    case AX_KEY_UPARROW: {
      struct Object *previous = RadioButton_FindSibling(hObject, FALSE);
      if (previous) {
        OBJ_SetFocus(previous);
        return RadioButton_Select(previous, GetRadioButton(previous));
      }
      return TRUE;
    }
    case AX_KEY_TAB:
      OBJ_SetFocus(_NextTabStop(hObject));
      return TRUE;
    default:
      return FALSE;
  }
}

