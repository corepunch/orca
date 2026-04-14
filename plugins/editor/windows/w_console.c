#include "../ed_local.h"

LRESULT ED_Console(HEDWND wnd, DWORD msg, wParam_t wparm, lParam_t lparm) {
  switch (msg) {
    case EVT_CREATE:
      ED_ClearConsole(wnd);
      ED_SetWindowFlags(wnd, EDWF_NOCLEAR);
      Con_SetTextAttribute(ED_GetClient(wnd), &(TERMINALCHAR) {
        .foreground = 7,
        .background = 0,
      });
      return 1;
    case EVT_GETTEXT:
      strncpy(lparm, "Console", wparm);
      return 1;
  }
  
  return 0;
}

#define MAXPRINTMSG 4096

void ED_Printf(lpcString_t format, ...) {
  CHAR msg[MAXPRINTMSG];
  
  va_list args;
  va_start(args, format);
  vsprintf(msg, format, args);
  va_end(args);
  
  EDWINPROC(Console);
  HEDWND console = ED_FindWindow(ED_Console);
  
  if (console) {
    ED_Echo(console, "%s\n", msg);
  }
  puts(msg);
}

