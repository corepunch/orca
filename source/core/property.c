#include <include/api.h>
#include <include/renderer.h>

#include "core_local.h"

#define ID_ContentOffset 0x35a57c45

enum uniform_precision { None, Low, Mid, High };

static size_t psize[] = {
  0,                  // kDataTypeNone
  sizeof(bool_t),     // kDataTypeBool
  sizeof(int),        // kDataTypeInt
  sizeof(int),        // kDataTypeEnum
  sizeof(float),      // kDataTypeFloat
  sizeof(char*),      // kDataTypeString
  sizeof(void*),      // kDataTypeEvent
  0,                  // kDataTypeStruct
  sizeof(lpObject_t), // kDataTypeObject
};

struct Property
{
  eDataType_t            type;
  enum uniform_precision precision;
  struct token*          programs[PropertyAttribute_Count];
  LPSTR                  programSources[PropertyAttribute_Count];
  uint32_t               flags;
  void*                  value;
  void*                  intermediate; // used to store object reference while value stores component
  lpObject_t             object;
  lpcPropertyType_t      pdesc;
  uint32_t               updateFrame;
  uint32_t               stateflags;
  lpProperty_t           callbackEvent;
  lpProperty_t           next;
  char                   states[];
};


void
Token_Release(struct token* token)
{
  SafeDelete(token->next, Token_Release);
  free(token);
}

static void*
PROP_GetState(lpProperty_t p, enum PropertyState state)
{
  return p->states + state * PROP_GetSize(p);
}

static bool_t
PROP_HasChanged(lpProperty_t property)
{
  if (memcmp(PROP_GetState(property, kPropertyStateOldValue), property->value, PROP_GetSize(property))) {
    memcpy((void*)PROP_GetState(property, kPropertyStateOldValue), property->value, PROP_GetSize(property));
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

void
PROP_Clear(lpProperty_t property)
{
  FOR_LOOP(i, PropertyState_Count) {
    if (property->stateflags & (1<<i)) {
      if (property->pdesc->DataType == kDataTypeString) {
        free(*(LPSTR*)PROP_GetState(property, i));
      }
    }
  }
  memset(property->states, 0, PROP_GetSize(property) * PropertyState_Count);
  property->stateflags = 0;
  if (property->type == kDataTypeString && property->value) {
    *(char**)property->value = NULL;
  }
}

bool_t
PROP_Update(lpProperty_t property)
{
  if (property == NULL ||
      property->updateFrame == core.frame)
    return FALSE;
  property->updateFrame = core.frame;
  FOR_LOOP(i, PropertyAttribute_Count)
  {
    if (property->programs[i]) {
      struct vm_register r = { 0 };
      if (!OBJ_RunProgram(property->object, property->programs[i], &r) ||
          !PROP_Import(property, i, &r)) {
#ifdef DEBUG_PROGRAM
        print_name(property->object);
        Con_Error("%s", property->pdesc->Name);
#endif
        Token_Release(property->programs[i]);
        property->programs[i] = NULL;
      } else {
//        TODO: what to do here?
//        property->state &= ~PF_NIL;
      }
    }
  }
  return TRUE;
}

bool_t
PROP_IsNull(lpcProperty_t property)
{
  return property == NULL || !property->stateflags;
}

lpProperty_t
PROP_FindByLongID(lpProperty_t list, uint32_t identifier) {
  FOR_EACH_LIST(struct Property, property, list) {
    if (property->pdesc->FullIdentifier == identifier) {
      return property;
    }
  }
  return NULL;
}

lpProperty_t
PROP_FindByShortID(lpProperty_t list, uint32_t identifier) {
  FOR_EACH_LIST(struct Property, property, list) {
    if (property->pdesc->ShortIdentifier == identifier) {
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
  return property->pdesc->ShortIdentifier;
}

lpcString_t
PROP_GetUserData(lpcProperty_t property)
{
  return property->pdesc->TypeString;
}

void const*
PROP_GetValue(lpcProperty_t property)
{
  switch (property->type) {
    case kDataTypeString:
      return *(lpcString_t*)property->value;
    case kDataTypeObject:
      return &property->intermediate;
    default:
      return property->value;
  }
}

void*
PROP_SetStateValue(lpProperty_t property,
                   void const* source,
                   enum PropertyState state)
{
  void* ptr = PROP_GetState(property, state);
  if (property->type == kDataTypeString) {
    if (property->stateflags & (1 << state)) {
      free(*(LPSTR*)PROP_GetState(property, state));
    }
    *(LPSTR*)ptr = strdup(source);
  } else if (property->type == kDataTypeObject) {
    int ident = fnv1a32(property->pdesc->TypeString);
    lpObject_t object = *(lpObject_t *)source;
    property->intermediate = object;
    if (!object) {
      memset(ptr, 0, PROP_GetSize(property));
      property->stateflags &= ~(1<<state);
      return ptr;
    }
    void* udata = OBJ_GetComponent(object, ident);
    if (!udata) {
      memset(ptr, 0, PROP_GetSize(property));
      property->stateflags &= ~(1<<state);
      Con_Error("No %s component in object %s(%s)", property->pdesc->TypeString, OBJ_GetName(object), OBJ_GetClassName(object));
      return ptr;
    }
    memcpy(ptr, &udata, PROP_GetSize(property));
  } else {
    memcpy(ptr, source, PROP_GetSize(property));
  }
  property->stateflags |= 1 << state;
  property->flags |= PF_MODIFIED;
  if (property->pdesc->FullIdentifier != ID_ContentOffset) {
    OBJ_SetDirty(property->object);
  } else {
    OBJ_SetFlags(property->object, OBJ_GetFlags(property->object) | OF_SCROLL);
  }
  return ptr;
}

void
PROP_SetValue(lpProperty_t property, void const* source)
{
  memcpy(property->value,
         PROP_SetStateValue(property, source, kPropertyStateNormal),
         PROP_GetSize(property));
}

static bool_t
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

static lpProperty_t
_PropertyAlloc(lua_State* L, lpObject_t object, lpcPropertyType_t pt)
{
  lpProperty_t property = ZeroAlloc(sizeof(struct Property) + pt->DataSize * PropertyState_Count);
  property->type    = pt->DataType;
  property->object  = object;
  property->pdesc   = pt;
  memset(property->states, 0xff, pt->DataSize * PropertyState_Count);
  _AssignCallback(L, property);
  return property;
}

lpProperty_t
PROP_Create(lua_State* L, lpObject_t object, lpcPropertyType_t pt)
{
  lpProperty_t property = _PropertyAlloc(L, object, pt);
  _RegisterProperty(object, property);
  return property;
}

lpProperty_t
OBJ_AddComponentProperty(lua_State* L, struct component* comp, lpcPropertyType_t desc)
{
  lpObject_t object = CMP_GetOwner(comp);
  lpProperty_t property = _PropertyAlloc(L, object, desc);
  PROP_AddToList(property, &_GetProperties(object));
  CMP_SetProperty(comp, property);
  return property;
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
          Con_Error("Unsupported value type in array table for property %s", p->pdesc->Name);
          free(mem);
          return 0;
      }
    }
    PROP_SetValue(p, &mem);
    return 0;
  }

  bool_t f_parse_property(lua_State*, lpProperty_t, lpcString_t);
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
        case kDataTypeStruct:
          if (!strcmp(p->pdesc->TypeString, "Color")) {
            f_parse_property(L, p, luaL_checkstring(L, idx));
            break;
          }
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
      PROP_SetValue(p, &number);
      break;
    case LUA_TNUMBER:
      switch (p->type) {
        case kDataTypeStruct:
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
        PROP_SetValue(p, &(lpObject_t ){luaX_checkObject(L, idx)});
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
      lua_pushstring(L, strlistget(*((int*)value), type->TypeString));
      break;
    case kDataTypeFloat:
      lua_pushnumber(L, *((float*)value));
      break;
    case kDataTypeStruct:
      memcpy(lua_newuserdata(L, type->DataSize), value, type->DataSize);
      luaL_setmetatable(L, type->TypeString);
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

static lpcString_t
PROP_GetShortName(lpcProperty_t property)
{
  if (property->pdesc->FullIdentifier != property->pdesc->ShortIdentifier) {
    return strrchr(property->pdesc->Name, '.') + 1;
  } else {
    return property->pdesc->Name;
  }
}

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
      OBJ_SendMessageW(object, kEventPropertyChanged, 0, &(struct PropertyChangedEventArgs){ .Property = property });
    }
    if (property->callbackEvent) {
      PROP_ExecuteChangedCallback(L, object, property);
    }
  }
}

void
PROP_AttachProgram(lpProperty_t p,
                   enum PropertyAttribute a,
                   struct token* program,
                   lpcString_t code)
{
  SafeSet(p->programSources[a], strdup(code), free);
  SafeSet(p->programs[a], program, Token_Release);
}

bool_t
PROP_HasProgram(lpProperty_t p)
{
  FOR_LOOP(i, PropertyAttribute_Count)
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
  return property->pdesc->Name;
}

lpcString_t
PROP_GetComponentName(lpcProperty_t property)
{
  return property->pdesc->Category;
}

uint32_t
PROP_GetSize(lpcProperty_t property)
{
  if (property->pdesc) {
    return property->pdesc->IsArray ? sizeof(void*) : (uint32_t)property->pdesc->DataSize;
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
// property->pdesc->Name); 	lpProperty_t callback = PROP_Create(L, object,
// callbackName, kDataTypeEvent, MAX_PROPERTY_STRING, NULL); 	property->callbackEvent
// = callback; 	lua_pushlightuserdata(L, callback); 	return 1;
// }

uint32_t
PROP_GetLongIdentifier(lpcProperty_t prop)
{
  return prop->pdesc->FullIdentifier;
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
    if (p->type == kDataTypeString) {
      FOR_LOOP(i, PropertyState_Count) {
        if (p->stateflags & (1 << i)) {
          free(*(LPSTR*)PROP_GetState(p, i));
        }
      }
    }
    FOR_LOOP(i, PropertyAttribute_Count)
    {
      if (p->programs[i]) {
        Token_Release(p->programs[i]);
      }
      if (p->programSources[i]) {
        free(p->programSources[i]);
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
      PROP_Clear(p);
      p->flags = (p->flags & ~PF_SPECIALIZED);
      memset(p->value, 0, PROP_GetSize(p));
    }
  }
}

lpcPropertyType_t 
PROP_GetDesc(lpcProperty_t prop) {
  return prop->pdesc;
}

#include <source/editor/ed_stab_property.h>
