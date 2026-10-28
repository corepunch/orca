#include <assert.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <include/orca.h>
#include <core/core.h>

extern int luaopen_orca(lua_State *L);
extern int luaopen_orca_core(lua_State *L);

struct test_observer {
  struct Object *source, *other;
  uint32_t calls, version;
  bool_t reenter;
};

static unsigned destroyed;

static LRESULT
ObserverProc(struct Object *self, void *component, uint32_t message, wParam_t wp, lParam_t lp)
{
  struct test_observer *observer = component;
  if (message == ID_Object_Destroy) { destroyed++; return FALSE; }
  if (message != ID_DataObject_CollectionChanged) return FALSE;
  struct DataObject_CollectionChangedEventArgs *event = lp;
  assert(event->Source == observer->source && event->Version > observer->version);
  observer->calls++;
  observer->version = event->Version;
  if (observer->other) core_UnobserveCollection(observer->source, observer->other);
  if (observer->reenter && observer->calls == 1) {
    OBJ_BeginUpdate(observer->source);
    struct Object *row = OBJ_Create(ID_DataObject);
    OBJ_AddChild(observer->source, row);
    OBJ_ReleaseRef(row);
    OBJ_EndUpdate(observer->source);
  } else if (observer->reenter) core_UnobserveCollection(observer->source, self);
  return TRUE;
}

int main(void)
{
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  luaL_requiref(L, "orca", luaopen_orca, 1);
  lua_pop(L, 1);
  luaopen_orca_core(L);
  lua_pop(L, 1);

  static struct test_observer const defaults;
  static struct ClassDesc observerClass = {
    .ClassName = "CollectionTestObserver", .ClassSize = sizeof(struct test_observer),
    .Defaults = &defaults, .ObjProc = ObserverProc,
  };
  observerClass.ClassID = fnv1a32(observerClass.ClassName);
  assert(OBJ_RegisterClass(&observerClass));
  struct Object *source = OBJ_Create(ID_DataObject);
  struct Object *self = OBJ_Create(observerClass.ClassID), *other = OBJ_Create(observerClass.ClassID);
  struct test_observer *observer = OBJ_GetComponent(self, observerClass.ClassID);
  struct test_observer *skipped = OBJ_GetComponent(other, observerClass.ClassID);
  observer->source = skipped->source = source;
  observer->other = other;
  observer->reenter = TRUE;
  assert(!core_ObserveCollection(self, other));
  assert(core_ObserveCollection(source, other));
  assert(core_ObserveCollection(source, self) && core_ObserveCollection(source, self));
  struct Object *row = OBJ_Create(ID_DataObject);
  OBJ_AddChild(source, row);
  assert(observer->calls == 2 && skipped->calls == 0);
  OBJ_MoveBefore(row, NULL);
  assert(observer->calls == 2);
  puts("PASS: Duplicate subscriptions, reentrant mutation and peer/self unsubscription");

  observer->other = NULL;
  observer->calls = 0;
  assert(core_ObserveCollection(source, other));
  assert(core_ObserveCollection(source, self));
  struct Object *trigger = OBJ_Create(ID_DataObject);
  OBJ_AddChild(source, trigger);
  OBJ_ReleaseRef(trigger);
  assert(observer->calls == 2 && skipped->calls == 2);
  assert(observer->version == skipped->version);
  core_UnobserveCollection(source, other);
  puts("PASS: Reentrant changes reach all observers in increasing version order");

  observer->reenter = FALSE;
  assert(core_ObserveCollection(source, self));
  struct Object *nested = OBJ_Create(ID_DataObject);
  OBJ_AddChild(row, nested);
  observer->calls = 0;
  OBJ_Clear(source);
  assert(observer->calls == 1 && OBJ_GetParent(row) == NULL);
  assert(OBJ_GetFirstChild(row) == nested && OBJ_GetParent(nested) == row);
  OBJ_Clear(source);
  assert(observer->calls == 1);
  core_UnobserveCollection(source, self);
  puts("PASS: Collection clear batches notifications and preserves retained nested record data");

  struct Object *ephemeral = OBJ_Create(observerClass.ClassID);
  assert(core_ObserveCollection(source, ephemeral));
  unsigned before = destroyed;
  OBJ_ReleaseRef(ephemeral);
  assert(destroyed == before + 1);
  OBJ_AddChild(source, row);
  assert(core_ObserveCollection(source, self));
  uint32_t calls = observer->calls;
  OBJ_ReleaseRef(source);
  assert(observer->calls == calls);
  OBJ_ReleaseRef(self);
  OBJ_ReleaseRef(other);
  OBJ_ReleaseRef(row);
  OBJ_ReleaseRef(nested);
  assert(OBJ_UnregisterClass(observerClass.ClassID));
  lua_close(L);
  puts("PASS: Weak subscriptions automatically detach on source or observer destruction");
  return 0;
}
