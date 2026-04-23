#include "object_internal.h"

void
PROP_ExecuteChangedCallback(lua_State* L,
                            lpObject_t pobj,
                            lpProperty_t hProperty)
{
  lpcString_t szCallback = PROP_GetCallbackMsg(hProperty);
  lpObject_t hRoot = OBJ_GetRoot(pobj);
  if (hRoot) {
    luaX_pushObject(L, pobj);
    luaX_pushProperty(L, hProperty);
    luaX_executecallback(L, hRoot, szCallback, 2);
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
