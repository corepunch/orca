#include "Layout.h"
#include <include/orca.h>
#include <core/core.h>

HANDLER(StackView, Node2D, MeasureOverride);

static struct Object *TreeView_Root(struct Object *item)
{
  for (struct Object *p = item; p; p = OBJ_GetParent(p))
    if (GetTreeView(p)) return p;
  return NULL;
}

static struct Object *TreeView_ParentItem(struct Object *item)
{
  for (struct Object *p = OBJ_GetParent(item); p; p = OBJ_GetParent(p)) {
    if (GetTreeViewItem(p)) return p;
    if (GetTreeView(p)) break;
  }
  return NULL;
}

static struct Object *TreeView_Host(struct Object *owner)
{
  struct ItemsControl *items = GetItemsControl(owner);
  return items && items->_itemsHost ? items->_itemsHost : owner;
}

typedef bool_t (*TreeVisit)(struct Object *, void *);

/* Collection ownership stays at each level; only container navigation crosses
   presenters. The root's ItemsControl never receives a descendant record. */
static struct Object *TreeView_Visit(struct Object *owner, bool_t visible, TreeVisit visit, void *arg)
{
  FOR_EACH_OBJECT(item, TreeView_Host(owner)) {
    struct TreeViewItem *treeItem = GetTreeViewItem(item);
    if (!treeItem) continue;
    if (visit(item, arg)) return item;
    if (!visible || treeItem->IsExpanded) {
      struct Object *found = TreeView_Visit(item, visible, visit, arg);
      if (found) return found;
    }
  }
  return NULL;
}

static bool_t TreeView_MatchesIdentity(struct Object *item, void *identity) { return item == identity; }

static char const *TreeView_Value(struct Object *item)
{
  struct TreeViewItem *treeItem = GetTreeViewItem(item);
  struct Property *value = OBJ_FindLongProperty(item, ID_TreeViewItem_SelectionValue);
  if (value) PROP_Update(value);
  return treeItem && treeItem->SelectionValue ? treeItem->SelectionValue : "";
}

static bool_t TreeView_MatchesValue(struct Object *item, void *value)
{
  return value && !strcmp(TreeView_Value(item), value);
}

static void TreeView_ApplySelection(struct Object *root, struct Object *selected);
static void HeaderedItemsControl_Build(struct Object *object);

static void HeaderedItemsControl_UpdateHasItems(struct Object *object)
{
  bool_t hasItems = FALSE;
  FOR_EACH_OBJECT(child, TreeView_Host(object)) {
    if (GetNode2D(child)) { hasItems = TRUE; break; }
  }
  PROP_SetValue(OBJ_FindLongProperty(object, ID_HeaderedItemsControl_HasItems), &hasItems);
}

static void TreeViewItem_SyncState(struct Object *object)
{
  struct TreeViewItem *item = GetTreeViewItem(object);
  struct HeaderedItemsControl *control = GetHeaderedItemsControl(object);
  if (!item || !control) return;
  if (control->_headerPresenter) {
    uint32_t flags = OBJ_GetFlags(control->_headerPresenter);
    OBJ_SetFlags(control->_headerPresenter, item->IsSelected ? flags | OF_SELECTED : flags & ~OF_SELECTED);
    _SendMessage(control->_headerPresenter, StyleController, ThemeChanged, .recursive = TRUE);
  }
  if (control->_itemsPresenter)
    PROP_SetValue(OBJ_FindLongProperty(control->_itemsPresenter, ID_Node_Visible), &item->IsExpanded);
  if (item->_expander) {
    GetNode2D(item->_expander)->IgnoreHitTest = !control->HasItems;
    struct Object *chevron = OBJ_FindChild(item->_expander, "Chevron", FALSE);
    if (chevron) PROP_SetValue(OBJ_FindLongProperty(chevron, ID_Node_Visible), &control->HasItems);
    uint32_t flags = OBJ_GetFlags(item->_expander);
    OBJ_SetFlags(item->_expander, item->IsExpanded ? flags | OF_CHECKED : flags & ~OF_CHECKED);
    _SendMessage(item->_expander, StyleController, ThemeChanged, .recursive = TRUE);
  }
  OBJ_SetDirty(object);
}

static struct Object *TreeView_Part(struct Object *parent, uint32_t type, char const *name, char const *css)
{
  struct Object *part = OBJ_Create(type);
  if (!part) return NULL;
  OBJ_SetName(part, name);
  _SendMessage(part, StyleController, AddClass, .ClassName = css);
  OBJ_AddChild(parent, part);
  OBJ_ReleaseRef(part);
  return part;
}

static void TreeViewItem_InheritTemplate(struct Object *object)
{
  struct TreeViewItem *item = GetTreeViewItem(object);
  if (!item) return;
  struct ItemsControl *items = GetItemsControl(object), *parent = NULL;
  for (struct Object *p = OBJ_GetParent(object); p; p = OBJ_GetParent(p)) {
    if (GetTreeView(p) || GetTreeViewItem(p)) { parent = GetItemsControl(p); break; }
  }
  if (parent && (!items->ItemTemplate || items->ItemTemplate == item->_inheritedTemplate)) {
    item->_inheritedTemplate = parent->ItemTemplate;
    struct Object *template = parent->ItemTemplate ? CMP_GetObject(parent->ItemTemplate) : NULL;
    PROP_SetValue(OBJ_FindLongProperty(object, ID_ItemsControl_ItemTemplate), &template);
  }
  if (parent && (!items->ItemNamePrefix || !*items->ItemNamePrefix) && parent->ItemNamePrefix)
    PROP_SetStringValue(OBJ_FindLongProperty(object, ID_ItemsControl_ItemNamePrefix), parent->ItemNamePrefix);
  struct Object *root = TreeView_Root(object);
  if (root) {
    char const *property = GetTreeView(root)->ValueProperty;
    char expression[MAX_PROPERTY_STRING];
    snprintf(expression, sizeof(expression), "{DataContext/%s}", property && *property ? property : "Name");
    OBJ_AttachPropertyProgram(object, "TreeViewItem.SelectionValue", expression, kBindingModeOneWay, TRUE);
  }
}

static void HeaderedItemsControl_Build(struct Object *object)
{
  struct HeaderedItemsControl *control = GetHeaderedItemsControl(object);
  if (!control || control->_building || !OBJ_GetParent(object)) return;
  control->_building = TRUE;
  struct TreeViewItem *item = GetTreeViewItem(object);
  if (!control->_headerPresenter) {
    control->_headerPresenter = TreeView_Part(object, item ? ID_TreeViewHeader : ID_Grid,
      "HeaderPresenter", "treeview-header");
    control->_itemsPresenter = TreeView_Part(object, ID_ItemsPresenter, "ItemsPresenter", "treeview-items");
    GetItemsControl(object)->_itemsHost = control->_itemsPresenter;
    if (control->_headerPresenter) {
      PROP_SetStringValue(OBJ_FindLongProperty(control->_headerPresenter, ID_Grid_Rows), "1fr");
      PROP_SetStringValue(OBJ_FindLongProperty(control->_headerPresenter, ID_Grid_Columns), item ? "16px 1fr" : "1fr");
      if (item) {
        item->_expander = TreeView_Part(control->_headerPresenter, ID_TreeViewExpander, "Expander", "treeview-expander");
        struct Object *chevron = TreeView_Part(item->_expander, ID_VectorView, "Chevron", "treeview-chevron");
        if (chevron) GetNode2D(chevron)->IgnoreHitTest = TRUE;
      }
    }
  }
  /* Authored children may be attached after the native parts already exist. */
  for (struct Object *child = OBJ_GetFirstChild(object), *next; child; child = next) {
    next = OBJ_GetNext(child);
    if (child != control->_headerPresenter && child != control->_itemsPresenter && GetNode2D(child))
      OBJ_AddChild(control->_itemsPresenter, child);
  }
  struct Object *header = control->Header ? CMP_GetObject(control->Header) : NULL;
  if (control->_headerContent != header) {
    if (control->_headerContent && OBJ_GetParent(control->_headerContent) == control->_headerPresenter)
      OBJ_RemoveFromParent(control->_headerContent);
    control->_headerContent = header;
    if (header) OBJ_AddChild(control->_headerPresenter, header);
  }
  control->_building = FALSE;
  HeaderedItemsControl_UpdateHasItems(object);
  TreeViewItem_SyncState(object);
}

static void TreeView_EnsureLevel(struct Object *owner)
{
  /* Item templates have no data scope until ItemsControl supplies a record and
     attaches the container. Do not cache a missing child binding beforehand. */
  if (GetHeaderedItemsControl(owner) && !OBJ_GetParent(owner)) return;
  HeaderedItemsControl_Build(owner);
  struct Property *source = OBJ_FindLongProperty(owner, ID_ItemsControl_ItemsSource);
  if (source) PROP_Update(source);
  struct ItemsControl *items = GetItemsControl(owner);
  if (items && !items->_started) _SendMessage(owner, ItemsControl, Refresh);
}

static void TreeView_EnsureItems(struct Object *owner)
{
  TreeView_EnsureLevel(owner);
  FOR_EACH_OBJECT(item, TreeView_Host(owner))
    if (GetTreeViewItem(item)) TreeView_EnsureItems(item);
}

static void TreeView_Ensure(struct Object *root)
{
  struct TreeView *tree = GetTreeView(root);
  if (!tree || tree->_ensuring) return;
  tree->_ensuring = TRUE;
  TreeView_EnsureItems(root);
  tree->_ensuring = FALSE;
}

static void TreeView_ApplySelectionFrom(struct Object *root, struct Object *selected, char const *oldValue)
{
  struct TreeView *tree = GetTreeView(root);
  if (!tree || tree->_changing) return;
  tree->_changing = TRUE;
  char const *value = selected ? TreeView_Value(selected) : "";
  struct Object *data = selected && GetNode(selected)->DataContext
    ? CMP_GetObject(GetNode(selected)->DataContext) : NULL;
  struct ItemsControl *items = GetItemsControl(root);
  struct Object *oldData = items->SelectedItem ? CMP_GetObject(items->SelectedItem) : NULL;
  if (selected == tree->_selectedContainer && !strcmp(tree->SelectedValue ? tree->SelectedValue : "", value) && oldData == data) {
    tree->_changing = FALSE;
    return;
  }
  char *previous = strdup(oldValue ? oldValue : tree->SelectedValue ? tree->SelectedValue : "");
  bool_t changed = selected != tree->_selectedContainer;
  struct Object *previousContainer = tree->_selectedContainer
    ? TreeView_Visit(root, FALSE, TreeView_MatchesIdentity, tree->_selectedContainer) : NULL;
  tree->_selectedContainer = selected;
  changed = changed || strcmp(previous, value);
  PROP_SetStringValue(OBJ_FindLongProperty(root, ID_TreeView_SelectedValue), value);
  PROP_SetValue(OBJ_FindLongProperty(root, ID_ItemsControl_SelectedItem), &data);
  bool_t no = FALSE, yes = TRUE;
  if (previousContainer && previousContainer != selected)
    PROP_SetValue(OBJ_FindLongProperty(previousContainer, ID_TreeViewItem_IsSelected), &no);
  if (selected) PROP_SetValue(OBJ_FindLongProperty(selected, ID_TreeViewItem_IsSelected), &yes);
  tree->_changing = FALSE;
  OBJ_SetDirty(root);
  if (changed) {
    struct TreeView_SelectionChangedEventArgs args = { .SelectedValue = tree->SelectedValue, .OldValue = previous };
    axPostMessageDataW(root, ID_TreeView_SelectionChanged, 0, &args, sizeof(args));
  }
  free(previous);
}

static void TreeView_ApplySelection(struct Object *root, struct Object *selected)
{
  TreeView_ApplySelectionFrom(root, selected, NULL);
}

static void TreeView_SyncSelection(struct Object *root)
{
  struct TreeView *tree = GetTreeView(root);
  if (!tree || tree->_changing || tree->_ensuring) return;
  struct Object *selected = tree->_selectedContainer
    ? TreeView_Visit(root, FALSE, TreeView_MatchesIdentity, tree->_selectedContainer) : NULL;
  if (!selected && tree->SelectedValue && *tree->SelectedValue)
    selected = TreeView_Visit(root, FALSE, TreeView_MatchesValue, (void *)tree->SelectedValue);
  TreeView_ApplySelection(root, selected);
}

static void TreeView_Reveal(struct Object *item)
{
  bool_t expanded = TRUE;
  for (struct Object *parent = TreeView_ParentItem(item); parent; parent = TreeView_ParentItem(parent))
    PROP_SetValue(OBJ_FindLongProperty(parent, ID_TreeViewItem_IsExpanded), &expanded);
}

HANDLER(HeaderedItemsControl, Object, Start)
{
  TreeView_EnsureLevel(hObject);
  return FALSE;
}

HANDLER(HeaderedItemsControl, Object, Attached)
{
  TreeView_EnsureLevel(hObject);
  return FALSE;
}

HANDLER(HeaderedItemsControl, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property || pHeaderedItemsControl->_building) return FALSE;
  uint32_t id = PROP_GetLongIdentifier(pPropertyChanged->Property);
  if (id == ID_HeaderedItemsControl_Header || id == ID_HeaderedItemsControl_HeaderHeight ||
      id == ID_HeaderedItemsControl_Indent) {
    HeaderedItemsControl_Build(hObject);
    OBJ_SetDirty(hObject);
  }
  return FALSE;
}

HANDLER(HeaderedItemsControl, ItemsControl, ItemsChanged)
{
  HeaderedItemsControl_UpdateHasItems(hObject);
  TreeViewItem_SyncState(hObject);
  struct Object *root = TreeView_Root(hObject);
  if (root) TreeView_SyncSelection(root);
  return TRUE;
}

HANDLER(HeaderedItemsControl, Node2D, MeasureOverride)
{
  TreeView_EnsureLevel(hObject);
  float headerHeight = MAX(0, pHeaderedItemsControl->HeaderHeight);
  struct Size header = pHeaderedItemsControl->_headerPresenter
    ? Layout_Measure(pHeaderedItemsControl->_headerPresenter, .Width = pMeasureOverride->Width, .Height = headerHeight)
    : (struct Size){0};
  struct TreeViewItem *item = GetTreeViewItem(hObject);
  struct Size children = {0};
  if (pHeaderedItemsControl->_itemsPresenter && (!item || item->IsExpanded))
    children = Layout_Measure(pHeaderedItemsControl->_itemsPresenter,
      .Width = MAX(0, pMeasureOverride->Width - pHeaderedItemsControl->Indent), .Height = INFINITY);
  return Layout_Return(pMeasureOverride, MAX(header.width, children.width + pHeaderedItemsControl->Indent),
    headerHeight + children.height);
}

HANDLER(HeaderedItemsControl, Node2D, ArrangeOverride)
{
  float headerHeight = MAX(0, pHeaderedItemsControl->HeaderHeight);
  if (pHeaderedItemsControl->_headerPresenter)
    Layout_Arrange(pHeaderedItemsControl->_headerPresenter, .X = pArrangeOverride->X, .Y = pArrangeOverride->Y,
      .Width = pArrangeOverride->Width, .Height = headerHeight);
  struct TreeViewItem *item = GetTreeViewItem(hObject);
  if (pHeaderedItemsControl->_itemsPresenter && (!item || item->IsExpanded))
    Layout_Arrange(pHeaderedItemsControl->_itemsPresenter,
      .X = pArrangeOverride->X + pHeaderedItemsControl->Indent, .Y = pArrangeOverride->Y + headerHeight,
      .Width = MAX(0, pArrangeOverride->Width - pHeaderedItemsControl->Indent),
      .Height = MAX(0, pArrangeOverride->Height - headerHeight));
  return Layout_Return(pArrangeOverride, pArrangeOverride->Width, pArrangeOverride->Height);
}

HANDLER(TreeViewItem, Object, Start)
{
  HeaderedItemsControl_Build(hObject);
  if (OBJ_GetParent(hObject)) TreeViewItem_InheritTemplate(hObject);
  return FALSE;
}

HANDLER(TreeViewItem, Object, Attached)
{
  HeaderedItemsControl_Build(hObject);
  TreeViewItem_InheritTemplate(hObject);
  return FALSE;
}

HANDLER(TreeViewItem, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property) return FALSE;
  uint32_t id = PROP_GetLongIdentifier(pPropertyChanged->Property);
  if (id == ID_TreeViewItem_IsExpanded || id == ID_TreeViewItem_IsSelected) TreeViewItem_SyncState(hObject);
  struct Object *root = TreeView_Root(hObject);
  struct TreeView *tree = GetTreeView(root);
  if (!tree || tree->_changing || tree->_ensuring) return FALSE;
  if (id == ID_TreeViewItem_IsSelected) {
    if (pTreeViewItem->IsSelected) { TreeView_Reveal(hObject); TreeView_ApplySelection(root, hObject); }
    else if (tree->_selectedContainer == hObject) TreeView_ApplySelection(root, NULL);
  }
  if (id == ID_TreeViewItem_SelectionValue) TreeView_SyncSelection(root);
  if (id == ID_TreeViewItem_IsExpanded && !pTreeViewItem->IsExpanded) {
    struct Object *selected = TreeView_Visit(root, FALSE, TreeView_MatchesIdentity, tree->_selectedContainer);
    for (struct Object *p = selected ? TreeView_ParentItem(selected) : NULL; p; p = TreeView_ParentItem(p))
      if (p == hObject) { TreeView_ApplySelection(root, hObject); break; }
  }
  return FALSE;
}

HANDLER(TreeViewItem, TreeViewItem, ToggleExpanded)
{
  bool_t expanded = !pTreeViewItem->IsExpanded;
  if (GetHeaderedItemsControl(hObject)->HasItems)
    PROP_SetValue(OBJ_FindLongProperty(hObject, ID_TreeViewItem_IsExpanded), &expanded);
  return TRUE;
}

HANDLER(TreeViewHeader, Node, LeftButtonUp)
{
  struct Object *item = OBJ_GetParent(hObject), *root = TreeView_Root(item);
  if (root) { TreeView_ApplySelection(root, item); OBJ_SetFocus(item); }
  return TRUE;
}

HANDLER(TreeViewExpander, Node, LeftButtonUp)
{
  struct Object *item = OBJ_GetParent(OBJ_GetParent(hObject));
  if (GetTreeViewItem(item)) _SendMessage(item, TreeViewItem, ToggleExpanded);
  return TRUE;
}

HANDLER(TreeView, Object, Create)
{
  OBJ_SetStyle(hObject, OBJ_GetStyle(hObject) | OF_TABSTOP);
  GetItemsControl(hObject)->_selectionManaged = TRUE;
  return FALSE;
}

HANDLER(TreeView, ItemsControl, ItemsChanged)
{
  TreeView_SyncSelection(hObject);
  return TRUE;
}

static bool_t TreeView_Rebind(struct Object *item, void *unused)
{
  (void)unused;
  TreeViewItem_InheritTemplate(item);
  return FALSE;
}

HANDLER(TreeView, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property || pTreeView->_changing) return FALSE;
  uint32_t id = PROP_GetLongIdentifier(pPropertyChanged->Property);
  if (id == ID_TreeView_SelectedValue) {
    pTreeView->_changing = TRUE;
    char *requested = strdup(pTreeView->SelectedValue ? pTreeView->SelectedValue : "");
    struct Object *previous = TreeView_Visit(hObject, FALSE, TreeView_MatchesIdentity, pTreeView->_selectedContainer);
    char *oldValue = strdup(previous ? TreeView_Value(previous) : "");
    TreeView_Ensure(hObject);
    struct Object *item = *requested ? TreeView_Visit(hObject, FALSE, TreeView_MatchesValue, requested) : NULL;
    pTreeView->_changing = FALSE;
    if (item) TreeView_Reveal(item);
    TreeView_ApplySelectionFrom(hObject, item, oldValue);
    free(oldValue); free(requested);
  }
  if (id == ID_TreeView_ValueProperty) TreeView_Visit(hObject, FALSE, TreeView_Rebind, NULL);
  return FALSE;
}

HANDLER(TreeView, TreeView, SelectItem)
{
  if (!pSelectItem->Value) return FALSE;
  TreeView_Ensure(hObject);
  struct Object *item = TreeView_Visit(hObject, FALSE, TreeView_MatchesValue, (void *)pSelectItem->Value);
  if (!item) return FALSE;
  TreeView_Reveal(item);
  TreeView_ApplySelection(hObject, item);
  return TRUE;
}

static bool_t TreeView_SetExpanded(struct Object *item, void *expanded)
{
  PROP_SetValue(OBJ_FindLongProperty(item, ID_TreeViewItem_IsExpanded), expanded);
  return FALSE;
}

HANDLER(TreeView, TreeView, ExpandAll)
{
  TreeView_Ensure(hObject);
  TreeView_Visit(hObject, FALSE, TreeView_SetExpanded, &pExpandAll->IsExpanded);
  return TRUE;
}

struct TreeNavigation { struct Object *current, *previous, *next, *last; bool_t after; };

static bool_t TreeView_Navigate(struct Object *item, void *arg)
{
  struct TreeNavigation *nav = arg;
  if (nav->after && !nav->next) nav->next = item;
  if (item == nav->current) nav->after = TRUE;
  else if (!nav->after) nav->previous = item;
  nav->last = item;
  return FALSE;
}

static bool_t TreeView_First(struct Object *item, void *unused) { (void)item; (void)unused; return TRUE; }

HANDLER(TreeView, Node, KeyDown)
{
  TreeView_Ensure(hObject);
  TreeView_SyncSelection(hObject);
  struct Object *current = pTreeView->_selectedContainer, *next = NULL;
  struct TreeNavigation nav = { .current = current };
  TreeView_Visit(hObject, TRUE, TreeView_Navigate, &nav);
  switch (pKeyDown->keyCode) {
    case AX_KEY_UPARROW: next = current ? nav.previous : TreeView_Visit(hObject, TRUE, TreeView_First, NULL); break;
    case AX_KEY_DOWNARROW: next = current ? nav.next : TreeView_Visit(hObject, TRUE, TreeView_First, NULL); break;
    case AX_KEY_HOME: next = TreeView_Visit(hObject, TRUE, TreeView_First, NULL); break;
    case AX_KEY_END: next = nav.last; break;
    case AX_KEY_RIGHTARROW:
      if (current && GetHeaderedItemsControl(current)->HasItems) {
        bool_t expanded = TRUE;
        if (!GetTreeViewItem(current)->IsExpanded) PROP_SetValue(OBJ_FindLongProperty(current, ID_TreeViewItem_IsExpanded), &expanded);
        else next = TreeView_Visit(current, TRUE, TreeView_First, NULL);
      }
      break;
    case AX_KEY_LEFTARROW:
      if (current && GetTreeViewItem(current)->IsExpanded) {
        bool_t expanded = FALSE;
        PROP_SetValue(OBJ_FindLongProperty(current, ID_TreeViewItem_IsExpanded), &expanded);
      } else if (current) next = TreeView_ParentItem(current);
      break;
    default: return FALSE;
  }
  if (next) { TreeView_ApplySelection(hObject, next); OBJ_SetFocus(next); }
  return TRUE;
}

HANDLER(TreeViewItem, Node, KeyDown)
{
  struct Object *root = TreeView_Root(hObject);
  return root ? OBJ_SendMessageW(root, ID_Node_KeyDown, wParam, pKeyDown) : FALSE;
}

HANDLER(TreeView, Node2D, MeasureOverride)
{
  TreeView_EnsureLevel(hObject);
  return StackView_MeasureOverride(hObject, GetStackView(hObject), wParam, pMeasureOverride);
}

static struct Object *TreeView_NewContainer(struct Node2D *template)
{
  struct Object *header = template ? OBJ_Instantiate(CMP_GetObject(template)) : OBJ_Create(ID_TextBlock);
  if (!header || GetTreeViewItem(header)) return header;
  struct Object *container = OBJ_Create(ID_TreeViewItem);
  if (!container) { OBJ_ReleaseRef(header); return NULL; }
  if (!template)
    OBJ_AttachPropertyProgram(header, "TextRun.Text", "{DataContext/Name}", kBindingModeOneWay, TRUE);
  PROP_SetValue(OBJ_FindLongProperty(container, ID_HeaderedItemsControl_Header), &header);
  OBJ_ReleaseRef(header);
  return container;
}

HANDLER(TreeView, ItemsControl, CreateContainer)
{
  return (LRESULT)TreeView_NewContainer(pCreateContainer->Template);
}

HANDLER(TreeViewItem, ItemsControl, CreateContainer)
{
  return (LRESULT)TreeView_NewContainer(pCreateContainer->Template);
}
