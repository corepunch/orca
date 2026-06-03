#include "object_internal.h"

#define ID_TextureTiling 0x861dbc5b

static struct Property *
_CreateProjectProperty(struct Object *object, uint32_t ident)
{
  struct PropertyType const *pt = OBJ_FindPropertyType(ident);
  if (pt) {
    return PROP_Create(NULL, object, pt);
  }
  return NULL;
}

static bool_t
_HasLocalValue(struct Property const *property)
{
  return property && (PROP_GetFlags(property) & PF_MODIFIED);
}

static void
_ApplyInheritedProperty(struct Object *object,
                        struct PropertyType const *desc,
                        void const *value,
                        bool_t notify_same_slot)
{
  struct Property *property = PROP_FindByLongID(object->properties,
                                               desc->FullIdentifier);
  if (_HasLocalValue(property)) {
    value = PROP_GetRawValueSlot(property);
    notify_same_slot = FALSE;
  } else {
    if (FAILED(OBJ_FindLongProperty(object, desc->FullIdentifier, &property))) {
      return;
    }
    void const *old_value = PROP_GetRawValueSlot(property);
    PROP_SetInheritedValue(property, value);
    if (notify_same_slot && old_value == value && value) {
      PROP_NotifyChanged(property);
    }
    value = PROP_GetRawValueSlot(property);
  }

  FOR_EACH_LIST(struct Object, child, object->children) {
    _ApplyInheritedProperty(child, desc, value, notify_same_slot);
  }
}

void
OBJ_PropagateInheritedProperty(struct Object *object, struct Property *property)
{
  if (!object || !property) {
    return;
  }
  struct PropertyType const *desc = PROP_GetDesc(property);
  if (!desc || !desc->IsInherited) {
    return;
  }
  void const *value = PROP_GetRawValueSlot(property);
  FOR_EACH_LIST(struct Object, child, object->children) {
    _ApplyInheritedProperty(child, desc, value, TRUE);
  }
}

void
OBJ_ApplyInheritedProperties(struct Object *object)
{
  if (!object) {
    return;
  }
  if (!object->parent) {
    for (struct Property *property = object->properties; property;
         property = PROP_GetNext(property)) {
      struct PropertyType const *desc = PROP_GetDesc(property);
      if (!desc || !desc->IsInherited || _HasLocalValue(property) ||
          PROP_IsNull(property)) {
        continue;
      }
      PROP_SetInheritedValue(property, NULL);
      OBJ_PropagateInheritedProperty(object, property);
    }
    return;
  }
  for (struct Property *property = object->parent->properties; property;
       property = PROP_GetNext(property)) {
    struct PropertyType const *desc = PROP_GetDesc(property);
    if (!desc || !desc->IsInherited || PROP_IsNull(property)) {
      continue;
    }
    _ApplyInheritedProperty(object, desc, PROP_GetRawValueSlot(property), FALSE);
  }
}

uint32_t
OBJ_GetUniforms(struct Object *object, struct uniform* pUniforms)
{
  uint32_t numunf = 0;
  bool_t hasTextureTiling = FALSE;
  for (struct Property *property = object->properties; property;
       property = PROP_GetNext(property)) {
    if (PROP_IsNull(property))
      continue;
    memset(pUniforms->Value, 0, sizeof(pUniforms->Value));
    pUniforms->Identifier = PROP_GetShortID(property);
    switch (PROP_GetType(property)) {
      case kDataTypeObject:
        if (!strcmp(PROP_GetUserData(property), "Texture")) {
          struct Texture *texture = GetTexture(*(void**)PROP_GetValue(property));
          pUniforms->Type = UT_SAMPLER_2D;
          memcpy(pUniforms->Value, &texture, sizeof(void*));
        } else {
          continue;
        }
        break;
      case kDataTypeFloat:
        pUniforms->Type = UT_FLOAT;
        PROP_CopyValue(property, pUniforms->Value);
        break;
      case kDataTypeColor:
        pUniforms->Type = UT_COLOR;
        PROP_CopyValue(property, pUniforms->Value);
        break;
      case kDataTypeStruct:
        switch (PROP_GetSize(property)) {
          case sizeof(struct vec2):
            hasTextureTiling |= pUniforms->Identifier == ID_TextureTiling;
            pUniforms->Type = UT_FLOAT_VEC2;
            PROP_CopyValue(property, pUniforms->Value);
            break;
          case sizeof(struct vec3):
            pUniforms->Type = UT_FLOAT_VEC3;
            PROP_CopyValue(property, pUniforms->Value);
            break;
          case sizeof(struct vec4):
            pUniforms->Type = UT_FLOAT_VEC4;
            PROP_CopyValue(property, pUniforms->Value);
            break;
        }
      case kDataTypeInt:
      case kDataTypeBool:
      case kDataTypeEnum:
        pUniforms->Type = UT_FLOAT;
        *pUniforms->Value = *(int*)PROP_GetValue(property);
        break;
      default:
        continue;
    }
    pUniforms++;
    numunf++;
  }
  if (!hasTextureTiling) {
    memset(pUniforms->Value, 0, sizeof(pUniforms->Value));
    pUniforms->Identifier = ID_TextureTiling;
    pUniforms->Type = UT_FLOAT_VEC2;
    pUniforms->Value[0] = 1;
    pUniforms->Value[1] = 1;
    pUniforms++;
    numunf++;
  }
  return numunf;
}


int
OBJ_GetInteger(struct Object const *object, uint32_t ident, int fallback)
{
  for (struct Property const *p = object->properties; p; p = PROP_GetNext(p)) {
    if (PROP_GetLongIdentifier(p) == ident) {
      switch (PROP_GetType(p)) {
        case kDataTypeInt:
        case kDataTypeEnum:
        case kDataTypeBool:
          return *(int *)PROP_GetValue(p);
        default:
          return fallback;
      }
    }
  }
  return fallback;
}

HRESULT
OBJ_FindLongProperty(struct Object *object,
                     uint32_t identifier,
                     struct Property ** ppProp)
{
  if ((*ppProp = PROP_FindByLongID(object->properties, identifier))) {
    return NOERROR;
  } else if ((*ppProp = _CreateClassProperty(object, identifier))) {
    return NOERROR;
  } else if ((*ppProp = _CreateProjectProperty(object, identifier))) {
    return NOERROR;
  } else {
    return E_ITEMNOTFOUND;
  }
}

HRESULT
OBJ_FindShortProperty(struct Object *object,
                      lpcString_t name,
                      struct Property ** ppProp)
{
  uint32_t identifier = fnv1a32(name);
  if ((*ppProp = PROP_FindByShortID(object->properties, identifier))) {
    return NOERROR;
  } else {
    HRESULT hr = OBJ_FindLongProperty(object, identifier, ppProp);
    if (SUCCEEDED(hr)) {
      return hr;
    }
    return hr;
  }
}

HRESULT
OBJ_SetPropertyValue(struct Object *object, lpcString_t name, void const* value)
{
  struct Property *prop;
  HRESULT hr = OBJ_FindShortProperty(object, name, &prop);
  if (SUCCEEDED(hr)) {
    PROP_SetValue(prop, value);
  }
  return hr;
}

struct Property *
OBJ_AddComponentProperty(lua_State* L, struct component* comp, struct PropertyType const *desc)
{
  /*static */struct Property *
  _PropertyAlloc(lua_State* L, struct Object *object, struct PropertyType const *pt);

  struct Object *object = CMP_GetOwner(comp);
  struct Property *property = _PropertyAlloc(L, object, desc);
  PROP_AddToList(property, &object->properties);
  CMP_SetProperty(comp, property);
  return property;
}
