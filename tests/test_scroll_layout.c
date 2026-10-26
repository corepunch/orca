#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <include/orca.h>
#include <core/core.h>
#include <UIKit/UIKit.h>
#include <assert.h>
#include <stdio.h>

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

  OBJ_ReleaseRef(row);
  OBJ_ReleaseRef(list);
  OBJ_ReleaseRef(screen);
  lua_close(L);
  puts("PASS: scroll-only frames reuse layout, update transforms, and invalidate on content/size changes");
  return 0;
}
