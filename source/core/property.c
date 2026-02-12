#include <include/api.h>
#include <include/renderer.h>

#include "core_local.h"

#define ID_ContentOffset 0x35a57c45

enum uniform_precision { None, Low, Mid, High };

static size_t psize[] = {
  MAX_PROPERTY_STRING,  // None
  sizeof(bool_t),  // Bool,
  sizeof(int),  // Int,
  sizeof(int),  // Enum,
  sizeof(float),  // Float,
  sizeof(vec2_t),  // Vector2D,
  sizeof(vec3_t),  // Vector3D,
  sizeof(vec4_t),  // Vector4D,
  sizeof(mat3_t),  // Matrix2D,
  sizeof(mat4_t),  // Matrix3D,
  sizeof(transform2_t),  // Transform2D,
  sizeof(transform3_t),  // Transform3D,
  MAX_PROPERTY_STRING,  // FixedString,
  sizeof(void*),  // LongString,
  sizeof(color_t),  // Color,
  sizeof(vec4_t),  // Edges,
  sizeof(objectTags_t),  // ObjectTags,
  sizeof(void*),  // Event,
  sizeof(lpObject_t),  // Object,
  0,  // Group,
};

ORCA_API lpcString_t
(*_PDESC_Parse)(lpObject_t hobj,
               lpcPropertyDesc_t pdesc,
               lpProperty_t property,
               lpcString_t string,
               void* dest);

ORCA_API lpObject_t
(*_OBJ_LoadDocument)(lua_State* L, xmlDocPtr doc);

struct Property
{
  shortStr_t name;
  eDataType_t type;
  enum uniform_precision precision;
  struct token* programs[ATTR_COUNT];
  LPSTR programSources[ATTR_COUNT];
  uint32_t longIdentifier;
  uint32_t shortIdentifier;
  uint32_t flags;
  lpcString_t classname;
  void* value;
  void* intermediate; // used to store object reference while value stores component
  lpcString_t userdata;
  lpObject_t object;
  char oldvalue[MAX_PROPERTY_STRING];
  lpcPropertyDesc_t pdesc;
  uint32_t updateFrame;
  lpProperty_t callbackEvent;
  lpProperty_t next;
};


void
Token_Release(struct token* token)
{
  SafeDelete(token->next, Token_Release);
  free(token);
}

static bool_t
PROP_HasChanged(lpProperty_t property)
{
  //    PROP_Update(property);
  if (memcmp(property->oldvalue, property->value, PROP_GetSize(property))) {
    memcpy(property->oldvalue, property->value, PROP_GetSize(property));
    return TRUE;
  }
  return FALSE;
}

lpProperty_t
PROP_AddToList(lpProperty_t property, lpProperty_t* list)
{
  ADD_TO_LIST(property, *list);
  return property;
}

//#define DEBUG_PROGRAM

#ifdef DEBUG_PROGRAM
static void
print_name(lpObject_t object)
{
  if (OBJ_GetParent(object)) {
    print_name(OBJ_GetParent(object));
  }
  Con_Error("%s/", OBJ_GetName(object));
}
#endif

bool_t
PROP_Update(lpProperty_t property)
{
  if (property == NULL ||
      property->updateFrame == core.frame)
    return FALSE;
  property->updateFrame = core.frame;
  FOR_LOOP(i, ATTR_COUNT)
  {
    if (property->programs[i]) {
      struct vm_register r = { 0 };
      if (!OBJ_RunProgram(property->object, property->programs[i], &r) ||
          !PROP_Import(property, i, &r)) {
#ifdef DEBUG_PROGRAM
        print_name(property->object);
        Con_Error("%s", property->name);
#endif
        Token_Release(property->programs[i]);
        property->programs[i] = NULL;
      } else {
        property->flags &= ~PF_NIL;
        extern int _numbindings;
        _numbindings++;
      }
    }
  }
  return TRUE;
}

bool_t
PROP_IsNull(lpcProperty_t property)
{
  return property == NULL || property->flags & PF_NIL;
}

lpProperty_t
PROP_FindByLongID(lpProperty_t list, uint32_t identifier) {
  FOR_EACH_LIST(struct Property, property, list) {
    if (property->longIdentifier == identifier) {
      return property;
    }
  }
  return NULL;
}

lpProperty_t
PROP_FindByShortID(lpProperty_t list, uint32_t identifier) {
  FOR_EACH_LIST(struct Property, property, list) {
    if (property->shortIdentifier == identifier) {
      return property;
    }
  }
  return NULL;
}

lpcString_t
PROP_GetCallbackEvent(lpcProperty_t p)
{
  return p->callbackEvent ? PROP_GetValue(p->callbackEvent) : NULL;
}

lpProperty_t
PROP_GetNext(lpcProperty_t property)
{
  return property->next;
}

uint32_t
PROP_GetFlags(lpcProperty_t property)
{
  return property->flags;
}

void
PROP_SetFlag(lpProperty_t property, uint32_t value)
{
  property->flags |= value;
}

eDataType_t
PROP_GetType(lpcProperty_t property)
{
  return property->type;
}

enum uniform_precision
PROP_GetPrecision(lpcProperty_t property)
{
  return property->precision;
}

uint32_t
PROP_GetShortID(lpcProperty_t property)
{
  return property->shortIdentifier;
}

lpcString_t
PROP_GetUserData(lpcProperty_t property)
{
  return property->userdata;
}

void const*
PROP_GetValue(lpcProperty_t property)
{
  switch (property->type) {
    case kDataTypeLongString:
      return *(lpcString_t*)property->value;
    case kDataTypeObject:
      return &property->intermediate;
    default:
      return property->value;
  }
}

void
PROP_SetValue(lpProperty_t property, void const* source)
{
  if (property->type == kDataTypeLongString) {
    LPSTR ptr = *(LPSTR*)property->value;
    *((LPSTR*)property->value) = strdup(source);
    if (ptr) {
      free(ptr);
    }
  } else if (property->type == kDataTypeObject) {
    if (property->userdata == NULL) {
      // TODO: it's a workaround
      property->userdata = "Texture";
    }
    int ident = fnv1a32(property->userdata);
    lpObject_t object = *(lpObject_t *)source;
    property->intermediate = object;
    if (!object) {
      memset(property->value, 0, PROP_GetSize(property));
      property->flags |= PF_NIL;
      return;
    }
    void* udata = OBJ_GetComponent(object, ident);
    if (!udata) {
      memset(property->value, 0, PROP_GetSize(property));
      property->flags |= PF_NIL;
      Con_Error("No %s in object %s", property->userdata, OBJ_GetName(object));
      return;
    }
    memcpy(property->value, &udata, PROP_GetSize(property));
  } else {
    memcpy(property->value, source, PROP_GetSize(property));
  }
  property->flags &= ~PF_NIL;
  property->flags |= PF_MODIFIED;
  if (property->longIdentifier != ID_ContentOffset) {
    OBJ_SetDirty(property->object);
  } else {
    OBJ_SetFlags(property->object, OBJ_GetFlags(property->object) | OF_SCROLL);
  }
}

static bool_t
_AssignCallback(lua_State* L, lpProperty_t property)
{
  if (!L)
    return FALSE;
  lpObject_t object = property->object;
  static path_t str;
  sprintf(str, ON_CHANGED_CALLBACK, property->name);
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

// static int create_property(lua_State *L) {
//	lpObject_t  object = luaX_checkObject(L, 1);
//	lpcString_t  name = luaL_checkstring(L, 2);
//	lpcString_t  type = luaL_checkstring(L, 3);
//	lpProperty_t property = PROP_Create(object, name, type);
//	static path_t str;
//	sprintf(str, ON_CHANGED_CALLBACK, PROP_GetShortName(property));
//	lua_getfield(L, -1, str);
//	if (lua_type(L, -1) == LUA_TFUNCTION) {
//		PROP_SetFlag(property, PF_HASCHANGECALLBACK | PF_NIL);
//	} else {
//		PROP_SetFlag(property, PF_NIL);
//	}
//	lua_pop(L, 2);
//
//	lua_pushlightuserdata(L, property);
//	return 1;
// }

lpProperty_t
PROP_Create(lua_State* L,
            lpObject_t object,
            lpcString_t name,
            eDataType_t type,
            lpcString_t udata)
{
  lpcString_t dot = strrchr(name, '.');
  lpProperty_t property = ZeroAlloc(sizeof(struct Property));
  property->longIdentifier = fnv1a32(name);
  property->shortIdentifier = dot ? fnv1a32(dot + 1) : property->longIdentifier;
  property->next = NULL;
  property->type = type;
  property->object = object;
  property->userdata = udata;
  memset(property->oldvalue, 0xff, PROP_GetSize(property));
  strncpy(property->name, name, sizeof(property->name));

  _RegisterProperty(object, property);
  _AssignCallback(L, property);

  return property;
}

lpProperty_t
CMP_CreateProperty(lua_State* L,
                   struct component* comp,
                   lpcPropertyDesc_t desc)
{
  lpProperty_t property = ZeroAlloc(sizeof(struct Property));
  assert(property);
  property->longIdentifier = desc->id->Identifier;
//  lpcString_t dot = strrchr(desc->name, '.');
//  if (dot) {
//    property->shortIdentifier = fnv1a32(dot + 1);
//  } else {
//    property->shortIdentifier = property->longIdentifier;
//  }
  property->longIdentifier = desc->FullIdentifier;
  property->shortIdentifier = desc->id->Identifier;
  property->next = NULL;
  property->type = desc->DataType;
  property->object = CMP_GetOwner(comp);
  property->userdata = desc->TypeString;
  property->pdesc = desc;
  property->classname = CMP_GetClassName(comp);

  PROP_AddToList(property, &_GetProperties(CMP_GetOwner(comp)));
  CMP_SetProperty(comp, property);

  _AssignCallback(L, property);

  //    PROP_SetValue(property, pdesc->value);
  memset(property->oldvalue, 0xff, PROP_GetSize(property));
  strncpy(property->name, desc->id->Name, sizeof(property->name));
  return property;
}

int luaX_readProperty(lua_State* L, int idx, lpProperty_t p)
{
  int luatype = lua_type(L, idx);
  float number;
  void* udata = NULL;

  if (lua_isnil(L, idx)) {
    PROP_SetFlag(p, PF_NIL);
    return 0;
  }

  // read enum value
  if (p->type == kDataTypeEnum && lua_type(L, idx) == LUA_TSTRING) {
    int value = strlistidx(luaL_checkstring(L, idx), p->userdata, NULL);
    if (value == -1) {
      Con_Error("Can't set value %s on %s",
             luaL_checkstring(L, idx),
             p->name);
    }
    PROP_SetValue(p, &value);
    return 0;
  }

  switch (p->type) {
    case kDataTypeEnum:
    case kDataTypeInt:
      PROP_SetValue(p, &(int){ (int)luaL_checkinteger(L, idx) });
      return 0;
    default:
      break;
  }
  switch (luatype) {
    case LUA_TSTRING:
			p->type = p->type == kDataTypeNone ? kDataTypeFixed : p->type;
      if (p->type == kDataTypeFixed) {
        strncpy(p->value, luaL_checkstring(L, idx), MAX_PROPERTY_STRING);
        p->flags &= ~PF_NIL;
        p->flags |= PF_MODIFIED;
      } else {
        PROP_Parse(p, luaL_checkstring(L, idx));
      }
      break;
    case LUA_TBOOLEAN:
      if (p->type == kDataTypeNone)
        p->type = kDataTypeBool;
      assert(p->type == kDataTypeBool);
      number = lua_toboolean(L, idx);
      PROP_SetValue(p, &number);
      break;
    case LUA_TNUMBER:
      if (p->type == kDataTypeGroup) {
        uint32_t k = 0;
        FOR_LOOP(j, p->pdesc->NumComponents) {
          switch (p->pdesc[j+1].DataType) {
            case kDataTypeFloat:
              ((float*)p->value)[k++] = luaL_checknumber(L, idx);
              break;
            case kDataTypeInt:
              ((int*)p->value)[k++] = luaL_checknumber(L, idx);
              break;
            case kDataTypeGroup:
              // skip
              break;
            default:
              return luaL_error(L, "incorrect type %d in group property %s for number input", p->pdesc[j+1].DataType, p->name);
          }
        }
      } else{
        assert(p->type == kDataTypeInt || p->type == kDataTypeFloat);
        number = luaL_checknumber(L, idx);
        PROP_SetValue(p, &number);
      }
      break;
    case LUA_TTABLE:
      if (p->type == kDataTypeObject) {
        PROP_SetValue(p, &(lpObject_t ){luaX_checkObject(L, idx)});
      } else {
        Con_Error("Can't assign a table value to %s", PROP_GetName(p));
      }
      break;
    case LUA_TUSERDATA:
      switch (p->type) {
        case kDataTypeObject:
          if ((udata = luaL_testudata(L, idx, API_TYPE_OBJECT))) PROP_SetValue(p, &udata);
          else return luaL_error(L, "Incorrect userdata for %s %s property\n", API_TYPE_OBJECT, p->name);
          break;
        case kDataTypeGroup:
          if ((udata = luaL_testudata(L, idx, p->userdata))) PROP_SetValue(p, udata);
          else return luaL_error(L, "Incorrect userdata for %s %s property\n", p->userdata, p->name);
          break;
        case kDataTypeVector2D:
          PROP_SetValue(p, luaL_checkudata(L, idx, "vec2"));
          break;
        case kDataTypeVector3D:
          PROP_SetValue(p, luaL_checkudata(L, idx, "vec3"));
          break;
        case kDataTypeVector4D:
          PROP_SetValue(p, luaL_checkudata(L, idx, "vec4"));
          break;
        default:
          return luaL_error(L, "Incorrect input (lua_type=%d) for (type=%d) %s property\n", lua_type(L, idx), p->type, p->name);
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
                   eDataType_t type,
                   void *value,
                   lpcString_t typestring,
                   size_t datasize)
{
  switch (type) {
    case kDataTypeBool:
      lua_pushboolean(L, *((bool_t*)value) != FALSE);
      break;
    case kDataTypeInt:
      lua_pushinteger(L, *((int*)value));
      break;
    case kDataTypeEnum:
      lua_pushstring(L, strlistget(*((int*)value), typestring));
      break;
    case kDataTypeFloat:
      lua_pushnumber(L, *((float*)value));
      break;
    case kDataTypeVector2D:
      memcpy(lua_newuserdata(L, sizeof(vec2_t)), value, sizeof(vec2_t));
      luaL_setmetatable(L, "vec2");
      break;
    case kDataTypeVector3D:
      memcpy(lua_newuserdata(L, sizeof(vec3_t)), value, sizeof(vec3_t));
      luaL_setmetatable(L, "vec3");
      break;
    case kDataTypeVector4D:
      memcpy(lua_newuserdata(L, sizeof(vec4_t)), value, sizeof(vec4_t));
      luaL_setmetatable(L, "vec4");
      break;
    case kDataTypeMatrix3D:
      memcpy(lua_newuserdata(L, sizeof(mat4_t)), value, sizeof(mat4_t));
      luaL_setmetatable(L, "mat4");
      break;
    case kDataTypeLongString:
      lua_pushstring(L, *(lpcString_t*)value);
      break;
    case kDataTypeFixed:
    case kDataTypeEvent:
      lua_pushstring(L, value);
      break;
    case kDataTypeNone:
      lua_pushnil(L);
      break;
    case kDataTypeObject: {
      lpObject_t object = *(lpObject_t*)value;
      if (typestring && strcmp(typestring, "Object") && *(void**)value) {
        object = CMP_GetObject(*(void**)value);
      }
      if (object) {
        lua_geti(L, LUA_REGISTRYINDEX, OBJ_GetLuaObject(object));
      } else {
        lua_pushnil(L);
      }
      break;
    }
    case kDataTypeGroup:
      memcpy(lua_newuserdata(L, datasize), value, datasize);
      luaL_setmetatable(L, typestring);
      break;
    default:
      fprintf(stderr, "push(): Unsupported property type %d\n", type);
      break;
  }
}

void luaX_pushProperty(lua_State* L, lpcProperty_t property)
{
  if (PROP_IsNull(property)) {
    lua_pushnil(L);
    return;
  }
  _pushproperty(L, property->type, property->value, property->userdata, property->pdesc?property->pdesc->DataSize:4);
}

INLINE bool_t
PROP_HasHandler(lpProperty_t property)
{
  if (property->flags &
      (PF_HASCHANGECALLBACK | PF_USED_IN_STATE_MANAGER | PF_USED_IN_TRIGGER))
    return TRUE;
  if (property->callbackEvent)
    return TRUE;
  return FALSE;
}

#define kEventPropertyChanged 0x6d47e0cc

void
PROP_ProcessEvents(lua_State* L,
                   lpProperty_t property,
                   lpObject_t object)
{
  for (; property; property = property->next) {
    if (!PROP_HasHandler(property) || !PROP_HasChanged(property))
      continue;
    PROP_Update(property);
    if (property->flags & PF_USED_IN_STATE_MANAGER) {
      SM_HandleControllerChange(object, property);
    }
    if (property->flags & PF_HASCHANGECALLBACK) {
      static path_t str;
      sprintf(str, ON_CHANGED_CALLBACK, PROP_GetShortName(property));
      luaX_pushProperty(L, property);
      luaX_executecallback(L, object, str, 1);
    }
    if (property->flags & PF_USED_IN_TRIGGER) {
      OBJ_SendMessageW(object, kEventPropertyChanged, 0,
                       &(PROPERTYCHANGEDSTRUCT){
                         .hProperty = property,
                         .L = L,
                       });
    }
    if (property->callbackEvent) {
      PROP_ExecuteChangedCallback(L, object, property);
    }
  }
}

void
PROP_AttachProgram(lpProperty_t p,
                   enum property_attribute a,
                   struct token* program,
                   lpcString_t code)
{
  SafeSet(p->programSources[a], strdup(code), free);
  SafeSet(p->programs[a], program, Token_Release);
}

bool_t
PROP_HasProgram(lpProperty_t p)
{
  FOR_LOOP(i, ATTR_COUNT)
  {
    if (p->programs[i]) {
      return TRUE;
    }
  }
  return FALSE;
}

lpcString_t
PROP_GetName(lpcProperty_t property)
{
  return property->name;
}

lpcString_t
PROP_GetComponentName(lpcProperty_t property)
{
  return property->classname;
}

void
PROP_GetFullName(lpcProperty_t property, LPSTR buf, int size)
{
  if (property->classname) {
    snprintf(buf, size, "%s.%s", property->classname, property->name);
  } else {
    strncpy(buf, property->name, size);
  }
}

lpcString_t
PROP_GetShortName(lpcProperty_t property)
{
  if (property->longIdentifier != property->shortIdentifier) {
    return strrchr(property->name, '.') + 1;
  } else {
    return property->name;
  }
}

uint32_t
PROP_GetSize(lpcProperty_t property)
{
  if (property->type == kDataTypeGroup) {
    uint32_t size = 0;
    FOR_LOOP(i, property->pdesc->NumComponents) {
      eDataType_t type = (property->pdesc+i+1)->DataType;
      if (type == kDataTypeGroup)
        continue;
      size += property->pdesc->DataSize;
    }
    return size;
  } else {
    return (uint32_t)psize[property->type];
  }
}

void
PROP_SetTypeSize(lpProperty_t p, eDataType_t t, uint32_t s)
{
  p->type = t;
}

// int f_create_changed_callback(lua_State *L) {
// 	lpObject_t  object = luaX_checkObject(L, 1);
// 	lpProperty_t property = lua_touserdata(L, 2);
// 	shortStr_t callbackName = {0};
// 	snprintf(callbackName, sizeof(callbackName), "%sChanged",
// property->name); 	lpProperty_t callback = PROP_Create(L, object,
// callbackName, kDataTypeEvent, MAX_PROPERTY_STRING, NULL); 	property->callbackEvent
// = callback; 	lua_pushlightuserdata(L, callback); 	return 1;
// }

uint32_t
PROP_GetLongIdentifier(lpcProperty_t prop)
{
  return prop->longIdentifier;
}

void
PROP_SetValuePtr(lpProperty_t prop, void* data)
{
  prop->value = data;
}

void
OBJ_ReleaseProperties(lpObject_t hobj)
{
  FOR_EACH_LIST(struct Property, p, OBJ_GetProperties(hobj))
  {
    FOR_LOOP(i, ATTR_COUNT)
    {
      if (p->programs[i]) {
        Token_Release(p->programs[i]);
      }
    }
    free(p);
  }
}

lpObject_t
PROP_GetObject(lpcProperty_t pprop)
{
  return pprop->object;
}

void
PROP_ClearSpecialized(lpProperty_t pprop) {
  FOR_EACH_LIST(struct Property, p, pprop) {
    if (p->flags & PF_SPECIALIZED) {
      p->flags = (p->flags & ~PF_SPECIALIZED) | PF_NIL;
      memset(p->value, 0, PROP_GetSize(p));
    }
  }
}

lpcPropertyDesc_t 
PROP_GetDesc(lpcProperty_t prop) {
  return prop->pdesc;
}

void
PROP_Parse(lpProperty_t p, lpcString_t string)
{
  if (!_PDESC_Parse) {
    fprintf(stderr, "PDESC_Parse is not registered, do 'require' of 'orca.parsers.xml' first\n");
  } else {
    _PDESC_Parse(p->object, p->pdesc ? p->pdesc: &(struct PropertyDesc){
      .DataType = p->type,
      .TypeString = p->userdata,
    }, p, string, p->value);
    p->flags &= ~PF_NIL;
    p->flags |= PF_MODIFIED;
    OBJ_SetDirty(p->object);
  }
}

#include <source/editor/ed_stab_property.h>
