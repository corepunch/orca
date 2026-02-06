#include "ed_local.h"

EDWINPROC(HierarchyNavigator);
EDWINPROC(Inspector);
EDWINPROC(SplitView);

LRESULT ED_EditorCommand(DWORD cmd) {
  extern HOBJ current_scene;
  switch (cmd) {
    case ID_FILE_QUIT:
//      editor.bHasFinished = TRUE;
      exit(0);
      return TRUE;
    case ID_FILE_RUN:
      ED_RunProject();
      return TRUE;
    case ID_EDIT_UNDO:
      ED_Undo();
      return TRUE;
    case ID_EDIT_REDO:
      ED_Redo();
      return TRUE;
    case ID_FILE_SAVE:
      if (current_scene) {
        OBJ_SaveDocument(current_scene);
        return TRUE;
      } else if (editor.screen) {
        OBJ_SaveDocument(editor.screen);
      } else {
        return FALSE;
      }
    case ID_NAV_SEL_HIERARCHY:
      if (ED_FindWindow(ED_HierarchyNavigator)) {
        ED_SetFocusedPanel(ED_FindWindow(ED_HierarchyNavigator));
      }
      return TRUE;
    case ID_NAV_SEL_INSPECTOR:
      if (ED_FindWindow(ED_Inspector)) {
        ED_SetFocusedPanel(ED_FindWindow(ED_Inspector));
      }
    return TRUE;
    default:
      return FALSE;
  }
}

EDWINPROC(RootWindow) {
  switch (msg) {
    case EVT_HOTKEY:
      return ED_EditorCommand(wparm);
    default:
      return ED_SplitView(wnd, msg, wparm, lparm);
  }
}
