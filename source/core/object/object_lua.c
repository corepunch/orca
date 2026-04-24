#include <include/api.h>
#include <include/orca.h>

#include <source/core/core_local.h>

#define OBJECT_FUNCTION(NAME, FUNCTION)                                        \
  case f_##NAME:                                                               \
    lua_pushcfunction(L, API_Object##FUNCTION);                                \
    return 1;

// #define DEBUG_COUNT_OBJECTS

struct Object *luaX_checkObject(lua_State* L, int arg) {
  return *(struct Object **)luaL_checkudata(L, arg, API_TYPE_OBJECT);
}

void luaX_pushObject(lua_State* L, struct Object const *self)
{
  if (!self) {
    lua_pushnil(L);
    return;
  }
  struct Object ** ud = lua_newuserdata(L, sizeof(struct Object *));
  *ud = (struct Object *)self;
  luaL_setmetatable(L, API_TYPE_OBJECT);
}

static int
set_prop(lua_State* L)
{
  OBJ_SetProperty(L, luaX_checkObject(L, 1), luaL_checkstring(L, 2));
  return 0;
}

static void
_ParseArguments(lua_State* L, struct Object *hobj)
{
  if (lua_type(L, 2) == LUA_TSTRING) {
    lpcString_t arg = luaL_checkstring(L, 2);
    if (*arg == '#') {
      WITH(char, dup, strdup(++arg), free)
      {
        OBJ_SetName(hobj, strtok(dup, "."));
        while ((arg = strtok(NULL, "."))) {
          _SendMessage(hobj, StyleController, AddClass, arg);
        }
        lua_remove(L, 2);
      }
    } else if (*arg == '.') {
      WITH(char, dup, strdup(++arg), free)
      {
        _SendMessage(hobj, StyleController, AddClass, strtok(dup, "."));
        while ((arg = strtok(NULL, "."))) {
          _SendMessage(hobj, StyleController, AddClass, arg);
        }
        lua_remove(L, 2);
      }
    }
  }
  if (lua_type(L, 2) == LUA_TTABLE) {
    lua_pushnil(L);
    while (lua_next(L, 2)) {
      luaL_checktype(L, -2, LUA_TSTRING);
      int key_idx = lua_absindex(L, -2);
      int val_idx = lua_absindex(L, -1);
      lua_pushcfunction(L, set_prop);
      luaX_pushObject(L, hobj);
      lua_pushvalue(L, key_idx);
      lua_pushvalue(L, val_idx);
      lua_call(L, 3, 0);
      lua_pop(L, 1); // pop value, keep key for next lua_next
    }
    lua_remove(L, 2);
    _SendMessage(hobj, Object, Start);
  }
  if (lua_type(L, 2) == LUA_TFUNCTION) {
    OBJ_Rebuild(L, hobj);
    lua_remove(L, 2);
  }
  if (lua_type(L, 2) == LUA_TSTRING) {
    OBJ_SetTextContent(hobj, luaL_checkstring(L, 2));
    lua_remove(L, 2);
  }
  if (lua_isnil(L, 2)) {
    lua_remove(L, 2);
    return;
  }
  if (lua_gettop(L) > 1) {
    Con_Error("Can't parse argument of type %d, stack size %d",
               lua_type(L, 2),
               lua_gettop(L));
  }
}

int OBJ_CreateFromLuaState(lua_State *L) {
  luaX_parsefield(struct ClassDesc const *, __nativeclass, 1, lua_touserdata);
  struct Object *pobj = OBJ_Create(__nativeclass->ClassID);

  struct Object ** ctx = lua_getextraspace(L);
  if (*ctx) {
    OBJ_AddChild(*ctx, pobj, FALSE);
  }

  // send "create" message
  OBJ_SendMessageW(pobj, ID_Object_Create, 0, L);

  _ParseArguments(L, pobj);

  // apply class-default style
  lua_getfield(L, 1, "apply");
  if (lua_type(L, -1) == LUA_TFUNCTION) {
    luaX_pushObject(L, pobj);
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
      Con_Error("Can't execute apply()");
    } else {
      _SendMessage(pobj, StyleController, AddClasses, luaL_checkstring(L, -1));
    }
  }
  lua_pop(L, 1);

  luaX_pushObject(L, pobj);

  return 1;
}

bool_t OBJ_Equals(struct Object const *self, struct Object const *other)
{
  return self == other;
}

void OBJ_SetContext(lua_State* L, struct Object *self)
{
  struct Object ** ctx = lua_getextraspace(L);
  *ctx = self;
}

#define ID_Node_ViewDidLoad 0x71bab7e1 // Node.ViewDidLoad
static int f_rebuild_finalize(lua_State *L, int status, lua_KContext ctx) {
  if (status != LUA_OK) {
    return luaL_error(L, luaL_tolstring(L, -1, NULL));
  }
  axPostMessageW((struct Object *)ctx, ID_Node_ViewDidLoad, 0, NULL);
  return 0;
}

static int f_rebuild(lua_State *L) {
  struct Object *self = luaX_checkObject(L, 1);
  if (lua_type(L, 2) == LUA_TFUNCTION) {
    OBJ_Clear(self);
    lua_pushvalue(L, 2);  /* fn */
    lua_pushvalue(L, 1);  /* self */
    return lua_pcallk(L, 1, 0, 0, (lua_KContext)self, f_rebuild_finalize);
  } else if (lua_type(L, 2) == LUA_TSTRING) {
    OBJ_SetTextContent(self, luaL_checkstring(L, 2));
  }
  axPostMessageW(self, ID_Node_ViewDidLoad, 0, NULL);
  return 0;
}

void OBJ_Rebuild(lua_State* L, struct Object *self) {
  lua_State* co = lua_newthread(L);
  *((struct Object **)lua_getextraspace(co)) = self;
  int ref = luaL_ref(L, LUA_REGISTRYINDEX);
  lua_pushcfunction(co, f_rebuild);
  luaX_pushObject(co, self);
  int nargs = 1;
  if (!lua_isnoneornil(L, 2)) {
    lua_pushvalue(L, 2);   /* copy fn/string to top of L */
    lua_xmove(L, co, 1);   /* move to co's stack as arg 2 */
    nargs = 2;
  }
  axPostMessageW(co, kEventResumeCoroutine, MAKEDWORD(nargs, ref), NULL);
}

struct Object *OBJ_Instantiate(lua_State* L, struct Object *prefab) {
  assert(!"Not implemented");
  return NULL;
}

ORCA_API struct Property *
luaX_getobjectcallback(lua_State* L, struct Object *object, uint32_t id)
{
  struct Property *event = PROP_FindByLongID(OBJ_GetProperties(object), id);
  if (event && PROP_GetType(event) == kDataTypeEvent) {
    lua_geti(L, LUA_REGISTRYINDEX, *(event_t*)PROP_GetValue(event));
    return event;
  }
  return NULL;
}
