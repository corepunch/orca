#include "../ed_local.h"

HOBJ SceneView_GetSelection(HEDWND);

#if 0

LPTREEVIEWDATA ED_GetHierarchyDataSource(void) {
  static TREEVIEWDATA data;
  data.EnumChildObjects = lpGame->EnumChildObjects;
  data.FilterObjects = lpGame->FilterObjects;
  data.GetObject = lpGame->GetObject;
  data.GetObjectProperty = lpGame->GetObjectProperty;
  data.SetObjectName = lpGame->RenameObject;
  data.NewObject = lpGame->NewObject;
  return &data;
}

LRESULT ED_HierarchyNavigator(HEDWND wnd, DWORD msg, wParam_t wparm, lParam_t lparm) {
  if (msg == EVT_CREATE){
    LPTREEVIEWDATA datasource = ED_GetHierarchyDataSource();
    LPVOID userdata = ED_AllocateTreeViewData(datasource);
    ED_SetUserData(wnd, userdata);
    return 1;
  } else {
    return ED_TreeView(wnd, msg, wparm, lparm);
  }
}

#else

uint32_t MapPointerTo32BitInt(void* ptr) {
  uint64_t address = (uint64_t)ptr;
  uint32_t upper = (uint32_t)(address >> 32);
  uint32_t lower = (uint32_t)(address & 0xFFFFFFFF);
  return upper ^ lower;
}

static BOOL _FindNode(LPXMLNODE root, HANDLE handle, LPXMLNODE *found) {
  if (handle == MapPointerTo32BitInt(root)) {
    *found = root;
    return TRUE;
  }
  xmlForEach(node, root) {
    if (_FindNode(node, handle, found)) {
      return TRUE;
    }
  }
  return FALSE;
}

BOOL XML_FindNode(HANDLE handle, LPXMLNODE *found) {
  //    FOR_EACH_LIST(EDPAK, pak, editor.project.packages) {
  //        if (_FindNode(xmlDocGetRootElement(pak->doc), handle, found))
  //            return TRUE;
  //    }
  if (_FindNode(editor.project.scene, handle, found))
    return TRUE;
  FOR_EACH_LIST(EDCACHE, cache, editor.project.cache) {
    if (_FindNode(xmlDocGetRootElement(cache->doc), handle, found))
      return TRUE;
  }
  return FALSE;
}


BOOL XML_HasChildren(LPXMLNODE node) {
  xmlForEach(child, node) {
    if (!xmlStrcmp(child->name, XMLSTR("Bindings")) ||
        !xmlStrcmp(child->name, XMLSTR("Aliases")))
      continue;
    return TRUE;
  }
  return FALSE;
}

LPXMLDOC ED_FindXMLCache(lpcString_t filename) {
  FOR_EACH_LIST(EDCACHE, cache, editor.project.cache) {
    if (!strcmp(cache->path, filename)) {
      return cache->doc;
    }
  }
  LPXMLDOC doc = xmlReadFile(FS_JoinPaths(editor.currentProject, filename), NULL, 0);
  if (doc) {
    LPEDCACHE cache = ZeroAlloc(sizeof(EDCACHE));
    strncpy(cache->path, filename, sizeof(cache->path));
    cache->doc = doc;
    ADD_TO_LIST(cache, editor.project.cache);
    return doc;
  }
  return NULL;
}

static LPOBJDEF _GetObjDef(LPXMLNODE node, EnumChildProc proc, LPVOID parm) {
  LPXMLSTR name = xmlGetProp(node, XMLSTR("name"));
  static struct _OBJDEF def;
  memset(&def, 0, sizeof(def));
  def.szName = (lpcString_t)name;
  def.dwStyle = 7;
  def.hIdentifier = MapPointerTo32BitInt(node);
  def.bHasChildren = XML_HasChildren(node) || !xmlStrcmp(node->name, XMLSTR("kanzi.SCAManager2D"));
  proc(&def, parm);
  xmlFree(name);
  return &def;
}

void XML_EnumChildObjects(HANDLE handle, EnumChildProc proc, LPVOID parm) {
  LPXMLNODE root = handle ? NULL : editor.project.scene;
  if (!handle || XML_FindNode(handle, &root)) {
    if (!xmlStrcmp(root->name, XMLSTR("kanzi.SCAManager2D"))) {
      LPXMLSTR SCA = xmlGetProp(root, XMLSTR("SCA"));
      if (!SCA)
        return;
      PATHSTR str={0};
      sprintf(str, "../../../icui/hmi_plugins/SCAManager/projects/ic/high/common/%s/Prefabs/%s.xml", SCA, SCA);
      LPXMLDOC doc = ED_FindXMLCache(FS_JoinPaths(editor.currentProject, str));
      if (doc) {
        _GetObjDef(xmlDocGetRootElement(doc), proc, parm);
      }
      xmlFree(SCA);
    } else xmlForEach(node, root) {
      if (!xmlStrcmp(node->name, XMLSTR("Bindings")) ||
          !xmlStrcmp(node->name, XMLSTR("Aliases")))
        continue;;
      _GetObjDef(node, proc, parm);
    }
  }
}

static void
_EnumChildObjects(LPVOID h, EnumChildProc p, LPVOID v) {
  UI_EnumChildObjects(h, p, v);
}
static void
_FilterObjects(LPVOID h, lpcString_t s, EnumChildProc p, LPVOID v) {
  UI_FilterObjects(h, s, p, v);
}
static BOOL
_GetObjectItem(LPVOID h, LPOBJDEF o) {
  return UI_GetObjectItem(h, o);
}
static BOOL
_GetObjectProperty(LPVOID h, lpcString_t s, LPPROPDEF d) {
  return UI_GetObjectProperty(h, s, d);
}
static BOOL
_SetObjectName(LPVOID h, lpcString_t s) {
  return UI_RenameObject(h, s);
}
static BOOL
_NewObject(LPVOID h, lpcString_t s, DWORD d) {
  return UI_NewObject(h, s, d) != NULL;
}

LPTREEVIEWDATA ED_GetHierarchyDataSource(void) {
  static TREEVIEWDATA data;
//  data.EnumChildObjects = XML_EnumChildObjects;
  data.EnumChildObjects = _EnumChildObjects;
  data.FilterObjects = _FilterObjects;
  data.GetObjectItem = _GetObjectItem;
  data.GetObjectProperty = _GetObjectProperty;
  data.SetObjectName = _SetObjectName;
  data.NewObject = _NewObject;
  return &data;
}

static xmlNodePtr xml;


LPEDCMD _AppendMenu(DWORD cmd_id, EDCMD *commands);

static LRESULT
ED_HierarchyOnCommand(HEDWND wnd, LPVOID udata, DWORD cmd)
{
  switch (cmd) {
    case ID_OBJECT:
      if (SceneView_GetSelection(wnd) && ED_GetSelectedItem(wnd)) {
        DWORD pos = Con_GetItemPosition(ED_GetClient(wnd), ED_GetSelectedItem(wnd));
        DWORD x = (LOWORD(pos) + 2) * CONSOLE_CHAR_WIDTH;
        DWORD y = (HIWORD(pos) + 1) * CONSOLE_CHAR_HEIGHT;
        ED_TrackPopupMenu(_AppendMenu(ID_OBJECT, editor.commands)->subcommands, x, y, wnd);
      }
      return TRUE;
    case ID_OBJECT_LABEL:
    case ID_OBJECT_IMAGE:
    case ID_OBJECT_STACK:
      TreeView_CreateItem(wnd, cmd);
      return TRUE;
    case ID_EDIT_COPY:
      if (SceneView_GetSelection(wnd)) {
        if (xml) {
          xmlFreeNode(xml);
        }
        xml = ED_ConvertNode(SceneView_GetSelection(wnd), NULL);
        return TRUE;
      } else {
        return FALSE;
      }
    case ID_EDIT_PASTE:
      if (xml) {
        HOBJ root = editor.screen;
        HOBJ obj = XML_ParseObjectNode(editor.L, xml, root);
        if (SceneView_GetSelection(wnd) && OBJ_GetParent(SceneView_GetSelection(wnd))) {
          OBJ_AddChild(OBJ_GetParent(SceneView_GetSelection(wnd)), obj, FALSE);
        } else {
          OBJ_AddChild(root, obj, FALSE);
        }
        ED_InvalidateWindow(wnd);
        return TRUE;
      } else {
        return FALSE;
      }
  }
//  EDWINPROC(SelectClass);
//  if (!strcmp(cmd, hierarchy_new_item) ) {
//    RECT rect = {
//      .x = 100,
//      .y = 100,
//      .width = CONSOLE_CHAR_WIDTH * 64,
//      .height = CONSOLE_CHAR_HEIGHT * 32
//    };
//    ED_SetFocusedPanel(ED_CreateWindow(ED_SelectClass, &rect, wnd, NULL));
//    return TRUE;
//  }
//  if (!strcmp(cmd, "menu:create")) {
//    RECT rect = {
//      .x = 100,
//      .y = 100,
//      .width = CONSOLE_CHAR_WIDTH * 32,
//      .height = CONSOLE_CHAR_HEIGHT * 32
//    };
//    ED_SetFocusedPanel(ED_CreateWindow(ED_SelectClass, &rect, wnd, NULL));
//    return TRUE;
//  }
  return ED_TreeView(wnd, EVT_HOTKEY, cmd, NULL);
}

static LRESULT Hierarchy_OnDrop(HEDWND wnd, WORD item, LPEDDRAGITEM drag) {
  LPVOID parent = TreeView_GetItem(wnd, item);
  HOBJ child = drag->object;
  ED_SetHoveredItem(wnd, 0);
  if (!parent || !child)
    return FALSE;
  for (HOBJ p = parent; p; p = OBJ_GetParent(p)) {
    if (p == child)
      return FALSE;
  }
  OBJ_AddChild(parent, child, FALSE);
  ED_InvalidateWindow(wnd);
  return TRUE;
}

static LRESULT Hierarchy_OnDrag(HEDWND wnd, WORD item, LPEDDRAGITEM drag) {
  struct _OBJDEF objdef;
  LPVOID obj = TreeView_GetItem(wnd, item);
  if (UI_GetObjectItem(obj, &objdef) && objdef.dwItemType == drag->dragtype) {
    ED_SetHoveredItem(wnd, item);
    return TRUE;
  } else {
    ED_SetHoveredItem(wnd, 0);
    return FALSE;
  }
}

LRESULT ED_HierarchyNavigator(HEDWND wnd, DWORD msg, wParam_t wparm, lParam_t lparm) {
  switch (msg) {
    case EVT_CREATE:
      ED_SetUserData(wnd, ED_AllocateTreeViewData(ED_GetHierarchyDataSource(), lparm));
      return 1;
    case EVT_GETTEXT:
      strncpy(lparm, "Hierarchy", wparm);
      return 1;
    case EVT_CDROP:
      return Hierarchy_OnDrop(wnd, LOWORD(wparm), lparm);
    case EVT_CDRAG:
      return Hierarchy_OnDrag(wnd, LOWORD(wparm), lparm);
    case EVT_COMMAND:
      switch (LOWORD(wparm)) {
        case TREE_ON_SELECT:
          ED_SendMessage(ED_SplitView_GetContent(ED_GetParent(wnd)), EVT_OBJECT_SELECTED, TRUE, lparm);
          ED_SendMessage(editor.inspector, EVT_OBJECT_SELECTED, TRUE, lparm);
          return TRUE;
      }
      break;
    case EVT_OBJECT_SELECTED:
      if (lparm && !wparm) {
        struct _OBJDEF def={0};
        ED_ClearConsole(wnd);
        ED_SendMessage(wnd, EVT_PRINT, 0, NULL);
        for (HOBJ obj = OBJ_GetParent(lparm); obj; obj = OBJ_GetParent(obj)) {
          UI_GetObjectItem(obj, &def);
          ED_SendMessage(wnd, EVT_COMMAND, TREE_EXPAND, &def.hIdentifier);
        }
        UI_GetObjectItem(lparm, &def);
        ED_SendMessage(wnd, EVT_COMMAND, TREE_SELECT, &def.hIdentifier);
      }
      break;
    case EVT_CONTEXTMENU:
      if (ED_TreeView(wnd, msg, wparm, lparm)) {
        MENU menu = {0};
        if (((LPTERMINALCHAR)lparm)->item) {
          ED_AppendMenu(menu, ID_EDIT_COPY);
          ED_AppendMenu(menu, ID_EDIT_PASTE);
          ED_AppendMenu(menu, ID_EDIT_DELETE);
          ED_AppendMenu(menu, ID_EDIT_RENAME);
          ED_AppendMenu(menu, ID_EDIT_DUPLICATE);
          ED_AppendMenu(menu, ID_EDIT_PASTE);
          ED_AppendMenu(menu, ID_OBJECT);
        } else {
          ED_AppendMenu(menu, ID_EDIT_PASTE);
          ED_AppendMenu(menu, ID_OBJECT);
        }
        ED_TrackPopupMenu(menu, LOWORD(wparm), HIWORD(wparm), wnd);
        return 1;
      }
      return 0;
    case EVT_HOTKEY:
      return ED_HierarchyOnCommand(wnd, ED_GetUserData(wnd), wparm);
    default:
      break;
  }
  return ED_TreeView(wnd, msg, wparm, lparm);
}

#endif
