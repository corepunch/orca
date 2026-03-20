#include "windows_local.h"
#include "../platform.h"

/* Internal event ring-buffer (power-of-two size so index wrap is a mask) */
static struct
{
  EVENT    queue[0x10000];
  uint16_t write, read;
  float    pointer_x, pointer_y;
} events = { 0 };

/* Previous mouse position for computing drag deltas */
static int16_t s_last_mouse_x = 0;
static int16_t s_last_mouse_y = 0;

/* Map a Win32 virtual-key code to a WI_KEY_* constant.
   Extended-key flag (bit 24 of the LPARAM) is passed in ext_key
   so that numpad-Enter can be distinguished from regular Enter. */
static uint32_t
vk_to_wi_key(WPARAM vk, int ext_key)
{
  switch (vk) {
  case VK_TAB:      return WI_KEY_TAB;
  case VK_RETURN:   return ext_key ? WI_KEY_KP_ENTER : WI_KEY_ENTER;
  case VK_ESCAPE:   return WI_KEY_ESCAPE;
  case VK_SPACE:    return WI_KEY_SPACE;
  case VK_BACK:     return WI_KEY_BACKSPACE;

  case VK_UP:       return WI_KEY_UPARROW;
  case VK_DOWN:     return WI_KEY_DOWNARROW;
  case VK_LEFT:     return WI_KEY_LEFTARROW;
  case VK_RIGHT:    return WI_KEY_RIGHTARROW;

  case VK_SHIFT:
  case VK_LSHIFT:
  case VK_RSHIFT:   return WI_KEY_SHIFT;
  case VK_CONTROL:
  case VK_LCONTROL:
  case VK_RCONTROL: return WI_KEY_CTRL;
  case VK_MENU:
  case VK_LMENU:
  case VK_RMENU:    return WI_KEY_ALT;

  case VK_F1:       return WI_KEY_F1;
  case VK_F2:       return WI_KEY_F2;
  case VK_F3:       return WI_KEY_F3;
  case VK_F4:       return WI_KEY_F4;
  case VK_F5:       return WI_KEY_F5;
  case VK_F6:       return WI_KEY_F6;
  case VK_F7:       return WI_KEY_F7;
  case VK_F8:       return WI_KEY_F8;
  case VK_F9:       return WI_KEY_F9;
  case VK_F10:      return WI_KEY_F10;
  case VK_F11:      return WI_KEY_F11;
  case VK_F12:      return WI_KEY_F12;

  case VK_INSERT:   return WI_KEY_INS;
  case VK_DELETE:   return WI_KEY_DEL;
  case VK_PRIOR:    return WI_KEY_PGUP;
  case VK_NEXT:     return WI_KEY_PGDN;
  case VK_HOME:     return WI_KEY_HOME;
  case VK_END:      return WI_KEY_END;
  case VK_PAUSE:    return WI_KEY_PAUSE;

  /* Numpad (num-lock off — cursor keys)  */
  case VK_NUMPAD7:  return WI_KEY_KP_HOME;
  case VK_NUMPAD8:  return WI_KEY_KP_UPARROW;
  case VK_NUMPAD9:  return WI_KEY_KP_PGUP;
  case VK_NUMPAD4:  return WI_KEY_KP_LEFTARROW;
  case VK_NUMPAD5:  return WI_KEY_KP_5;
  case VK_NUMPAD6:  return WI_KEY_KP_RIGHTARROW;
  case VK_NUMPAD1:  return WI_KEY_KP_END;
  case VK_NUMPAD2:  return WI_KEY_KP_DOWNARROW;
  case VK_NUMPAD3:  return WI_KEY_KP_PGDN;
  case VK_NUMPAD0:  return WI_KEY_KP_INS;
  case VK_DECIMAL:  return WI_KEY_KP_DEL;
  case VK_DIVIDE:   return WI_KEY_KP_SLASH;
  case VK_SUBTRACT: return WI_KEY_KP_MINUS;
  case VK_ADD:      return WI_KEY_KP_PLUS;

  default:
    /* Letters and digits share their WI_KEY value with ASCII */
    if ((vk >= 'A' && vk <= 'Z') || (vk >= '0' && vk <= '9')) {
      return (uint32_t)vk;
    }
    return 0;
  }
}

/* Collect modifier flags from the current keyboard state */
static uint32_t
win32_modifiers(void)
{
  uint32_t mods = 0;
  if (GetKeyState(VK_SHIFT)   & 0x8000) mods |= WI_MOD_SHIFT;
  if (GetKeyState(VK_CONTROL) & 0x8000) mods |= WI_MOD_CTRL;
  if (GetKeyState(VK_MENU)    & 0x8000) mods |= WI_MOD_ALT;
  if (GetKeyState(VK_LWIN)    & 0x8000) mods |= WI_MOD_CMD;
  if (GetKeyState(VK_RWIN)    & 0x8000) mods |= WI_MOD_CMD;
  return mods;
}

/* Win32 window procedure — translates native messages into WI events */
LRESULT CALLBACK
WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg) {

  case WM_KEYDOWN:
  case WM_SYSKEYDOWN: {
    int ext = (lparam >> 24) & 1; /* extended-key flag */
    uint32_t key = vk_to_wi_key(wparam, ext);
    if (key) {
      events.queue[events.write++] = (EVENT){
        .message = kEventKeyDown,
        .wParam  = key | win32_modifiers(),
      };
    }
    break;
  }

  case WM_KEYUP:
  case WM_SYSKEYUP: {
    int ext = (lparam >> 24) & 1;
    uint32_t key = vk_to_wi_key(wparam, ext);
    if (key) {
      events.queue[events.write++] = (EVENT){
        .message = kEventKeyUp,
        .wParam  = key | win32_modifiers(),
      };
    }
    break;
  }

  case WM_CHAR: {
    uint32_t ch = (uint32_t)wparam;
    if (ch >= 32 && ch < 127) {
      events.queue[events.write++] = (EVENT){
        .message = kEventChar,
        .wParam  = ch,
      };
    }
    break;
  }

  case WM_LBUTTONDOWN:
    SetCapture(hwnd);
    events.queue[events.write++] = (EVENT){
      .message = kEventLeftMouseDown,
      .x = (uint16_t)LOWORD(lparam),
      .y = (uint16_t)HIWORD(lparam),
    };
    break;

  case WM_LBUTTONUP:
    ReleaseCapture();
    events.queue[events.write++] = (EVENT){
      .message = kEventLeftMouseUp,
      .x = (uint16_t)LOWORD(lparam),
      .y = (uint16_t)HIWORD(lparam),
    };
    break;

  case WM_LBUTTONDBLCLK:
    events.queue[events.write++] = (EVENT){
      .message = kEventLeftDoubleClick,
      .x = (uint16_t)LOWORD(lparam),
      .y = (uint16_t)HIWORD(lparam),
    };
    break;

  case WM_RBUTTONDOWN:
    events.queue[events.write++] = (EVENT){
      .message = kEventRightMouseDown,
      .x = (uint16_t)LOWORD(lparam),
      .y = (uint16_t)HIWORD(lparam),
    };
    break;

  case WM_RBUTTONUP:
    events.queue[events.write++] = (EVENT){
      .message = kEventRightMouseUp,
      .x = (uint16_t)LOWORD(lparam),
      .y = (uint16_t)HIWORD(lparam),
    };
    break;

  case WM_MBUTTONDOWN:
    events.queue[events.write++] = (EVENT){
      .message = kEventOtherMouseDown,
      .x = (uint16_t)LOWORD(lparam),
      .y = (uint16_t)HIWORD(lparam),
    };
    break;

  case WM_MBUTTONUP:
    events.queue[events.write++] = (EVENT){
      .message = kEventOtherMouseUp,
      .x = (uint16_t)LOWORD(lparam),
      .y = (uint16_t)HIWORD(lparam),
    };
    break;

  case WM_RBUTTONDBLCLK:
    events.queue[events.write++] = (EVENT){
      .message = kEventRightDoubleClick,
      .x = (uint16_t)LOWORD(lparam),
      .y = (uint16_t)HIWORD(lparam),
    };
    break;

  case WM_MBUTTONDBLCLK:
    events.queue[events.write++] = (EVENT){
      .message = kEventOtherDoubleClick,
      .x = (uint16_t)LOWORD(lparam),
      .y = (uint16_t)HIWORD(lparam),
    };
    break;

  case WM_MOUSEMOVE: {
    int16_t cur_x = (int16_t)LOWORD(lparam);
    int16_t cur_y = (int16_t)HIWORD(lparam);
    int16_t dx = cur_x - s_last_mouse_x;
    int16_t dy = cur_y - s_last_mouse_y;
    s_last_mouse_x = cur_x;
    s_last_mouse_y = cur_y;
    events.pointer_x = (float)cur_x;
    events.pointer_y = (float)cur_y;

    uint32_t msg;
    if (wparam & MK_LBUTTON)
      msg = kEventLeftMouseDragged;
    else if (wparam & MK_RBUTTON)
      msg = kEventRightMouseDragged;
    else if (wparam & MK_MBUTTON)
      msg = kEventOtherMouseDragged;
    else
      msg = kEventMouseMoved;

    events.queue[events.write++] = (EVENT){
      .message = msg,
      .x = (uint16_t)cur_x,
      .y = (uint16_t)cur_y,
      .dx = dx,
      .dy = dy,
    };
    break;
  }

  case WM_MOUSEWHEEL: {
    short delta = (short)HIWORD(wparam);
    int16_t dy  = (delta > 0) ? 1 : -1;
    /* Convert screen coords to client coords for the position */
    POINT pt = { LOWORD(lparam), HIWORD(lparam) };
    ScreenToClient(hwnd, &pt);
    events.queue[events.write++] = (EVENT){
      .message = kEventScrollWheel,
      .x  = (uint16_t)pt.x,
      .y  = (uint16_t)pt.y,
      .dy = dy,
    };
    break;
  }

  case WM_SIZE: {
    int nw = (int)LOWORD(lparam);
    int nh = (int)HIWORD(lparam);
    if (nw != g_win_width || nh != g_win_height) {
      g_win_width  = nw;
      g_win_height = nh;
      events.queue[events.write++] = (EVENT){
        .message = kEventWindowResized,
        .wParam  = MAKEDWORD(nw, nh),
      };
    }
    break;
  }

  case WM_PAINT: {
    PAINTSTRUCT ps;
    BeginPaint(hwnd, &ps);
    EndPaint(hwnd, &ps);
    events.queue[events.write++] = (EVENT){
      .message = kEventWindowPaint,
    };
    break;
  }

  case WM_SETFOCUS:
    events.queue[events.write++] = (EVENT){
      .message = kEventSetFocus,
    };
    break;

  case WM_KILLFOCUS:
    events.queue[events.write++] = (EVENT){
      .message = kEventKillFocus,
    };
    break;

  case WM_CLOSE:
    events.queue[events.write++] = (EVENT){
      .message = kEventWindowClosed,
    };
    /* Do not call DestroyWindow; let the application decide */
    return 0;

  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;

  default:
    break;
  }

  return DefWindowProcA(hwnd, msg, wparam, lparam);
}

/* Pump the Win32 message queue, pushing translated events onto the ring buffer */
static void
win32_process_messages(void)
{
  MSG msg;
  while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
  }
}

int
WI_WaitEvent(longTime_t timeout_ms)
{
  if (timeout_ms == 0) {
    /* Block indefinitely until a message arrives */
    WaitMessage();
    win32_process_messages();
    return 1;
  }
  DWORD ret = MsgWaitForMultipleObjects(0, NULL, FALSE,
                                         (DWORD)timeout_ms, QS_ALLINPUT);
  win32_process_messages();
  return (ret == WAIT_OBJECT_0) ? 1 : 0;
}

int
WI_PollEvent(PEVENT pEvent)
{
  win32_process_messages();

  if (events.read != events.write) {
    *pEvent = events.queue[events.read++];
    return 1;
  }
  return 0;
}

void
WI_PostMessageW(void *hobj, uint32_t event, uint32_t wparam, void *lparam)
{
  if (events.write - events.read >=
      (int)(sizeof(events.queue) / sizeof(events.queue[0]))) {
    return;
  }
  events.queue[events.write++] = (EVENT){
    .target  = hobj,
    .message = event,
    .wParam  = wparam,
    .lParam  = lparam,
  };
}

void
WI_RemoveFromQueue(void *hobj)
{
  uint16_t r = events.read;
  uint16_t w = events.write;
  uint16_t nw = events.read;

  while (r != w) {
    if (events.queue[r].target != hobj) {
      events.queue[nw++] = events.queue[r];
    }
    r++;
  }
  events.write = nw;
}

void
NotifyFileDropEvent(char const *filename, float x, float y)
{
  (void)filename;
  (void)x;
  (void)y;
}
