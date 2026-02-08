#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

int f_peek_iterator(lua_State* L)
{
  struct WI_Message msg;
  lpObject_t __userdata = lua_touserdata(L, lua_upvalueindex(1));
  WI_PollEvent(&msg);
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
    case kEventWindowClosed:
    case kEventWindowPaint:
    case kEventWindowChangedScreen:
    case kEventWindowResized:
    case 0x23d83fd3: // kEventReadCommands
      msg.target = __userdata;
      break;
  }
  struct WI_Message* out = lua_newuserdata(L, sizeof(struct WI_Message));
  luaL_setmetatable(L, "Event");
  memcpy(out, &msg, sizeof(struct WI_Message));
  return 1;
}

int f_peek_message(lua_State* L) {
  luaX_parsefield(lpObject_t, __userdata, 1, luaL_checkudata, API_TYPE_OBJECT);
  lua_pushlightuserdata(L, __userdata);
  lua_pushcclosure(L, f_peek_iterator, 1);
  return 1;
}

//void
//WI_PostMessageW(lpObject_t hobj, uint32_t Msg, wParam_t wParam, lParam_t lParam)
//{
//  static uint32_t message_id = 0;
//  struct WI_Message data = { hobj, Msg, wParam, lParam, message_id++ };
//  // HACK: unclear why this happens
//  if (Msg == kEventWindowPaint) {
//    for (uint16_t r = queue.read; r != queue.write; r++) {
//      if (queue.data[r].message == kEventWindowPaint &&
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
//  WI_PostMessageW(hobj, fnv1a32(Msg), wParam, lParam);
//}

#define MAX_CLIENTS 256
typedef int (*message_proc_t)(lua_State*, struct WI_Message*);
static message_proc_t clients[MAX_CLIENTS];

bool_t SV_DispatchMessage(lua_State* L, struct WI_Message* msg) {
  if (!msg->target)
    return FALSE;
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i] && clients[i](L, msg)) {
      return TRUE;
    }
  }
  return FALSE;
}

int f_dispatch_message(lua_State* L) {
  lua_pushboolean(L, SV_DispatchMessage(L, luaL_checkudata(L, 1, "Event")));
  return 1;
}

bool_t SV_RegisterMessageProc(int (*proc)(lua_State*, struct WI_Message *)) {
  memmove(&clients[1], &clients[0], (MAX_CLIENTS - 1) * sizeof(clients[0]));
  clients[0] = proc;
  return FALSE;
}

bool_t SV_UnregisterMessageProc(int (*proc)(lua_State*, struct WI_Message *)) {
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
