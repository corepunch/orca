#include "property_internal.h"

static lpcString_t
PROP_GetShortName(struct Property const *property)
{
  return property->pdesc->Name;
}

static bool_t
invoke_changed_callback_from_property_events(lua_State* L, struct Object *object, struct Property *property, int num_args)
{
  if (!property || !property->changeCallback) {
    lua_pop(L, num_args);
    return FALSE;
  }

  lua_geti(L, LUA_REGISTRYINDEX, property->changeCallback);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1 + num_args);
    return FALSE;
  }

  lua_insert(L, -(num_args + 1)); /* func below args */
  luaX_pushObject(L, object);
  lua_insert(L, -(num_args + 1)); /* self below args */

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
PROP_FireNotification(lua_State* L,
                      struct Property *property,
                      struct Object *object)
{
  if (!property || !PROP_HasHandler(property)) return;
  // Clear the queued flag before firing so that cascaded PROP_SetValue calls
  // (e.g. from state-manager applying a state) can re-enqueue the property.
  property->flags &= ~PF_NOTIFICATION_QUEUED;
  PROP_Update(property);
  if (property->flags & PF_USED_IN_STATE_MANAGER) {
    _SendMessage(object, StateManagerController, ControllerChanged, .Property = property);
  }
  if (property->flags & PF_HASCHANGECALLBACK) {
    static path_t str;
    sprintf(str, ON_CHANGED_CALLBACK, PROP_GetShortName(property));
    luaX_pushProperty(L, property);
    invoke_changed_callback_from_property_events(L, object, property, 1);
  }
  if (property->flags & PF_USED_IN_TRIGGER) {
    _SendMessage(object, Object, PropertyChanged, .Property = property);
  }
}

void
PROP_AttachProgram(struct Property *p,
                   struct token* program)
{
  if (p->binding) {
    SafeSet(p->binding->token, program, Token_Release);
    return;
  }
  struct Object *binding_object = NULL;
  if (OBJ_FindClassW(ID_Binding)) {
    binding_object = OBJ_Create(ID_Binding);
    if (binding_object) {
      OBJ_AddRef(binding_object);
      p->binding = GetBinding(binding_object);
    }
  }
  if (!p->binding) {
    p->binding = ZeroAlloc(sizeof(struct Binding));
    if (!p->binding) {
      if (binding_object) {
        OBJ_ReleaseRef(binding_object);
      }
      SafeDelete(program, Token_Release);
      return;
    }
  }

  p->binding->property = p;
  p->binding->owner = binding_object;
  p->binding->token = program;

  if (!p->inBindingIndex) {
    p->nextBinding = core.binding_properties;
    core.binding_properties = p;
    p->inBindingIndex = TRUE;
  }
}

ORCA_API bool_t
PROP_SetBinding(struct Property *property,
                lpcString_t expression,
                eBindingMode_t mode,
                bool_t enabled)
{
  (void)mode;
  if (!property) {
    Con_Error("PROP_SetBinding: target property is NULL");
    return FALSE;
  }

  if (!expression || !*expression) {
    Con_Error("PROP_SetBinding: empty binding expression for %s/%s",
              OBJ_GetClassName(property->object),
              PROP_GetName(property));
    return FALSE;
  }

  struct token *compiled = enabled ? Token_Create(expression) : NULL;
  PROP_AttachProgram(property, compiled);
  return TRUE;
}

bool_t
PROP_HasProgram(struct Property *p)
{
  return p && p->binding && p->binding->token != NULL;
}
