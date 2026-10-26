#include <include/api.h>
#include <include/renderer.h>
#include <assert.h>
#include <stdio.h>

/* Exercise the iOS queue and yielding Lua iterator on the host without UIKit. */
extern void ios_begin_message_frame(void);
extern void ios_enqueue(struct AXmessage message);
extern void ios_shutdown_messages(void);
extern int f_get_message(lua_State *L);

static int animations, paints, touches, timers, handled;
static bool_t flinging = TRUE, repainting = TRUE;

static int dispatch(lua_State *L)
{
  struct AXmessage *message = luaL_checkudata(L, 1, "Event");
  if (++handled > 100) return luaL_error(L, "A display callback starved the UIKit run loop");
  if (message->message == ID_Object_Animate && flinging) {
    animations++;
    axPostMessageW(NULL, ID_Object_Animate, 0, NULL);
  } else if (message->message == ID_Window_Paint) {
    paints++;
    if (repainting) axPostMessageW(NULL, kEventWindowPaint, 0, NULL);
  } else if (message->message == kEventScrollWheel) {
    assert(message->pointer.flags & AX_POINTER_TOUCH);
    if (message->pointer.flags & AX_POINTER_SCROLL_BEGIN) flinging = FALSE;
    touches++;
  } else if (message->message == kEventTimer) {
    timers++;
    if (message->wParam) return lua_yield(L, 0);
  }
  return 0;
}

static void frame(lua_State *L, lua_State *thread)
{
  ios_begin_message_frame();
  int results = 0, status = lua_resume(thread, L, 0, &results);
  if (status != LUA_YIELD) fprintf(stderr, "%s\n", lua_tostring(thread, -1));
  assert(status == LUA_YIELD && results == 0);
}

int main(void)
{
  lua_State *L = luaL_newstate();
  assert(L);
  luaL_openlibs(L);
  luaL_newmetatable(L, "Event");
  lua_pop(L, 1);
  lua_pushcfunction(L, f_get_message); lua_setglobal(L, "getMessage");
  lua_pushcfunction(L, dispatch); lua_setglobal(L, "dispatch");
  lua_State *thread = lua_newthread(L);
  assert(luaL_loadstring(thread, "for message in getMessage do dispatch(message) end") == LUA_OK);

  axPostMessageW(NULL, ID_Object_Animate, 0, NULL);
  axPostMessageW(NULL, kEventWindowPaint, 0, NULL);
  frame(L, thread);
  assert(animations == 1 && paints == 1 && handled == 2);
  struct AXmessage message;
  assert(!axPeekMessage(&message)); // Successors wait for the next display callback.

  // UIKit can now deliver a new gesture while the old animation remains queued.
  uint32_t phases[] = { AX_POINTER_SCROLL_BEGIN, AX_POINTER_SCROLL_UPDATE, AX_POINTER_SCROLL_END };
  for (size_t i = 0; i < sizeof(phases) / sizeof(*phases); i++)
    ios_enqueue((struct AXmessage){ .message = kEventScrollWheel,
      .pointer = { .flags = AX_POINTER_TOUCH | phases[i], .time = 100 + 16 * i } });
  axPostMessageW(NULL, kEventTimer, 0, NULL);
  frame(L, thread);
  assert(animations == 2 && paints == 2 && touches == 3 && timers == 1 && !flinging);
  frame(L, thread);
  assert(animations == 2 && paints == 3); // A stale animation cannot revive the fling.

  // An application callback can itself yield before the batch has been drained.
  axPostMessageW(NULL, kEventTimer, 1, NULL);
  axPostMessageW(NULL, kEventTimer, 0, NULL);
  frame(L, thread);
  assert(timers == 2);
  frame(L, thread);
  assert(timers == 3 && touches == 3);

  repainting = FALSE;
  frame(L, thread);
  int idle = handled;
  frame(L, thread);
  assert(handled == idle); // Empty batches still suspend instead of ending the iterator.
  ios_shutdown_messages();
  lua_close(L);
  puts("PASS: iOS Lua frame pump yields during inertia, accepts fresh touches and resumes early yields");
  return 0;
}
