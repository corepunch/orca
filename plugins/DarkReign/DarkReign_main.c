#include <include/api.h>
#include <plugins/SpriteKit/SpriteKit.h>
#include "DarkReign.h"

/* forward declaration – defined in SprFormat.c, compiled as one unit */
static lpObject_t _SprFile_Load(lua_State* L, uint8_t const *data, uint32_t size, lpcString_t name);

static int
f_load_spr(lua_State *L)
{
  lpcString_t module = luaL_checkstring(L, 1);
  path_t path = {0};
  snprintf(path, sizeof(path), "%s.spr", FS_PathFromModule(module));

  struct file *f = FS_LoadFile(path);
  if (!f)
    return 0;

  lpObject_t obj = _SprFile_Load(L, f->data, f->size, module);
  FS_FreeFile(f);

  if (!obj) {
    fprintf(stderr, "DarkReign: failed to parse SPR file '%s'\n", path);
    return 0;
  }

  fprintf(stderr, "DarkReign: loaded SPR file '%s' as object '%s'\n", path, module);

  if (!GetSpriteAnimation(obj)) {
    fprintf(stderr, "DarkReign: SpriteAnimation class not found, cannot load '%s'\n", path);
    return 0;
  }

  FS_RegisterObject(obj, module);
  luaX_pushObject(L, obj);
  return 1;
}

void
on_darkreign_module_registered(lua_State *L)
{
  fprintf(stderr, "DarkReign: module registered, setting up SPR loader\n");
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
