#include "../ed_local.h"

#define PADDING "         "

#define NEW_PROJECT 1
#define OPEN_PROJECT 2

#define NUM_CMDS (sizeof(cmds)/sizeof(*cmds))

typedef struct {
  lpcString_t szText;
  LRESULT (*lpProc)(HEDWND);
} cmd_t;

static LRESULT NewProject(HEDWND hWnd) {
  return 0;
}

static LRESULT OpenProject(HEDWND hWnd) {
  path_t path={0};
  eimp.GetOpenFileName(&(WI_OpenFileName){
    .lpstrFile = path,
    .nMaxFile = sizeof(path),
  });
  return 0;
}

static LRESULT CloneProject(HEDWND hWnd) {
  return 0;
}

static LRESULT ConnectProject(HEDWND hWnd) {
  return 0;
}

cmd_t cmds[] = {
  { "New Project...", NewProject },
  { "Open Project...", OpenProject },
  { "Clone Git Repository...", CloneProject },
  { "Connect to...", ConnectProject },
};

static DWORD loop_index(DWORD index) {
  if (index > NUM_CMDS) return 1;
  if (index < 1) return NUM_CMDS;
  return index;
}

LRESULT ED_WelcomeView(HEDWND wnd, DWORD msg, wParam_t wparm, lParam_t lparm) {
  LPTERMINALCHAR ch = lparm;
  switch (msg) {
    case EVT_CREATE:
      ED_SetSelectedItem(wnd, 1);
      return 1;
    case EVT_PRINT:
      ED_Echo(wnd, STYLE_LABEL "\n\n\n\n");
      ED_Echo(wnd, PADDING "ORCA v1.0\n");
      ED_Echo(wnd, STYLE_HIDDEN);
      ED_Echo(wnd, PADDING "Optimized Rendering and Composition API\n\n");
      
      ED_Echo(wnd, STYLE_TREEVIEW_STYLE(10));
      
      FOR_LOOP(i, NUM_CMDS) {
        ED_Echo(wnd, PADDING FMT_COMMAND "%s\n", i+1, cmds[i].szText);
        ED_Echo(wnd, FMT_COMMAND, 0);
      }
      
      ED_Echo(wnd, STYLE_LABEL "\n");
      ED_Echo(wnd, PADDING "Recent\n");
      return 1;
    case kEventKeyDown:
      if ((wparm&0xffff) == WI_KEY_ENTER && ED_GetSelectedItem(wnd) > 0) {
        cmds[ED_GetSelectedItem(wnd)-1].lpProc(wnd);
        return 1;
      } else {
        return 0;
      }
    case EVT_CCLICK:
      ED_SetFocusedPanel(wnd);
      if (ch->item-1 < NUM_CMDS) {
        ED_SetSelectedItem(wnd, ch->item);
        cmds[ch->item-1].lpProc(wnd);
      }
      return 1;
    case EVT_HOTKEY:
      switch (wparm) {
        case ID_NAV_NEXTROW:
          ED_SetSelectedItem(wnd, loop_index(ED_GetSelectedItem(wnd) + 1));
          return 1;
        case ID_NAV_PREVROW:
          ED_SetSelectedItem(wnd, loop_index(ED_GetSelectedItem(wnd) - 1));
          return 1;
        default:
          return 0;
      }
    case EVT_GETTEXT:
      strncpy(lparm, "Welcome", wparm);
      return 1;
    default:
      return 0;
  }
}
