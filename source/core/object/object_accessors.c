#include "object_internal.h"

lpcString_t
OBJ_GetName(lpcObject_t object)
{
  return object->Name ? object->Name : "";
}

lpcString_t
OBJ_GetClassName(lpcObject_t object)
{
  if (object->components) {
    return CMP_GetClassName(object->components);
  } else {
    return "(none)";
  }
}

bool_t
OBJ_CheckName(lpcObject_t object, lpcString_t szName)
{
  return !strcmp(OBJ_GetName(object), szName);
}

void
OBJ_SetName(lpObject_t object, lpcString_t szName)
{
  SafeFree(object->Name);
  object->Name = strdup(szName);
  object->identifier = fnv1a32(object->Name);
}

void
OBJ_SetSourceFile(lpObject_t pobj, lpcString_t szFilename)
{
  SafeSet(pobj->SourceFile, strdup(szFilename), free);
}

void
OBJ_SetTextContent(lpObject_t pobj, lpcString_t szValue)
{
  SafeSet(pobj->TextContent, strdup(szValue), free);
  OBJ_SetDirty(pobj);
}

void
OBJ_SetClassName(lpObject_t pobj, lpcString_t szClassName)
{
  SafeSet(pobj->ClassName, strdup(szClassName), free);
}

lpcString_t OBJ_GetSourceFile(lpcObject_t object) {
  return object->SourceFile;
}

lpcString_t OBJ_GetTextContent(lpcObject_t object) {
  return object->TextContent;
}

lpObject_t OBJ_GetParent(lpcObject_t object) {
  return object->parent;
}

lpObject_t OBJ_GetFirstChild(lpcObject_t object) {
  return object->children;
}

lpObject_t OBJ_GetNext(lpcObject_t object) {
  return object->next;
}

lpObject_t OBJ_GetRoot(lpObject_t object) {
  if (object->SourceFile) {
    return object;
  } else if (object->parent) {
    return OBJ_GetRoot(object->parent);
  } else {
    return object;
  }
}

lpProperty_t OBJ_GetProperties(lpcObject_t object) {
  return object->properties;
}

long OBJ_GetTimestamp(lpcObject_t pobj) {
  return pobj->dirty;
}

uint32_t OBJ_GetStyle(lpcObject_t pobj) {
  return pobj->flags;
}

void OBJ_SetStyle(lpObject_t pobj, uint32_t dwFlags) {
  pobj->flags = dwFlags;
}

uint32_t OBJ_GetFlags(lpcObject_t pobj) {
  return pobj->flags;
}

void OBJ_SetFlags(lpObject_t pobj, uint32_t dwFlags) {
  pobj->flags = dwFlags;
}

uint32_t OBJ_GetIdentifier(lpcObject_t pobj) {
  return pobj->identifier;
}

uint32_t OBJ_GetUniqueID(lpcObject_t pobj) {
  return pobj->unique;
}

uint32_t OBJ_GetLuaObject(lpcObject_t pobj) {
  return pobj->luaObject;
}

uint32_t OBJ_GetAlias(lpcObject_t pobj) {
  return pobj->alias;
}

void OBJ_SetLuaObject(lpObject_t pobj, uint32_t lua_id) {
  pobj->luaObject = lua_id;
}

void OBJ_SetAlias(lpObject_t pobj, uint32_t alias) {
  pobj->alias = alias;
}

lua_State*
OBJ_GetDomain(lpObject_t self) {
  return self->domain;
}

lpProperty_t
OBJ_GetPropertyAtIndex(lpObject_t object,
                       uint32_t classid,
                       size_t classsize,
                       uint32_t index)
{
  void* userdata = OBJ_GetComponent(object, classid);
  if (!userdata)
    return NULL;
  lpProperty_t* props = (void*)(userdata + classsize);
  return props[index];
}
