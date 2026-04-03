#include <plugins/UIKit/UIKit.h>
#include <include/api.h>

#define DRAG_SESSION "__DRAG_SESSION__"
#define DRAG_THRESHOLD 4

int
f_beginDraggingSession(lua_State *L)
{
  lua_pushvalue(L, 1);
  lua_setfield(L, LUA_REGISTRYINDEX, DRAG_SESSION);
  return 0;
}

// Maps raw platform mouse event IDs to typed Input message IDs.
static uint32_t
map_mouse_event_id(uint32_t event_id)
{
  switch (event_id) {
    case kEventLeftMouseUp:      return ID_Input_LeftMouseUp;
    case kEventRightMouseUp:     return ID_Input_RightMouseUp;
    case kEventOtherMouseUp:     return ID_Input_OtherMouseUp;
    case kEventLeftMouseDown:    return ID_Input_LeftMouseDown;
    case kEventRightMouseDown:   return ID_Input_RightMouseDown;
    case kEventOtherMouseDown:   return ID_Input_OtherMouseDown;
    case kEventLeftMouseDragged: return ID_Input_LeftMouseDragged;
    case kEventRightMouseDragged:return ID_Input_RightMouseDragged;
    case kEventOtherMouseDragged:return ID_Input_OtherMouseDragged;
    case kEventMouseMoved:       return ID_Input_MouseMoved;
    case kEventScrollWheel:      return ID_Input_ScrollWheel;
    default:                     return event_id;
  }
}

// Maps raw platform key event IDs to typed Input message IDs.
static uint32_t
map_key_event_id(uint32_t event_id)
{
  switch (event_id) {
    case kEventKeyDown: return ID_Input_KeyDown;
    case kEventKeyUp:   return ID_Input_KeyUp;
    case kEventChar:    return ID_Input_Char;
    default:            return event_id;
  }
}

// Pushes a MouseMessageMsgArgs to Lua, transforming Position into obj's local space.
static void
push_mouse_args(lua_State* L, lpObject_t obj, lParam_t lParam)
{
  struct MouseMessageMsgArgs args = *(struct MouseMessageMsgArgs const*)lParam;
#ifdef MOUSE_EVENTS_USE_LOCAL_SPACE
  if (GetNode2D(obj)) {
    struct mat4 inv = MAT4_Inverse(&GetNode2D(obj)->Matrix);
    struct vec3 pt = MAT4_MultiplyVector3D(&inv, &(struct vec3){args.Position.x, args.Position.y, 0});
    args.Position = (struct vec2){pt.x, pt.y};
  }
#endif
  luaX_pushMouseMessageMsgArgs(L, &args);
}

// Pushes a KeyMessageMsgArgs to Lua.
static void
push_key_args(lua_State* L, lpObject_t obj, lParam_t lParam)
{
  (void)obj;
  luaX_pushKeyMessageMsgArgs(L, (struct KeyMessageMsgArgs const*)lParam);
}

// Pushes a single string (timer callback name) to Lua.
static void
push_string_arg(lua_State* L, lpObject_t obj, lParam_t lParam)
{
  (void)obj;
  lua_pushstring(L, (const char*)lParam);
}

// Unified event dispatch. Walks the parent chain from start, looking for either
// a Lua callback registered under luaId or a native C handler registered under
// nativeId. push_args (optional) pushes one extra Lua argument from lParam.
// NOTE: lParam may point to a stack-allocated MsgArgs struct. Native handlers
// (called via OBJ_SendMessageW) must not store this pointer beyond the call.
static LRESULT
UIKit_send_message(lua_State* L,
                   lpObject_t sender, lpObject_t start,
                   uint32_t luaId, uint32_t nativeId,
                   lParam_t lParam,
                   void (*push_args)(lua_State*, lpObject_t, lParam_t))
{
  for (lpObject_t obj = start; obj; obj = OBJ_GetParent(obj)) {
    lpcString_t szCallback = OBJ_FindCallbackForID(obj, luaId);
    if (szCallback) {
      luaX_import(L, "orca", "async");
      if (luaX_pushObject(L, obj), lua_isnil(L, -1)) {
        Con_Warning("Object has no Lua representation: %p", obj);
        lua_pop(L, 2);
        continue;
      }
      lua_getfield(L, -1, szCallback);
      lua_insert(L, -2); // Move callback before self
      luaX_pushObject(L, sender);
      int nargs = 3; // async(callback, self, sender)
      if (push_args) {
        push_args(L, obj, lParam);
        nargs++;
      }
      if (lua_pcall(L, nargs, 0, 0) != LUA_OK) {
        Con_Error("%s(): %s", szCallback, lua_tostring(L, -1));
        lua_pop(L, 1);
      }
      return TRUE;
    }
    if (OBJ_SendMessageW(obj, nativeId, 0, lParam))
      return TRUE;
  }
  return FALSE;
}

static void
process_dragndrop(lua_State *L, struct WI_Message *e, lpObject_t sender)
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
        luaX_parsefield(bool_t, active, -1, lua_toboolean);
        luaX_parsefield(lpObject_t , view, -1, luaX_checkObject);
        if (active) {
          if (GetNode(view)) {
            GetNode(view)->Visible = FALSE;
          }
          e->message = ID_Input_DragDrop;
        }
      }
      lua_pop(L, 1);
      break;
    case kEventLeftMouseDragged:
      if (lua_getfield(L, LUA_REGISTRYINDEX, DRAG_SESSION) == LUA_TTABLE) {
        luaX_parsefield(bool_t, active, -1, lua_toboolean);
        luaX_parsefield(uint32_t, startloc, -1, (uint32_t)luaL_optinteger, -1);
        luaX_parsefield(lpObject_t , view, -1, luaX_checkObject);
        uint16_t sx = LOWORD(startloc), sy = HIWORD(startloc);
        if (startloc == -1) {
          lua_pushinteger(L, e->wParam);
          lua_setfield(L, -2, "startloc");
        } else if (abs(sx-e->x)>DRAG_THRESHOLD||abs(sy-e->y)>DRAG_THRESHOLD) {
          lua_pushboolean(L, TRUE);
          lua_setfield(L, -2, "active");
          active = TRUE;
        }
        if (active) {
          e->message = ID_Input_DragEnter;
          if (view && GetNode(view)) {
            GetNode(view)->Visible = TRUE;
          }
          if (view && GetNode2D(view)) {
            GetNode2D(view)->LayoutTransform.translation = (vec2_t){e->x,e->y};
          }
        }
      }
      lua_pop(L, 1);
      break;
  }
}

lpcString_t WI_KeynumToString(uint32_t keynum);

static bool_t is_printable(int ch) {
  return ch >= 0x20 && ch <= 0x7E;
}

LRESULT
UI_HandleMouseEvent(lua_State* L, lpObject_t root, struct WI_Message* e)
{
  uint16_t x = e->x, y = e->y;
  lpObject_t sender = NULL;
  while (OBJ_GetNext(root)) {
    root = OBJ_GetNext(root);
  }
  // If the focused object has the OF_NOACTIVATE flag, we need to check
  // if the mouse event is targeting it or any of its parents before falling back
  // to hit testing from the root. This allows for interactive elements that
  // don't take focus (e.g. scrollbars) to still receive mouse events without
  // disrupting keyboard focus.
  lpObject_t focused = core_GetFocus();
  if (focused && OBJ_GetFlags(focused)&OF_NOACTIVATE) {
    if ((sender = (lpObject_t)_SendMessage(focused, Node, HitTest, x, y)))
      goto handle;
    if (e->message == kEventLeftMouseDown) {
      for (lpObject_t mod = focused, p = OBJ_GetParent(focused);
           mod && (OBJ_GetFlags(mod)&OF_NOACTIVATE);
           mod = p, p = p?OBJ_GetParent(p):NULL)
      {
        if ((sender = (lpObject_t)_SendMessage(mod, Node, HitTest, x, y)))
          goto handle;
        OBJ_RemoveFromParent(L, mod);
      }
      return TRUE;
    }
  }
  
  if (!(sender = (lpObject_t)_SendMessage(root, Node, HitTest, x, y)))
    return FALSE;
  
handle:
  process_dragndrop(L, e, sender);

  struct MouseMessageMsgArgs args = {
    .Position = {(float)x, (float)y},
    .Delta    = {(float)e->dx, (float)e->dy},
    .ModifierKeys = 0,
    .Source   = sender,
  };

  bool_t success = UIKit_send_message(L, sender, sender,
                                       e->message, map_mouse_event_id(e->message),
                                       &args, push_mouse_args);

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

bool_t
UI_HandleKeyEvent(lua_State *L, struct WI_Message* e)
{
  lpObject_t focused = core_GetFocus();
  if (!focused)
    return FALSE;
  
  // Dispatch a synthetic Char event for printable key presses before the KeyDown.
  if (e->message == kEventKeyDown && is_printable(e->wParam & 0xff)) {
    char ch = e->wParam & 0xff;
    if (e->wParam & WI_MOD_SHIFT) {
      if (ch >= '0' && ch <= '9') {
        lpcString_t sym = ")!@#$%^&*(";
        ch = sym[ch - '0'];
      } else {
        ch = toupper(ch);
      }
    }
    struct KeyMessageMsgArgs char_args = {
      .KeyCode      = (uint32_t)(unsigned char)ch,
      .Key          = WI_KeynumToString(e->wParam & 0xffff),
      .ModifierKeys = e->wParam & (WI_MOD_SHIFT | WI_MOD_CTRL | WI_MOD_ALT | WI_MOD_CMD),
      .Source       = focused,
    };
    UIKit_send_message(L, focused, focused,
                       kEventChar, ID_Input_Char,
                       &char_args, push_key_args);
  }

  struct KeyMessageMsgArgs args = {
    .KeyCode      = e->keyCode,
    .Key          = WI_KeynumToString(e->keyCode),
    .ModifierKeys = e->wParam & (WI_MOD_SHIFT | WI_MOD_CTRL | WI_MOD_ALT | WI_MOD_CMD),
    .Source       = focused,
  };
  return UIKit_send_message(L, focused, focused,
                             e->message, map_key_event_id(e->message),
                             &args, push_key_args);
}

bool_t
CORE_HandleObjectMessage(lua_State *L, struct WI_Message* msg)
{
  void (*push_args)(lua_State*, lpObject_t, lParam_t) = NULL;
  if (msg->message == ID_Object_Timer && msg->lParam)
    push_args = push_string_arg;
  if (UIKit_send_message(L, msg->target, msg->target,
                          msg->message, msg->message, msg->lParam, push_args))
    return TRUE;
  switch (msg->message) {
    case 0x71bab7e1: // ID_Node_ViewDidLoad
      return TRUE;
  }
  return FALSE;
}

LRESULT ui_handle_event(lua_State *L, struct WI_Message* msg) {
  int tmp;
  switch (msg->message) {
    case kEventWindowPaint:
    case kEventWindowResized:
      if (CORE_HandleObjectMessage(L, &(struct WI_Message) {
        .target = msg->target,
        .message = msg->message = kEventWindowPaint ? ID_window_WindowPaint : ID_window_WindowResized,
        .lParam = &(struct WindowPaintMsgArgs) {
          .WindowWidth = LOWORD(msg->wParam),
          .WindowHeight = HIWORD(msg->wParam),
        }
      })) {
        return TRUE;
      } else {
        return FALSE;
      }
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
      return UI_HandleMouseEvent(L, msg->target, msg);
    case kEventKeyDown:
    case kEventKeyUp:
      return UI_HandleKeyEvent(L, msg);
    case kEventResumeCoroutine:
      switch (lua_resume(msg->target, L, LOWORD(msg->wParam), &tmp)) {
        case LUA_OK:
          WI_PostMessageW(msg->target, kEventStopCoroutine, msg->wParam, NULL);
          break;
        case LUA_YIELD:
          WI_PostMessageW(msg->target, kEventResumeCoroutine, MAKEDWORD(0, HIWORD(msg->wParam)), NULL);
          break;
        default:
          WI_PostMessageW(msg->target, kEventStopCoroutine, msg->wParam, NULL);
          if (!lua_isnil(msg->target, -1)) {
            lpcString_t err = lua_tostring(msg->target, -1);
            if (err) fprintf(stderr, "co.resume(): %s\n", err);
          }
          break;
      }
      lua_pop(L, 1);
      return FALSE;
    case kEventStopCoroutine:
      luaL_unref(L, LUA_REGISTRYINDEX, HIWORD(msg->wParam));
      WI_RemoveFromQueue(msg->target);
      WI_PostMessageW(NULL, kEventWindowPaint, WI_GetSize(NULL), 0);
      return FALSE;
    default:
      return CORE_HandleObjectMessage(L, msg);
  }
  return FALSE;
}
