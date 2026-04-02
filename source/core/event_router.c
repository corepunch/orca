#include "core_local.h"

// Registry key used to store the active drag-and-drop session table.
#define DRAG_SESSION    "__DRAG_SESSION__"
#define DRAG_THRESHOLD  4

// ---------------------------------------------------------------------------
// Pluggable callbacks — set by UIKit (or any plugin that provides spatial
// layout / hit-testing).  Core drives all routing; UIKit only supplies the
// two UIKit-specific operations below.
// ---------------------------------------------------------------------------

static CORE_HitTestProc   s_hit_test_proc   = NULL;
static CORE_DragGhostProc s_drag_ghost_proc = NULL;

ORCA_API void
CORE_RegisterHitTestProc(CORE_HitTestProc proc)
{
  s_hit_test_proc = proc;
}

ORCA_API void
CORE_RegisterDragGhostProc(CORE_DragGhostProc proc)
{
  s_drag_ghost_proc = proc;
}

// ---------------------------------------------------------------------------
// Helpers shared by CORE_HandleMouseEvent
// ---------------------------------------------------------------------------

// Push the mouse-event arguments that a Lua callback expects onto the stack.
// Returns the number of extra values pushed (added on top of the base 3:
// async, self, sender).
static int
lua_pushmousevent(lua_State* L, lpObject_t obj, struct WI_Message* e)
{
  struct vec3 point = { LOWORD(e->wParam), HIWORD(e->wParam) };
  switch (e->message) {
    case kEventMouseMoved:
    case kEventLeftMouseDown:
    case kEventRightMouseDown:
    case kEventOtherMouseDown:
    case kEventLeftMouseUp:
    case kEventRightMouseUp:
    case kEventOtherMouseUp:
    case kEventScrollWheel:
    case kEventLeftMouseDragged:
    case kEventRightMouseDragged:
    case kEventOtherMouseDragged:
      lua_pushnumber(L, point.x);
      lua_pushnumber(L, point.y);
      lua_pushnumber(L, LOWORD((intptr_t)e->lParam));
      lua_pushnumber(L, HIWORD((intptr_t)e->lParam));
      return 4;
    case kEventDragDrop:
    case kEventDragEnter:
      lua_getfield(L, LUA_REGISTRYINDEX, DRAG_SESSION);
      lua_pushnumber(L, point.x);
      lua_pushnumber(L, point.y);
      return 3;
    default:
      return 0;
  }
}

// Map a platform kEvent* to the corresponding ID_Input_* and dispatch it to
// the native (C) message handler of obj.  Returns the handler's result.
static LRESULT
send_mouse_message(lua_State* L, lpObject_t obj, struct WI_Message* e)
{
  uint32_t msg = (uint32_t)-1;
  switch (e->message) {
    case kEventLeftMouseUp:        msg = ID_Input_LeftMouseUp;        break;
    case kEventRightMouseUp:       msg = ID_Input_RightMouseUp;       break;
    case kEventOtherMouseUp:       msg = ID_Input_OtherMouseUp;       break;
    case kEventLeftMouseDown:      msg = ID_Input_LeftMouseDown;      break;
    case kEventRightMouseDown:     msg = ID_Input_RightMouseDown;     break;
    case kEventOtherMouseDown:     msg = ID_Input_OtherMouseDown;     break;
    case kEventLeftMouseDragged:   msg = ID_Input_LeftMouseDragged;   break;
    case kEventRightMouseDragged:  msg = ID_Input_RightMouseDragged;  break;
    case kEventOtherMouseDragged:  msg = ID_Input_OtherMouseDragged;  break;
    case kEventMouseMoved:         msg = ID_Input_MouseMoved;         break;
    case kEventScrollWheel:        msg = ID_Input_ScrollWheel;        break;
    default:
      return FALSE;
  }
  return OBJ_SendMessageW(obj, msg, 0, e);
}

// Handle focus changes and drag-and-drop session state for mouse events.
// May transform e->message (e.g. kEventLeftMouseDragged -> ID_Input_DragEnter)
// when an active drag session is detected.
static void
process_dragndrop(lua_State* L, struct WI_Message* e, lpObject_t sender)
{
  switch (e->message) {
    case kEventLeftMouseDown:
    case kEventRightMouseDown:
    case kEventOtherMouseDown:
      OBJ_SetFocus(sender);
      break;

    case kEventLeftMouseUp:
      OBJ_SetFocus(sender);
      if (lua_getfield(L, LUA_REGISTRYINDEX, DRAG_SESSION) == LUA_TTABLE) {
        luaX_parsefield(bool_t,   active, -1, lua_toboolean);
        luaX_parsefield(lpObject_t, view, -1, luaX_checkObject);
        if (active) {
          if (s_drag_ghost_proc)
            s_drag_ghost_proc(L, view, FALSE, 0, 0);
          e->message = ID_Input_DragDrop;
        }
      }
      lua_pop(L, 1);
      break;

    case kEventLeftMouseDragged:
      if (lua_getfield(L, LUA_REGISTRYINDEX, DRAG_SESSION) == LUA_TTABLE) {
        luaX_parsefield(bool_t,   active,   -1, lua_toboolean);
        luaX_parsefield(uint32_t, startloc, -1, (uint32_t)luaL_optinteger, -1);
        luaX_parsefield(lpObject_t, view,   -1, luaX_checkObject);
        uint16_t sx = LOWORD(startloc), sy = HIWORD(startloc);
        if (startloc == (uint32_t)-1) {
          lua_pushinteger(L, e->wParam);
          lua_setfield(L, -2, "startloc");
        } else if (abs(sx - e->x) > DRAG_THRESHOLD || abs(sy - e->y) > DRAG_THRESHOLD) {
          lua_pushboolean(L, TRUE);
          lua_setfield(L, -2, "active");
          active = TRUE;
        }
        if (active) {
          e->message = ID_Input_DragEnter;
          if (s_drag_ghost_proc)
            s_drag_ghost_proc(L, view, TRUE, e->x, e->y);
        }
      }
      lua_pop(L, 1);
      break;
  }
}

// ---------------------------------------------------------------------------
// CORE_HandleMouseEvent
// ---------------------------------------------------------------------------

bool_t
CORE_HandleMouseEvent(lua_State* L, lpObject_t root, struct WI_Message* e)
{
  if (!s_hit_test_proc)
    return FALSE;

  uint16_t x = LOWORD(e->wParam), y = HIWORD(e->wParam);
  lpObject_t sender = NULL;
  bool_t success = FALSE;

  while (OBJ_GetNext(root))
    root = OBJ_GetNext(root);

  // If the focused object carries OF_NOACTIVATE (e.g. a floating scrollbar),
  // let it intercept events before falling back to hit-testing from the root.
  lpObject_t focused = core_GetFocus();
  if (focused && (OBJ_GetFlags(focused) & OF_NOACTIVATE)) {
    if ((sender = s_hit_test_proc(focused, x, y)))
      goto handle;
    if (e->message == kEventLeftMouseDown) {
      for (lpObject_t mod = focused, p = OBJ_GetParent(focused);
           mod && (OBJ_GetFlags(mod) & OF_NOACTIVATE);
           mod = p, p = p ? OBJ_GetParent(p) : NULL)
      {
        if ((sender = s_hit_test_proc(mod, x, y)))
          goto handle;
        OBJ_RemoveFromParent(L, mod);
      }
      return TRUE;
    }
  }

  // Normal hit-test from the root of the scene.
  if (!(sender = s_hit_test_proc(root, x, y)))
    return FALSE;

handle:
  // Update focus and drag-session state (may mutate e->message).
  process_dragndrop(L, e, sender);

  // Bubble the event up the parent chain until a handler claims it.
  for (lpObject_t obj = sender; !success && obj; obj = OBJ_GetParent(obj)) {
    lpcString_t szCallback = OBJ_FindCallbackForID(obj, e->message);
    if (szCallback) {
      luaX_import(L, "orca", "async");
      if (luaX_pushObject(L, obj), lua_isnil(L, -1)) {
        Con_Warning("Object has no Lua representation: %p", obj);
        lua_pop(L, 2);
        continue;
      }
      lua_getfield(L, -1, szCallback);
      lua_insert(L, -2); // move callback before obj
      luaX_pushObject(L, sender);
      if (lua_pcall(L, lua_pushmousevent(L, obj, e) + 3, 0, 0) != LUA_OK) {
        Con_Error("%s(): %s", szCallback, lua_tostring(L, -1));
        lua_pop(L, 1);
      }
      success = TRUE;
    } else if (send_mouse_message(L, obj, e)) {
      success = TRUE;
    }
  }

  // Post-dispatch: hover update and drag-session cleanup.
  void CORE_UpdateHover(void);
  switch (e->message) {
    case kEventMouseMoved:
      CORE_UpdateHover();
      break;
    case kEventDragDrop:
    case kEventLeftMouseUp:
      lua_pushnil(L);
      lua_setfield(L, LUA_REGISTRYINDEX, DRAG_SESSION);
      break;
  }
  return success || e->message == ID_Input_DragEnter;
}

// ---------------------------------------------------------------------------
// CORE_HandleKeyEvent  (was in message.c)
// ---------------------------------------------------------------------------

lpcString_t WI_KeynumToString(uint32_t keynum);

static bool_t is_printable(int ch) {
  return ch >= 0x20 && ch <= 0x7E;
}

bool_t
CORE_HandleKeyEvent(lua_State* L, struct WI_Message* e)
{
  if (!core_GetFocus())
    return FALSE;

  shortStr_t comp = {0};
  if (e->wParam & WI_MOD_CTRL)  strcat(comp, "ctrl+");
  if (e->wParam & WI_MOD_ALT)   strcat(comp, "alt+");
  if (e->wParam & WI_MOD_SHIFT) strcat(comp, "shift+");
  if (e->wParam & WI_MOD_CMD)   strcat(comp, "cmd+");
  strcat(comp, WI_KeynumToString(e->wParam));

  if (e->message == kEventKeyDown) {
    lua_getfield(L, LUA_REGISTRYINDEX, CORE_KEMAP);
    lua_getfield(L, -1, comp);
    if (lua_isstring(L, -1)) {
      LPSTR buf = strdup(luaL_checkstring(L, -1));
      for (lpcString_t tok = strtok(buf, ";"); tok; tok = strtok(NULL, ";")) {
        int f_executeCommand(lua_State* L);
        lua_pushcfunction(L, f_executeCommand);
        lua_pushstring(L, tok);
        if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
          Con_Error("%s", luaL_checkstring(L, -1));
          lua_pop(L, 1);
          continue;
        }
        if (lua_toboolean(L, -1)) {
          free(buf);
          lua_pop(L, 3);
          return TRUE;
        } else {
          lua_pop(L, 1);
        }
      }
      free(buf);
    }
    lua_pop(L, 2);
  }

  if (e->message == kEventKeyDown && is_printable(e->wParam & 0xff)) {
    struct WI_Message tmp = *e;
    tmp.message = kEventChar;
    CORE_HandleKeyEvent(L, &tmp);
  }

  for (lpObject_t obj = core_GetFocus(); obj; obj = OBJ_GetParent(obj)) {
    lpcString_t szCallback = OBJ_FindCallbackForID(obj, e->message);
    if (szCallback) {
      luaX_import(L, "orca", "async");
      luaX_pushObject(L, obj);
      lua_getfield(L, -1, "handleEvent");
      lua_insert(L, -2); // move handleEvent before obj
      lua_pushstring(L, szCallback);
      luaX_pushObject(L, core_GetFocus());
      if (e->message == kEventChar) {
        char ch = e->wParam & 0xff;
        if (e->wParam & WI_MOD_SHIFT) {
          if (ch >= '0' && ch <= '9') {
            lpcString_t sym = ")!@#$%^&*(";
            ch = sym[ch - '0'];
          } else {
            ch = toupper(ch);
          }
        }
        lua_pushlstring(L, &ch, 1);
      } else {
#if __linux__
        uint32_t len = 0;
        while (len < sizeof(e->lParam) && ((lpcString_t)&e->lParam)[len])
          len++;
        lua_pushlstring(L, (lpcString_t)&e->lParam, len);
#else
        lua_pushstring(L, WI_KeynumToString(e->wParam));
#endif
      }
      lua_pushstring(L, comp);
      if (lua_pcall(L, 6, 1, 0) != LUA_OK) {
        Con_Error("%s(): %s", szCallback, luaL_checkstring(L, -1));
        lua_pop(L, 1);
      }
      return TRUE;
    }
    if (OBJ_SendMessageW(obj, e->message, 0, e)) {
      return TRUE;
    }
  }
  return FALSE;
}

// ---------------------------------------------------------------------------
// CORE_HandleObjectMessage  (was in message.c)
// ---------------------------------------------------------------------------

bool_t
CORE_HandleObjectMessage(lua_State* L, struct WI_Message* msg)
{
  for (lpObject_t hobj = msg->target; hobj; hobj = OBJ_GetParent(hobj))
  {
    lpcString_t szCallback = OBJ_FindCallbackForID(hobj, msg->message);
    if (szCallback) {
      luaX_import(L, "orca", "async");
      luaX_pushObject(L, hobj);
      assert(!lua_isnil(L, -1));
      lua_getfield(L, -1, szCallback);
      lua_insert(L, -2); // move callback before obj
      luaX_pushObject(L, hobj);
      uint32_t numargs = 3;
      if (msg->message == ID_Object_Timer && msg->lParam) {
        lua_pushstring(L, msg->lParam);
        numargs++;
      }
      if (lua_pcall(L, numargs, 0, 0) != LUA_OK) {
        Con_Error("%s(): %s", szCallback, luaL_checkstring(L, -1));
        lua_pop(L, 1);
      }
      return TRUE;
    }
    if (OBJ_SendMessageW(hobj, msg->message, msg->wParam, msg->lParam))
      return TRUE;
    switch (msg->message) {
      case 0x71bab7e1: // ID_Node_ViewDidLoad
        return TRUE;
    }
  }
  return FALSE;
}

// ---------------------------------------------------------------------------
// CORE_RouteEvent — unified routing entry point
// ---------------------------------------------------------------------------

LRESULT
CORE_RouteEvent(lua_State* L, struct WI_Message* msg)
{
  switch (msg->message) {
    // Mouse events: resolve target via hit-testing.
    case kEventLeftMouseDown:
    case kEventRightMouseDown:
    case kEventOtherMouseDown:
    case kEventLeftMouseUp:
    case kEventRightMouseUp:
    case kEventOtherMouseUp:
    case kEventLeftMouseDragged:
    case kEventRightMouseDragged:
    case kEventOtherMouseDragged:
    case kEventMouseMoved:
    case kEventScrollWheel:
      return CORE_HandleMouseEvent(L, msg->target, msg);

    // Key events: route to the focused object.
    case kEventKeyDown:
    case kEventKeyUp:
      return CORE_HandleKeyEvent(L, msg);

    // Generic messages: route to the explicit target.
    default:
      return CORE_HandleObjectMessage(L, msg);
  }
}
