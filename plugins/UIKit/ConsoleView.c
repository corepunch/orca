#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <plugins/UIKit/UIKit.h>

#define SCROLL_SENSIVITY 1.2f
#define BUFFER_WIDTH 256
#define BUFFER_HEIGHT 256
#define TAB_SIZE 4
#define MEMSIZE(T) (T->BufferWidth*T->BufferHeight*4)

#define kMsgPaint 0x36b37bd3

static uint32_t
PointToData(struct ConsoleView const* t, int mx, int my, uint32_t *cursor)
{
  struct vec2 scrolled = VEC2_Sub(&(struct vec2){mx, my}, &t->_scroll);
  uint32_t x = scrolled.x / CONSOLE_CHAR_WIDTH;
  uint32_t y = scrolled.y / CONSOLE_CHAR_HEIGHT;
  if (x < t->BufferWidth && y < t->BufferHeight) {
    if (cursor) *cursor = y * t->BufferWidth + x;
    return t->_buffer[x + y * t->BufferWidth];
  }
  return -1;
}

struct tstate {
  uint8_t glyph;
  uint8_t foreground:4;
  uint8_t background:4;
  uint16_t item;
};

static lpcString_t
_cmd(struct ConsoleView *t, lpcString_t str, struct tstate *state)
{
  uint32_t v[3];
  uint8_t cmd;
  if (sscanf(str, "[%d;%d%c", v, v+1, &cmd)==3) {
    switch (cmd) {
      case 'f':
        t->Cursor = v[0] + v[1] * t->BufferWidth;
        break;
      case 'm':
        state->foreground = v[0];
        state->background = v[1];
        break;
      case 'c':
        assert(0);
        break;
    }
    return strchr(str, cmd);
  }
  if (sscanf(str, "[%d%c", v, &cmd)==2) {
    switch (cmd) {
      case 'c':
        state->item = v[0];
        break;
      case 'm':
        state->foreground = v[0];
        state->background = 0;
        break;
      case 'G':
        t->Cursor = (t->Cursor / t->BufferWidth) * t->BufferWidth;
        break;
    }
    return strchr(str, cmd);
  }
  
  return str;
}

static void
_println(struct ConsoleView *t, int32_t index, lpcString_t text, size_t slen)
{
  struct tstate state = { .foreground = 7, .item = (uint16_t)index };
  int c = t->Cursor;
  for (lpcString_t s = text; s - text < (ptrdiff_t)slen; s++) {
    if (*s == '\x21' && *(s+1) == '[') {
      s = _cmd(t, ++s, &state);
    } else if (*s == '\n') {
      t->Cursor = ((t->Cursor / t->BufferWidth) + 1) * t->BufferWidth;
    } else if (*s == '\t') {
      int start = (t->Cursor / t->BufferWidth) * t->BufferWidth;
      t->Cursor = start + (((t->Cursor - start) / TAB_SIZE) + 1) * TAB_SIZE;
    } else {
      state.glyph = (uint8_t)*s;
      t->_buffer[t->Cursor++] = *(uint32_t*)&state;
    }
  }
  while (t->Cursor % t->BufferWidth) {
    state.glyph = 0;
    t->_buffer[t->Cursor++] = *(uint32_t*)&state;
  }
  t->ContentHeight += MAX(1, (t->Cursor - c) / t->BufferWidth);
}

static uint64_t
pack_index_glyph(uint32_t index, uint8_t glyph)
{
  return ((uint64_t)index << 32) | glyph;
}

static uint64_t
pack_position(int32_t x, int32_t y)
{
  return ((uint64_t)(((uint32_t)x) ^ 0x80000000u) << 32) |
         ((((uint32_t)y) ^ 0x80000000u));
}

static uint64_t
query_unpack(struct Object *obj, struct ConsoleView const* t, float x, float y)
{
#ifdef MOUSE_EVENTS_USE_LOCAL_SPACE
  struct vec3 out = (struct vec3){x,y,0};
#else
  struct mat4 inv = MAT4_Inverse(&GetNode2D(obj)->Matrix);
  struct vec3 out = MAT4_MultiplyVector3D(&inv, &(struct vec3){x,y,0});
#endif
  uint32_t data = PointToData(t, out.x, out.y, NULL);

  return pack_index_glyph(data >> 16, data & 0xff);
}

static bool_t
query_index_position(struct Object *o,
                     struct ConsoleView const* t,
                     int32_t i,
                     int32_t offx,
                     int32_t offy,
                     bool_t global,
                     int32_t *outx,
                     int32_t *outy)
{
  if (i == 0)
    return FALSE;
  FOR_LOOP(j, t->BufferWidth*t->BufferHeight) {
    if (t->_buffer[j]>>16 == i) {
      if (global) {
        struct vec3 p = {
          .x = ((j%t->BufferWidth)+offx)*CONSOLE_CHAR_WIDTH,
          .y = ((j/t->BufferWidth)+offy)*CONSOLE_CHAR_HEIGHT,
        };
        struct vec3 out = MAT4_MultiplyVector3D(&GetNode2D(o)->Matrix, &p);
        *outx = (int32_t)(out.x + t->_scroll.x);
        *outy = (int32_t)(out.y + t->_scroll.y);
      } else {
        *outx = (int32_t)(((j%t->BufferWidth)+offx)*CONSOLE_CHAR_WIDTH + t->_scroll.x);
        *outy = (int32_t)(((j/t->BufferWidth)+offy)*CONSOLE_CHAR_HEIGHT + t->_scroll.y);
      }
      return TRUE;
    }
  }
  return FALSE;
}

HANDLER(ConsoleView, ConsoleView, Unpack) {
  if (!pConsoleView->_buffer) return FALSE;
  return (LRESULT)(intptr_t)query_unpack(hObject, pConsoleView, pUnpack->X, pUnpack->Y);
}

HANDLER(ConsoleView, ConsoleView, GetIndexPosition) {
  int32_t x, y;
  if (!pConsoleView->_buffer) return FALSE;
  if (!query_index_position(hObject,
                            pConsoleView,
                            pGetIndexPosition->Index,
                            pGetIndexPosition->OffsetX,
                            pGetIndexPosition->OffsetY,
                            pGetIndexPosition->Global,
                            &x,
                            &y)) {
    return FALSE;
  }
  return (LRESULT)(intptr_t)pack_position(x, y);
}

HANDLER(ConsoleView, ConsoleView, Println) {
  if (!pConsoleView->_buffer) return FALSE;
  lpcString_t text = pPrintln ? pPrintln->Text : NULL;
  int32_t index = pPrintln ? pPrintln->Index : 0;
  if (text) {
    _println(pConsoleView, index, text, strlen(text));
  } else {
    _println(pConsoleView, index, "", 0);
  }
  return FALSE;
}

HANDLER(ConsoleView, ConsoleView, Erase) {
  if (!pConsoleView->_buffer) return FALSE;
  memset(pConsoleView->_buffer, 0, MEMSIZE(pConsoleView));
  pConsoleView->Cursor = 0;
  pConsoleView->ContentHeight = 0;
  return FALSE;
}

HANDLER(ConsoleView, ConsoleView, Invalidate) {
  axPostMessageW(hObject, kMsgPaint, 0, NULL);
  return FALSE;
}

HANDLER(ConsoleView, Object, Create) {
  pConsoleView->_buffer = ZeroAlloc(MEMSIZE(pConsoleView));
  axPostMessageW(hObject, kMsgPaint, 0, NULL);
  return FALSE;
}

HANDLER(ConsoleView, Node2D, DrawBrush) {
  bool_t bFocused = OBJ_IsFocused(hObject)||OBJ_GetFlags(hObject)&OF_NOACTIVATE;
  R_DrawConsole(&(DRAWCONSOLESTRUCT){
    .Buffer = pConsoleView->_buffer,
    .Width = pConsoleView->BufferWidth,
    .Height = pConsoleView->ContentHeight+1,
    .Scroll = pConsoleView->_scroll,
    .Cursor = pConsoleView->Cursor,
    .SelectedItem = pConsoleView->SelectedIndex,
    .SoftSelection = !bFocused,
    .DrawShadow = pConsoleView->DropShadow,
    .Rect = (struct rect) {
      GetNode2D(hObject)->Matrix.v[12],
      GetNode2D(hObject)->Matrix.v[13],
      GetNode(hObject)->Size.Axis[0].Actual,
      GetNode(hObject)->Size.Axis[1].Actual
    },
  });
  return TRUE;
}

HANDLER(ConsoleView, Node, ScrollWheel) {
  int const h = (pConsoleView->ContentHeight) * CONSOLE_CHAR_HEIGHT;
  float const space = GetNode(hObject)->Size.Axis[1].Actual - h;
  pConsoleView->_scroll.y += pScrollWheel->deltaY / SCROLL_SENSIVITY;
  pConsoleView->_scroll.y = MAX(space, pConsoleView->_scroll.y);
  pConsoleView->_scroll.y = MIN(0, pConsoleView->_scroll.y);
  return TRUE;
}
