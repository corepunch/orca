#include "object_internal.h"
#include "source/core/property/property_internal.h"

void OBJ_SetFocus(struct Object *pobj)
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
  (void)mode;
  struct token *compiled = enabled ? Token_Create(program) : NULL;
  struct Property *property = OBJ_FindLongProperty(self, fnv1a32(name));
  if (property) {
    PROP_AttachProgram(property, compiled);
    if (property->binding) {
      free((char *)property->binding->Expression);
      property->binding->Expression = strdup(program);
      property->binding->updateFrame = (uint32_t)-1;
    }
    PROP_SetFlag(property, PF_MODIFIED);
    property->updateFrame = core.frame == 0 ? (uint32_t)-1 : core.frame - 1;
    return TRUE;
  } else {
    return FALSE;
  }
}
