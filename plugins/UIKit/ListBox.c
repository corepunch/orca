#include <UIKit/UIKit.h>

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

  uint32_t childIdent = fnv1a32(valueProp);
  struct Object *child = OBJ_FindImmediateChild(dataContext, childIdent);
  if (child) {
    return OBJ_GetName(child);
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

  FOR_EACH_OBJECT(child, hObject) {
    const char *val = ListBox_GetItemValue(hObject, pListBox, child);
    if (val && strcmp(val, pListBox->SelectedValue) == 0) {
      uint32_t flags = OBJ_GetFlags(child);
      OBJ_SetFlags(child, flags | OF_SELECTED);
    } else {
      uint32_t flags = OBJ_GetFlags(child);
      OBJ_SetFlags(child, flags & ~OF_SELECTED);
    }
  }
}

// ListBox_Start
HANDLER(ListBox, Object, Start) {
  OBJ_Clear(hObject);
  if (GetItemsControl(hObject)->ItemsSource) {
    struct Node2D* template = GetItemsControl(hObject)->ItemTemplate;
    FOR_EACH_OBJECT(data, CMP_GetObject(GetItemsControl(hObject)->ItemsSource)) {
      struct Object* item;
      if (template) {
        item = OBJ_Instantiate(CMP_GetObject(GetItemsControl(hObject)->ItemTemplate));
      } else {
        if ((item = OBJ_Create(ID_TextBlock))) {
          PROP_SetValue(OBJ_FindLongProperty(item, ID_TextRun_Text), "Item");
        }
      }
      if (!item) {
        Con_Printf("Can not instantiate ListBox item template");
        return FALSE;
      }
      // Set DataContext directly on the Node struct (same as binding system reads it)
      struct Node *itemNode = GetNode(item);
      if (itemNode) {
        itemNode->DataContext = OBJ_GetComponent(data, ID_DataObject);
      }
      OBJ_AddChild(hObject, item);
    }
    // Auto-select first item if no SelectedValue is set
    if (!pListBox->SelectedValue || !pListBox->SelectedValue[0]) {
      struct Object *first = NULL;
      FOR_EACH_OBJECT(child, hObject) {
        first = child;
        break;
      }
      if (first) {
        ListBox_SetSelected(hObject, pListBox, first);
      }
    } else {
      ListBox_SyncToSelectedValue(hObject, pListBox);
    }
    return TRUE;
  } else {
    return FALSE;
  }
}

// ListBox_PropertyChanged — sync SelectedValue from binding
HANDLER(ListBox, Object, PropertyChanged) {
  if (!pPropertyChanged->Property)
    return FALSE;

  if (PROP_GetLongIdentifier(pPropertyChanged->Property) == ID_ListBox_SelectedValue) {
    ListBox_SyncToSelectedValue(hObject, pListBox);
  }

  return FALSE;
}

// ListBox_LeftButtonUp — find clicked child and select it
HANDLER(ListBox, Node, LeftButtonUp) {
  struct Object *clickedChild = NULL;

  FOR_EACH_OBJECT(child, hObject) {
    struct Node2D *childNode = GetNode2D(child);
    if (!childNode) continue;

    struct rect frame = childNode->_rect;
    float x = pLeftButtonUp->x;
    float y = pLeftButtonUp->y;

    if (x >= frame.x && x <= frame.x + frame.width &&
        y >= frame.y && y <= frame.y + frame.height) {
      clickedChild = child;
      break;
    }
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
