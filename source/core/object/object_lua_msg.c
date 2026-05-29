#include <include/api.h>
#include <include/orca.h>

#include <source/core/core_local.h>

extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* valueptr);

static bool_t
_LuaMessagePayload(lua_State *L,
                   lpcString_t message,
                   int args_index,
                   char payload[MAX_MESSAGE_SIZE],
                   size_t *payload_size)
{
  uint32_t count = 0;
  struct PropertyType const *fields = OBJ_FindMessagePropertyTypes(message, &count);
  if (!fields || count == 0) {
    *payload_size = 0;
    return FALSE;
  }

  memset(payload, 0, MAX_MESSAGE_SIZE);
  *payload_size = 0;
  bool_t use_table = lua_type(L, args_index) == LUA_TTABLE;
  int table_idx = use_table ? lua_absindex(L, args_index) : 0;

  FOR_LOOP(i, count) {
    struct PropertyType const *field = &fields[i];
    size_t field_end = field->Offset + field->DataSize;
    if (field->Offset >= MAX_MESSAGE_SIZE || field_end > MAX_MESSAGE_SIZE) {
      Con_Error("Message field '%s' exceeds payload capacity for '%s'",
                field->Name ? field->Name : "<unnamed>",
                message);
      continue;
    }
    if (field_end > *payload_size) {
      *payload_size = field_end;
    }
    if (use_table) {
      lua_getfield(L, table_idx, field->Name);
      read_property(L, -1, field, payload + field->Offset);
      lua_pop(L, 1);
    } else if (lua_type(L, args_index + (int)i) != LUA_TNONE) {
      read_property(L, args_index + (int)i, field, payload + field->Offset);
    }
  }
  return TRUE;
}

void OBJ_post(lua_State* L, struct Object *self, lpcString_t message)
{
  char payload[MAX_MESSAGE_SIZE];
  size_t payload_size = 0;

  if (_LuaMessagePayload(L, message, 3, payload, &payload_size)) {
    SV_PostMessageData(self, message, 0, payload, payload_size);
  } else {
    SV_PostMessage(self, message, 0, NULL);
  }
}

int OBJ_send(lua_State* L, struct Object *self, lpcString_t message)
{
  char payload[MAX_MESSAGE_SIZE];
  size_t payload_size = 0;
  LRESULT result;
  if (_LuaMessagePayload(L, message, 3, payload, &payload_size)) {
    result = OBJ_SendMessage(self, message, 0, payload);
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
