#include <include/api.h>

ORCA_API int
parse_property(const char* str,
               struct PropertyType const* prop,
               void* struct_ptr)
{
  void* valueptr = ((char*)struct_ptr + prop->Offset);
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
      fprintf(stderr, "Invalid enum value '%s' for property '%s'\n", str, prop->Name);
      return FALSE;
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
    default:
      fprintf(stderr, "Unsupported property type for parsing\n");
      return FALSE;
  }
  return TRUE;
}

ORCA_API void
read_property(lua_State *L,
              int idx,
              struct PropertyType const* prop,
              void* struct_ptr)
{
  void* valueptr = ((char*)struct_ptr + prop->Offset);
  switch (prop->DataType) {
    case kDataTypeBool:
      *(bool*)valueptr = lua_toboolean(L, idx) != 0;
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
      if (*(char**)valueptr) free(*(char**)valueptr); // Free existing string if necessary
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
      memcpy(valueptr, luaL_checkudata(L, idx, prop->TypeString), prop->DataSize);
      break;
    case kDataTypeObject:
      if (lua_type(L, idx) == LUA_TTABLE) {
        lua_getfield(L, idx, "__userdata");
        if (lua_isnil(L, -1)) {
          luaL_error(L, "Expected an table with __userdata field");
        }
        *(void**)valueptr = *(void**)lua_touserdata(L, -1);
        lua_pop(L, 1); // Remove the __userdata field from the stack
        break;
//      } else if (lua_type(L, idx) == LUA_TSTRING) {
//        lua_getglobal(L, "require");
//        lua_insert(L, -2);
//        lua_call(L, 1, 1);
//        struct Object* object = luaX_checkObject(L, -1);
//        if (prop->TypeString) {
//          *(void**)valueptr = OBJ_GetComponent(object, fnv1a32(prop->TypeString));
//        } else {
//          *(void**)valueptr = object;
//        }
      } else {
        luaL_error(L, "Unsupported input type for property of type object");
        break;
      }
    default:
      luaL_error(L, "Unsupported property type");
      break;
    }
  }
  
ORCA_API int
write_property(lua_State *L,
               int idx,
               struct PropertyType const* prop,
               void const* struct_ptr)
{
    void const* valueptr = ((char const*)struct_ptr + prop->Offset);
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
        lua_pushlightuserdata(L, *(void**)valueptr);
        luaL_setmetatable(L, prop->TypeString);
        break;
      default:
        break;
    }
    return 1;
  }
  
