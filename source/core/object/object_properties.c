#include "object_internal.h"

#define ID_TextureTiling 0x861dbc5b

static lpProperty_t
_CreateProjectProperty(lpObject_t object, uint32_t ident)
{
  lpcPropertyType_t pt = OBJ_FindPropertyType(ident);
  if (pt) {
    return PROP_Create(NULL, object, pt);
  }
  return NULL;
}

uint32_t
OBJ_GetUniforms(lpObject_t object, struct uniform* pUniforms)
{
  uint32_t numunf = 0;
  bool_t hasTextureTiling = FALSE;
  for (lpProperty_t property = object->properties; property;
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
          case sizeof(vec2_t):
            hasTextureTiling |= pUniforms->Identifier == ID_TextureTiling;
            pUniforms->Type = UT_FLOAT_VEC2;
            PROP_CopyValue(property, pUniforms->Value);
            break;
          case sizeof(vec3_t):
            pUniforms->Type = UT_FLOAT_VEC3;
            PROP_CopyValue(property, pUniforms->Value);
            break;
          case sizeof(vec4_t):
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

void
OBJ_UpdateProperties(lpObject_t object)
{
  for (lpProperty_t p = object->properties; p; p = PROP_GetNext(p)) {
    PROP_Update(p);
  }
  FOR_EACH_CHILD(object, OBJ_UpdateProperties);
}

int
OBJ_GetInteger(lpcObject_t object, uint32_t ident, int fallback)
{
  for (lpcProperty_t p = object->properties; p; p = PROP_GetNext(p)) {
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
OBJ_FindLongProperty(lpObject_t object,
                     uint32_t identifier,
                     lpProperty_t* ppProp)
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
OBJ_FindShortProperty(lpObject_t object,
                      lpcString_t short_name,
                      lpProperty_t* ppProp)
{
  uint32_t identifier = fnv1a32(short_name);
  if ((*ppProp = PROP_FindByShortID(object->properties, identifier))) {
    return NOERROR;
  } else {
    return OBJ_FindLongProperty(object, identifier, ppProp);
  }
}

HRESULT
OBJ_SetPropertyValue(lpObject_t object, lpcString_t name, void const* value)
{
  lpProperty_t prop;
  HRESULT hr = OBJ_FindShortProperty(object, name, &prop);
  if (SUCCEEDED(hr)) {
    PROP_SetValue(prop, value);
  }
  return hr;
}

#define SUFFIX "Changed"
#define SUFFIX_LEN 7

void
OBJ_RegisterPropertyChangedCallback(lpObject_t object, lpcString_t name)
{
  if (strncmp(name, "on", 2) || !isupper(name[2]))
    return;
  size_t str_len = strlen(name);
  if (str_len >= SUFFIX_LEN && strcmp(name + str_len - SUFFIX_LEN, SUFFIX) == 0) {
    size_t property_len = str_len - SUFFIX_LEN;
    shortStr_t pname = { 0 };
    lpProperty_t pProp = NULL;
    strncpy(pname, name + 2, property_len - 2);
    if (SUCCEEDED(OBJ_FindShortProperty(object, pname, &pProp))) {
      PROP_SetFlag(pProp, PF_HASCHANGECALLBACK);
    } else {
      Con_Error("Could not find property for %s", name);
    }
  }
}


lpProperty_t
OBJ_AddComponentProperty(lua_State* L, struct component* comp, lpcPropertyType_t desc)
{
  /*static */lpProperty_t
  _PropertyAlloc(lua_State* L, lpObject_t object, lpcPropertyType_t pt);

  lpObject_t object = CMP_GetOwner(comp);
  lpProperty_t property = _PropertyAlloc(L, object, desc);
  PROP_AddToList(property, &object->properties);
  CMP_SetProperty(comp, property);
  return property;
}
