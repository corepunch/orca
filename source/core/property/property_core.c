#include "property_internal.h"

void
Token_Release(struct token* token)
{
  SafeDelete(token->next, Token_Release);
  free(token);
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

void
PROP_SetTypeSize(lpProperty_t p, eDataType_t t, uint32_t s)
{
  p->type = t;
}

void
PROP_SetValuePtr(lpProperty_t prop, void* data)
{
  prop->value = data;
}

#include <source/editor/ed_stab_property.h>
