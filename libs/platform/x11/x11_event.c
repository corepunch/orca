#include "../platform.h"
#include "x11_local.h"

#include <poll.h>

static struct
{
  EVENT queue[0x10000];
  WORD  write, read;
  float pointer_x, pointer_y;
} events = { 0 };

/* Map an X11 KeySym to a WI_KEY_* value */
static uint32_t
keysym_to_wi_key(KeySym sym)
{
  /* Printable ASCII: pass through directly */
  if (sym >= 0x20 && sym <= 0x7e) {
    return (uint32_t)sym;
  }

  switch (sym) {
  case XK_Tab:        return WI_KEY_TAB;
  case XK_Return:     return WI_KEY_ENTER;
  case XK_Escape:     return WI_KEY_ESCAPE;
  case XK_space:      return WI_KEY_SPACE;
  case XK_BackSpace:  return WI_KEY_BACKSPACE;

  case XK_Up:         return WI_KEY_UPARROW;
  case XK_Down:       return WI_KEY_DOWNARROW;
  case XK_Left:       return WI_KEY_LEFTARROW;
  case XK_Right:      return WI_KEY_RIGHTARROW;

  case XK_Alt_L:
  case XK_Alt_R:      return WI_KEY_ALT;
  case XK_Control_L:
  case XK_Control_R:  return WI_KEY_CTRL;
  case XK_Shift_L:
  case XK_Shift_R:    return WI_KEY_SHIFT;

  case XK_F1:         return WI_KEY_F1;
  case XK_F2:         return WI_KEY_F2;
  case XK_F3:         return WI_KEY_F3;
  case XK_F4:         return WI_KEY_F4;
  case XK_F5:         return WI_KEY_F5;
  case XK_F6:         return WI_KEY_F6;
  case XK_F7:         return WI_KEY_F7;
  case XK_F8:         return WI_KEY_F8;
  case XK_F9:         return WI_KEY_F9;
  case XK_F10:        return WI_KEY_F10;
  case XK_F11:        return WI_KEY_F11;
  case XK_F12:        return WI_KEY_F12;

  case XK_Insert:     return WI_KEY_INS;
  case XK_Delete:     return WI_KEY_DEL;
  case XK_Page_Down:  return WI_KEY_PGDN;
  case XK_Page_Up:    return WI_KEY_PGUP;
  case XK_Home:       return WI_KEY_HOME;
  case XK_End:        return WI_KEY_END;
  case XK_Pause:      return WI_KEY_PAUSE;

  case XK_KP_Home:    return WI_KEY_KP_HOME;
  case XK_KP_Up:      return WI_KEY_KP_UPARROW;
  case XK_KP_Page_Up: return WI_KEY_KP_PGUP;
  case XK_KP_Left:    return WI_KEY_KP_LEFTARROW;
  case XK_KP_Begin:   return WI_KEY_KP_5;
  case XK_KP_Right:   return WI_KEY_KP_RIGHTARROW;
  case XK_KP_End:     return WI_KEY_KP_END;
  case XK_KP_Down:    return WI_KEY_KP_DOWNARROW;
  case XK_KP_Page_Down: return WI_KEY_KP_PGDN;
  case XK_KP_Enter:   return WI_KEY_KP_ENTER;
  case XK_KP_Insert:  return WI_KEY_KP_INS;
  case XK_KP_Delete:  return WI_KEY_KP_DEL;
  case XK_KP_Divide:  return WI_KEY_KP_SLASH;
  case XK_KP_Subtract: return WI_KEY_KP_MINUS;
  case XK_KP_Add:     return WI_KEY_KP_PLUS;

  default:            return 0;
  }
}

/* Collect modifier flags from an X11 state mask */
static uint32_t
x11_modifiers(unsigned int state)
{
  uint32_t mods = 0;
  if (state & ShiftMask)   mods |= WI_MOD_SHIFT;
  if (state & ControlMask) mods |= WI_MOD_CTRL;
  if (state & Mod1Mask)    mods |= WI_MOD_ALT;  /* Alt */
  if (state & Mod4Mask)    mods |= WI_MOD_CMD;  /* Super/Meta */
  return mods;
}

/* Process all pending X11 events and push them onto the internal queue */
static void
x11_process_events(void)
{
  if (!x_display) {
    return;
  }

  /* State for double-click detection */
  static uint32_t last_button = 0;
  static Time     last_time   = 0;
  static int      last_x      = 0;
  static int      last_y      = 0;
  /* Bitmask of currently held mouse buttons (bit N = button N) */
  static uint32_t buttons_held = 0;

  XEvent xev;
  while (XPending(x_display)) {
    XNextEvent(x_display, &xev);

    switch (xev.type) {

    case KeyPress:
    case KeyRelease: {
      KeySym sym = XLookupKeysym(&xev.xkey, 0);
      uint32_t key = keysym_to_wi_key(sym);
      if (key) {
        uint32_t mods = x11_modifiers(xev.xkey.state);
        EVENT *e = &events.queue[events.write++];
        e->target  = NULL;
        e->message = (xev.type == KeyPress) ? kEventKeyDown : kEventKeyUp;
        e->wParam  = key | mods;
        e->lParam  = NULL;
        /* Store the UTF-8 character for key-down events */
        if (xev.type == KeyPress) {
          XLookupString(&xev.xkey, (char*)&e->lParam,
                        (int)sizeof(e->lParam) - 1, NULL, NULL);
        }
      }
      break;
    }

    case ButtonPress:
    case ButtonRelease: {
      bool_t pressed = (xev.type == ButtonPress);
      uint32_t msg = 0;
      switch (xev.xbutton.button) {
      case Button1:
        if (pressed) buttons_held |=  (1u << Button1);
        else         buttons_held &= ~(1u << Button1);
        if (pressed) {
          /* Double-click detection */
          int dx = xev.xbutton.x - last_x;
          int dy = xev.xbutton.y - last_y;
          int dist2 = dx*dx + dy*dy;
          if (last_button == Button1 &&
              (xev.xbutton.time - last_time) <= 300 &&
              dist2 <= 25) {
            msg = kEventLeftDoubleClick;
            last_time = 0; /* reset so triple-click doesn't double-fire */
          } else {
            msg = kEventLeftMouseDown;
            last_time = xev.xbutton.time;
            last_button = Button1;
            last_x = xev.xbutton.x;
            last_y = xev.xbutton.y;
          }
        } else {
          msg = kEventLeftMouseUp;
        }
        break;
      case Button2:
        if (pressed) buttons_held |=  (1u << Button2);
        else         buttons_held &= ~(1u << Button2);
        if (pressed) {
          int dx = xev.xbutton.x - last_x;
          int dy = xev.xbutton.y - last_y;
          int dist2 = dx*dx + dy*dy;
          if (last_button == Button2 &&
              (xev.xbutton.time - last_time) <= 300 &&
              dist2 <= 25) {
            msg = kEventOtherDoubleClick;
            last_time = 0;
          } else {
            msg = kEventOtherMouseDown;
            last_time = xev.xbutton.time;
            last_button = Button2;
            last_x = xev.xbutton.x;
            last_y = xev.xbutton.y;
          }
        } else {
          msg = kEventOtherMouseUp;
        }
        break;
      case Button3:
        if (pressed) buttons_held |=  (1u << Button3);
        else         buttons_held &= ~(1u << Button3);
        if (pressed) {
          int dx = xev.xbutton.x - last_x;
          int dy = xev.xbutton.y - last_y;
          int dist2 = dx*dx + dy*dy;
          if (last_button == Button3 &&
              (xev.xbutton.time - last_time) <= 300 &&
              dist2 <= 25) {
            msg = kEventRightDoubleClick;
            last_time = 0;
          } else {
            msg = kEventRightMouseDown;
            last_time = xev.xbutton.time;
            last_button = Button3;
            last_x = xev.xbutton.x;
            last_y = xev.xbutton.y;
          }
        } else {
          msg = kEventRightMouseUp;
        }
        break;
      case Button4:
        if (pressed) {
          events.queue[events.write++] = (EVENT){
            .x = (uint16_t)xev.xbutton.x,
            .y = (uint16_t)xev.xbutton.y,
            .dy = 1,
            .message = kEventScrollWheel,
          };
        }
        break;
      case Button5:
        if (pressed) {
          events.queue[events.write++] = (EVENT){
            .x = (uint16_t)xev.xbutton.x,
            .y = (uint16_t)xev.xbutton.y,
            .dy = -1,
            .message = kEventScrollWheel,
          };
        }
        break;
      }
      if (msg) {
        events.queue[events.write++] = (EVENT){
          .x = (uint16_t)xev.xbutton.x,
          .y = (uint16_t)xev.xbutton.y,
          .message = msg,
        };
      }
      break;
    }

    case MotionNotify: {
      int16_t dx = (int16_t)(xev.xmotion.x - (int)events.pointer_x);
      int16_t dy = (int16_t)(xev.xmotion.y - (int)events.pointer_y);
      events.pointer_x = (float)xev.xmotion.x;
      events.pointer_y = (float)xev.xmotion.y;

      uint32_t msg;
      if (buttons_held & (1u << Button1))
        msg = kEventLeftMouseDragged;
      else if (buttons_held & (1u << Button3))
        msg = kEventRightMouseDragged;
      else if (buttons_held & (1u << Button2))
        msg = kEventOtherMouseDragged;
      else
        msg = kEventMouseMoved;

      events.queue[events.write++] = (EVENT){
        .x = (uint16_t)xev.xmotion.x,
        .y = (uint16_t)xev.xmotion.y,
        .dx = dx,
        .dy = dy,
        .message = msg,
      };
      break;
    }

    case ConfigureNotify: {
      extern struct _WND window;
      if (xev.xconfigure.width != window.width ||
          xev.xconfigure.height != window.height) {
        window.width  = xev.xconfigure.width;
        window.height = xev.xconfigure.height;
        events.queue[events.write++] = (EVENT){
          .wParam  = MAKEDWORD(xev.xconfigure.width, xev.xconfigure.height),
          .message = kEventWindowResized,
        };
      }
      break;
    }

    case Expose:
      if (xev.xexpose.count == 0) {
        events.queue[events.write++] = (EVENT){
          .message = kEventWindowPaint,
        };
      }
      break;

    case FocusIn:
      events.queue[events.write++] = (EVENT){
        .message = kEventSetFocus,
      };
      break;

    case FocusOut:
      events.queue[events.write++] = (EVENT){
        .message = kEventKillFocus,
      };
      break;

    case ClientMessage:
      if ((Atom)xev.xclient.data.l[0] == wm_delete_window) {
        events.queue[events.write++] = (EVENT){
          .message = kEventWindowClosed,
        };
      }
      break;

    default:
      break;
    }
  }
}

int
WI_WaitEvent(TIME timeout_ms)
{
  if (!x_display) {
    return 0;
  }

  if (XPending(x_display)) {
    x11_process_events();
    return 1;
  }

  if (timeout_ms > 0) {
    struct pollfd fds[1];
    fds[0].fd     = ConnectionNumber(x_display);
    fds[0].events = POLLIN;
    int ret = poll(fds, 1, (int)timeout_ms);
    if (ret > 0) {
      x11_process_events();
      return 1;
    }
    return 0;
  }

  /* Block indefinitely */
  XEvent xev;
  XPeekEvent(x_display, &xev);
  x11_process_events();
  return 1;
}

int
WI_PollEvent(PEVENT pEvent)
{
  x11_process_events();

  if (events.read != events.write) {
    *pEvent = events.queue[events.read++];
    return 1;
  }
  return 0;
}

void
NotifyWindowEvent(void* window, uint32_t eventType, uint32_t wparam)
{
  events.queue[events.write++] = (EVENT){
    .wParam   = wparam,
    .message  = eventType,
    .target   = window,
  };
}

void
WI_PostMessageW(void* hobj, uint32_t event, uint32_t wparam, void* lparam)
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
WI_RemoveFromQueue(void* hobj)
{
  WORD read_idx  = events.read;
  WORD write_idx = events.write;
  WORD new_write = events.read;

  while (read_idx != write_idx) {
    if (events.queue[read_idx].target != hobj) {
      events.queue[new_write++] = events.queue[read_idx];
    }
    read_idx++;
  }
  events.write = new_write;
}

void
NotifyFileDropEvent(char const* filename, float x, float y)
{
  (void)filename;
  (void)x;
  (void)y;
}
