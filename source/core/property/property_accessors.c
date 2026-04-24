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
  return property->pdesc->TypeString;
}

void const*
PROP_GetValue(lpcProperty_t property)
{
  // If this is a component, not a direct object reference
  if (property->type == kDataTypeObject && property->pdesc->TypeString) {
    static lpObject_t obj[256];
    static uint8_t i = 0;
    obj[i] = CMP_GetObject(*(void **)property->value);
    return &obj[i++];
  } else {
    return property->value;
  }
}

void
PROP_SetDirty(lpProperty_t property, enum PropertyState state)
{
  property->stateflags |= 1 << state;
  property->flags |= PF_MODIFIED;
  if (property->pdesc->FullIdentifier != ID_ContentOffset) {
    OBJ_SetDirty(property->object);
  } else {
    OBJ_SetFlags(property->object, OBJ_GetFlags(property->object) | OF_SCROLL);
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
    *(LPSTR*)ptr = strdup(*(LPSTR*)source);
  } else if (property->type == kDataTypeObject) {
    int ident = fnv1a32(property->pdesc->TypeString);
    lpObject_t object = *(lpObject_t *)source;
    if (!object) {
      memset(ptr, 0, PROP_GetSize(property));
      property->stateflags &= ~(1<<state);
      return ptr;
    }
    void* udata = OBJ_GetComponent(object, ident);
    if (!udata) {
      memset(ptr, 0, PROP_GetSize(property));
      property->stateflags &= ~(1<<state);
      Con_Error("No %s component in object %s(%s)", property->pdesc->TypeString, OBJ_GetName(object), OBJ_GetClassName(object));
      return ptr;
    }
    memcpy(ptr, &udata, PROP_GetSize(property));
  } else {
    memcpy(ptr, source, PROP_GetSize(property));
  }
  PROP_SetDirty(property, kPropertyStateNormal);
  return ptr;
}

static void*
PROP_NormalizeObjectValue(lpcProperty_t property, void const* source)
{
  lpObject_t object;
  if (!source) return NULL;
  object = *(lpObject_t const*)source;
  if (!object) return NULL;
  if (property->pdesc->TypeString && strcmp(property->pdesc->TypeString, "Object") == 0)
    return object;
  return OBJ_GetComponent(object, fnv1a32(property->pdesc->TypeString));
}

static bool_t
PROP_IsSameValue(lpcProperty_t property, void const* source)
{
  if (!(property->value && source && (property->stateflags & (1 << kPropertyStateNormal))))
    return FALSE;
  if (property->type == kDataTypeString) {
    lpcString_t old = *(lpcString_t*)property->value;
    lpcString_t newv = *(lpcString_t const*)source;
    return old == newv || (old && newv && strcmp(old, newv) == 0);
  }
  if (property->type == kDataTypeObject) {
    void* oldv = *(void**)property->value;
    void* newv = PROP_NormalizeObjectValue(property, source);
    lpObject_t object = *(lpObject_t const*)source;
    if (!object) return oldv == NULL;
    if (!newv) return oldv == NULL;
    return oldv == newv;
  }
  return memcmp(property->value, source, PROP_GetSize(property)) == 0;
}

void
PROP_SetValue(lpProperty_t property, void const* source)
{
  if (PROP_IsSameValue(property, source)) return;
  memcpy(property->value,
         PROP_SetStateValue(property, source, kPropertyStateNormal),
         PROP_GetSize(property));
  _SendMessage(property->object, Object, PropertyChanged, property);
  if (PROP_HasHandler(property) && !(property->flags & PF_NOTIFICATION_QUEUED)) {
    if (core.pending_notification_count < MAX_PENDING_NOTIFICATIONS) {
      property->flags |= PF_NOTIFICATION_QUEUED;
      core.pending_notifications[core.pending_notification_count++] = property;
    } else {
      Con_Printf("WARNING: property notification queue full, dropping %s\n",
                 PROP_GetName(property));
    }
  }
}

void
PROP_SetStringValue(lpProperty_t property, char const* source) {
  assert(property->pdesc->DataType == kDataTypeString);
  PROP_SetValue(property, &source);
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
