#include "../ed_local.h"

typedef struct _SCENEVIEW {
  PATHSTR filename;
  lpObject_t scene;
  lpObject_t selected;
} SCENEVIEW, *LPSCENEVIEW;

EDWINPROC(HierarchyNavigator);
EDWINPROC(CanvasView);
EDWINPROC(SplitView);
EDWINPROC(SceneView);

lpObject_t OBJ_LoadDocument(lua_State*, xmlDocPtr);

lpcString_t ED_GetWindowDocument(HEDWND wnd) {
  struct _SCENEVIEW *data = ED_GetUserData(wnd);
  return data->filename;
}

HOBJ SceneView_GetSelection(HEDWND wnd) {
  if (!wnd) {
    return NULL;
  }
  if (ED_GetWindowProc(wnd) == ED_SceneView) {
    LPSCENEVIEW data = ED_GetUserData(wnd);
    return data->selected;
  }
  return SceneView_GetSelection(ED_GetParent(wnd));
}

EDWINPROC(SceneView) {
  LPSCENEVIEW data = ED_GetUserData(wnd);
  switch (msg) {
    case EVT_CREATE:
      xmlWith(xmlDoc, doc, xmlReadFile(lparm, NULL, 0), xmlFreeDoc) {
        PATHSTR path = {0};
        strncpy(path, lparm, sizeof(path));
        if (strrchr(path, '.')) *strrchr(path, '.') = 0;
        xmlNodeSetName((xmlNodePtr)doc, BAD_CAST path);
        HOBJ root = OBJ_LoadDocument(editor.L, doc);
        ED_SplitView(wnd, msg, wparm, &(SPLITVIEWDESC) {
          .left = { ED_HierarchyNavigator, 0, root },
          .right = { ED_CanvasView, 0, root },
          .split = 0.2,
          .horizontal = TRUE,
        });
        data = ED_GetUserData(wnd);
        data->scene = root;
      }
      return TRUE;
    case EVT_GETTEXT:
      if (editor.screen || data->scene) {
        lpcString_t filename = OBJ_GetSourceFile(data->scene?data->scene:editor.screen);
        if (filename) {
          strncpy(lparm, FS_GetBaseName(filename), wparm);
        } else {
          strncpy(lparm, "New Scene", wparm);
        }
      } else {
        strncpy(lparm, "Scene", wparm);
      }
      return 0;
    case EVT_EXTRASPACE:
      return sizeof(SCENEVIEW);
    default:
      return ED_SplitView(wnd, msg, wparm, lparm);
  }
}
