#include "property_internal.h"

void
Token_Release(struct token* token)
{
  SafeDelete(token->next, Token_Release);
  FOR_LOOP(i, TOKEN_MAX_ARGS)
  {
    SafeDelete(token->args[i], Token_Release);
  }
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
  FOR_EACH_LIST(struct property_program, pp, core.programs)
  {
    if (pp->property != property) continue;
    if (pp->updateFrame == core.frame) continue;
    pp->updateFrame = core.frame;
    struct vm_register r = { 0 };
    if (!OBJ_RunProgram(property->object, pp->token, &r) ||
        !PROP_Import(property, pp->attr, &r)) {
#ifdef DEBUG_PROGRAM
      print_name(property->object);
#endif
      Con_Error("Eror in program %s/%s", OBJ_GetName(property->object), property->pdesc->Name);
      REMOVE_FROM_LIST(struct property_program, pp, core.programs);
      Token_Release(pp->token);
      free(pp->code);
      free(pp);
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

/*static */lpProperty_t
_PropertyAlloc(lua_State* L, lpObject_t object, lpcPropertyType_t pt)
{
  (void)L;
  lpProperty_t property = ZeroAlloc(sizeof(struct Property) + pt->DataSize * PropertyState_Count);
  property->type    = pt->DataType;
  property->object  = object;
  property->pdesc   = pt;
  memset(property->states, 0xff, pt->DataSize * PropertyState_Count);
  return property;
}

lpProperty_t
PROP_Create(lua_State* L, lpObject_t object, lpcPropertyType_t pt)
{
  lpProperty_t property = _PropertyAlloc(L, object, pt);
  _RegisterProperty(object, property);
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
    FOR_EACH_LIST(struct property_program, pp, core.programs) {
      if (pp->property == p) {
        REMOVE_FROM_LIST(struct property_program, pp, core.programs);
        Token_Release(pp->token);
        free(pp->code);
        free(pp);
      }
    }
    free(p);
  }
}

void
PROP_RunAllPrograms(void)
{
  FOR_EACH_LIST(struct property_program, pp, core.programs)
  {
    if (pp->updateFrame == core.frame) continue;
    pp->updateFrame = core.frame;
    pp->property->updateFrame = core.frame;
    struct vm_register r = { 0 };
    if (!OBJ_RunProgram(pp->property->object, pp->token, &r) ||
        !PROP_Import(pp->property, pp->attr, &r)) {
      Con_Error("Eror in program %s/%s", OBJ_GetName(pp->property->object), pp->property->pdesc->Name);
      REMOVE_FROM_LIST(struct property_program, pp, core.programs);
      Token_Release(pp->token);
      free(pp->code);
      free(pp);
    }
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

ORCA_API void
PDESC_Print(lpcPropertyType_t pdesc, LPSTR buffer, uint32_t len, float const* pf)
{
  switch (pdesc->DataType) {
    case kDataTypeBool:
      strcpy(buffer, *(int*)pf ? "true" : "false");
      break;
    case kDataTypeInt:
      snprintf(buffer, len, "%dd", *(int*)pf);
      break;
    case kDataTypeEnum:
      strncpy(buffer, pdesc->EnumValues[*(int*)pf], len);
      break;
    case kDataTypeColor: {
      byte_t r = pf[0] * 255, g = pf[1] * 255, b = pf[2] * 255, a = pf[3] * 255;
      if (pf[3] < 1) {
        snprintf(buffer, len, "#%02x%02x%02x%02x", a, r, g, b);
      } else {
        snprintf(buffer, len, "#%02x%02x%02x", r, g, b);
      }
      break;
    }
    case kDataTypeStruct:
    case kDataTypeFloat:
      snprintf(buffer, len, "%g", pf[0]);
      FOR_LOOP(i, (int)pdesc->DataSize/sizeof(float)-1) {
        size_t buflen = strlen(buffer);
        snprintf(buffer + buflen, len - buflen, " %g", pf[i+1]);
      }
      break;
    case kDataTypeString:
      if (pf) {
        strncpy(buffer, (lpcString_t)pf, len);
      }
      break;
    case kDataTypeObject:
      if (*(handle_t *)pf) {
        lpcString_t s = "nil";
        if (s) {
          strncpy(buffer, s, len);
        } else {
          *buffer = 0;
        }
      } else {
        strncpy(buffer, "nil", len);
      }
      break;
    default:
      Con_Error("Unknown type %d in property %s\n", pdesc->DataType, pdesc->Name);
      break;
  }
}

ORCA_API void
PROP_Print(lpProperty_t p, LPSTR buffer, uint32_t len)
{
  PDESC_Print(p->pdesc, buffer, len, PROP_GetValue(p));
}
