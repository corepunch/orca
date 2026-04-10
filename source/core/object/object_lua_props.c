#include <include/api.h>
#include <include/orca.h>

#include <source/core/core_local.h>

enum
{
  p_id = 0x37386ae0,
  p_Name = 0x0fe07306,
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

#include <plugins/UIKit/UIKit.h>
#include <source/filesystem/filesystem.h>
bool_t
OBJ_SetProperty(lua_State* L, lpObject_t self, lpcString_t name)
{
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
					_SendMessage(self, StyleController, ThemeChanged, .recursive = TRUE);
				}
			} else if ((flags & OF_SELECTED)) {
					OBJ_SetFlags(self, flags & ~OF_SELECTED);
					_SendMessage(self, StyleController, ThemeChanged, .recursive = TRUE);
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
  if (!strcmp(name, "x") && GetNode2D(self)) {
    GetNode2D(self)->LayoutTransform.translation.x = luaL_checknumber(L, 3);
    return TRUE;
  }
  if (!strcmp(name, "y") && GetNode2D(self)) {
    GetNode2D(self)->LayoutTransform.translation.y = luaL_checknumber(L, 3);
    return TRUE;
  }
  lpProperty_t property = NULL;
  if (lua_type(L, 3) == LUA_TTABLE) { // store table for safekeeping
    luaX_parsefield(lpObject_t, __userdata, 3, luaL_testudata, API_TYPE_OBJECT);
    if (__userdata) {
      lua_pushfstring(L, "__hook_%s", name);
      lua_pushvalue(L, 3);
      lua_rawset(L, 1);
    }
  }
  if (SUCCEEDED(OBJ_FindShortProperty(self, name, &property))) {
    luaX_readProperty(L, 3, property);
    return TRUE;
  } else if (lua_type(L, 3) == LUA_TFUNCTION) {
    void OBJ_RegisterPropertyChangedCallback(lpObject_t object, lpcString_t name);
    OBJ_RegisterPropertyChangedCallback(self, name); // TODO: move to implocit callbacks registration?
    return FALSE;
  } else {
//    fprintf(stderr, "Can't find property %s\n", name);
    return FALSE;
  }
}

int f_msgSend(lua_State *L) {
  struct Object* this_ = luaX_checkObject(L, 1);
  const char* message = lua_tostring(L, lua_upvalueindex(1));
  lua_pushstring(L, message);
  lua_insert(L, 2);
  return OBJ_send(L, this_, message); // sync+direct, returns value or nil
}

int OBJ_GetProperty(lua_State* L, lpObject_t self, lpcString_t name)
{
  uint32_t ident = fnv1a32(name);
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
  
  if (!strcmp(name, "ActualX") && GetNode2D(self)) {
    lua_pushnumber(L, GetNode2D(self)->_actual_pos[0]);
    return 1;
  }
  if (!strcmp(name, "ActualY") && GetNode2D(self)) {
    lua_pushnumber(L, GetNode2D(self)->_actual_pos[1]);
    return 1;
  }
  
  lpcProperty_t property = NULL;
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
}
