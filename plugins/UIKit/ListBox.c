#include <UIKit/UIKit.h>
#include <source/core/core_local.h>
#include <source/core/property/property_internal.h>

static const char *
ListBox_GetItemValue(struct Object *hObject, struct ListBox *pListBox, struct Object *item)
{
  struct Node *node = GetNode(item);
  if (!node || !node->DataContext) return NULL;

  struct Object *dataContext = CMP_GetObject(node->DataContext);
  if (!dataContext) return NULL;

  const char *valueProp = pListBox->ValueProperty;
  if (!valueProp || !valueProp[0]) valueProp = "Name";

  if (strcmp(valueProp, "Name") == 0) {
    return OBJ_GetName(dataContext);
  }

  struct Property *prop = OBJ_FindPropertyByPath(dataContext, valueProp);
  if (prop && PROP_GetType(prop) == kDataTypeString) {
    const char *value = *(const char * const *)PROP_GetValue(prop);
    if (value && value[0]) {
      return value;
    }
  }

  uint32_t childIdent = fnv1a32(valueProp);
  struct Object *child = OBJ_FindImmediateChild(dataContext, childIdent);
  if (child) {
    return OBJ_GetName(child);
  }
  return NULL;
}

static void
ListBox_SetItemSelected(struct Object *item, bool_t selected)
{
  if (!item) return;

  uint32_t flags = OBJ_GetFlags(item);
  flags = selected ? (flags | OF_SELECTED) : (flags & ~OF_SELECTED);

  OBJ_SetFlags(item, flags);
  OBJ_SetDirty(item);
  _SendMessage(item, StyleController, ThemeChanged, .recursive = TRUE);
}

static void
ListBox_SyncLinkedPageHost(struct Object *hObject, const char *value)
{
  if (!value || !value[0]) return;

  struct Object *parent = OBJ_GetParent(hObject);
  if (!parent) return;

  FOR_EACH_OBJECT(sibling, parent) {
    if (!GetPageHost(sibling)) continue;

    struct Property *prop = OBJ_FindLongProperty(sibling, ID_PageHost_ActivePage);
    if (prop) {
      PROP_SetStringValue(prop, value);
    }
    return;
  }
}

static struct Object *
ListBox_PickItemFromPoint(struct Object *hObject, struct ListBox *pListBox, float x, float y)
{
  struct Node2D *node2D = GetNode2D(hObject);
  struct StackView *stack = GetStackView(hObject);
  if (!node2D || !stack) return NULL;

  int count = 0;
  FOR_EACH_OBJECT(child, hObject) {
    (void)child;
    count++;
  }
  if (count <= 0) return NULL;

  struct mat4 inv = MAT4_Inverse(&node2D->Matrix);
  struct vec3 local = MAT4_MultiplyVector3D(&inv, &(struct vec3){ x, y, 0 });

  float width = Node2D_GetFrame(node2D, kBox3FieldWidth);
  float height = Node2D_GetFrame(node2D, kBox3FieldHeight);

  int index = 0;
  if (stack->Direction == kDirectionHorizontal && width > 0) {
    float slot = width / (float)count;
    index = (int)(local.x / slot);
  } else if (height > 0) {
    float slot = height / (float)count;
    index = (int)(local.y / slot);
  }

  if (index < 0) index = 0;
  if (index >= count) index = count - 1;

  int i = 0;
  FOR_EACH_OBJECT(child, hObject) {
    if (i == index) {
      return child;
    }
    i++;
  }

  return NULL;
}

static void
ListBox_SetSelected(struct Object *hObject, struct ListBox *pListBox, struct Object *item)
{
  const char *newValue = ListBox_GetItemValue(hObject, pListBox, item);
  if (!newValue) return;

  const char *oldValue = pListBox->SelectedValue;
  if (oldValue && newValue && strcmp(oldValue, newValue) == 0) return;

  char *savedOld = oldValue ? strdup(oldValue) : NULL;

  struct Property *prop = OBJ_FindLongProperty(hObject, ID_ListBox_SelectedValue);
  if (prop) PROP_SetStringValue(prop, newValue);

  // Update SelectedItem from the clicked item's DataContext
  struct Node *itemNode = GetNode(item);
  if (itemNode && itemNode->DataContext) {
    struct ItemsControl *ic = GetItemsControl(hObject);
    if (ic) {
      ic->SelectedItem = GetDataObject(CMP_GetObject(itemNode->DataContext));
      PROP_NotifyChanged(OBJ_FindLongProperty(hObject, ID_ItemsControl_SelectedItem));
    }
  }

  OBJ_SetDirty(hObject);

  struct ListBox_SelectionChangedEventArgs args = {
    .SelectedValue = pListBox->SelectedValue,
    .OldValue      = savedOld,
  };
  axPostMessageDataW(hObject, ID_ListBox_SelectionChanged, 0, &args, sizeof(args));
}

static void
ListBox_SyncToSelectedValue(struct Object *hObject, struct ListBox *pListBox)
{
  if (!pListBox->SelectedValue || !pListBox->SelectedValue[0]) return;

  struct ItemsControl *ic = GetItemsControl(hObject);

  FOR_EACH_OBJECT(child, hObject) {
    const char *val = ListBox_GetItemValue(hObject, pListBox, child);
    if (val && strcmp(val, pListBox->SelectedValue) == 0) {
      ListBox_SetItemSelected(child, TRUE);
      // Sync SelectedItem to this child's DataContext
      struct Node *itemNode = GetNode(child);
      if (itemNode && itemNode->DataContext && ic) {
        ic->SelectedItem = GetDataObject(CMP_GetObject(itemNode->DataContext));
        PROP_NotifyChanged(OBJ_FindLongProperty(hObject, ID_ItemsControl_SelectedItem));
      }
    } else {
      ListBox_SetItemSelected(child, FALSE);
    }
  }
}

// ListBox_Start
HANDLER(ListBox, Object, Start) {
  OBJ_Clear(hObject); // wipe existing children
  struct ItemsControl *ic = GetItemsControl(hObject);
  if (!ic->ItemsSource) return FALSE; // nothing to bind
  struct Node2D *tpl = ic->ItemTemplate;
  FOR_EACH_OBJECT(data, CMP_GetObject(ic->ItemsSource)) { // iterate data items
    // instantiate template, or fall back to a plain TextBlock
    struct Object *item = tpl ? OBJ_Instantiate(CMP_GetObject(tpl)) : OBJ_Create(ID_TextBlock);
    if (!item) return Con_Printf("Can not instantiate ListBox item template"), FALSE;
    if (!tpl) {
      char *text = "Item";
      PROP_SetValue(OBJ_FindLongProperty(item, ID_TextRun_Text), &text);
    }
    // set DataContext directly on Node (binding system reads it from here)
    struct Node *n = GetNode(item);
    n && (n->DataContext = GetDataContext(data));
    OBJ_AddChild(hObject, item);
  }
  if (!pListBox->SelectedValue || !*pListBox->SelectedValue) {
    // no selection yet -> auto-select first child
    struct Object *first = NULL;
    FOR_EACH_OBJECT(c, hObject) { first = c; break; }
    if (first) ListBox_SetSelected(hObject, pListBox, first);
  } else {
    // restore selection matching existing SelectedValue
    ListBox_SyncToSelectedValue(hObject, pListBox);
  }
  return TRUE;
}

// ListBox_PropertyChanged — sync SelectedValue from binding
HANDLER(ListBox, Object, PropertyChanged) {
  if (!pPropertyChanged->Property)
    return FALSE;

  if (PROP_GetLongIdentifier(pPropertyChanged->Property) == ID_ListBox_SelectedValue) {
    ListBox_SyncToSelectedValue(hObject, pListBox);
    ListBox_SyncLinkedPageHost(hObject, pListBox->SelectedValue);
  }

  return FALSE;
}

// ListBox_LeftButtonUp — find clicked child and select it
HANDLER(ListBox, Node, LeftButtonUp) {
  struct Object *clickedChild = NULL;

  FOR_EACH_OBJECT(child, hObject) {
    struct Object *hit = (struct Object *)_SendMessage(child, Node, HitTest,
      .x = pLeftButtonUp->x,
      .y = pLeftButtonUp->y);
    if (hit) {
      clickedChild = child;
      break;
    }
  }

  if (!clickedChild) {
    clickedChild = ListBox_PickItemFromPoint(hObject, pListBox,
      pLeftButtonUp->x,
      pLeftButtonUp->y);
  }

  if (clickedChild) {
    ListBox_SetSelected(hObject, pListBox, clickedChild);
    return TRUE;
  }

  return FALSE;
}

// ListBox_SelectItem — select an item by its value
HANDLER(ListBox, ListBox, SelectItem) {
  if (!pSelectItem->Value || !pSelectItem->Value[0]) return FALSE;

  FOR_EACH_OBJECT(child, hObject) {
    const char *val = ListBox_GetItemValue(hObject, pListBox, child);
    if (val && strcmp(val, pSelectItem->Value) == 0) {
      ListBox_SetSelected(hObject, pListBox, child);
      return TRUE;
    }
  }

  return FALSE;
}
