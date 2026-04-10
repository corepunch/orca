#include <plugins/UIKit/UIKit.h>
#include <include/api.h>

int f_Thickness_New(lua_State* L) {
  struct Thickness self={0};
  switch (lua_gettop(L))
  {
    case 4:
      self.Axis[0] = (struct EdgeShorthand){luaL_checknumber(L, 1), luaL_checknumber(L, 3)};
      self.Axis[1] = (struct EdgeShorthand){luaL_checknumber(L, 2), luaL_checknumber(L, 4)};
      return (luaX_pushThickness(L, &self), 1);
    case 3:
      self.Axis[0] = (struct EdgeShorthand){luaL_checknumber(L, 1), luaL_checknumber(L, 3)};
      self.Axis[1] = (struct EdgeShorthand){luaL_checknumber(L, 2), luaL_checknumber(L, 2)};
      return (luaX_pushThickness(L, &self), 1);
    case 2:
      self.Axis[0] = (struct EdgeShorthand){luaL_checknumber(L, 1), luaL_checknumber(L, 1)};
      self.Axis[1] = (struct EdgeShorthand){luaL_checknumber(L, 2), luaL_checknumber(L, 2)};
      return (luaX_pushThickness(L, &self), 1);
    case 1:
      self.Axis[0] = (struct EdgeShorthand){luaL_checknumber(L, 1), luaL_checknumber(L, 1)};
      self.Axis[1] = (struct EdgeShorthand){luaL_checknumber(L, 1), luaL_checknumber(L, 1)};
      return (luaX_pushThickness(L, &self), 1);
    default:
      return 0;
  }
}

int f_EdgeShorthand_New(lua_State* L) {
  switch (lua_gettop(L)) {
    case 2:
      return (luaX_pushEdgeShorthand(L, &(struct EdgeShorthand){luaL_checknumber(L, 1), luaL_checknumber(L, 2)}), 1);
    case 1:
      return (luaX_pushEdgeShorthand(L, &(struct EdgeShorthand){luaL_checknumber(L, 1), luaL_checknumber(L, 1)}), 1);
    default:
      return 0;
  }
}

int f_CornerRadius_New(lua_State* L) {
  switch (lua_gettop(L)) {
    case 4:
      return (luaX_pushCornerRadius(L, &(struct CornerRadius){luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4)}), 1);
    case 1:
      return (luaX_pushCornerRadius(L, &(struct CornerRadius){luaL_checknumber(L, 1), luaL_checknumber(L, 1), luaL_checknumber(L, 1), luaL_checknumber(L, 1)}), 1);
    default:
      return 0;
  }
}

int f_Thickness_TextConvert(lua_State* L) {
  float a, b, c, d;
  struct Thickness self={0};
  switch (sscanf(luaL_checkstring(L, 1), "%f %f %f %f", &a, &b, &c, &d)) {
    case 4:
      self.Axis[0] = (struct EdgeShorthand){a, c};
      self.Axis[1] = (struct EdgeShorthand){b, d};
      return (luaX_pushThickness(L, &self), 1);
    case 3:
      self.Axis[0] = (struct EdgeShorthand){a, c};
      self.Axis[1] = (struct EdgeShorthand){b, b};
      return (luaX_pushThickness(L, &self), 1);
    case 2:
      self.Axis[0] = (struct EdgeShorthand){a, a};
      self.Axis[1] = (struct EdgeShorthand){b, b};
      return (luaX_pushThickness(L, &self), 1);
    case 1:
      self.Axis[0] = (struct EdgeShorthand){a, a};
      self.Axis[1] = (struct EdgeShorthand){a, a};
      return (luaX_pushThickness(L, &self), 1);
    default:
      return 0;
  }
}

int f_EdgeShorthand_TextConvert(lua_State* L) {
  float a = 0, b = 0;
  switch (sscanf(luaL_checkstring(L, 1), "%f %f", &a, &b)) {
    case 2:
      return (luaX_pushEdgeShorthand(L, &(struct EdgeShorthand){a, b}), 1);
    case 1:
      return (luaX_pushEdgeShorthand(L, &(struct EdgeShorthand){a, a}), 1);
    default:
      return 0;
  }
}

int f_CornerRadius_TextConvert(lua_State* L) {
  float a, b, c, d;
  switch (sscanf(luaL_checkstring(L, 1), "%f %f %f %f", &a, &b, &c, &d)) {
    case 4:
      return (luaX_pushCornerRadius(L, &(struct CornerRadius){a, b, c, d}), 1);
    case 1:
      return (luaX_pushCornerRadius(L, &(struct CornerRadius){a, a, a, a}), 1);
    default:
      return 0;
  }
}

bool_t is_server = FALSE;

extern int f_ConsoleView_getIndexPosition(lua_State *L);
extern int f_ConsoleView_unpack(lua_State *L);

extern int f_beginDraggingSession(lua_State *L);
extern LRESULT ui_handle_event(lua_State* L, struct WI_Message *msg);

void on_ui_module_registered(lua_State* L) {
  API_CallRequire(L, "orca.core", 0);
  lua_getglobal(L, "SERVER");
  is_server = lua_toboolean(L, -1);
  lua_pop(L, 1);
  SV_RegisterMessageProc(ui_handle_event);
  lua_pushcfunction(L, f_beginDraggingSession);
  lua_setfield(L, -2, "beginDraggingSession");
  
#define OVERRIDE_FROMSTRING(NAME) \
  int f_##NAME##_TextConvert(lua_State* L); \
  int f_##NAME##_New(lua_State* L); \
  lua_getfield(L, -1, #NAME); \
  lua_pushcfunction(L, f_##NAME##_TextConvert); \
  lua_setfield(L, -2, "fromstring"); \
  lua_pushcfunction(L, f_##NAME##_New); \
  lua_setfield(L, -2, "new"); \
  lua_pop(L, 1);
  OVERRIDE_FROMSTRING(Thickness)
  OVERRIDE_FROMSTRING(EdgeShorthand)
  OVERRIDE_FROMSTRING(CornerRadius)
#undef OVERRIDE_FROMSTRING

  // Expose ConsoleView query helpers as module-level functions
  lua_pushcfunction(L, f_ConsoleView_unpack);
  lua_setfield(L, -2, "consoleViewUnpack");
  lua_pushcfunction(L, f_ConsoleView_getIndexPosition);
  lua_setfield(L, -2, "consoleViewGetIndexPosition");

  // Register UIKit table in package.loaded so TerminalView.lua can require it
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "loaded");
  lua_pushvalue(L, -3); // UIKit table
  lua_setfield(L, -2, "orca.UIKit");
  lua_pop(L, 2); // pop loaded, package

  // Load TerminalView Lua extension and expose it as UIKit.TerminalView
  if (API_CallRequire(L, "orca.UIKit.TerminalView", 1) == LUA_OK) {
    if (lua_istable(L, -1) || lua_isfunction(L, -1)) {
      lua_setfield(L, -2, "TerminalView");
    } else {
      lua_pop(L, 1);
    }
  }
}

