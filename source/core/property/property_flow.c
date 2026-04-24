#include <include/api.h>
#include <source/geometry/geometry.h>
#include <source/core/core.h>

// Forward declaration — avoids a circular include (filesystem.h → core.h → here)
extern struct Object* FS_LoadObjectFromXML(const char* path);

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
parse_property(const char* str,
               struct PropertyType const* prop,
               void* valueptr)
{
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
      Con_Printf("parse_property(%s): Invalid enum value for property '%s'", str, prop->Name);
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
    case kDataTypeStruct:
      if (OBJ_ParseStruct(prop->TypeString, str, valueptr, (size_t)prop->DataSize))
        return TRUE;
      Con_Printf("parse_property: no C parser registered for struct '%s' (property '%s')", prop->TypeString, prop->Name);
      return FALSE;
    case kDataTypeObject: {
      lpObject_t loaded = FS_LoadObjectFromXML(str);
      if (!loaded) {
        Con_Printf("parse_property: failed to load object '%s' for property '%s'", str, prop->Name);
        return FALSE;
      }
      *(lpObject_t*)valueptr = loaded;
      return TRUE;
    }
//    case kDataTypeEvent:
//      return TRUE;
    default:
      Con_Printf("parse_property(%s): Unsupported property type %d for parsing", prop->Name, prop->DataType);
      return FALSE;
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
      switch (lua_type(L, (idx = lua_absindex(L, idx)))) {
        case LUA_TUSERDATA:
          *(void**)valueptr = _lua_checkobject(L, idx);
          break;
        case LUA_TSTRING:
          parse_property(luaL_checkstring(L, idx), prop, valueptr);
          break;
        case LUA_TTABLE:
          {
            lpcClassDesc_t cls = OBJ_FindClass(prop->TypeString);
            if (!cls) {
              luaL_error(L, "Class '%s' not found for property '%s'", prop->TypeString, prop->Name);
              return;
            }
            lpObject_t obj = OBJ_Create(cls->ClassID);
            if (!obj) {
              luaL_error(L, "Failed to create object of class '%s' for property '%s'", prop->TypeString, prop->Name);
              return;
            }

            int table_idx = lua_absindex(L, idx);
            lua_pushnil(L);
            while (lua_next(L, table_idx) != 0) {
              if (lua_type(L, -2) == LUA_TSTRING) {
                lpcString_t short_name = lua_tostring(L, -2);
                lpProperty_t property = NULL;
                if (SUCCEEDED(OBJ_FindShortProperty(obj, short_name, &property))) {
                  char buf[MAX_PROPERTY_STRING] = {0};
                  int luaX_readProperty(lua_State*, int, lpProperty_t);
                  luaX_readProperty(L, -1, property);
                  PROP_SetValue(property, buf);
                  if (PROP_GetType(property) == kDataTypeString) {
                    Con_Error("Double-check if it chrashes afterwards");
                    free(*(char**)buf);
                  }
                }
              }
              lua_pop(L, 1); // pop value, keep key for lua_next
            }

            *(lpObject_t*)valueptr = obj;
          }
          break;
        default:
          luaL_error(L, "Unsupported input type %d for property %s of type object", lua_type(L, idx), prop->Name);
          break;
      }
      break;
    case kDataTypeEvent:
      if (*(event_t *)valueptr) {
        luaL_unref(L, LUA_REGISTRYINDEX, *(event_t *)valueptr);
        *(event_t *)valueptr = 0;
      }
      if (lua_type(L, idx) == LUA_TFUNCTION) {
        lua_pushvalue(L, idx);
        *(event_t *)valueptr = luaL_ref(L, LUA_REGISTRYINDEX);
      } else if (lua_type(L, idx) == LUA_TSTRING) {
        parse_property(luaL_checkstring(L, idx), prop, valueptr);
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
  
