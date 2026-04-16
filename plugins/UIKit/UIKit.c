#include <plugins/UIKit/UIKit.h>
#include <include/api.h>

bool_t is_server = FALSE;

extern int f_ConsoleView_getIndexPosition(lua_State *L);
extern int f_ConsoleView_unpack(lua_State *L);

extern int f_beginDraggingSession(lua_State *L);
extern LRESULT ui_handle_event(lua_State* L, struct AXmessage *msg);

void on_ui_module_registered(lua_State* L) {
  luaX_require(L, "orca.core", 0);
  lua_getglobal(L, "SERVER");
  is_server = lua_toboolean(L, -1);
  lua_pop(L, 1);
  SV_RegisterMessageProc(ui_handle_event);
  lua_pushcfunction(L, f_beginDraggingSession);
  lua_setfield(L, -2, "beginDraggingSession");
  
  // Expose ConsoleView query helpers as module-level functions
  lua_pushcfunction(L, f_ConsoleView_unpack);
  lua_setfield(L, -2, "consoleViewUnpack");
  lua_pushcfunction(L, f_ConsoleView_getIndexPosition);
  lua_setfield(L, -2, "consoleViewGetIndexPosition");
}

void
after_ui_module_registered(lua_State* L) {
  // Register UIKit table in package.loaded so TerminalView.lua can require it
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "loaded");
  lua_pushvalue(L, -3); // UIKit table
  lua_setfield(L, -2, "orca.UIKit");
  lua_pop(L, 2); // pop loaded, package
  // Load TerminalView Lua extension and expose it as UIKit.TerminalView.
  // Guard against recursive require() returning package.loaded's in-progress
  // sentinel (boolean true) when LayerPrefabPlaceholder triggers require()
  // during this same luaopen call — only assign if a real table/function was returned.
  if (luaX_require(L, "orca.UIKit.TerminalView", 1) == LUA_OK) {
    if (lua_istable(L, -1) || lua_isfunction(L, -1)) {
      lua_setfield(L, -2, "TerminalView");
    } else {
      lua_pop(L, 1);
    }
  }
}
