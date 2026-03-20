#include <EGL/egl.h>
#include <screen/screen.h>
#include <sys/keycodes.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "qnx_local.h"

extern EGLDisplay egl_display;
extern EGLContext egl_ctx;
extern EGLSurface egl_surface;
extern screen_context_t screen_ctx;

extern void  BeginPaint(HWND hWnd);
extern void  EndPaint(HWND hWnd, LPMETRICS lpMetrics);
extern float GetWindowScale(HWND hWnd);
extern void  GetWindowSize(HWND hWnd, LPSIZE2 lpSize);

/*
 * Event queue with blocking wait support
 */

static struct
{
  struct WI_Message data[0x10000];
  uint16_t read, write;
} queue = { 0 };

static sem_t event_sem;

/* Screen event handle (created on first use) */
static screen_event_t screen_ev = NULL;

/* Previous pointer position and button state for drag/delta tracking */
static int qnx_ptr_x = 0;
static int qnx_ptr_y = 0;
static int qnx_buttons = 0;

/* Map a QNX key symbol to a WI_KEY_* constant */
static uint32_t
qnx_keysym_to_wi(int sym)
{
  /* Printable ASCII: normalise letters to lowercase */
  if (sym >= 0x20 && sym <= 0x7e)
    return (uint32_t)tolower(sym);

  switch (sym) {
  case KEYCODE_TAB:        return WI_KEY_TAB;
  case KEYCODE_RETURN:     return WI_KEY_ENTER;
  case KEYCODE_ESCAPE:     return WI_KEY_ESCAPE;
  case KEYCODE_SPACE:      return WI_KEY_SPACE;
  case KEYCODE_BACKSPACE:  return WI_KEY_BACKSPACE;

  case KEYCODE_UP:         return WI_KEY_UPARROW;
  case KEYCODE_DOWN:       return WI_KEY_DOWNARROW;
  case KEYCODE_LEFT:       return WI_KEY_LEFTARROW;
  case KEYCODE_RIGHT:      return WI_KEY_RIGHTARROW;

  case KEYCODE_LEFT_ALT:
  case KEYCODE_RIGHT_ALT:  return WI_KEY_ALT;
  case KEYCODE_LEFT_CTRL:
  case KEYCODE_RIGHT_CTRL: return WI_KEY_CTRL;
  case KEYCODE_LEFT_SHIFT:
  case KEYCODE_RIGHT_SHIFT: return WI_KEY_SHIFT;

  case KEYCODE_F1:         return WI_KEY_F1;
  case KEYCODE_F2:         return WI_KEY_F2;
  case KEYCODE_F3:         return WI_KEY_F3;
  case KEYCODE_F4:         return WI_KEY_F4;
  case KEYCODE_F5:         return WI_KEY_F5;
  case KEYCODE_F6:         return WI_KEY_F6;
  case KEYCODE_F7:         return WI_KEY_F7;
  case KEYCODE_F8:         return WI_KEY_F8;
  case KEYCODE_F9:         return WI_KEY_F9;
  case KEYCODE_F10:        return WI_KEY_F10;
  case KEYCODE_F11:        return WI_KEY_F11;
  case KEYCODE_F12:        return WI_KEY_F12;

  case KEYCODE_INSERT:     return WI_KEY_INS;
  case KEYCODE_DELETE:     return WI_KEY_DEL;
  case KEYCODE_PG_UP:      return WI_KEY_PGUP;
  case KEYCODE_PG_DOWN:    return WI_KEY_PGDN;
  case KEYCODE_HOME:       return WI_KEY_HOME;
  case KEYCODE_END:        return WI_KEY_END;
  case KEYCODE_PAUSE:      return WI_KEY_PAUSE;

  case KEYCODE_KP_HOME:    return WI_KEY_KP_HOME;
  case KEYCODE_KP_UP:      return WI_KEY_KP_UPARROW;
  case KEYCODE_KP_PG_UP:   return WI_KEY_KP_PGUP;
  case KEYCODE_KP_LEFT:    return WI_KEY_KP_LEFTARROW;
  case KEYCODE_KP_FIVE:    return WI_KEY_KP_5;
  case KEYCODE_KP_RIGHT:   return WI_KEY_KP_RIGHTARROW;
  case KEYCODE_KP_END:     return WI_KEY_KP_END;
  case KEYCODE_KP_DOWN:    return WI_KEY_KP_DOWNARROW;
  case KEYCODE_KP_PG_DOWN: return WI_KEY_KP_PGDN;
  case KEYCODE_KP_ENTER:   return WI_KEY_KP_ENTER;
  case KEYCODE_KP_INSERT:  return WI_KEY_KP_INS;
  case KEYCODE_KP_DELETE:  return WI_KEY_KP_DEL;
  case KEYCODE_KP_DIVIDE:  return WI_KEY_KP_SLASH;
  case KEYCODE_KP_MINUS:   return WI_KEY_KP_MINUS;
  case KEYCODE_KP_PLUS:    return WI_KEY_KP_PLUS;

  default: return 0;
  }
}

/* Collect WI modifier flags from a QNX key modifier bitmask */
static uint32_t
qnx_key_modifiers(int mods)
{
  uint32_t wi_mods = 0;
  if (mods & KEYMOD_SHIFT)   wi_mods |= WI_MOD_SHIFT;
  if (mods & KEYMOD_CTRL)    wi_mods |= WI_MOD_CTRL;
  if (mods & KEYMOD_ALT)     wi_mods |= WI_MOD_ALT;
  return wi_mods;
}

/* Drain pending QNX Screen events into the WI queue */
static void
qnx_process_screen_events(void)
{
  if (!screen_ctx)
    return;

  if (!screen_ev) {
    if (screen_create_event(&screen_ev) != 0) {
      screen_ev = NULL;
      return;
    }
  }

  /* Double-click state */
  static uint32_t last_btn = 0;
  static uint64_t last_btn_ns = 0;
  static int      last_btn_x = 0;
  static int      last_btn_y = 0;

  while (screen_get_event(screen_ctx, screen_ev, 0) == 0) {
    int type = SCREEN_EVENT_NONE;
    screen_get_event_property_iv(screen_ev, SCREEN_PROPERTY_TYPE, &type);

    if (type == SCREEN_EVENT_NONE)
      break;

    switch (type) {

    case SCREEN_EVENT_POINTER: {
      int buttons = 0;
      int pos[2] = { 0, 0 };
      screen_get_event_property_iv(screen_ev, SCREEN_PROPERTY_BUTTONS,         &buttons);
      screen_get_event_property_iv(screen_ev, SCREEN_PROPERTY_SOURCE_POSITION, pos);

      int16_t dx = (int16_t)(pos[0] - qnx_ptr_x);
      int16_t dy = (int16_t)(pos[1] - qnx_ptr_y);
      int prev_buttons = qnx_buttons;
      qnx_ptr_x  = pos[0];
      qnx_ptr_y  = pos[1];
      qnx_buttons = buttons;

      /* Detect button press/release transitions */
      int pressed  = buttons  & ~prev_buttons;
      int released = prev_buttons & ~buttons;

      /* Scroll wheel: QNX uses bit flags SCREEN_MOUSE_BUTTON_SCROLL_UP/DOWN */
      if (buttons & SCREEN_MOUSE_BUTTON_SCROLL_UP) {
        struct WI_Message msg = {
          .message = kEventScrollWheel,
          .x = (uint16_t)pos[0],
          .y = (uint16_t)pos[1],
          .dy = 1,
        };
        queue.data[queue.write++] = msg;
        sem_post(&event_sem);
        break;
      }
      if (buttons & SCREEN_MOUSE_BUTTON_SCROLL_DOWN) {
        struct WI_Message msg = {
          .message = kEventScrollWheel,
          .x = (uint16_t)pos[0],
          .y = (uint16_t)pos[1],
          .dy = -1,
        };
        queue.data[queue.write++] = msg;
        sem_post(&event_sem);
        break;
      }

      /* Button press/release events */
      struct { int mask; uint32_t down; uint32_t up; uint32_t dbl; } btns[] = {
        { SCREEN_LEFT_MOUSE_BUTTON,   kEventLeftMouseDown,  kEventLeftMouseUp,  kEventLeftDoubleClick  },
        { SCREEN_RIGHT_MOUSE_BUTTON,  kEventRightMouseDown, kEventRightMouseUp, kEventRightDoubleClick },
        { SCREEN_MIDDLE_MOUSE_BUTTON, kEventOtherMouseDown, kEventOtherMouseUp, kEventOtherDoubleClick },
      };

      for (int i = 0; i < 3; i++) {
        if (pressed & btns[i].mask) {
          struct timespec ts;
          clock_gettime(CLOCK_MONOTONIC, &ts);
          uint64_t now_ns = (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;

          int bx = pos[0] - last_btn_x;
          int by = pos[1] - last_btn_y;
          uint32_t msg_type = btns[i].down;
          if (last_btn == (uint32_t)btns[i].mask &&
              (now_ns - last_btn_ns) <= 300000000ULL &&
              (bx*bx + by*by) <= 25) {
            msg_type = btns[i].dbl;
            last_btn_ns = 0;
          } else {
            last_btn = btns[i].mask;
            last_btn_ns = now_ns;
            last_btn_x = pos[0];
            last_btn_y = pos[1];
          }
          struct WI_Message ev = {
            .message = msg_type,
            .x = (uint16_t)pos[0],
            .y = (uint16_t)pos[1],
          };
          queue.data[queue.write++] = ev;
          sem_post(&event_sem);
        }
        if (released & btns[i].mask) {
          struct WI_Message ev = {
            .message = btns[i].up,
            .x = (uint16_t)pos[0],
            .y = (uint16_t)pos[1],
          };
          queue.data[queue.write++] = ev;
          sem_post(&event_sem);
        }
      }

      /* Mouse motion */
      {
        uint32_t move_msg;
        if (buttons & SCREEN_LEFT_MOUSE_BUTTON)
          move_msg = kEventLeftMouseDragged;
        else if (buttons & SCREEN_RIGHT_MOUSE_BUTTON)
          move_msg = kEventRightMouseDragged;
        else if (buttons & SCREEN_MIDDLE_MOUSE_BUTTON)
          move_msg = kEventOtherMouseDragged;
        else
          move_msg = kEventMouseMoved;

        struct WI_Message ev = {
          .message = move_msg,
          .x = (uint16_t)pos[0],
          .y = (uint16_t)pos[1],
          .dx = dx,
          .dy = dy,
        };
        queue.data[queue.write++] = ev;
        sem_post(&event_sem);
      }
      break;
    }

    case SCREEN_EVENT_KEYBOARD: {
      int flags = 0, sym = 0, mods = 0;
      screen_get_event_property_iv(screen_ev, SCREEN_PROPERTY_KEY_FLAGS,     &flags);
      screen_get_event_property_iv(screen_ev, SCREEN_PROPERTY_KEY_SYM,       &sym);
      screen_get_event_property_iv(screen_ev, SCREEN_PROPERTY_KEY_MODIFIERS, &mods);

      uint32_t wi_key = qnx_keysym_to_wi(sym);
      if (wi_key) {
        uint32_t wi_mods = qnx_key_modifiers(mods);
        bool_t pressed = (flags & KEY_DOWN) != 0;
        struct WI_Message ev = {
          .message = pressed ? kEventKeyDown : kEventKeyUp,
          .wParam  = wi_key | wi_mods,
        };
        /* Store printable character in lParam */
        if (pressed && sym >= 0x20 && sym <= 0x7e) {
          char ch = (char)sym;
          memcpy(&ev.lParam, &ch, 1);
        }
        queue.data[queue.write++] = ev;
        sem_post(&event_sem);
      }
      break;
    }

    case SCREEN_EVENT_CLOSE: {
      struct WI_Message ev = { .message = kEventWindowClosed };
      queue.data[queue.write++] = ev;
      sem_post(&event_sem);
      break;
    }

    default:
      break;
    }
  }
}

static void
init_event_sem(void)
{
  static int initialized = 0;
  if (!initialized) {
    sem_init(&event_sem, 0, 0);
    initialized = 1;
  }
}

void
WI_PostMessageW(void *hobj, uint32_t event, uint32_t wparam, void *lparam)
{
  init_event_sem();

  /* Coalesce duplicate resize/paint events */
  for (uint16_t r = queue.read; r != queue.write; r++) {
    if (queue.data[r].message != event)
      continue;
    switch (event) {
      case kEventWindowResized:
      case kEventWindowPaint:
        queue.data[r].target = hobj;
        queue.data[r].wParam = wparam;
        queue.data[r].lParam = lparam;
        return;
    }
  }

  queue.data[queue.write].target  = hobj;
  queue.data[queue.write].message = event;
  queue.data[queue.write].wParam  = wparam;
  queue.data[queue.write].lParam  = lparam;
  queue.write++;

  sem_post(&event_sem);
}

int
WI_PollEvent(struct WI_Message *e)
{
  qnx_process_screen_events();
  if (queue.read == queue.write)
    return 0;
  *e = queue.data[queue.read++];
  return 1;
}

void
WI_RemoveFromQueue(void *target)
{
  for (uint16_t r = queue.read; r != queue.write; r++)
    if (queue.data[r].target == target)
      memset(&queue.data[r], 0, sizeof(queue.data[r]));
}

int
WI_WaitEvent(longTime_t msec)
{
  init_event_sem();

  if (msec > 0) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec  += msec / 1000;
    ts.tv_nsec += (msec % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
      ts.tv_sec++;
      ts.tv_nsec -= 1000000000;
    }
    return sem_timedwait(&event_sem, &ts) == 0 ? 1 : 0;
  }

  sem_wait(&event_sem);
  return 1;
}

void
NotifyFileDropEvent(char const *filename, float x, float y)
{
  (void)filename;
  (void)x;
  (void)y;
}

/*
 * File dialogs (not supported on QNX)
 */

bool_t
WI_GetOpenFileName(WI_OpenFileName const *ofn)
{
  (void)ofn;
  return FALSE;
}

bool_t
WI_GetSaveFileName(WI_OpenFileName const *ofn)
{
  (void)ofn;
  return FALSE;
}

bool_t
WI_GetFolderName(WI_OpenFileName const *ofn)
{
  (void)ofn;
  return FALSE;
}

/*
 * Timing
 */

longTime_t
WI_GetMilliseconds(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (longTime_t)(ts.tv_sec * 1000UL + ts.tv_nsec / 1000000UL);
}

void
WI_Sleep(longTime_t msec)
{
  struct timespec ts;
  ts.tv_sec  = msec / 1000;
  ts.tv_nsec = (msec % 1000) * 1000000;
  nanosleep(&ts, NULL);
}

/*
 * Theme / system info
 */

bool_t
WI_IsDarkTheme(void)
{
  return FALSE;
}

/*
 * Window operations
 */

bool_t
WI_CreateSurface(uint32_t width, uint32_t height)
{
  (void)width;
  (void)height;
  return FALSE;
}

float
WI_GetScaling(void)
{
  return GetWindowScale(NULL);
}

bool_t
WI_SetSize(uint32_t width, uint32_t height, bool_t centered)
{
  (void)centered;
  /* Actual screen/EGL window resize is not yet implemented on QNX.
   * Notify the application so it can respond to the size change. */
  WI_PostMessageW(NULL, kEventWindowResized, MAKEDWORD(width, height), NULL);
  return TRUE;
}

uint32_t
WI_GetSize(struct WI_Size *pSize)
{
  SIZE2 size = { 0, 0 };
  GetWindowSize(NULL, &size);
  if (pSize) {
    pSize->width  = (uint32_t)size.width;
    pSize->height = (uint32_t)size.height;
  }
  return MAKEDWORD(size.width, size.height);
}

void
WI_MakeCurrentContext(void)
{
  BeginPaint(NULL);
}

void
WI_BeginPaint(void)
{
  BeginPaint(NULL);
}

void
WI_EndPaint(void)
{
  EndPaint(NULL, NULL);
}

void
WI_BindFramebuffer(void)
{
  /* EGL uses the default framebuffer (0) */
}

/*
 * Key names
 */

char const *
WI_KeynumToString(uint32_t keynum)
{
  static char tinystr[2];

  if (keynum == (uint32_t)-1)
    return "<KEY NOT FOUND>";

  keynum = keynum & 0xff;

  if (keynum > 32 && keynum < 127) {
    tinystr[0] = (char)keynum;
    tinystr[1] = 0;
    return tinystr;
  }

  return "<UNKNOWN KEYNUM>";
}
