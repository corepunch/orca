#include "../ed_local.h"

typedef struct _STATUSBARSTRUCT {
  //    HEDWND previous;
  //    EDTEXTEDIT prevtextedit;
  //    lpcString_t *items;
} STATUSBARSTRUCT, *LPSTATUSBARSTRUCT;

static void WriteWindowCommands(HEDWND wnd, HEDWND subject) {
  if (ED_GetParent(subject)) {
    WriteWindowCommands(wnd, ED_GetParent(subject));
  }
  Con_FillLineWithColor(ED_GetClient(wnd), editor.state == ED_EDIT ? 7 : 4);
  for (LPEDCMD cmd = editor.commands; cmd->szCommand; cmd++) {
//    if (cmd->hWnd == subject && cmd->szName) {
//      if (editor.state == ED_EDIT) {
//        ED_Echo(wnd, "\033[4;7m ");
//      } else {
//        ED_Echo(wnd, "\033[12;4m ");
//      }
//      if (cmd->dwKey & WI_MOD_CMD) ED_Echo(wnd, "Cmd+");
//      if (cmd->dwKey & WI_MOD_CTRL) ED_Echo(wnd, "Ctrl+");
//      if (cmd->dwKey & WI_MOD_ALT) ED_Echo(wnd, "Alt+");
//      if (cmd->dwKey & WI_MOD_SHIFT) ED_Echo(wnd, "Shift+");
//      DWORD key = cmd->dwKey & 0xff;
//      for (lpcString_t s = eimp.WI_KeynumToString(key); *s; s++) {
//        
//        ED_Echo(wnd, "%c", toupper(*s));
//      }
//      if (editor.state == ED_EDIT) {
//        ED_Echo(wnd, "\033[0;7m %s  ", cmd->szName);
//      } else if (!strcmp(cmd->szCommand, "editor:run")) {
//        ED_Echo(wnd, "\033[7;4m Stop  ");
//      } else {
//        ED_Echo(wnd, "\033[7;4m %s  ", cmd->szName);
//      }
//    }
  }
}

void ED_DrawStatusBar(HEDWND wnd, LPSTATUSBARSTRUCT statusbar) {
  if (editor.focus) {
    WriteWindowCommands(wnd, editor.focus);
  } 
}

LRESULT
ED_ClickStatusBar(HEDWND wnd, LPSTATUSBARSTRUCT statusbar, LPTERMINALCHAR data)
{
  if (data->item == 1024) {
    ED_RunProject();
    return 1;
  }
  return 0;
}

EDWINPROC(StatusBar) {
  LPSTATUSBARSTRUCT statusbar = ED_GetUserData(wnd);
  switch (msg) {
    case EVT_PRINT:
      ED_DrawStatusBar(wnd, statusbar);
      return 1;
    case EVT_CCLICK:
      ED_SetFocusedPanel(wnd);
      return ED_ClickStatusBar(wnd, statusbar, lparm);
  }
  return 0;
}
