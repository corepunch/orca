#include "object_internal.h"
#include "source/core/property/property_internal.h"

void CORE_SetKeyboardNavigation(bool_t enabled)
{
  if (core.keyboardNavigation == enabled) return;
  core.keyboardNavigation = enabled;
  if (core.focus)
    _SendMessage(core.focus, StyleController, ThemeChanged, .recursive = TRUE);
}

void OBJ_SetFocus(struct Object *pobj)
{
  if (core.focus == pobj || (pobj && (pobj->flags & OF_NOACTIVATE)))
    return;
  struct Object *previous = core.focus;
  core.focus = pobj;
  if (previous) {
    axPostMessageW(previous, ID_Node_KillFocus, 0, pobj);
    _SendMessage(previous, StyleController, ThemeChanged, .recursive = TRUE);
  }
  if (pobj) {
    axPostMessageW(pobj, ID_Node_SetFocus, 0, previous);
    _SendMessage(pobj, StyleController, ThemeChanged, .recursive = TRUE);
  }
}

struct Object *
core_GetFocus(void)
{
  if (core.focus && !OBJ_IsHidden(core.focus)) {
    return core.focus;
  } else {
    return NULL;
  }
}

bool_t OBJ_IsFocused(struct Object const *pobj) {
  return core.focus == pobj;
}

void OBJ_SetHover(struct Object *pobj) {
  core.hover2 = pobj;
}

struct Object *core_GetHover(void) {
  return core.hover;
}

bool_t
OBJ_AttachPropertyProgram(struct Object *self,
                          lpcString_t name,
                          lpcString_t program,
                          eBindingMode_t mode,
                          bool_t enabled)
{
  struct Property *property = strchr(name, '.') ? OBJ_FindLongProperty(self, fnv1a32(name))
                                               : OBJ_FindShortProperty(self, fnv1a32(name));
  return property && PROP_SetBinding(property, program, mode, enabled);
}
