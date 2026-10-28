#include <include/api.h>
#include <geometry/geometry.h>
#include <core/core.h>

#include "property_internal.h"

#include <math.h>
#include <strings.h>
#include <errno.h>

// Forward declaration — avoids a circular include (filesystem.h → core.h → here)
extern struct Object* FS_LoadObject(const char* path);
extern struct Object* FS_ResolveDataSource(const char* name, const char** out_params);
extern int f_msgSend(lua_State *L);

static void read_struct_table(lua_State *L, int idx, struct StructDesc const *desc, void *valueptr);
static struct Object *create_object_from_table(lua_State *L, int idx, struct PropertyType const *prop);

static struct Object *
payload_object(struct PropertyType const *prop, void const *value)
{
  void *pointer = *(void *const *)value;
  return pointer && prop->TypeString && strcmp(prop->TypeString, "Object")
    ? CMP_GetObject(pointer) : pointer;
}

static void clear_payload_field(struct PropertyType const *prop, void *value);

#define PAYLOAD_MAX_DEPTH 32
struct PayloadAncestor {
  struct StructDesc const *desc;
  void const *source;
};

static bool_t clone_struct_payload(struct StructDesc const *desc, void *destination,
  void const *source, uint32_t depth, struct PayloadAncestor *ancestors);

static bool_t
clone_payload_field(struct PropertyType const *prop, void *destination, void const *source,
  uint32_t depth, struct PayloadAncestor *ancestors)
{
  if (prop->IsArray) return FALSE;
  if (prop->FixedCount) {
    struct PropertyType element = *prop;
    element.FixedCount = 0;
    element.DataSize /= prop->FixedCount;
    memset(destination, 0, prop->DataSize);
    for (uint32_t i = 0; i < prop->FixedCount; i++) {
      if (clone_payload_field(&element, (char *)destination + i * element.DataSize,
            (char const *)source + i * element.DataSize, depth, ancestors)) continue;
      clear_payload_field(prop, destination);
      return FALSE;
    }
    return TRUE;
  }
  if (prop->DataType == kDataTypeString && prop->DataSize == sizeof(char *)) {
    char const *text = *(char const *const *)source;
    *(char **)destination = text ? strdup(text) : NULL;
    return !text || *(char **)destination;
  }
  if (prop->DataType == kDataTypeObject) {
    memcpy(destination, source, prop->DataSize);
    struct Object *object = payload_object(prop, source);
    if (object) OBJ_AddRef(object);
    return TRUE;
  }
  if (prop->DataType == kDataTypeStruct) {
    struct StructDesc const *desc = OBJ_FindStructDesc(prop->TypeString);
    if (desc && prop->IsPointer) {
      void const *sourcePointer = *(void *const *)source;
      *(void **)destination = NULL;
      if (!sourcePointer) return TRUE;
      void *copy = calloc(1, desc->StructSize);
      if (!copy) return FALSE;
      if (!clone_struct_payload(desc, copy, sourcePointer, depth + 1, ancestors)) { free(copy); return FALSE; }
      *(void **)destination = copy;
      return TRUE;
    }
    if (desc && !prop->IsPointer) return clone_struct_payload(desc, destination, source, depth + 1, ancestors);
  }
  memcpy(destination, source, prop->DataSize);
  return TRUE;
}

static void
clear_payload_field(struct PropertyType const *prop, void *value)
{
  if (prop->IsArray) return;
  if (prop->FixedCount) {
    struct PropertyType element = *prop;
    element.FixedCount = 0;
    element.DataSize /= prop->FixedCount;
    for (uint32_t i = 0; i < prop->FixedCount; i++)
      clear_payload_field(&element, (char *)value + i * element.DataSize);
  } else if (prop->DataType == kDataTypeString && prop->DataSize == sizeof(char *)) {
    free(*(char **)value);
  } else if (prop->DataType == kDataTypeObject) {
    struct Object *object = payload_object(prop, value);
    if (object) OBJ_ReleaseRef(object);
  } else if (prop->DataType == kDataTypeStruct) {
    struct StructDesc const *desc = OBJ_FindStructDesc(prop->TypeString);
    if (desc && prop->IsPointer && *(void **)value) {
      core_ClearStructPayload(desc, *(void **)value);
      free(*(void **)value);
    } else if (desc && !prop->IsPointer) {
      core_ClearStructPayload(desc, value);
    }
  }
  memset(value, 0, prop->DataSize);
}

static bool_t
clone_struct_payload(struct StructDesc const *desc, void *destination, void const *source,
  uint32_t depth, struct PayloadAncestor *ancestors)
{
  if (depth >= PAYLOAD_MAX_DEPTH) return FALSE;
  for (uint32_t i = 0; i < depth; i++)
    if (ancestors[i].desc == desc && ancestors[i].source == source) return FALSE;
  ancestors[depth] = (struct PayloadAncestor){ desc, source };
  memcpy(destination, source, desc->StructSize);
  for (uint32_t i = 0; i < desc->NumProperties; i++) {
    struct PropertyType const *field = desc->Properties + i;
    if (clone_payload_field(field, (char *)destination + field->Offset,
          (char const *)source + field->Offset, depth, ancestors)) continue;
    for (uint32_t j = 0; j < i; j++)
      clear_payload_field(desc->Properties + j, (char *)destination + desc->Properties[j].Offset);
    memset(destination, 0, desc->StructSize);
    return FALSE;
  }
  return TRUE;
}

bool_t
core_CloneStructPayload(struct StructDesc const *desc, void *destination, void const *source)
{
  if (!desc || !destination || !source) return FALSE;
  struct PayloadAncestor ancestors[PAYLOAD_MAX_DEPTH];
  memset(destination, 0, desc->StructSize);
  return clone_struct_payload(desc, destination, source, 0, ancestors);
}

void
core_ClearStructPayload(struct StructDesc const *desc, void *value)
{
  if (!desc || !value) return;
  for (uint32_t i = 0; i < desc->NumProperties; i++)
    clear_payload_field(desc->Properties + i, (char *)value + desc->Properties[i].Offset);
  memset(value, 0, desc->StructSize);
}

void
luaX_pushStruct(lua_State *L, struct StructDesc const *desc, void const *value)
{
  if (!value) { lua_pushnil(L); return; }
  void *copy = lua_newuserdata(L, desc->StructSize);
  memset(copy, 0, desc->StructSize);
  luaL_setmetatable(L, desc->StructName);
  if (!core_CloneStructPayload(desc, copy, value))
    luaL_error(L, "Cannot snapshot struct %s: unsupported array, cyclic/deep payload, or allocation failure", desc->StructName);
}

void
luaX_readStructProperty(lua_State *L, int index, struct PropertyType const *prop, void *value)
{
  index = lua_absindex(L, index);
  if (lua_isnoneornil(L, index)) { clear_payload_field(prop, value); return; }
  if (prop->IsArray) luaL_error(L, "Array field %s requires explicit length metadata", prop->Name);
  if (prop->FixedCount) {
    luaL_checktype(L, index, LUA_TTABLE);
    luaL_argcheck(L, lua_rawlen(L, index) == prop->FixedCount, index, "fixed array has the wrong length");
    clear_payload_field(prop, value);
    struct PropertyType element = *prop;
    element.FixedCount = 0;
    element.DataSize /= prop->FixedCount;
    for (uint32_t i = 0; i < prop->FixedCount; i++) {
      lua_rawgeti(L, index, i + 1);
      luaX_readStructProperty(L, -1, &element, (char *)value + i * element.DataSize);
      lua_pop(L, 1);
    }
    return;
  }
  if (prop->DataType == kDataTypeObject) {
    struct Object *object = NULL;
    read_property(L, index, prop, &object);
    void *component = object;
    if (object && prop->TypeString && strcmp(prop->TypeString, "Object")) {
      component = OBJ_GetComponent(object, fnv1a32(prop->TypeString));
      if (!component) {
        if (lua_istable(L, index)) OBJ_ReleaseRef(object);
        luaL_error(L, "Expected %s for struct field %s", prop->TypeString, prop->Name);
      }
    }
    if (object && !lua_istable(L, index)) OBJ_AddRef(object);
    clear_payload_field(prop, value);
    *(void **)value = component;
    return;
  }
  if (prop->DataType == kDataTypeStruct) {
    struct StructDesc const *desc = OBJ_FindStructDesc(prop->TypeString);
    if (!desc) {
      if (!prop->IsPointer) luaL_error(L, "Unknown struct type %s", prop->TypeString);
      luaL_checktype(L, index, LUA_TLIGHTUSERDATA);
      clear_payload_field(prop, value);
      *(void **)value = lua_touserdata(L, index);
      return;
    }
    if (!lua_istable(L, index)) luaL_checkudata(L, index, desc->StructName);
    clear_payload_field(prop, value);
    void *destination = value;
    if (prop->IsPointer) {
      destination = calloc(1, desc->StructSize);
      if (!destination) luaL_error(L, "Cannot allocate struct %s", desc->StructName);
      *(void **)value = destination;
    }
    if (lua_istable(L, index)) {
      for (uint32_t i = 0; i < desc->NumProperties; i++) {
        struct PropertyType const *field = desc->Properties + i;
        lua_getfield(L, index, field->Name);
        luaX_readStructProperty(L, -1, field, (char *)destination + field->Offset);
        lua_pop(L, 1);
      }
    } else if (!core_CloneStructPayload(desc, destination, luaL_checkudata(L, index, desc->StructName))) {
      luaL_error(L, "Cannot copy struct %s", desc->StructName);
    }
    return;
  }
  if (prop->IsPointer && prop->DataType != kDataTypeString) {
    luaL_checktype(L, index, LUA_TLIGHTUSERDATA);
    clear_payload_field(prop, value);
    *(void **)value = lua_touserdata(L, index);
    return;
  }
  // Scalar readers validate before writing and preserve the previous value on failure.
  if (prop->DataType == kDataTypeString) luaL_checkstring(L, index);
  read_property(L, index, prop, value);
}

int
luaX_newStruct(lua_State *L, struct StructDesc const *desc)
{
  int arguments = lua_gettop(L);
  if (lua_islightuserdata(L, 1)) {
    luaX_pushStruct(L, desc, lua_touserdata(L, 1));
    return 1;
  }
  void *value = lua_newuserdata(L, desc->StructSize);
  memset(value, 0, desc->StructSize);
  luaL_setmetatable(L, desc->StructName);
  if (arguments && lua_istable(L, 1)) {
    for (uint32_t i = 0; i < desc->NumProperties; i++) {
      struct PropertyType const *field = desc->Properties + i;
      lua_getfield(L, 1, field->Name);
      luaX_readStructProperty(L, -1, field, (char *)value + field->Offset);
      lua_pop(L, 1);
    }
  } else {
    for (uint32_t i = 0; i < desc->NumProperties && i < (uint32_t)arguments; i++) {
      struct PropertyType const *field = desc->Properties + i;
      luaX_readStructProperty(L, i + 1, field, (char *)value + field->Offset);
    }
  }
  return 1;
}

ORCA_API int
parse_property(const char* str, struct PropertyType const* prop, void* valueptr)
{
  switch (prop->DataType) {
    case kDataTypeBool:
      *(bool*)valueptr = strcasecmp(str, "true") == 0 || strcmp(str, "1") == 0;
      return TRUE;
    case kDataTypeInt:
      if (prop->IsUnsigned) {
        char *end = NULL;
        errno = 0;
        unsigned long long number = strtoull(str, &end, 10);
        while (end && isspace((unsigned char)*end)) end++;
        if (errno || str == end || !end || *end || *str == '-' || number > UINT32_MAX) return FALSE;
        *(uint32_t *)valueptr = (uint32_t)number;
      } else {
        *(int*)valueptr = atoi(str);
      }
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
      // A trailing ":child" selects a named child of the resolved object, e.g.
      // "Project/DataSourceLibrary/Name:ChildName". Datasources are referenced
      // by their fully qualified path with no direct-file fallback.
      const char *colon = strrchr(path, ':');
      struct Object *loaded = NULL;
      if (colon) {
        size_t pathLen = colon - path;
        char dsName[256];
        snprintf(dsName, sizeof(dsName), "%.*s", (int)pathLen, path);
        loaded = FS_ResolveDataSource(dsName, NULL);
        if (loaded) {
          loaded = OBJ_FindChild(loaded, colon + 1, FALSE);
        }
      } else {
        // A datasource path resolves through the provider registry; any other
        // object path is loaded from the filesystem.
        loaded = FS_ResolveDataSource(path, NULL);
        if (!loaded) {
          loaded = FS_LoadObject(path);
        }
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
      if (prop->IsUnsigned) {
        lua_Integer number = luaL_checkinteger(L, idx);
        luaL_argcheck(L, number >= 0 && (uint64_t)number <= UINT32_MAX, idx, "unsigned integer is outside its 32-bit range");
        *(uint32_t *)valueptr = (uint32_t)number;
      } else {
        *(int*)valueptr = (int)luaL_checkinteger(L, idx);
      }
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

static int
populate_object_from_table(lua_State *L)
{
  struct Object *obj = lua_touserdata(L, lua_upvalueindex(1));
  int table_idx = 1;
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
    } else if (lua_type(L, -2) == LUA_TNUMBER && lua_type(L, -1) == LUA_TUSERDATA) {
      /* Array-part entries that are constructed objects become children,
       * e.g. DataSourceLibrary = { XmlDataSource { ... } }. */
      struct Object *child = luaX_checkObject(L, -1);
      if (child) OBJ_AddChild(obj, child);
    }
    lua_pop(L, 1);
  }

  return 0;
}

static struct Object *
create_object_from_table(lua_State *L, int idx, struct PropertyType const *prop)
{
  struct ClassDesc const *cls = OBJ_FindClass(prop->TypeString);
  if (!cls)
    return luaL_error(L, "Class '%s' not found for property '%s'", prop->TypeString, prop->Name), NULL;

  struct Object *obj = OBJ_Create(cls->ClassID);
  if (!obj)
    return luaL_error(L, "Failed to create object of class '%s' for property '%s'", prop->TypeString, prop->Name), NULL;

  idx = lua_absindex(L, idx);
  lua_pushlightuserdata(L, obj);
  lua_pushcclosure(L, populate_object_from_table, 1);
  lua_pushvalue(L, idx);
  // The payload cannot own this object until all its properties are populated.
  if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
    OBJ_ReleaseRef(obj);
    lua_error(L);
    return NULL;
  }
  return obj;
}
ORCA_API int
write_property(lua_State *L, struct PropertyType const* prop, void const* valueptr)
{
    if (prop->FixedCount) {
      struct PropertyType element = *prop;
      element.FixedCount = 0;
      element.DataSize /= prop->FixedCount;
      lua_createtable(L, prop->FixedCount, 0);
      for (uint32_t i = 0; i < prop->FixedCount; i++) {
        write_property(L, &element, (char const *)valueptr + i * element.DataSize);
        lua_rawseti(L, -2, i + 1);
      }
      return 1;
    }
    if (prop->IsPointer && prop->DataType != kDataTypeObject &&
        prop->DataType != kDataTypeStruct && prop->DataType != kDataTypeString) {
      if (*(void *const *)valueptr) lua_pushlightuserdata(L, *(void *const *)valueptr);
      else lua_pushnil(L);
      return 1;
    }
    // void const* valueptr = ((char const*)struct_ptr + prop->Offset);
    if (prop->IsArray) {
      void *items = valueptr ? *(void *const*)valueptr : NULL;
      int count = (valueptr && items) ? ((int const*)valueptr)[sizeof(void*)/sizeof(int)] : 0;
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
        lua_pushinteger(L, prop->IsUnsigned ? (lua_Integer)*(uint32_t const *)valueptr : *(int const *)valueptr);
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
      case kDataTypeStruct: {
        struct StructDesc const *desc = OBJ_FindStructDesc(prop->TypeString);
        void const *source = prop->IsPointer ? *(void *const *)valueptr : valueptr;
        if (!source) lua_pushnil(L);
        else if (desc) luaX_pushStruct(L, desc, source);
        else if (prop->IsPointer) lua_pushlightuserdata(L, (void *)source);
        else return luaL_error(L, "Unknown struct type %s", prop->TypeString);
        break;
      }
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
  if (PROP_GetType(p) == kDataTypeEvent && lua_type(L, idx) == LUA_TSTRING) {
    if (!PROP_BindHandler(p, lua_tostring(L, idx))) return luaL_error(L, "Could not bind event handler");
    return 0;
  }
  if (lua_isnil(L, idx)) {
    if (PROP_GetType(p) == kDataTypeEvent && p->value && *(event_t *)p->value) {
      luaL_unref(L, LUA_REGISTRYINDEX, *(event_t *)p->value);
      *(event_t *)p->value = 0;
    }
    PROP_Clear(p);
    if (PROP_GetType(p) == kDataTypeEvent) {
      p->handlerOverride = TRUE;
      p->handlerName = strdup("");
    }
    PROP_NotifyChanged(p);
    PROP_WriteBindingSource(p);
    return 0;
  }

  if (PROP_GetType(p) == kDataTypeObject && lua_istable(L, idx)) {
    struct Object *object = create_object_from_table(L, idx, p->pdesc);
    PROP_SetValue(p, &object);
    if (p->object) {
      OBJ_AddChild(p->object, object);
      PROP_SetFlag(p, PF_OWNS_OBJECT_CHILD);
    }
    return 0;
  }

  if (PROP_GetType(p) == kDataTypeEvent) {
    luaL_checktype(L, idx, LUA_TFUNCTION);
    event_t *slot = (event_t *)PROP_GetValue(p);
    if (slot && *slot) { luaL_unref(L, LUA_REGISTRYINDEX, *slot); *slot = 0; }
    free(p->handlerName);
    p->handlerName = NULL;
    p->handlerOverride = TRUE;
  }

  // Pointer-string properties copy the Lua bytes themselves; an intermediate
  // read_property allocation would be duplicated and then abandoned.
  if (PROP_GetType(p) == kDataTypeString && !p->pdesc->IsArray &&
      p->pdesc->DataSize <= sizeof(char *)) {
    PROP_SetStringValue(p, luaL_checkstring(L, idx));
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
