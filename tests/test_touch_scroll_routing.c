#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <include/orca.h>
#include <include/api.h>
#include <core/core.h>
#include <UIKit/UIKit.h>
#include <assert.h>
#include <stdio.h>

extern int luaopen_orca(lua_State *L);
extern int luaopen_orca_core(lua_State *L);
extern int luaopen_orca_UIKit(lua_State *L);
/* Exercise the platform-to-UIKit bridge with real native input messages. */
extern LRESULT UI_HandleMouseEvent(lua_State *L, struct Object *root, struct AXmessage *event);
extern bool_t UI_HandleKeyEvent(lua_State *L, struct AXmessage *event);
static void layout(struct Object *screen);

static void test_focus_visuals(lua_State *L)
{
  assert(luaL_dostring(L,
    "local ui = require 'orca.UIKit'\n"
    "local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = 'NoResize',\n"
    "  StyleSheet = ui.loadObjectFromCssString [[\n"
    "    .command:focus-visible { ring-width: 2; }\n"
    "    .field:focus { opacity: 0.75; }\n"
    "  ]] }\n"
    "local button = screen + ui.Button { class = 'command', Width = 100, Height = 100 }\n"
    "local input = screen + ui.Input { class = 'field', Width = 100, Height = 40, MarginTop = 120 }\n"
    "return screen, button, input") == LUA_OK);
  struct Object *screen = luaX_checkObject(L, -3);
  struct Object *button = luaX_checkObject(L, -2);
  struct Object *input = luaX_checkObject(L, -1);
  layout(screen);
  struct AXmessage down = {
    .message = kEventLeftButtonDown, .wParam = MAKEDWORD(50, 50),
    .pointer = { .flags = AX_POINTER_TOUCH },
  };
  UI_HandleMouseEvent(L, screen, &down);
  assert(OBJ_IsFocused(button));
  assert(GetNode2D(button)->Ring.Width == 0);
  struct AXmessage key = { .message = kEventKeyDown, .keyCode = AX_KEY_TAB };
  UI_HandleKeyEvent(L, &key);
  OBJ_SetFocus(button);
  assert(GetNode2D(button)->Ring.Width == 2);
  /* The same focused button must update when the input method changes. */
  UI_HandleMouseEvent(L, screen, &down);
  assert(OBJ_IsFocused(button));
  assert(GetNode2D(button)->Ring.Width == 0);
  struct AXmessage character = { .message = kEventChar, .lParam = (void *)(uintptr_t)'a' };
  UI_HandleKeyEvent(L, &character);
  assert(GetNode2D(button)->Ring.Width == 0);
  UI_HandleKeyEvent(L, &key);
  OBJ_SetFocus(button);
  assert(GetNode2D(button)->Ring.Width == 2);
  down.pointer.flags = 0;
  UI_HandleMouseEvent(L, screen, &down);
  assert(OBJ_IsFocused(button));
  assert(GetNode2D(button)->Ring.Width == 0);
  /* Touch focus still enables text entry and ordinary :focus styles. */
  down.wParam = MAKEDWORD(50, 140);
  down.pointer.flags = AX_POINTER_TOUCH;
  UI_HandleMouseEvent(L, screen, &down);
  assert(OBJ_IsFocused(input));
  assert(GetNode(input)->Opacity == 0.75f);
  UI_HandleKeyEvent(L, &character);
  assert(!strcmp(GetTextRun(input)->Text, "a"));
  OBJ_SetFocus(NULL);
  lua_pop(L, 3);
}

static void layout(struct Object *screen)
{
  _SendMessage(screen, Screen, UpdateLayout, .Width = 200, .Height = 200);
}

static struct Object *list(struct Object *screen, int height)
{
  struct Object *view = OBJ_Create(ID_StackView), *content = OBJ_Create(ID_Node2D);
  GetNode2D(view)->Overflow.y = kOverflowScroll;
  GetNode(content)->Size.Axis[1].Requested = height;
  OBJ_AddChild(view, content);
  OBJ_AddChild(screen, view);
  OBJ_ReleaseRef(content);
  layout(screen);
  return view;
}

static bool_t touch(lua_State *L, struct Object *screen, uint32_t phase, int delta, uint32_t time)
{
  struct AXmessage event = {
    .message = kEventScrollWheel, .wParam = MAKEDWORD(50, 50),
    .lParam = (void *)(uintptr_t)MAKEDWORD(0, delta),
    .pointer = { .flags = AX_POINTER_TOUCH | phase, .time = time },
  };
  return UI_HandleMouseEvent(L, screen, &event);
}

int main(void)
{
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  assert(luaL_dostring(L, "package.path = 'share/?.lua;share/?/init.lua;' .. package.path") == LUA_OK);
  luaL_requiref(L, "orca", luaopen_orca, 1); lua_pop(L, 1);
  luaL_requiref(L, "orca.core", luaopen_orca_core, 1); lua_pop(L, 1);
  luaL_requiref(L, "orca.UIKit", luaopen_orca_UIKit, 1); lua_pop(L, 1);

  test_focus_visuals(L);

  struct Object *screen = OBJ_Create(ID_Screen);
  GetScreen(screen)->Width = GetScreen(screen)->Height = 200;
  GetScreen(screen)->ResizeMode = kResizeModeNoResize;
  struct Object *first = list(screen, 600);
  assert(touch(L, screen, AX_POINTER_SCROLL_BEGIN, 0, 100));
  /* The point now hits another container; an ongoing drag keeps its owner. */
  struct Object *second = list(screen, 600);
  assert(touch(L, screen, AX_POINTER_SCROLL_UPDATE, -30, 116));
  assert(GetNode2D(first)->ContentOffset.y == -30);
  assert(GetNode2D(second)->ContentOffset.y == 0);
  assert(touch(L, screen, AX_POINTER_SCROLL_END, 0, 130));
  assert(!touch(L, screen, AX_POINTER_SCROLL_UPDATE, -30, 146));
  assert(GetNode2D(first)->ContentOffset.y == -30);

  /* A new touch captures the current hit and cancels the previous owner. */
  assert(touch(L, screen, AX_POINTER_SCROLL_BEGIN, 0, 160));
  assert(touch(L, screen, AX_POINTER_SCROLL_UPDATE, -20, 176));
  assert(GetNode2D(second)->ContentOffset.y == -20);
  assert(touch(L, screen, AX_POINTER_SCROLL_BEGIN, 0, 190));
  assert(touch(L, screen, AX_POINTER_SCROLL_CANCEL, 0, 206));
  assert(!touch(L, screen, AX_POINTER_SCROLL_UPDATE, -20, 220));
  assert(GetNode2D(second)->ContentOffset.y == -20);

  /* Detaching or hiding the captured view must not redirect the same gesture. */
  assert(touch(L, screen, AX_POINTER_SCROLL_BEGIN, 0, 240));
  OBJ_RemoveFromParent(second);
  assert(!touch(L, screen, AX_POINTER_SCROLL_UPDATE, -20, 256));
  assert(GetNode2D(first)->ContentOffset.y == -30);
  OBJ_AddChild(screen, second);
  layout(screen);
  assert(touch(L, screen, AX_POINTER_SCROLL_BEGIN, 0, 270));
  GetNode(second)->Visible = FALSE;
  assert(!touch(L, screen, AX_POINTER_SCROLL_UPDATE, -20, 286));
  GetNode(second)->Visible = TRUE;

  /* A tap stops motion without changing the offset or creating a drag. */
  float offset = GetNode2D(second)->ContentOffset.y;
  assert(touch(L, screen, AX_POINTER_SCROLL_BEGIN, 0, 300));
  assert(touch(L, screen, AX_POINTER_SCROLL_END, 0, 316));
  assert(GetNode2D(second)->ContentOffset.y == offset);

  /* A non-overflowing child passes ownership to its scrolling ancestor. */
  struct Object *shortList = OBJ_GetFirstChild(second);
  GetNode2D(shortList)->Overflow.y = kOverflowScroll;
  layout(screen);
  assert(touch(L, screen, AX_POINTER_SCROLL_BEGIN, 0, 340));
  assert(touch(L, screen, AX_POINTER_SCROLL_UPDATE, -15, 356));
  assert(GetNode2D(shortList)->ContentOffset.y == 0);
  assert(GetNode2D(second)->ContentOffset.y < offset);
  assert(touch(L, screen, AX_POINTER_SCROLL_CANCEL, 0, 372));

  /* Navigating to another screen discards the old screen's capture. */
  assert(touch(L, screen, AX_POINTER_SCROLL_BEGIN, 0, 390));
  struct Object *nextScreen = OBJ_Create(ID_Screen);
  GetScreen(nextScreen)->Width = GetScreen(nextScreen)->Height = 200;
  GetScreen(nextScreen)->ResizeMode = kResizeModeNoResize;
  struct Object *nextList = list(nextScreen, 600);
  assert(!touch(L, nextScreen, AX_POINTER_SCROLL_UPDATE, -20, 406));
  assert(GetNode2D(nextList)->ContentOffset.y == 0);
  OBJ_ReleaseRef(nextList);
  OBJ_ReleaseRef(nextScreen);

  /* Mouse wheels retain their immediate, clamped scroll semantics. */
  struct AXmessage wheel = {
    .message = kEventScrollWheel, .wParam = MAKEDWORD(50, 50),
    .lParam = (void *)(uintptr_t)MAKEDWORD(0, -5000),
  };
  assert(UI_HandleMouseEvent(L, screen, &wheel));
  float minimum = GetNode(second)->Size.Axis[1].Actual - GetNode(second)->Size.Axis[1].Scroll;
  assert(GetNode2D(second)->ContentOffset.y == minimum);

  /* A finger never hovers: touch clears the mouse hover instead of moving it. */
  struct Object *hoverScreen = OBJ_Create(ID_Screen), *target = OBJ_Create(ID_Node2D);
  GetScreen(hoverScreen)->Width = GetScreen(hoverScreen)->Height = 200;
  GetScreen(hoverScreen)->ResizeMode = kResizeModeNoResize;
  OBJ_SetFlags(target, OBJ_GetFlags(target) | OF_HOVERABLE);
  OBJ_AddChild(hoverScreen, target);
  layout(hoverScreen);
  struct AXmessage move = { .message = kEventMouseMoved, .wParam = MAKEDWORD(50, 50) };
  UI_HandleMouseEvent(L, hoverScreen, &move);
  assert(core_GetHover() == target);
  struct AXmessage touchMove = move, touchDown = { .message = kEventLeftButtonDown, .wParam = MAKEDWORD(50, 50) };
  touchMove.pointer.flags = touchDown.pointer.flags = AX_POINTER_TOUCH;
  UI_HandleMouseEvent(L, hoverScreen, &touchMove);
  assert(core_GetHover() == NULL);
  UI_HandleMouseEvent(L, hoverScreen, &move);
  UI_HandleMouseEvent(L, hoverScreen, &touchDown);
  assert(core_GetHover() == NULL);
  OBJ_ReleaseRef(target);
  OBJ_ReleaseRef(hoverScreen);

  OBJ_ReleaseRef(first);
  OBJ_ReleaseRef(second);
  OBJ_ReleaseRef(screen);
  lua_close(L);
  puts("PASS: touch scroll captures its container, ends/cancels safely, preserves mouse wheel behavior, and never hovers");
  return 0;
}
