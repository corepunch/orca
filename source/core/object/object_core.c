#include "object_internal.h"

static uint32_t unique_counter = 0;
static int64_t g_object_count = 0;
static void OBJ_Release(struct Object *pobj);

int64_t OBJ_GetObjectCount(void) { return g_object_count; }

uint32_t
OBJ_AddRef(struct Object *pobj)
{
  if (!pobj) {
    return 0;
  }
  return ++pobj->refcount;
}

uint32_t
OBJ_ReleaseRef(struct Object *pobj)
{
  if (!pobj) {
    return 0;
  }
  if (pobj->refcount > 0) {
    --pobj->refcount;
  }
  if (pobj->refcount == 0) {
    OBJ_Release(pobj);
    return 0;
  }
  return pobj->refcount;
}

ORCA_API struct Object *
OBJ_Create(uint32_t class_id) {
  g_object_count++;
  struct ClassDesc const *cls = OBJ_FindClassW(class_id);
   if (!cls) {
    Con_Error("Class ID 0x%08x not found\n", class_id);
    return NULL;
  }
  struct Object *object = ZeroAlloc(sizeof(struct Object));
  OBJ_AddRef(object);
  object->unique = ++unique_counter;
  OBJ_AddComponent(object, class_id);
  OBJ_SetDirty(object);
  if (cls->DefaultName) {
    OBJ_SetName(object, cls->DefaultName);
  }
  // send "create" message
  OBJ_SendMessageW(object, ID_Object_Create, 0, NULL);
  return object;
}

void
OBJ_DetachFromParent(struct Object *self)
{
  if (self->parent) {
    REMOVE_FROM_LIST(struct Object, self, self->parent->children);
    REMOVE_FROM_LIST(struct Object, self, self->parent);
    self->parent = NULL;
  }
}

void
OBJ_SetDirty(struct Object *pobj)
{
  pobj->dirty = core.realtime;
  for (struct Object *hParent = pobj; hParent; hParent = hParent->parent) {
    hParent->flags |= OF_DIRTY;
  }
}

void
OBJ_ClearDirtyFlags(struct Object *pobj)
{
  pobj->flags &= ~OF_DIRTY;
  FOR_EACH_CHILD(pobj, OBJ_ClearDirtyFlags);
}

void
OBJ_Clear(struct Object *pobj)
{
  FOR_EACH_OBJECT(other, pobj) {
    OBJ_Clear(other);
    OBJ_RemoveFromParent(other);
  }
  pobj->children = NULL;
}

void
OBJ_RemoveFromParent(struct Object *self)
{
  bool_t had_parent = self->parent != NULL;
  OBJ_DetachFromParent(self);
  if (core.focus == self) core.focus = NULL;
  if (core.hover == self) core.hover = NULL;
  if (core.hover2 == self) core.hover2 = NULL;
  axRemoveFromQueue(self);
  if (had_parent) {
    OBJ_ReleaseRef(self);
  }
}

#include "../property/property_internal.h"

static void
OBJ_ClearInheritedProperties(struct Object *pobj)
{
  for (struct Property *p = pobj->properties; p; p = PROP_GetNext(p)) {
    struct PropertyType const *desc = PROP_GetDesc(p);
    if (desc && desc->IsInherited && !PROP_IsNull(p)) {
      PROP_Clear(p);
    }
  }
}

static void
OBJ_Release(struct Object *pobj)
{
#ifdef DEBUG_COUNT_OBJECTS
  counter--;
#endif
  g_object_count--;
  OBJ_ClearInheritedProperties(pobj);
  OBJ_Clear(pobj);
  OBJ_SendMessage(pobj, "Destroy", 0, NULL);
  OBJ_DetachFromParent(pobj);
  if (core.focus == pobj) core.focus = NULL;
  if (core.hover == pobj) core.hover = NULL;
  if (core.hover2 == pobj) core.hover2 = NULL;
  axRemoveFromQueue(pobj);

  for (struct Property *p = pobj->properties; p; p = PROP_GetNext(p)) {
    if (PROP_GetType(p) == kDataTypeEvent && *(event_t*)PROP_GetValue(p)) {
      axPostMessageW(NULL, kEventClearReference, *(event_t*)PROP_GetValue(p), NULL);
    }
    if (p->changeCallback) {
      axPostMessageW(NULL, kEventClearReference, p->changeCallback, NULL);
      p->changeCallback = 0;
    }
    PROP_Clear(p);
  }

  // Dispatch Object.Release so components (e.g., StyleController) can clean up.
  // Script-side cleanup is deferred through kEventClearReference.
  OBJ_SendMessageW(pobj, ID_Object_Release, 0, NULL);
  OBJ_ReleaseProperties(pobj);
  OBJ_ReleaseComponents(pobj);

  SafeFree(pobj->SourceFile);
  SafeFree(pobj->TextContent);
  SafeFree(pobj->Name);
  SafeFree(pobj->ClassName);

  pobj->refcount = 0;
  free(pobj);
}

HRESULT
_RegisterProperty(struct Object *object, struct Property *property)
{
  struct PropertyType const *desc = PROP_GetDesc(property);
  uint32_t psize = desc && desc->IsInherited && !desc->IsArray
    ? sizeof(void *)
    : PROP_GetSize(property);
  assert(object->datasize + psize < MAX_OBJECT_DATA);
  PROP_AddToList(property, &object->properties);
  if (!CMP_SetProperty(object->components, property)) {
    PROP_SetValuePtr(property, object->data + object->datasize);
    object->datasize += psize;
  }
  return NOERROR;
}

void
OBJ_AddComponentByName(lua_State* L, struct Object *pobj, lpcString_t className)
{
  struct ClassDesc const *cls = OBJ_FindClass(className);
  if (!cls) {
    luaL_error(L, "addComponent: class '%s' not found", className);
    return;
  }
  OBJ_AddComponent(pobj, cls->ClassID);
}

static uint8_t find_tag(lpcString_t tag) {
  uint8_t i;
  for (i = 0; i < MAX_TAGS; i++) {
    if (!strcmp(core.tags[i], tag)) {
      return i;
    }
  }
  if (i < MAX_TAGS) {
    strcpy(core.tags[i], tag);
    return i;
  }
  return 0xFF;
}

ORCA_API objectTags_t GetTagsFromString(lpcString_t value) {
  objectTags_t tags = 0;
  char *tmp = strdup(value);
  for (lpcString_t tag = strtok(tmp, ","); tag; tag = strtok(NULL, ",")) {
    uint8_t tag_id = find_tag(tag);
    if (tag_id != 0xFF) {
      tags |= (1 << tag_id);
    }
  }
  free(tmp);
  return tags;
}

int OBJ_SetTimer(struct Object *self, int duration)
{
  return axSetTimer(self, MAX(duration, 1), NULL, TRUE);;
}

struct Object *
OBJ_FindChildByAlias(struct Object *object, uint32_t lParam)
{
  struct Node* node = GetNode(object);
  FOR_LOOP(i, node ? node->NumResources : 0) {
    lpcString_t key = node->Resources[i].Key;
    lpcString_t value = node->Resources[i].Value;
    if (!key || !*key || !value || !*value) {
      continue;
    }
    if (fnv1a32(key) == lParam) {
      return OBJ_FindByPath(object, value);
    }
  }
  if (OBJ_GetParent(object)) {
    return OBJ_FindChildByAlias(OBJ_GetParent(object), lParam);
  } else {
    return NULL;
  }
}

ORCA_API bool_t
UI_EnumObjectAliases(struct Object *object, EnumAliasProc proc, void* args)
{
  FOR_LOOP(i, GetNode(object) ? GetNode(object)->NumResources : 0) {
    proc(GetNode(object)->Resources[i].Key, GetNode(object)->Resources[i].Value, args);
  }
  return TRUE;
}
