#include <plugins/UIKit/UIKit.h>
#include <include/api.h>

bool_t is_server = FALSE;

extern int f_beginDraggingSession(lua_State *L);
extern LRESULT ui_handle_event(lua_State* L, struct AXmessage *msg);

// Defined in css_parser.c (compiled into the same plugin).
extern lpObject_t CSS_ParseStyleSheet(const char* css_text);
extern int f_CSS_ParseStyleSheet(lua_State* L);

// Load a file and return a NUL-terminated heap buffer (caller must free).
// Returns NULL and logs an error on failure.
static char*
load_text_file(const char* path)
{
  struct file* fp = FS_LoadFile(path);
  if (!fp) return NULL;
  char* buf = (char*)malloc(fp->size + 1);
  if (buf) {
    memcpy(buf, fp->data, fp->size);
    buf[fp->size] = '\0';
  }
  FS_FreeFile(fp);
  return buf;
}

// C file loader for .css files, registered with OBJ_RegisterFileLoader.
// Reads the file and delegates to the pure-C CSS_ParseStyleSheet.
static struct Object*
UIKit_LoadCSSFile(const char* path)
{
  char* buf = load_text_file(path);
  if (!buf) {
    Con_Error("UIKit_LoadCSSFile: can't load '%s'", path);
    return NULL;
  }
  lpObject_t sheet = CSS_ParseStyleSheet(buf);
  free(buf);
  return sheet;
}

// Lua searcher for .css files.  Returns a loader closure that calls
// CSS_ParseStyleSheet on the file contents.
static int css_loader(lua_State* L)
{
  const char* path = luaL_checkstring(L, lua_upvalueindex(1));
  char* buf = load_text_file(path);
  if (!buf) return luaL_error(L, "UIKit: can't open '%s'", path);
  lpObject_t sheet = CSS_ParseStyleSheet(buf);
  free(buf);
  if (!sheet) return luaL_error(L, "UIKit: CSS parse failed for '%s'", path);
  luaX_pushObject(L, sheet);
  return 1;
}

extern bool_t FS_FileExists(const char* path);

static int css_searcher(lua_State* L)
{
  const char* module = luaL_checkstring(L, 1);
  path_t fullpath = {0};
  snprintf(fullpath, sizeof(fullpath), "%s.css", module);
  if (FS_FileExists(fullpath)) {
    lua_pushstring(L, fullpath);
    lua_pushcclosure(L, css_loader, 1);
    return 1;
  }
  return 0;
}

void on_ui_module_registered(lua_State* L) {
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
  // Register UIKit table in package.loaded so TerminalView.lua can require it.
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "loaded");
  lua_pushvalue(L, -3); // UIKit table
  lua_setfield(L, -2, "orca.UIKit");
  lua_pop(L, 2); // pop loaded, package

  // Attach core.StyleSheet.Parse as a C function so Lua code can do:
  //   local sheet = core.StyleSheet.Parse(css_string)
  if (luaX_require(L, "orca.core", 1) == LUA_OK) {
    lua_getfield(L, -1, "StyleSheet");
    if (lua_istable(L, -1)) {
      lua_pushcfunction(L, f_CSS_ParseStyleSheet);
      lua_setfield(L, -2, "Parse");
    }
    lua_pop(L, 2); // pop StyleSheet, core
  }

  // Add package.searchers entry for .css files.
  lua_getglobal(L, "table");
  if (lua_istable(L, -1)) {
    lua_getfield(L, -1, "insert");
    if (lua_isfunction(L, -1)) {
      lua_getglobal(L, "package");
      if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "searchers");
        lua_remove(L, -2); // pop "package", leave "searchers"
        if (lua_istable(L, -1)) {
          lua_pushcfunction(L, css_searcher);
          if (lua_pcall(L, 2, 0, 0) != LUA_OK)
            lua_pop(L, 1); // discard error
        } else {
          lua_pop(L, 2); // pop "searchers", "insert"
        }
      } else {
        lua_pop(L, 2); // pop "package", "insert"
      }
    } else {
      lua_pop(L, 1); // pop "insert"
    }
  }
  lua_pop(L, 1); // pop "table"

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
