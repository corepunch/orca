#include "property_internal.h"

static int read_array(lua_State *L, int idx, lpProperty_t p) {
  if (lua_type(L, (idx = lua_absindex(L, idx))) != LUA_TTABLE) {
    return luaL_error(L, "Expected a table for array property %s", p->pdesc->Name);
  }
  size_t numitems = lua_rawlen(L, idx);
  void *mem = lua_newuserdatauv(L, p->pdesc->DataSize * numitems, 0);
  ((int*)p->value)[sizeof(void*)/sizeof(int)] = (int)numitems;
  lua_pushnil(L);
  for (uint32_t i = 0; lua_next(L, idx) != 0; i++, lua_pop(L, 1)) {
    if (lua_type(L, -2) != LUA_TNUMBER) {
      return luaL_error(L, "Expected numeric keys in array table for property %s", p->pdesc->Name);
    }
    switch (lua_type(L, -1)) {
      case LUA_TUSERDATA:
        memcpy((char*)mem + i * p->pdesc->DataSize, luaL_checkudata(L, -1, p->pdesc->TypeString), p->pdesc->DataSize);
        break;
      case LUA_TTABLE:
        if (luaL_getmetatable(L, p->pdesc->TypeString)) {
          lua_pushvalue(L, -2);
          if (lua_pcall(L, 1, 1, 0)) {
            return luaL_error(L, "read_array(): %s", luaL_checkstring(L, -1));
          }
          memcpy((char*)mem + i * p->pdesc->DataSize, luaL_checkudata(L, -1, p->pdesc->TypeString), p->pdesc->DataSize);
          lua_pop(L, 1);
          break;
        } else {
          return luaL_error(L, "Expected userdata of type %s in array table for property %s", p->pdesc->TypeString, p->pdesc->Name);
        }
      case LUA_TNUMBER:
        switch (p->pdesc->DataType) {
          case kDataTypeFloat:
            ((float*)mem)[i] = luaL_checknumber(L, -1);
            break;
          case kDataTypeInt:
          case kDataTypeEnum:
            ((int*)mem)[i] = (int)luaL_checkinteger(L, -1);
            break;
          default:
            return luaL_error(L, "Unsupported data type in array table for property %s", p->pdesc->Name);
        }
        break;
      default:
        return luaL_error(L, "Unsupported value type %d in array table for property %s", lua_type(L, -1), p->pdesc->Name);
    }
  }
  void *tmp = malloc(p->pdesc->DataSize * numitems);
  memcpy(tmp, mem, p->pdesc->DataSize * numitems);
  PROP_SetValue(p, &tmp);
  return 0;
}

extern void read_property(lua_State *, int, struct PropertyType const*, void*);
extern int write_property(lua_State *L, struct PropertyType const* prop, void const* valueptr);

int luaX_readProperty(lua_State* L, int idx, lpProperty_t p)
{
  if (lua_isnil(L, idx)) {
    PROP_Clear(p);
    return 0;
  }

  // handle array properties
  if (p->pdesc && p->pdesc->IsArray) {
    return read_array(L, idx, p);
  }
  
  char buf[MAX_PROPERTY_STRING]={0};
  read_property(L, idx, p->pdesc, buf);
  PROP_SetValue(p, buf);

  return 0;
}

void luaX_pushProperty(lua_State* L, lpcProperty_t property)
{
  if (PROP_IsNull(property)) {
    lua_pushnil(L);
    return;
  }
  write_property(L, property->pdesc, property->value);
}

bool_t
PROP_RegisterChangedCallback(lua_State* L, lpProperty_t property, int callback_idx)
{
  PROP_SetFlag(property, PF_HASCHANGECALLBACK);
  if (property->changeCallback) {
    luaL_unref(L, LUA_REGISTRYINDEX, property->changeCallback);
  }
  if (lua_isfunction(L, callback_idx)) {
    lua_pushvalue(L, callback_idx);
    property->changeCallback = (event_t)luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (lua_isnil(L, callback_idx)) {
     property->changeCallback = 0;
  } else {
    return FALSE;
  }
  return TRUE;
}

