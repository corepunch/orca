#include "../ed_local.h"

#define SYM_EXPAND 20
#define SYM_COLLAPSE 21
#define MAX_EXPANDED_ITEMS 256
#define MAX_FILTER_TEXT 256
#define MAX_TREEVIEW_ITEMS 256

typedef struct _TREEITEM {
  SHORTSTR name;
  HANDLE   identifier;
  DWORD    indent;
  DWORD    position;
  DWORD    type;
  FLOAT    location;
  LPVOID   object;
} TREEITEM, *LPEDPROJITEM;

typedef struct _TREEVIEW {
  DWORD          expandedItems[MAX_EXPANDED_ITEMS];
  CHAR           filtertext[MAX_FILTER_TEXT];
  TREEITEM       items[MAX_TREEVIEW_ITEMS];
  BYTE           numitems;
  LPTREEVIEWDATA dataSource;
  LPVOID         rootItem;
  struct {
    DWORD  type;
    DWORD  type2;
    HANDLE parent;
    BOOL   active;
  } newitem;
  struct {
    LPVOID selectedObject;
    DWORD  selectedIndex;
  } restore;
} *LPTREEVIEW;

struct _Iterator {
  HEDWND     wnd;
  LPTREEVIEW treeview;
  DWORD      indent;
  DWORD      item;
  BOOL       noExpandButton;
};

static DWORD
ED_AddTreeViewItem(HEDWND wnd, LPOBJDEF def, DWORD indent)
{
  LPTREEVIEW treeview = ED_GetUserData(wnd);
  LPEDPROJITEM item = &treeview->items[treeview->numitems];
  if (def->szName) {
    strncpy(item->name, def->szName, sizeof(item->name));
  } else {
    strncpy(item->name, "(null)", sizeof(item->name));
  }
  item->identifier = def->hIdentifier;
  item->type = 0;
  item->location = (Con_GetCurrentLine(ED_GetClient(wnd))+1) * CONSOLE_CHAR_HEIGHT;
  item->position = ED_GetClient(wnd)->cursor;
  item->indent = indent;
  item->object = def->lpObjectItself;
  return treeview->numitems++;
}

static BOOL ED_FindProjItem(HEDWND wnd, HANDLE ident, BYTE *index) {
  LPTREEVIEW treeview = ED_GetUserData(wnd);
  FOR_LOOP(i, treeview->numitems) {
    if (treeview->items[i].identifier == ident) {
      *index = i;
      return TRUE;
    }
  }
  return FALSE;
}

static BOOL TreeView_IsItemExpanded(LPTREEVIEW treeview, DWORD id) {
  FOR_LOOP(i, MAX_EXPANDED_ITEMS) {
    if (treeview->expandedItems[i] == id)
      return TRUE;
  }
  return FALSE;
}

void TreeView_SetItemExpanded(LPTREEVIEW treeview, DWORD id, BOOL value) {
  if (value) {
    TreeView_SetItemExpanded(treeview, id, FALSE);
    FOR_LOOP(i, MAX_EXPANDED_ITEMS) {
      if (treeview->expandedItems[i] == 0) {
        treeview->expandedItems[i] = id;
        return;
      }
    }
  } else {
    FOR_LOOP(i, MAX_EXPANDED_ITEMS) {
      if (treeview->expandedItems[i] == id) {
        treeview->expandedItems[i] = 0;
      }
    }
  }
}

static void TreeView_DrawExpandButton(HEDWND wnd, BOOL bHasChildren, DWORD itemnum) {
  LPTREEVIEW treeview = ED_GetUserData(wnd);
  if (!bHasChildren) {
    ED_Echo(wnd, "%*s", EXPAND_BUTTON_WIDTH, "");
  } else if (TreeView_IsItemExpanded(treeview, treeview->items[itemnum].identifier)) {
    ED_Echo(wnd, FMT_COMMAND "\x1c\x1d", itemnum);
  } else {
    ED_Echo(wnd, FMT_COMMAND "\x1e\x1f", itemnum);
  }
}

static void TreeView_PrintItem(LPOBJDEF lpObjDef, LPVOID lpParam) {
  struct _Iterator *it = lpParam;
  if (Con_CheckOverflow(ED_GetClient(it->wnd)))
    return;
  Echo(ED_GetClient(it->wnd), STYLE_TREEVIEW_STYLE(lpObjDef->dwStyle));
  Echo(ED_GetClient(it->wnd), FMT_COMMAND, it->item);
  Echo(ED_GetClient(it->wnd), "%*s", it->indent, "");
  TreeView_DrawExpandButton(it->wnd, lpObjDef->bHasChildren, it->item);
  Echo(ED_GetClient(it->wnd), FMT_COMMAND, it->item);
//  if (lpObjDef->bHasAddButton) {
//    DWORD trail = it->wnd->Rect.width/CONSOLE_CHAR_WIDTH - it->indent - EXPAND_BUTTON_WIDTH - 1;
//    Echo(ED_GetClient(it->wnd), "%-*s\x12\x13\n", trail, lpObjDef->szName);
//  } else {
    DWORD trail = ED_GetClient(it->wnd)->width - it->indent - EXPAND_BUTTON_WIDTH - 1;
    Echo(ED_GetClient(it->wnd), "%-*s\n", trail, lpObjDef->szName);
//  }
}

static void TreeView_PrintFilteredItem(LPOBJDEF lpObjDef, LPVOID lpParam) {
  struct _Iterator *it = lpParam;
  if (Con_CheckOverflow(ED_GetClient(it->wnd)))
    return;
  DWORD item = ED_AddTreeViewItem(it->wnd, lpObjDef, 0);
  TreeView_PrintItem(lpObjDef, &(struct _Iterator) {
    .wnd = it->wnd,
    .treeview = it->treeview,
    .item = item,
  });
}

static void TreeView_SelectFirstItem(LPOBJDEF lpObjDef, LPVOID lpParam) {
  struct _Iterator *it = lpParam;
  if (it->indent == 0) {
    ED_SendMessage(it->wnd, EVT_COMMAND, TREE_ON_SELECT, lpObjDef->lpObjectItself);
    it->indent++;
  }
}

static void TreeView_PrintChild(LPOBJDEF lpObjDef, LPVOID lpParam) {
  struct _Iterator *it = lpParam;
  LPTREEVIEW tv = it->treeview;
  if (Con_CheckOverflow(ED_GetClient(it->wnd)))
    return;
  DWORD item = ED_AddTreeViewItem(it->wnd, lpObjDef, it->indent);
  BOOL newitemHere = tv->newitem.active && tv->newitem.parent == lpObjDef->hIdentifier;
  
  if (newitemHere) {
    lpObjDef->bHasChildren = TRUE;
  }
  
  TreeView_PrintItem(lpObjDef, &(struct _Iterator) {
    .wnd = it->wnd,
    .treeview = it->treeview,
    .indent = it->indent,
    .item = item,
  });
  
  if (newitemHere) {
    Echo(ED_GetClient(it->wnd), "\n");
  }
  
  if (TreeView_IsItemExpanded(it->treeview, lpObjDef->hIdentifier)) {
    it->treeview->dataSource->EnumChildObjects(lpObjDef->lpObjectItself,
                                               TreeView_PrintChild,
                                               &(struct _Iterator) {
      .wnd = it->wnd,
      .treeview = it->treeview,
      .indent = it->indent + 1,
    });
  }
}

static void TreeView_Draw(HEDWND wnd, LPTREEVIEW treeview) {
  struct _Iterator it = { .wnd = wnd, .treeview = treeview, .indent = 0 };
  if (!treeview->dataSource) {
    ED_Echo(wnd, STYLE_LABEL "Data source is not set");
    return;
  }
  memset(treeview->items, 0, sizeof(treeview->items));
  treeview->numitems = 1;
  if (editor.textEdit.active && editor.textEdit.dest == treeview->filtertext) {
    ED_TextField(ED_GetClient(wnd));
    treeview->dataSource->FilterObjects(treeview->rootItem, editor.textEditBuffer, TreeView_PrintFilteredItem, &it);
  } else {
    if (treeview->newitem.active && treeview->newitem.parent == -1) {
      ED_Echo(wnd, "\n");
    }
    if (treeview->rootItem) {
      struct _OBJDEF def;
      UI_GetObjectItem(treeview->rootItem, &def);
      TreeView_PrintChild(&def, &it);
    } else {
      treeview->dataSource->EnumChildObjects(treeview->rootItem, TreeView_PrintChild, &it);
    }
  }
}

static void TreeView_RefreshSelectedItem(HEDWND wnd, LPTREEVIEW treeview, LPEDPROJITEM item) {
  if (!item->identifier) {
    ED_SetSelectedItem(wnd, -1);
    ED_SendMessage(wnd, EVT_COMMAND, MAKEDWORD(TREE_ON_SELECT, -1), NULL);
    return;
  }
  // erase previous highlight
  ED_SetSelectedItem(wnd, (DWORD)(item - treeview->items));
  ED_ScrollToLocation(wnd, item->location);
  ED_SendMessage(wnd, EVT_COMMAND, MAKEDWORD(TREE_ON_SELECT, item-treeview->items), item->object);
}

static LPVOID TreeView_GetObject(LPTREEVIEW treeview, DWORD item_id) {
  FOR_LOOP(i, MAX_TREEVIEW_ITEMS) {
    if (treeview->items[i].identifier == item_id) {
      return treeview->items[i].object;
    }
  }
  return NULL;
}

static void TreeView_RenameCallback(TEXTEDITEVENT evt, lParam_t lpParm) {
  HEDWND wnd = lpParm;
  LPTREEVIEW treeview = ED_GetUserData(lpParm);
  LPTREEVIEWDATA dataSource = treeview->dataSource;
  if (evt == TEXTEDIT_ACCEPT) {
    LPVOID obj = TreeView_GetObject(treeview, editor.textEdit.pindex);
    dataSource->SetObjectName(obj, editor.textEditBuffer);
    ED_InvalidateWindow(wnd);
  }
}

static void TreeView_CreateCallback(TEXTEDITEVENT evt, lParam_t lpParm) {
  HEDWND wnd = lpParm;
  LPTREEVIEW treeview = ED_GetUserData(lpParm);
  LPTREEVIEWDATA dataSource = treeview->dataSource;
  if (evt == TEXTEDIT_CANCEL) {
    memset(&treeview->newitem, 0, sizeof(treeview->newitem));
    ED_InvalidateWindow(wnd);
  } else if (evt == TEXTEDIT_ACCEPT) {
    LPVOID obj = TreeView_GetObject(treeview, editor.textEdit.pindex);
    DWORD item_id = dataSource->NewObject(obj?obj:treeview->rootItem, editor.textEditBuffer, treeview->newitem.type);
    memset(&treeview->newitem, 0, sizeof(treeview->newitem));
    ED_ClearConsole(wnd);
    TreeView_Draw(wnd, treeview);
    for (int i = 0; i < MAX_TREEVIEW_ITEMS; i++) {
      if (treeview->items[i].identifier == item_id) {
        ED_SetSelectedItem(wnd, i);
        ED_SendMessage(wnd, EVT_COMMAND, MAKEDWORD(TREE_ON_SELECT, i), treeview->items[i].object);
      }
    }
  }
}

void TreeView_NewItem(HEDWND wnd, LPTREEVIEW treeview, DWORD item_id, DWORD type) {
  void TreeView_CreateCallback(TEXTEDITEVENT evt, lParam_t lpParm);
  DWORD itmpos = EXPAND_BUTTON_WIDTH, offs = 0;
  LPEDPROJITEM item = NULL;
  FOR_LOOP(i, MAX_TREEVIEW_ITEMS) {
    if (treeview->items[i].identifier == item_id) {
      item = &treeview->items[i];
    }
  }
  treeview->newitem.type = type;
  treeview->newitem.parent = item_id;
  treeview->newitem.active = TRUE;
  ED_ClearConsole(wnd);
  if (item) {
    TreeView_SetItemExpanded(treeview, item->identifier, TRUE);
    itmpos += item->position + item->indent;
    offs += ED_GetClient(wnd)->width+1;
  }
  TreeView_Draw(wnd, treeview);
  DWORD end = (itmpos/ED_GetClient(wnd)->width+1)*ED_GetClient(wnd)->width;
  ED_SetTextInput(&(EDTEXTEDIT) {
    .dest = "Noname",
    .type = kDataTypeFixed,
    .pindex = item_id,
    .bounds = MAKEDWORD(itmpos+offs, end-itmpos-2),
    .callback = TreeView_CreateCallback,
    .parm = wnd,
  });
}

void TreeView_CreateItem(HEDWND wnd, DWORD type) {
  DWORD TreeView_GetIdentifierAtIndex(struct _TREEVIEW *, WORD);
  void TreeView_SetItemExpanded(struct _TREEVIEW *, DWORD, BOOL);
  struct _TREEVIEW *treeview = ED_GetUserData(wnd);
  DWORD id = TreeView_GetIdentifierAtIndex(treeview, ED_GetSelectedItem(wnd));
  TreeView_SetItemExpanded(treeview, id, TRUE);
  TreeView_NewItem(wnd, treeview, id, type);
}

static BOOL
TreeView_OnContext(HEDWND wnd, LPTREEVIEW treeview, DWORD wparam, LPTERMINALCHAR data)
{
  LPEDPROJITEM item = &treeview->items[data->item];
  TreeView_RefreshSelectedItem(wnd, treeview, item);
//  TreeView_SetItemExpanded(treeview, item->identifier, TRUE);
//  ED_InvalidateWindow(wnd);
//  ED_SetFocusedPanel(ED_CreateWindow(ED_SelectClass, &rect, NULL, wnd));
  return TRUE;
}

static BOOL
TreeView_OnClick(HEDWND wnd, LPTREEVIEW treeview, DWORD _, LPTERMINALCHAR data)
{
  LPEDPROJITEM item = &treeview->items[data->item];
  switch (data->character) {
    case 0x1e:
    case 0x1f:
      TreeView_SetItemExpanded(treeview, item->identifier, TRUE);
      ED_InvalidateWindow(wnd);
      break;
    case 0x1c:
    case 0x1d:
      TreeView_SetItemExpanded(treeview, item->identifier, FALSE);
      ED_InvalidateWindow(wnd);
      break;
//    case 0x12:
//    case 0x13:
//      TreeView_NewItem(wnd, treeview, data->item, PROJITEM_SCENE);
//      break;
    default:
      if (data->item) {
        TreeView_RefreshSelectedItem(wnd, treeview, item);
      }
      break;
  }
  return TRUE;
}

static BOOL
TreeView_OnStartDrag(HEDWND wnd, LPTREEVIEW treeview, DWORD _, LPTERMINALCHAR data)
{
  LPEDPROJITEM item = &treeview->items[data->item];
  struct _OBJDEF objdef;
  if (treeview->dataSource->GetObjectItem(item->object, &objdef)) {
    ED_SetDragData(objdef.dwItemType, objdef.szName, item->object, item->indent+2);
    return TRUE;
  } else {
    return FALSE;
  }
}

LPVOID TreeView_GetItem(HEDWND wnd, WORD item) {
  LPTREEVIEW treeview = ED_GetUserData(wnd);
  if (item < treeview->numitems) {
    return treeview->items[item].object;
  } else {
    return NULL;
  }
}

static void _NextItem(HEDWND wnd, LPTREEVIEW treeview,  BYTE index) {
  if (index < treeview->numitems - 1) {
    LPEDPROJITEM item = item = &treeview->items[++index];
    TreeView_RefreshSelectedItem(wnd, treeview, item);
  }
}

static void _PrevItem(HEDWND wnd, LPTREEVIEW treeview, BYTE index) {
  if (index > 1) {
    LPEDPROJITEM item = item = &treeview->items[--index];
    TreeView_RefreshSelectedItem(wnd, treeview, item);
  }
}

static void TreeView_RevealItem(LPTREEVIEW treeview, WORD index) {
  LPTREEVIEWDATA dataSource = treeview->dataSource;
  for (LPVOID h = treeview->items[index].object; h;) {
    struct _OBJDEF objdef;
    if (dataSource->GetObjectItem(h, &objdef)) {
      TreeView_SetItemExpanded(treeview, objdef.hIdentifier, TRUE);
      FOR_LOOP(i, MAX_TREEVIEW_ITEMS) {
        if (treeview->items[i].identifier == objdef.hParent) {
          h = treeview->items[i].object;
        }
      }
    } else {
      break;
    }
  }
}

static void TreeView_FilterCallack(TEXTEDITEVENT evt, lParam_t lpParm) {
  HEDWND wnd = lpParm;
  LPTREEVIEW treeview = ED_GetUserData(lpParm);
  LPTREEVIEWDATA dataSource = treeview->dataSource;
  switch (evt) {
    case TEXTEDIT_DOWN:
      _NextItem(lpParm, treeview, ED_GetSelectedItem(wnd));
      break;
    case TEXTEDIT_UP:
      _PrevItem(lpParm, treeview, ED_GetSelectedItem(wnd));
      break;
    case TEXTEDIT_CANCEL:
      ED_SendMessage(wnd, EVT_COMMAND, MAKEDWORD(TREE_ON_SELECT, 0), treeview->restore.selectedObject);
      ED_SetSelectedItem(wnd, treeview->restore.selectedIndex);
      ED_InvalidateWindow(wnd);
      break;
    case TEXTEDIT_ACCEPT:
      TreeView_RevealItem(treeview, ED_GetSelectedItem(wnd));
      ED_InvalidateWindow(wnd);
      break;
    case TEXTEDIT_ERASE:
    case TEXTEDIT_INPUT:
      dataSource->FilterObjects(treeview->rootItem,
                                editor.textEditBuffer,
                                TreeView_SelectFirstItem,
                                &(struct _Iterator) {
        .wnd = wnd,
        .indent = 0
      });
      ED_InvalidateWindow(wnd);
      break;
  }
}

static LRESULT
TreeView_OnCommand(HEDWND wnd, LPTREEVIEW treeview, wParam_t cmd)
{
  BYTE index = ED_GetSelectedItem(wnd);
  switch (cmd) {
    case ID_NAV_NEXT: {
      EDWINPROC(Inspector);
      HEDWND inspector = ED_FindWindow(ED_Inspector);
      if (inspector) {
        ED_SetFocusedPanel(inspector);
      }
      return TRUE;
    }
    case ID_EDIT_FILTER:
      ED_ResetScroll(wnd);
      treeview->restore.selectedObject = NULL;//editor.selectedObject;
      treeview->restore.selectedIndex = ED_GetSelectedItem(wnd);
      memset(treeview->filtertext, 0, sizeof(treeview->filtertext));
      ED_SetTextInput(&(EDTEXTEDIT) {
        .dest = treeview->filtertext,
        .type = kDataTypeFixed,
        .placeholder = "Enter filter text",
        .bounds = ED_GetClient(wnd)->width << 16,
        .callback = TreeView_FilterCallack,
        .parm = wnd
      });
      ED_InvalidateWindow(wnd);
      return TRUE;
  }
  
  if (!ED_FindProjItem(wnd, treeview->items[index].identifier, &index)) {
    if (cmd == ID_NAV_NEXTROW) {
      index = -1;
      _NextItem(wnd, treeview, index);
      return TRUE;
    } else {
      return FALSE;
    }
  }
  switch (cmd) {
    case ID_NAV_NEXTROW:
      _NextItem(wnd, treeview, index);
      return TRUE;
    case ID_NAV_PREVROW:
      _PrevItem(wnd, treeview, index);
      return TRUE;
    case ID_NAV_NEXTCOL:
      TreeView_SetItemExpanded(treeview, treeview->items[index].identifier, TRUE);
      ED_InvalidateWindow(wnd);
      return TRUE;
    case ID_NAV_PREVCOL:
      TreeView_SetItemExpanded(treeview, treeview->items[index].identifier, FALSE);
      ED_InvalidateWindow(wnd);
      return TRUE;
    case ID_EDIT_TOGGLE: {
      LPEDPROJITEM item = item = &treeview->items[index];
      struct _PROPDEF visible;
      if (treeview->dataSource->GetObjectProperty(item->object,"QuickHide", &visible)) {
        BOOL toggled = !*(BOOL *)visible.lpEditorValue;
        ED_StoreUndo(visible.lpEditorValue, &toggled, sizeof(BOOL));
      }
      ED_InvalidateWindow(wnd);
      return TRUE;
    }
    case ID_EDIT_RENAME: {
      LPEDPROJITEM item = &treeview->items[index];
      int itmpos = item->position+item->indent+EXPAND_BUTTON_WIDTH;
      int end = (itmpos/ED_GetClient(wnd)->width+1)*ED_GetClient(wnd)->width;
      ED_SetTextInput(&(EDTEXTEDIT) {
        .dest = item->name,
        .type = kDataTypeFixed,
        .pindex = item->identifier,
        .bounds = itmpos | ((end-itmpos)<<16),
        .callback = TreeView_RenameCallback,
        .parm = wnd,
      });
    }
  }
//  if (!strcmp(cmd, treeview_new_item)) {
//    TreeView_NewItem(wnd, treeview, treeview->items[index].identifier, 0);
//  }
  return FALSE;
}

LRESULT ED_TreeView(HEDWND wnd, DWORD msg, wParam_t wparm, lParam_t lparm) {
  LPTREEVIEW treeview = ED_GetUserData(wnd);
  switch (msg) {
    case EVT_PRINT:
      TreeView_Draw(wnd, treeview);
      return TRUE;
    case EVT_CONTEXTMENU:
      ED_SetFocusedPanel(wnd);
      return treeview->dataSource ? TreeView_OnContext(wnd, treeview, wparm, lparm) : 0;
    case EVT_CCLICK:
      ED_SetFocusedPanel(wnd);
      return treeview->dataSource ? TreeView_OnClick(wnd, treeview, wparm, lparm) : 0;
    case EVT_CGRAB:
      return treeview->dataSource ? TreeView_OnStartDrag(wnd, treeview, wparm, lparm) : 0;
    case EVT_HOTKEY:
      return treeview->dataSource ? TreeView_OnCommand(wnd, treeview, wparm) : 0;
    case EVT_COMMAND:
      switch (LOWORD(wparm)) {
        case TREE_EXPAND:
          TreeView_SetItemExpanded(treeview, *(DWORD*)lparm, TRUE);
          return TRUE;
        case TREE_SELECT:
          FOR_LOOP(i, MAX_EXPANDED_ITEMS) {
            if (treeview->items[i].identifier == *(DWORD*)lparm) {
              ED_SetSelectedItem(wnd, i);
            }
          }
          return TRUE;
        default:
          return FALSE;
      }
  }
  return FALSE;
}

LPVOID ED_AllocateTreeViewData(LPTREEVIEWDATA lpDataSource, LPVOID root) {
  LPTREEVIEW userdata = ZeroAlloc(sizeof(struct _TREEVIEW));
  userdata->dataSource = lpDataSource;
  userdata->rootItem = root;
  return userdata;
}

DWORD TreeView_GetIdentifierAtIndex(LPTREEVIEW treeview, WORD index) {
  if (index == 0xffff) {
    return -1;
  }
  if (index > sizeof(treeview->items)/sizeof(*treeview->items)) {
    return -1;
  }
  return treeview->items[index].identifier;
}

void TreeView_SetRootItem(HEDWND wnd, LPVOID item) {
  LPTREEVIEW treeview = ED_GetUserData(wnd);
  treeview->rootItem = item;
}
