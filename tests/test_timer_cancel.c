#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <include/api.h>
#include <include/orca.h>
#include <core/core.h>
#include <UIKit/UIKit.h>
#include <assert.h>
#include <stdio.h>

extern int luaopen_orca(lua_State *L);
extern int luaopen_orca_core(lua_State *L);
extern int luaopen_orca_UIKit(lua_State *L);

/* A late tick can find the next one of the same repeating timer already
 * queued. Cancelling the timer must drop those ticks and nothing else. */
int main(void)
{
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  assert(luaL_dostring(L, "package.path = 'share/?.lua;share/?/init.lua;' .. package.path") == LUA_OK);
  luaL_requiref(L, "orca", luaopen_orca, 1); lua_pop(L, 1);
  luaL_requiref(L, "orca.core", luaopen_orca_core, 1); lua_pop(L, 1);
  luaL_requiref(L, "orca.UIKit", luaopen_orca_UIKit, 1); lua_pop(L, 1);

  struct AXmessage msg;
  while (axPeekMessage(&msg)) {}
  struct Object *owner = OBJ_Create(ID_Node2D), *other = OBJ_Create(ID_Node2D);
  int cancelled = OBJ_SetTimer(owner, 60000), kept = OBJ_SetTimer(owner, 60000);
  int foreign = OBJ_SetTimer(other, 60000);
  assert(cancelled > 0 && kept > 0 && foreign > 0);

  axPostMessageW(owner, kEventTimer, cancelled, NULL);
  axPostMessageW(owner, kEventTimer, kept, NULL);
  axPostMessageW(other, kEventTimer, foreign, NULL);
  axPostMessageW(owner, kEventKeyDown, cancelled, NULL);
  axPostMessageW(owner, kEventTimer, cancelled, NULL);
  OBJ_CancelTimer(owner, cancelled);

  assert(axPeekMessage(&msg) && msg.target == owner && msg.message == kEventTimer && (int)msg.wParam == kept);
  assert(axPeekMessage(&msg) && msg.target == other && msg.message == kEventTimer && (int)msg.wParam == foreign);
  assert(axPeekMessage(&msg) && msg.target == owner && msg.message == kEventKeyDown);
  assert(!axPeekMessage(&msg));

  OBJ_CancelTimer(owner, kept);
  OBJ_CancelTimer(other, foreign);
  OBJ_ReleaseRef(owner);
  OBJ_ReleaseRef(other);
  assert(luaL_dostring(L,
    "calls, resumed, counts = 0, 0, {}; "
    "owner = require('orca.core').Timer { Tick = function(self, args) "
    "calls = calls + 1; counts[args.timerId] = (counts[args.timerId] or 0) + 1; "
    "self:cancelTimer(args.timerId); coroutine.yield(); resumed = resumed + 1 end }; "
    "timerId = owner:setTimer(60000)") == LUA_OK);
  lua_getglobal(L, "owner");
  struct Object *timer = luaX_checkObject(L, -1);
  lua_pop(L, 1);
  lua_getglobal(L, "timerId");
  int timerId = (int)lua_tointeger(L, -1);
  lua_pop(L, 1);
  // A stalled native loop can dispatch several expirations before Lua starts.
  for (int i = 0; i < 2; i++) {
    struct AXmessage tick = { .target = timer, .message = kEventTimer, .wParam = timerId };
    OBJ_BeginMessage(timer);
    assert(SV_DispatchMessage(L, &tick));
  }
  int independent = OBJ_SetTimer(timer, 60000);
  struct AXmessage independentTick = { .target = timer, .message = kEventTimer, .wParam = independent };
  OBJ_BeginMessage(timer);
  assert(SV_DispatchMessage(L, &independentTick));
  core_FlushQueue(L);
  assert(luaL_dostring(L, "assert(calls == 2 and counts[timerId] == 1 and resumed == 2, 'Cancellation must drop queued Lua callbacks and resume the running handler')") == LUA_OK);
  // Cancellation before either queued callback begins must suppress both.
  assert(luaL_dostring(L, "calls, resumed = 0, 0; timerId = owner:setTimer(60000)") == LUA_OK);
  lua_getglobal(L, "timerId");
  timerId = (int)lua_tointeger(L, -1);
  lua_pop(L, 1);
  for (int i = 0; i < 2; i++) {
    struct AXmessage tick = { .target = timer, .message = kEventTimer, .wParam = timerId };
    OBJ_BeginMessage(timer);
    assert(SV_DispatchMessage(L, &tick));
  }
  OBJ_CancelTimer(timer, timerId);
  lua_gc(L, LUA_GCCOLLECT);
  core_FlushQueue(L);
  assert(luaL_dostring(L, "assert(calls == 0 and resumed == 0)") == LUA_OK);
  lua_close(L);
  puts("PASS: cancelling a timer drops its queued ticks and keeps every other message");
  return 0;
}
