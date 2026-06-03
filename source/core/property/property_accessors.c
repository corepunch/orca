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
  if (property->pdesc && property->pdesc->IsInherited && !property->pdesc->IsArray) {
    return *(void * const *)property->value;
  }
  return property->value;
}

void const*
PROP_GetValue(struct Property const *property)
{
  if (property && property->pdesc && property->pdesc->IsArray) {
    return property->value;
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
PROP_MarkValueStored(struct Property *property, bool_t local)
{
  if (local) {
    PROP_SetDirty(property);
  } else {
    property->flags |= PF_INHERITED;
    OBJ_SetDirty(property->object);
  }
}

static void
PROP_SetStoredValue(struct Property *property,
                    void const* source,
                    bool_t local)
{
  void *storage = property->value;
  if (property->pdesc->IsInherited && !property->pdesc->IsArray) {
    storage = *(void **)property->value;
    if (local && !(property->flags & PF_OWNS_VALUE)) {
      storage = ZeroAlloc(PROP_GetSize(property));
      ISET_SLOT(property, storage);
      property->flags |= PF_OWNS_VALUE;
    }
  }
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
    PROP_MarkValueStored(property, local);
    return;
  }
  if (PROP_GetType(property) == kDataTypeString) {
    if (*(LPSTR*)storage) {
      free(*(LPSTR*)storage);
    }
    *(LPSTR*)storage = strdup(*(LPSTR*)source);
  } else if (PROP_GetType(property) == kDataTypeObject) {
    struct Object *object = *(struct Object **)source;
    bool_t old_value_is_owned = !property->pdesc->IsInherited ||
      (property->flags & PF_OWNS_VALUE);
    struct Object *old_object = old_value_is_owned ? PROP_GetObjectValue(property) : NULL;
    /* Object properties own a ref, but they do not parent the object. */
    if (!object) {
      memset(storage, 0, PROP_GetSize(property));
      property->flags &= ~(PF_MODIFIED | PF_INHERITED);
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
    if (old_object) {
      OBJ_ReleaseRef(old_object);
    }
  } else {
    memcpy(storage, source, PROP_GetSize(property));
  }
  PROP_MarkValueStored(property, local);
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
  if (!(slot && source && (property->flags & (PF_MODIFIED | PF_INHERITED))))
    return FALSE;
  if (property->pdesc->IsArray) {
    if (property->pdesc->DataType != kDataTypeObject) {
      return memcmp(property->value, source, sizeof(void*) + sizeof(int)) == 0;
    }
    void **old_items = *(void**)property->value;
    int old_count = ((int*)property->value)[sizeof(void*)/sizeof(int)];
    void **new_items = *(void* const*)source;
    int new_count = ((int const*)source)[sizeof(void*)/sizeof(int)];
    if (old_count != new_count) return FALSE;
    FOR_LOOP(i, old_count) {
      if (old_items == NULL || new_items == NULL) {
        if (old_items != new_items) return FALSE;
        break;
      }
      if (old_items[i] != new_items[i]) return FALSE;
    }
    return TRUE;
  }
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
PROP_SetValue(struct Property *property, void const* source)
{
  if (PROP_IsSameValue(property, source) &&
      (!property->pdesc->IsInherited || (property->flags & PF_MODIFIED))) {
    if (property->flags & PF_INHERITED) {
      property->flags &= ~PF_INHERITED;
      PROP_SetDirty(property);
      if (property->pdesc->IsInherited) {
        OBJ_PropagateInheritedProperty(property->object, property);
      }
    }
    return;
  }
  property->flags &= ~PF_INHERITED;
  PROP_SetStoredValue(property, source, TRUE);
  if (property->pdesc->IsInherited) {
    OBJ_PropagateInheritedProperty(property->object, property);
  }
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
PROP_SetInheritedValue(struct Property *property, void const* source)
{
  if (!property || !property->pdesc || !property->pdesc->IsInherited) return;
  if (property->flags & PF_MODIFIED) return;
  if (!property->pdesc->IsArray) {
    void *old_slot = *(void **)property->value;
    if (old_slot == source) return;
    ISET_SLOT(property, source);
    if (source) {
      PROP_MarkValueStored(property, FALSE);
    } else {
      property->flags &= ~PF_INHERITED;
      OBJ_SetDirty(property->object);
    }
    return;
  }
  if (PROP_IsSameValue(property, source)) return;
  PROP_SetStoredValue(property, source, FALSE);
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
