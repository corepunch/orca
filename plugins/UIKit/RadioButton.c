#include <include/orca.h>
#include <source/core/core_local.h>

#include <plugins/UIKit/UIKit.h>

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

static bool_t
RadioButton_Select(struct Object *object, struct RadioButton *button)
{
  struct Object *group = OBJ_GetParent(object);
  if (!group || !GetRadioGroup(group))
    return TRUE;

  struct RadioGroup *rg = GetRadioGroup(group);

  if (button->IsChecked)
    return TRUE;

  const char *oldValue = rg->SelectedValue;

  RadioButton_SetChecked(object, button, TRUE);

  rg->SelectedValue = button->Value;
  OBJ_SetDirty(group);

  FOR_EACH_OBJECT(child, group) {
    if (child == object) continue;
    struct RadioButton *rb = GetRadioButton(child);
    if (rb && rb->IsChecked) {
      RadioButton_SetChecked(child, rb, FALSE);
    }
  }

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
  return RadioButton_Select(hObject, pRadioButton);
}

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

HANDLER(RadioButton, Node2D, DrawBrush)
{
  if (pDrawBrush->foreground) return FALSE;

  struct Node2D *pNode2D = GetNode2D(hObject);
  if (!pNode2D) return FALSE;

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
    R_DrawEntity(pDrawBrush->viewdef, &entity);

    /* Inner white dot */
    float dotSize = indicatorSize * 0.4f;
    float dotOffset = (indicatorSize - dotSize) * 0.5f;
    entity.bbox = BOX3_FromRect(((struct rect){bx + dotOffset, by + dotOffset, dotSize, dotSize}));
    entity.radius = (struct vec4){dotSize * 0.5f, dotSize * 0.5f, dotSize * 0.5f, dotSize * 0.5f};
    entity.material.color = (struct color){1.0f, 1.0f, 1.0f, 1.0f};
    R_DrawEntity(pDrawBrush->viewdef, &entity);
  } else {
    /* Empty circle: light fill with a subtle border drawn as a slightly
       larger background circle behind a smaller inner white circle. */
    entity.material = (struct ViewMaterial){
      .color     = (struct color){0.7f, 0.7f, 0.75f, 1.0f},
      .opacity   = pNode2D->_opacity,
      .blendMode = BLEND_MODE_ALPHA,
    };
    R_DrawEntity(pDrawBrush->viewdef, &entity);

    float innerSize = indicatorSize - 3.0f;
    float innerOffset = 1.5f;
    entity.bbox = BOX3_FromRect(((struct rect){bx + innerOffset, by + innerOffset, innerSize, innerSize}));
    entity.radius = (struct vec4){innerSize * 0.5f, innerSize * 0.5f, innerSize * 0.5f, innerSize * 0.5f};
    entity.material.color = (struct color){0.95f, 0.95f, 0.97f, 1.0f};
    R_DrawEntity(pDrawBrush->viewdef, &entity);
  }

  return FALSE;
}
