#include "../ed_local.h"

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

static void PrintPlugin(LPOBJDEF objdef, LPVOID wnd) {
  BOOL value=1;
  if (!strstr(objdef->szName, ".lua")) {
    return;
  }
  ED_Echo(wnd, STYLE_LABEL);
  ED_BooleanField(ED_GetClient(wnd),&value,objdef->szName,32);
  ED_Echo(wnd, "\n");
}

EDWINPROC(Plugins) {
  switch (msg) {
    case EVT_PRINT:
      lua_getglobal(editor.L, "PLUGDIR");
      FS_EnumDir(luaL_checkstring(editor.L, -1), PrintPlugin, wnd);
      lua_pop(editor.L, 1);
      return TRUE;
//    case EVT_COMMAND:
//      switch (LOWORD(wparm)) {
//        case CBN_SELCHANGE:
//          ED_SplitView_SetContent(wnd, ED_Inspector, obj);
//          return TRUE;
//        default:
//          return FALSE;
//      }
    case EVT_GETTEXT:
      strncpy(lparm, "Plugins", wparm);
      return TRUE;
  }
  return FALSE;
}
