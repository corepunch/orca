#include "property_internal.h"

struct Object *
PROP_GetObjectValue(struct Property const *property)
{
  void const *slot = PROP_GetRawValueSlot(property);
  if (!property || PROP_GetType(property) != kDataTypeObject || !slot) {
    return NULL;
  }
  /* Object-typed properties store a reference-counted object pointer.
   * If the property targets a component, the stored pointer is the component
   * userdata and CMP_GetObject recovers the owning object. */
  if (property->pdesc->TypeString) {
    return CMP_GetObject(*(void **)slot);
  }
  return *(struct Object **)slot;
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

void const *
PROP_GetRawValueSlot(struct Property const *property)
{
  if (!property || !property->value) {
    return NULL;
  }
  return property->value;
}

void const*
PROP_GetValue(struct Property const *property)
{
  if (!property) return NULL;
  if (property->pdesc && property->pdesc->IsArray) {
    return property->value;
  }
  if (property->pdesc && property->pdesc->IsInherited && !(property->flags & PF_MODIFIED)) {
    for (struct Object *obj = property->object ? OBJ_GetParent(property->object) : NULL; obj; obj = OBJ_GetParent(obj)) {
      struct Property *p = OBJ_FindLongProperty(obj, property->pdesc->FullIdentifier);
      if (p && (p->flags & PF_MODIFIED)) {
        return PROP_GetValue(p);
      }
    }
    return NULL;
  }
  void const *slot = PROP_GetRawValueSlot(property);
  if (!slot) {
    return NULL;
  }
  if (PROP_GetType(property) == kDataTypeObject) {
    static struct Object *obj[256];
    static uint8_t i = 0;
    obj[i] = PROP_GetObjectValue(property);
    return &obj[i++];
  } else {
    return slot;
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
  void *storage = property->value;
  if (property->pdesc->IsArray) {
    void *old_ptr = *(void**)storage;
    int old_count = ((int*)storage)[sizeof(void*)/sizeof(int)];
    if (old_ptr) {
      if (property->pdesc->DataType == kDataTypeObject) {
        FOR_LOOP(i, old_count) {
          if (((void**)old_ptr)[i]) {
            OBJ_ReleaseRef((struct Object*)((void**)old_ptr)[i]);
          }
        }
      }
      free(old_ptr);
    }
    memcpy(storage, source, sizeof(void*) + sizeof(int));
    if (property->pdesc->DataType == kDataTypeObject) {
      void **items = *(void**)storage;
      int count = ((int*)storage)[sizeof(void*)/sizeof(int)];
      FOR_LOOP(i, count) {
        if (items && items[i]) {
          OBJ_AddRef((struct Object*)items[i]);
        }
      }
    }
    PROP_SetDirty(property);
    return;
  }
  if (PROP_GetType(property) == kDataTypeString) {
    if (*(LPSTR*)storage) {
      free(*(LPSTR*)storage);
    }
    *(LPSTR*)storage = strdup(*(LPSTR*)source);
  } else if (PROP_GetType(property) == kDataTypeObject) {
    struct Object *object = *(struct Object **)source;
    struct Object *old_object = PROP_GetObjectValue(property);
    /* Object properties own a ref, but they do not parent the object. */
    if (!object) {
      memset(storage, 0, PROP_GetSize(property));
      property->flags &= ~PF_MODIFIED;
      if (old_object) {
        OBJ_ReleaseRef(old_object);
      }
      return;
    }
    if (property->pdesc->TypeString) {
      int ident = fnv1a32(property->pdesc->TypeString);
      void* udata = OBJ_GetComponent(object, ident);
      if (!udata) {
        Con_Error("No %s component in object %s(%s)",
                  property->pdesc->TypeString,
                  OBJ_GetName(object),
                  OBJ_GetClassName(object));
        return;
      }
      memcpy(storage, &udata, PROP_GetSize(property));
    } else {
      memcpy(storage, &object, PROP_GetSize(property));
    }
    OBJ_AddRef(object);
    OBJ_SendMessageW(object, ID_Object_Start, 0, NULL);
    if (old_object) {
      OBJ_ReleaseRef(old_object);
    }
  } else {
    memcpy(storage, source, PROP_GetSize(property));
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
  if (!property->pdesc->TypeString || strcmp(property->pdesc->TypeString, "Object") == 0)
    return object;
  return OBJ_GetComponent(object, fnv1a32(property->pdesc->TypeString));
}

static bool_t
PROP_IsSameValue(struct Property const *property, void const* source)
{
  void const *slot = PROP_GetRawValueSlot(property);
  if (!(slot && source && (property->flags & PF_MODIFIED)))
    return FALSE;
  if (property->pdesc->IsArray)
    return FALSE;
  if (PROP_GetType(property) == kDataTypeString) {
    lpcString_t old = *(lpcString_t const*)slot;
    lpcString_t newv = *(lpcString_t const*)source;
    return old == newv || (old && newv && strcmp(old, newv) == 0);
  }
  if (PROP_GetType(property) == kDataTypeObject) {
    void* oldv = *(void* const*)slot;
    void* newv = PROP_NormalizeObjectValue(property, source);
    struct Object *object = *(struct Object *const*)source;
    if (!object) return oldv == NULL;
    if (!newv) return oldv == NULL;
    return oldv == newv;
  }
  return memcmp(slot, source, PROP_GetSize(property)) == 0;
}

void
PROP_NotifyChanged(struct Property *property)
{
  if (property->pdesc->IsArray) {
    return;
  }
  _SendMessage(property->object, Object, PropertyChanged, property);
  if (PROP_HasHandler(property) && !(property->flags & PF_NOTIFICATION_QUEUED)) {
    property->flags |= PF_NOTIFICATION_QUEUED;
    axPostMessageW(property->object, ID_PropertyChangedMessage,
                   PROP_GetShortID(property), property);
  }
}

void
PROP_SetValue(struct Property *property, void const* source)
{
  if (!property || PROP_IsSameValue(property, source)) {
    return;
  }
  PROP_SetStoredValue(property, source);
  if (property->pdesc->IsArray) {
    return;
  }
  PROP_NotifyChanged(property);
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
    return 0;
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
