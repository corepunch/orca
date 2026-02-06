#include "../ed_local.h"

EDWINPROC(SplitView);
EDWINPROC(Menu);
EDWINPROC(WelcomeView);
EDWINPROC(Inspector);

static LRESULT
ED_StaticList(HEDWND wnd, DWORD msg, wParam_t wparm, lParam_t lparm)
{
  lpcString_t *items = ED_GetUserData(wnd);
  switch (msg) {
    case EVT_CREATE:
      ED_SetUserData(wnd, lparm);
      return 1;
    case EVT_PRINT:
      ED_Echo(wnd, STYLE_MENU);
      for (lpcString_t *item = items, *start = item; *item; item++) {
        ED_Echo(wnd, FMT_COMMAND, (item - start) + 1);
        ED_Echo(wnd, " %-*s ", ED_GetClient(wnd)->width-2, *item);
      }
      return 1;
    case EVT_CGRAB:
      ED_SetSelectedItem(wnd, wparm);
      ED_SetFocusedPanel(wnd);
      return 1;
    case EVT_CCLICK:
      if (ED_GetSelectedItem(wnd)) {
        DWORD parm = MAKEDWORD(CBN_SELCHANGE, ED_GetSelectedItem(wnd) - 1);
        ED_SendMessage(ED_GetParent(wnd), EVT_COMMAND, parm, wnd);
      }
      return 1;
      //    case EVT_CDROP:
      //      return ED_DropInspector(wnd, inspector, wparm, lparm);
//    case EVT_HOTKEY:
//      return ED_MenuCommand(wnd, ED_GetUserData(wnd), wparm, lparm);
//    case EVT_GETTEXT:
//      strncpy(lparm, "Menu", wparm);
//      return 1;
  }
  return 0;
}

lpcString_t cats[] = {
  "Display Mode",
  "Screen Resolution",
  "Text Color Scheme",
  "Keyboard Layout",
  "Mouse Sensitivity",
  "Sound Volume",
  "Startup Program",
  "Network Configuration",
  "Com Port Settings",
  "Baud Rate",
  "Disk Caching",
  "Memory Manager",
  "Boot Order",
  "Auto Power-Off",
  "Date & Time",
  "Region & Language",
  "Default Editor",
  "Log File Location",
  "Security Level",
  "Diagnostics Mode",
  NULL
};

extern lpObject_t OBJ_MakeNativeObject(lpcClassDesc_t cls);
extern struct ClassDesc _Project;

EDWINPROC(ProjectSettings) {
  static lpObject_t obj = NULL;
  if (!obj) obj = OBJ_MakeNativeObject(&_Project);
  switch (msg) {
    case EVT_CREATE:
      ED_SplitView(wnd, msg, wparm, &(SPLITVIEWDESC) {
        .left = { ED_StaticList, 0, cats },
        .right = { ED_Inspector, 0, NULL },
        .split = 0.33,
        .horizontal = TRUE,
      });
      return TRUE;
    case EVT_COMMAND:
      switch (LOWORD(wparm)) {
        case CBN_SELCHANGE:
          ED_SplitView_SetContent(wnd, ED_Inspector, obj);
          return TRUE;
        default:
          return FALSE;
      }
    case EVT_GETTEXT:
      strncpy(lparm, "Project Settings", wparm);
      return TRUE;
  }
  return ED_SplitView(wnd, msg, wparm, lparm);
}
