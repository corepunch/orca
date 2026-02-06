#include "../ed_local.h"

static BOOL
Project_GetObjectProperty(LPVOID lpObject,
                          lpcString_t szName,
                          LPPROPDEF lpPropDef)
{
  return FALSE;
}

static void
_EnumDir(LPVOID obj, EnumChildProc proc, LPVOID parm) {
  if (obj) {
    PATHSTR root_dir={0};
    FS_GetFileName((int)(intptr_t)obj, root_dir, TRUE);
    FS_EnumDir(root_dir, proc, parm);
  } else {
    FS_EnumDir(NULL, proc, parm);
  }
}

static void
_FilterFiles(LPVOID _, lpcString_t szFilter, EnumChildProc lpProc, LPVOID lpParm) {
  FS_FilterFiles(szFilter, lpProc, lpParm);
}

static BOOL
_FindFileByHandle(LPVOID lpObject, LPOBJDEF lpObjDef) {
  return FS_FindFileByHandle((HANDLE)(intptr_t)lpObject, lpObjDef);
}

static BOOL
_RenameFile(LPVOID lpObject, lpcString_t szFilename) {
  return FS_RenameFile((HANDLE)(intptr_t)lpObject, szFilename);
}

static DWORD
_NewFile(LPVOID lpObject, lpcString_t szName, DWORD dwType) {
  if (dwType == PROJITEM_PROJECT) {
    path_t pPath={0};
    if (eimp.GetFolderName(&(WI_OpenFileName) {
      .lpstrFile = pPath,
      .nMaxFile = sizeof(pPath),
    })) {
      lpcString_t szDir = FS_JoinPaths(pPath, szName);
      FS_MakeDirectory(szDir);
      snprintf(pPath, sizeof(pPath), "%s/%s.xml", szDir, szName);
      xmlWith(xmlDoc, doc, FS_InitProject(szName), xmlFreeDoc) {
        xmlSaveFormatFile(pPath, doc, TRUE);
      }
      return FS_NewFile((HANDLE)(intptr_t)lpObject, pPath, PROJITEM_PROJECT);
    } else {
      return FALSE;
    }
  } else {
    return FS_NewFile((HANDLE)(intptr_t)lpObject, szName, dwType);
  }
}

LPTREEVIEWDATA ED_GetProjectDataSource(void) {
  extern struct _IFILESYSTEM lpFileSystem;
  static TREEVIEWDATA data;
  data.EnumChildObjects = _EnumDir;
  data.FilterObjects = _FilterFiles;
  data.GetObjectItem = _FindFileByHandle;
  data.SetObjectName = _RenameFile;
  data.NewObject = _NewFile;
  data.GetObjectProperty = Project_GetObjectProperty;
  return &data;
}

EDWINPROC(SelectFileType);

struct _TREEVIEW;

static LRESULT
ED_ProjectOnCommand(HEDWND wnd, LPVOID udata, wParam_t cmd)
{
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
  switch (cmd) {
    case ID_ASSET_SCENE:
      TreeView_CreateItem(wnd, PROJITEM_SCENE);
      return TRUE;
    case ID_ASSET_MATERIAL:
      TreeView_CreateItem(wnd, PROJITEM_MATERIAL);
      return TRUE;
    case ID_ASSET_PROJECT_REFERENCE:
      TreeView_CreateItem(wnd, PROJITEM_PROJECT);
      return TRUE;
  }
//  if (!strcmp(cmd, "menu:create")) {
//    RECT rect = {
//      .x = 100,
//      .y = 100,
//      .width = CONSOLE_CHAR_WIDTH * 32,
//      .height = CONSOLE_CHAR_HEIGHT * 32
//    };
//    ED_SetFocusedPanel(ED_CreateWindow(ED_Menu, 0, &rect, wnd, &(EDCMD []) {
//      { 0, "create:scene", "Scene" },
//      { 0, "create:material", "Material" },
//      { 0, "create:shader", "Shader" },
//      { 0 }
//    }));
//    return TRUE;
//  }
  return ED_TreeView(wnd, EVT_HOTKEY, cmd, 0);
}

DWORD TreeView_GetIdentifierAtIndex(struct _TREEVIEW *, WORD);

LRESULT ED_ProjectNavigator(HEDWND wnd, DWORD msg, wParam_t wparm, lParam_t lparm) {
  LPTREEVIEWDATA datasource;
  struct _OBJDEF objdef;
  switch (msg) {
    case EVT_CREATE:
      datasource = ED_GetProjectDataSource();
      ED_SetUserData(wnd, ED_AllocateTreeViewData(datasource, NULL));
      return TRUE;
    case EVT_CDCLICK: {
      LPTERMINALCHAR ch = lparm;
      DWORD id = TreeView_GetIdentifierAtIndex(ED_GetUserData(wnd), ch->item);
      if (FS_FindFileByHandle(id, &objdef)) {
        if (strstr(objdef.szFullPath, "Project References/")) {
          path_t path={0};
          FS_GetProjectReference(objdef.szName, path, sizeof(path));
          lua_pushstring(editor.L, path);
          lua_setglobal(editor.L, "RELOAD");
        }
      }
      return TRUE;
    }
    case EVT_CONTEXTMENU:
      if (ED_TreeView(wnd, msg, wparm, lparm)) {
        LPTERMINALCHAR ch = lparm;
        DWORD id = TreeView_GetIdentifierAtIndex(ED_GetUserData(wnd), ch->item);
        MENU menu={0};
        if (FS_FindFileByHandle(id, &objdef)) {
          Con_Error("Context for %s", objdef.szFullPath);
          switch (FS_FindLibraryType(objdef.szFullPath)) {
            case kMaterialLibrary:
              ED_AppendMenu(menu, ID_ASSET_MATERIAL);
              break;
            case kProjectReferenceLibrary:
              ED_AppendMenu(menu, ID_ASSET_PROJECT_REFERENCE);
              ED_AppendMenu(menu, ID_ASSET_PROJECT_REFERENCE_EXISTING);
              break;
            default:
              return FALSE;
          }
        } else {
          ED_AppendMenu(menu, ID_EDIT_COPY);
          ED_AppendMenu(menu, ID_EDIT_PASTE);
          ED_AppendMenu(menu, ID_EDIT_DELETE);
          ED_AppendMenu(menu, ID_EDIT_RENAME);
          ED_AppendMenu(menu, ID_EDIT_DUPLICATE);
          ED_AppendMenu(menu, ID_ASSET);
        }
        ED_TrackPopupMenu(menu, LOWORD(wparm), HIWORD(wparm), wnd);
        return TRUE;
      } else {
        return FALSE;
      }
    case EVT_HOTKEY:
      return ED_ProjectOnCommand(wnd, ED_GetUserData(wnd), wparm);
    case EVT_COMMAND:
      switch (LOWORD(wparm)) {
        case TREE_ON_SELECT:
//          ED_SendMessage(HEDWND_BOARDCAST, EVT_FILE_SELECTED, 0, lparm);
          if (FS_FindFileByHandle((uint32_t)(intptr_t)lparm, &objdef)) {
            Con_Error("Opening %s", objdef.szFullPath);
//            ED_OpenDocument(objdef.szFullPath);
          }
          return TRUE;
      }
      break;
    case EVT_GETTEXT:
      strncpy(lparm, "Library", wparm);
      return 1;
  }
  return ED_TreeView(wnd, msg, wparm, lparm);
}

