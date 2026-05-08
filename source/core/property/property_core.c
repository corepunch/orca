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

struct Property *
PROP_AddToList(struct Property *property, struct Property ** list)
{
  ADD_TO_LIST(property, *list);
  return property;
}

//#define DEBUG_PROGRAM

#ifdef DEBUG_PROGRAM
static void
print_name(struct Object *object)
{
  if (OBJ_GetParent(object)) {
    print_name(OBJ_GetParent(object));
  }
  Con_Error("%s/", OBJ_GetName(object));
}
#endif

bool_t
PROP_Update(struct Property *property)
{
  if (property == NULL ||
      property->updateFrame == core.frame)
    return FALSE;
  property->updateFrame = core.frame;
  FOR_EACH_LIST(struct property_program, pp, core.programs)
  {
    if (pp->property != property) continue;
    if (!pp->token) continue; // disabled / failed to compile
    if (pp->updateFrame == core.frame) continue;
    pp->updateFrame = core.frame;
    struct vm_register r = { 0 };
    if (!OBJ_RunProgram(property->object, pp->token, &r)) {
#ifdef DEBUG_PROGRAM
      print_name(property->object);
#endif
      Con_Error("Binding update failed while running program for %s/%s",
                OBJ_GetName(property->object),
                property->pdesc->Name);
      REMOVE_FROM_LIST(struct property_program, pp, core.programs);
      SafeDelete(pp->token, Token_Release);
      free(pp->code);
      free(pp);
    } else if (!PROP_Import(property, pp->attr, &r)) {
#ifdef DEBUG_PROGRAM
      print_name(property->object);
#endif
      Con_Error("Binding update failed while importing into %s/%s",
                OBJ_GetName(property->object),
                property->pdesc->Name);
      REMOVE_FROM_LIST(struct property_program, pp, core.programs);
      SafeDelete(pp->token, Token_Release);
      free(pp->code);
      free(pp);
    }
  }
  return TRUE;
}

static void
_ReleaseArrayItems(struct Property *property)
{
  if (!property->pdesc->IsArray || !property->value) {
    return;
  }
  if (property->pdesc->DataType != kDataTypeObject) {
    return;
  }
  void **items = *(void**)property->value;
  int count = ((int*)property->value)[sizeof(void*)/sizeof(int)];
  FOR_LOOP(i, count) {
    if (items && items[i]) {
      OBJ_ReleaseRef((struct Object*)items[i]);
    }
  }
}

bool_t
PROP_IsNull(struct Property const *property)
{
  return property == NULL || !(property->flags & PF_MODIFIED);
}

void
PROP_Clear(struct Property *property)
{
  void *old_array = NULL;
  if (property->pdesc->DataType == kDataTypeString && property->value && *(LPSTR*)property->value) {
    free(*(LPSTR*)property->value);
  }
  if (property->pdesc->IsArray && property->value) {
    old_array = *(void**)property->value;
  }
  _ReleaseArrayItems(property);
  if (old_array) {
    free(old_array);
  }
  if (property->pdesc->DataType == kDataTypeObject && property->value && !property->pdesc->IsArray) {
    struct Object *object = PROP_GetObjectValue(property);
    if (object) {
      OBJ_ReleaseRef(object);
    }
  }
  if (property->value) {
    memset(property->value, 0, PROP_GetSize(property));
  }
  property->flags &= ~PF_MODIFIED;
  if (property->pdesc->DataType == kDataTypeString && property->value) {
    *(char**)property->value = NULL;
  }
}

/*static */struct Property *
_PropertyAlloc(lua_State* L, struct Object *object, struct PropertyType const *pt)
{
  (void)L;
  struct Property *property = ZeroAlloc(sizeof(struct Property));
  property->object  = object;
  property->pdesc   = pt;
  return property;
}

struct Property *
PROP_Create(lua_State* L, struct Object *object, struct PropertyType const *pt)
{
  struct Property *property = _PropertyAlloc(L, object, pt);
  _RegisterProperty(object, property);
  return property;
}

void
OBJ_ReleaseProperties(struct Object *hobj)
{
  FOR_EACH_LIST(struct Property, p, OBJ_GetProperties(hobj))
  {
    PROP_Clear(p);
    FOR_EACH_LIST(struct property_program, pp, core.programs) {
      if (pp->property == p) {
        REMOVE_FROM_LIST(struct property_program, pp, core.programs);
        SafeDelete(pp->token, Token_Release);
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
    if (!pp->token) continue; // disabled / failed to compile
    if (pp->updateFrame == core.frame) continue;
    pp->updateFrame = core.frame;
    pp->property->updateFrame = core.frame;
    struct vm_register r = { 0 };
    if (!OBJ_RunProgram(pp->property->object, pp->token, &r)) {
      Con_Error("Binding update failed while running program for %s/%s",
                OBJ_GetName(pp->property->object),
                pp->property->pdesc->Name);
      REMOVE_FROM_LIST(struct property_program, pp, core.programs);
      SafeDelete(pp->token, Token_Release);
      free(pp->code);
      free(pp);
    } else if (!PROP_Import(pp->property, pp->attr, &r)) {
      Con_Error("Binding update failed while importing into %s/%s",
                OBJ_GetName(pp->property->object),
                pp->property->pdesc->Name);
      REMOVE_FROM_LIST(struct property_program, pp, core.programs);
      SafeDelete(pp->token, Token_Release);
      free(pp->code);
      free(pp);
    }
  }
}

void
PROP_ClearSpecialized(struct Property *pprop) {
  FOR_EACH_LIST(struct Property, p, pprop) {
    if (p->flags & PF_SPECIALIZED) {
      PROP_Clear(p);
      p->flags = (p->flags & ~PF_SPECIALIZED);
      memset(p->value, 0, PROP_GetSize(p));
    }
  }
}

void
PROP_SetTypeSize(struct Property *p, eDataType_t t, uint32_t s)
{
  (void)p;
  (void)t;
  (void)s;
}

void
PROP_SetValuePtr(struct Property *prop, void* data)
{
  prop->value = data;
}

ORCA_API void
PDESC_Print(struct PropertyType const *pdesc, LPSTR buffer, uint32_t len, float const* pf)
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
PROP_Print(struct Property *p, LPSTR buffer, uint32_t len)
{
  PDESC_Print(p->pdesc, buffer, len, PROP_GetValue(p));
}
