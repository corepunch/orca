#include "../platform.h"
#include "webgl_local.h"

static struct
{
  EVENT queue[0x10000];
  WORD write, read;
  float pointer_x, pointer_y;
  uint32_t buttons; /* bitmask of currently pressed mouse buttons */
} events = { 0 };

static uint32_t
map_keycode(unsigned long keyCode)
{
  switch (keyCode) {
  case 8:   return WI_KEY_BACKSPACE;
  case 9:   return WI_KEY_TAB;
  case 13:  return WI_KEY_ENTER;
  case 16:  return WI_KEY_SHIFT;
  case 17:  return WI_KEY_CTRL;
  case 18:  return WI_KEY_ALT;
  case 27:  return WI_KEY_ESCAPE;
  case 32:  return WI_KEY_SPACE;
  case 33:  return WI_KEY_PGUP;
  case 34:  return WI_KEY_PGDN;
  case 35:  return WI_KEY_END;
  case 36:  return WI_KEY_HOME;
  case 37:  return WI_KEY_LEFTARROW;
  case 38:  return WI_KEY_UPARROW;
  case 39:  return WI_KEY_RIGHTARROW;
  case 40:  return WI_KEY_DOWNARROW;
  case 45:  return WI_KEY_INS;
  case 46:  return WI_KEY_DEL;
  case 112: return WI_KEY_F1;
  case 113: return WI_KEY_F2;
  case 114: return WI_KEY_F3;
  case 115: return WI_KEY_F4;
  case 116: return WI_KEY_F5;
  case 117: return WI_KEY_F6;
  case 118: return WI_KEY_F7;
  case 119: return WI_KEY_F8;
  case 120: return WI_KEY_F9;
  case 121: return WI_KEY_F10;
  case 122: return WI_KEY_F11;
  case 123: return WI_KEY_F12;
  default:
    if (keyCode >= 65 && keyCode <= 90) {
      return (uint32_t)tolower((int)keyCode); // A-Z -> a-z
    }
    return (uint32_t)keyCode;
  }
}

static EM_BOOL
on_mousemove(int eventType, const EmscriptenMouseEvent *e, void *userData)
{
  (void)eventType;
  (void)userData;
  int16_t dx = (int16_t)(e->targetX - (int)events.pointer_x);
  int16_t dy = (int16_t)(e->targetY - (int)events.pointer_y);
  events.pointer_x = (float)e->targetX;
  events.pointer_y = (float)e->targetY;

  uint32_t msg;
  if (events.buttons & (1u << 0))
    msg = kEventLeftMouseDragged;
  else if (events.buttons & (1u << 2))
    msg = kEventRightMouseDragged;
  else if (events.buttons & (1u << 1))
    msg = kEventOtherMouseDragged;
  else
    msg = kEventMouseMoved;

  events.queue[events.write++] = (EVENT){
    .x = (uint16_t)e->targetX,
    .y = (uint16_t)e->targetY,
    .dx = dx,
    .dy = dy,
    .message = msg,
  };
  return EM_TRUE;
}

static EM_BOOL
on_mousedown(int eventType, const EmscriptenMouseEvent *e, void *userData)
{
  (void)eventType;
  (void)userData;
  events.buttons |= (1u << e->button);
  uint32_t msg;
  switch (e->button) {
  case 0:  msg = kEventLeftMouseDown;  break;
  case 2:  msg = kEventRightMouseDown; break;
  default: msg = kEventOtherMouseDown; break;
  }
  events.queue[events.write++] = (EVENT){
    .x = (uint16_t)e->targetX,
    .y = (uint16_t)e->targetY,
    .message = msg,
  };
  return EM_TRUE;
}

static EM_BOOL
on_mouseup(int eventType, const EmscriptenMouseEvent *e, void *userData)
{
  (void)eventType;
  (void)userData;
  events.buttons &= ~(1u << e->button);
  uint32_t msg;
  switch (e->button) {
  case 0:  msg = kEventLeftMouseUp;  break;
  case 2:  msg = kEventRightMouseUp; break;
  default: msg = kEventOtherMouseUp; break;
  }
  events.queue[events.write++] = (EVENT){
    .x = (uint16_t)e->targetX,
    .y = (uint16_t)e->targetY,
    .message = msg,
  };
  return EM_TRUE;
}

static EM_BOOL
on_touchstart(int eventType, const EmscriptenTouchEvent *e, void *userData)
{
  (void)eventType;
  (void)userData;
  if (e->numTouches < 1)
    return EM_TRUE;
  const EmscriptenTouchPoint *t = &e->touches[0];
  events.pointer_x = (float)t->targetX;
  events.pointer_y = (float)t->targetY;
  events.buttons |= (1u << 0);
  events.queue[events.write++] = (EVENT){
    .x = (uint16_t)t->targetX,
    .y = (uint16_t)t->targetY,
    .message = kEventLeftMouseDown,
  };
  return EM_TRUE;
}

static EM_BOOL
on_touchmove(int eventType, const EmscriptenTouchEvent *e, void *userData)
{
  (void)eventType;
  (void)userData;
  if (e->numTouches < 1)
    return EM_TRUE;
  const EmscriptenTouchPoint *t = &e->touches[0];
  int16_t dx = (int16_t)(t->targetX - (int)events.pointer_x);
  int16_t dy = (int16_t)(t->targetY - (int)events.pointer_y);
  events.pointer_x = (float)t->targetX;
  events.pointer_y = (float)t->targetY;
  events.queue[events.write++] = (EVENT){
    .x = (uint16_t)t->targetX,
    .y = (uint16_t)t->targetY,
    .dx = dx,
    .dy = dy,
    .message = kEventLeftMouseDragged,
  };
  return EM_TRUE;
}

static EM_BOOL
on_touchend(int eventType, const EmscriptenTouchEvent *e, void *userData)
{
  (void)eventType;
  (void)userData;
  events.buttons &= ~(1u << 0);
  /* Use last known pointer position when no active touches remain */
  uint16_t x = (uint16_t)events.pointer_x;
  uint16_t y = (uint16_t)events.pointer_y;
  if (e->numTouches > 0) {
    x = (uint16_t)e->touches[0].targetX;
    y = (uint16_t)e->touches[0].targetY;
  }
  events.queue[events.write++] = (EVENT){
    .x = x,
    .y = y,
    .message = kEventLeftMouseUp,
  };
  return EM_TRUE;
}

static EM_BOOL
on_wheel(int eventType, const EmscriptenWheelEvent *e, void *userData)
{
  (void)eventType;
  (void)userData;
  int16_t dx = (int16_t)e->deltaX;
  int16_t dy = (int16_t)e->deltaY;
  events.queue[events.write++] = (EVENT){
    .x = (uint16_t)events.pointer_x,
    .y = (uint16_t)events.pointer_y,
    .lParam = (void *)(intptr_t)((dy << 16) | (uint16_t)dx),
    .message = kEventScrollWheel,
  };
  return EM_TRUE;
}

static EM_BOOL
on_keydown(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{
  (void)eventType;
  (void)userData;
  uint32_t keycode = map_keycode(e->keyCode);
  events.queue[events.write++] = (EVENT){
    .message = kEventKeyDown,
    .wParam = keycode,
  };
  return EM_FALSE; // allow default browser behaviour
}

static EM_BOOL
on_keyup(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{
  (void)eventType;
  (void)userData;
  uint32_t keycode = map_keycode(e->keyCode);
  events.queue[events.write++] = (EVENT){
    .message = kEventKeyUp,
    .wParam = keycode,
  };
  return EM_FALSE;
}

static EM_BOOL
on_resize(int eventType, const EmscriptenUiEvent *e, void *userData)
{
  (void)eventType;
  (void)userData;
  (void)e;

  /* Get the canvas element's actual CSS display size */
  double css_width, css_height;
  emscripten_get_element_css_size("#canvas", &css_width, &css_height);

  /* Resize the canvas drawing buffer */
  emscripten_set_canvas_element_size("#canvas", 
    (int)(css_width * WI_GetScaling() + 0.5), 
    (int)(css_height * WI_GetScaling() + 0.5));

  /* Update global state */
  g_canvas_width  = (int)css_width;
  g_canvas_height = (int)css_height;

  /* Notify the engine with the canvas size (not window size) */

  events.queue[events.write++] = (EVENT){
    .message = kEventWindowResized,
    .wParam = MAKEDWORD(g_canvas_width, g_canvas_height),
  };
  return EM_TRUE;
}

void
webgl_register_callbacks(void)
{
  emscripten_set_mousemove_callback("#canvas", NULL, EM_FALSE, on_mousemove);
  emscripten_set_mousedown_callback("#canvas", NULL, EM_FALSE, on_mousedown);
  emscripten_set_mouseup_callback("#canvas", NULL, EM_FALSE, on_mouseup);
  // emscripten_set_dblclick_callback("#canvas", NULL, EM_FALSE, on_dblclick);
  emscripten_set_wheel_callback("#canvas", NULL, EM_FALSE, on_wheel);
  emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, EM_FALSE, on_keydown);
  emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, EM_FALSE, on_keyup);
  emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, EM_FALSE, on_resize);
  emscripten_set_touchstart_callback("#canvas", NULL, EM_FALSE, on_touchstart);
  emscripten_set_touchmove_callback("#canvas", NULL, EM_FALSE, on_touchmove);
  emscripten_set_touchend_callback("#canvas", NULL, EM_FALSE, on_touchend);
  emscripten_set_touchcancel_callback("#canvas", NULL, EM_FALSE, on_touchend);
}

int
WI_WaitEvent(TIME time)
{
  (void)time;
  // In the browser all events arrive via registered callbacks; nothing to block on.
  return 0;
}

int
WI_PollEvent(PEVENT pEvent)
{
  if (events.read != events.write) {
    *pEvent = events.queue[events.read++];
    return 1;
  }
  return 0;
}

void
WI_PostMessageW(void *hobj, uint32_t event, uint32_t wparam, void *lparam)
{
  if ((WORD)(events.write - events.read) == (WORD)(sizeof(events.queue) / sizeof(events.queue[0]) - 1)) {
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
WI_RemoveFromQueue(void *hobj)
{
  WORD read_idx = events.read;
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
NotifyFileDropEvent(char const *filename, float x, float y)
{
  (void)filename;
  (void)x;
  (void)y;
}
