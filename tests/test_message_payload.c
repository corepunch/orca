#include <include/api.h>
#include <core/core.h>
#include <assert.h>
#include <stdio.h>

extern int luaopen_orca(lua_State *L);
extern int luaopen_orca_core(lua_State *L);
extern int luaopen_orca_system(lua_State *L);
extern int f_peek_message(lua_State *L);
extern int f_event_gc(lua_State *L);

static unsigned disposed, destroyed;
static void *reentrant_target;

static void dispose(void *value)
{
  disposed++;
  free(value);
  if (reentrant_target) {
    void *target = reentrant_target;
    reentrant_target = NULL;
    axRemoveFromQueue(target);
    axPostMessageW(target, kEventKeyUp, 123, NULL);
  }
}

static void post_owned(void *target, uint32_t event, uint32_t word)
{
  void *value = malloc(32);
  assert(value);
  struct AXmessage message = { .target = target, .message = event, .wParam = word, .lParam = value };
  assert(axOwnMessageData(&message, value, dispose));
  axPostMessageOwnedW(message);
}

static void drain(void)
{
  struct AXmessage message;
  while (axPeekMessage(&message)) axDisposeMessage(&message);
}

static LRESULT object_proc(struct Object *self, void *component, uint32_t event, wParam_t word, lParam_t args)
{
  if (event == ID_Object_Destroy) destroyed++;
  return FALSE;
}

int main(void)
{
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  luaL_requiref(L, "orca", luaopen_orca, 1); lua_pop(L, 1);
  luaopen_orca_core(L); lua_pop(L, 1);
  luaL_requiref(L, "orca.system", luaopen_orca_system, 1); lua_pop(L, 1);
  drain();
  int a, b;
  struct AXmessage message;

  unsigned char *bytes = malloc(100000);
  for (unsigned i = 0; i < 100000; i++) bytes[i] = (unsigned char)i;
  for (unsigned i = 0; i < 8; i++) axPostMessageDataW(NULL, 0x12345678, i, bytes, 100000);
  memset(bytes, 0, 100000);
  free(bytes);
  for (unsigned i = 0; i < 8; i++) {
    assert(axPeekMessage(&message) && message.wParam == i);
    for (unsigned j = 0; j < 100000; j++) assert(((unsigned char *)message.lParam)[j] == (unsigned char)j);
    axDisposeMessage(&message);
    assert(!message.ownedPayload && !message.lParam);
    axDisposeMessage(&message);
  }
  puts("PASS: queued copies survive source mutation, arena wrap and payloads above 64 KiB");

  char *name = strdup("temporary handler");
  struct Object_BindHandlerEventArgs args = { .PropertyId = 7, .FunctionName = name };
  axPostMessageDataW(NULL, ID_Object_BindHandler, 0, &args, sizeof(args));
  memset(name, 'x', strlen(name));
  free(name);
  assert(axPeekMessage(&message));
  assert(!strcmp(((struct Object_BindHandlerEventArgs *)message.lParam)->FunctionName, "temporary handler"));
  axDisposeMessage(&message);

  static unsigned defaults;
  static struct ClassDesc cls = { .ClassName = "PayloadLifetimeProbe", .ClassSize = sizeof(defaults),
    .Defaults = &defaults, .ObjProc = object_proc };
  cls.ClassID = fnv1a32(cls.ClassName);
  assert(OBJ_RegisterClass(&cls));
  struct Object *record = OBJ_Create(cls.ClassID);
  struct DataObject_CollectionChangedEventArgs change = { .Source = record, .Version = 1 };
  axPostMessageDataW(NULL, ID_DataObject_CollectionChanged, 0, &change, sizeof(change));
  OBJ_ReleaseRef(record);
  assert(!destroyed);
  assert(axPeekMessage(&message));
  assert(((struct DataObject_CollectionChangedEventArgs *)message.lParam)->Source == record);
  axDisposeMessage(&message);
  assert(destroyed == 1);
  puts("PASS: reflected string and Object fields remain owned until payload disposal");

  post_owned(&a, kEventKeyDown, 1);
  post_owned(&a, kEventKeyDown, 2);
  post_owned(&b, kEventKeyDown, 1);
  axRemoveMessages(&a, kEventKeyDown, 1);
  assert(disposed == 1);
  reentrant_target = &b;
  axRemoveFromQueue(&a);
  assert(disposed == 3);
  assert(axPeekMessage(&message) && message.target == &b && message.wParam == 123);
  axDisposeMessage(&message);
  assert(!axPeekMessage(&message));
  // Cocoa/iOS coalesce paints; other backends retain both queue entries.
  // Either policy must preserve targets and release every payload exactly once.
  unsigned before_paints = disposed;
  post_owned(&a, kEventWindowPaint, 1);
  post_owned(&a, kEventWindowPaint, 2);
  bool_t coalesced = disposed == before_paints + 1;
  assert(disposed == before_paints || coalesced);
  unsigned after_same_target = disposed;
  post_owned(&b, kEventWindowPaint, 3);
  assert(disposed == after_same_target);
  assert(axPeekMessage(&message) && message.target == &a && message.message == kEventWindowPaint);
  if (!coalesced) {
    assert(message.wParam == 1);
    axDisposeMessage(&message);
    assert(axPeekMessage(&message) && message.target == &a && message.message == kEventWindowPaint);
  }
  assert(message.wParam == 2);
  axDisposeMessage(&message);
  assert(axPeekMessage(&message) && message.target == &b && message.wParam == 3 &&
         message.message == kEventWindowPaint);
  axDisposeMessage(&message);
  assert(!axPeekMessage(&message));
  assert(disposed == before_paints + 3);
  post_owned(&a, kEventKeyDown, 1);
  axClearMessageQueue();
  assert(disposed == 7);
  puts("PASS: selective cancellation, reentrant disposal, backend paint policy and clear release once");

  post_owned(NULL, 0x12345678, 0);
  assert(f_peek_message(L) == 1);
  lua_pop(L, 1);
  lua_gc(L, LUA_GCCOLLECT);
  assert(disposed == 8);
  puts("PASS: an abandoned Lua Event releases its queued payload during collection");

  post_owned(NULL, 0x12345678, 0);
  assert(axPeekMessage(&message));
  assert(!SV_DispatchMessage(L, &message));
  assert(disposed == 9 && !message.ownedPayload && !message.lParam);
  puts("PASS: dispatch releases an unhandled message payload");

  for (unsigned i = 0; i < 65535; i++) axPostMessageW(NULL, 0x12345678, i, NULL);
  post_owned(NULL, 0x12345678, 0);
  assert(disposed == 10);
  axClearMessageQueue();
  puts("PASS: a full ring disposes rejected owned payloads");
  lua_close(L);
  return 0;
}
