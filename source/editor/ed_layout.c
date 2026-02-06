#include "ed_local.h"

EDWINPROC(RootWindow);
EDWINPROC(SceneView);
EDWINPROC(GameView);
EDWINPROC(HierarchyNavigator);
EDWINPROC(ProjectNavigator);
EDWINPROC(Inspector);
EDWINPROC(Bindings);
EDWINPROC(MenuBar);
EDWINPROC(StatusBar);
EDWINPROC(Spreadsheet);
EDWINPROC(TabView);
EDWINPROC(WelcomeView);
EDWINPROC(Console);
EDWINPROC(TextEditor);
EDWINPROC(CreateObject);
EDWINPROC(SplitView);
EDWINPROC(RootWindow);
EDWINPROC(Plugins);

HEDWND ED_InitLayout(void) {
  SIZE2 size;
  WI_GetSize(&size);
  RECT rect = { 0, 0, size.width, size.height };
  return ED_CreateWindow(ED_RootWindow, 0, &rect, NULL, &(SPLITVIEWDESC) {
    .left = { ED_SplitView, 0, &(SPLITVIEWDESC) {
      .left = { ED_TabView, EDWF_TITLEBAR, (EDCREATE[]) {
        {ED_Console},
        /*{ED_Spreadsheet},*/
        {ED_WelcomeView},
        {ED_TextEditor},
//        {ED_SceneView, 0, "/Users/igor/Developer/ui-framework/projects/Example/Application.xml"},//editor.screen},
        {ED_GameView},
        {NULL}
      }},
        .right = { ED_SplitView, 0, &(SPLITVIEWDESC) {
          .left = { ED_MenuBar },
          .right = { ED_SplitView, 0, &(SPLITVIEWDESC) {
            .left = {ED_TabView, EDWF_TITLEBAR, (EDCREATE[]) {
              {ED_Plugins},
              {ED_Bindings},
              {ED_Inspector},
              {NULL}
            }},
            .right = { ED_ProjectNavigator, EDWF_TITLEBAR },
            .split = 0.5,
          }},
            .split = 0.05,
        }},
        .split = 0.775,
        .horizontal = TRUE,
    }},
      .right = ED_StatusBar,
      .split = 1,
  });
}

