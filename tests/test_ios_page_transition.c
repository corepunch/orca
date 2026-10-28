#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <assert.h>
#include <stdio.h>
#include <source/ios_page_transition.h>

// Exercise the production state owner with a deterministic native capture
// adapter. Renaming avoids interposition on the desktop UIKit shared library.
#define ORCA_TEST_NATIVE_TRANSITIONS 1
#define PageTransition_Duration TestTransition_Duration
#define PageTransition_Apply TestTransition_Apply
#define PageTransition_NativeChanged TestTransition_NativeChanged
#define PageTransition_NativeAnimate TestTransition_NativeAnimate
#define PageTransition_NativeActive TestTransition_NativeActive
#define PageTransition_NativePresent TestTransition_NativePresent
#include <plugins/UIKit/PageTransition.c>

extern int luaopen_orca(lua_State *L);
extern int luaopen_orca_core(lua_State *L);
extern int luaopen_orca_UIKit(lua_State *L);

static bool_t capture_ok = TRUE, overlay;
static uint64_t captured_generation;
static OrcaIOSPageTransitionCompletion captured_completion;
static struct OrcaIOSPageTransition captured_spec;

bool_t ORCA_IOS_CanPageTransition(void) { return TRUE; }
bool_t ORCA_IOS_PageTransitionActive(void) { return overlay; }
void ORCA_IOS_ClearPageTransition(void)
{
  bool_t was_active = overlay;
  overlay = FALSE;
  if (was_active && captured_completion) {
    OrcaIOSPageTransitionCompletion callback = captured_completion;
    captured_completion = NULL;
    callback(captured_generation, FALSE);
  }
}
bool_t ORCA_IOS_StartPageTransition(struct OrcaIOSPageTransition const *spec,
  uint64_t generation, OrcaIOSPageTransitionCompletion completion)
{
  if (!capture_ok) return FALSE;
  ORCA_IOS_ClearPageTransition();
  captured_spec = *spec;
  captured_generation = generation;
  captured_completion = completion;
  overlay = TRUE;
  return TRUE;
}

static int completions(struct Object *page)
{
  int count = 0;
  struct AXmessage message;
  while (axPeekMessage(&message))
    if (message.target == page && message.message == ID_Pivot_TransitionCompleted) count++;
  return count;
}

static void mode(struct Object *page, enum TransitionMode value)
{
  GetPivot(page)->TransitionMode = value;
  GetPivot(page)->_transitionElapsed = -1;
  PageTransition_NativeChanged(page, kTransitionTypeTurnstile, value);
}

static void finish(void)
{
  OrcaIOSPageTransitionCompletion callback = captured_completion;
  captured_completion = NULL;
  callback(captured_generation, TRUE);
}

int main(void)
{
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  assert(luaL_dostring(L, "package.path = 'share/?.lua;share/?/init.lua;' .. package.path") == LUA_OK);
  luaL_requiref(L, "orca", luaopen_orca, 1); lua_pop(L, 1);
  luaL_requiref(L, "orca.core", luaopen_orca_core, 1); lua_pop(L, 1);
  luaL_requiref(L, "orca.UIKit", luaopen_orca_UIKit, 1); lua_pop(L, 1);
  struct Object *screen = OBJ_Create(ID_Screen), *other = OBJ_Create(ID_Screen);
  struct Object *page = OBJ_Create(ID_Pivot);
  OBJ_AddChild(screen, page);
  GetNode(page)->Size.Axis[0].Actual = 390;
  GetNode(page)->Size.Axis[1].Actual = 844;
  GetNode2D(page)->Matrix = MAT4_Identity();
  struct Object *footer = OBJ_Create(ID_Node2D);
  OBJ_AddChild(screen, footer);
  GetNode2D(footer)->SafeAreaEdge = kSafeAreaEdgeBottom;
  GetNode2D(footer)->Matrix = MAT4_Identity();
  GetNode2D(footer)->Matrix.v[13] = 786;
  completions(page);

  mode(page, kTransitionModeForwardIn);
  assert(PageTransition_NativeActive(page) && PageTransition_NativeAnimate(page));
  PageTransition_NativePresent(screen, 390, 844);
  assert(overlay && !captured_spec.hold && captured_spec.fromAngle == 80 && captured_spec.toAngle == 0);
  assert(captured_spec.height == 786); // the fixed application bar never rotates
  finish();
  PageTransition_NativeAnimate(page);
  assert(GetPivot(page)->TransitionMode == kTransitionModeNone);
  assert(!PageTransition_NativeActive(page) && completions(page) == 1);
  PageTransition_NativeAnimate(page);
  assert(completions(page) == 0);

  mode(page, kTransitionModeForwardOut);
  PageTransition_NativePresent(screen, 390, 844);
  assert(captured_spec.hold);
  finish();
  PageTransition_NativeAnimate(page);
  assert(GetPivot(page)->TransitionMode == kTransitionModeForwardOut);
  assert(PageTransition_NativeActive(page) && overlay && completions(page) == 1);
  // A scene pause after an exit already completed must not navigate twice.
  ORCA_IOS_ClearPageTransition();
  ORCA_IOS_CancelPageTransitionRequests();
  assert(!PageTransition_NativeActive(page) && completions(page) == 0);
  mode(page, kTransitionModeForwardOut);
  PageTransition_NativePresent(screen, 390, 844);
  finish(); PageTransition_NativeAnimate(page);
  assert(completions(page) == 1);
  // Showing a retained tab without an entrance removes the held exit.
  PageTransition_NativePresent(other, 390, 844);
  assert(!PageTransition_NativeActive(page) && !overlay);

  mode(page, kTransitionModeBackwardIn);
  PageTransition_NativePresent(screen, 390, 844);
  uint64_t stale_generation = captured_generation;
  OrcaIOSPageTransitionCompletion stale_completion = captured_completion;
  mode(page, kTransitionModeForwardIn);
  PageTransition_NativePresent(screen, 390, 844);
  stale_completion(stale_generation, TRUE);
  assert(GetPivot(page)->TransitionMode == kTransitionModeForwardIn && completions(page) == 0);
  finish(); PageTransition_NativeAnimate(page);
  assert(GetPivot(page)->TransitionMode == kTransitionModeNone && completions(page) == 1);

  // Background/rotation cancellation resolves navigation without waiting for
  // another display callback, including before the initial capture paint.
  mode(page, kTransitionModeBackwardIn);
  PageTransition_NativePresent(screen, 390, 844);
  ORCA_IOS_ClearPageTransition();
  assert(!PageTransition_NativeActive(page) && GetPivot(page)->TransitionMode == kTransitionModeNone);
  assert(completions(page) == 1);
  mode(page, kTransitionModeForwardOut);
  ORCA_IOS_CancelPageTransitionRequests();
  assert(!PageTransition_NativeActive(page) && completions(page) == 1);

  capture_ok = FALSE;
  mode(page, kTransitionModeForwardIn);
  PageTransition_NativePresent(screen, 390, 844);
  assert(!PageTransition_NativeActive(page) && !overlay);
  assert(GetPivot(page)->TransitionMode == kTransitionModeForwardIn && completions(page) == 0);
  mode(page, kTransitionModeNone);
  OBJ_ReleaseRef(page); OBJ_ReleaseRef(footer); OBJ_ReleaseRef(screen); OBJ_ReleaseRef(other);
  lua_close(L);
  puts("PASS: native transition capture, exactly-once completion, held exits, stale callbacks, cancellation and renderer fallback");
  return 0;
}
