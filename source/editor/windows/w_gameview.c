#include "../ed_local.h"

typedef struct _GAMEVIEW {
  PATHSTR filename;
  lpObject_t scene;
  lpObject_t selected;
} GAMEVIEW, *LPGAMEVIEW;

EDWINPROC(HierarchyNavigator);
EDWINPROC(CanvasView);
EDWINPROC(SplitView);
EDWINPROC(GameView);

HOBJ GameView_GetSelection(HEDWND wnd) {
  if (!wnd) {
    return NULL;
  }
  if (ED_GetWindowProc(wnd) == ED_GameView) {
    LPGAMEVIEW data = ED_GetUserData(wnd);
    return data->selected;
  }
  return GameView_GetSelection(ED_GetParent(wnd));
}
void TreeView_SetRootItem(HEDWND wnd, LPVOID item);
EDWINPROC(GameView) {
//  LPGAMEVIEW data = ED_GetUserData(wnd);
  switch (msg) {
    case EVT_CREATE:
//      xmlWith(xmlDoc, doc, xmlReadFile(lparm, NULL, 0), xmlFreeDoc) {
//        PATHSTR path = {0};
//        strncpy(path, lparm, sizeof(path));
//        if (strrchr(path, '.')) *strrchr(path, '.') = 0;
//        xmlNodeSetName((xmlNodePtr)doc, BAD_CAST path);
//        HOBJ root = OBJ_LoadDocument(editor.L, doc);
      ED_SplitView(wnd, msg, wparm, &(SPLITVIEWDESC) {
        .left = { ED_HierarchyNavigator, 0, editor.screen },
        .right = { ED_CanvasView, 0, editor.screen },
        .split = 0.2,
        .horizontal = TRUE,
      });
//        data = ED_GetUserData(wnd);
//        data->scene = root;
//      }
      return TRUE;
    case EVT_GETTEXT:
      strncpy(lparm, "Game", wparm);
      return 0;
    case EVT_EXTRASPACE:
      return sizeof(GAMEVIEW);
    case EVT_COMMAND:
      if (LOWORD(wparm) == GAME_SELECT_SCREEN) {
        TreeView_SetRootItem(ED_SplitView_GetSideBar(wnd), lparm);
        return TRUE;
      }
      return FALSE;
    default:
      return ED_SplitView(wnd, msg, wparm, lparm);
  }
}
