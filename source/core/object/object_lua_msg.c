#include <include/api.h>
#include <include/orca.h>

#include <source/core/core_local.h>

static struct {
  char data[0x10000];
  uint16_t writer;
} g_mem;

void OBJ_PostMessage(lua_State* L, lpObject_t self, lpcString_t message)
{
  if (lua_type(L, 3) == LUA_TUSERDATA) {
    size_t size = lua_rawlen(L, 3);
    if (size > sizeof(g_mem.data)) {
      Con_Error("Message data too large: %zu bytes", size);
      return;
    }    
    if (g_mem.writer + size > sizeof(g_mem.data)) {
      g_mem.writer = 0;
    }
    handle_t data = g_mem.data + g_mem.writer;
    memcpy(data, lua_touserdata(L, 3), size);
    g_mem.writer += size;
    SV_PostMessage(self, message, 0, data);
  } else if (!strcmp(message, "WindowPaint")) {
    struct WI_Size size;
    WI_GetSize(&size);
    SV_PostMessage(self, message, MAKEDWORD(size.width, size.height), NULL);
  } else {
    SV_PostMessage(self, message, 0, NULL);
  }
}

// HACK: hacked in quickly
void OBJ_SendMessage2(lua_State* L, lpObject_t self, lpcString_t message)
{
  const int nargs = MAX(lua_gettop(L) - 2, 0);
  fixedString_t argtype={0};
  snprintf(argtype, sizeof(argtype), "%sMsgArgs", message);
  if (luaL_testudata(L, 3, argtype)) {
    OBJ_SendMessage(self, message, 0, lua_touserdata(L, 3));
  } else if (!strcmp(message, "WindowPaint")) {
    struct WI_Size size;
    WI_GetSize(&size);
    OBJ_SendMessage(self, message, MAKEDWORD(size.width, size.height), NULL);
  } else if (luaL_getmetatable(L, argtype)) {
    lua_insert(L, 3);
    lua_call(L, nargs, 1);
    OBJ_SendMessage(self, message, 0, lua_touserdata(L, -1));
    lua_pop(L, 1);
  } else {
    OBJ_SendMessage(self, message, 0, NULL);
  }
}

#include <plugins/UIKit/UIKit.h>

#define MAX_HIERARCHY_DEPTH 64

lpObject_t OBJ_DispatchEvent(lua_State* L, lpObject_t self, lpcString_t event, enum MessageRouting routing)
{
  uint32_t dwNumArgs = MAX(0, lua_gettop(L) - 2);
  shortStr_t pszEventName;
  strncpy(pszEventName, event, sizeof(pszEventName));
  lua_remove(L, 2); // clear event name to send object with args to parents

  if (routing == kMessageRoutingDirect) {
    struct HandleMessageMsgArgs ev = {
      .FirstArg = 1,
      .NumArgs = dwNumArgs + 1,
    };
    strncpy(ev.EventName, pszEventName, sizeof(ev.EventName));
    return OBJ_SendMessage(self, "HandleMessage", 0, &ev) ? self : NULL;
  }

  if (routing == kMessageRoutingBubbling) {
    for (lpObject_t obj = self; obj; obj = OBJ_GetParent(obj)) {
      struct HandleMessageMsgArgs ev = {
        .FirstArg = 1,
        .NumArgs = dwNumArgs + 1,
      };
      strncpy(ev.EventName, pszEventName, sizeof(ev.EventName));
      if (OBJ_SendMessage(obj, "HandleMessage", 0, &ev)) {
        return obj;
      }
    }
    return NULL;
  }

  // For Tunneling and TunnelingBubbling, collect the path from self up to root
  lpObject_t path[MAX_HIERARCHY_DEPTH];
  int count = 0;
  for (lpObject_t obj = self; obj && count < MAX_HIERARCHY_DEPTH; obj = OBJ_GetParent(obj)) {
    path[count++] = obj;
  }
  if (count == MAX_HIERARCHY_DEPTH && OBJ_GetParent(path[count - 1])) {
    Con_Error("DispatchEvent: hierarchy depth exceeds %d, tunneling will not reach root", MAX_HIERARCHY_DEPTH);
  }

  if (routing == kMessageRoutingTunneling) {
    // Dispatch from root down to self
    for (int i = count - 1; i >= 0; i--) {
      struct HandleMessageMsgArgs ev = {
        .FirstArg = 1,
        .NumArgs = dwNumArgs + 1,
      };
      strncpy(ev.EventName, pszEventName, sizeof(ev.EventName));
      if (OBJ_SendMessage(path[i], "HandleMessage", 0, &ev)) {
        return path[i];
      }
    }
    return NULL;
  }

  // kMessageRoutingTunnelingBubbling: first tunnel down (root→self), then bubble up (self→root).
  // Each object receives the event twice — once during each phase — unless a handler stops
  // propagation by returning true, which terminates the entire dispatch immediately.
  for (int i = count - 1; i >= 0; i--) {
    struct HandleMessageMsgArgs ev = {
      .FirstArg = 1,
      .NumArgs = dwNumArgs + 1,
    };
    strncpy(ev.EventName, pszEventName, sizeof(ev.EventName));
    if (OBJ_SendMessage(path[i], "HandleMessage", 0, &ev)) {
      return path[i];
    }
  }
  for (lpObject_t obj = self; obj; obj = OBJ_GetParent(obj)) {
    struct HandleMessageMsgArgs ev = {
      .FirstArg = 1,
      .NumArgs = dwNumArgs + 1,
    };
    strncpy(ev.EventName, pszEventName, sizeof(ev.EventName));
    if (OBJ_SendMessage(obj, "HandleMessage", 0, &ev)) {
      return obj;
    }
  }
  return NULL;
}
