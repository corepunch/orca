#include <include/api.h>
#include <plugins/SpriteKit/SpriteKit.h>
#include "DarkReign.h"

/* forward declaration – defined in SprFormat.c, compiled as one unit */
lpObject_t _SprFile_Load(lua_State* L, uint8_t const *data, uint32_t size, lpcString_t name);

/*
 * Level 2 – LOADER closure.
 * Called by Lua's require machinery after the searcher confirms the file
 * exists. Loads and parses the .spr, registers the object, then returns
 * the CLASS closure (f_spr_class) as the result of require().
 * Upvalue 1: the module name string.
 */
static int
f_do_load_spr(lua_State *L)
{
  lpcString_t module = lua_tostring(L, lua_upvalueindex(1));
  path_t path = {0};
  snprintf(path, sizeof(path), "%s.spr", FS_PathFromModule(module));

  struct file *f = FS_LoadFile(path);
  if (!f)
    return luaL_error(L, "SPR file not found: %s", path);

  lpObject_t obj = _SprFile_Load(L, f->data, f->size, module);
  FS_FreeFile(f);

  if (!obj)
    return luaL_error(L, "Failed to parse SPR file: %s", path);

  if (!GetSpriteAnimation(obj))
    return luaL_error(L, "SpriteAnimation class not found, cannot load '%s'", path);

  FS_RegisterObject(obj, module);
  luaX_pushObject(L, obj);
  return 1;
}

/*
 * Level 1 – SEARCHER.
 * Registered in package.searchers. Checks whether a .spr file exists for
 * the given module path and returns the LOADER closure, or nil to let other
 * searchers try.
 */
static int
f_load_spr(lua_State *L)
{
  lpcString_t module = luaL_checkstring(L, 1);
  path_t path = {0};
  snprintf(path, sizeof(path), "%s.spr", FS_PathFromModule(module));

  if (!FS_FileExists(path))
    return 0;

  lua_pushvalue(L, 1);                      /* module name as upvalue for LOADER */
  lua_pushcclosure(L, f_do_load_spr, 1);
  return 1;
}

void
on_darkreign_module_registered(lua_State *L)
{
  lua_register(L, "darkreign_loadspr", f_load_spr);
  if (luaL_dostring(L, "table.insert(package.searchers, darkreign_loadspr)\n") != LUA_OK) {
    fprintf(stderr, "DarkReign: failed to register SPR package searcher: %s\n",
            lua_tostring(L, -1));
    lua_pop(L, 1);
  }
}
