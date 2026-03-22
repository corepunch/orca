#include "object_internal.h"

void
OBJ_EmitPropertyChangedEvents(lua_State* L, lpObject_t object)
{
  SM_EnsureStateManagerInitialized(object);
  PROP_ProcessEvents(L, OBJ_GetProperties(object), object);
  OBJ_UpdateTimers(object);
  FOR_EACH_OBJECT(it, object) OBJ_EmitPropertyChangedEvents(L, it);
}

void
PROP_ExecuteChangedCallback(lua_State* L,
                            lpObject_t pobj,
                            lpProperty_t hProperty)
{
  lpcString_t szCallback = PROP_GetCallbackEvent(hProperty);
  lpObject_t hRoot = OBJ_GetRoot(pobj);
  if (hRoot) {
    lua_geti(L, LUA_REGISTRYINDEX, OBJ_GetLuaObject(pobj));
    luaX_pushProperty(L, hProperty);
    luaX_executecallback(L, hRoot, szCallback, 2);
  }
}

void
OBJ_Awake(lua_State* L, lpObject_t object)
{
  if (!(object->flags & OF_UPDATED_ONCE)) {
    lpcString_t cb = OBJ_FindCallbackForID(object, kEventAwake);
    if (cb) {
      luaX_pushObject(L, object);
      lua_getfield(L, -1, cb);
      lua_insert(L, -2); // Move callback before obj
      if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        Con_Error("%s", luaL_checkstring(L, -1));
        lua_pop(L, 1);
      }
    }
    OBJ_SendMessageW(object, kEventAwake, 0, NULL);
    OBJ_ApplyStyles(object, FALSE);
    object->flags |= OF_UPDATED_ONCE;
  }
  FOR_EACH_OBJECT(child, object) OBJ_Awake(L, child);
}

void OBJ_SetFocus(lpObject_t pobj)
{
  if (core.focus == pobj || (pobj && (pobj->flags & OF_NOACTIVATE)))
    return;
  if (core.focus) {
    SV_PostMessage(core.focus, "KillFocus", 0, pobj);
    SV_PostMessage(core.focus, "ThemeChanged", 0, 0);
  }
  if (pobj) {
    SV_PostMessage(pobj, "SetFocus", 0, core.focus);
    SV_PostMessage(pobj, "ThemeChanged", 0, 0);
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
