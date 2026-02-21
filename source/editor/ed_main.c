#include "ed_local.h"

BOOL running = TRUE;

struct _EDWND {
  EDPANELPROC fnProc;
  RECT        Rect;
  LPTERMINAL  client;
  LPTERMINAL  pTitleBar;
  LPVOID      usedata;
  VECTOR2     scroll;
  WORD        selectedItem;
  WORD        hoveredItem;
  DWORD       flags;
  HEDWND      parent;
  HEDWND      children;
  HEDWND      next;
  struct {
    LPEDTBBTN buttons;
    DWORD     num_buttons;
  } toolbar;
};

EDWINPROC(RootWindow);
EDWINPROC(SceneView);
EDWINPROC(StatusBar);

EDIMPORT eimp;
EDITOR editor;

HEDWND ED_CreateWindow(EDPANELPROC proc,
                       DWORD flags,
                       LPCRECT rect,
                       HEDWND parent,
                       lParam_t param)
{
  HEDWND self = ZeroAlloc(sizeof(struct _EDWND));
  DWORD width = CONSOLE_WIDTH;
  //  DWORD height = CONSOLE_HEIGHT;
  if (rect) {
    memcpy(&self->Rect, rect, sizeof(RECT));
  } else {
    self->Rect.width = 64;
    self->Rect.height = 64;
  }
  if (parent) {
    ADD_TO_LIST(self, parent->children);
    self->parent = parent;
  } else if (editor.root) {
    ADD_TO_LIST(self, editor.root->children);
    self->parent = editor.root;
  }
  self->flags = flags;
  self->fnProc = proc;
  self->client = ED_CreateConsole(width, CONSOLE_HEIGHT);
  self->pTitleBar = ED_CreateConsole(512, 1);
  ED_SendMessage(self, EVT_CREATE, 0, param);
  ED_SendMessage(parent, EVT_WINDOWADDED, 0, self);
  return self;
}

DWORD ED_GetWindowFlags(HEDWND hwnd) {
  return hwnd->flags;
}
void ED_SetWindowFlags(HEDWND hWnd, DWORD dwFlags) {
  hWnd->flags = dwFlags;
}

void ED_DestroyWindow(HEDWND hWnd) {
  if (editor.focus == hWnd) {
    editor.focus = NULL;
  }
  ED_SendMessage(hWnd, EVT_DESTROY, 0, NULL);
  if (hWnd->parent) {
    REMOVE_FROM_LIST(EDWND, hWnd, hWnd->parent->children);
  }
  FOR_EACH_LIST(struct _EDWND, c, hWnd->children) {
    ED_DestroyWindow(c);
  }
  free(hWnd);
}

void ED_SetUserData(HEDWND wnd, LPVOID lpParm) {
  wnd->usedata = lpParm;
}

LPVOID ED_AllocUserData(HEDWND wnd, DWORD size) {
  wnd->usedata = ZeroAlloc(size);
  return wnd->usedata;
}

void ED_ReleaseUserData(HEDWND wnd) {
  if (wnd->usedata) {
    free(wnd->usedata);
    wnd->usedata = NULL;
  }
}

LPVOID ED_GetUserData(HEDWND wnd) {
  return wnd->usedata;
}

EDPANELPROC ED_GetWindowProc(HEDWND wnd){
  return wnd->fnProc;
}

WORD ED_GetSelectedItem(HEDWND wnd) {
  return wnd->selectedItem;
}

void ED_SetSelectedItem(HEDWND wnd, WORD itemIndex) {
  wnd->selectedItem = itemIndex;
}

void ED_SetHoveredItem(HEDWND wnd, WORD itemIndex) {
  wnd->hoveredItem = itemIndex;
}

HEDWND ED_GetParent(HEDWND wnd) {
  return wnd->parent;
}

void ED_ResetScroll(HEDWND hwnd) {
  memset(&hwnd->scroll, 0, sizeof(hwnd->scroll));
}

VECTOR2 ED_GetScroll(HEDWND wnd) {
  return wnd->scroll;
}

BOOL ED_FunctionEquals(HEDWND wnd, void *ptr) {
  return wnd->fnProc == ptr;
}

void ED_ScrollToLocation(HEDWND wnd, FLOAT location) {
  RECT rect = ED_GetClientRect(wnd);
  wnd->scroll.y = fmin(wnd->scroll.y, rect.height - location);
  wnd->scroll.y = fmax(wnd->scroll.y, -location + CONSOLE_CHAR_HEIGHT);
}

void ED_SetWindowRect(HEDWND wnd, LPCRECT lpRect) {
  wnd->Rect = *lpRect;
  RECT client = ED_GetClientRect(wnd);
  ED_SendMessage(wnd, EVT_MOVE, MAKEDWORD(lpRect->x, lpRect->y), NULL);
  ED_SendMessage(wnd, EVT_SIZE, MAKEDWORD(client.width, client.height), NULL);
}

RECT ED_GetWindowRect(HEDWND wnd) {
  return wnd->Rect;
}

void ED_InvalidateWindow(HEDWND wnd) {
  wnd->flags &= ~EDWF_NODRAW;
  
  FOR_EACH_LIST(EDWND, child, wnd->children) {
    ED_InvalidateWindow(child);
  }
}

ORCA_API void ED_Init(EDIMPORT imp) {
  eimp = imp;

  FS_InitHash();
  
  memset(&editor, 0, sizeof(editor));

  editor.root = ED_InitLayout();
//  editor.root = ZeroAlloc(sizeof(EDWND));
//  editor.root->fnProc = ED_RootWindow;
//  editor.layout = ED_InitLayout();
  
//  EDWINPROC(ProjectSettings);
//  ED_CreateWindow(ED_ProjectSettings, EDWF_TITLEBAR, &(RECT){400,100,600,400}, NULL, NULL);

  ED_RegisterRootCommands();
}

ORCA_API void ED_Shutdown(void) {
  FS_ShutdownHash();
  free(editor.root);
}

static EDWINPROC(BroadcastMessage) {
  ED_SendMessage(wnd, msg, wparm, lparm);
  FOR_EACH_LIST(struct _EDWND, ch, wnd->children) {
    ED_BroadcastMessage(ch, msg, wparm, lparm);
  }
  return TRUE;
}

EDWINPROC(SendMessage) {
  if (wnd == HEDWND_BOARDCAST) {
    ED_BroadcastMessage(editor.root, msg, wparm, lparm);
    return 1;
  }
  if (!wnd || wnd->flags&EDWF_DISABLED)
    return 0;
  DWORD result = wnd->fnProc(wnd, msg, wparm, lparm);
  if (result) {
    return result;
  } else switch (msg) {
    case TB_ADDBUTTONS:
      if (wnd->toolbar.buttons) {
        free(wnd->toolbar.buttons);
        wnd->toolbar.buttons = NULL;
        wnd->toolbar.num_buttons = 0;
      }
      if (wparm) {
        wnd->toolbar.num_buttons = wparm;
        wnd->toolbar.buttons = malloc(sizeof(EDTBBTN)*wparm);
        memcpy(wnd->toolbar.buttons, lparm, sizeof(EDTBBTN)*wparm);
      }
      return TRUE;
    case TB_CHECKBUTTON:
      FOR_LOOP(i, wnd->toolbar.num_buttons) {
        LPEDTBBTN btn = wnd->toolbar.buttons+i;
        if (btn->idCommand != wparm)
          continue;
        if (lparm) {
          btn->fsState |= TBSTATE_CHECKED;
        } else {
          btn->fsState &= ~TBSTATE_CHECKED;
        }
      }
      return TRUE;
    default:
      return FALSE;
  }
}

void ED_DrawTitleBar(HEDWND hWnd) {
  Con_Clear(hWnd->pTitleBar);
  Echo(hWnd->pTitleBar, STYLE_HEADER "\x10\x11");
  if (!ED_SendMessage(hWnd, EVT_TITLEBAR, 0, 0)) {
    char title[64]={0};
    hWnd->fnProc(hWnd, EVT_GETTEXT, sizeof(title), title);
    Echo(hWnd->pTitleBar, STYLE_HEADER " %s ", title);
  }
  while (hWnd->pTitleBar->cursor < hWnd->Rect.width/CONSOLE_CHAR_WIDTH - 2)
//    Echo(hWnd->pTitleBar, STYLE_HEADER "\xb0");
    Echo(hWnd->pTitleBar, STYLE_HEADER " ");
  Echo(hWnd->pTitleBar, STYLE_HEADER "\x14\x15");
  R_DrawConsole(&(DRAWCONSOLESTRUCT){
    .Buffer = hWnd->pTitleBar->buffer,
    .Width = hWnd->pTitleBar->width,
    .Height = hWnd->pTitleBar->height,
    .Cursor = hWnd->pTitleBar->width,
    .Rect = ED_GetTitleBarRect(hWnd),
  });
}

void ED_DrawToolBar(HEDWND wnd) {
  RECT  toolbar  = ED_GetToolBarRect(wnd);
  COLOR panel    = {0.75,0.75,0.75,1};
  COLOR selected = {0.95,0.65,0.25,1};
  
  R_DrawImage(&(DRAWIMAGESTRUCT) {
    .rect = toolbar,
    .color = panel,
  });
  
  FOR_LOOP(i, wnd->toolbar.num_buttons) {
    if (wnd->toolbar.buttons[i].fsState&TBSTATE_CHECKED) {
      R_DrawImage(&(DRAWIMAGESTRUCT) {
        .rect = {
          toolbar.x+i*TOOLBAR_BUTTON_SIZE,
          toolbar.y+(TOOLBAR_HEIGHT-TOOLBAR_BUTTON_SIZE)/2,
          TOOLBAR_BUTTON_SIZE,
          TOOLBAR_BUTTON_SIZE
        }, .color = selected,
      });
    }
    R_DrawToolbarIcon(&(DRAWTOOLBARICONSTRUCT){
      .x=toolbar.x+i*TOOLBAR_BUTTON_SIZE+(TOOLBAR_BUTTON_SIZE-TOOLBAR_ICON_SIZE)/2,
      .y=toolbar.y+(TOOLBAR_HEIGHT-TOOLBAR_ICON_SIZE)/2,
      .icon=wnd->toolbar.buttons[i].iBitmap
    });
  }
}

BOOL ED_IsWindowFocused(HEDWND wnd) {
  if (editor.focus == wnd) {
    return TRUE;
  }
  FOR_EACH_LIST(struct _EDWND, c, wnd->children) {
    if (editor.focus == c) {
      return TRUE;
    }
  }
  return FALSE;
}

void ED_DrawWindowBorder(HEDWND wnd) {
  TERMINALCHAR ch = { .background = WINDOW_BORDER_COLOR };
  RECT r = ED_GetWindowRect(wnd);
  r.x -= 1;
  r.y -= 1;
  r.width += 2;
  r.height += 2;
  R_DrawConsole(&(DRAWCONSOLESTRUCT) {
    .Buffer = &ch,
    .Width = 1,
    .Height = 1,
    .Rect = r,
    .Cursor = -1,
  });
}

void ED_DrawTextContent(HEDWND wnd) {
  if (!ED_GetClient(wnd))
    return;
  WORD h = ED_GetClient(wnd)->contentheight + 1;
  WORD sel = -1;
  if (wnd->hoveredItem) {
    sel = wnd->hoveredItem;
  } else {
    sel = ED_GetSelectedItem(wnd);
  }
  R_DrawConsole(&(DRAWCONSOLESTRUCT) {
    .Buffer = ED_GetClient(wnd)->buffer,
    .Width = ED_GetClient(wnd)->width,
    .Height = MIN(h, ED_GetClient(wnd)->height),
    .Cursor = ED_GetClient(wnd)->cursor,
    .Scroll = wnd->scroll,
    .Rect = ED_GetClientRect(wnd),
    .SelectedItem = sel,
    .StickHeader = FALSE,
    .SoftSelection = !ED_IsWindowFocused(wnd) && !(wnd->flags&EDWF_NOFOCUS),
    .DrawShadow = wnd->flags & EDWF_CASTSHADOW,
  });
}

static void ED_DrawWindow(HEDWND wnd) {
  if (wnd->next) {
    ED_DrawWindow(wnd->next);
  }
  if (!(wnd->flags&EDWF_DISABLED)) {
    ED_DrawWindowBorder(wnd);
    if (wnd->flags&EDWF_TITLEBAR) {
      ED_DrawTitleBar(wnd);
    }
    if (wnd->flags&EDWF_TOOLBAR) {
      ED_DrawToolBar(wnd);
    }
    if (!(wnd->flags & EDWF_NODRAW)) {
      wnd->flags |= EDWF_NODRAW;
      if (!(wnd->flags & EDWF_NOCLEAR)) {
        ED_ClearConsole(wnd);
      }
      ED_SendMessage(wnd, EVT_PRINT, 0, NULL);
      ED_DrawTextContent(wnd);
      ED_SendMessage(wnd, EVT_PAINT, 0, NULL);
    } else {
      ED_DrawTextContent(wnd);
    }
    if (wnd->children) {
      ED_DrawWindow(wnd->children);
    }
  }
}

ORCA_API void ED_Draw(void) {
  R_BeginFrame((struct color){ 0.05, 0.05, 0.05 });

  ED_DrawWindow(editor.root->children);
  
  if (editor.dragItem.active) {
    TERMINALCHAR console[DROP_ITEM_LENGTH] = { 0 };
    lpcString_t str = editor.dragItem.name;
    FOR_LOOP(i, MIN(DROP_ITEM_LENGTH, (DWORD)strlen(str))) {
      console[i].character = str[i];
      console[i].background = 13;
      console[i].foreground = 0;
    }
    R_DrawConsole(&(DRAWCONSOLESTRUCT){
      .Buffer = console,
      .Cursor = -1,
      .Width = (int)strlen(str),
      .Height = 1,
      .Rect = {
        .x = editor.mouseLocation.x - editor.dragItem.offset.x + editor.dragItem.indent * CONSOLE_CHAR_WIDTH,
        .y = editor.mouseLocation.y - editor.dragItem.offset.y,
        .width = CONSOLE_CHAR_WIDTH * (int)strlen(str),
        .height = CONSOLE_CHAR_HEIGHT * 1
      }
    });
  }
  R_EndFrame();
}

void ED_CancelInput(void) {
  editor.textEdit.active = FALSE;
  if (editor.focus) {
    ED_InvalidateWindow(editor.focus);
  }
}

void ED_CancelDragOperation(void) {
  memset(&editor.dragItem, 0, sizeof(editor.dragItem));
}

LPTERMINAL ED_GetClient(HEDWND wnd) {
  return wnd->client;
}

void ED_MoveToFront(HEDWND wnd) {
  HEDWND w = wnd;
  while (w->parent != editor.root && w->parent) w = w->parent;
  REMOVE_FROM_LIST(struct _EDWND, w, editor.root->children);
  ADD_TO_LIST(w, editor.root->children);
}

void ED_SetFocusedPanel(HEDWND wnd) {
  if (editor.focus == wnd || (wnd && (wnd->flags&EDWF_NOFOCUS)))
    return;
  HEDWND _fcs = editor.focus;
  editor.focus = NULL;
  if (_fcs) {
    ED_SendMessage(_fcs, kEventKillFocus, 0, wnd);
    memset(&editor.textEdit, 0, sizeof(editor.textEdit));
  }
  editor.focus = wnd;
  if (wnd) {
    ED_MoveToFront(wnd);
  }
  ED_SendMessage(wnd, kEventSetFocus, 0, NULL);
  if (ED_FindWindow(ED_StatusBar)) {
    ED_InvalidateWindow(ED_FindWindow(ED_StatusBar));
  }
}

static HEDWND ED_FindWindowAtLocation(wParam_t wparam, HEDWND root) {
  VECTOR2 mouse = { LOWORD(wparam), HIWORD(wparam) };
  if (ED_GetWindowFlags(root)&EDWF_DISABLED)
    return NULL;
  FOR_EACH_LIST(struct _EDWND, wnd, root->children) {
    if (ED_FindWindowAtLocation(wparam, wnd)) {
      return ED_FindWindowAtLocation(wparam, wnd);
    }
  }
  RECT rect = ED_GetWindowRect(root);
  return RECT_Contains(&rect, &mouse) ? root : NULL;
}

static LRESULT ED_Command(HEDWND hWnd, wParam_t wparam, LPEDCMD cmds) {
  for (HEDWND w = hWnd; w; w = w->parent) {
    for (EDCMD *cmd = cmds; cmd && cmd->szCommand; cmd++) {
      if (cmd->dwKey != wparam) {
        if (ED_Command(w, wparam, cmd->subcommands)) {
          return TRUE;
        }
        continue;
      }
      if (ED_SendMessage(w, EVT_HOTKEY, cmd->szCommand, NULL)) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

TERMINALCHAR
ED_GetClickedConsoleData(HEDWND wnd, wParam_t wparam, DWORD *index) {
  RECT rect = ED_GetClientRect(wnd);
  VECTOR2 mouse = { LOWORD(wparam), HIWORD(wparam) };
  VECTOR2 local = VEC2_Sub(&mouse, (LPVECTOR2)&rect);
  VECTOR2 scrolled = VEC2_Sub(&local, &wnd->scroll);
  DWORD x = scrolled.x / CONSOLE_CHAR_WIDTH;
  DWORD y = scrolled.y / CONSOLE_CHAR_HEIGHT;
  if (x < ED_GetClient(wnd)->width && y < ED_GetClient(wnd)->height) {
    *index = y * ED_GetClient(wnd)->width + x;
  }
  
  return Con_GetPoint(ED_GetClient(wnd), scrolled);
}

TERMINALCHAR ED_GetClickedTitleBarData(HEDWND wnd, wParam_t wparam) {
  RECT rect = ED_GetTitleBarRect(wnd);
  VECTOR2 mouse = { LOWORD(wparam), HIWORD(wparam) };
  VECTOR2 local = VEC2_Sub(&mouse, (LPVECTOR2)&rect);
  return Con_GetPoint(wnd->pTitleBar, local);
}

BOOL ED_ScrollConsole(HEDWND wnd, wParam_t wparam, lParam_t lparam) {
  if (!wnd || !wnd->client)
    return FALSE;
  int const h = (ED_GetClient(wnd)->contentheight) * CONSOLE_CHAR_HEIGHT;
  int16_t delta = (int16_t)HIWORD((intptr_t)lparam);
  wnd->scroll.y += delta / SCROLL_SENSIVITY;
  wnd->scroll.y = MAX(ED_GetClientRect(wnd).height - h, wnd->scroll.y);
  wnd->scroll.y = MIN(0, wnd->scroll.y);
  return TRUE;
}

VECTOR2 ED_GetDragOffset(LPTERMINAL lpTerminal, VECTOR2 m) {
  VECTOR2 offset = {
    fmodf(m.x, CONSOLE_CHAR_WIDTH),
    fmodf(m.y, CONSOLE_CHAR_HEIGHT),
  };
  TERMINALCHAR p1 = Con_GetPoint(lpTerminal, m);
  while (TRUE) {
    m.x -= CONSOLE_CHAR_WIDTH;
    if (m.x < 0)
      break;
    TERMINALCHAR p2 = Con_GetPoint(lpTerminal, m);
    if (p2.item != p1.item)
      break;
    offset.x += CONSOLE_CHAR_WIDTH;
  }
  return offset;
}

static LRESULT HandleCommand(DWORD msg, wParam_t wparam, lParam_t lparam) {
  for (HEDWND w = editor.focus; w; w = w->parent) {
    if (ED_Command(w, wparam, editor.commands)) {
      return TRUE;
    }
    if (ED_SendMessage(w, msg, wparam, lparam)) {
      ED_InvalidateWindow(w);
      return TRUE;
    }
  }
  return FALSE;
}

static BOOL bEditorVisible = FALSE;

#include "../renderer/r_local.h"

static DWORD _LocalCoord(HEDWND wnd, DWORD wparam) {
  if (!wnd) return wparam;
  WORD x = LOWORD(wparam) - ED_GetClientRect(wnd).x;
  WORD y = HIWORD(wparam) - ED_GetClientRect(wnd).y;
  return MAKEDWORD(x, y);
}

#define ClickedTitlebar(wnd, wparam) \
((wnd->flags&EDWF_TITLEBAR) && \
HIWORD(wparam) - wnd->Rect.y < CONSOLE_CHAR_HEIGHT)

#define ClickedToolbar(wnd, wparam) \
((wnd->flags&EDWF_TOOLBAR) && \
HIWORD(wparam) - wnd->Rect.y < ED_GetToolBarRect(wnd).y + ED_GetToolBarRect(wnd).height)

LRESULT ED_DispatchMessage(DWORD msg, wParam_t wparam, lParam_t lparam) {
  if (msg == kEventKeyDown && (wparam&0xff) == '`') {
    if (bEditorVisible) {
      bEditorVisible = FALSE;
    } else {
      bEditorVisible = TRUE;
    }
    return TRUE;
  }
  if (!bEditorVisible) {
    return FALSE;
  }
  HEDWND wnd = ED_FindWindowAtLocation(wparam, editor.root);
  TERMINALCHAR data = {0};
  DWORD curindex=0;
  static HEDWND dragging=NULL;
  static DWORD dragged=0;
  if (dragging) {
    switch (msg) {
      case kEventLeftMouseDragged:
        dragging->Rect.x += (int16_t)LOWORD((intptr_t)lparam);
        dragging->Rect.y += (int16_t)HIWORD((intptr_t)lparam);
        dragged += abs((int16_t)LOWORD((intptr_t)lparam)) + abs((int16_t)HIWORD((intptr_t)lparam));
        ED_SendMessage(dragging, EVT_MOVE, MAKEDWORD((int16_t)dragging->Rect.x, (int16_t)dragging->Rect.y), NULL);
        return TRUE;
      case kEventLeftMouseUp:
        dragging = NULL;
        if (dragged > DRAG_THRESHOLD) {
          dragged = 0;
          return TRUE;
        }
        break;
    }
  }
  if (wnd) {
    if (ClickedTitlebar(wnd, wparam)) {
      TERMINALCHAR ch = ED_GetClickedTitleBarData(wnd, wparam);
      switch (msg) {
        case kEventLeftMouseDown:
          dragging = wnd;
          dragged = 0;
          ED_SetFocusedPanel(wnd);
          return TRUE;
        case kEventLeftMouseUp:
          if (ch.character == 0x10 || ch.character == 0x11) { // close button
            ED_DestroyWindow(wnd);
            ED_InvalidateWindow(editor.root);
            return 1;
          } else if (wnd->children && ch.item > 0) {
            FOR_EACH_LIST(EDWND, hChild, wnd->children) {
              if (ch.item == 1) {
                ED_SetFocusedPanel(hChild);
                void TabView_SetActive(HEDWND wnd, HEDWND active);
                TabView_SetActive(wnd, hChild);
                return 1;
              }
              ch.item--;
            }
            return 1;
          }
      }
    }
    
    if (ED_GetClient(wnd)) {
      data = ED_GetClickedConsoleData(wnd, wparam, &curindex);
    }
    
    if (ClickedToolbar(wnd, wparam)) {
      DWORD but = (LOWORD(wparam)-wnd->Rect.x)/TOOLBAR_BUTTON_SIZE;
      switch (but < wnd->toolbar.num_buttons ? msg : 0) {
        case kEventLeftMouseUp:
          ED_SetFocusedPanel(wnd);
          ED_SendMessage(wnd, EVT_HOTKEY, MAKEDWORD(wnd->toolbar.buttons[but].idCommand, 0), NULL);
          return 1;
      }
    }
    switch (msg) {
      case kEventLeftMouseDown:
      case kEventRightMouseDown:
      case kEventOtherMouseDown:
      case kEventLeftMouseUp:
      case kEventRightMouseUp:
      case kEventOtherMouseUp:
      case kEventLeftMouseDragged:
      case kEventRightMouseDragged:
      case kEventOtherMouseDragged:
      case kEventMouseMoved:
      case kEventScrollWheel:
        if (ClickedToolbar(wnd, wparam))
          return TRUE;
    }
  }
  switch (msg) {
    case kEventWindowPaint:
      ED_SetWindowRect(editor.root, &(RECT){0,0,LOWORD(wparam),HIWORD(wparam)});
      ED_Draw();
      return TRUE;
    case kEventLeftDoubleClick:
      ED_SendMessage(wnd, EVT_CDCLICK, curindex, &data);
      return TRUE;
    case kEventLeftMouseUp:
//      ED_CancelInput();
      if (editor.dragItem.active) {
//        ED_SendMessage(wnd, EVT_MOUSEDROP, 0, evt);
        ED_SendMessage(wnd, EVT_CDROP, data.item, &editor.dragItem);
      } else {
        ED_SendMessage(wnd, kEventLeftMouseUp, _LocalCoord(wnd, wparam), lparam);
        ED_SendMessage(wnd, EVT_CCLICK, curindex, &data);
      }
      ED_CancelDragOperation();
      return TRUE;
    case kEventRightMouseUp:
      ED_SendMessage(wnd, kEventRightMouseUp, _LocalCoord(wnd, wparam), lparam);
      ED_SendMessage(wnd, EVT_CONTEXTMENU, _LocalCoord(wnd, wparam), &data);
      return TRUE;
    case kEventScrollWheel:
      if (ED_SendMessage(wnd, kEventScrollWheel, _LocalCoord(wnd, wparam), lparam) ||
          ED_ScrollConsole(wnd, _LocalCoord(wnd, wparam), lparam)) {
        return TRUE;
      } else {
        return FALSE;
      }
    case kEventLeftMouseDown:
      if (ED_SendMessage(wnd, EVT_CGRAB, data.item, &data) ||
          ED_SendMessage(wnd, kEventLeftMouseDown, _LocalCoord(wnd, wparam), lparam))
      {
        RECT rect = ED_GetClientRect(wnd);
        VECTOR2 mouse = { LOWORD(wparam), HIWORD(wparam) };
        VECTOR2 local = VEC2_Sub(&mouse, (LPVECTOR2)&rect);
        VECTOR2 ptr = VEC2_Sub(&local, &wnd->scroll);
        editor.dragItem.offset = ED_GetDragOffset(ED_GetClient(wnd), ptr);
        editor.dragItem.startdrag = *(LPVECTOR2)&mouse;
        editor.dragItem.window = wnd;
        return TRUE;
      } else {
        return FALSE;
      }
    case kEventLeftMouseDragged:
      editor.mouseLocation.x = LOWORD(wparam);
      editor.mouseLocation.y = HIWORD(wparam);
      // LPVECTOR2 location = (LPVECTOR2)&evt->location;
      FLOAT dist = VEC2_Distance(&editor.dragItem.startdrag,
                                 &editor.mouseLocation);
      if (editor.dragItem.dragtype &&
//          editor.dragItem.window == wnd &&
          dist > DRAG_N_DROP_DISTANCE)
      {
        editor.dragItem.active = TRUE;
        ED_SendMessage(wnd, EVT_CDRAG, data.item, &editor.dragItem);
        ED_SendMessage(wnd, EVT_MOUSEDRAG, wparam, &data);
      }
      return ED_SendMessage(wnd, kEventLeftMouseDragged, _LocalCoord(wnd, wparam), lparam);
    case kEventKeyDown:
      if (editor.textEdit.active) {
        ED_HandleTextInput(wparam);
        return TRUE;
      } else if (!editor.focus) {
        return FALSE;
      } else if (HandleCommand(msg, wparam, lparam)) {
        return TRUE;
      } else if (ED_SendMessage(wnd, kEventKeyDown, wparam, lparam)) {
        return TRUE;
      } else {
        return FALSE;
      }
    case kEventWindowResized:
      ED_SetWindowRect(editor.root, &(RECT){0,0,LOWORD(wparam),HIWORD(wparam)});
      ED_Draw();
      return TRUE;
    default:
      //            lpSystem->HACK_HandleEvent(L, evt);
      return TRUE;
  }
}

static HEDWND _FindWindowForDocument(HEDWND root, lpcString_t path) {
  if (root->fnProc == ED_SceneView) {
    lpcString_t doc = ED_GetWindowDocument(root);
    if (doc && !strcmp(doc, path)) {
      return root;
    }
  }
  FOR_EACH_LIST(struct _EDWND, w, root->children) {
    HEDWND p = _FindWindowForDocument(w, path);
    if (p) {
      return p;
    }
  }
  return NULL;
}

HEDWND ED_OpenDocument(lpcString_t szFilename) {
  static BYTE counter = 0;
  RECT rect = {
    .x = 64 + counter / 2,
    .y = 64 + counter,
    .width = CONSOLE_CHAR_WIDTH * 64,
    .height = CONSOLE_CHAR_HEIGHT * 32
  };
  counter += 32;
  HEDWND existing = _FindWindowForDocument(editor.root, szFilename);
  if (existing) {
    ED_MoveToFront(existing);
    return existing;
  }
  if (strstr(szFilename, ".xml")) {
    return ED_CreateWindow(ED_SceneView, 0, &rect, editor.root, (void*)szFilename);
  }
  return NULL;
}

HEDWND ED_FindWindowInChildren(HEDWND wnd, EDPANELPROC proc) {
  if (wnd->fnProc == proc) {
    return wnd;
  }
  FOR_EACH_LIST(struct _EDWND, ch, wnd->children) {
    if (ED_FindWindowInChildren(ch, proc)) {
      return ED_FindWindowInChildren(ch, proc);
    }
  }
  return NULL;
}

HEDWND ED_FindWindow(EDPANELPROC proc) {
  return ED_FindWindowInChildren(editor.root, proc);
}

void ED_InvalidateEditor(void) {
  ED_Shutdown();
  ED_Init(eimp);
}

LPTERMINAL ED_CreateConsole(DWORD dwWidth, DWORD dwHeight) {
  LPTERMINAL console = ZeroAlloc(sizeof(struct _TERMINAL));
  console->width = dwWidth;
  console->height = dwHeight;
  return console;
}

void ED_PrintStatus(lpcString_t format, ...) {
  EDWINPROC(MenuBar);
  HEDWND menubar = ED_FindWindow(ED_MenuBar);
  va_list args;
  va_start(args, format);
  vsnprintf(editor.tooltip, sizeof(editor.tooltip), format, args);
  va_end(args);
  ED_InvalidateWindow(menubar);
}

ORCA_API BOOL ED_IsRunning(void) {
  return !editor.bHasFinished;
}

static int _DispatchMessage(lua_State *L, struct WI_Message *msg) {
  return ED_DispatchMessage(msg->message, msg->wParam, msg->lParam);
}

static int f_setScreen(lua_State *L) {
  editor.screen = luaX_checkObject(L, 1);
  ED_SendMessage(HEDWND_BOARDCAST, EVT_COMMAND, GAME_SELECT_SCREEN, editor.screen);
  return 0;
}

static luaL_Reg const lib_editor[] = {
  { "setScreen", f_setScreen },
  { NULL, NULL }
};

ORCA_API int luaopen_orca_editor(lua_State* L)
{
  luaL_newlib(L, lib_editor);

  lpcString_t WI_KeynumToString(uint32_t);
  
  ED_Init((EDIMPORT) {
    .GetOpenFileName = WI_GetOpenFileName,
    .GetFolderName = WI_GetFolderName,
//    .WI_KeynumToString = WI_KeynumToString,
  });
  
  editor.L = L;
  
  SV_RegisterMessageProc(_DispatchMessage);
  
  return 1;
}

#include "ed_tabview.h"
