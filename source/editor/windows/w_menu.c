#include "../ed_local.h"

#define MAX_INSPECTOR_ITEMS 256

LRESULT
ED_MenuCommand(HEDWND wnd, EDCMD *items, wParam_t wparam, lpcString_t lparam)
{
  DWORD numitems = 0;
  for (EDCMD *item = items; item->szName; item++, numitems++);
  LPEDCMD cmd = numitems > (ED_GetSelectedItem(wnd)-1) ?&items[ED_GetSelectedItem(wnd)-1] : NULL;
  switch (wparam) {
    case ID_NAV_PREVROW:
      ED_SetSelectedItem(wnd, ED_GetSelectedItem(wnd)>1?ED_GetSelectedItem(wnd)-1:numitems);
      return TRUE;
    case ID_NAV_NEXTROW:
      ED_SetSelectedItem(wnd, ED_GetSelectedItem(wnd)<numitems?ED_GetSelectedItem(wnd)+1:1);
      return TRUE;
    case ID_NAV_CANCEL:
      ED_SetFocusedPanel(ED_GetParent(wnd));
      return TRUE;
    case ID_EDIT_RENAME: // both are Enter key
    case ID_NAV_ACCEPT:
      if (!cmd) {
        return FALSE;
      } else if (cmd->subcommands) {
        RECT w = ED_GetWindowRect(wnd);
        w.x += w.width;
        w.y += (ED_GetSelectedItem(wnd)-1) * CONSOLE_CHAR_HEIGHT;
        ED_SetFocusedPanel(ED_CreateWindow(ED_Menu, 0, &w, wnd, (void*)cmd->subcommands));
      } else {
        for (HEDWND w = wnd; w; w = ED_GetParent(w)) {
          if (!ED_FunctionEquals(w, ED_Menu)) {
            DWORD wparam = cmd->szCommand;
            ED_SetFocusedPanel(w);
            ED_SendMessage(w, EVT_HOTKEY, wparam, NULL);
            return TRUE;
          }
        }
      }
      return TRUE;
    default:
      return TRUE;
  }
}

LRESULT
ED_Menu(HEDWND wnd, DWORD msg, wParam_t wparm, lParam_t lparm)
{
  EDCMD *items = ED_GetUserData(wnd);
  DWORD numitems = 0, maxwidth = 0;
  BOOL hassubs = FALSE;
  RECT rect = ED_GetWindowRect(wnd);
  switch (msg) {
    case EVT_CREATE:
      for (EDCMD *item = lparm; item->szName; item++, numitems++) {
        hassubs |= !!item->subcommands;
        maxwidth = MAX((int)strlen(item->szName), maxwidth);
      }
      memcpy(ED_AllocUserData(wnd, (numitems+1) * sizeof(EDCMD)), lparm, (numitems+1) * sizeof(EDCMD));
      ED_SetWindowFlags(wnd, ED_GetWindowFlags(wnd)|EDWF_CASTSHADOW);
      ED_SetWindowRect(wnd, &(RECT) {
        rect.x,
        rect.y,
        (maxwidth + (hassubs?3:2)) * CONSOLE_CHAR_WIDTH,
        MIN(MAX_MENU_HEIGHT, numitems) * CONSOLE_CHAR_HEIGHT,
      });
      return 1;
    case EVT_PRINT:
      ED_Echo(wnd, STYLE_MENU);
      for (EDCMD *item = items; item->szName; item++) {
        ED_Echo(wnd, FMT_COMMAND, ++numitems);
        char str[64]={0};
        if (item->subcommands) {
          DWORD w = (int)(rect.width) / CONSOLE_CHAR_WIDTH - 3;
          snprintf(str, sizeof(str), "%-*s\x1e\x1f", w, item->szName);
        } else {
          strncpy(str, item->szName, sizeof(str));
        }
        ED_Echo(wnd, " %-*s ", ED_GetClient(wnd)->width-2, str);
      }
      return 1;
    case EVT_CGRAB:
      ED_SetSelectedItem(wnd, wparm);
      return 1;
    case EVT_CCLICK:
      if (ED_GetSelectedItem(wnd)) {
        ED_SendMessage(wnd, EVT_HOTKEY, ID_NAV_ACCEPT, NULL);
      } else for (HEDWND w = wnd; w; w = ED_GetParent(w)) {
        if (!ED_FunctionEquals(w, ED_Menu)) {
          ED_SetFocusedPanel(w);
          return TRUE;
        }
      }
      return 1;
      //    case EVT_CDROP:
      //      return ED_DropInspector(wnd, inspector, wparm, lparm);
    case EVT_HOTKEY:
      return ED_MenuCommand(wnd, ED_GetUserData(wnd), wparm, lparm);
    case EVT_GETTEXT:
      strncpy(lparm, "Menu", wparm);
      return 1;
    case kEventKillFocus:
      for (HEDWND w = lparm; w; w = ED_GetParent(w)) {
        if (w == wnd) {
          return 0;
        }
      }
      for (HEDWND w = wnd, p = ED_GetParent(w);
           w && w != lparm && ED_FunctionEquals(w, ED_Menu);
           w = p, p = ED_GetParent(p))
      {
        ED_DestroyWindow(w);
      }
      return 0;
    case EVT_DESTROY:
      free(ED_GetUserData(wnd));
      return 0;
  }
  return 0;
}
