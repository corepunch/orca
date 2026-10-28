#include <include/api.h>
#include <include/orca.h>

#include <source/core/core_local.h>

static uint32_t
_MessagePayloadPropertyCount(struct ClassDesc const *cls)
{
  if (!cls) {
    return 0;
  }
  return cls->NumProperties;
}

static void
_ClearActionPayload(struct Object *action, struct ClassDesc const *cls)
{
  if (!action || !cls) {
    return;
  }
  void *component = OBJ_GetComponent(action, cls->ClassID);
  if (!component) {
    return;
  }
  // Clear only reflected payload fields; component headers belong to the action.
  struct StructDesc payload = {
    .Properties = cls->Properties,
    .NumProperties = _MessagePayloadPropertyCount(cls),
  };
  core_ClearStructPayload(&payload, component);
}

static bool_t
_PopulateActionFromLua(lua_State *L,
                       struct Object *action,
                       struct ClassDesc const *cls,
                       int args_index)
{
  uint32_t count = _MessagePayloadPropertyCount(cls);
  void *component = OBJ_GetComponent(action, cls->ClassID);
  if (!component) {
    return FALSE;
  }
  bool_t use_table = lua_type(L, args_index) == LUA_TTABLE;
  int table_idx = use_table ? lua_absindex(L, args_index) : 0;

  FOR_LOOP(i, count) {
    struct PropertyType const *field = &cls->Properties[i];
    void *value = (char *)component + field->Offset;
    if (use_table) {
      lua_getfield(L, table_idx, field->Name);
      if (!lua_isnil(L, -1)) {
        luaX_readStructProperty(L, -1, field, value);
      }
      lua_pop(L, 1);
    } else if (lua_type(L, args_index + (int)i) != LUA_TNONE) {
      luaX_readStructProperty(L, args_index + (int)i, field, value);
    }
  }
  return TRUE;
}

static int
_PopulateActionProtected(lua_State *L)
{
  struct Object *action = lua_touserdata(L, lua_upvalueindex(1));
  struct ClassDesc const *cls = lua_touserdata(L, lua_upvalueindex(2));
  if (!_PopulateActionFromLua(L, action, cls, 1))
    return luaL_error(L, "Cannot populate message %s", cls->ClassName);
  return 0;
}

static struct Object *
_CreateMessageAction(lua_State *L,
                     lpcString_t message,
                     bool_t post)
{
  struct ClassDesc const *cls = OBJ_FindClass(message);
  if (!cls) {
    return NULL;
  }

  struct Object *action = OBJ_Create(cls->ClassID);
  if (!action) {
    return NULL;
  }

  lpcString_t direct_target = ".";
  struct Property *target_prop = OBJ_FindLongProperty(action, ID_SendMessageAction_Target);
  if (target_prop) {
    PROP_SetValue(target_prop, &direct_target);
  }

  if (post) {
    struct Property *mode_prop = OBJ_FindLongProperty(action, ID_SendMessageAction_Mode);
    enum DispatchMode mode = kDispatchModePost;
    if (mode_prop) {
      PROP_SetValue(mode_prop, &mode);
    }
  }

  int arguments = lua_gettop(L);
  lua_pushlightuserdata(L, action);
  lua_pushlightuserdata(L, (void *)cls);
  lua_pushcclosure(L, _PopulateActionProtected, 2);
  for (int i = 3; i <= arguments; i++) lua_pushvalue(L, i);
  if (lua_pcall(L, arguments > 2 ? arguments - 2 : 0, 0, 0) != LUA_OK) {
    _ClearActionPayload(action, cls);
    OBJ_ReleaseRef(action);
    lua_error(L);
    return NULL;
  }

  return action;
}

void OBJ_post(lua_State* L, struct Object *self, lpcString_t message)
{
  struct Object *action = _CreateMessageAction(L, message, TRUE);
  if (!action) {
    SV_PostMessage(self, message, 0, NULL);
    return;
  }
  _SendMessage(action, Action, Dispatch, .Sender = self);
  _ClearActionPayload(action, OBJ_FindClass(message));
  OBJ_ReleaseRef(action);
}

int OBJ_send(lua_State* L, struct Object *self, lpcString_t message)
{
  struct Object *action = _CreateMessageAction(L, message, FALSE);
  LRESULT result;
  if (action) {
    result = _SendMessage(action, Action, Dispatch, .Sender = self);
    _ClearActionPayload(action, OBJ_FindClass(message));
    OBJ_ReleaseRef(action);
  } else {
    result = OBJ_SendMessage(self, message, 0, NULL);
  }
  if (result) {
    lua_pushinteger(L, (intptr_t)result);
  } else {
    lua_pushnil(L);
  }
  return 1;
}
