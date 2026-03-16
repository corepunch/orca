#include <include/api.h>
#include "DarkReign.h"

/* forward declaration – defined in SprFormat.c, compiled as one unit */
static lpObject_t _SprFile_Load(uint8_t const *data, uint32_t size, lpcString_t name);

static int
f_load_spr(lua_State *L)
{
  lpcString_t module = luaL_checkstring(L, 1);
  path_t path = {0};
  snprintf(path, sizeof(path), "%s.spr", FS_PathFromModule(module));

  struct file *f = FS_LoadFile(path);
  if (!f)
    return 0;

  lpObject_t obj = _SprFile_Load(f->data, f->size, module);
  FS_FreeFile(f);

  if (!obj)
    return 0;

  FS_RegisterObject(obj, module);
  luaX_pushObject(L, obj);
  return 1;
}

void
on_darkreign_module_registered(lua_State *L)
{
  lua_register(L, "darkreign_loadspr", f_load_spr);
  if (luaL_dostring(L,
    "table.insert(package.searchers, function(path)\n"
    "  local ok, obj = pcall(darkreign_loadspr, path)\n"
    "  return ok and obj and function() return obj end or nil\n"
    "end)\n"
  ) != LUA_OK) {
    fprintf(stderr, "DarkReign: failed to register SPR package searcher: %s\n",
            lua_tostring(L, -1));
    lua_pop(L, 1);
  }
}
