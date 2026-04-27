#include "property_internal.h"

struct Property *
PROP_FindByLongID(struct Property *list, uint32_t identifier) {
  FOR_EACH_LIST(struct Property, property, list) {
    if (property->pdesc->FullIdentifier == identifier) {
      return property;
    }
  }
  return NULL;
}

struct Property *
PROP_FindByShortID(struct Property *list, uint32_t identifier) {
  FOR_EACH_LIST(struct Property, property, list) {
    if (property->pdesc->ShortIdentifier == identifier) {
      return property;
    }
  }
  return NULL;
}

struct Property *
PROP_GetNext(struct Property const *property)
{
  return property->next;
}

uint32_t
PROP_GetFlags(struct Property const *property)
{
  return property->flags;
}

void
PROP_SetFlag(struct Property *property, uint32_t value)
{
  property->flags |= value;
}

eDataType_t
PROP_GetType(struct Property const *property)
{
  return property->pdesc->DataType;
}

uint32_t
PROP_GetShortID(struct Property const *property)
{
  return property->pdesc->ShortIdentifier;
}

lpcString_t
PROP_GetUserData(struct Property const *property)
{
  return property->pdesc->TypeString;
}

void const*
PROP_GetValue(struct Property const *property)
{
  // If this is a component, not a direct object reference
  if (PROP_GetType(property) == kDataTypeObject && property->pdesc->TypeString) {
    static struct Object *obj[256];
    static uint8_t i = 0;
    obj[i] = CMP_GetObject(*(void **)property->value);
    return &obj[i++];
  } else {
    return property->value;
  }
}

void
PROP_SetDirty(struct Property *property)
{
  property->flags |= PF_MODIFIED;
  if (property->pdesc->FullIdentifier != ID_ContentOffset) {
    OBJ_SetDirty(property->object);
  } else {
    OBJ_SetFlags(property->object, OBJ_GetFlags(property->object) | OF_SCROLL);
  }
}

static void
PROP_SetStoredValue(struct Property *property,
                    void const* source)
{
  if (property->pdesc->IsArray) {
    void *old_ptr = *(void**)property->value;
    if (old_ptr) {
      free(old_ptr);
    }
    memcpy(property->value, source, sizeof(void*) + sizeof(int));
    PROP_SetDirty(property);
    return;
  }
  if (PROP_GetType(property) == kDataTypeString) {
    if (*(LPSTR*)property->value) {
      free(*(LPSTR*)property->value);
    }
    *(LPSTR*)property->value = strdup(*(LPSTR*)source);
  } else if (PROP_GetType(property) == kDataTypeObject) {
    int ident = fnv1a32(property->pdesc->TypeString);
    struct Object *object = *(struct Object **)source;
    if (!object) {
      memset(property->value, 0, PROP_GetSize(property));
      property->flags &= ~PF_MODIFIED;
      return;
    }
    void* udata = OBJ_GetComponent(object, ident);
    if (!udata) {
      memset(property->value, 0, PROP_GetSize(property));
      property->flags &= ~PF_MODIFIED;
      Con_Error("No %s component in object %s(%s)", property->pdesc->TypeString, OBJ_GetName(object), OBJ_GetClassName(object));
      return;
    }
    memcpy(property->value, &udata, PROP_GetSize(property));
    // if not a node, i.e. an Image, Model, Sound, etc., add the object as a child to ensure it gets released properly
    if (!GetNode(object)) {
      if (OBJ_GetParent(object)) {
        Con_Error("Object %s being assigned to a property %s is already parented", OBJ_GetName(object), PROP_GetName(property));
      } else {
        OBJ_AddChild(property->object, object, FALSE);
      }
    }
  } else {
    memcpy(property->value, source, PROP_GetSize(property));
  }
  PROP_SetDirty(property);
}

static void*
PROP_NormalizeObjectValue(struct Property const *property, void const* source)
{
  struct Object *object;
  if (!source) return NULL;
  object = *(struct Object *const*)source;
  if (!object) return NULL;
  if (property->pdesc->TypeString && strcmp(property->pdesc->TypeString, "Object") == 0)
    return object;
  return OBJ_GetComponent(object, fnv1a32(property->pdesc->TypeString));
}

static bool_t
PROP_IsSameValue(struct Property const *property, void const* source)
{
  if (!(property->value && source && (property->flags & PF_MODIFIED)))
    return FALSE;
  if (PROP_GetType(property) == kDataTypeString) {
    lpcString_t old = *(lpcString_t*)property->value;
    lpcString_t newv = *(lpcString_t const*)source;
    return old == newv || (old && newv && strcmp(old, newv) == 0);
  }
  if (PROP_GetType(property) == kDataTypeObject) {
    void* oldv = *(void**)property->value;
    void* newv = PROP_NormalizeObjectValue(property, source);
    struct Object *object = *(struct Object *const*)source;
    if (!object) return oldv == NULL;
    if (!newv) return oldv == NULL;
    return oldv == newv;
  }
  return memcmp(property->value, source, PROP_GetSize(property)) == 0;
}

void
PROP_SetValue(struct Property *property, void const* source)
{
  if (PROP_IsSameValue(property, source)) return;
  PROP_SetStoredValue(property, source);
  _SendMessage(property->object, Object, PropertyChanged, property);
  if (PROP_HasHandler(property) && !(property->flags & PF_NOTIFICATION_QUEUED)) {
    property->flags |= PF_NOTIFICATION_QUEUED;
    axPostMessageW(property->object, ID_PropertyChangedMessage,
                   PROP_GetShortID(property), property);
  }
}

void
PROP_SetStringValue(struct Property *property, char const* source)
{
  assert(property->pdesc->DataType == kDataTypeString);
  PROP_SetValue(property, &source);
}

lpcString_t
PROP_GetName(struct Property const *property)
{
  return property->pdesc->Name;
}

lpcString_t
PROP_GetComponentName(struct Property const *property)
{
  return property->pdesc->Category;
}

uint32_t
PROP_GetSize(struct Property const *property)
{
  if (property->pdesc) {
    return property->pdesc->IsArray ? sizeof(void*) : (uint32_t)property->pdesc->DataSize;
  } else {
    return (uint32_t)psize[kDataTypeNone];
  }
}

uint32_t
PROP_GetLongIdentifier(struct Property const *prop)
{
  return prop->pdesc->FullIdentifier;
}

struct Object *
PROP_GetObject(struct Property const *pprop)
{
  return pprop->object;
}

struct PropertyType const *
PROP_GetDesc(struct Property const *prop) {
  return prop->pdesc;
}
