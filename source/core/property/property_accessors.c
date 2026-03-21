#include "property_internal.h"

lpProperty_t
PROP_FindByLongID(lpProperty_t list, uint32_t identifier) {
  FOR_EACH_LIST(struct Property, property, list) {
    if (property->pdesc->FullIdentifier == identifier) {
      return property;
    }
  }
  return NULL;
}

lpProperty_t
PROP_FindByShortID(lpProperty_t list, uint32_t identifier) {
  FOR_EACH_LIST(struct Property, property, list) {
    if (property->pdesc->ShortIdentifier == identifier) {
      return property;
    }
  }
  return NULL;
}

lpcString_t
PROP_GetCallbackEvent(lpcProperty_t p)
{
  return p->callbackEvent ? PROP_GetValue(p->callbackEvent) : NULL;
}

lpProperty_t
PROP_GetNext(lpcProperty_t property)
{
  return property->next;
}

uint32_t
PROP_GetFlags(lpcProperty_t property)
{
  return property->flags;
}

void
PROP_SetFlag(lpProperty_t property, uint32_t value)
{
  property->flags |= value;
}

eDataType_t
PROP_GetType(lpcProperty_t property)
{
  return property->type;
}

enum uniform_precision
PROP_GetPrecision(lpcProperty_t property)
{
  return property->precision;
}

uint32_t
PROP_GetShortID(lpcProperty_t property)
{
  return property->pdesc->ShortIdentifier;
}

lpcString_t
PROP_GetUserData(lpcProperty_t property)
{
  return property->pdesc->ClassRef ? property->pdesc->ClassRef->ClassName : property->pdesc->TypeString;
}

void const*
PROP_GetValue(lpcProperty_t property)
{
  switch (property->type) {
    case kDataTypeString:
      return *(lpcString_t*)property->value;
    case kDataTypeObject:
      return &property->intermediate;
    default:
      return property->value;
  }
}

void*
PROP_SetStateValue(lpProperty_t property,
                   void const* source,
                   enum PropertyState state)
{
  void* ptr = PROP_GetState(property, state);
  if (property->type == kDataTypeString) {
    if (property->stateflags & (1 << state)) {
      free(*(LPSTR*)PROP_GetState(property, state));
    }
    *(LPSTR*)ptr = strdup(source);
  } else if (property->type == kDataTypeObject) {
    uint32_t ident = property->pdesc->ClassRef
                   ? property->pdesc->ClassRef->ClassID
                   : fnv1a32(property->pdesc->TypeString);
    lpcString_t typeName = property->pdesc->ClassRef
                         ? property->pdesc->ClassRef->ClassName
                         : property->pdesc->TypeString;
    lpObject_t object = *(lpObject_t *)source;
    property->intermediate = object;
    if (!object) {
      memset(ptr, 0, PROP_GetSize(property));
      property->stateflags &= ~(1<<state);
      return ptr;
    }
    void* udata = OBJ_GetComponent(object, ident);
    if (!udata) {
      memset(ptr, 0, PROP_GetSize(property));
      property->stateflags &= ~(1<<state);
      Con_Error("No %s component in object %s(%s)", typeName, OBJ_GetName(object), OBJ_GetClassName(object));
      return ptr;
    }
    memcpy(ptr, &udata, PROP_GetSize(property));
  } else {
    memcpy(ptr, source, PROP_GetSize(property));
  }
  property->stateflags |= 1 << state;
  property->flags |= PF_MODIFIED;
  if (property->pdesc->FullIdentifier != ID_ContentOffset) {
    OBJ_SetDirty(property->object);
  } else {
    OBJ_SetFlags(property->object, OBJ_GetFlags(property->object) | OF_SCROLL);
  }
  return ptr;
}

void
PROP_SetValue(lpProperty_t property, void const* source)
{
  memcpy(property->value,
         PROP_SetStateValue(property, source, kPropertyStateNormal),
         PROP_GetSize(property));
}

lpcString_t
PROP_GetName(lpcProperty_t property)
{
  return property->pdesc->Name;
}

lpcString_t
PROP_GetComponentName(lpcProperty_t property)
{
  return property->pdesc->Category;
}

uint32_t
PROP_GetSize(lpcProperty_t property)
{
  if (property->pdesc) {
    return property->pdesc->IsArray ? sizeof(void*) : (uint32_t)property->pdesc->DataSize;
  } else {
    return (uint32_t)psize[property->type];
  }
}

uint32_t
PROP_GetLongIdentifier(lpcProperty_t prop)
{
  return prop->pdesc->FullIdentifier;
}

lpObject_t
PROP_GetObject(lpcProperty_t pprop)
{
  return pprop->object;
}

lpcPropertyType_t
PROP_GetDesc(lpcProperty_t prop) {
  return prop->pdesc;
}
