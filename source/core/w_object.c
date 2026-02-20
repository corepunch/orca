#include <include/api.h>
#include <include/orca.h>

#include "core_local.h"

#define OBJECT_FUNCTION(NAME, FUNCTION)                                        \
  case f_##NAME:                                                               \
    lua_pushcfunction(L, API_Object##FUNCTION);                                \
    return 1;

// #define DEBUG_COUNT_OBJECTS

enum
{
  p_id = 0x37386ae0,
  p_Name= 0x0fe07306,
  p_parent = 0xeacdfcfd,
  p_children = 0x67a9c9d2,
  p_lineage = 0x31fcfa9e,
  p_uniqueID = 0x2837a4fb,
  p_className = 0xf4795adc,
  p_source_file = 0x27ba5603,
  p_focus = 0x14204413,
  p_hover = 0x70ceab2d,
	p_selected = 0x4e0a1774,
	p_class = 0xab3e0bff,
  p_screen = 0x55c54c11,
};

lpObject_t luaX_checkObject(lua_State* L, int arg) {
  if (lua_type(L, arg) == LUA_TTABLE) {
    luaX_parsefield(lpObject_t, __userdata, arg, luaL_checkudata, API_TYPE_OBJECT);
    return __userdata;
  } else {
    return luaL_checkudata(L, arg, API_TYPE_OBJECT);
  }
}

void luaX_pushObject(lua_State* L, lpcObject_t self)
{
  if (self && OBJ_GetLuaObject(self)) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, OBJ_GetLuaObject(self));
    assert(lua_type(L, -1)==LUA_TTABLE);
  } else {
    lua_pushnil(L);
  }
}

static void
_parse_args(lua_State* L, lpObject_t hobj)
{
  if (lua_type(L, 2) == LUA_TSTRING) {
    lpcString_t arg = luaL_checkstring(L, 2);
    if (*arg == '#') {
      xmlWith(char, dup, strdup(++arg), free)
      {
        OBJ_SetName(hobj, strtok(dup, "."));
        while ((arg = strtok(NULL, ".")))
          OBJ_AddClass(hobj, arg);
        lua_remove(L, 2);
      }
    } else if (*arg == '.') {
      xmlWith(char, dup, strdup(++arg), free)
      {
        OBJ_AddClass(hobj, strtok(dup, "."));
        while ((arg = strtok(NULL, ".")))
          OBJ_AddClass(hobj, arg);
        lua_remove(L, 2);
      }
    }
  }
  if (lua_type(L, 2) == LUA_TTABLE) {
    lua_pushnil(L);
    while (lua_next(L, 2)) {
      shortStr_t _key;
      strncpy(_key, luaL_checkstring(L, -2), sizeof(_key));
      lua_settable(L, 1);
      lua_pushstring(L, _key);
    }
    lua_remove(L, 2);
    OBJ_SendMessageW(hobj, kEventStart, 0, NULL);
  }
  if (lua_type(L, 2) == LUA_TFUNCTION) {
    lua_pushstring(L, "body");
    lua_pushvalue(L, 2);
    lua_settable(L, 1);
    lua_remove(L, 2);

    // TODO: a cleaner way? this clears "body" after populating once
    lua_pushboolean(L, 1);
    lua_setfield(L, 1, "__erasebody");
  }
  if (lua_type(L, 2) == LUA_TSTRING) {
    OBJ_SetTextContent(hobj, luaL_checkstring(L, 2));
    lua_remove(L, 2);
  }
  if (lua_isnil(L, 2)) {
    lua_remove(L, 2);
    return;
  }
  if (lua_gettop(L) > 1) {
    Con_Error("Can't parse argument of type %d, stack size %d",
               lua_type(L, 2),
               lua_gettop(L));
  }
}

static void
_assign_callbacks(lua_State* L, lpObject_t pobj, int idx)
{
	luaX_pushObject(L, pobj);
	int obj_index = lua_gettop(L);
  lua_pushvalue(L, idx);
  while (lua_type(L, -1) != LUA_TNIL) {
    lua_pushnil(L);
    while (lua_next(L, -2)) {
			shortStr_t _key;
			strncpy(_key, luaL_checkstring(L, -2), sizeof(_key));
			if (strncmp(_key, "__", 2)) {
				lua_settable(L, obj_index);
				lua_pushstring(L, _key);
			} else {
				lua_pop(L, 1);
			}
    }
    if (!lua_getmetatable(L, -1)) {
      break;
    }
    lua_remove(L, -2);
  }
  lua_pop(L, 2);
}

int f_new_Object(lua_State *L) {
  luaX_parsefield(lpcClassDesc_t, __nativeclass, 1, lua_touserdata);

  lpObject_t pobj = OBJ_Create(L, __nativeclass);
  lua_setfield(L, 1, "__userdata");

  // register object in registry
  lua_pushvalue(L, 1);
  OBJ_SetLuaObject(pobj, luaL_ref(L, LUA_REGISTRYINDEX));

  lpObject_t* ctx = lua_getextraspace(L);
  if (*ctx) {
    OBJ_AddChild(*ctx, pobj, FALSE);
  }

  // send "create" message
  OBJ_SendMessageW(pobj, kEventCreate, 0, L);
  
	_assign_callbacks(L, pobj, 1);
  _parse_args(L, pobj);

  // TODO: is there a better way to add class-default style?
  lua_getfield(L, 1, "apply");
  if (lua_type(L, -1) == LUA_TFUNCTION) {
    lua_pushvalue(L, 1);
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
      Con_Error("Can't execute apply()");
    } else {
      OBJ_ParseClassAttribute(pobj, luaL_checkstring(L, -1));
    }
  }
  lua_pop(L, 1);
  luaX_pushObject(L, pobj);
  return 1;
}

static int f_object_iterator(lua_State* L)
{
  lpObject_t* object = lua_touserdata(L, lua_upvalueindex(1));
  if ((*object) == NULL)
    return 0;
  luaX_pushObject(L, *object);
  *object = OBJ_GetNext(*object);
  return 1;
}

static int f_object_lineage(lua_State* L)
{
  lpObject_t* object = lua_touserdata(L, lua_upvalueindex(1));
  if ((*object) == NULL)
    return 0;
  luaX_pushObject(L, *object);
  *object = OBJ_GetParent(*object);
  return 1;
}

// static int API_PropertiesIterator(lua_State *L) {
//	lpProperty_t *property = lua_touserdata(L, lua_upvalueindex(1));
//	if ((*property) == NULL)
//		return 0;
//	lua_pushstring(L, PROP_GetName(*property));
//	*property = (*property)->next;
//	return 1;
// }

bool_t OBJ_Equals(lpcObject_t self, lpcObject_t other)
{
  return self == other;
}

static lpcString_t PascalCase(lpcString_t pname) {
  // convert to PascalCase
  static shortStr_t tmp;
  if (islower(*pname)) {
    strncpy(tmp, pname, sizeof(tmp));
    *tmp = toupper(*tmp);
    pname = tmp;
  }
  return pname;
}

#include <source/UIKit/UIKit.h>
bool_t OBJ_API(SetProperty, lpcString_t name) {
  switch (fnv1a32(name)) {
    case p_id:
		case p_Name:
			OBJ_SetName(self, luaL_checkstring(L, 3));
			return TRUE;
		case p_selected: {
			uint32_t flags = OBJ_GetFlags(self);
			if (lua_toboolean(L, 3)) {
				if (!(flags & OF_SELECTED)) {
					OBJ_SetFlags(self, flags | OF_SELECTED);
					OBJ_ApplyStyles(self, TRUE);
				}
			} else if ((flags & OF_SELECTED)) {
					OBJ_SetFlags(self, flags & ~OF_SELECTED);
					OBJ_ApplyStyles(self, TRUE);
			}
			return TRUE;
		}
		case p_class:
			if (lua_type(L, 3) == LUA_TSTRING) {
				OBJ_ParseClassAttribute(self, luaL_checkstring(L, 3));
				return TRUE;
			} else {
				luaL_checktype(L, 3, LUA_TTABLE);
				lua_pushnil(L);
				while (lua_next(L, 3)) {
					struct style_class* _ParseClass(lpcString_t str);
					void _AddClass(lpObject_t obj, struct style_class* cls);
					_AddClass(self, _ParseClass(luaL_checkstring(L, -1)));
					lua_pop(L, 1);
				}
				lua_pop(L, 1);
				return TRUE;
			}
  }
  if (!strcmp(name, "modal")) {
    OBJ_SetModal(self, luaX_checkObject(L, 3));
    OBJ_Awake(L, luaX_checkObject(L, 3));
    OBJ_SendMessageW(luaX_checkObject(L, 3), kEventAttached, 0, self);
    return TRUE;
  }
  if (!strcmp(name, "x") && GetNode2D(self)) {
    GetNode2D(self)->LayoutTransform.translation.x = luaL_checknumber(L, 3);
    return TRUE;
  }
  if (!strcmp(name, "y") && GetNode2D(self)) {
    GetNode2D(self)->LayoutTransform.translation.y = luaL_checknumber(L, 3);
    return TRUE;
  }
  // convert to PascalCase
  lpProperty_t property = NULL;
  if (SUCCEEDED(OBJ_FindShortProperty(self, PascalCase(name), &property))) {
    luaX_readProperty(L, 3, property);
    return TRUE;
  } else if (lua_type(L, 3) == LUA_TSTRING) {
    property = PROP_Create(L, self, name, kDataTypeLongString, NULL);
    luaX_readProperty(L, 3, property);
    return TRUE;
  } else if (lua_type(L, 3) == LUA_TNUMBER) {
    property = PROP_Create(L, self, name, kDataTypeFloat, NULL);
    luaX_readProperty(L, 3, property);
    return TRUE;
  } else if (lua_type(L, 3) == LUA_TBOOLEAN) {
    property = PROP_Create(L, self, name, kDataTypeBool, NULL);
    luaX_readProperty(L, 3, property);
    return TRUE;
  } else if (luaL_testudata(L, 3, API_TYPE_OBJECT)) {
    property = PROP_Create(L, self, name, kDataTypeObject, "Object");
    luaX_readProperty(L, 3, property);
    return TRUE;
  } else if (!strcmp(name, "Material.Texture")) {
    property = PROP_Create(L, self, name, kDataTypeObject, "Texture");
    luaX_readProperty(L, 3, property);
    return TRUE;
  } else if (lua_type(L, 3) == LUA_TFUNCTION) {
    OBJ_ProcessFunctions(self, name);
    return FALSE;
  } else {
    return FALSE;
  }
}

void OBJ_API(PostMessage, lpcString_t message)
{
  SV_PostMessage(self, message, 0, lua_touserdata(L, 3));
}

lpObject_t OBJ_API(DispatchEvent, lpcString_t event)
{
  uint32_t dwNumArgs = MAX(0, lua_gettop(L) - 2);
  shortStr_t pszEventName;
  strncpy(pszEventName, event, sizeof(pszEventName));
  lua_remove(L, 2); // clear event name to send object with args to parents
  for (lpObject_t obj = self; obj; obj = OBJ_GetParent(obj)) {
    HANDLEMESSAGESTRUCT event = {
      .L = L,
      .EventName = pszEventName,
      .FirstArg = 1,
      .NumArgs = dwNumArgs + 1,
    };
    if (OBJ_SendMessage(obj, "HandleMessage", 0, &event)) {
      return obj;
    }
  }
  return NULL;
}

static int __paint(lua_State *L) {
  lpObject_t o=luaX_checkObject(L, 1);
  int width = luaL_checknumber(L, 2);
  int height = luaL_checknumber(L, 3);
  API_CallRequire(L, "orca.renderer", 1);
  lua_getfield(L, -1, "getSize");
  WI_PostMessageW(o, kEventWindowPaint, MAKEDWORD(width, height), NULL);
  return 0;
}

int OBJ_API(GetProperty, lpcString_t name)
{
  uint32_t ident = fnv1a32(name);
  if (!strcmp(name, "paint")) {
    lua_pushcfunction(L, __paint);
    return 1;
  }
  switch (ident) {
    case p_id:
    case p_Name:
      if (OBJ_GetName(self)) {
        lua_pushstring(L, OBJ_GetName(self));
      } else {
        lua_pushnil(L);
      }
      return 1;
    case p_className:
      lua_pushstring(L, OBJ_GetClassName(self));
      return 1;
    case p_source_file:
      lua_pushstring(L, OBJ_GetSourceFile(self));
      return 1;
    case p_parent:
      if (OBJ_GetParent(self)) {
        luaX_pushObject(L, OBJ_GetParent(self));
      } else {
        lua_pushnil(L);
      }
      return 1;
    case p_uniqueID:
      lua_pushinteger(L, OBJ_GetUniqueID(self));
      return 1;
    case p_children:
      *(lpObject_t*)lua_newuserdata(L, sizeof(lpObject_t*)) =
        OBJ_GetFirstChild(self);
      lua_pushcclosure(L, f_object_iterator, 1);
      return 1;
    case p_lineage:
      *(lpObject_t*)lua_newuserdata(L, sizeof(lpObject_t*)) = self;
      lua_pushcclosure(L, f_object_lineage, 1);
      return 1;
    case p_focus:
      lua_pushboolean(L, core_GetFocus() == self);
      return 1;
    case p_hover:
      lua_pushboolean(L, core_GetHover() == self);
      return 1;
		case p_selected:
			lua_pushboolean(L, OBJ_GetFlags(self) & OF_SELECTED);
			return 1;
    case p_screen: {
#define ID_Screen 0x9bd8c631
      if ((self=OBJ_FindParentOfClass(self, ID_Screen))) luaX_pushObject(L, self);
      else lua_pushnil(L);
      return 1;
    }
    default:
      break;
  }

#define kEventPushProperty 0xc5ebaf40
  int found = OBJ_SendMessageW(self, kEventPushProperty, ident, L);
  if (found) {
    return found;
  }
  
  lpProperty_t property = NULL;
  bool_t OBJ_PushClassProperty(lua_State *, lpObject_t, uint32_t);
  
  if (OBJ_PushClassProperty(L, self, ident)) {
    return 1;
  } else if ((property = PROP_FindByShortID(OBJ_GetProperties(self), ident))) {
    luaX_pushProperty(L, property);
    return 1;
  } else {
    lua_pushnil(L);
    return 1;
  }

#if 0
  
  if (SUCCEEDED(OBJ_FindShortProperty(self, PascalCase(name), &property))) {
    luaX_pushProperty(L, property);
    return 1;
  } else if (SUCCEEDED(OBJ_FindShortProperty(self, name, &property))) {
    luaX_pushProperty(L, property);
    return 1;
  } else {
    // lpObject_t  child = OBJ_FindImmediateChild(self,
    // identifier); if (child) {
    //     lua_geti(L, LUA_REGISTRYINDEX, child->luaObject);
    //     return 1;
    // }
    // lua_getfield(L, -1, name);
    lua_pushnil(L);
    return 1;
  }
#endif
}

//static LUAFX(string, translate)
//{
//  if (!strchr(string_1, '/')) {
//    lpcString_t s = Loc_GetString(GetLocalization(L), string_1, LOC_RESOURCE);
//    lua_pushstring(L, s);
//  } else {
//    lua_pushvalue(L, 1);
//  }
//  return 1;
//}

void OBJ_API(SetContext)
{
  lpObject_t* ctx = lua_getextraspace(L);
  *ctx = self;
}

void OBJ_Play(lpObject_t self, lpcString_t animation)
{
//  OBJ_SetAnimation(self, string_2);
}

void OBJ_API(Bind, lpcString_t property, lpcString_t expression) {
  lpProperty_t hProperty = NULL;
  if (FAILED(OBJ_FindShortProperty(self, property, &hProperty))) {
    hProperty = PROP_Create(L, self, property, kDataTypeNone, NULL);
  }
//  PROP_AttachProgram(hProperty, 0, expression, TRUE);
  // TODO: implement binding in scripts
  Con_Error("Binding not implemented");
}
