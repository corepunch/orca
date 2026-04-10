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
PointToData(ConsoleViewCPtr t, int mx, int my, uint32_t *cursor)
{
  vec2_t scrolled = VEC2_Sub(&(vec2_t){mx, my}, &t->_scroll);
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
_cmd(ConsoleViewPtr t, lpcString_t str, struct tstate *state)
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
_println(ConsoleViewPtr t, lua_Integer index, lpcString_t text, size_t slen)
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

int f_ConsoleView_unpack(lua_State *L) {
  float x = luaL_checknumber(L, 2);
  float y = luaL_checknumber(L, 3);
  lpObject_t obj = luaX_checkObject(L, 1);
#ifdef MOUSE_EVENTS_USE_LOCAL_SPACE
  struct vec3 out = (struct vec3){x,y,0};
#else
  struct mat4 inv = MAT4_Inverse(&GetNode2D(obj)->Matrix);
  struct vec3 out = MAT4_MultiplyVector3D(&inv, &(struct vec3){x,y,0});
#endif
  ConsoleViewPtr t = GetConsoleView(obj);
  uint32_t data = PointToData(t, out.x, out.y, NULL);

  lua_pushinteger(L, data>>16);
  lua_pushlstring(L, (char*)&data, 1);
  return 2;
}

int f_ConsoleView_getIndexPosition(lua_State *L) {
  lpObject_t o = luaX_checkObject(L, 1);
  ConsoleViewPtr t = GetConsoleView(o);
  lua_Integer i = luaL_checkinteger(L, 2);
  lua_Integer offx = luaL_optinteger(L, 3, 0);
  lua_Integer offy = luaL_optinteger(L, 4, 0);
  if (i == 0)
    return 0;
  FOR_LOOP(j, t->BufferWidth*t->BufferHeight) {
    if (t->_buffer[j]>>16 == i) {
      if (lua_toboolean(L, 5)) {
        struct vec3 p = {
          .x = ((j%t->BufferWidth)+offx)*CONSOLE_CHAR_WIDTH,
          .y = ((j/t->BufferWidth)+offy)*CONSOLE_CHAR_HEIGHT,
        };
        struct vec3 out = MAT4_MultiplyVector3D(&GetNode2D(o)->Matrix, &p);
        lua_pushnumber(L, out.x+t->_scroll.x);
        lua_pushnumber(L, out.y+t->_scroll.y);
      } else {
        lua_pushnumber(L, ((j%t->BufferWidth)+offx)*CONSOLE_CHAR_WIDTH+t->_scroll.x);
        lua_pushnumber(L, ((j/t->BufferWidth)+offy)*CONSOLE_CHAR_HEIGHT+t->_scroll.y);
      }
      return 2;
    }
  }
  return 0;
}

HANDLER(ConsoleView, ConsoleView, Println) {
  if (!pConsoleView->_buffer) return FALSE;
  lpcString_t text = pPrintln ? pPrintln->Text : NULL;
  lua_Integer index = pPrintln ? pPrintln->Index : 0;
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
  WI_PostMessageW(hObject, kMsgPaint, 0, NULL);
  return FALSE;
}

HANDLER(ConsoleView, Object, Create) {
  pConsoleView->_buffer = ZeroAlloc(MEMSIZE(pConsoleView));
  WI_PostMessageW(hObject, kMsgPaint, 0, NULL);
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
