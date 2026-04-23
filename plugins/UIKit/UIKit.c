#include <plugins/UIKit/UIKit.h>
#include <include/api.h>

bool_t is_server = FALSE;

// Lua state stored during module init so that UIKit_LoadCSSFile can call back
// into Lua to parse CSS text via core.StyleSheet.Parse.
static lua_State* s_css_lua = NULL;

extern int f_beginDraggingSession(lua_State *L);
extern LRESULT ui_handle_event(lua_State* L, struct AXmessage *msg);

// C file loader for .css files, registered with OBJ_RegisterFileLoader.
// Reads the file and delegates parsing to core.StyleSheet.Parse (set up by
// orca.UIKit.file-css loaded in after_ui_module_registered).
static struct Object*
UIKit_LoadCSSFile(const char* path)
{
  if (!s_css_lua) {
    Con_Error("UIKit_LoadCSSFile: CSS loader not initialised (UIKit not loaded?)");
    return NULL;
  }
  lua_State* L = s_css_lua;

  struct file* fp = FS_LoadFile(path);
  if (!fp) {
    Con_Error("UIKit_LoadCSSFile: can't load '%s'", path);
    return NULL;
  }

  // Resolve core.StyleSheet.Parse and call it with the CSS text.
  lua_getglobal(L, "require");
  lua_pushstring(L, "orca.core");
  if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
    Con_Error("UIKit_LoadCSSFile: require orca.core failed: %s", lua_tostring(L, -1));
    lua_pop(L, 1);
    FS_FreeFile(fp);
    return NULL;
  }
  // stack: [core_table]
  lua_getfield(L, -1, "StyleSheet");
  lua_getfield(L, -1, "Parse");
  // stack: [core_table, StyleSheet_table, Parse_fn]
  lua_pushlstring(L, (const char*)fp->data, fp->size);
  FS_FreeFile(fp);

  if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
    Con_Error("UIKit_LoadCSSFile: CSS parse error in '%s': %s",
              path, lua_tostring(L, -1));
    lua_pop(L, 3);  // error, StyleSheet_table, core_table
    return NULL;
  }
  // stack: [core_table, StyleSheet_table, sheet_userdata]
  struct Object* sheet = luaX_checkObject(L, -1);
  lua_pop(L, 3);  // sheet, StyleSheet_table, core_table
  return sheet;
}

void on_ui_module_registered(lua_State* L) {
  s_css_lua = L;
  luaX_require(L, "orca.core", 0);
  lua_getglobal(L, "SERVER");
  is_server = lua_toboolean(L, -1);
  lua_pop(L, 1);
  SV_RegisterMessageProc(ui_handle_event);
  lua_pushcfunction(L, f_beginDraggingSession);
  lua_setfield(L, -2, "beginDraggingSession");
  OBJ_RegisterFileLoader(".css", UIKit_LoadCSSFile);
}

void
after_ui_module_registered(lua_State* L) {
  // Register UIKit table in package.loaded so TerminalView.lua can require it
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "loaded");
  lua_pushvalue(L, -3); // UIKit table
  lua_setfield(L, -2, "orca.UIKit");
  lua_pop(L, 2); // pop loaded, package
  // Load the CSS parser and register core.StyleSheet.Parse + package.searchers
  // entry for .css files.  Must run before TerminalView so that StyleSheet.Parse
  // is available to any Lua code loaded transitively from here.
  luaX_require(L, "orca.UIKit.file-css", 0);
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
