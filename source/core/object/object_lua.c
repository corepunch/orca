#include <include/api.h>
#include <include/orca.h>

#include <source/core/core_local.h>

#define OBJECT_FUNCTION(NAME, FUNCTION)                                        \
  case f_##NAME:                                                               \
    lua_pushcfunction(L, API_Object##FUNCTION);                                \
    return 1;

// #define DEBUG_COUNT_OBJECTS

lpObject_t luaX_checkObject(lua_State* L, int arg) {
  if (lua_type(L, arg) == LUA_TTABLE) {
    luaX_parsefield(lpObject_t, __userdata, arg, luaL_checkudata, API_TYPE_OBJECT);
    return __userdata;
  } else {
    return luaL_checkudata(L, arg, API_TYPE_OBJECT);
  }
}

void luaX_pushObject(lua_State* L, lpcObject_t self)
{
  if (self && OBJ_GetLuaObject(self)) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, OBJ_GetLuaObject(self));
    assert(lua_type(L, -1)==LUA_TTABLE);
  } else {
    lua_pushnil(L);
  }
}

static void
_parse_args(lua_State* L, lpObject_t hobj)
{
  if (lua_type(L, 2) == LUA_TSTRING) {
    lpcString_t arg = luaL_checkstring(L, 2);
    if (*arg == '#') {
      WITH(char, dup, strdup(++arg), free)
      {
        OBJ_SetName(hobj, strtok(dup, "."));
        while ((arg = strtok(NULL, ".")))
          OBJ_AddClass(hobj, arg);
        lua_remove(L, 2);
      }
    } else if (*arg == '.') {
      WITH(char, dup, strdup(++arg), free)
      {
        OBJ_AddClass(hobj, strtok(dup, "."));
        while ((arg = strtok(NULL, ".")))
          OBJ_AddClass(hobj, arg);
        lua_remove(L, 2);
      }
    }
  }
  if (lua_type(L, 2) == LUA_TTABLE) {
    lua_pushnil(L);
    while (lua_next(L, 2)) {
      shortStr_t _key;
      strncpy(_key, luaL_checkstring(L, -2), sizeof(_key));
      lua_settable(L, 1);
      lua_pushstring(L, _key);
    }
    lua_remove(L, 2);
    
    _SendMessage(hobj, Start);
  }
  if (lua_type(L, 2) == LUA_TFUNCTION) {
    lua_pushstring(L, "body");
    lua_pushvalue(L, 2);
    lua_settable(L, 1);
    lua_remove(L, 2);

    // TODO: a cleaner way? this clears "body" after populating once
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

static void
_assign_callbacks(lua_State* L, lpObject_t pobj, int idx)
{
  luaX_pushObject(L, pobj);
  int obj_index = lua_gettop(L);
  lua_pushvalue(L, idx);
  while (lua_type(L, -1) != LUA_TNIL) {
    lua_pushnil(L);
    while (lua_next(L, -2)) {
      shortStr_t _key;
      strncpy(_key, luaL_checkstring(L, -2), sizeof(_key));
      if (strncmp(_key, "__", 2)) {
        lua_settable(L, obj_index);
        lua_pushstring(L, _key);
      } else {
        lua_pop(L, 1);
      }
    }
    if (!lua_getmetatable(L, -1)) {
      break;
    }
    lua_remove(L, -2);
  }
  lua_pop(L, 2);
}

int OBJ_CreateFromLuaState(lua_State *L) {
  luaX_parsefield(lpcClassDesc_t, __nativeclass, 1, lua_touserdata);
  lpObject_t pobj = OBJ_Create(L, __nativeclass);
  if (lua_getfield(L, 1, "__class") == LUA_TTABLE) {
    luaX_parsefield(lpcString_t, __name, -1, luaL_optstring, NULL);
    if (__name && strcmp(__name, "LuaBehaviour")) {
      OBJ_SetName(pobj, __name);
      OBJ_SetClassName(pobj, __name);
    }
  }
  lua_pop(L, 1);
  
  lua_setfield(L, 1, "__userdata");
  
  // register object in registry
  lua_pushvalue(L, 1);
  OBJ_SetLuaObject(pobj, luaL_ref(L, LUA_REGISTRYINDEX));

  lpObject_t* ctx = lua_getextraspace(L);
  if (*ctx) {
    OBJ_AddChild(*ctx, pobj, FALSE);
  }

  // send "create" message
  OBJ_SendMessageW(pobj, ID_Create, 0, L);
  
_assign_callbacks(L, pobj, 1);
  _parse_args(L, pobj);

  // TODO: is there a better way to add class-default style?
  lua_getfield(L, 1, "apply");
  if (lua_type(L, -1) == LUA_TFUNCTION) {
    lua_pushvalue(L, 1);
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
      Con_Error("Can't execute apply()");
    } else {
      OBJ_ParseClassAttribute(pobj, luaL_checkstring(L, -1));
    }
  }
  lua_pop(L, 1);
  luaX_pushObject(L, pobj);
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

void OBJ_Play(lpObject_t self, lpcString_t animation)
{
//  OBJ_SetAnimation(self, string_2);
}

static int f_rebuild_finalize(lua_State *L, int status, lua_KContext ctx) {
  lpObject_t self = (lpObject_t)ctx;
  if (status != LUA_OK) {
    return luaL_error(L, luaL_tolstring(L, -1, NULL));
  }
  if (OBJ_GetFlags(self) & OF_CLEARBODY) {
    lua_pushnil(L);
    lua_setfield(L, 1, "body");
    OBJ_SetFlags(self, OBJ_GetFlags(self) & ~OF_CLEARBODY);
  }
  WI_PostMessageW(self, ID_ViewDidLoad, 0, NULL);
  return 0;
}

static int f_rebuild(lua_State *L) {
  lpObject_t self = luaX_checkObject(L, 1);
  if (!lua_isnoneornil(L, 2)) { // set body
    lua_pushvalue(L, 2);
    lua_setfield(L, 1, "body");
  }
  lua_getfield(L, 1, "body");
  if (lua_type(L, -1) == LUA_TFUNCTION) {
    OBJ_Clear(L, self);
    lua_pushvalue(L, 1);
    return lua_pcallk(L, 1, 0, 0, (lua_KContext)self, f_rebuild_finalize);
  } else if (lua_type(L, -1) == LUA_TSTRING) {
    OBJ_SetTextContent(self, luaL_checkstring(L, -1));
  }
  lua_pop(L, 1);
  WI_PostMessageW(self, ID_ViewDidLoad, 0, NULL); // TODO: replace with direct call to avoid unnecessary message dispatch
  return 0;
}

void OBJ_Rebuild(lua_State* L, lpObject_t self) {
  const int nargs = lua_gettop(L);
  lua_State* co = lua_newthread(L);
  *((lpObject_t *)lua_getextraspace(co)) = luaX_checkObject(L, 1);
  int ref = luaL_ref(L, LUA_REGISTRYINDEX);
  lua_pushcfunction(co, f_rebuild);
  lua_xmove(L, co, nargs);
  WI_PostMessageW(co, ID_ResumeCoroutine, MAKEDWORD(nargs, ref), NULL);
}

lpObject_t OBJ_Instantiate(lua_State* L, lpObject_t prefab) {
  assert(!"Not implemented");
  return NULL;
}
