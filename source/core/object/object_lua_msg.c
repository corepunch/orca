#include <include/api.h>
#include <include/orca.h>

#include <source/core/core_local.h>

static struct {
  char data[0x10000];
  uint16_t writer;
} g_mem;

static handle_t write_event_data(lua_State* L, size_t size, const void *udata) {
  if (size > sizeof(g_mem.data)) {
    luaL_error(L, "Message data too large: %zu bytes", size);
    return NULL;
  }
  if (g_mem.writer + size > sizeof(g_mem.data)) {
    g_mem.writer = 0;
  }
  handle_t data = g_mem.data + g_mem.writer;
  memcpy(data, udata, size);
  g_mem.writer += size;
  return data;
}

void OBJ_send(lua_State* L, lpObject_t self, lpcString_t message)
{
  const int nargs = MAX(lua_gettop(L) - 2, 0);
  fixedString_t argtype={0};
  // Support qualified message names like "Object.Create" -> "Object_CreateEventArgs"
  fixedString_t qualified={0};
  strncpy(qualified, message, sizeof(qualified) - 1);
  for (char *p = qualified; *p; p++) if (*p == '.') *p = '_';
  snprintf(argtype, sizeof(argtype), "%sEventArgs", qualified);
  
  if (luaL_testudata(L, 3, argtype)) {
    SV_PostMessage(self, message, 0, write_event_data(L, lua_rawlen(L, 3), lua_touserdata(L, 3)));
  } else if (luaL_getmetatable(L, argtype)) { // Check if a constructor exists for this message args type
    lua_insert(L, 3); // Move the constructor below the message args on the stack
    lua_call(L, nargs, 1); // Call the constructor to create the message args struct
    SV_PostMessage(self, message, 0, write_event_data(L, lua_rawlen(L, -1), lua_touserdata(L, -1))); // Write the constructed message args to the event data buffer and post the message
    lua_pop(L, 1); // Pop the result of the message args constructor
  } else {
    lua_pop(L, 1); // Pop the metatable result
    SV_PostMessage(self, message, 0, NULL);
  }
}

int OBJ_fetch(lua_State* L, lpObject_t self, lpcString_t message)
{
  const int nargs = MAX(lua_gettop(L) - 2, 0);
  fixedString_t argtype={0};
  // Support qualified message names like "Object.Create" -> "Object_CreateEventArgs"
  fixedString_t qualified={0};
  strncpy(qualified, message, sizeof(qualified) - 1);
  for (char *p = qualified; *p; p++) if (*p == '.') *p = '_';
  snprintf(argtype, sizeof(argtype), "%sEventArgs", qualified);
  
  LRESULT result;
  if (luaL_testudata(L, 3, argtype)) {
    result = OBJ_SendMessage(self, message, 0, lua_touserdata(L, 3));
  } else if (luaL_getmetatable(L, argtype)) { // Check if a constructor exists for this message args type
    lua_insert(L, 3); // Move the constructor below the message args on the stack
    lua_call(L, nargs, 1); // Call the constructor to create the message args struct
    result = OBJ_SendMessage(self, message, 0, lua_touserdata(L, -1)); // Send the message with the constructed args
    lua_pop(L, 1); // Pop the result of the message args constructor
  } else {
    lua_pop(L, 1); // Pop the metatable result
    result = OBJ_SendMessage(self, message, 0, NULL);
  }
  lua_pushinteger(L, result);
  return 1;
}

