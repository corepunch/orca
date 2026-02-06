#include "../ed_local.h"

#define MAX_PROP_TYPES 256

static LPSTR _GetPropertyTypeName(HEDWND wnd, WORD index) {
  return ED_GetUserData(wnd) + MAX_PROPERTY_STRING * index;
}

static void
ED_PrintPropertyNewType(lpcString_t name, LPVOID parm)
{
  HEDWND wnd = parm;
  if (Con_CheckOverflow(ED_GetClient(wnd)))
    return;
  CHAR _tmp1[MAX_TEXT_EDIT]={0};
  CHAR _tmp2[MAX_PROPERTY_STRING]={0};
  for (LPSTR a=_tmp1,b=editor.textEditBuffer; *b; *(a++)=tolower(*(b++)));
  for (LPSTR a=_tmp2,b=(LPSTR)name; *b; *(a++)=tolower(*(b++)));
  if (!strstr(_tmp2, _tmp1))
    return;
  WORD index = ED_GetClient(wnd)->contentheight+1;
  strncpy(_GetPropertyTypeName(wnd, index), name, MAX_PROPERTY_STRING);
  ED_Echo(wnd, FMT_COMMAND, index);
  ED_Echo(wnd, STYLE_HIDDEN);
  ED_Echo(wnd, STYLE_LABEL "%-*s", ED_GetClient(wnd)->width, name);
}

LRESULT
ED_NewProperty(HEDWND wnd, DWORD msg, wParam_t wparm, lParam_t lparm)
{
  switch (msg) {
    case EVT_CREATE:
      ED_AllocUserData(wnd, MAX_PROP_TYPES * MAX_PROPERTY_STRING);
      memcpy(ED_GetUserData(wnd), &lparm, sizeof(lparm));
      return 1;
    case EVT_DESTROY:
      ED_ReleaseUserData(wnd);
      return 1;
    case EVT_CCLICK:
      ED_HandleTextInput(WI_KEY_ENTER);
      return 1;
    case EVT_CGRAB:
      ED_SetSelectedItem(wnd, wparm);
      return 1;
    case EVT_PRINT:
      ED_ClearConsole(wnd);
      UI_EnumObjectPropertyTypes(*(LPVOID*)ED_GetUserData(wnd), ED_PrintPropertyNewType, wnd);
      ED_SetSelectedItem(wnd, MIN(1, ED_GetClient(wnd)->contentheight));
      return 1;
    case EVT_HOTKEY:
      switch (LOWORD(wparm)) {
        case ID_NAV_NEXTROW:
          if (ED_GetSelectedItem(wnd) < ED_GetClient(wnd)->contentheight) {
            ED_SetSelectedItem(wnd, ED_GetSelectedItem(wnd)+1);
            ED_ScrollToLocation(wnd, ED_GetSelectedItem(wnd) * CONSOLE_CHAR_HEIGHT);
          }
          return TRUE;
        case ID_NAV_PREVROW:
          if (ED_GetSelectedItem(wnd) > 1) {
            ED_SetSelectedItem(wnd, ED_GetSelectedItem(wnd)-1);
            ED_ScrollToLocation(wnd, ED_GetSelectedItem(wnd) * CONSOLE_CHAR_HEIGHT);
          }
          return TRUE;
        case ID_NAV_ACCEPT:
          if (ED_GetSelectedItem(wnd) > 0) {
            HPROP p = NULL;
            lpcString_t s =_GetPropertyTypeName(wnd, ED_GetSelectedItem(wnd));
            OBJ_FindShortProperty(*(LPVOID*)ED_GetUserData(wnd), s, &p);
            return TRUE;
          } else {
            return FALSE;
          }
        default:
          return FALSE;
      }
      return FALSE;
  }
  
  return 0;
}
