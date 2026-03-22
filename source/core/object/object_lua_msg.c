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
  if (lua_type(L, 3) == LUA_TUSERDATA) {
    OBJ_SendMessage(self, message, 0, lua_touserdata(L, 3));
  } else if (!strcmp(message, "WindowPaint")) {
    struct WI_Size size;
    WI_GetSize(&size);
    OBJ_SendMessage(self, message, MAKEDWORD(size.width, size.height), NULL);
  } else {
    OBJ_SendMessage(self, message, 0, NULL);
  }
}

#include <plugins/UIKit/UIKit.h>

lpObject_t OBJ_DispatchEvent(lua_State* L, lpObject_t self, lpcString_t event)
{
  uint32_t dwNumArgs = MAX(0, lua_gettop(L) - 2);
  shortStr_t pszEventName;
  strncpy(pszEventName, event, sizeof(pszEventName));
  lua_remove(L, 2); // clear event name to send object with args to parents
  for (lpObject_t obj = self; obj; obj = OBJ_GetParent(obj)) {
    struct HandleMessageEventArgs event = {
      .FirstArg = 1,
      .NumArgs = dwNumArgs + 1,
    };
    strncpy(event.EventName, pszEventName, sizeof(event.EventName));
    if (OBJ_SendMessage(obj, "HandleMessage", 0, &event)) {
      return obj;
    }
  }
  return NULL;
}
