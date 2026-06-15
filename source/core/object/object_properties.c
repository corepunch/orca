#include "object_internal.h"
#include "source/core/property/property_internal.h"

#define ID_TextureTiling 0x861dbc5b

static struct Property *
_CreateProjectProperty(struct Object *object, uint32_t ident)
{
  struct PropertyType const *pt = core_FindPropertyType(ident);
  if (pt) {
    return PROP_Create(NULL, object, pt);
  }
  return NULL;
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

struct Property *
OBJ_FindLongProperty(struct Object *object,
                     uint32_t identifier)
{
  FOR_EACH_LIST(struct Property, property, object->properties) {
    if (property->pdesc->FullIdentifier == identifier) {
      return property;
    }
  }
  struct Property* p = _CreateClassProperty(object, identifier);
  if (p) return p;
  p = _CreateProjectProperty(object, identifier);
  if (p) return p;
  return NULL;
}

struct Property *
OBJ_FindShortProperty(struct Object *object, uint32_t identifier)
{
  FOR_EACH_LIST(struct Property, property, object->properties) {
    if (property->pdesc->ShortIdentifier == identifier) {
      return property;
    }
  }
  return OBJ_FindLongProperty(object, identifier);
}

bool_t
OBJ_ReadProperty(struct Object *object, uint32_t identifier, void *output)
{
  struct Property *property = NULL;

  if (!object || !output) {
    return FALSE;
  }

  property = OBJ_FindLongProperty(object, identifier);
  if (!property) {
    struct Object *parent = OBJ_GetParent(object);
    return parent ? OBJ_ReadProperty(parent, identifier, output) : FALSE;
  }

  if (PROP_GetDesc(property)->IsInherited &&
      !(PROP_GetFlags(property) & PF_MODIFIED)) {
    struct Object *parent = OBJ_GetParent(object);
    return parent ? OBJ_ReadProperty(parent, identifier, output) : FALSE;
  }

  void const *slot = PROP_GetRawValueSlot(property);
  if (!slot) {
    return FALSE;
  }

  memcpy(output, slot, PROP_GetSize(property));
  return TRUE;
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
