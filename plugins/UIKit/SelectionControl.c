#include "SelectionControl.h"
#include <math.h>

struct Object *_NextTabStop(struct Object *object);

static struct Object *
SelectionControl_CreatePart(struct Object *parent, uint32_t type, char const *name, char const *css)
{
  struct Object *part = OBJ_Create(type);
  if (!part) return NULL;
  OBJ_SetName(part, name);
  GetNode2D(part)->IgnoreHitTest = TRUE;
  _SendMessage(part, StyleController, AddClass, .ClassName = css);
  OBJ_AddChild(parent, part);
  OBJ_ReleaseRef(part);
  return part;
}

bool_t
SelectionControl_IsEnabled(struct Object *object)
{
  for (struct Object *o = object; o; o = OBJ_GetParent(o))
    if (OBJ_GetFlags(o) & OF_DISABLED) return FALSE;
  struct SelectionControl *control = GetSelectionControl(object);
  return control && control->IsEnabled;
}

void
SelectionControl_SetState(struct Object *object, bool_t checked, bool_t mixed)
{
  uint32_t flags = OBJ_GetFlags(object) & ~(OF_CHECKED | OF_INDETERMINATE);
  if (checked) flags |= OF_CHECKED;
  if (mixed) flags |= OF_INDETERMINATE;
  OBJ_SetFlags(object, flags);
  OBJ_SetDirty(object);
}

static void
SelectionControl_SyncEnabled(struct Object *object, struct SelectionControl *control)
{
  uint32_t flags = OBJ_GetFlags(object);
  OBJ_SetFlags(object, control->IsEnabled ? flags & ~OF_DISABLED : (flags | OF_DISABLED) & ~OF_SELECTED);
  uint32_t style = OBJ_GetStyle(object);
  OBJ_SetStyle(object, control->IsEnabled ? style | OF_TABSTOP : style & ~OF_TABSTOP);
}

void
SelectionControl_Build(struct Object *object, char const *css, bool_t isSwitch)
{
  struct SelectionControl *control = GetSelectionControl(object);
  if (!control || control->_indicator) return;
  _SendMessage(object, StyleController, AddClass, .ClassName = "selection-control");
  _SendMessage(object, StyleController, AddClass, .ClassName = css);
  control->_indicator = SelectionControl_CreatePart(object, ID_Node2D, "Indicator", "control-indicator");
  control->_label = SelectionControl_CreatePart(object, ID_TextBlock, "Label", "control-label");
  if (!control->_indicator || !control->_label) return;
  if (isSwitch) {
    _SendMessage(control->_indicator, StyleController, AddClass, .ClassName = "control-track");
    SelectionControl_CreatePart(control->_indicator, ID_Node2D, "Thumb", "control-thumb");
  } else if (GetCheckBox(object)) {
    struct Object *mark = SelectionControl_CreatePart(control->_indicator, ID_VectorView, "CheckMark", "control-mark");
    if (mark) _SendMessage(mark, StyleController, AddClass, .ClassName = "control-check");
    SelectionControl_CreatePart(control->_indicator, ID_Node2D, "Indeterminate", "control-indeterminate");
  } else {
    SelectionControl_CreatePart(control->_indicator, ID_Node2D, "Mark", "control-mark");
  }
  SelectionControl_SyncEnabled(object, control);
}

static void
SelectionControl_SyncLabel(struct Object *object, struct SelectionControl *control)
{
  if (!control->_label) return;
  char const *text = GetTextRun(object)->Text;
  char const *label = GetTextRun(control->_label)->Text;
  if (!strcmp(text ? text : "", label ? label : "")) return;
  struct Property *p = OBJ_FindLongProperty(control->_label, ID_TextRun_Text);
  if (p) PROP_SetStringValue(p, text ? text : "");
}

HANDLER(SelectionControl, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property) return FALSE;
  uint32_t id = PROP_GetLongIdentifier(pPropertyChanged->Property);
  if (id == ID_SelectionControl_IsEnabled)
    SelectionControl_SyncEnabled(hObject, pSelectionControl);
  if (id == ID_TextRun_Text)
    SelectionControl_SyncLabel(hObject, pSelectionControl);
  if (id == ID_SelectionControl_IndicatorSize || id == ID_RadioButton_IndicatorSize) {
    if (id == ID_RadioButton_IndicatorSize)
      pSelectionControl->IndicatorSize = GetRadioButton(hObject)->IndicatorSize;
    pSelectionControl->_indicatorSizeSet = TRUE;
    OBJ_SetDirty(hObject);
  }
  return FALSE;
}

HANDLER(SelectionControl, Node, LeftButtonDown)
{
  if (SelectionControl_IsEnabled(hObject))
    OBJ_SetFlags(hObject, OBJ_GetFlags(hObject) | OF_SELECTED);
  return TRUE;
}

HANDLER(SelectionControl, Node, LeftButtonDragged)
{
  OBJ_SetFlags(hObject, OBJ_GetFlags(hObject) & ~OF_SELECTED);
  return FALSE;
}

HANDLER(SelectionControl, Node2D, MeasureOverride)
{
  SelectionControl_SyncLabel(hObject, pSelectionControl);
  if (!pSelectionControl->_indicator || !pSelectionControl->_label) return MAKEDWORD(0, 0);
  if (pSelectionControl->_indicatorSizeSet) {
    struct Node *node = GetNode(pSelectionControl->_indicator);
    node->Size.Axis[0].Requested = MAX(0, pSelectionControl->IndicatorSize) * (GetToggleSwitch(hObject) ? 58.f / 28 : 1);
    node->Size.Axis[1].Requested = MAX(0, pSelectionControl->IndicatorSize);
  }
  LRESULT indicator = _SendMessage(pSelectionControl->_indicator, Node2D, Measure,
    .Width = pMeasureOverride->Width, .Height = pMeasureOverride->Height);
  float spacing = GetTextRun(pSelectionControl->_label)->Text && *GetTextRun(pSelectionControl->_label)->Text
    ? pSelectionControl->IndicatorSpacing : 0;
  LRESULT label = _SendMessage(pSelectionControl->_label, Node2D, Measure,
    .Width = MAX(0, pMeasureOverride->Width - LOWORD(indicator) - spacing), .Height = pMeasureOverride->Height);
  struct Node2D *node = GetNode2D(hObject);
  float minHeight = MAX(0, GetNode(hObject)->Size.Axis[1].Min - TOTAL_PADDING(node, 1));
  return MAKEDWORD(LOWORD(indicator) + spacing + LOWORD(label),
    MAX(minHeight, MAX(HIWORD(indicator), HIWORD(label))));
}

HANDLER(SelectionControl, Node2D, ArrangeOverride)
{
  struct Object *indicator = pSelectionControl->_indicator, *label = pSelectionControl->_label;
  if (!indicator || !label) return MAKEDWORD(pArrangeOverride->Width, pArrangeOverride->Height);
  struct Node2D *part = GetNode2D(indicator);
  float width = NODE2D_FRAME(part, Size, 0).Desired + TOTAL_MARGIN(part, 0);
  float height = NODE2D_FRAME(part, Size, 1).Desired + TOTAL_MARGIN(part, 1);
  _SendMessage(indicator, Node2D, Arrange,
    .X = pArrangeOverride->X, .Y = pArrangeOverride->Y + MAX(0, (pArrangeOverride->Height - height) / 2),
    .Width = width, .Height = height);
  float spacing = GetTextRun(label)->Text && *GetTextRun(label)->Text ? pSelectionControl->IndicatorSpacing : 0;
  _SendMessage(label, Node2D, Arrange,
    .X = pArrangeOverride->X + width + spacing, .Y = pArrangeOverride->Y,
    .Width = MAX(0, pArrangeOverride->Width - width - spacing), .Height = pArrangeOverride->Height);
  return MAKEDWORD(pArrangeOverride->Width, pArrangeOverride->Height);
}

HANDLER(SelectionControl, Node2D, UpdateGeometry)
{
  struct Node2D *node = GetNode2D(hObject);
  node->_rect = (struct rect){0, 0, Node2D_GetFrame(node, kBox3FieldWidth), Node2D_GetFrame(node, kBox3FieldHeight)};
  return TRUE;
}

HANDLER(SelectionControl, Node2D, DrawForeground)
{
  // The generated label draws the text; the root retains TextBlock's public API.
  return TRUE;
}

static void
CheckBox_PostChanged(struct Object *object, struct CheckBox *check)
{
  struct CheckBox_CheckedChangedEventArgs args = {
    .IsChecked = check->IsChecked, .IsIndeterminate = check->IsIndeterminate,
  };
  axPostMessageDataW(object, ID_CheckBox_CheckedChanged, 0, &args, sizeof(args));
}

static bool_t
CheckBox_Activate(struct Object *object, struct CheckBox *check)
{
  if (!SelectionControl_IsEnabled(object)) return TRUE;
  bool_t checked = check->IsIndeterminate || !check->IsChecked;
  check->_changing = TRUE;
  struct Property *p = OBJ_FindLongProperty(object, ID_CheckBox_IsChecked);
  if (p) PROP_SetValue(p, &checked);
  if (check->IsIndeterminate) {
    bool_t mixed = FALSE;
    PROP_SetValue(OBJ_FindLongProperty(object, ID_CheckBox_IsIndeterminate), &mixed);
  }
  check->_changing = FALSE;
  CheckBox_PostChanged(object, check);
  return TRUE;
}

HANDLER(CheckBox, Object, Create)
{
  SelectionControl_Build(hObject, "checkbox", FALSE);
  SelectionControl_SetState(hObject, pCheckBox->IsChecked, pCheckBox->IsIndeterminate);
  return FALSE;
}

HANDLER(CheckBox, Object, PropertyChanged)
{
  uint32_t id = pPropertyChanged->Property ? PROP_GetLongIdentifier(pPropertyChanged->Property) : 0;
  if (id == ID_CheckBox_IsChecked || id == ID_CheckBox_IsIndeterminate) {
    SelectionControl_SetState(hObject, pCheckBox->IsChecked, pCheckBox->IsIndeterminate);
    if (!pCheckBox->_changing) CheckBox_PostChanged(hObject, pCheckBox);
  }
  return FALSE;
}

HANDLER(CheckBox, Node, LeftButtonUp)
{
  OBJ_SetFlags(hObject, OBJ_GetFlags(hObject) & ~OF_SELECTED);
  return CheckBox_Activate(hObject, pCheckBox);
}

HANDLER(CheckBox, Node, KeyDown)
{
  if (!SelectionControl_IsEnabled(hObject)) return TRUE;
  switch (pKeyDown->keyCode) {
    case AX_KEY_SPACE: case AX_KEY_ENTER: case AX_KEY_KP_ENTER:
      return CheckBox_Activate(hObject, pCheckBox);
    case AX_KEY_TAB:
      OBJ_SetFocus(_NextTabStop(hObject));
      return TRUE;
    default: return FALSE;
  }
}

HANDLER(ToggleSwitch, Object, Create)
{
  SelectionControl_Build(hObject, "toggle-switch", TRUE);
  SelectionControl_SetState(hObject, pToggleSwitch->IsOn, FALSE);
  return FALSE;
}

HANDLER(ToggleSwitch, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property || PROP_GetLongIdentifier(pPropertyChanged->Property) != ID_ToggleSwitch_IsOn)
    return FALSE;
  SelectionControl_SetState(hObject, pToggleSwitch->IsOn, FALSE);
  struct ToggleSwitch_ToggledEventArgs args = { .IsOn = pToggleSwitch->IsOn };
  axPostMessageDataW(hObject, ID_ToggleSwitch_Toggled, 0, &args, sizeof(args));
  return FALSE;
}

static bool_t
ToggleSwitch_Activate(struct Object *object, struct ToggleSwitch *toggle)
{
  if (!SelectionControl_IsEnabled(object)) return TRUE;
  bool_t on = !toggle->IsOn;
  PROP_SetValue(OBJ_FindLongProperty(object, ID_ToggleSwitch_IsOn), &on);
  return TRUE;
}

HANDLER(ToggleSwitch, Node, LeftButtonUp)
{
  OBJ_SetFlags(hObject, OBJ_GetFlags(hObject) & ~OF_SELECTED);
  return ToggleSwitch_Activate(hObject, pToggleSwitch);
}

HANDLER(ToggleSwitch, Node, KeyDown)
{
  if (!SelectionControl_IsEnabled(hObject)) return TRUE;
  switch (pKeyDown->keyCode) {
    case AX_KEY_SPACE: case AX_KEY_ENTER: case AX_KEY_KP_ENTER:
      return ToggleSwitch_Activate(hObject, pToggleSwitch);
    case AX_KEY_TAB:
      OBJ_SetFocus(_NextTabStop(hObject));
      return TRUE;
    default: return FALSE;
  }
}
