#include <include/api.h>

lpObject_t luaX_checkObject(lua_State* L, int arg) {
  if (lua_type(L, arg) == LUA_TTABLE) {
    luaX_parsefield(lpObject_t, __userdata, arg, luaL_checkudata, API_TYPE_OBJECT);
    return __userdata;
  } else {
    return luaL_checkudata(L, arg, API_TYPE_OBJECT);
  }
}

ORCA_API int
parse_property(lua_State* L,
               const char* str,
               struct PropertyType const* prop,
//               void* struct_ptr)
               void* valueptr)
{
//  void* valueptr = ((char*)struct_ptr + prop->Offset);
  switch (prop->DataType) {
    case kDataTypeBool:
      *(bool*)valueptr = strcasecmp(str, "true") == 0 || strcmp(str, "1") == 0;
      return TRUE;
    case kDataTypeInt:
      *(int*)valueptr = atoi(str);
      return TRUE;
    case kDataTypeEnum:
      for (int i = 0; prop->EnumValues[i] != NULL; i++) {
        if (strcmp(str, prop->EnumValues[i]) == 0) {
          *(int*)valueptr = i;
          return TRUE;
        }
      }
      return luaL_error(L, "parse_property(%s): Invalid enum value for property '%s'\n", str, prop->Name);
    case kDataTypeFloat:
      *(float*)valueptr = atof(str);
      return TRUE;
    case kDataTypeString:
      if (*(char**)valueptr) free(*(char**)valueptr); // Free existing string if necessary
      *(char**)valueptr = strdup(str);
      return TRUE;
    case kDataTypeColor:
      *(struct color*)valueptr = COLOR_Parse(str);
      return TRUE;
    case kDataTypeStruct:
      if (luaL_getmetatable(L, prop->TypeString) && lua_getfield(L, -1, "fromstring")) {
        lua_pushstring(L, str);
        lua_call(L, 1, 1);
        memcpy(valueptr, lua_touserdata(L, -1), prop->DataSize);
        lua_pop(L, 2); // Pop the result and the metatable
        return TRUE;
      } else {
        return luaL_error(L, "parse_property(%s): Can't find struct '%s'\n", prop->Name, prop->TypeString);
      }
      return TRUE;
    case kDataTypeObject:
      lua_getglobal(L, "require");
      lua_pushstring(L, str);
      lua_call(L, 1, 1);
      if (lua_getfield(L, -1, "__userdata") == LUA_TNIL) {
        return luaL_error(L, "parse_property(%s): The module '%s' does not return a valid object for property '%s'\n", str, str, prop->Name);
      }
      *(void**)valueptr = lua_touserdata(L, -1);
      // struct Object* object = luaX_checkObject(L, -1);
//      if (prop->TypeString) {
//        *(void**)valueptr = OBJ_GetComponent(object, fnv1a32(prop->TypeString));
//      } else {
        // *(void**)valueptr = object;
//      }
      return TRUE;
    default:
      return luaL_error(L, "parse_property(%s): Unsupported property type %d for parsing\n", prop->Name, prop->DataType);
  }
  return TRUE;
}

ORCA_API void
read_property(lua_State *L,
              int idx,
              struct PropertyType const* prop,
              // void* struct_ptr)
              void* valueptr)
{
  // void* valueptr = ((char*)struct_ptr + prop->Offset);
  switch (prop->DataType) {
    case kDataTypeBool:
      *(int*)valueptr = lua_toboolean(L, idx) != 0;
      break;
    case kDataTypeInt:
      *(int*)valueptr = (int)luaL_checkinteger(L, idx);
      break;
    case kDataTypeEnum:
      *(int*)valueptr = (int)luaL_checkoption(L, idx, NULL, prop->EnumValues);
      break;
    case kDataTypeFloat:
      *(float*)valueptr = (float)luaL_checknumber(L, idx);
      break;
    case kDataTypeString:
      if (*(char**)valueptr && *(intptr_t*)valueptr != -1) free(*(char**)valueptr); // Free existing string if necessary
      *(char**)valueptr = strdup(luaL_checkstring(L, idx));
      break;
    case kDataTypeColor:
      if (lua_isstring(L, idx)) {
        *(struct color*)valueptr = COLOR_Parse(luaL_checkstring(L, idx));
      } else {
        memcpy(valueptr, luaL_checkudata(L, idx, "Color"), prop->DataSize);
      }
      break;
    case kDataTypeStruct:
      float* a = luaL_checkudata(L, idx, prop->TypeString);
      int* b = luaL_checkudata(L, idx, prop->TypeString);
      memcpy(valueptr, luaL_checkudata(L, idx, prop->TypeString), prop->DataSize);
      break;
    case kDataTypeObject:
      if (lua_type(L, (idx = lua_absindex(L, idx))) == LUA_TTABLE) {
        if (lua_getfield(L, idx, "__userdata") == LUA_TNIL) {
          lua_pop(L, 1);
          luaL_getmetatable(L, prop->TypeString);
          lua_pushvalue(L, idx);
          lua_call(L, 1, 1);
          if (lua_getfield(L, -1, "__userdata") == LUA_TNIL) {
            luaL_error(L, "Expected an object of type '%s' for property '%s'", prop->TypeString, prop->Name);
          }
        }
        *(void**)valueptr = lua_touserdata(L, -1);
        lua_pop(L, 1); // Remove the __userdata field from the stack
        break;
      } else {
        luaL_error(L, "Unsupported input type %d for property %s of type object", lua_type(L, idx), prop->Name);
        break;
      }
    default:
      luaL_error(L, "Unsupported property type");
      break;
    }
  }
  
ORCA_API int
write_property(lua_State *L,
               struct PropertyType const* prop,
              //  void const* struct_ptr)
               void const* valueptr)
{
    // void const* valueptr = ((char const*)struct_ptr + prop->Offset);
    switch (prop->DataType) {
      case kDataTypeBool:
        lua_pushboolean(L, *(bool*)valueptr);
        break;
      case kDataTypeInt:
        lua_pushinteger(L, *(int*)valueptr);
        break;
      case kDataTypeEnum:
        lua_pushstring(L, prop->EnumValues[*(int*)valueptr]);
        break;
      case kDataTypeFloat:
        lua_pushnumber(L, *(float*)valueptr);
        break;
      case kDataTypeString:
        lua_pushstring(L, *(char**)valueptr);
        break;
      case kDataTypeColor:
        lua_newuserdata(L, sizeof(struct color));
        luaL_setmetatable(L, "Color");
        memcpy(lua_touserdata(L, -1), valueptr, sizeof(struct color));
        break;
      case kDataTypeStruct:
        lua_pushlightuserdata(L, (void*)valueptr);
        luaL_setmetatable(L, prop->TypeString);
        break;
      case kDataTypeObject:
        if (*(char**)valueptr) {
          lua_geti(L, LUA_REGISTRYINDEX, *(int*)(*(char**)valueptr+LUASTATE_IN_OBJECT));
        } else {
          lua_pushnil(L);
        }
        break;
      default:
        break;
    }
    return 1;
  }
  
