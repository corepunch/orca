#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <include/orca.h>
#include <core/core.h>
#include <UIKit/UIKit.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

extern int luaopen_orca(lua_State *L);
extern int luaopen_orca_core(lua_State *L);
extern int luaopen_orca_UIKit(lua_State *L);

static int measure_count;

static LRESULT
MeasureProbe_Proc(struct Object *object, void *component, uint32_t message,
                  wParam_t wp, lParam_t lp)
{
  if (message == ID_Node2D_MeasureOverride) measure_count++;
  return FALSE;
}

static struct ClassDesc probe_class = {
  .ClassName = "MeasureProbe", .DefaultName = "MeasureProbe",
  .ClassID = 0xabcdf171, .ClassSize = sizeof(int),
  .ObjProc = MeasureProbe_Proc, .ParentClasses = { ID_Node2D, 0 },
};

static void
layout(struct Object *screen)
{
  _SendMessage(screen, Screen, UpdateLayout, .Width = GetScreen(screen)->Width,
               .Height = GetScreen(screen)->Height);
}

static void
touch_scroll(struct Object *list, enum ScrollPhase phase, int dy, uint32_t time)
{
  assert(_SendMessage(list, Node, ScrollWheel, .scrollPhase = phase, .deltaY = dy, .timestamp = time));
}

static void
animate_scroll(struct Object *list, int milliseconds)
{
  GetNode2D(list)->_scrollAnimTime = axGetMilliseconds() - milliseconds;
  OBJ_SendMessageW(list, ID_Object_Animate, 0, NULL);
}

static void
test_touch_physics(struct Object *screen, struct Object *list)
{
  struct Node2D *view = GetNode2D(list);
  assert(view->InertiaScrolling && view->RubberBandScrolling);
  struct vec2 offset = { 0, -100 };
  PROP_SetValue(OBJ_FindLongProperty(list, ID_Node2D_ContentOffset), &offset);
  touch_scroll(list, kScrollPhaseBegin, 0, 1000);
  touch_scroll(list, kScrollPhaseUpdate, -40, 1020);
  touch_scroll(list, kScrollPhaseUpdate, -40, 1040);
  touch_scroll(list, kScrollPhaseEnd, 0, 1045);
  assert(view->_scrollAnimating && !view->_scrollTracking);
  float released = view->ContentOffset.y;
  animate_scroll(list, 16);
  assert(view->ContentOffset.y < released); // release continues in drag direction
  // A new finger stops momentum without moving the content.
  float interrupted = view->ContentOffset.y;
  touch_scroll(list, kScrollPhaseBegin, 0, 1100);
  animate_scroll(list, 16);
  assert(view->ContentOffset.y == interrupted && !view->_scrollAnimating);
  touch_scroll(list, kScrollPhaseCancel, 0, 1110);

  // Slow overdrag resists both edges and cancellation springs back without fling.
  offset.y = 0;
  PROP_SetValue(OBJ_FindLongProperty(list, ID_Node2D_ContentOffset), &offset);
  touch_scroll(list, kScrollPhaseBegin, 0, 2000);
  touch_scroll(list, kScrollPhaseUpdate, 100, 2020);
  assert(view->ContentOffset.y > 0 && view->ContentOffset.y < 100);
  touch_scroll(list, kScrollPhaseCancel, 0, 2021);
  for (int i = 0; i < 300 && view->_scrollAnimating; i++) animate_scroll(list, 16);
  assert(!view->_scrollAnimating && view->ContentOffset.y == 0);
  float minimum = view->_node->Size.Axis[1].Actual - view->_node->Size.Axis[1].Scroll;
  offset.y = minimum;
  PROP_SetValue(OBJ_FindLongProperty(list, ID_Node2D_ContentOffset), &offset);
  touch_scroll(list, kScrollPhaseBegin, 0, 3000);
  touch_scroll(list, kScrollPhaseUpdate, -100, 3020);
  assert(view->ContentOffset.y < minimum && view->ContentOffset.y > minimum - 100);
  touch_scroll(list, kScrollPhaseEnd, 0, 3200); // held before release: no fling
  assert(view->_scrollVelocity.y == 0);
  for (int i = 0; i < 300 && view->_scrollAnimating; i++) animate_scroll(list, 16);
  assert(!view->_scrollAnimating && view->ContentOffset.y == minimum);

  // Opt-outs keep hard bounds and release without momentum.
  view->RubberBandScrolling = view->InertiaScrolling = FALSE;
  touch_scroll(list, kScrollPhaseBegin, 0, 4000);
  touch_scroll(list, kScrollPhaseUpdate, -100, 4020);
  touch_scroll(list, kScrollPhaseEnd, 0, 4021);
  assert(view->ContentOffset.y == minimum && !view->_scrollAnimating);
  view->RubberBandScrolling = view->InertiaScrolling = TRUE;
  // Animation must not remeasure layout, including spring frames.
  OBJ_ClearDirtyFlags(screen);
  measure_count = 0;
  offset.y = -100;
  PROP_SetValue(OBJ_FindLongProperty(list, ID_Node2D_ContentOffset), &offset);
  touch_scroll(list, kScrollPhaseBegin, 0, UINT32_MAX - 10);
  touch_scroll(list, kScrollPhaseUpdate, -20, 9); // sample clock wraps
  touch_scroll(list, kScrollPhaseEnd, 0, 10);
  animate_scroll(list, 16);
  layout(screen);
  assert(measure_count == 0 && view->ContentOffset.y < -120);
  // Programmatic offsets interrupt motion instead of being overwritten next tick.
  offset.y = -80;
  PROP_SetValue(OBJ_FindLongProperty(list, ID_Node2D_ContentOffset), &offset);
  assert(!view->_scrollAnimating && !view->_scrollTracking);
  animate_scroll(list, 16);
  assert(view->ContentOffset.y == -80);
  touch_scroll(list, kScrollPhaseBegin, 0, 5000);
  touch_scroll(list, kScrollPhaseUpdate, -20, 5020);
  touch_scroll(list, kScrollPhaseEnd, 0, 5021);
  // Hiding a retained ancestor stops child motion, rather than scrolling offscreen.
  struct Object *page = OBJ_Create(ID_Node2D);
  OBJ_AddChild(screen, page);
  OBJ_RemoveFromParent(list);
  OBJ_AddChild(page, list);
  GetNode(page)->Visible = FALSE;
  animate_scroll(list, 16);
  assert(!view->_scrollAnimating);
  OBJ_RemoveFromParent(list);
  OBJ_AddChild(screen, list);
  OBJ_RemoveFromParent(page);
  OBJ_ReleaseRef(page);
  touch_scroll(list, kScrollPhaseBegin, 0, 5100);
  view->Overflow.y = kOverflowHidden;
  float before = view->ContentOffset.y;
  assert(!_SendMessage(list, Node, ScrollWheel, .scrollPhase = kScrollPhaseUpdate, .deltaY = -20, .timestamp = 5120));
  assert(!view->_scrollTracking && view->ContentOffset.y == before);
  view->Overflow.y = kOverflowScroll;
  touch_scroll(list, kScrollPhaseBegin, 0, 5200);
  touch_scroll(list, kScrollPhaseUpdate, -20, 5220);
  touch_scroll(list, kScrollPhaseEnd, 0, 5221);
  // Shrinking content while decelerating converges to the new boundary.
  GetNode(list)->Size.Axis[1].Scroll = GetNode(list)->Size.Axis[1].Actual;
  for (int i = 0; i < 300 && view->_scrollAnimating; i++) animate_scroll(list, 33);
  assert(!view->_scrollAnimating && fabsf(view->ContentOffset.y) < 0.25f);
}

int main(void)
{
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  assert(luaL_dostring(L, "package.path = 'share/?.lua;share/?/init.lua;' .. package.path") == LUA_OK);
  luaL_requiref(L, "orca", luaopen_orca, 1);
  lua_pop(L, 1);
  luaL_requiref(L, "orca.core", luaopen_orca_core, 1);
  lua_pop(L, 1);
  luaL_requiref(L, "orca.UIKit", luaopen_orca_UIKit, 1);
  lua_pop(L, 1);
  OBJ_RegisterClass(&probe_class);

  struct Object *screen = OBJ_Create(ID_Screen);
  struct Object *list = OBJ_Create(ID_StackView);
  struct Object *row = OBJ_Create(probe_class.ClassID);
  GetScreen(screen)->Width = 390;
  GetScreen(screen)->Height = 844;
  GetScreen(screen)->ResizeMode = kResizeModeNoResize;
  GetNode2D(list)->Overflow.y = kOverflowScroll;
  GetNode(row)->Size.Axis[1].Requested = 1200;
  OBJ_AddChild(screen, list);
  OBJ_AddChild(list, row);
  layout(screen);
  assert(measure_count > 0);
  OBJ_ClearDirtyFlags(screen);

  measure_count = 0;
  _SendMessage(list, Node, ScrollWheel, .deltaY = -20);
  layout(screen);
  assert(measure_count == 0);
  assert(GetNode2D(list)->ContentOffset.y == -20);
  // Layout is reused, but the scroll offset must reach descendant transforms.
  assert(GetNode2D(row)->Matrix.v[13] == -20);

  float height = 1400;
  PROP_SetValue(OBJ_FindLongProperty(row, ID_Node_Height), &height);
  layout(screen);
  assert(measure_count > 0);
  assert(GetNode(list)->Size.Axis[1].Scroll == 1400);
  OBJ_ClearDirtyFlags(screen);

  measure_count = 0;
  struct vec2 offset = { 0, -40 };
  PROP_SetValue(OBJ_FindLongProperty(list, ID_Node2D_ContentOffset), &offset);
  layout(screen);
  assert(measure_count == 0);
  assert(GetNode2D(row)->Matrix.v[13] == -40);
  OBJ_ClearDirtyFlags(screen);

  _SendMessage(list, Node2D, SetScrollTop, .Value = 1000);
  layout(screen);
  assert(measure_count == 0);
  GetScreen(screen)->Width = 420;
  layout(screen);
  assert(measure_count > 0);

  test_touch_physics(screen, list);

  OBJ_ReleaseRef(row);
  OBJ_ReleaseRef(list);
  OBJ_ReleaseRef(screen);
  lua_close(L);
  puts("PASS: scroll layout reuse, touch inertia, rubber-band return, interruption, opt-outs and resized content");
  return 0;
}
