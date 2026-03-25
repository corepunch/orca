#include "property_internal.h"

bool_t
_AssignCallback(lua_State* L, lpProperty_t property)
{
  if (!L)
    return FALSE;
  lpObject_t object = property->object;
  static path_t str;
  sprintf(str, ON_CHANGED_CALLBACK, property->pdesc->Name);
  lua_geti(L, LUA_REGISTRYINDEX, OBJ_GetLuaObject(object));
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
  if (lua_type(L, idx) != LUA_TTABLE) {
    Con_Error("Expected a table for array property %s", p->pdesc->Name);
    return 0;
  }
  size_t numitems = lua_rawlen(L, idx);
  void *mem = malloc(p->pdesc->DataSize * numitems);
  ((int*)p->value)[sizeof(void*)/sizeof(int)] = (int)numitems;
  lua_pushnil(L);
  for (uint32_t i = 0; lua_next(L, idx) != 0; i++, lua_pop(L, 1)) {
    if (lua_type(L, -2) != LUA_TNUMBER) {
      Con_Error("Expected numeric keys in array table for property %s", p->pdesc->Name);
      free(mem);
      return 0; 
    }
    switch (lua_type(L, -1)) {
      case LUA_TUSERDATA:
        memcpy((char*)mem + i * p->pdesc->DataSize, luaL_checkudata(L, -1, p->pdesc->TypeString), p->pdesc->DataSize);
        break;
      case LUA_TTABLE:
        if (luaL_getmetatable(L, p->pdesc->TypeString)) {
          lua_pushvalue(L, -2);
          if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
            Con_Error("%s", luaL_checkstring(L, -1));
            lua_pop(L, 1);
            return 0;
          }
          memcpy((char*)mem + i * p->pdesc->DataSize, luaL_checkudata(L, -1, p->pdesc->TypeString), p->pdesc->DataSize);
          lua_pop(L, 1);
          break;
        } else {
          Con_Error("Expected userdata of type %s in array table for property %s", p->pdesc->TypeString, p->pdesc->Name);
          return 0;
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
            Con_Error("Unsupported data type in array table for property %s", p->pdesc->Name);
            free(mem);
            return 0;
        }
        break;
      default:
        Con_Error("Unsupported value type %d in array table for property %s", lua_type(L, -1), p->pdesc->Name);
        free(mem);
        return 0;
    }
  }
  PROP_SetValue(p, &mem);
  return 0;
}

int luaX_readProperty(lua_State* L, int idx, lpProperty_t p)
{
  int luatype = lua_type(L, idx);
  float number;
  void* udata = NULL;

  if (lua_isnil(L, idx)) {
    PROP_Clear(p);
    return 0;
  }
  // handle array properties
  if (p->pdesc && p->pdesc->IsArray) {
    return read_array(L, idx, p);
  }

  switch (luatype) {
    case LUA_TSTRING:
			p->type = p->type == kDataTypeNone ? kDataTypeString : p->type;
      switch (p->type) {
        case kDataTypeString:
          PROP_SetValue(p, luaL_checkstring(L, idx));
          break;
        // We support loading of objects by passing a string to property
        case kDataTypeInt:
//          Con_Error("Passing string to int property %s", p->pdesc->Name);
        case kDataTypeObject:
        case kDataTypeEnum:
          f_parse_property(L, p, luaL_checkstring(L, idx));
          break;
        case kDataTypeColor:
          f_parse_property(L, p, luaL_checkstring(L, idx));
          break;
        case kDataTypeStruct:
        default:
          Con_Error("Trying to parse a property %s with a string %s", p->pdesc->Name, luaL_checkstring(L, idx));
          assert(!"Parsing of properties is deprecated");
          break;
      }
      break;
    case LUA_TBOOLEAN:
      if (p->type == kDataTypeNone)
        p->type = kDataTypeBool;
      assert(p->type == kDataTypeBool);
      number = lua_toboolean(L, idx);
      PROP_SetValue(p, &(int){number});
      break;
    case LUA_TNUMBER:
      switch (p->type) {
        case kDataTypeStruct:
        case kDataTypeColor:
          // TODO: do we want to support such syntax?
          for (size_t s = 0; s < p->pdesc->DataSize/sizeof(float); s++) {
            ((float*)p->value)[s] = luaL_checknumber(L, idx);
          }
          break;
        case kDataTypeFloat:
          PROP_SetValue(p, &(float){luaL_checknumber(L, idx)});
          break;
        case kDataTypeBool:
        case kDataTypeInt:
        case kDataTypeEnum:
          number = luaL_checknumber(L, idx);
          PROP_SetValue(p, &(int){luaL_checknumber(L, idx)});
          break;
        default:
          assert(!"Unsupported type for property assigning from number");
          break;
      }
      break;
    case LUA_TTABLE:
      if (p->type == kDataTypeObject) {
        luaX_parsefield(lpObject_t, __userdata, idx, luaL_testudata, API_TYPE_OBJECT);
        if (__userdata) {
          PROP_SetValue(p, &__userdata);//(lpObject_t ){luaX_checkObject(L, idx)});
        } else if (*p->pdesc->TypeString) {
          lua_getfield(L, LUA_REGISTRYINDEX, p->pdesc->TypeString);
          lua_pushvalue(L, idx);
          lua_call(L, 1, 1);
          PROP_SetValue(p, &(lpObject_t ){luaX_checkObject(L, -1)});
        } else {
          Con_Error("Can't assign a table value to %s", PROP_GetName(p));
        }
      } else {
        Con_Error("Can't assign a table value to %s", PROP_GetName(p));
      }
      break;
    case LUA_TUSERDATA:
      switch (p->type) {
        case kDataTypeObject:
          if ((udata = luaL_testudata(L, idx, API_TYPE_OBJECT))) PROP_SetValue(p, &udata);
          else return luaL_error(L, "Incorrect userdata for %s(%s) property\n", p->pdesc->Name, API_TYPE_OBJECT);
          break;
        case kDataTypeStruct:
          PROP_SetValue(p, luaL_checkudata(L, idx, PROP_GetUserData(p)));
          break;
        case kDataTypeColor:
          PROP_SetValue(p, luaX_checkcolor(L, idx));
          break;
        default:
          return luaL_error(L, "Incorrect input (lua_type=%d) for (type=%d) %s property\n", lua_type(L, idx), p->type, p->pdesc->Name);
      }
      break;
    case LUA_TLIGHTUSERDATA:
      //        Con_Warning("Using light user data");
      PROP_SetValue(p, &(handle_t){ lua_touserdata(L, idx) });
      break;
  }
  return 0;
}

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
      lua_pushstring(L, value);
      break;
    case kDataTypeNone:
      lua_pushnil(L);
      break;
    case kDataTypeObject: {
      lpObject_t object = *(lpObject_t*)value;
      if (strcmp(type->TypeString, "Object") && *(void**)value) {
        object = CMP_GetObject(*(void**)value);
      }
      if (object) {
        lua_geti(L, LUA_REGISTRYINDEX, OBJ_GetLuaObject(object));
      } else {
        lua_pushnil(L);
      }
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
