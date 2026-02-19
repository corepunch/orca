#include "ed_local.h"

//static void ED_DrawEmptyTabView(HEDWND wnd) {
//  ED_Echo(wnd, STYLE_LABEL "No panels");
//}

static void ED_PrintTabViewTitleBar(HEDWND wnd) {
  DWORD dwIndex = 1;
  FOR_EACH_LIST(EDWND, hChild, wnd->children) {
    char title[64]={0};
    hChild->fnProc(hChild, EVT_GETTEXT, sizeof(title), title);
    Echo(wnd->pTitleBar, FMT_COMMAND, dwIndex);
    if (ED_GetWindowFlags(hChild)&EDWF_DISABLED) {
      Echo(wnd->pTitleBar, STYLE_HEADER_INACTIVE " %s ", title);
    } else {
      Echo(wnd->pTitleBar, STYLE_HEADER " %s ", title);
    }
    dwIndex++;
  }
}

BOOL TabView_IsActive(HEDWND tab) {
  return !(ED_GetWindowFlags(tab)&EDWF_DISABLED);
}

void TabView_SetActive(HEDWND wnd, HEDWND active) {
  FOR_EACH_LIST(EDWND, tab, wnd->children) {
    if (tab == active) {
      ED_SetWindowFlags(tab, tab->flags & ~EDWF_DISABLED);
    } else {
      ED_SetWindowFlags(tab, tab->flags | EDWF_DISABLED);
    }
  }
}

EDWINPROC(TabView) {
  RECT client = ED_GetClientRect(wnd);
  HEDWND prev = NULL;
  switch (msg) {
    case EVT_CREATE:
      for (EDCREATE *cr = lparm; cr && cr->proc; cr++) {
        ED_CreateWindow(cr->proc, cr->flags, NULL, wnd, cr->parm);
      }
      return 1;
    case EVT_TITLEBAR:
      ED_PrintTabViewTitleBar(wnd);
      return 1;
    case EVT_WINDOWADDED:
      TabView_SetActive(wnd, lparm);
      ED_SetWindowFlags(lparm, ED_GetWindowFlags(lparm)&~EDWF_TITLEBAR);
      ED_SetWindowRect(lparm, &client);
      return 1;
    case EVT_MOVE:
    case EVT_SIZE:
      FOR_EACH_LIST(EDWND, hChild, wnd->children) {
        ED_SetWindowRect(hChild, &client);
      }
      return 1;
    case EVT_HOTKEY:
      switch (LOWORD(wparm)) {
        case ID_NAV_NEXTTAB:
          FOR_EACH_LIST(EDWND, hChild, wnd->children) {
            if (TabView_IsActive(hChild)) {
              TabView_SetActive(wnd, hChild->next?hChild->next:wnd->children);
              break;
            }
          }
          return 1;
        case ID_NAV_PREVTAB:
          FOR_EACH_LIST(EDWND, hChild, wnd->children) {
            if (TabView_IsActive(hChild)) {
              if (prev) {
                TabView_SetActive(wnd, prev);
              } else FOR_EACH_LIST(EDWND, hOther, wnd->children) {
                TabView_SetActive(wnd, hOther);
              }
              break;
            }
            prev = hChild;
          }
          return 1;
        default:
          return 0;
      }
    default:
      return FALSE;
  }
}

//void ED_SetMenuItems(lpcString_t *items) {
//    HEDWND wnd = editor.menuBar;
//    LPTABVIEWSTRUCT menubar = ED_GetUserData(wnd);
//    menubar->selected = 0;
//    menubar->items = items;
//    menubar->previous = editor.focus;
//    memcpy(&menubar->prevtextedit, &editor.textEdit, sizeof(EDTEXTEDIT));
//    ED_SetFocusedPanel(wnd);
//}
