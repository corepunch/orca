#include <UIKit/UIKit.h>
#include <source/core/core_local.h>
#include <source/core/property/property_internal.h>

struct ItemsControlEntry {
  struct Object *data, *view;
  struct ItemsControlEntry *next;
  bool_t seen;
};

static void
ItemsControl_ReleaseEntry(struct Object *owner, struct ItemsControlEntry *entry)
{
  if (OBJ_GetParent(entry->view) == owner) OBJ_RemoveFromParent(entry->view);
  OBJ_ReleaseRef(entry->view);
  OBJ_ReleaseRef(entry->data);
  free(entry);
}

static struct ItemsControlEntry **
ItemsControl_FindSlot(struct ItemsControlEntry **table, size_t capacity, struct Object *data)
{
  size_t slot = ((uintptr_t)data >> 4) & (capacity - 1);
  while (table[slot] && table[slot]->data != data) slot = (slot + 1) & (capacity - 1);
  return table + slot;
}

static void
ItemsControl_Synchronize(struct ItemsControl *ic)
{
  if (!ic) return;
  struct Object *hObject = CMP_GetObject(ic);
  ic->_started = TRUE;
  if (ic->_refreshing) { ic->_refreshPending = TRUE; return; }
  ic->_refreshing = TRUE;
  do {
    ic->_refreshPending = FALSE;
    struct Object *source = ic->ItemsSource ? CMP_GetObject(ic->ItemsSource) : NULL;
    if (source != ic->_observedSource) {
      if (source) OBJ_AddRef(source);
      if (ic->_observedSource) {
        core_UnobserveCollection(ic->_observedSource, hObject);
        OBJ_ReleaseRef(ic->_observedSource);
      }
      ic->_observedSource = source;
      if (source) core_ObserveCollection(source, hObject);
    }
    if (ic->_appliedTemplate != ic->ItemTemplate) {
      while (ic->_entries) {
        struct ItemsControlEntry *entry = ic->_entries;
        ic->_entries = entry->next;
        ItemsControl_ReleaseEntry(hObject, entry);
      }
      ic->_appliedTemplate = ic->ItemTemplate;
    }

    size_t count = 0, oldCount = 0, capacity = 8;
    if (source) FOR_EACH_OBJECT(data, source) { if (GetDataObject(data)) count++; }
    for (struct ItemsControlEntry *entry = ic->_entries; entry; entry = entry->next) oldCount++;
    while (capacity < (count + oldCount) * 2) capacity *= 2;
    struct ItemsControlEntry **table = calloc(capacity, sizeof(*table));
    struct ItemsControlEntry **ordered = count ? calloc(count, sizeof(*ordered)) : NULL;
    struct Object **snapshot = count ? calloc(count, sizeof(*snapshot)) : NULL;
    if (!table || (count && (!ordered || !snapshot))) {
      free(table); free(ordered); free(snapshot);
      break;
    }
    size_t index = 0;
    bool_t selectionPresent = !ic->SelectedItem;
    /* A template's Start handler may mutate the source while rows are created. */
    if (source) FOR_EACH_OBJECT(data, source) {
      if (!GetDataObject(data)) continue;
      if (ic->SelectedItem && data == CMP_GetObject(ic->SelectedItem)) selectionPresent = TRUE;
      snapshot[index++] = data;
      OBJ_AddRef(data);
    }
    for (struct ItemsControlEntry *entry = ic->_entries; entry; entry = entry->next) {
      entry->seen = FALSE;
      *ItemsControl_FindSlot(table, capacity, entry->data) = entry;
    }
    for (index = 0; index < count; index++) {
      struct Object *data = snapshot[index];
      struct ItemsControlEntry **slot = ItemsControl_FindSlot(table, capacity, data);
      struct ItemsControlEntry *entry = *slot;
      if (!entry) {
        struct Object *view = ic->ItemTemplate
          ? OBJ_Instantiate(CMP_GetObject(ic->ItemTemplate)) : OBJ_Create(ID_TextBlock);
        if (!view) { OBJ_ReleaseRef(data); continue; }
        entry = calloc(1, sizeof(*entry));
        if (!entry) { OBJ_ReleaseRef(view); OBJ_ReleaseRef(data); continue; }
        entry->data = data;
        entry->view = view;
        entry->next = ic->_entries;
        ic->_entries = entry;
        *slot = entry;
        if (!ic->ItemTemplate) PROP_SetStringValue(OBJ_FindLongProperty(view, ID_TextRun_Text), "Item");
        PROP_SetValue(OBJ_FindLongProperty(view, ID_Node_DataContext), &data);
      } else {
        OBJ_ReleaseRef(data);
      }
      entry->seen = TRUE;
      ordered[index] = entry;
      if (OBJ_GetParent(entry->view) != hObject) OBJ_AddChild(hObject, entry->view);
    }
    struct ItemsControlEntry **link = &ic->_entries;
    while (*link) {
      struct ItemsControlEntry *entry = *link;
      if (entry->seen) { link = &entry->next; continue; }
      *link = entry->next;
      ItemsControl_ReleaseEntry(hObject, entry);
    }
    struct Object *before = NULL;
    for (index = count; index > 0; index--) {
      struct ItemsControlEntry *entry = ordered[index - 1];
      if (!entry) continue;
      if (OBJ_GetNext(entry->view) != before) OBJ_MoveBefore(entry->view, before);
      before = entry->view;
    }
    if (!selectionPresent) {
      struct Object *none = NULL;
      PROP_SetValue(OBJ_FindLongProperty(hObject, ID_ItemsControl_SelectedItem), &none);
    }
    free(snapshot); free(ordered); free(table);
    _SendMessage(hObject, ItemsControl, ItemsChanged);
  } while (ic->_refreshPending);
  ic->_refreshing = FALSE;
}

HANDLER(ItemsControl, Object, Start) {
  pItemsControl->_started = TRUE;
  ItemsControl_Synchronize(pItemsControl);
  return TRUE;
}

HANDLER(ItemsControl, Object, PropertyChanged) {
  if (!pPropertyChanged->Property) return FALSE;
  uint32_t identifier = PROP_GetLongIdentifier(pPropertyChanged->Property);
  if (identifier != ID_ItemsControl_ItemsSource && identifier != ID_ItemsControl_ItemTemplate) return FALSE;
  if (pItemsControl->_started || pItemsControl->_observedSource || pItemsControl->_entries)
    ItemsControl_Synchronize(pItemsControl);
  return FALSE;
}

HANDLER(ItemsControl, DataObject, CollectionChanged) {
  if (pCollectionChanged->Source != pItemsControl->_observedSource) return FALSE;
  ItemsControl_Synchronize(pItemsControl);
  return TRUE;
}

HANDLER(ItemsControl, ItemsControl, Refresh) {
  ItemsControl_Synchronize(pItemsControl);
  return TRUE;
}

HANDLER(ItemsControl, Object, Destroy) {
  if (pItemsControl->_observedSource) {
    core_UnobserveCollection(pItemsControl->_observedSource, hObject);
    OBJ_ReleaseRef(pItemsControl->_observedSource);
    pItemsControl->_observedSource = NULL;
  }
  while (pItemsControl->_entries) {
    struct ItemsControlEntry *entry = pItemsControl->_entries;
    pItemsControl->_entries = entry->next;
    ItemsControl_ReleaseEntry(hObject, entry);
  }
  return TRUE;
}

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
      struct Object *data = CMP_GetObject(itemNode->DataContext);
      PROP_SetValue(OBJ_FindLongProperty(hObject, ID_ItemsControl_SelectedItem), &data);
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
        struct Object *data = CMP_GetObject(itemNode->DataContext);
        PROP_SetValue(OBJ_FindLongProperty(hObject, ID_ItemsControl_SelectedItem), &data);
      }
    } else {
      ListBox_SetItemSelected(child, FALSE);
    }
  }
}

HANDLER(ListBox, ItemsControl, ItemsChanged) {
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
