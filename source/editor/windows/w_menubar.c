#include "../ed_local.h"

extern EDCMD ed_cmd[];

typedef struct _MENUBARSTRUCT {
  HEDWND previous;
  EDTEXTEDIT prevtextedit;
  lpcString_t *items;
} MENUBARSTRUCT, *LPMENUBARSTRUCT;

void ED_DrawMenuBar(HEDWND wnd, LPMENUBARSTRUCT menubar) {
//  if (menubar->items) {
//    for (lpcString_t *item = menubar->items; *item; item++) {
//      ED_Echo(wnd,
//           FMT_COMMAND STYLE_MENUBAR " %s ",
//           (DWORD)(item - menubar->items + 1),
//           *item);
//    }
//  } else {
//    ED_Echo(wnd, FMT_COMMAND, 1024);
//    ED_Echo(wnd, STYLE_LABEL "%s\n", editor.tooltip);
//  }
  
  ED_Echo(wnd, STYLE_MENUBAR);
  ED_Echo(wnd, " ");
  for (EDCMD *s = ed_cmd; s->szName; s++) {
    ED_Echo(wnd, FMT_COMMAND, s - ed_cmd + 1);
    ED_Echo(wnd, " %s ", s->szName);
  }
  while (ED_GetClient(wnd)->cursor%ED_GetClient(wnd)->width) {
    ED_Echo(wnd, " ");
  }
}

LRESULT ED_MenuBarCommand(HEDWND wnd, LPMENUBARSTRUCT menubar, DWORD wparm) {
  if (!menubar->items) {
    return FALSE;
  }
  switch (wparm) {
    case ID_NAV_NEXTCOL:
      if (menubar->items[ED_GetSelectedItem(wnd)] != 0) {
        ED_SetSelectedItem(wnd, ED_GetSelectedItem(wnd) + 1);
      } else {
        ED_SetSelectedItem(wnd, 1);
      }
      return TRUE;
    case ID_NAV_PREVCOL:
      if (ED_GetSelectedItem(wnd) > 1) {
        ED_SetSelectedItem(wnd, ED_GetSelectedItem(wnd) - 1);
      } else {
        int count = 0;
        while (menubar->items[count]) count++;
        ED_SetSelectedItem(wnd, count);
      }
      return TRUE;
    case ID_NAV_CANCEL:
    case ID_NAV_MENU:
      ED_SetFocusedPanel(menubar->previous);
      menubar->items = NULL;
      memcpy(&editor.textEdit, &menubar->prevtextedit, sizeof(EDTEXTEDIT));
      return TRUE;
    default:
      break;
  }
  return FALSE;
}

LRESULT
ED_ClickMenuBar(HEDWND wnd, LPMENUBARSTRUCT menubar, LPTERMINALCHAR data)
{
  ED_SetSelectedItem(wnd, data->item);
  DWORD pos = Con_GetItemPosition(ED_GetClient(wnd), data->item);  
  RECT rect = {
    LOWORD(pos) * CONSOLE_CHAR_WIDTH + ED_GetWindowRect(wnd).x,
    (HIWORD(pos) + 1) * CONSOLE_CHAR_HEIGHT + ED_GetWindowRect(wnd).y,
  };
  void *submenu = (void *)ed_cmd[data->item-1].subcommands;
  if (submenu) {
    ED_SetFocusedPanel(ED_CreateWindow(ED_Menu, 0, &rect, wnd, submenu));
  }
  return 0;
}

EDWINPROC(MenuBar) {
  LPMENUBARSTRUCT menubar = ED_GetUserData(wnd);
  switch (msg) {
    case EVT_CREATE:
      ED_AllocUserData(wnd, sizeof(struct _MENUBARSTRUCT));
      return 1;
    case EVT_PRINT:
      ED_DrawMenuBar(wnd, menubar);
      return 1;
    case EVT_CCLICK:
      ED_SetFocusedPanel(wnd);
      return ED_ClickMenuBar(wnd, menubar, lparm);
    case EVT_HOTKEY:
      return ED_MenuBarCommand(wnd, menubar, wparm);
  }
  return 0;
}

void ED_SetMenuItems(lpcString_t *items) {
  EDWINPROC(MenuBar);
  HEDWND wnd = ED_FindWindow(ED_MenuBar);
  LPMENUBARSTRUCT menubar = ED_GetUserData(wnd);
  ED_SetSelectedItem(wnd, 1);
  menubar->items = items;
  menubar->previous = editor.focus;
  memcpy(&menubar->prevtextedit, &editor.textEdit, sizeof(EDTEXTEDIT));
  ED_InvalidateWindow(wnd);
  ED_SetFocusedPanel(wnd);
}
