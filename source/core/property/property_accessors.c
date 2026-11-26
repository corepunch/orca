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
  if ((value & PF_SPECIALIZED) && !(property->flags & PF_SPECIALIZED) &&
      !property->pdesc->IsArray) {
    // State styles temporarily override the resolved value, including inline defaults.
    struct Property *base = ZeroAlloc(sizeof(*base));
    base->object = property->object;
    base->pdesc = property->pdesc;
    base->flags = property->flags & (PF_MODIFIED | PF_OWNS_OBJECT_CHILD | PF_STYLED);
    property->flags &= ~PF_OWNS_OBJECT_CHILD;
    base->value = ZeroAlloc(PROP_GetSize(property));
    void *resolved = property->stateOverride ? property->stateOverride->base : property->value;
    memcpy(base->value, resolved, PROP_GetSize(property));
    if (PROP_GetType(property) == kDataTypeString) {
      char *text = *(char **)resolved;
      *(char **)base->value = text ? strdup(text) : NULL;
    } else if (PROP_GetType(property) == kDataTypeObject) {
      struct Object *object = PROP_GetObjectValue(property);
      if (object) OBJ_AddRef(object);
    }
    base->binding = property->binding;
    property->binding = NULL;
    property->specializedBase = base;
  }
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
  if (property->pdesc && property->pdesc->IsInherited &&
      !(property->flags & PF_MODIFIED) && !property->stateOverride) {
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
  /* Any write is authored until the style engine marks it PF_STYLED again. */
  property->flags = (property->flags | PF_MODIFIED) & ~PF_STYLED;
  if (PROP_GetShortID(property) != ID_ContentOffset) {
    OBJ_SetDirty(property->object);
  } else {
    for (struct Object *object = property->object; object; object = OBJ_GetParent(object))
      OBJ_SetFlags(object, OBJ_GetFlags(object) | OF_SCROLL);
  }
}

static void
PROP_SetStoredValue(struct Property *property,
                    void const* source)
{
  void *storage = property->value;
  if (property->pdesc->IsArray) {
    PROP_InvalidateBindingPaths();
    if (OBJ_GetComponent(property->object, ID_DataObject)) PROP_BindingDataChanged();
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
    // The source may be a substring of the value being replaced.
    char const *text = *(char const *const *)source;
    char *copy = text ? strdup(text) : NULL;
    if (*(LPSTR*)storage) {
      free(*(LPSTR*)storage);
    }
    *(LPSTR*)storage = copy;
  } else if (PROP_GetType(property) == kDataTypeObject) {
    struct Object *object = *(struct Object **)source;
    struct Object *old_object = PROP_GetObjectValue(property);
    /* Object references own a ref, but do not normally parent the object. */
    if (!object) {
      memset(storage, 0, PROP_GetSize(property));
      property->flags &= ~PF_MODIFIED;
      if (old_object) {
        if ((property->flags & PF_OWNS_OBJECT_CHILD) &&
            OBJ_GetParent(old_object) == property->object) {
          OBJ_DetachFromParent(old_object);
          OBJ_ReleaseRef(old_object); /* hierarchy reference */
        }
        property->flags &= ~PF_OWNS_OBJECT_CHILD;
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
      if ((property->flags & PF_OWNS_OBJECT_CHILD) &&
          OBJ_GetParent(old_object) == property->object) {
        OBJ_DetachFromParent(old_object);
        OBJ_ReleaseRef(old_object); /* hierarchy reference */
      }
      property->flags &= ~PF_OWNS_OBJECT_CHILD;
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

bool_t
PROP_HasStateOverride(struct Property const *property)
{
  return property && property->stateOverride != NULL;
}

void const *
PROP_GetStateBaseValue(struct Property const *property, void *owner)
{
  if (!property) return NULL;
  if (!property->stateOverride) return PROP_GetValue(property);
  if (property->stateOverride->owner != owner) {
    Con_Error("StateManager: invalid state override owner");
    return NULL;
  }
  if (property->pdesc->IsInherited && !(property->flags & PF_MODIFIED)) {
    for (struct Object *obj = OBJ_GetParent(property->object); obj; obj = OBJ_GetParent(obj)) {
      struct Property *p = OBJ_FindLongProperty(obj, property->pdesc->FullIdentifier);
      if (p && (p->flags & PF_MODIFIED)) return PROP_GetValue(p);
    }
    return NULL;
  }
  return property->stateOverride->base;
}

bool_t
PROP_StateOverrideConflicts(struct Property const *property)
{
  if (!property || !property->value || !property->object) return FALSE;
  uintptr_t a = (uintptr_t)property->value;
  size_t size = PROP_GetSize(property);
  for (struct Property *other = OBJ_GetProperties(property->object); other; other = other->next) {
    if (other == property || !other->stateOverride || !other->value) continue;
    uintptr_t b = (uintptr_t)other->value;
    size_t otherSize = PROP_GetSize(other);
    if ((a <= b && b - a < size) || (b < a && a - b < otherSize)) return TRUE;
  }
  return FALSE;
}

bool_t
PROP_BeginStateOverride(struct Property *property, void *owner)
{
  if (!property || !owner || !property->value || !property->pdesc ||
      property->pdesc->IsArray || !PROP_GetSize(property))
    return Con_Error("StateManager: invalid or unsupported state override property"), FALSE;
  if (property->stateOverride) {
    if (property->stateOverride->owner == owner) return TRUE;
    return Con_Error("StateManager: property %s already has a state override", PROP_GetName(property)), FALSE;
  }
  if (PROP_StateOverrideConflicts(property))
    return Con_Error("StateManager: overlapping state override for %s", PROP_GetName(property)), FALSE;
  eDataType_t type = PROP_GetType(property);
  if (type == kDataTypeObject || type == kDataTypeEvent || property->pdesc->IsPointer) {
    Con_Error("StateManager: unsupported state override type for %s", PROP_GetName(property));
    return FALSE;
  }
  struct StatePropertyOverride *state = calloc(1, sizeof(*state));
  if (!state) return Con_Error("StateManager: cannot allocate property override"), FALSE;
  state->base = malloc(PROP_GetSize(property));
  if (!state->base) return free(state), Con_Error("StateManager: cannot allocate base value"), FALSE;
  memcpy(state->base, property->value, PROP_GetSize(property));
  if (type == kDataTypeString) {
    char *text = *(char **)state->base;
    *(char **)state->base = text ? strdup(text) : NULL;
    if (text && !*(char **)state->base)
      return free(state->base), free(state), Con_Error("StateManager: cannot copy base string"), FALSE;
  }
  state->owner = owner;
  if (property->pdesc->IsInherited && !(property->flags & PF_MODIFIED)) {
    void const *inherited = PROP_GetValue(property);
    if (inherited) {
      if (type == kDataTypeString) {
        char const *text = *(char const *const *)inherited;
        char *copy = text ? strdup(text) : NULL;
        if (text && !copy) {
          free(*(char **)state->base);
          free(state->base);
          free(state);
          return Con_Error("StateManager: cannot copy inherited string"), FALSE;
        }
        free(*(char **)property->value);
        *(char **)property->value = copy;
      } else {
        memcpy(property->value, inherited, PROP_GetSize(property));
      }
    }
  }
  property->stateOverride = state;
  return TRUE;
}

bool_t
PROP_SetStateOverride(struct Property *property, void *owner, void const *value)
{
  if (!property || !property->stateOverride || property->stateOverride->owner != owner || !value) {
    Con_Error("StateManager: invalid state override owner or value");
    return FALSE;
  }
  if (PROP_IsSameValue(property, value)) return TRUE;
  uint32_t flags = property->flags;
  PROP_SetStoredValue(property, value);
  property->flags = flags;
  PROP_NotifyChanged(property);
  return TRUE;
}

void
PROP_DiscardStateOverride(struct Property *property)
{
  if (!property || !property->stateOverride) return;
  struct StatePropertyOverride *state = property->stateOverride;
  if (PROP_GetType(property) == kDataTypeString) free(*(char **)state->base);
  free(state->base);
  free(state);
  property->stateOverride = NULL;
}

bool_t
PROP_EndStateOverride(struct Property *property, void *owner)
{
  if (!property || !property->stateOverride || property->stateOverride->owner != owner) {
    Con_Error("StateManager: invalid state override owner");
    return FALSE;
  }
  struct StatePropertyOverride *state = property->stateOverride;
  if (PROP_GetType(property) == kDataTypeString) free(*(char **)property->value);
  memcpy(property->value, state->base, PROP_GetSize(property));
  free(state->base);
  free(state);
  property->stateOverride = NULL;
  OBJ_SetDirty(property->object);
  PROP_NotifyChanged(property);
  return TRUE;
}

void
PROP_NotifyChanged(struct Property *property)
{
  if (property->pdesc->DataType == kDataTypeObject) PROP_InvalidateBindingPaths();
  if (OBJ_GetComponent(property->object, ID_DataObject)) PROP_BindingDataChanged();
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
  if (!property || !source) return;
  if (PROP_StateOverrideConflicts(property)) {
    Con_Error("StateManager: write to overlapping state property %s", PROP_GetName(property));
    return;
  }
  if (property->stateOverride) {
    struct Property base = *property;
    base.value = property->stateOverride->base;
    base.stateOverride = NULL;
    if (PROP_IsSameValue(&base, source)) return;
    PROP_SetStoredValue(&base, source);
    property->flags = base.flags;
    PROP_WriteBindingSource(&base);
    return;
  }
  if (PROP_IsSameValue(property, source)) return;
  PROP_SetStoredValue(property, source);
  if (property->pdesc->IsArray) {
    return;
  }
  PROP_NotifyChanged(property);
  PROP_WriteBindingSource(property);
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
