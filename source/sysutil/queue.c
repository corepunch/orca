#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

int f_peek_iterator(lua_State* L)
{
  struct AXmessage msg = {0};
  lpObject_t __userdata = lua_touserdata(L, lua_upvalueindex(1));
  int has_event = axPollEvent(&msg);
#if __EMSCRIPTEN__
  /* Return nil when the queue is empty so the Lua for-loop terminates and
     the main coroutine can yield back to emscripten_set_main_loop. */
  if (!has_event) {
    return 0;
  }
#else
  (void)has_event;  /* suppress -Wunused-but-set-variable on non-Emscripten targets */
#endif
  static Window_PaintMsg_t wnd;
  wnd.WindowWidth = LOWORD(msg.wParam);
  wnd.WindowHeight = HIWORD(msg.wParam);
  switch (msg.message) {
    case kEventLeftMouseDown:
    case kEventRightMouseDown:
    case kEventOtherMouseDown:
    case kEventLeftMouseUp:
    case kEventRightMouseUp:
    case kEventOtherMouseUp:
    case kEventLeftMouseDragged:
    case kEventRightMouseDragged:
    case kEventOtherMouseDragged:
    case kEventLeftDoubleClick:
    case kEventRightDoubleClick:
    case kEventOtherDoubleClick:
    case kEventMouseMoved:
    case kEventScrollWheel:
    case kEventKeyDown:
    case kEventKeyUp:
    case kEventReadCommands:
      msg.target = __userdata;
      break;
    case kEventWindowPaint:
      msg.message = ID_Window_Paint;
      msg.target = __userdata;
      msg.lParam = &wnd;
      break;
    case kEventWindowResized:
      msg.message = ID_Window_Resized;
      msg.target = __userdata;
      msg.lParam = &wnd;
      break;
    case kEventWindowChangedScreen:
      msg.message = ID_Window_ChangedScreen;
      msg.target = __userdata;
      break;
    case kEventWindowClosed:
      msg.message = ID_Window_Closed;
      msg.target = __userdata;
      break;
  }
  struct AXmessage* out = lua_newuserdata(L, sizeof(struct AXmessage));
  luaL_setmetatable(L, "Event");
  memcpy(out, &msg, sizeof(struct AXmessage));
  return 1;
}

int f_peek_message(lua_State* L) {
  luaX_parsefield(lpObject_t, __userdata, 1, luaL_checkudata, API_TYPE_OBJECT);
  lua_pushlightuserdata(L, __userdata);
  lua_pushcclosure(L, f_peek_iterator, 1);
  return 1;
}

//void
//axPostMessageW(lpObject_t hobj, uint32_t Msg, wParam_t wParam, lParam_t lParam)
//{
//  static uint32_t message_id = 0;
//  struct AXmessage data = { hobj, Msg, wParam, lParam, message_id++ };
//  // HACK: unclear why this happens
//  if (Msg == kMsgWindowPaint) {
//    for (uint16_t r = queue.read; r != queue.write; r++) {
//      if (queue.data[r].message == kMsgWindowPaint &&
//          queue.data[r].hobj == hobj)
//      {
//        memset(&queue.data[r], 0, sizeof(queue.data[r]));
//        //      Con_Error("%d %d %d %08x", queue.write - queue.read, r, queue.read, queue.data[r].hobj);
//      }
//    }
//  }
//  queue.data[queue.write++] = data;
//}

//void
//SV_PostMessage(lpObject_t hobj, lpcString_t Msg, wParam_t wParam, lParam_t lParam)
//{
//  axPostMessageW(hobj, fnv1a32(Msg), wParam, lParam);
//}

#define MAX_CLIENTS 256
#define kMsgReadCommands 0x23d83fd3
typedef LRESULT (*message_proc_t)(lua_State*, struct AXmessage*);
static message_proc_t clients[MAX_CLIENTS];

ORCA_API bool_t SV_DispatchMessage(lua_State* L, struct AXmessage* msg) {
  if (!msg->target && msg->message != kMsgReadCommands)
    return FALSE;
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i] && clients[i](L, msg)) {
      return TRUE;
    }
  }
  return FALSE;
}

ORCA_API void
axBuildModifiersString(wParam_t wParam, char* buf, size_t size)
{
  buf[0] = '\0';
  if (wParam & AX_MOD_CTRL)  strncat(buf, "ctrl+",  size - strlen(buf) - 1);
  if (wParam & AX_MOD_ALT)   strncat(buf, "alt+",   size - strlen(buf) - 1);
  if (wParam & AX_MOD_SHIFT) strncat(buf, "shift+", size - strlen(buf) - 1);
  if (wParam & AX_MOD_CMD)   strncat(buf, "cmd+",   size - strlen(buf) - 1);
}

lpcString_t axKeynumToString(uint32_t keynum);

/* Only ASCII printable range: translating extended/Unicode characters requires
   UTF-8 decoding, which is left to higher-level key handlers. */
static bool_t is_printable_char(int ch) {
  return ch >= 0x20 && ch <= 0x7E;
}

/* Apply shift-key transformation to a printable ASCII character. */
static char apply_shift(char ch) {
  if (ch >= '0' && ch <= '9') {
    lpcString_t sym = ")!@#$%^&*(";
    return sym[ch - '0'];
  }
  return toupper(ch);
}

int f_translate_message(lua_State* L) {
  struct AXmessage const* msg = luaL_checkudata(L, 1, "Event");
  if (msg->message == kEventKeyDown && is_printable_char(msg->wParam & 0xff)) {
    char ch = msg->wParam & 0xff;
    if (msg->wParam & AX_MOD_SHIFT)
      ch = apply_shift(ch);
    lParam_t lp = 0;
    *(char*)&lp = ch;
    axPostMessageW(msg->target, kEventChar, msg->wParam, lp);
  }
  return 0;
}

ORCA_API void
axKeyEventToText(struct AXmessage const* e, char* buf, size_t size)
{
  if (e->message == kEventChar) {
    /* lParam stores the computed character written by f_translate_message. */
    uint32_t len = 0;
    while (len < sizeof(e->lParam) && ((lpcString_t)&e->lParam)[len])
      len++;
    if (len >= size) len = (uint32_t)size - 1;
    memcpy(buf, &e->lParam, len);
    buf[len] = '\0';
  } else {
#if __linux__
    uint32_t len = 0;
    while (len < sizeof(e->lParam) && ((lpcString_t)&e->lParam)[len])
      len++;
    if (len >= size) len = (uint32_t)size - 1;
    memcpy(buf, &e->lParam, len);
    buf[len] = '\0';
#else
    snprintf(buf, size, "%s", axKeynumToString(e->wParam));
#endif
  }
}

int f_event_new(lua_State* L);
int f_dispatch_message(lua_State* L) {
  if (lua_istable(L, 1)) {
    lua_pushcfunction(L, f_event_new);
    lua_insert(L, -2);
    lua_call(L, 1, 1);
    lua_pushcfunction(L, f_dispatch_message);
    lua_insert(L, -2);
    lua_call(L, 1, 1);
  } else {
    lua_pushboolean(L, SV_DispatchMessage(L, luaL_checkudata(L, 1, "Event")));
  }
  return 1;
}

bool_t SV_RegisterMessageProc(LRESULT (*proc)(lua_State*, struct AXmessage *)) {
  memmove(&clients[1], &clients[0], (MAX_CLIENTS - 1) * sizeof(clients[0]));
  clients[0] = proc;
  return FALSE;
}

bool_t SV_UnregisterMessageProc(LRESULT (*proc)(lua_State*, struct AXmessage *)) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i] == proc) {
      int remaining = MAX_CLIENTS - i - 1;
      if (remaining > 0) {
        memmove(&clients[i], &clients[i + 1], remaining * sizeof(clients[0]));
      }
      clients[MAX_CLIENTS - 1] = NULL; // clean up last slot
      return TRUE;
    }
  }
  return FALSE;
}

ORCA_API void SV_Init(void) {
  memset(clients, 0, sizeof(clients));
}

ORCA_API void SV_Shutdown(void) {
}
