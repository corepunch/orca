#include <plugins/UIKit/UIKit.h>
#include <include/api.h>

#define DRAG_SESSION "__DRAG_SESSION__"
#define DRAG_THRESHOLD 4

extern lpProperty_t
luaX_getobjectcallback(lua_State* L, lpObject_t object, uint32_t id);

bool_t
CORE_HandleObjectMessage(lua_State *L, struct AXmessage* msg)
{
  for (lpObject_t hobj = msg->target; hobj; hobj = OBJ_GetParent(hobj))
  {
    lpProperty_t handler = luaX_getobjectcallback(L, hobj, msg->message);
    if (handler) {
      luaX_import(L, "orca", "async");
      lua_insert(L, -2);
      luaX_pushObject(L, hobj); // self
      luaX_pushObject(L, msg->target); // sender

      // push event data
      luaL_getmetatable(L, PROP_GetDesc(handler)->TypeString);
      lua_pushlightuserdata(L, msg->lParam);
      lua_call(L, 1, 1);

      // call orca.async
      if (lua_pcall(L, 4, 0, 0) != LUA_OK) {
        Con_Error("Message handler 0x%08x: %s", msg->message, luaL_checkstring(L, -1));
        lua_pop(L, 1);
      }
      return TRUE;
    }
    if (OBJ_SendMessageW(hobj, msg->message, msg->wParam, msg->lParam))
      return TRUE;
  }
  return FALSE;
}

int
f_beginDraggingSession(lua_State *L)
{
  //  luaX_parsefield(bool_t, active, 1, lua_toboolean);
  //  luaX_parsefield(bool_t, item, 1, lua_toboolean);
  lua_pushvalue(L, 1);
  lua_setfield(L, LUA_REGISTRYINDEX, DRAG_SESSION);
  return 0;
}

static bool
convert_mouse_message(struct AXmessage* e, uint32_t* out_msg, Node_MouseMessageMsg_t* out_mouse)
{
  *out_mouse = (Node_MouseMessageMsg_t){
    .x = e->x,
    .y = e->y,
    .deltaX = e->dx,
    .deltaY = e->dy,
    .button = kMouseButtonNone,
    .clickCount = 0,
  };
  switch (e->message) {
    case kEventLeftMouseUp:        *out_msg = ID_Node_LeftMouseUp;        out_mouse->button = kMouseButtonLeft; out_mouse->clickCount = 1; break;
    case kEventRightMouseUp:       *out_msg = ID_Node_RightMouseUp;       out_mouse->button = kMouseButtonRight; out_mouse->clickCount = 1; break;
    case kEventOtherMouseUp:       *out_msg = ID_Node_OtherMouseUp;       out_mouse->button = kMouseButtonMiddle; out_mouse->clickCount = 1; break;
    case kEventLeftMouseDown:      *out_msg = ID_Node_LeftMouseDown;      out_mouse->button = kMouseButtonLeft; break;
    case kEventRightMouseDown:     *out_msg = ID_Node_RightMouseDown;     out_mouse->button = kMouseButtonRight;  break;
    case kEventOtherMouseDown:     *out_msg = ID_Node_OtherMouseDown;     out_mouse->button = kMouseButtonMiddle; break;
    case kEventLeftMouseDragged:   *out_msg = ID_Node_LeftMouseDragged;   out_mouse->button = kMouseButtonLeft; break;
    case kEventRightMouseDragged:  *out_msg = ID_Node_RightMouseDragged;  out_mouse->button = kMouseButtonRight;  break;
    case kEventOtherMouseDragged:  *out_msg = ID_Node_OtherMouseDragged;  out_mouse->button = kMouseButtonMiddle; break;
    case kEventLeftDoubleClick:    *out_msg = ID_Node_LeftDoubleClick;    out_mouse->button = kMouseButtonLeft; out_mouse->clickCount = 2; break;
    case kEventRightDoubleClick:   *out_msg = ID_Node_RightDoubleClick;   out_mouse->button = kMouseButtonRight;  out_mouse->clickCount = 2; break;
    case kEventOtherDoubleClick:   *out_msg = ID_Node_OtherDoubleClick;   out_mouse->button = kMouseButtonMiddle; out_mouse->clickCount = 2; break;
    case kEventMouseMoved:         *out_msg = ID_Node_MouseMoved;         out_mouse->button = kMouseButtonNone; break;
    case kEventScrollWheel:        *out_msg = ID_Node_ScrollWheel;        out_mouse->button = kMouseButtonNone; break;
    default:
      return false;
  }
  return true;
}
//
//static int
//lua_pushmousevent(lua_State* L, lpObject_t obj, struct AXmessage* e)
//{
//  struct vec3 point = {LOWORD(e->wParam),HIWORD(e->wParam)};
//#ifdef MOUSE_EVENTS_USE_LOCAL_SPACE
//  if (GetNode2D(obj)) {
//    struct mat4 inv = MAT4_Inverse(&GetNode2D(obj)->Matrix);
//    point = MAT4_MultiplyVector3D(&inv, &point);
//  }
//#endif
//  uint32_t msg;
//  Node_MouseMessageMsg_t mouse;
//  if (!convert_mouse_message(e, &msg, &mouse)) {
//    return 0;
//  }
//  switch (e->message) {
//    case kEventMouseMoved:
//    case kEventLeftMouseDown:
//    case kEventRightMouseDown:
//    case kEventOtherMouseDown:
//    case kEventLeftMouseUp:
//    case kEventRightMouseUp:
//    case kEventOtherMouseUp:
//    case kEventScrollWheel:
//    case kEventLeftMouseDragged:
//    case kEventRightMouseDragged:
//    case kEventOtherMouseDragged:
//      {
//        lua_pushlightuserdata(L, (void*)(intptr_t)msg);
//        lua_gettable(L, LUA_REGISTRYINDEX);
//        // luaL_checktype(L, -1, LUA_TTABLE);
//        lua_pushlightuserdata(L, &mouse);
//        lua_call(L, 1, 1);
//      }
//      // luaX_pushMouse_MouseMessageEventArgs(L, &mouse);
//      return 1;
//    case kEventDragDrop:
//    case kEventDragEnter:
//      lua_getfield(L, LUA_REGISTRYINDEX, DRAG_SESSION);
//      // lua_pushnumber(L, LOWORD(e->wParam));
//      // lua_pushnumber(L, HIWORD(e->wParam));
//      lua_pushnumber(L, point.x);
//      lua_pushnumber(L, point.y);
//      return 3;
//    default:
//      return 0;
//  }
//}

static void
process_dragndrop(lua_State *L, struct AXmessage *e, lpObject_t sender)
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
          e->message = ID_Node_DragDrop;
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
          e->message = ID_Node_DragEnter;
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

void axBuildModifiersString(wParam_t wParam, char* buf, size_t size);
void axKeyEventToText(struct AXmessage const* e, char* buf, size_t size);

static bool
build_key_msg(struct AXmessage const* e, Node_KeyMessageMsg_t* key, uint32_t *msg)
{
  static char modifiersString[MAX_PROPERTY_STRING];
  static char hotKey[MAX_PROPERTY_STRING];
  static char text[MAX_PROPERTY_STRING];
  switch (e->message) {
    case kEventKeyDown: *msg = ID_Node_KeyDown; break;
    case kEventKeyUp:   *msg = ID_Node_KeyUp; break;
    case kEventChar:    *msg = ID_Node_TextInput; break;
    default:
      return false;
  }
  key->keyCode = e->keyCode;
  key->character = *(unsigned char*)&e->lParam;
  key->modifiers = e->wParam & (AX_MOD_SHIFT|AX_MOD_CTRL|AX_MOD_ALT|AX_MOD_CMD);
  key->modifiersString = modifiersString;
  key->hotKey = hotKey;
  key->text = text;
  axKeyEventToText(e, text, sizeof(text));
  axBuildModifiersString(e->wParam, modifiersString, sizeof(modifiersString));
  snprintf(hotKey, sizeof(hotKey), "%s%s", key->modifiersString, key->text);
  return true;
}

LRESULT
UI_HandleMouseEvent(lua_State* L, lpObject_t root, struct AXmessage* e)
{
  uint16_t x = LOWORD(e->wParam), y = HIWORD(e->wParam);
  lpObject_t sender = NULL;
  bool_t success = FALSE;
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
  
  // Perform hit testing from the root if we didn't find a target through
  // the focused object path
  if (!(sender = (lpObject_t)_SendMessage(root, Node, HitTest, x, y)))
    return FALSE;
  
handle:
  
  // If the event is a mouse down or drag, we need to check if there's
  // an active drag session in order to properly route drag and drop events.
  // This is necessary because once a drag session is active, the mouse events
  // will be captured by the drag and drop system and won't go through the normal
  // hit testing and routing process.
  process_dragndrop(L, e, sender);
  
  uint32_t msg;
  Node_MouseMessageMsg_t mouse;
  convert_mouse_message(e, &msg, &mouse);

  // Route the event up the parent chain until it's handled.
  success = CORE_HandleObjectMessage(L, &(struct AXmessage) {
    .target = sender,
    .message = msg,
    .wParam = 0,
    .lParam = &mouse,
  });
  // This allows for event delegation, where a parent object can choose to
  // handle events for its children. For example, a list item could delegate
  // mouse events to its parent list for handling selection.
  // for (lpObject_t obj = sender; !success && obj; obj = OBJ_GetParent(obj)) {
  //   lpcString_t szCallback = OBJ_FindCallbackForID(obj, e->message);
  //   if (szCallback) {
  //     luaX_import(L, "orca", "async");
  //     if (luaX_pushObject(L, obj), lua_isnil(L, -1)) {
  //       Con_Warning("Object has no Lua representation: %p", obj);
  //       lua_pop(L, 2);
  //       continue;
  //     }
  //     lua_getfield(L, -1, szCallback);
  //     lua_insert(L, -2); // Move callback before obj
  //     luaX_pushObject(L, sender);
  //     if (lua_pcall(L, lua_pushmousevent(L, obj, e) + 3, 0, 0) != LUA_OK) {
  //       Con_Error("%s(): %s", szCallback, lua_tostring(L, -1));
  //       lua_pop(L, 1);
  //     }
  //     success = TRUE;
  //   } else if (send_mouse_message(obj, e)) {
  //     success = TRUE;
  //   }
  // }
  
  // If the event wasn't handled by any object in the hierarchy, we still
  // need to process drag and drop events in order to properly update the
  // drag session state and visibility of the dragged view. For example,
  // if the user releases the mouse button to drop an item, we need to clear
  // the drag session and make the dragged view visible again, even if the
  // drop target doesn't handle the event.
  void CORE_UpdateHover(void);
  switch (e->message) {
    case kEventMouseMoved:
      CORE_UpdateHover();
      break;
    case kEventDragDrop:
      lua_pushnil(L);
      lua_setfield(L, LUA_REGISTRYINDEX, DRAG_SESSION);
      break;
    case kEventLeftMouseUp:
      lua_pushnil(L);
      lua_setfield(L, LUA_REGISTRYINDEX, DRAG_SESSION);
      break;
  }
  return success || e->message == ID_Node_DragEnter;
}

bool_t
UI_HandleKeyEvent(lua_State *L, struct AXmessage* e)
{
  uint32_t msg;
  struct Node_KeyMessageEventArgs key = {0};
  build_key_msg(e, &key, &msg);
  return core_GetFocus() && CORE_HandleObjectMessage(L, &(struct AXmessage) {
    .target = core_GetFocus(),
    .message = msg,
    .wParam = 0,
    .lParam = &key,
  });  
  // for (lpObject_t obj = core_GetFocus(); obj; obj = OBJ_GetParent(obj)) {
  //   lpcString_t szCallback = OBJ_FindCallbackForID(obj, e->message);
  //   if (szCallback) {
  //     luaX_import(L, "orca", "async");
  //     luaX_pushObject(L, obj);
  //     //      lua_getfield(L, -1, OBJ_FindCallbackForID(obj, e->message));
  //     lua_getfield(L, -1, "handleEvent");
  //     lua_insert(L, -2); // Move callback before obj
  //     lua_pushstring(L, szCallback);
  //     luaX_pushObject(L, core_GetFocus());
  //     Keyboard_KeyMessageMsg_t key = {0};
  //     build_key_msg(e, &key);
  //     luaX_pushKeyboard_KeyMessageEventArgs(L, &key);
  //     /* Stack: [orca.async, obj.handleEvent, obj, szCallback, focusedObj, keyMsg]
  //      * Calls: obj.handleEvent(obj, szCallback, focusedObj, keyMsg) */
  //     if (lua_pcall(L, 5, 1, 0) != LUA_OK) {
  //       Con_Error("%s(): %s", szCallback, luaL_checkstring(L, -1));
  //       lua_pop(L, 1);
  //     }
  //     return TRUE;
  //   }
  //   if (send_key_message(obj, e)) {
  //     return TRUE;
  //   }
  // }
  // return FALSE;
}


LRESULT ui_handle_event(lua_State *L, struct AXmessage* msg) {
  int tmp;
  switch (msg->message) {
    case kEventLeftMouseDown:
    case kEventRightMouseDown:
    case kEventOtherMouseDown:
    case kEventLeftMouseUp:
    case kEventRightMouseUp:
    case kEventOtherMouseUp:
    case kEventLeftMouseDragged:
    case kEventRightMouseDragged:
    case kEventOtherMouseDragged:
    case kEventLeftDoubleClick:
    case kEventRightDoubleClick:
    case kEventOtherDoubleClick:
    case kEventMouseMoved:
    case kEventScrollWheel:
      return UI_HandleMouseEvent(L, msg->target, msg);
    case kEventKeyDown:
    case kEventKeyUp:
    case kEventChar:
      return UI_HandleKeyEvent(L, msg);
    case kEventResumeCoroutine:
      switch (lua_resume(msg->target, L, LOWORD(msg->wParam), &tmp)) {
        case LUA_OK:
          axPostMessageW(msg->target, kEventStopCoroutine, msg->wParam, NULL);
          break;
        case LUA_YIELD:
          axPostMessageW(msg->target, kEventResumeCoroutine, MAKEDWORD(0, HIWORD(msg->wParam)), NULL);
          break;
        default:
          axPostMessageW(msg->target, kEventStopCoroutine, msg->wParam, NULL);
          if (!lua_isnil(msg->target, -1)) {
            lpcString_t err = lua_tostring(msg->target, -1);
            if (err) fprintf(stderr, "co.resume(): %s\n", err);
          }
          break;
      }
      lua_pop(L, 1);
      /* Return FALSE so kEventStopCoroutine (posted above) can still trigger
         a repaint via axPostMessageW; returning TRUE would short-circuit
         SV_DispatchMessage and the Lua event loop would issue a redundant
         second Window.Paint via its `result` check. */
      return FALSE;
    case kEventStopCoroutine:
      luaL_unref(L, LUA_REGISTRYINDEX, HIWORD(msg->wParam));
      axRemoveFromQueue(msg->target);
      axPostMessageW(NULL, kEventWindowPaint, axGetSize(NULL), 0);
      /* Return FALSE: the coroutine is fully cleaned up; the Window.Paint
         posted above will reach CORE_HandleObjectMessage on its own pass. */
      return FALSE;
    default:
      return CORE_HandleObjectMessage(L, msg);
  }
  return FALSE;
}
