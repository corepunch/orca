#include "property_internal.h"

bool_t
_AssignCallback(lua_State* L, lpProperty_t property)
{
  if (!L)
    return FALSE;
  lpObject_t object = property->object;
  static path_t str;
  sprintf(str, ON_CHANGED_CALLBACK, property->pdesc->Name);
  luaX_pushObject(L, object);
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    return FALSE;
  }
  lua_getfield(L, -1, str);
  if (lua_type(L, -1) == LUA_TFUNCTION) {
    property->flags |= PF_HASCHANGECALLBACK;
  }
  lua_pop(L, 2);
  return TRUE;
}

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

int luaX_readProperty(lua_State* L, int idx, lpProperty_t p)
{
//  int luatype = lua_type(L, idx);
//  float number;
//  void* udata = NULL;

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

int f_msgSend(lua_State *L);
void _pushproperty(lua_State* L,
                   void *value,
                   lpcPropertyType_t type)
{
  switch (type->DataType) {
    case kDataTypeBool:
      lua_pushboolean(L, *((bool_t*)value) != FALSE);
      break;
    case kDataTypeInt:
      lua_pushinteger(L, *((int*)value));
      break;
    case kDataTypeEnum:
      lua_pushstring(L, type->EnumValues[*((int*)value)]);
      break;
    case kDataTypeFloat:
      lua_pushnumber(L, *((float*)value));
      break;
    case kDataTypeStruct:
      memcpy(lua_newuserdata(L, type->DataSize), value, type->DataSize);
      luaL_setmetatable(L, type->TypeString);
      break;
    case kDataTypeColor:
      luaX_pushcolor(L, value);
      break;
    case kDataTypeString:
      lua_pushstring(L, *(lpcString_t*)value);
      break;
    case kDataTypeEvent:
      lua_pushfstring(L, "%s.%s", type->Category, type->Name);
      lua_pushcclosure(L, f_msgSend, 1);
      break;
    case kDataTypeNone:
      lua_pushnil(L);
      break;
    case kDataTypeObject: {
      lpObject_t object = *(lpObject_t*)value;
      if (strcmp(type->TypeString, "Object") && *(void**)value) {
        object = CMP_GetObject(*(void**)value);
      }
      luaX_pushObject(L, object);
      break;
    }
    default:
      fprintf(stderr, "push(): Unsupported property type %d\n", type->DataType);
      break;
  }
}

void luaX_pushProperty(lua_State* L, lpcProperty_t property)
{
  if (PROP_IsNull(property)) {
    lua_pushnil(L);
    return;
  }
  _pushproperty(L, property->value, property->pdesc);
}
