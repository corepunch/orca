#include "object_internal.h"

// #define DEBUG_COUNT_OBJECTS



#ifdef DEBUG_COUNT_OBJECTS
static int counter = 0;
#endif

static uint32_t unique_counter = 0;

ORCA_API lpObject_t
OBJ_MakeNativeObject(uint32_t class_id) {
lpcClassDesc_t cls = OBJ_FindClassW(class_id);
  lpObject_t object = ZeroAlloc(sizeof(struct Object));
  object->components = OBJ_AddComponent(object, cls->ClassID);
//  object->window = WI_Get(L);
//  object->game = GetGame(L);
//  object->localization = GetLocalization(L);
  object->unique = ++unique_counter;
//  OBJ_SetDirty(object);
  return object;
}

lpObject_t
OBJ_Create(lua_State* L, lpcClassDesc_t cls)
{
  assert(offsetof(struct Object, luaObject) == LUASTATE_IN_OBJECT);

#ifdef DEBUG_COUNT_OBJECTS
  Con_Error("number objects: %d", counter++);
#endif
  lpObject_t object = lua_newuserdata(L, sizeof(struct Object));
  luaL_setmetatable(L, API_TYPE_OBJECT);
  memset(object, 0, sizeof(struct Object));
  object->unique = ++unique_counter;
  object->domain = L;
  OBJ_AddComponent(object, cls->ClassID);
  OBJ_SetDirty(object);
  OBJ_SetName(object, cls->DefaultName);
  
  return object;
}

void
OBJ_SetDirty(lpObject_t pobj)
{
  pobj->dirty = core.realtime;
  for (lpObject_t hParent = pobj; hParent; hParent = hParent->parent) {
    hParent->flags |= OF_DIRTY;
  }
}

void
OBJ_ClearDirtyFlags(lpObject_t pobj)
{
  pobj->flags &= ~OF_DIRTY;
  FOR_EACH_CHILD(pobj, OBJ_ClearDirtyFlags);
}

void
OBJ_Clear(lua_State* L, lpObject_t pobj)
{
  FOR_EACH_OBJECT(other, pobj) {
    OBJ_Clear(L, other);
    OBJ_RemoveFromParent(L, other);
  }
  pobj->children = NULL;
}

void
OBJ_RemoveFromParent(lua_State* L, lpObject_t self)
{
  if (self->parent) {
    REMOVE_FROM_LIST(struct Object, self, self->parent->children); // remove from parent's children
    REMOVE_FROM_LIST(struct Object, self, self->parent); // modal windows are at the same level
    self->parent = NULL;
  }
  if (core.focus == self) core.focus = NULL;
  if (core.hover == self) core.hover = NULL;
  if (core.hover2 == self) core.hover2 = NULL;
  OBJ_Clear(L, self);
  WI_RemoveFromQueue(self);
  if (self->luaObject) {
    luaL_unref(L, LUA_REGISTRYINDEX, self->luaObject);
    self->luaObject = 0;
  }
}

void
OBJ_Release(lua_State* L, lpObject_t pobj)
{
#ifdef DEBUG_COUNT_OBJECTS
  counter--;
#endif
  OBJ_Clear(L, pobj);
  OBJ_SendMessage(pobj, "Destroy", 0, NULL);
  OBJ_RemoveFromParent(L, pobj);

  for (lpProperty_t p = pobj->properties; p; p = PROP_GetNext(p)) {
    if (PROP_GetType(p) == kDataTypeEvent && *(event_t*)PROP_GetValue(p)) {
      luaL_unref(L, LUA_REGISTRYINDEX, *(event_t*)PROP_GetValue(p));
    }
    PROP_Clear(p);
  }

  // Dispatch Object.Release so attach-only components (e.g., StyleController) can clean up
  OBJ_SendMessageW(pobj, ID_Object_Release, 0, NULL);
  OBJ_ReleaseComponents(pobj);
  OBJ_ReleaseProperties(pobj);
  OBJ_ReleaseAliases(pobj);

  SafeFree(pobj->SourceFile);
  SafeFree(pobj->TextContent);
  SafeFree(pobj->Name);
  SafeFree(pobj->ClassName);
}

HRESULT
_RegisterProperty(lpObject_t object, lpProperty_t property)
{
  uint32_t psize = PROP_GetSize(property);
  assert(object->datasize + psize < MAX_OBJECT_DATA);
  PROP_AddToList(property, &object->properties);
  if (!CMP_SetProperty(object->components, property)) {
    PROP_SetValuePtr(property, object->data + object->datasize);
    object->datasize += psize;
  }
  return NOERROR;
}

void*
OBJ_GetObjectComponent(lpObject_t pobj, enum component_type c)
{
  return &pobj->comps[c];
}

void
OBJ_AddComponentByName(lua_State* L, lpObject_t pobj, lpcString_t className)
{
  lpcClassDesc_t cls = OBJ_FindClass(className);
  if (!cls) {
    luaL_error(L, "addComponent: class '%s' not found", className);
    return;
  }
  if (!cls->IsAttachOnly) {
    luaL_error(L, "addComponent: class '%s' is not attach-only; use it as a standalone object instead", className);
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

int OBJ_SetTimer(lpObject_t self, int duration)
{
  return WI_SetTimer(self, MAX(duration, 1), NULL, TRUE);;
}

#include <source/editor/ed_stab_object.h>
