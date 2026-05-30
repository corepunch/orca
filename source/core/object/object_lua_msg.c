#include <include/api.h>
#include <include/orca.h>

#include <source/core/core_local.h>

extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* valueptr);

static uint32_t
_MessagePayloadPropertyCount(struct ClassDesc const *cls)
{
  if (!cls || cls->NumProperties < 2) {
    return 0;
  }
  return cls->NumProperties - 2;
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
  uint32_t count = _MessagePayloadPropertyCount(cls);
  FOR_LOOP(i, count) {
    struct PropertyType const *field = &cls->Properties[i];
    void *value = (char *)component + field->Offset;
    if (field->IsArray) {
      free(*(void **)value);
      *(void **)value = NULL;
      continue;
    }
    if (field->DataType == kDataTypeString && field->DataSize == sizeof(char *)) {
      free(*(char **)value);
      *(char **)value = NULL;
    }
  }
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
        read_property(L, -1, field, value);
      }
      lua_pop(L, 1);
    } else if (lua_type(L, args_index + (int)i) != LUA_TNONE) {
      read_property(L, args_index + (int)i, field, value);
    }
  }
  return TRUE;
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

  struct PropertyType const *target_field = cls->NumProperties >= 2
    ? &cls->Properties[cls->NumProperties - 2]
    : NULL;
  struct PropertyType const *mode_field = cls->NumProperties >= 1
    ? &cls->Properties[cls->NumProperties - 1]
    : NULL;

  if (target_field) {
    struct Property *target_prop = NULL;
    lpcString_t direct_target = ".";
    if (SUCCEEDED(OBJ_FindLongProperty(action, target_field->FullIdentifier, &target_prop)) && target_prop) {
      PROP_SetValue(target_prop, &direct_target);
    }
  }
  if (post && mode_field) {
    struct Property *mode_prop = NULL;
    enum DispatchMode mode = kDispatchModePost;
    if (SUCCEEDED(OBJ_FindLongProperty(action, mode_field->FullIdentifier, &mode_prop)) && mode_prop) {
      PROP_SetValue(mode_prop, &mode);
    }
  }

  if (!_PopulateActionFromLua(L, action, cls, 3)) {
    _ClearActionPayload(action, cls);
    OBJ_ReleaseRef(action);
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
