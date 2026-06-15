#include <include/orca.h>
#include <source/core/core_local.h>
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

  uint32_t flags = OBJ_GetFlags(object);
  flags = checked ? (flags | OF_SELECTED) : (flags & ~OF_SELECTED);
  OBJ_SetFlags(object, flags);

  OBJ_SetDirty(object);
  _SendMessage(object, StyleController, ThemeChanged, .recursive = FALSE);
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
  if (!rg)
    return;

  /* Capture old value before the property setter frees it. */
  char *oldValue = rg->SelectedValue ? strdup(rg->SelectedValue) : NULL;

  /* Update SelectedValue through the property API so the previous string is
     freed correctly, the new value is strdup'd, and Object.PropertyChanged
     fires on the group (which calls RadioGroup_SyncToSelectedValue to clear
     sibling buttons). */
  struct Property *prop = NULL;
  if (SUCCEEDED(OBJ_FindLongProperty(group, ID_RadioGroup_SelectedValue, &prop)))
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
  if (button->IsChecked)
    return TRUE;

  RadioButton_SetChecked(object, button, TRUE);
  RadioButton_SyncGroup(object, button);
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
    if (!GetRadioButton(child)) continue;
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

// RadioButton_Create
HANDLER(RadioButton, Object, Create)
{
  pRadioButton->AccentColor = FS_GetThemeColor(THEME_COLOR_ACCENT_BACKGROUND);
  GetNode(hObject)->Border.Color = FS_GetThemeColor(THEME_COLOR_CONTROL_BORDER);
  GetNode2D(hObject)->Background.Color = FS_GetThemeColor(THEME_COLOR_CONTROL_BACKGROUND);
  struct Property *foregroundProp = NULL;
  if (SUCCEEDED(OBJ_FindLongProperty(hObject, ID_Node2D_ForegroundColor, &foregroundProp))) {
    struct color foreground = FS_GetThemeColor(THEME_COLOR_ACCENT_FOREGROUND);
    PROP_SetValue(foregroundProp, &foreground);
  }

  OBJ_SetStyle(hObject, OBJ_GetStyle(hObject) | OF_TABSTOP);
  struct Property *prop = PROP_FindByLongID(OBJ_GetProperties(hObject), ID_RadioButton_IsChecked);
  if (prop) PROP_SetFlag(prop, PF_USED_IN_TRIGGER);

  RadioButton_SetChecked(hObject, pRadioButton, pRadioButton->IsChecked);
  return FALSE;
}

// RadioButton_Attached
HANDLER(RadioButton, Object, Attached)
{
  RadioButton_SyncToGroupSelection(hObject, pRadioButton);
  return FALSE;
}

// RadioButton_PropertyChanged
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

// RadioButton_LeftButtonUp
HANDLER(RadioButton, Node, LeftButtonUp)
{
  return RadioButton_Select(hObject, pRadioButton);
}

// RadioButton_KeyDown
HANDLER(RadioButton, Node, KeyDown)
{
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

// RadioButton_DrawBackground
HANDLER(RadioButton, Node2D, DrawBackground)
{
  struct Node2D *pNode2D = GetNode2D(hObject);
  if (!pNode2D) return FALSE;
  struct Node *pNode = GetNode(hObject);
  if (!pNode) return FALSE;

  float indicatorSize = pRadioButton->IndicatorSize;
  float h = Node2D_GetFrame(pNode2D, kBox3FieldHeight);
  float bx = pNode2D->_rect.x;
  float by = pNode2D->_rect.y + (h - indicatorSize) * 0.5f;

  /* Draw circular indicator using a fully-rounded box. */
  struct ViewEntity entity;
  memset(&entity, 0, sizeof(entity));
  entity.matrix  = pNode2D->Matrix;
  entity.bbox    = BOX3_FromRect(((struct rect){bx, by, indicatorSize, indicatorSize}));
  entity.radius  = (struct vec4){indicatorSize * 0.5f, indicatorSize * 0.5f,
                                  indicatorSize * 0.5f, indicatorSize * 0.5f};

  if (pRadioButton->IsChecked) {
    /* Filled accent-color circle */
    entity.material = (struct ViewMaterial){
      .color     = pRadioButton->AccentColor,
      .opacity   = pNode2D->_opacity,
      .blendMode = BLEND_MODE_ALPHA,
    };
    R_DrawEntity(pDrawBackground->viewdef, &entity);

    /* Inner white dot */
    float dotSize = indicatorSize * 0.4f;
    float dotOffset = (indicatorSize - dotSize) * 0.5f;
    entity.bbox = BOX3_FromRect(((struct rect){bx + dotOffset, by + dotOffset, dotSize, dotSize}));
    entity.radius = (struct vec4){dotSize * 0.5f, dotSize * 0.5f, dotSize * 0.5f, dotSize * 0.5f};
    struct BrushShorthand foreground = { 0 };
    Node2D_ReadProperty(hObject, Foreground.Color, &foreground.Color);
    Node2D_ReadProperty(hObject, Foreground.Image, &foreground.Image);
    Node2D_ReadProperty(hObject, Foreground.Material, &foreground.Material);
    entity.material.color = foreground.Color;
    R_DrawEntity(pDrawBackground->viewdef, &entity);
  } else {
    /* Empty circle: light fill with a subtle border drawn as a slightly
       larger background circle behind a smaller inner white circle. */
    entity.material = (struct ViewMaterial){
      .color     = pNode->Border.Color,
      .opacity   = pNode2D->_opacity,
      .blendMode = BLEND_MODE_ALPHA,
    };
    R_DrawEntity(pDrawBackground->viewdef, &entity);

    float innerSize = indicatorSize - 3.0f;
    float innerOffset = 1.5f;
    entity.bbox = BOX3_FromRect(((struct rect){bx + innerOffset, by + innerOffset, innerSize, innerSize}));
    entity.radius = (struct vec4){innerSize * 0.5f, innerSize * 0.5f, innerSize * 0.5f, innerSize * 0.5f};
    entity.material.color = pNode2D->Background.Color;
    R_DrawEntity(pDrawBackground->viewdef, &entity);
  }

  return FALSE;
}
