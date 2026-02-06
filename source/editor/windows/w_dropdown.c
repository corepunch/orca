#include "../ed_local.h"

static void
ED_PrintDropDown(HEDWND wnd, EDDROPDOWN* data)
{
  FOR_LOOP(i, data->dwNumItems) {
    ED_Echo(wnd, FMT_COMMAND, i+1);
    ED_Echo(wnd, STYLE_LABEL "%-*s", ED_GetClient(wnd)->width, data->pStrings[i]);
  }
}

static BOOL
DropDown_OnClick(HEDWND wnd, EDDROPDOWN *dd, DWORD _, LPTERMINALCHAR data)
{
  WORD item = data->item;
  ED_SetSelectedItem(wnd, item-1);
  ED_InvalidateWindow(wnd);
  ED_SendMessage(dd->pParent, EVT_COMMAND, MAKEDWORD(CBN_SELCHANGE, ED_GetSelectedItem(wnd)), wnd);
  ED_DestroyWindow(wnd);
  return TRUE;
}

LRESULT
ED_DropDownCommand(HEDWND wnd, EDDROPDOWN *dd, wParam_t wparam, lpcString_t cmd)
{
  switch (wparam) {
    case ID_NAV_PREVROW:
      ED_SetSelectedItem(wnd, ED_GetSelectedItem(wnd)>1?ED_GetSelectedItem(wnd)-1:dd->dwNumItems);
      ED_SendMessage(dd->pParent, EVT_COMMAND, MAKEDWORD(CBN_SELCHANGE, ED_GetSelectedItem(wnd)-1), wnd);
      return TRUE;
    case ID_NAV_NEXTROW:
      ED_SetSelectedItem(wnd, ED_GetSelectedItem(wnd)<dd->dwNumItems?ED_GetSelectedItem(wnd)+1:1);
      ED_SendMessage(dd->pParent, EVT_COMMAND, MAKEDWORD(CBN_SELCHANGE, ED_GetSelectedItem(wnd)-1), wnd);
      return TRUE;
    case ID_NAV_CANCEL:
      ED_SendMessage(dd->pParent, EVT_COMMAND, MAKEDWORD(CBN_SELCHANGE, dd->dwSelected), wnd);
      ED_SetFocusedPanel(dd->pParent);
      return TRUE;
    case ID_NAV_ACCEPT:
      ED_SetFocusedPanel(dd->pParent);
      return TRUE;
  }
  return FALSE;
}

LRESULT
ED_DropDown(HEDWND wnd, DWORD msg, wParam_t wparm, lParam_t lparm)
{
  switch (msg) {
    case EVT_CREATE:
      memcpy(ED_AllocUserData(wnd, sizeof(EDDROPDOWN)), lparm, sizeof(EDDROPDOWN));
      ED_SetSelectedItem(wnd, ((EDDROPDOWN*)lparm)->dwSelected + 1);
      return 1;
    case EVT_DESTROY:
      free(ED_GetUserData(wnd));
      return 1;
    case EVT_CCLICK:
      DropDown_OnClick(wnd, ED_GetUserData(wnd), wparm, lparm);
      return 1;
    case EVT_CGRAB:
      ED_SetSelectedItem(wnd, wparm);
      return 1;
    case EVT_PRINT:
      ED_PrintDropDown(wnd, ED_GetUserData(wnd));
      return 1;
    case EVT_HOTKEY:
      return ED_DropDownCommand(wnd, ED_GetUserData(wnd), wparm, lparm);
    case EVT_GETTEXT:
      if (*((EDDROPDOWN*)ED_GetUserData(wnd))->pTitle) {
        strncpy(lparm, ((EDDROPDOWN*)ED_GetUserData(wnd))->pTitle, wparm);
      } else {
        strncpy(lparm, "DropDown", wparm);
      }
      return 1;
    case kEventKillFocus:
      ED_DestroyWindow(wnd);
      return 0;
  }
  
  return 0;
}
