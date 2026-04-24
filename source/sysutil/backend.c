#include <include/api.h>
#include <include/orca.h>

static uint32_t parse_event_name(lpcString_t name)
{
  if (!strcmp(name, "LeftButtonDown")) return kEventLeftButtonDown;
  if (!strcmp(name, "RightButtonDown")) return kEventRightButtonDown;
  if (!strcmp(name, "OtherButtonDown")) return kEventOtherButtonDown;
  if (!strcmp(name, "LeftButtonUp")) return kEventLeftButtonUp;
  if (!strcmp(name, "RightButtonUp")) return kEventRightButtonUp;
  if (!strcmp(name, "OtherButtonUp")) return kEventOtherButtonUp;
  if (!strcmp(name, "LeftButtonDragged")) return kEventLeftButtonDragged;
  if (!strcmp(name, "RightButtonDragged")) return kEventRightButtonDragged;
  if (!strcmp(name, "OtherButtonDragged")) return kEventOtherButtonDragged;
  if (!strcmp(name, "LeftDoubleClick")) return kEventLeftDoubleClick;
  if (!strcmp(name, "RightDoubleClick")) return kEventRightDoubleClick;
  if (!strcmp(name, "OtherDoubleClick")) return kEventOtherDoubleClick;
  if (!strcmp(name, "MouseMoved")) return kEventMouseMoved;
  if (!strcmp(name, "ScrollWheel")) return kEventScrollWheel;
  if (!strcmp(name, "KeyDown")) return kEventKeyDown;
  if (!strcmp(name, "KeyUp")) return kEventKeyUp;
  if (!strcmp(name, "Char")) return kEventChar;
  if (!strcmp(name, "ReadCommands")) return kEventReadCommands;
  if (!strcmp(name, "WindowPaint")) return kEventWindowPaint;
  if (!strcmp(name, "WindowResized")) return kEventWindowResized;
  if (!strcmp(name, "WindowChangedScreen")) return kEventWindowChangedScreen;
  if (!strcmp(name, "WindowClosed")) return kEventWindowClosed;
  return fnv1a32(name);
}

//int //f_table_Window(lua_State* L);

int f_event_message(lua_State* L)
{
  struct AXmessage const* msg = luaL_checkudata(L, 1, "Event");
  lua_pushinteger(L, msg->message);
  return 1;
}

int f_event_is(lua_State* L)
{
  struct AXmessage const* msg = luaL_checkudata(L, 1, "Event");
  lpcString_t name = luaL_checkstring(L, 2);
  lua_pushboolean(L, msg->message == fnv1a32(name));
  return 1;
}

void axBuildModifiersString(wParam_t wParam, char* buf, size_t size);
void axKeyEventToText(struct AXmessage const* e, char* buf, size_t size);

int f_event_index(lua_State* L)
{
  struct AXmessage const* msg = luaL_checkudata(L, 1, "Event");
  lpcString_t name = luaL_checkstring(L, 2);
  if (!strcmp(name, "is")) {
    lua_pushcfunction(L, f_event_is);
    return 1;
  }
  if (!strcmp(name, "message")) {
    lua_pushinteger(L, msg->message);
    return 1;
  }
  if (!strcmp(name, "wparam")) {
    lua_pushinteger(L, msg->wParam);
    return 1;
  }
  if (!strcmp(name, "x")) {
    lua_pushinteger(L, LOWORD(msg->wParam));
    return 1;
  }
  if (!strcmp(name, "y")) {
    lua_pushinteger(L, HIWORD(msg->wParam));
    return 1;
  }
  if (!strcmp(name, "dx")) {
    lua_pushinteger(L, LOWORD((intptr_t)msg->lParam));
    return 1;
  }
  if (!strcmp(name, "dy")) {
    lua_pushinteger(L, HIWORD((intptr_t)msg->lParam));
    return 1;
  }
  if (!strcmp(name, "key")) {
    lua_pushinteger(L, LOWORD(msg->wParam));
    return 1;
  }
  if (!strcmp(name, "modflags")) {
    lua_pushinteger(L, HIWORD(msg->wParam));
    return 1;
  }
  if (!strcmp(name, "modifiers")) {
    char comp[32] = {0};
    axBuildModifiersString(msg->wParam, comp, sizeof(comp));
    lua_pushstring(L, comp);
    return 1;
  }
  if (!strcmp(name, "text")) {
    char text[MAX_NAMELEN] = {0};
    axKeyEventToText(msg, text, sizeof(text));
    lua_pushstring(L, text);
    return 1;
  }
  return luaL_error(L, "No field %s in Event", name);
}

int f_event_new(lua_State* L) {
  struct AXmessage* msg = lua_newuserdata(L, sizeof(struct AXmessage));
  luaL_setmetatable(L, "Event");
  if (lua_type(L, 1) == LUA_TTABLE) {
    lua_pushnil(L);  // first key
    while (lua_next(L, 1) != 0) {
      if (lua_type(L, -2) == LUA_TSTRING) {
        const char* key = lua_tostring(L, -2);
        if (strcmp(key, "target") == 0) { msg->target = *(struct Object **)luaL_checkudata(L, -1, API_TYPE_OBJECT);
        } else if (strcmp(key, "message") == 0) { msg->message = parse_event_name(luaL_checkstring(L, -1));
        } else if (strcmp(key, "x") == 0) { msg->x = luaL_checknumber(L, -1);
        } else if (strcmp(key, "y") == 0) { msg->y = luaL_checknumber(L, -1);
        } else if (strcmp(key, "key") == 0) { msg->keyCode = luaL_checknumber(L, -1);
        } else if (strcmp(key, "modflags") == 0) { msg->modflags = luaL_checknumber(L, -1);
        } else if (strcmp(key, "text") == 0) { strncpy((char*)&msg->lParam, luaL_checkstring(L, -1), sizeof(msg->lParam));
        }
      }
      lua_pop(L, 1); // remove value, keep key for next iteration
    }
  }
  return 1;
}
