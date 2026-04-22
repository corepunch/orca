#include <include/api.h>
#include <include/orca.h>

#include <source/core/core_local.h>

#define OBJECT_FUNCTION(NAME, FUNCTION)                                        \
  case f_##NAME:                                                               \
    lua_pushcfunction(L, API_Object##FUNCTION);                                \
    return 1;

// #define DEBUG_COUNT_OBJECTS

static void get_object_extras(lua_State* L, lpObject_t obj);

lpObject_t luaX_checkObject(lua_State* L, int arg) {
  return *(lpObject_t*)luaL_checkudata(L, arg, API_TYPE_OBJECT);
}

void luaX_pushObject(lua_State* L, lpcObject_t self)
{
  if (!self) {
    lua_pushnil(L);
    return;
  }
  lpObject_t* ud = lua_newuserdata(L, sizeof(lpObject_t));
  *ud = (lpObject_t)self;
  luaL_setmetatable(L, API_TYPE_OBJECT);
}

static int
set_prop(lua_State* L)
{
  OBJ_SetProperty(L, luaX_checkObject(L, 1), luaL_checkstring(L, 2));
  return 0;
}

static void
_ParseArguments(lua_State* L, lpObject_t hobj)
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
    // Store body in per-object extras table for rebuild()
    get_object_extras(L, hobj);
    lua_pushvalue(L, 2);
    lua_setfield(L, -2, "body");
    lua_pop(L, 1);
    lua_remove(L, 2);
    OBJ_SetFlags(hobj, OBJ_GetFlags(hobj) | OF_CLEARBODY);
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
  luaX_parsefield(lpcClassDesc_t, __nativeclass, 1, lua_touserdata);
  lpObject_t pobj = OBJ_Create(L, __nativeclass);

  lpObject_t* ctx = lua_getextraspace(L);
  if (*ctx) {
    OBJ_AddChild(*ctx, pobj, FALSE);
  }

  // send "create" message
  OBJ_SendMessageW(pobj, ID_Object_Create, 0, L);

  _ParseArguments(L, pobj);

  /* Store the class table in per-object extras for Lua method lookup */
  get_object_extras(L, pobj);
  lua_pushvalue(L, 1); /* arg 1 = class table */
  lua_setfield(L, -2, "__class");
  lua_pop(L, 1);

  /* If no body was provided explicitly, check if the class defines a __body */
  if (!(OBJ_GetFlags(pobj) & OF_CLEARBODY)) {
    lua_getfield(L, 1, "__body");
    if (lua_type(L, -1) == LUA_TFUNCTION) {
      get_object_extras(L, pobj);
      lua_pushvalue(L, -2);
      lua_setfield(L, -2, "body");
      lua_pop(L, 2);
      OBJ_SetFlags(pobj, OBJ_GetFlags(pobj) | OF_CLEARBODY);
    } else {
      lua_pop(L, 1);
    }
  }

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

  /* If a body function was set during _ParseArguments, schedule rebuild. */
  if (OBJ_GetFlags(pobj) & OF_CLEARBODY) {
    void OBJ_Rebuild(lua_State*, lpObject_t);
    OBJ_Rebuild(L, pobj);
  }

  return 1;
}

bool_t OBJ_Equals(lpcObject_t self, lpcObject_t other)
{
  return self == other;
}

void OBJ_SetContext(lua_State* L, lpObject_t self)
{
  lpObject_t* ctx = lua_getextraspace(L);
  *ctx = self;
}

/* Per-object Lua extras table, stored in registry under "__object_extras".
 * The table uses weak values so entries are collected when unused.
 * Returns the extras table for `obj` on top of the stack. */
static void
get_object_extras(lua_State* L, lpObject_t obj)
{
  lua_getfield(L, LUA_REGISTRYINDEX, "__object_extras");
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    lua_newtable(L);  /* strong-value table — cleaned up in OBJ_Release */
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "__object_extras");
  }
  lua_pushlightuserdata(L, obj);
  lua_gettable(L, -2);
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushlightuserdata(L, obj);
    lua_pushvalue(L, -2);
    lua_settable(L, -4);
  }
  lua_remove(L, -2); /* remove __object_extras, leave extras table on top */
}

/* Public alias used by core_export.c */
void get_object_extras_pub(lua_State* L, lpObject_t obj) { get_object_extras(L, obj); }

#define ID_Node_ViewDidLoad 0x71bab7e1 // Node.ViewDidLoad
static int f_rebuild_finalize(lua_State *L, int status, lua_KContext ctx) {
  lpObject_t self = (lpObject_t)ctx;
  if (status != LUA_OK) {
    return luaL_error(L, luaL_tolstring(L, -1, NULL));
  }
  if (OBJ_GetFlags(self) & OF_CLEARBODY) {
    get_object_extras(L, self);
    lua_pushnil(L);
    lua_setfield(L, -2, "body");
    lua_pop(L, 1);
    OBJ_SetFlags(self, OBJ_GetFlags(self) & ~OF_CLEARBODY);
  }
  axPostMessageW(self, ID_Node_ViewDidLoad, 0, NULL);
  return 0;
}

static int f_rebuild(lua_State *L) {
  lpObject_t self = luaX_checkObject(L, 1);
  if (!lua_isnoneornil(L, 2)) {
    get_object_extras(L, self);
    lua_pushvalue(L, 2);
    lua_setfield(L, -2, "body");
    lua_pop(L, 1);
  }
  get_object_extras(L, self);
  lua_getfield(L, -1, "body");
  lua_remove(L, -2);
  if (lua_type(L, -1) == LUA_TFUNCTION) {
    OBJ_Clear(L, self);
    lua_pushvalue(L, 1);
    return lua_pcallk(L, 1, 0, 0, (lua_KContext)self, f_rebuild_finalize);
  } else if (lua_type(L, -1) == LUA_TSTRING) {
    OBJ_SetTextContent(self, luaL_checkstring(L, -1));
  }
  lua_pop(L, 1);
  axPostMessageW(self, ID_Node_ViewDidLoad, 0, NULL);
  return 0;
}

void OBJ_Rebuild(lua_State* L, lpObject_t self) {
  lua_State* co = lua_newthread(L);
  *((lpObject_t *)lua_getextraspace(co)) = self;
  int ref = luaL_ref(L, LUA_REGISTRYINDEX);
  lua_pushcfunction(co, f_rebuild);
  luaX_pushObject(co, self);
  axPostMessageW(co, kEventResumeCoroutine, MAKEDWORD(1, ref), NULL);
}

lpObject_t OBJ_Instantiate(lua_State* L, lpObject_t prefab) {
  assert(!"Not implemented");
  return NULL;
}

ORCA_API lpProperty_t
luaX_getobjectcallback(lua_State* L, lpObject_t object, uint32_t id)
{
  lpProperty_t event = PROP_FindByLongID(OBJ_GetProperties(object), id);
  if (event && PROP_GetType(event) == kDataTypeEvent) {
    lua_geti(L, LUA_REGISTRYINDEX, *(event_t*)PROP_GetValue(event));
    return event;
  }
  return NULL;
}
