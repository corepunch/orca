#include "object_internal.h"

lpcString_t
OBJ_GetName(struct Object const *object)
{
  return object->Name ? object->Name : "";
}

lpcString_t
OBJ_GetClassName(struct Object const *object)
{
  return (object && object->type) ? object->type->ClassName : "(none)";
}

bool_t
OBJ_CheckName(struct Object const *object, lpcString_t szName)
{
  return !strcmp(OBJ_GetName(object), szName);
}

void
OBJ_SetName(struct Object *object, lpcString_t szName)
{
  SafeFree(object->Name);
  object->Name = strdup(szName);
  object->identifier = fnv1a32(object->Name);
}

void
OBJ_SetSourceFile(struct Object *pobj, lpcString_t szFilename)
{
  SafeSet(pobj->SourceFile, strdup(szFilename), free);
}

void
OBJ_SetTextContent(struct Object *pobj, lpcString_t szValue)
{
  SafeSet(pobj->TextContent, strdup(szValue), free);
  OBJ_SetDirty(pobj);
}

lpcString_t OBJ_GetSourceFile(struct Object const *object) {
  return object->SourceFile;
}

lpcString_t OBJ_GetTextContent(struct Object const *object) {
  return object->TextContent;
}

struct Object *OBJ_GetParent(struct Object const *object) {
  return object->parent;
}

struct Object *OBJ_GetFirstChild(struct Object const *object) {
  return object->children;
}

struct Object *OBJ_GetNext(struct Object const *object) {
  return object->next;
}

struct Object *OBJ_GetRoot(struct Object *object) {
  if (object->SourceFile) {
    return object;
  } else if (object->parent) {
    return OBJ_GetRoot(object->parent);
  } else {
    return object;
  }
}

struct Property *OBJ_GetProperties(struct Object const *object) {
  return object->properties;
}

long OBJ_GetTimestamp(struct Object const *pobj) {
  return pobj->dirty;
}

uint32_t OBJ_GetStyle(struct Object const *pobj) {
  return pobj->flags;
}

void OBJ_SetStyle(struct Object *pobj, uint32_t dwFlags) {
  pobj->flags = dwFlags;
}

uint32_t OBJ_GetFlags(struct Object const *pobj) {
  return pobj->flags;
}

void OBJ_SetFlags(struct Object *pobj, uint32_t dwFlags) {
  pobj->flags = dwFlags;
}

uint32_t OBJ_GetIdentifier(struct Object const *pobj) {
  return pobj->identifier;
}

uint32_t OBJ_GetUniqueID(struct Object const *pobj) {
  return pobj->unique;
}

uint32_t OBJ_GetAlias(struct Object const *pobj) {
  return pobj->alias;
}

void OBJ_SetAlias(struct Object *pobj, uint32_t alias) {
  pobj->alias = alias;
}

struct Property *
OBJ_GetPropertyByOffset(struct Object *object,
                        uint32_t classid,
                        uint32_t offset)
{
  FOR_EACH_LIST(struct component, cmp, object->components) {
    if (cmp->pcls->ClassID != classid) continue;
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      if (cmp->pcls->Properties[i].Offset == offset) {
        struct Property **props = (void*)(cmp->pUserData + cmp->pcls->ClassSize);
        return props[i];
      }
    }
  }
  return NULL;
}
