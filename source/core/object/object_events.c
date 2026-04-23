#include "object_internal.h"

static bool_t
invoke_event_property_from_object_events(lua_State* L, lpObject_t object, lpProperty_t callback, int num_args)
{
  if (!callback || PROP_GetType(callback) != kDataTypeEvent) {
    lua_pop(L, num_args);
    return FALSE;
  }
  event_t ref = *(event_t*)PROP_GetValue(callback);
  if (!ref) {
    lua_pop(L, num_args);
    return FALSE;
  }

  lua_geti(L, LUA_REGISTRYINDEX, ref);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1 + num_args);
    return FALSE;
  }

  lua_insert(L, -(num_args + 1));
  luaX_pushObject(L, object);
  lua_insert(L, -(num_args + 1));

  if (lua_pcall(L, num_args + 1, 1, 0) != LUA_OK) {
    Con_Error("property callback: %s", lua_tostring(L, -1));
    lua_pop(L, 1);
    return FALSE;
  }
  bool_t ret = lua_toboolean(L, -1);
  lua_pop(L, 1);
  return ret;
}

void
OBJ_EmitPropertyChangedEvents(lua_State* L, lpObject_t object)
{
  PROP_ProcessEvents(L, OBJ_GetProperties(object), object);
  FOR_EACH_OBJECT(it, object) OBJ_EmitPropertyChangedEvents(L, it);
}

void
PROP_ExecuteChangedCallback(lua_State* L,
                            lpObject_t pobj,
                            lpProperty_t hProperty)
{
  lpcString_t szCallback = PROP_GetCallbackMsg(hProperty);
  lpObject_t hRoot = OBJ_GetRoot(pobj);
  if (hRoot && szCallback && *szCallback) {
    lpProperty_t callback = NULL;
    if (FAILED(OBJ_FindShortProperty(hRoot, szCallback, &callback))) {
      return;
    }
    luaX_pushObject(L, pobj);
    luaX_pushProperty(L, hProperty);
    invoke_event_property_from_object_events(L, hRoot, callback, 2);
  }
}

void OBJ_SetFocus(lpObject_t pobj)
{
  if (core.focus == pobj || (pobj && (pobj->flags & OF_NOACTIVATE)))
    return;
  if (core.focus) {
    axPostMessageW(core.focus, ID_Node_KillFocus, 0, pobj);
    _SendMessage(core.focus, StyleController, ThemeChanged, .recursive = FALSE);
  }
  if (pobj) {
    axPostMessageW(pobj, ID_Node_SetFocus, 0, core.focus);
    _SendMessage(pobj, StyleController, ThemeChanged, .recursive = FALSE);
  }
  core.focus = pobj;
}

lpObject_t
core_GetFocus(void)
{
  if (core.focus && !OBJ_IsHidden(core.focus)) {
    return core.focus;
  } else {
    return NULL;
  }
}

bool_t OBJ_IsFocused(lpcObject_t pobj) {
  return core.focus == pobj;
}

void OBJ_SetHover(lpObject_t pobj) {
  core.hover2 = pobj;
}

lpObject_t core_GetHover(void) {
  return core.hover;
}

bool_t
OBJ_AttachPropertyProgram(lpObject_t self,
                          lpcString_t name,
                          lpcString_t program,
                          ePropertyAttribute_t attribute,
                          eBindingMode_t mode,
                          bool_t enabled)
{
  struct token *compiled = enabled ? Token_Create(program) : NULL;
  struct Property *property = NULL;
  if (SUCCEEDED(OBJ_FindLongProperty(self, fnv1a32(name), &property))) {
    PROP_AttachProgram(property, attribute, compiled, program);
    return TRUE;
  } else {
    return FALSE;
  }
}
