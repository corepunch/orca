#include "object_internal.h"

lpcString_t
OBJ_GetName(struct Object const *object)
{
  return object->Name ? object->Name : "";
}

lpcString_t
OBJ_GetClassName(struct Object const *object)
{
  if (object->components) {
    return CMP_GetClassName(object->components);
  } else {
    return "(none)";
  }
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

void
OBJ_SetClassName(struct Object *pobj, lpcString_t szClassName)
{
  SafeSet(pobj->ClassName, strdup(szClassName), free);
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
OBJ_GetPropertyAtIndex(struct Object *object,
                       uint32_t classid,
                       size_t classsize,
                       uint32_t index)
{
  void* userdata = OBJ_GetComponent(object, classid);
  if (!userdata)
    return NULL;
  struct Property ** props = (void*)(userdata + classsize);
  return props[index];
}
