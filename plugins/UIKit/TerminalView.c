#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <plugins/UIKit/UIKit.h>

#define ITEMS_LIST "__items"
#define SCROLL_SENSIVITY 1.2f
#define BUFFER_WIDTH 256
#define BUFFER_HEIGHT 256
#define TAB_SIZE 4
#define MEMSIZE(T) (T->BufferWidth*T->BufferHeight*4)

#define kMsgPaint 0x36b37bd3

static uint32_t
PointToData(TerminalViewCPtr t, int mx, int my, uint32_t *cursor)
{
  vec2_t scrolled = VEC2_Sub(&(vec2_t){mx, my}, &t->_scroll);
  uint32_t x = scrolled.x / CONSOLE_CHAR_WIDTH;
  uint32_t y = scrolled.y / CONSOLE_CHAR_HEIGHT;
  if (x < t->BufferWidth && y < t->BufferHeight) {
    if (cursor) *cursor = y * x < t->BufferWidth + x;
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
_cmd(TerminalViewPtr t, lpcString_t str, struct tstate *state)
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

HANDLER(TerminalView, Object, Create) {
  pTerminalView->_buffer = ZeroAlloc(MEMSIZE(pTerminalView));
  WI_PostMessageW(hObject, kMsgPaint, 0, NULL);
  return FALSE;
}

HANDLER(TerminalView, TerminalView, println) {
  lua_State* L = (lua_State*)pprintln;
  struct tstate state = { .foreground = 7 };
  lua_Integer len = 0;
  lua_getfield(L, 1, ITEMS_LIST);
  if (!lua_isnil(L, -1)) {
    lua_len(L, -1);
    len = lua_tointeger(L, -1);
    lua_pop(L, 1);
  }
  if (lua_isnil(L, 3)) {
    len = 0;
  } else {
    lua_pushvalue(L, 3);
    lua_seti(L, -2, ++len);
  }
  lua_pop(L, 1);
  int n = lua_gettop(L), c = pTerminalView->Cursor;
  state.item = len;
  for (int arg = 4; arg <= n; arg++) {
    size_t slen;
    uint8_t prev = 0;
    const char *str = luaL_tolstring(L, arg, &slen);
    for (lpcString_t s = str; s - str < slen; prev = *s, s++) {
      if (*s == '\x21' && *(s+1) == '[') {
        s = _cmd(pTerminalView, ++s, &state);
      } else if (*s == '\n') {
        pTerminalView->Cursor = ((pTerminalView->Cursor / pTerminalView->BufferWidth) + 1) * pTerminalView->BufferWidth;
      } else if (*s == '\t') {
        int start = (pTerminalView->Cursor / pTerminalView->BufferWidth) * pTerminalView->BufferWidth;
        pTerminalView->Cursor = start + (((pTerminalView->Cursor - start) / TAB_SIZE) + 1) * TAB_SIZE;
      } else {
        state.glyph = (uint8_t)*s;
        pTerminalView->_buffer[pTerminalView->Cursor++] = *(uint32_t*)&state;
      }
    }
    lua_pop(L, 1);
  }
  while (pTerminalView->Cursor % pTerminalView->BufferWidth) {
    state.glyph = 0;
    pTerminalView->_buffer[pTerminalView->Cursor++] = *(uint32_t*)&state;
  }
  pTerminalView->_contentHeight += MAX(1, (pTerminalView->Cursor - c) / pTerminalView->BufferWidth);
  lua_pushinteger(L, len);
  return 1;
}

HANDLER(TerminalView, TerminalView, unpack) {
  lua_State* L = (lua_State*)punpack;
  float x = luaL_checknumber(L, 3);
  float y = luaL_checknumber(L, 4);
#ifdef MOUSE_EVENTS_USE_LOCAL_SPACE
  struct vec3 out = (struct vec3){x,y,0};
#else
  struct mat4 inv = MAT4_Inverse(&GetNode2D(hObject)->Matrix);
  struct vec3 out = MAT4_MultiplyVector3D(&inv, &(struct vec3){x,y,0});
#endif
  uint32_t data = PointToData(pTerminalView, out.x, out.y, NULL);
  lua_getfield(L, 1, ITEMS_LIST);
  lua_geti(L, -1, data>>16);
  lua_replace(L, -2);
  lua_pushinteger(L, data>>16);
  lua_pushlstring(L, (char*)&data, 1);
  return 3;
}

HANDLER(TerminalView, TerminalView, erase) {
  lua_State* L = (lua_State*)perase;
  memset(pTerminalView->_buffer, 0, MEMSIZE(pTerminalView));
  pTerminalView->Cursor = 0;
  pTerminalView->_contentHeight = 0;
  lua_newtable(L);
  lua_setfield(L, 1, ITEMS_LIST);
  return 0;
}

HANDLER(TerminalView, TerminalView, invalidate) {
  (void)pinvalidate;
  WI_PostMessageW(hObject, kMsgPaint, 0, NULL);
  return 0;
}

HANDLER(TerminalView, TerminalView, getIndexPosition) {
  lua_State* L = (lua_State*)pgetIndexPosition;
  lua_Integer i = luaL_checkinteger(L, 3);
  lua_Integer offx = luaL_optinteger(L, 4, 0);
  lua_Integer offy = luaL_optinteger(L, 5, 0);
  if (i == 0) {
    return 0;
  }
  FOR_LOOP(j, pTerminalView->BufferWidth*pTerminalView->BufferHeight) {
    if (pTerminalView->_buffer[j]>>16 == i) {
      if (lua_toboolean(L, 6)) {
        struct vec3 p = {
          .x = ((j%pTerminalView->BufferWidth)+offx)*CONSOLE_CHAR_WIDTH,
          .y = ((j/pTerminalView->BufferWidth)+offy)*CONSOLE_CHAR_HEIGHT,
        };
        struct vec3 out = MAT4_MultiplyVector3D(&GetNode2D(hObject)->Matrix, &p);
        lua_pushnumber(L, out.x+pTerminalView->_scroll.x);
        lua_pushnumber(L, out.y+pTerminalView->_scroll.y);
      } else {
        lua_pushnumber(L, ((j%pTerminalView->BufferWidth)+offx)*CONSOLE_CHAR_WIDTH+pTerminalView->_scroll.x);
        lua_pushnumber(L, ((j/pTerminalView->BufferWidth)+offy)*CONSOLE_CHAR_HEIGHT+pTerminalView->_scroll.y);
      }
      return 2;
    }
  }
  return 0;
}

HANDLER(TerminalView, TerminalView, selectedItem) {
  lua_State* L = (lua_State*)pselectedItem;
  lua_getfield(L, 1, ITEMS_LIST);
  lua_geti(L, -1, pTerminalView->SelectedIndex);
  return 1;
}

HANDLER(TerminalView, TerminalView, numItems) {
  lua_State* L = (lua_State*)pnumItems;
  lua_getfield(L, 1, ITEMS_LIST);
  lua_len(L, -1);
  return 1;
}

HANDLER(TerminalView, Node2D, DrawBrush) {
  bool_t bFocused = OBJ_IsFocused(hObject)||OBJ_GetFlags(hObject)&OF_NOACTIVATE;
  R_DrawConsole(&(DRAWCONSOLESTRUCT){
    .Buffer = pTerminalView->_buffer,
    .Width = pTerminalView->BufferWidth,
    .Height = pTerminalView->_contentHeight+1,
    .Scroll = pTerminalView->_scroll,
    .Cursor = pTerminalView->Cursor,
    .SelectedItem = pTerminalView->SelectedIndex,
    .SoftSelection = !bFocused,
    .DrawShadow = pTerminalView->DropShadow,
    .Rect = (struct rect) {
      GetNode2D(hObject)->Matrix.v[12],
      GetNode2D(hObject)->Matrix.v[13],
      GetNode(hObject)->Size.Axis[0].Actual,
      GetNode(hObject)->Size.Axis[1].Actual
    },
  });
  return TRUE;
}

HANDLER(TerminalView, Mouse, ScrollWheel) {
  int const h = (pTerminalView->_contentHeight) * CONSOLE_CHAR_HEIGHT;
  float const space = GetNode(hObject)->Size.Axis[1].Actual - h;
  pTerminalView->_scroll.y += pScrollWheel->deltaY / SCROLL_SENSIVITY;
  pTerminalView->_scroll.y = MAX(space, pTerminalView->_scroll.y);
  pTerminalView->_scroll.y = MIN(0, pTerminalView->_scroll.y);
  return TRUE;
}
