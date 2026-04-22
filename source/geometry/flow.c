#include <include/api.h>
#include <source/geometry/geometry.h>
#include <source/core/core.h>

lpObject_t _lua_checkobject(lua_State* L, int arg) {
  return *(lpObject_t*)luaL_checkudata(L, arg, API_TYPE_OBJECT);
}

void _lua_pushobject(lua_State* L, lpcObject_t self)
{
  if (!self) {
    lua_pushnil(L);
    return;
  }
  lpObject_t* ud = lua_newuserdata(L, sizeof(lpObject_t));
  *ud = (lpObject_t)self;
  luaL_setmetatable(L, API_TYPE_OBJECT);
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
      if (lua_type(L, -1) != LUA_TUSERDATA) {
        return luaL_error(L, "parse_property(%s): The module '%s' does not return a valid object for property '%s'\n", str, str, prop->Name);
      }
      *(void**)valueptr = _lua_checkobject(L, -1);
      lua_pop(L, 1);
      return TRUE;
//    case kDataTypeEvent:
//      return TRUE;
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
  if (lua_isnoneornil(L, idx)) return; // absent/nil arg → leave value zero-initialised
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
      if (prop->DataSize > sizeof(char*)) {
        strncpy((char*)valueptr, luaL_checkstring(L, idx), prop->DataSize - 1);
        ((char*)valueptr)[prop->DataSize - 1] = '\0';
      } else {
        if (*(char**)valueptr && *(intptr_t*)valueptr != -1) free(*(char**)valueptr); // Free existing string if necessary
        *(char**)valueptr = strdup(luaL_checkstring(L, idx));
      }
      break;
    case kDataTypeColor:
      if (lua_isstring(L, idx)) {
        *(struct color*)valueptr = COLOR_Parse(luaL_checkstring(L, idx));
      } else {
        memcpy(valueptr, luaL_checkudata(L, idx, "Color"), prop->DataSize);
      }
      break;
    case kDataTypeStruct:
      memcpy(valueptr, luaL_checkudata(L, idx, prop->TypeString), prop->DataSize);
      break;
    case kDataTypeObject:
      if (lua_type(L, (idx = lua_absindex(L, idx))) == LUA_TUSERDATA) {
        *(void**)valueptr = _lua_checkobject(L, idx);
        break;
      } else if (lua_type(L, idx) == LUA_TSTRING) {
        parse_property(L, luaL_checkstring(L, idx), prop, valueptr);
        break;
      } else {
        luaL_error(L, "Unsupported input type %d for property %s of type object", lua_type(L, idx), prop->Name);
        break;
      }
    case kDataTypeEvent:
      if (*(event_t *)valueptr) {
        luaL_unref(L, LUA_REGISTRYINDEX, *(event_t *)valueptr);
        *(event_t *)valueptr = 0;
      }
      if (lua_type(L, idx) == LUA_TFUNCTION) {
        *(event_t *)valueptr = luaL_ref(L, LUA_REGISTRYINDEX);
      } else if (lua_type(L, idx) == LUA_TSTRING) {
        parse_property(L, luaL_checkstring(L, idx), prop, valueptr);
        break;
      } else {
        luaL_error(L, "Unsupported input type %d for property %s of type event", lua_type(L, idx), prop->Name);
      }
      break;
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
        /* Distinguish pointer fields (lpcString_t) from fixed-size char arrays:
         * pointer fields have DataSize == sizeof(char*); fixed-size char arrays
         * must have DataSize > sizeof(char*) (e.g. char text[32]).
         * Using a char[8] on a 64-bit platform would be ambiguous — use at
         * least char[sizeof(char*)+1] for fixed-size string fields. */
        if (prop->DataSize > sizeof(char*)) {
          lua_pushstring(L, (char*)valueptr);
        } else {
          lua_pushstring(L, *(char**)valueptr);
        }
        break;
      case kDataTypeColor:
        lua_newuserdata(L, sizeof(struct color));
        luaL_setmetatable(L, "Color");
        memcpy(lua_touserdata(L, -1), valueptr, sizeof(struct color));
        break;
      case kDataTypeStruct:
        memcpy(lua_newuserdata(L, prop->DataSize), valueptr, prop->DataSize);
        luaL_setmetatable(L, prop->TypeString);
        break;
      case kDataTypeEvent:
        assert(!"Not implemented due to OBJ_send dependency");
//        lua_pushfstring(L, "%s.%s", prop->Category, prop->Name);
//        lua_pushcclosure(L, f_msgSend, 1);
        break;
      case kDataTypeObject:
        if (*(lpObject_t const*)valueptr) {
          _lua_pushobject(L, *(lpObject_t const*)valueptr);
        } else {
          lua_pushnil(L);
        }
        break;
      default:
        break;
    }
    return 1;
  }
  
