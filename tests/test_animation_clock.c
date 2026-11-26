#include <include/orca.h>
#include <include/api.h>
#include <core/core.h>
#include <UIKit/UIKit.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

// Exercise the exported private clock seam without depending on core internals.
extern uint32_t ANIM_GetTime(void);
extern void ANIM_SetClock(uint32_t (*clock)(void *), void *context);
extern void ANIM_BeginFrame(void);
extern void ANIM_EndFrame(void);
extern void ANIM_SetPaused(bool_t paused);
// Component detach is private; the regression exercises mutation during dispatch.
extern void CMP_Detach(void *userdata);

extern int luaopen_orca(lua_State *L);
extern int luaopen_orca_core(lua_State *L);
extern int luaopen_orca_UIKit(lua_State *L);

struct FakeClock { uint32_t now; };

enum MutationRole { Mutator, Victim, Tail, Replacement };
struct MutationFixture {
  void *victim;
  int ticks[4], releases;
  bool_t armed, nested, detach_tail, release_owner;
};
struct MutationComponent {
  struct MutationFixture *fixture;
  enum MutationRole role;
};
static struct ClassDesc mutation_class;

static void *add_mutation_component(struct Object *object, struct MutationFixture *fixture,
                                   enum MutationRole role)
{
  struct MutationComponent *component = CMP_GetUserData(OBJ_AddComponent(object, mutation_class.ClassID));
  component->fixture = fixture;
  component->role = role;
  return component;
}

static LRESULT mutation_message(struct Object *object, void *userdata, uint32_t message,
                               wParam_t wParam, lParam_t lParam)
{
  (void)wParam; (void)lParam;
  struct MutationComponent *component = userdata;
  struct MutationFixture *fixture = component->fixture;
  if (message == ID_Object_Release) {
    fixture->releases++;
    return FALSE;
  }
  if (message == ID_Object_Animate) {
    fixture->ticks[component->role]++;
    if (component->role == Mutator && fixture->armed) {
      fixture->armed = FALSE;
      if (fixture->release_owner) OBJ_ReleaseRef(object);
      else {
        CMP_Detach(fixture->victim);
        add_mutation_component(object, fixture, Replacement);
        if (fixture->nested) OBJ_SendMessageW(object, ID_Object_Animate, 0, NULL);
      }
    } else if (component->role == Tail && fixture->detach_tail) CMP_Detach(component);
    return TRUE;
  }
  if (message == ID_Object_Timer) {
    fixture->ticks[component->role]++;
    return TRUE;
  }
  return FALSE;
}

static void test_dispatch_mutation(void)
{
  static struct ClassDesc owner_class;
  owner_class = (struct ClassDesc) {
    .ClassName = "TestAnimationOwner", .ClassID = fnv1a32("TestAnimationOwner"),
  };
  OBJ_RegisterClass(&owner_class);
  mutation_class = (struct ClassDesc) {
    .ClassName = "TestAnimationMutation", .ClassID = fnv1a32("TestAnimationMutation"),
    .ObjProc = mutation_message, .ClassSize = sizeof(struct MutationComponent),
  };
  OBJ_RegisterClass(&mutation_class);
  for (int nested = 0; nested < 2; nested++) {
    struct MutationFixture fixture = { .armed = TRUE, .nested = nested, .detach_tail = TRUE };
    struct Object *object = OBJ_Create(owner_class.ClassID);
    add_mutation_component(object, &fixture, Mutator);
    fixture.victim = add_mutation_component(object, &fixture, Victim);
    add_mutation_component(object, &fixture, Tail);
    assert(OBJ_SendMessageW(object, ID_Object_Animate, 0, NULL));
    assert(fixture.ticks[Mutator] == 1 + nested);
    assert(fixture.ticks[Victim] == 0 && fixture.ticks[Tail] == 1);
    assert(fixture.ticks[Replacement] == nested);
    OBJ_SendMessageW(object, ID_Object_Animate, 0, NULL);
    assert(fixture.ticks[Mutator] == 2 + nested && fixture.ticks[Tail] == 1);
    assert(fixture.ticks[Replacement] == 1 + nested);
    OBJ_ReleaseRef(object);
    assert(fixture.releases == 2); // detached components are already gone
  }

  struct MutationFixture fixture = { .armed = TRUE, .release_owner = TRUE };
  struct Object *object = OBJ_Create(owner_class.ClassID);
  add_mutation_component(object, &fixture, Mutator);
  add_mutation_component(object, &fixture, Tail);
  OBJ_SendMessageW(object, ID_Object_Animate, 0, NULL);
  assert(fixture.ticks[Mutator] == 1 && fixture.ticks[Tail] == 1);
  assert(fixture.releases == 2); // the last external reference went inside Animate

  fixture = (struct MutationFixture) { 0 };
  object = OBJ_Create(owner_class.ClassID);
  add_mutation_component(object, &fixture, Mutator);
  add_mutation_component(object, &fixture, Tail);
  OBJ_SendMessageW(object, ID_Object_Timer, 0, NULL);
  assert(fixture.ticks[Mutator] == 1 && fixture.ticks[Tail] == 0);
  OBJ_SendMessageW(object, ID_Object_Release, 0, NULL);
  assert(fixture.releases == 2); // Release remains synchronous
  OBJ_ReleaseRef(object);
}

static uint32_t fake_time(void *context) { return ((struct FakeClock *)context)->now; }

static void test_clock(struct FakeClock *clock)
{
  ANIM_SetClock(fake_time, clock);
  uint32_t start = ANIM_GetTime();
  clock->now += 25;
  assert(ANIM_GetTime() - start == 25);

  struct FakeClock replacement = { 900000 };
  ANIM_SetClock(fake_time, &replacement);
  assert(ANIM_GetTime() - start == 25);
  replacement.now += 40;
  assert(ANIM_GetTime() - start == 65);
  ANIM_SetClock(fake_time, clock);
  assert(ANIM_GetTime() - start == 65);

  ANIM_BeginFrame();
  uint32_t frame = ANIM_GetTime();
  clock->now += 10;
  assert(ANIM_GetTime() == frame);
  ANIM_BeginFrame();
  clock->now += 20;
  assert(ANIM_GetTime() == frame);
  ANIM_EndFrame();
  assert(ANIM_GetTime() == frame);
  ANIM_EndFrame();
  assert(ANIM_GetTime() - frame == 30);

  ANIM_SetPaused(TRUE);
  uint32_t paused = ANIM_GetTime();
  clock->now += 5000;
  assert(ANIM_GetTime() == paused);
  ANIM_SetPaused(TRUE); // repeated lifecycle notifications must not reset the pause
  clock->now += 5000;
  assert(ANIM_GetTime() == paused);
  ANIM_SetPaused(FALSE);
  assert(ANIM_GetTime() == paused);
  ANIM_SetPaused(FALSE);
  clock->now += 15;
  assert(ANIM_GetTime() - paused == 15);

  struct FakeClock wrapping = { UINT32_MAX - 10 };
  ANIM_SetClock(fake_time, &wrapping);
  uint32_t before_wrap = ANIM_GetTime();
  wrapping.now += 20;
  assert(ANIM_GetTime() - before_wrap == 20);
  ANIM_SetClock(fake_time, clock);
}

static void run(lua_State *L, char const *script)
{
  int status = luaL_dostring(L, script);
  if (status != LUA_OK) fprintf(stderr, "%s\n", lua_tostring(L, -1));
  assert(status == LUA_OK);
}

static void tick(lua_State *L, struct FakeClock *clock, uint32_t delta)
{
  clock->now += delta;
  ANIM_BeginFrame();
  run(L, "for _, node in ipairs(animated) do node:send('Object.Animate') end");
  ANIM_EndFrame();
}

static int evaluate_state(lua_State *L)
{
  struct Object *object = luaX_checkObject(L, 1);
  struct StateManagerController_ControllerChangedEventArgs args = {
    .Property = OBJ_FindLongProperty(object, ID_StateManagerController_ActiveState),
  };
  OBJ_SendMessageW(object, ID_StateManagerController_ControllerChanged, 0, &args);
  return 0;
}

static void test_components(lua_State *L, struct FakeClock *clock)
{
  run(L,
    "local core, ui = require 'orca.core', require 'orca.UIKit'\n"
    "screen = ui.Screen { Width = 200, Height = 200, ResizeMode = 'NoResize' }\n"
    "animated = {}\n"
    "local function player(node)\n"
    "  node:addComponentByName('AnimationPlayer')\n"
    "  local clip = core.AnimationClip { StopTime = 1 }\n"
    "  local curve = clip + core.AnimationCurve { Property = 'Opacity' }\n"
    "  curve.Keyframes = { { Time = 0 }, { Time = 1 } }\n"
    "  node.Clip = clip\n"
    "  node:send('AnimationPlayer.Play')\n"
    "end\n"
    "for order = 1, 2 do\n"
    "  local node = screen + ui.Node2D { Width = 0, Opacity = 1 }\n"
    "  if order == 1 then player(node) end\n"
    "  node:doTween('Node.Width', 1000, 'Linear', 'InOut', 100)\n"
    "  if order == 2 then player(node) end\n"
    "  animated[order] = node\n"
    "end\n");
  tick(L, clock, 0); // AnimationPlayer primes its previous-frame time.
  tick(L, clock, 250);
  run(L,
    "for _, node in ipairs(animated) do\n"
    "  assert(math.abs(node.Width - 25) < .001, 'tween must advance alongside player')\n"
    "  assert(math.abs(node.CurrentTime - .25) < .001, 'player must share tween time')\n"
    "  assert(node.Opacity == 0, 'player must apply its curve')\n"
    "  assert(node.Playing)\n"
    "end\n");

  ANIM_SetPaused(TRUE);
  tick(L, clock, 4000);
  run(L,
    "for _, node in ipairs(animated) do\n"
    "  assert(math.abs(node.Width - 25) < .001)\n"
    "  assert(math.abs(node.CurrentTime - .25) < .001)\n"
    "end\n");
  ANIM_SetPaused(FALSE);
  tick(L, clock, 750);
  run(L,
    "for _, node in ipairs(animated) do\n"
    "  assert(math.abs(node.Width - 100) < .001, 'tween must reach final value')\n"
    "  assert(math.abs(node.CurrentTime - 1) < .001, 'player must reach final time')\n"
    "  assert(not node.Playing, 'player must complete')\n"
    "  node:removeFromParent()\n"
    "end\n"
    "animated, screen = nil, nil\n"
    "collectgarbage('collect')\n");
}

static void test_pivot(lua_State *L, struct FakeClock *clock)
{
  run(L,
    "local ui = require 'orca.UIKit'\n"
    "screen = ui.Screen { Width = 200, Height = 200, ResizeMode = 'NoResize' }\n"
    "local page = screen + ui.Pivot { Width = 0, NavigationTransition = 'Fade' }\n"
    "page:doTween('Node.Width', 1000, 'Linear', 'InOut', 100)\n"
    "page.TransitionMode = 'ForwardIn'\n"
    "animated = { page }\n");
  tick(L, clock, 0);
  tick(L, clock, 125);
  run(L,
    "assert(animated[1].TransitionMode == 'ForwardIn')\n"
    "assert(math.abs(animated[1].Width - 12.5) < .001, 'Pivot must not suppress tween')\n");
  tick(L, clock, 125);
  run(L,
    "assert(animated[1].TransitionMode == 'None', 'page transition must complete')\n"
    "assert(math.abs(animated[1].Width - 25) < .001, 'page and tween must share clock')\n");
  tick(L, clock, 750);
  run(L,
    "assert(math.abs(animated[1].Width - 100) < .001)\n"
    "animated[1]:removeFromParent()\n"
    "animated, screen = nil, nil\n"
    "collectgarbage('collect')\n");
}

static void test_managed_states(lua_State *L, struct FakeClock *clock)
{
  run(L,
    "local core, ui = require 'orca.core', require 'orca.UIKit'\n"
    "screen = ui.Screen { Width = 200, Height = 200, ResizeMode = 'NoResize' }\n"
    "base = screen + ui.Node2D { Width = 40 }; base:setName('base')\n"
    "panel = screen + ui.Node2D {}\n"
    "label = panel + ui.TextBlock { Text = 'base' }; label:setName('label')\n"
    "panel:attachPropertyProgram('Node.Width', '{../base/Width}', 'TwoWay', true)\n"
    "core.runAllPrograms(panel)\n"
    "manager = core.StateManager()\n"
    "group = manager + core.StateGroup { ControllerProperty = 'ActiveState' }\n"
    "group:addChild(core.StateTransition { Duration = 1, Interpolation = 'Linear' })\n"
    "group:addChild(core.State { Value = 'Closed', ['Node.Width'] = 10, ['Node.Visible'] = false })\n"
    "open = group + core.State { Value = 'Open', ['Node.Width'] = 110, ['Node.Visible'] = true }\n"
    "caption = open + core.State { Path = 'label', ['TextRun.Text'] = 'open' }\n"
    "panel:addComponentByName('StateManagerController')\n"
    "panel.ActiveState = 'Closed'; panel.StateManager = manager\n"
    "panel:send('Object.Start'); core.flushQueue()\n"
    "assert(panel.Width == 10 and not panel.Visible and not panel.IsTransitioning, 'initial state is final')\n"
    "assert(base.Width == 40, 'initial override does not write to TwoWay source')\n"
    "panel.ActiveState = 'Open'; evaluateState(panel)\n"
    "assert(panel.Visible and panel.IsTransitioning, 'incoming content is visible at transition start')\n"
    "assert(label.Text == 'open', 'strings switch discretely')\n"
    "caption['TextRun.Text'] = 'changed definition'\n"
    "animated = { panel }\n");
  tick(L, clock, 500);
  run(L,
    "assert(math.abs(panel.Width - 60) < .001, 'transition reaches its numeric midpoint')\n"
    "assert(label.Text == 'open', 'active transition owns its string endpoint')\n"
    "panel.ActiveState = 'Closed'; evaluateState(panel)\n"
    "assert(math.abs(panel.Width - 60) < .001 and panel.Visible, 'reversal starts at the displayed pose')\n");
  tick(L, clock, 500);
  run(L,
    "assert(math.abs(panel.Width - 35) < .001 and panel.Visible, 'outgoing content survives its exit')\n"
    "panel.ActiveState = 'Closed'; evaluateState(panel)\n");
  tick(L, clock, 500);
  run(L,
    "assert(panel.Width == 10 and not panel.Visible and not panel.IsTransitioning, 'same state does not restart')\n"
    "base.Width = 77; require('orca.core').advanceFrame(); require('orca.core').runAllPrograms(panel)\n"
    "assert(panel.Width == 10 and base.Width == 77, 'binding updates the retained base, not the presentation')\n"
    "panel.ReducedMotion = true; panel.ActiveState = 'Open'; evaluateState(panel)\n"
    "assert(panel.Width == 110 and panel.Visible and not panel.IsTransitioning, 'reduced motion reaches the same final state')\n"
    "assert(base.Width == 77, 'animation never propagates to the TwoWay source')\n"
    "panel.ReducedMotion = false; panel.ActiveState = 'Unmatched'; evaluateState(panel)\n");
  tick(L, clock, 500);
  run(L,
    "assert(math.abs(panel.Width - 93.5) < .001, 'unmatched state returns towards the live binding')\n"
    "base.Width = 88; require('orca.core').advanceFrame(); require('orca.core').runAllPrograms(panel)\n");
  tick(L, clock, 500);
  run(L,
    "assert(panel.Width == 88 and base.Width == 88, 'exit restores the latest, not the captured, base')\n"
    "panel.ReducedMotion = true; panel.ActiveState = 'Open'; evaluateState(panel)\n"
    "open:removeFromParent()\n"
    "assert(panel.Width == 88 and label.Text == 'base', 'removing the selected state restores live bases')\n"
    "group:addChild(open)\n"
    "assert(panel.Width == 110, 'adding the matching state re-evaluates an idle controller')\n"
    "group:removeFromParent()\n");
  tick(L, clock, 0);
  run(L,
    "assert(panel.Width == 88, 'removing a managed group restores its base')\n"
    "panel.StateManager = manager; require('orca.core').flushQueue()\n"
    "base.Width = 99; require('orca.core').advanceFrame(); require('orca.core').runAllPrograms(panel)\n"
    "assert(panel.Width == 99, 'the binding remains attached after removal')\n"
    "local bad = screen + require('orca.UIKit').Node2D { Width = 10, Height = 20 }\n"
    "local sm = require('orca.core').StateManager()\n"
    "local sg = sm + require('orca.core').StateGroup { ControllerProperty = 'Width' }\n"
    "sg:addChild(require('orca.core').State { Value = 'not-a-number', ['Node.Height'] = 77 })\n"
    "bad:addComponentByName('StateManagerController'); bad.StateManager = sm; bad:send('Object.Start')\n"
    "bad.Width = 0; require('orca.core').flushQueue()\n"
    "assert(bad.Height == 20, 'an invalid numeric state does not match zero')\n"
    "bad:removeFromParent()\n"
    "panel:removeFromParent(); base:removeFromParent()\n"
    "animated, panel, base, manager, group, screen, open, caption, label = nil, nil, nil, nil, nil, nil, nil, nil, nil\n"
    "collectgarbage('collect')\n");
}

int main(void)
{
  struct FakeClock clock = { 10000 };
  test_clock(&clock);
  lua_State *L = luaL_newstate();
  assert(L);
  luaL_openlibs(L);
  run(L, "package.path = 'share/?.lua;share/?/init.lua;' .. package.path");
  luaL_requiref(L, "orca", luaopen_orca, 1); lua_pop(L, 1);
  luaL_requiref(L, "orca.core", luaopen_orca_core, 1); lua_pop(L, 1);
  luaL_requiref(L, "orca.UIKit", luaopen_orca_UIKit, 1); lua_pop(L, 1);
  lua_pushcfunction(L, evaluate_state);
  lua_setglobal(L, "evaluateState");
  test_dispatch_mutation();
  clock.now += 0u - ANIM_GetTime();
  assert(ANIM_GetTime() == 0);
  test_components(L, &clock);
  clock.now += UINT32_MAX - ANIM_GetTime();
  assert(ANIM_GetTime() == UINT32_MAX);
  test_components(L, &clock); // production player primes at max, then crosses the wrap
  test_pivot(L, &clock);
  test_managed_states(L, &clock);
  lua_close(L);
  ANIM_SetClock(NULL, NULL);
  puts("PASS: animation clock, concurrent components, managed state transitions and dispatch lifetimes");
  return 0;
}
