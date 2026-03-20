#include "../platform.h"

#include "wayland_local.h"
#include <unistd.h>
#include <poll.h>
#include <linux/input-event-codes.h>

static struct
{
  EVENT queue[0x10000];
  WORD write, read;
  VECTOR2D pointer;
  uint32_t buttons;        /* bitmask of currently pressed buttons */
  uint32_t mods;           /* current WI_MOD_* modifier flags */
  uint32_t last_btn;       /* button of last click (for double-click) */
  uint32_t last_btn_time;  /* timestamp of last click (ms) */
} events = { 0 };

extern struct xkb_state* xkb_state;

static struct wl_pointer* pointer = NULL;
static struct wl_keyboard* keyboard = NULL;

static void
pointer_enter(void* data,
              struct wl_pointer* pointer,
              uint32_t serial,
              struct wl_surface* surface,
              wl_fixed_t sx,
              wl_fixed_t sy)
{
}

static void
pointer_leave(void* data,
              struct wl_pointer* pointer,
              uint32_t serial,
              struct wl_surface* surface)
{
}

static void
pointer_motion(void* data,
               struct wl_pointer* pointer,
               uint32_t time,
               wl_fixed_t sx,
               wl_fixed_t sy)
{
  float new_x = wl_fixed_to_double(sx);
  float new_y = wl_fixed_to_double(sy);
  int16_t dx = (int16_t)(new_x - events.pointer.x);
  int16_t dy = (int16_t)(new_y - events.pointer.y);
  events.pointer.x = new_x;
  events.pointer.y = new_y;

  uint32_t msg;
  if (events.buttons & 1u)                                   /* BTN_LEFT   */
    msg = kEventLeftMouseDragged;
  else if (events.buttons & (1u << (BTN_RIGHT - BTN_LEFT)))  /* BTN_RIGHT  */
    msg = kEventRightMouseDragged;
  else if (events.buttons & (1u << (BTN_MIDDLE - BTN_LEFT))) /* BTN_MIDDLE */
    msg = kEventOtherMouseDragged;
  else
    msg = kEventMouseMoved;

  events.queue[events.write++] = (EVENT){
    .x = (uint16_t)new_x,
    .y = (uint16_t)new_y,
    .dx = dx,
    .dy = dy,
    .message = msg,
  };
}

static void
pointer_button(void* data,
               struct wl_pointer* pointer,
               uint32_t serial,
               uint32_t time,
               uint32_t button,
               uint32_t state)
{
  bool_t pressed = state == WL_POINTER_BUTTON_STATE_PRESSED;

  /* Track button bitmask (offset from BTN_LEFT for compact storage) */
  uint32_t bit = 1u << (button - BTN_LEFT);
  if (pressed)
    events.buttons |= bit;
  else
    events.buttons &= ~bit;

  /* Determine event type, checking for double-click */
  uint32_t down_msg, up_msg, dbl_msg;
  switch (button) {
  case BTN_LEFT:
    down_msg = kEventLeftMouseDown;
    up_msg   = kEventLeftMouseUp;
    dbl_msg  = kEventLeftDoubleClick;
    break;
  case BTN_RIGHT:
    down_msg = kEventRightMouseDown;
    up_msg   = kEventRightMouseUp;
    dbl_msg  = kEventRightDoubleClick;
    break;
  case BTN_MIDDLE:
    down_msg = kEventOtherMouseDown;
    up_msg   = kEventOtherMouseUp;
    dbl_msg  = kEventOtherDoubleClick;
    break;
  default:
    return;
  }

  if (pressed) {
    uint32_t msg = down_msg;
    if (events.last_btn == button && (time - events.last_btn_time) <= 300) {
      msg = dbl_msg;
      events.last_btn_time = 0; /* reset so triple-click doesn't double-fire */
    } else {
      events.last_btn_time = time;
      events.last_btn = button;
    }
    events.queue[events.write++] = (EVENT){
      .x = (uint16_t)events.pointer.x,
      .y = (uint16_t)events.pointer.y,
      .message = msg,
    };
  } else {
    events.queue[events.write++] = (EVENT){
      .x = (uint16_t)events.pointer.x,
      .y = (uint16_t)events.pointer.y,
      .message = up_msg,
    };
  }
}

static void
pointer_axis(void* data,
             struct wl_pointer* pointer,
             uint32_t time,
             uint32_t axis,
             wl_fixed_t value)
{
  int16_t dx = axis == WL_POINTER_AXIS_VERTICAL_SCROLL ? 0 : wl_fixed_to_double(value);
  int16_t dy = axis == WL_POINTER_AXIS_VERTICAL_SCROLL ? wl_fixed_to_double(value) : 0;
  events.queue[events.write++] = (EVENT) {
    .x = events.pointer.x,
    .y = events.pointer.y,
    .lParam = (void*)(intptr_t)((dy)<<16|dx),
    .message = kEventScrollWheel
  };
}

// Attach pointer listener
static struct wl_pointer_listener pointer_listener = {
  .enter = pointer_enter,
  .leave = pointer_leave,
  .motion = pointer_motion,
  .button = pointer_button,
  .axis = pointer_axis,
};

static void
keyboard_keymap(void* data,
                struct wl_keyboard* keyboard,
                uint32_t format,
                int fd,
                uint32_t size)
{
  close(fd);
}

static void
keyboard_enter(void* data,
               struct wl_keyboard* keyboard,
               uint32_t serial,
               struct wl_surface* surface,
               struct wl_array* keys)
{
  events.queue[events.write++] = (EVENT){ .message = kEventSetFocus };
}

static void
keyboard_leave(void* data,
               struct wl_keyboard* keyboard,
               uint32_t serial,
               struct wl_surface* surface)
{
  events.queue[events.write++] = (EVENT){ .message = kEventKillFocus };
}

static uint32_t
keysym_to_wi_key(xkb_keysym_t sym)
{
  /* Printable ASCII: normalise letters to lowercase */
  if (sym >= 0x20 && sym <= 0x7e)
    return (uint32_t)tolower((int)sym);

  switch (sym) {
  case XKB_KEY_Tab:       return WI_KEY_TAB;
  case XKB_KEY_Return:    return WI_KEY_ENTER;
  case XKB_KEY_Escape:    return WI_KEY_ESCAPE;
  case XKB_KEY_space:     return WI_KEY_SPACE;
  case XKB_KEY_BackSpace: return WI_KEY_BACKSPACE;

  case XKB_KEY_Up:        return WI_KEY_UPARROW;
  case XKB_KEY_Down:      return WI_KEY_DOWNARROW;
  case XKB_KEY_Left:      return WI_KEY_LEFTARROW;
  case XKB_KEY_Right:     return WI_KEY_RIGHTARROW;

  case XKB_KEY_Alt_L:
  case XKB_KEY_Alt_R:     return WI_KEY_ALT;
  case XKB_KEY_Control_L:
  case XKB_KEY_Control_R: return WI_KEY_CTRL;
  case XKB_KEY_Shift_L:
  case XKB_KEY_Shift_R:   return WI_KEY_SHIFT;
  case XKB_KEY_Super_L:
  case XKB_KEY_Super_R:   return WI_KEY_ALT; /* map Super to Alt for uniformity */

  case XKB_KEY_F1:        return WI_KEY_F1;
  case XKB_KEY_F2:        return WI_KEY_F2;
  case XKB_KEY_F3:        return WI_KEY_F3;
  case XKB_KEY_F4:        return WI_KEY_F4;
  case XKB_KEY_F5:        return WI_KEY_F5;
  case XKB_KEY_F6:        return WI_KEY_F6;
  case XKB_KEY_F7:        return WI_KEY_F7;
  case XKB_KEY_F8:        return WI_KEY_F8;
  case XKB_KEY_F9:        return WI_KEY_F9;
  case XKB_KEY_F10:       return WI_KEY_F10;
  case XKB_KEY_F11:       return WI_KEY_F11;
  case XKB_KEY_F12:       return WI_KEY_F12;

  case XKB_KEY_Insert:    return WI_KEY_INS;
  case XKB_KEY_Delete:    return WI_KEY_DEL;
  case XKB_KEY_Page_Down: return WI_KEY_PGDN;
  case XKB_KEY_Page_Up:   return WI_KEY_PGUP;
  case XKB_KEY_Home:      return WI_KEY_HOME;
  case XKB_KEY_End:       return WI_KEY_END;
  case XKB_KEY_Pause:     return WI_KEY_PAUSE;

  case XKB_KEY_KP_Home:     return WI_KEY_KP_HOME;
  case XKB_KEY_KP_Up:       return WI_KEY_KP_UPARROW;
  case XKB_KEY_KP_Page_Up:  return WI_KEY_KP_PGUP;
  case XKB_KEY_KP_Left:     return WI_KEY_KP_LEFTARROW;
  case XKB_KEY_KP_Begin:    return WI_KEY_KP_5;
  case XKB_KEY_KP_Right:    return WI_KEY_KP_RIGHTARROW;
  case XKB_KEY_KP_End:      return WI_KEY_KP_END;
  case XKB_KEY_KP_Down:     return WI_KEY_KP_DOWNARROW;
  case XKB_KEY_KP_Page_Down: return WI_KEY_KP_PGDN;
  case XKB_KEY_KP_Enter:    return WI_KEY_KP_ENTER;
  case XKB_KEY_KP_Insert:   return WI_KEY_KP_INS;
  case XKB_KEY_KP_Delete:   return WI_KEY_KP_DEL;
  case XKB_KEY_KP_Divide:   return WI_KEY_KP_SLASH;
  case XKB_KEY_KP_Subtract: return WI_KEY_KP_MINUS;
  case XKB_KEY_KP_Add:      return WI_KEY_KP_PLUS;

  default: return 0;
  }
}

static uint32_t
wayland_modifiers(void)
{
  uint32_t mods = 0;
  if (xkb_state_mod_name_is_active(xkb_state, XKB_MOD_NAME_SHIFT,
                                   XKB_STATE_MODS_EFFECTIVE) > 0)
    mods |= WI_MOD_SHIFT;
  if (xkb_state_mod_name_is_active(xkb_state, XKB_MOD_NAME_CTRL,
                                   XKB_STATE_MODS_EFFECTIVE) > 0)
    mods |= WI_MOD_CTRL;
  if (xkb_state_mod_name_is_active(xkb_state, XKB_MOD_NAME_ALT,
                                   XKB_STATE_MODS_EFFECTIVE) > 0)
    mods |= WI_MOD_ALT;
  if (xkb_state_mod_name_is_active(xkb_state, XKB_MOD_NAME_LOGO,
                                   XKB_STATE_MODS_EFFECTIVE) > 0)
    mods |= WI_MOD_CMD;
  return mods;
}

static void
keyboard_key(void* data,
             struct wl_keyboard* keyboard,
             uint32_t serial,
             uint32_t time,
             uint32_t key,
             uint32_t state)
{
  xkb_keycode_t keycode = key + 8;
  xkb_keysym_t keysym = xkb_state_key_get_one_sym(xkb_state, keycode);
  uint32_t wi_key = keysym_to_wi_key(keysym);

  if (!wi_key)
    return;

  uint32_t mods = wayland_modifiers();
  bool_t pressed = (state == WL_KEYBOARD_KEY_STATE_PRESSED);
  uint32_t msg = pressed ? kEventKeyDown : kEventKeyUp;

  PEVENT e = &events.queue[events.write++];
  *e = (EVENT){
    .message = msg,
    .wParam  = wi_key | mods,
  };

  /* For key-down, also store the UTF-8 character in lParam */
  if (pressed) {
    e->lParam = NULL;
    xkb_state_key_get_utf8(xkb_state, keycode, (char*)&e->lParam,
                           sizeof(e->lParam));
  }
}

static void
keyboard_modifiers(void* data,
                   struct wl_keyboard* keyboard,
                   uint32_t serial,
                   uint32_t mods_depressed,
                   uint32_t mods_latched,
                   uint32_t mods_locked,
                   uint32_t group)
{
  xkb_state_update_mask(xkb_state,
                        mods_depressed, mods_latched, mods_locked,
                        0, 0, group);
}

// Attach keyboard listener
static struct wl_keyboard_listener keyboard_listener = {
  .keymap = keyboard_keymap,
  .enter = keyboard_enter,
  .leave = keyboard_leave,
  .key = keyboard_key,
  .modifiers = keyboard_modifiers,
};

static void
seat_capabilities(void* data, struct wl_seat* seat, uint32_t capabilities)
{
  if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
    pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(pointer, &pointer_listener, &events);
  }
  if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
    keyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(keyboard, &keyboard_listener, &events);
  }
}

static struct wl_seat_listener seat_listener = {
  .capabilities = seat_capabilities,
};

struct wl_seat_listener*
get_seat_listener(void)
{
  return &seat_listener;
}

static void
xdg_surface_configure(void* data,
                      struct xdg_surface* xdg_surface,
                      uint32_t serial)
{
  xdg_surface_ack_configure(xdg_surface, serial);
  events.queue[events.write++] = (EVENT){ .message = kEventWindowPaint };
}

static struct xdg_surface_listener xdg_surface_listener = {
  xdg_surface_configure,
};

struct xdg_surface_listener*
get_xdg_surface_listener(void)
{
  return &xdg_surface_listener;
}

int
WI_WaitEvent(TIME time)
{
  extern struct wl_display* display;

  if (!display)
    return 0;

  if (time > 0) {
    // Use poll or epoll to wait with timeout
    struct pollfd fds[1];
    fds[0].fd = wl_display_get_fd(display);
    fds[0].events = POLLIN;
    
    int ret = poll(fds, 1, time);
    if (ret > 0) {
      wl_display_dispatch(display);
      return 1;
    }
    return 0;
  }
  
  // No timeout, just dispatch pending events
  wl_display_dispatch_pending(display);
  return 0;
}

int
WI_PollEvent(PEVENT pEvent)
{
  if (events.read != events.write) {
    *pEvent = events.queue[events.read++];
    return 1;
  } else {
    return 0;
  }
}

void NotifyWindowEvent(void *window, uint32_t eventType, uint32_t wparam) {
  events.queue[events.write++] = (EVENT){ 
    .wParam = wparam,
    .message = kEventWindowPaint,
    .target = window,
  };
}

void
WI_PostMessageW(void* hobj, uint32_t event, uint32_t wparam, void* lparam)
{
  if (events.write - events.read >= sizeof(events.queue) / sizeof(events.queue[0])) {
    // Queue is full, ignore the message
    return;
  }
  events.queue[events.write++] = (EVENT){
    .target = hobj,
    .message = event,
    .wParam = wparam,
    .lParam = lparam,
  };
}

void
WI_RemoveFromQueue(void* hobj)
{
  WORD read_idx = events.read;
  WORD write_idx = events.write;
  WORD new_write = events.read;
  
  // Scan through the queue and keep events not matching hobj
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
  // File drop not fully implemented yet
  // Would need to allocate and copy filename string
  (void)filename;
  (void)x;
  (void)y;
}
