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

static void collect_form_inputs(lua_State* L, lpObject_t node, int table_idx)
{
  if (!node) return;

  lpcString_t cls = OBJ_GetClassName(node);
  if (cls && !strcmp(cls, "Input")) {
    lpcString_t name = OBJ_GetName(node);
    if (name && *name) {
      OBJ_GetProperty(L, node, "Text");
      if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_pushstring(L, "");
      }
      lua_setfield(L, table_idx, name);
    }
  }

  FOR_EACH_OBJECT(child, node) {
    collect_form_inputs(L, child, table_idx);
  }
}

static int f_obj_populate_inputs(lua_State* L)
{
  lpObject_t self = luaX_checkObject(L, 1);
  lua_newtable(L);
  int table_idx = lua_absindex(L, -1);
  collect_form_inputs(L, self, table_idx);
  return 1;
}

static bool_t
capitalize_first_char(char* out, size_t out_size, lpcString_t name)
{
  if (!name || !*name || out_size < 2) {
    return FALSE;
  }
  snprintf(out, out_size, "%s", name);
  if (out[0] >= 'a' && out[0] <= 'z') {
    out[0] = (char)(out[0] - ('a' - 'A'));
    return TRUE;
  }
  return FALSE;
}

static bool_t
is_on_changed_callback_name(lpcString_t name)
{
  if (!name || strncmp(name, "on", 2) != 0 || !name[2]) {
    return FALSE;
  }
  size_t len = strlen(name);
  return len > 7 && strcmp(name + len - 7, "Changed") == 0;
}

static bool_t
ensure_event_property(lua_State* L, lpObject_t self, lpcString_t name, lpProperty_t* out)
{
  if (SUCCEEDED(OBJ_FindShortProperty(self, name, out)) && PROP_GetType(*out) == kDataTypeEvent) {
    return TRUE;
  }

  struct PropertyType pt = {
    .ShortIdentifier = fnv1a32(name),
    .FullIdentifier = fnv1a32(name),
    .DataType = kDataTypeEvent,
    .DataSize = sizeof(void*),
    .Name = strdup(name),
    .Category = "",
  };
  if (!pt.Name) {
    return FALSE;
  }
  OBJ_RegisterPropertyType(&pt);
  return SUCCEEDED(OBJ_FindShortProperty(self, name, out)) && PROP_GetType(*out) == kDataTypeEvent;
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
        _SendMessage(self, StyleController, AddClasses, luaL_checkstring(L, 3));
				return TRUE;
			} else {
				luaL_checktype(L, 3, LUA_TTABLE);
				lua_pushnil(L);
				while (lua_next(L, 3)) {
          _SendMessage(self, StyleController, AddClass, luaL_checkstring(L, -1));
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
  if (SUCCEEDED(OBJ_FindShortProperty(self, name, &property))) {
    luaX_readProperty(L, 3, property);
    return TRUE;
  } else if (lua_type(L, 3) == LUA_TFUNCTION) {
    /* Allow lowercase method-style callback binding to event properties.
     * Example: obj.loadView = fn -> binds Node.LoadView event property. */
    shortStr_t alias = {0};
    if (capitalize_first_char(alias, sizeof(alias), name) &&
        SUCCEEDED(OBJ_FindShortProperty(self, alias, &property)) &&
        PROP_GetType(property) == kDataTypeEvent) {
      luaX_readProperty(L, 3, property);
      return TRUE;
    }

    void OBJ_RegisterPropertyChangedCallback(lpObject_t object, lpcString_t name);
    if (is_on_changed_callback_name(name)) {
      OBJ_RegisterPropertyChangedCallback(self, name);
      if (ensure_event_property(L, self, name, &property)) {
        luaX_readProperty(L, 3, property);
        return TRUE;
      }
    }
    return FALSE;
  } else {
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
  /* For method-style lowercase names (e.g. loadView), allow binding to
   * same-named event properties (LoadView) before checking metatable methods. */
  shortStr_t callback_alias = {0};
  lpProperty_t callback_prop = NULL;
  if (capitalize_first_char(callback_alias, sizeof(callback_alias), name) &&
      SUCCEEDED(OBJ_FindShortProperty(self, callback_alias, &callback_prop)) &&
      PROP_GetType(callback_prop) == kDataTypeEvent) {
    luaX_pushProperty(L, callback_prop);
    return 1;
  }

  if (!strcmp(name, "populateInputs")) {
    lua_pushcfunction(L, f_obj_populate_inputs);
    return 1;
  }

  /* Resolve methods first so obj.method style access remains intact. */
  luaL_getmetatable(L, API_TYPE_OBJECT);
  lua_getfield(L, -1, name);
  lua_remove(L, -2);
  if (!lua_isnil(L, -1)) {
    return 1;
  }
  lua_pop(L, 1);

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
    shortStr_t alias = {0};
    if (capitalize_first_char(alias, sizeof(alias), name)) {
      uint32_t alias_ident = fnv1a32(alias);
      if (OBJ_PushClassProperty(L, self, alias_ident)) {
        return 1;
      } else if ((property = PROP_FindByShortID(OBJ_GetProperties(self), alias_ident))) {
        luaX_pushProperty(L, property);
        return 1;
      }
    }
    lua_pushnil(L);
    return 1;
  }
}
