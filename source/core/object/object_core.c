#include "object_internal.h"
#include "../property/property_internal.h"

#include <include/api.h>
#include <filesystem/filesystem.h>

extern struct Object *
OBJ_FindChild(struct Object *object, lpcString_t name, bool_t recursive);

extern int
luaX_readProperty(lua_State* L, int idx, struct Property *p);

static uint32_t unique_counter = 0;
static int64_t g_object_count = 0;
static void OBJ_Release(struct Object *pobj);

static struct {
  void *target;             // target of the message taken from the queue
  struct Object *released;  // that target, once its last reference went
} in_flight;

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
    if (pobj == in_flight.target) in_flight.released = pobj;
    else OBJ_Release(pobj);
    return 0;
  }
  return pobj->refcount;
}

void
OBJ_EndMessage(void *target)
{
  if (target != in_flight.target) return;
  struct Object *released = in_flight.released;
  in_flight.target = NULL;
  in_flight.released = NULL;
  // A handler may have taken a new reference since.
  if (released && released->refcount == 0) OBJ_Release(released);
}

void
OBJ_BeginMessage(void *target)
{
  OBJ_EndMessage(in_flight.target);
  in_flight.target = target;
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
    PROP_InvalidateBindingPaths();
    struct Object *parent = self->parent;
    REMOVE_FROM_LIST(struct Object, self, parent->children);
    self->parent = NULL;
    OBJ_SetDirty(parent);
    OBJ_NotifyCollectionChanged(parent);
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
  pobj->flags &= ~(OF_DIRTY | OF_SCROLL);
  FOR_EACH_CHILD(pobj, OBJ_ClearDirtyFlags);
}

void
OBJ_Clear(struct Object *pobj)
{
  if (GetDataObject(pobj)) {
    OBJ_BeginUpdate(pobj);
    FOR_EACH_OBJECT(other, pobj) OBJ_RemoveFromParent(other);
    OBJ_EndUpdate(pobj);
    return;
  }
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
  OBJ_RemoveFromQueue(self);
  if (had_parent) {
    OBJ_ReleaseRef(self);
  }
}

static void
OBJ_Release(struct Object *pobj)
{
  PROP_InvalidateBindingPaths();
  OBJ_ClearCollectionObservers(pobj);
#ifdef DEBUG_COUNT_OBJECTS
  counter--;
#endif
  g_object_count--;
  OBJ_Clear(pobj);
  OBJ_SendMessageW(pobj, ID_Object_Destroy, 0, NULL);
  OBJ_DetachFromParent(pobj);
  if (core.focus == pobj) core.focus = NULL;
  if (core.hover == pobj) core.hover = NULL;
  if (core.hover2 == pobj) core.hover2 = NULL;
  OBJ_RemoveFromQueue(pobj);

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
  if (pobj->body_ref) {
    axPostMessageW(NULL, kEventClearReference, pobj->body_ref, NULL);
    pobj->body_ref = 0;
  }
  if (pobj->controller_ref) {
    axPostMessageW(NULL, kEventClearReference, pobj->controller_ref, NULL);
    pobj->controller_ref = 0;
  }

  // Dispatch Object.Release so components (e.g., StyleController) can clean up.
  // Script-side cleanup is deferred through kEventClearReference.
  OBJ_SendMessageW(pobj, ID_Object_Release, 0, NULL);
  OBJ_ReleaseProperties(pobj);
  OBJ_ReleaseComponents(pobj);

  SafeFree(pobj->SourceFile);
  SafeFree(pobj->TextContent);
  SafeFree(pobj->Name);

  pobj->refcount = 0;
  free(pobj);
}

HRESULT
_RegisterProperty(struct Object *object, struct Property *property)
{
  struct PropertyType const *desc = PROP_GetDesc(property);
  PROP_AddToList(property, &object->properties);
  if (!object->components || !CMP_SetProperty(object->components, property)) {
    uint32_t psize = PROP_GetSize(property);
    if (desc && desc->IsArray) {
      psize = sizeof(void *) + sizeof(int);
    }
    if (psize == 0) {
      psize = 1;
    }
    PROP_SetValuePtr(property, ZeroAlloc(psize));
    PROP_SetFlag(property, PF_OWNS_STORAGE);
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
  for (i = 0; i < MAX_TAGS && *core.tags[i]; i++) {
    if (!strcmp(core.tags[i], tag)) {
      return i;
    }
  }
  if (i < MAX_TAGS) {
    strncpy(core.tags[i], tag, sizeof(core.tags[i]) - 1);
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
      tags |= ((objectTags_t)1 << tag_id);
    }
  }
  free(tmp);
  return tags;
}

struct TimerCallback {
  struct TimerCallback *next, **previous;
  struct Object *owner;
  int timerId;
};
static struct TimerCallback *timer_callbacks;

static void unlink_timer_callback(struct TimerCallback *call)
{
  if (!call->previous) return;
  *call->previous = call->next;
  if (call->next) call->next->previous = call->previous;
  call->previous = NULL;
}

static int collect_timer_callback(lua_State *L)
{
  unlink_timer_callback(lua_touserdata(L, 1));
  return 0;
}

static int finish_timer_callback(lua_State *L, int status, lua_KContext context) { return 0; }

static int run_timer_callback(lua_State *L)
{
  struct TimerCallback *call = lua_touserdata(L, lua_upvalueindex(2));
  if (!call->previous) return 0;
  // Once started, cancellation must not interrupt a yielding handler.
  unlink_timer_callback(call);
  int arguments = lua_gettop(L);
  lua_pushvalue(L, lua_upvalueindex(1));
  lua_insert(L, 1);
  lua_callk(L, arguments, 0, 0, finish_timer_callback);
  return 0;
}

void OBJ_GuardTimerCallback(lua_State *L, struct Object *owner, int timerId)
{
  // Install finalization before linking a token; later Lua allocations may fail.
  if (luaL_newmetatable(L, "orca.TimerCallback")) {
    lua_pushcfunction(L, collect_timer_callback);
    lua_setfield(L, -2, "__gc");
  }
  lua_pop(L, 1);
  struct TimerCallback *call = lua_newuserdata(L, sizeof(*call));
  *call = (struct TimerCallback){ .owner = owner, .timerId = timerId };
  luaL_getmetatable(L, "orca.TimerCallback");
  lua_setmetatable(L, -2);
  call->next = timer_callbacks;
  call->previous = &timer_callbacks;
  if (call->next) call->next->previous = &call->next;
  timer_callbacks = call;
  // Queued coroutine arguments retain the sender until this callback runs.
  lua_pushcclosure(L, run_timer_callback, 2);
}

int OBJ_SetTimer(struct Object *self, int duration)
{
  return axSetTimer(self, MAX(duration, 1), NULL, TRUE);
}

void OBJ_CancelTimer(struct Object *self, int timerId)
{
  if (timerId <= 0) return;
  axCancelTimer((uint32_t)timerId);
  // A tick queued before cancellation must not reach the handler.
  axRemoveMessages(self, kEventTimer, (uint32_t)timerId);
  // Native ticks may already have become queued Lua callback coroutines.
  for (struct TimerCallback *call = timer_callbacks, *next; call; call = next) {
    next = call->next;
    if (call->owner != self || call->timerId != timerId) continue;
    unlink_timer_callback(call);
  }
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

struct Object *
OBJ_Instantiate(struct Object *prefab)
{
  const char* scene = FS_SerializeObjectToString(prefab);
  struct Object* obj = FS_LoadObjectFromString(scene);
  free((char*)scene);
  return obj;
}

// Closure stored by BindHandler / wire_controller:
// upvalue 1 = controller table, upvalue 2 = function.
// When the event fires, orca.async calls this as func(node, args, sender).
// We replace node with the controller table so handlers see self = controller.
int
_bind_handler_closure(lua_State *L)
{
  lua_pushvalue(L, lua_upvalueindex(1)); // push controller table
  lua_replace(L, 1);                     // replace node self with controller table
  lua_pushvalue(L, lua_upvalueindex(2)); // push the function
  lua_insert(L, 1);                      // move it before self
  lua_call(L, lua_gettop(L) - 1, LUA_MULTRET);
  return lua_gettop(L);
}

static void
wire_controller(lua_State *L, int tbl, struct Object *target)
{
  tbl = lua_absindex(L, tbl);
  lua_pushnil(L);
  while (lua_next(L, tbl)) {
    if (lua_type(L, -2) != LUA_TSTRING || lua_type(L, -1) != LUA_TFUNCTION) {
      lua_pop(L, 1); continue;
    }
    const char *key = lua_tostring(L, -2);
    const char *sep = strrchr(key, '_');
    struct Object *obj = target;
    const char *event = key;
    if (sep) {
      char name[MAX_PROPERTY_STRING];
      size_t len = sep - key < (int)sizeof(name) - 1 ? (size_t)(sep - key) : sizeof(name) - 1;
      memcpy(name, key, len); name[len] = '\0';
      struct Object *child = OBJ_FindChild(target, name, TRUE);
      if (child) { obj = child; event = sep + 1; }
    }
    struct Property *ep = OBJ_FindShortProperty(obj, fnv1a32(event));
    if (ep && PROP_GetType(ep) == kDataTypeEvent && !ep->handlerName && !ep->handlerOverride) {
      // Wrap in closure: upvalue 1 = controller table, upvalue 2 = function.
      // This makes self = controller (with self.view = root node) in handlers.
      lua_pushvalue(L, tbl);
      lua_insert(L, -2);
      lua_pushcclosure(L, _bind_handler_closure, 2);
      luaX_readProperty(L, -1, ep);
      ep->handlerOverride = FALSE;
      lua_pop(L, 1);
    } else {
      lua_pop(L, 1);
    }
  }
}

static void
rebind_controller_handlers(struct Object *target)
{
  FOR_EACH_PROPERTY(property, OBJ_GetProperties(target)) {
    struct Property *event = (struct Property *)property;
    if (PROP_GetType(event) == kDataTypeEvent && event->handlerName && *event->handlerName)
      PROP_BindHandler(event, event->handlerName);
  }
  FOR_EACH_OBJECT(child, target) rebind_controller_handlers(child);
}

void
OBJ_LoadController(lua_State *L, struct Object *self, const char *path)
{
  if (!path || !*path) return;

  // Load file via package searcher (fs_findmodule) without require's cache.
  lua_getglobal(L, "fs_findmodule");
  lua_pushstring(L, path);
  if (lua_pcall(L, 1, 1, 0) != LUA_OK || lua_isnil(L, -1)) {
    // Shared modules (share/orca/...) live on package.path, not in a project.
    lua_pop(L, 1);
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "searchpath");
    lua_pushstring(L, path);
    lua_getfield(L, -3, "path");
    lua_remove(L, -4);
    const char *file = lua_pcall(L, 2, 1, 0) == LUA_OK ? lua_tostring(L, -1) : NULL;
    if (!file || luaL_loadfile(L, file) != LUA_OK) {
      Con_Error("LoadController: cannot find '%s'", path);
      lua_pop(L, file ? 2 : 1);
      return;
    }
    lua_remove(L, -2);
  }

  // Execute the loaded chunk to get the controller table.
  if (lua_pcall(L, 0, 1, 0) != LUA_OK || !lua_istable(L, -1)) {
    Con_Error("LoadController: '%s' did not return a table", path);
    lua_pop(L, 1);
    return;
  }

  lua_pushvalue(L, -1);
  if (self->controller_ref) luaL_unref(L, LUA_REGISTRYINDEX, self->controller_ref);
  self->controller_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  struct Property *controller = OBJ_FindShortProperty(self, fnv1a32("Controller"));
  if (controller && PROP_GetType(controller) == kDataTypeString)
    PROP_SetStringValue(controller, path);

  // Set controller.view = the root node so handlers can access the view hierarchy.
  luaX_pushObject(L, self);
  lua_setfield(L, -2, "view");

  wire_controller(L, -1, self);
  rebind_controller_handlers(self);
  lua_pop(L, 1);
}

bool_t
OBJ_FindControllerFunction(lua_State *L, struct Object *obj, const char *name)
{
  if (!name || !*name) return FALSE;
  for (struct Object *hobj = obj; hobj; hobj = OBJ_GetParent(hobj)) {
    if (!hobj->controller_ref) continue;
    lua_geti(L, LUA_REGISTRYINDEX, hobj->controller_ref);
    if (!lua_istable(L, -1)) { lua_pop(L, 1); continue; }
    lua_getfield(L, -1, name);
    if (lua_isfunction(L, -1)) {
      lua_remove(L, -2); // drop table, leave function on top
      return TRUE;
    }
    lua_pop(L, 2); // nil, table
  }
  return FALSE;
}
