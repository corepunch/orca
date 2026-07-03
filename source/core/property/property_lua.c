#include <include/api.h>
#include <geometry/geometry.h>
#include <core/core.h>

#include "property_internal.h"

#include <math.h>
#include <strings.h>

// Forward declaration — avoids a circular include (filesystem.h → core.h → here)
extern struct Object* FS_LoadObject(const char* path);
extern int f_msgSend(lua_State *L);

static void read_struct_table(lua_State *L, int idx,
                              struct StructDesc const *desc, void *valueptr);
static struct Object *create_object_from_table(lua_State *L, int idx,
                                               struct PropertyType const *prop);

ORCA_API int
parse_property(const char* str, struct PropertyType const* prop, void* valueptr)
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
      Con_Error("Invalid enum value %s for property '%s'", str, prop->Name);
      return FALSE;
    case kDataTypeFloat:
      *(float*)valueptr = (!strcasecmp(str, "auto") || !strcasecmp(str, "nan")) ? NAN : atof(str);
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
      Con_Error("No C parser registered for struct '%s' (property '%s')", prop->TypeString, prop->Name);
      return FALSE;
    case kDataTypeObject: {
      lpcString_t path = str;
      if (prop->TypeString && !strcmp(prop->TypeString, "FontFamily")) {
        lpcString_t font_path = CORE_FindFontFamily(str);
        if (font_path) path = font_path;
      }
      // Support "file:child" colon syntax (e.g. "Data/File:ChildName")
      const char *colon = strrchr(path, ':');
      struct Object *loaded = NULL;
      if (colon) {
        size_t pathLen = colon - path;
        char filePath[256];
        snprintf(filePath, sizeof(filePath), "%.*s", (int)pathLen, path);
        struct Object *root = FS_LoadObject(filePath);
        if (root) loaded = OBJ_FindChild(root, colon + 1, FALSE);
      } else {
        loaded = FS_LoadObject(path);
      }
      if (!loaded) {
        Con_Error("Failed to load object '%s' for property '%s'", str, prop->Name);
        return FALSE;
      }
      *(struct Object **)valueptr = loaded;
      return TRUE;
    }
//    case kDataTypeEvent:
//      return TRUE;
    default:
      Con_Error("Unsupported property type %d for parsing of property %s", prop->DataType, prop->Name);
      return FALSE;
  }
  return TRUE;
}

ORCA_API void
read_property(lua_State *L, int idx, struct PropertyType const* prop, void* valueptr)
{
  // void* valueptr = ((char*)struct_ptr + prop->Offset);
  if (lua_isnoneornil(L, idx)) return; // absent/nil arg → leave value zero-initialised

  if (prop->IsArray) {
    idx = lua_absindex(L, idx);
    if (lua_type(L, idx) != LUA_TTABLE)
      luaL_error(L, "Expected a table for array property %s", prop->Name);
    int numitems = (int)lua_rawlen(L, idx);
    void *tmp = calloc((size_t)numitems, prop->DataSize);
    int i = 0;
    lua_pushnil(L);
    while (lua_next(L, idx) != 0) {
      if (lua_type(L, -2) != LUA_TNUMBER) {
        free(tmp);
        luaL_error(L, "Expected numeric keys in array table for property %s", prop->Name);
      }
      switch (lua_type(L, -1)) {
        case LUA_TUSERDATA:
          memcpy((char*)tmp + i * prop->DataSize, luaL_checkudata(L, -1, prop->TypeString), prop->DataSize);
          break;
        case LUA_TTABLE:
          if (prop->DataType == kDataTypeStruct) {
            struct StructDesc const *desc = OBJ_FindStructDesc(prop->TypeString);
            if (!desc) {
              free(tmp);
              luaL_error(L, "Struct '%s' not found for array property '%s'", prop->TypeString, prop->Name);
            }
            read_struct_table(L, -1, desc, (char*)tmp + i * prop->DataSize);
          } else if (prop->DataType == kDataTypeObject) {
            struct Object *obj = create_object_from_table(L, -1, prop);
            memcpy((char*)tmp + i * prop->DataSize, &obj, sizeof(obj));
          } else {
            free(tmp);
            luaL_error(L, "Unsupported table element for array property %s", prop->Name);
          }
          break;
        case LUA_TNUMBER:
          switch (prop->DataType) {
            case kDataTypeFloat: ((float*)tmp)[i] = (float)luaL_checknumber(L, -1);  break;
            case kDataTypeInt:
            case kDataTypeEnum:  ((int*)tmp)[i]   = (int)luaL_checkinteger(L, -1);   break;
            default:
              free(tmp);
              luaL_error(L, "Unsupported data type in array table for property %s", prop->Name);
          }
          break;
        default:
          free(tmp);
          luaL_error(L, "Unsupported value type %d in array table for property %s", lua_type(L, -1), prop->Name);
      }
      i++;
      lua_pop(L, 1);
    }
    *(void**)valueptr = tmp;
    ((int*)valueptr)[sizeof(void*)/sizeof(int)] = numitems;
    return;
  }

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
      if (lua_istable(L, idx)) {
        struct StructDesc const *desc = OBJ_FindStructDesc(prop->TypeString);
        if (!desc) {
          luaL_error(L, "Struct '%s' not found for property '%s'", prop->TypeString, prop->Name);
        }
        read_struct_table(L, idx, desc, valueptr);
      } else {
        memcpy(valueptr, luaL_checkudata(L, idx, prop->TypeString), prop->DataSize);
      }
      break;
    case kDataTypeObject:
      switch (lua_type(L, (idx = lua_absindex(L, idx)))) {
        case LUA_TUSERDATA:
          *(void**)valueptr = luaX_checkObject(L, idx);
          break;
        case LUA_TSTRING:
          parse_property(luaL_checkstring(L, idx), prop, valueptr);
          break;
        case LUA_TTABLE:
          *(struct Object **)valueptr = create_object_from_table(L, idx, prop);
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

static void
read_struct_table(lua_State *L, int idx, struct StructDesc const *desc, void *valueptr)
{
  idx = lua_absindex(L, idx);
  memset(valueptr, 0, desc->StructSize);
  for (uint32_t i = 0; i < desc->NumProperties; i++) {
    lua_getfield(L, idx, desc->Properties[i].Name);
    read_property(L, -1, &desc->Properties[i], (char*)valueptr + desc->Properties[i].Offset);
    lua_pop(L, 1);
  }
}

static struct Object *
create_object_from_table(lua_State *L, int idx, struct PropertyType const *prop)
{
  struct ClassDesc const *cls = OBJ_FindClass(prop->TypeString);
  if (!cls) {
    luaL_error(L, "Class '%s' not found for property '%s'", prop->TypeString, prop->Name);
  }

  struct Object *obj = OBJ_Create(cls->ClassID);
  if (!obj) {
    luaL_error(L, "Failed to create object of class '%s' for property '%s'", prop->TypeString, prop->Name);
  }

  int table_idx = lua_absindex(L, idx);
  lua_pushnil(L);
  while (lua_next(L, table_idx) != 0) {
    if (lua_type(L, -2) == LUA_TSTRING) {
      lpcString_t short_name = lua_tostring(L, -2);
      struct Property *property = OBJ_FindShortProperty(obj, fnv1a32(short_name));
      if (property) {
        int luaX_readProperty(lua_State*, int, struct Property *);
        luaX_readProperty(L, -1, property);
      } else if (lua_type(L, -1) == LUA_TSTRING &&
                 (!strcmp(short_name, "Name") || !strcmp(short_name, "id"))) {
        OBJ_SetName(obj, lua_tostring(L, -1));
      }
    }
    lua_pop(L, 1);
  }

  return obj;
}
  
ORCA_API int
write_property(lua_State *L, struct PropertyType const* prop, void const* valueptr)
{
    // void const* valueptr = ((char const*)struct_ptr + prop->Offset);
    if (prop->IsArray) {
      void *items = valueptr ? *(void *const*)valueptr : NULL;
      int count = valueptr ? ((int const*)valueptr)[sizeof(void*)/sizeof(int)] : 0;
      struct PropertyType elem = *prop;
      elem.IsArray = FALSE;

      lua_createtable(L, count, 0);
      for (int i = 0; i < count; i++) {
        void const *item = NULL;
        switch (prop->DataType) {
          case kDataTypeString:
            item = &((char**)items)[i];
            break;
          case kDataTypeObject:
            item = &((void**)items)[i];
            break;
          default:
            item = (char*)items + ((size_t)i * prop->DataSize);
            break;
        }
        write_property(L, &elem, item);
        lua_rawseti(L, -2, i + 1);
      }
      return 1;
    }

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
        lua_pushfstring(L, "%s.%s", prop->Category, prop->Name);
        lua_pushcclosure(L, f_msgSend, 1);
        break;
      case kDataTypeObject:
        {
          struct Object *object = *(struct Object *const*)valueptr;
          if (strcmp(prop->TypeString, "Object") && *(void**)valueptr) {
            object = CMP_GetObject(*(void**)valueptr);
          }
          if (object) {
            luaX_pushObject(L, object);
          } else {
            lua_pushnil(L);
          }
        }
        break;
      default:
        break;
    }
    return 1;
  }

int 
luaX_readProperty(lua_State* L, int idx, struct Property *p)
{
  if (lua_isnil(L, idx)) {
    if (PROP_GetType(p) == kDataTypeEvent && p->value && *(event_t *)p->value) {
      luaL_unref(L, LUA_REGISTRYINDEX, *(event_t *)p->value);
      *(event_t *)p->value = 0;
    }
    PROP_Clear(p);
    return 0;
  }

  char buf[MAX_PROPERTY_STRING]={0};
  read_property(L, idx, p->pdesc, buf);
  PROP_SetValue(p, buf);

  return 0;
}

void 
luaX_pushProperty(lua_State* L, struct Property const *property)
{
  if (PROP_IsNull(property)) {
    lua_pushnil(L);
    return;
  }
  write_property(L, property->pdesc, property->value);
}

bool_t
PROP_RegisterChangedCallback(lua_State* L, struct Property *property, int callback_idx)
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
