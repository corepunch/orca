#include <plugins/UIKit/UIKit.h>
#include <include/api.h>

#define DRAG_SESSION "__DRAG_SESSION__"
#define DRAG_THRESHOLD 4

int f_Thickness_New(lua_State* L) {
  struct Thickness self={0};
  switch (lua_gettop(L))
  {
    case 4:
      self.Axis[0] = (struct EdgeShorthand){luaL_checknumber(L, 1), luaL_checknumber(L, 3)};
      self.Axis[1] = (struct EdgeShorthand){luaL_checknumber(L, 2), luaL_checknumber(L, 4)};
      return (luaX_pushThickness(L, &self), 1);
    case 3:
      self.Axis[0] = (struct EdgeShorthand){luaL_checknumber(L, 1), luaL_checknumber(L, 3)};
      self.Axis[1] = (struct EdgeShorthand){luaL_checknumber(L, 2), luaL_checknumber(L, 2)};
      return (luaX_pushThickness(L, &self), 1);
    case 2:
      self.Axis[0] = (struct EdgeShorthand){luaL_checknumber(L, 1), luaL_checknumber(L, 1)};
      self.Axis[1] = (struct EdgeShorthand){luaL_checknumber(L, 2), luaL_checknumber(L, 2)};
      return (luaX_pushThickness(L, &self), 1);
    case 1:
      self.Axis[0] = (struct EdgeShorthand){luaL_checknumber(L, 1), luaL_checknumber(L, 1)};
      self.Axis[1] = (struct EdgeShorthand){luaL_checknumber(L, 1), luaL_checknumber(L, 1)};
      return (luaX_pushThickness(L, &self), 1);
    default:
      return 0;
  }
}

int f_EdgeShorthand_New(lua_State* L) {
  switch (lua_gettop(L)) {
    case 2:
      return (luaX_pushEdgeShorthand(L, &(struct EdgeShorthand){luaL_checknumber(L, 1), luaL_checknumber(L, 2)}), 1);
    case 1:
      return (luaX_pushEdgeShorthand(L, &(struct EdgeShorthand){luaL_checknumber(L, 1), luaL_checknumber(L, 1)}), 1);
    default:
      return 0;
  }
}

int f_CornerRadius_New(lua_State* L) {
  switch (lua_gettop(L)) {
    case 4:
      return (luaX_pushCornerRadius(L, &(struct CornerRadius){luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4)}), 1);
    case 1:
      return (luaX_pushCornerRadius(L, &(struct CornerRadius){luaL_checknumber(L, 1), luaL_checknumber(L, 1), luaL_checknumber(L, 1), luaL_checknumber(L, 1)}), 1);
    default:
      return 0;
  }
}

int f_Thickness_TextConvert(lua_State* L) {
  float a, b, c, d;
  struct Thickness self={0};
  switch (sscanf(luaL_checkstring(L, 1), "%f %f %f %f", &a, &b, &c, &d)) {
    case 4:
      self.Axis[0] = (struct EdgeShorthand){a, c};
      self.Axis[1] = (struct EdgeShorthand){b, d};
      return (luaX_pushThickness(L, &self), 1);
    case 3:
      self.Axis[0] = (struct EdgeShorthand){a, c};
      self.Axis[1] = (struct EdgeShorthand){b, b};
      return (luaX_pushThickness(L, &self), 1);
    case 2:
      self.Axis[0] = (struct EdgeShorthand){a, a};
      self.Axis[1] = (struct EdgeShorthand){b, b};
      return (luaX_pushThickness(L, &self), 1);
    case 1:
      self.Axis[0] = (struct EdgeShorthand){a, a};
      self.Axis[1] = (struct EdgeShorthand){a, a};
      return (luaX_pushThickness(L, &self), 1);
    default:
      return 0;
  }
}

int f_EdgeShorthand_TextConvert(lua_State* L) {
  float a = 0, b = 0;
  switch (sscanf(luaL_checkstring(L, 1), "%f %f", &a, &b)) {
    case 2:
      return (luaX_pushEdgeShorthand(L, &(struct EdgeShorthand){a, b}), 1);
    case 1:
      return (luaX_pushEdgeShorthand(L, &(struct EdgeShorthand){a, a}), 1);
    default:
      return 0;
  }
}

int f_CornerRadius_TextConvert(lua_State* L) {
  float a, b, c, d;
  switch (sscanf(luaL_checkstring(L, 1), "%f %f %f %f", &a, &b, &c, &d)) {
    case 4:
      return (luaX_pushCornerRadius(L, &(struct CornerRadius){a, b, c, d}), 1);
    case 1:
      return (luaX_pushCornerRadius(L, &(struct CornerRadius){a, a, a, a}), 1);
    default:
      return 0;
  }
}

static int lua_pushmousevent(lua_State* L,
                             lpObject_t obj,
                             struct WI_Message* e)
{
  struct vec3 point = {LOWORD(e->wParam),HIWORD(e->wParam)};
#ifdef MOUSE_EVENTS_USE_LOCAL_SPACE
  if (GetNode2D(obj)) {
    struct mat4 inv = MAT4_Inverse(&GetNode2D(obj)->Matrix);
    point = MAT4_MultiplyVector3D(&inv, &point);
  }
#endif
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
      // lua_pushnumber(L, LOWORD(e->wParam));
      // lua_pushnumber(L, HIWORD(e->wParam));
      lua_pushnumber(L, point.x);
      lua_pushnumber(L, point.y);
      lua_pushnumber(L, LOWORD((intptr_t)e->lParam));
      lua_pushnumber(L, HIWORD((intptr_t)e->lParam));
      return 4;
    case kEventDragDrop:
    case kEventDragEnter:
      lua_getfield(L, LUA_REGISTRYINDEX, DRAG_SESSION);
      // lua_pushnumber(L, LOWORD(e->wParam));
      // lua_pushnumber(L, HIWORD(e->wParam));
      lua_pushnumber(L, point.x);
      lua_pushnumber(L, point.y);
      return 3;
    default:
      return 0;
  }
}

static void process_dragndrop(lua_State *L, struct WI_Message *e, lpObject_t sender) {
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

LRESULT send_mouse_message(lua_State* L, lpObject_t obj, struct WI_Message* e) {
  uint32_t msg = -1;
  switch (e->message) {
    case kEventLeftMouseUp: msg = ID_Input_LeftMouseUp; break;
    case kEventRightMouseUp: msg = ID_Input_RightMouseUp; break;
    case kEventOtherMouseUp: msg = ID_Input_OtherMouseUp; break;
    case kEventLeftMouseDown: msg = ID_Input_LeftMouseDown; break;
    case kEventRightMouseDown: msg = ID_Input_RightMouseDown; break;
    case kEventOtherMouseDown: msg = ID_Input_OtherMouseDown; break;
    case kEventLeftMouseDragged: msg = ID_Input_LeftMouseDragged; break;
    case kEventRightMouseDragged: msg = ID_Input_RightMouseDragged; break;
    case kEventOtherMouseDragged: msg = ID_Input_OtherMouseDragged; break;
    case kEventMouseMoved: msg = ID_Input_MouseMoved; break;
    case kEventScrollWheel: msg = ID_Input_ScrollWheel; break;
    default:
      return FALSE;
  }
  return OBJ_SendMessageW(obj, msg, 0, e);
}

LRESULT
UI_HandleMouseEvent(lua_State* L, lpObject_t root, struct WI_Message* e)
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
  
  // Route the event up the parent chain until it's handled. 
  // This allows for event delegation, where a parent object can choose to 
  // handle events for its children. For example, a list item could delegate 
  // mouse events to its parent list for handling selection.
  for (lpObject_t obj = sender; !success && obj; obj = OBJ_GetParent(obj)) {
    if (OBJ_FindCallbackForID(obj, e->message)) {
      lpcString_t szCallback = OBJ_FindCallbackForID(obj, e->message);
      luaX_import(L, "orca", "async");
      if (luaX_pushObject(L, obj), lua_isnil(L, -1)) {
        Con_Warning("Object has no Lua representation: %p", obj);
        lua_pop(L, 2);
        continue;
      }
      lua_getfield(L, -1, szCallback);
      lua_insert(L, -2); // Move callback before obj
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
  return success || e->message == ID_Input_DragEnter;
}

LRESULT ui_handle_event(lua_State* L, struct WI_Message *msg) {
  switch (msg->message) {
    case kEventLeftMouseUp:
    case kEventRightMouseUp:
    case kEventOtherMouseUp:
    case kEventLeftMouseDown:
    case kEventRightMouseDown:
    case kEventOtherMouseDown:
    case kEventLeftMouseDragged:
    case kEventRightMouseDragged:
    case kEventOtherMouseDragged:
    case kEventMouseMoved:
    case kEventScrollWheel:
      return UI_HandleMouseEvent(L, msg->target, msg);
    default:
      return FALSE;
  }
}

bool_t is_server = FALSE;

static int f_beginDraggingSession(lua_State *L) {
  //  luaX_parsefield(bool_t, active, 1, lua_toboolean);
  //  luaX_parsefield(bool_t, item, 1, lua_toboolean);
  lua_pushvalue(L, 1);
  lua_setfield(L, LUA_REGISTRYINDEX, DRAG_SESSION);
  return 0;
}

void on_ui_module_registered(lua_State* L) {
  API_CallRequire(L, "orca.core", 0);
  lua_getglobal(L, "SERVER");
  is_server = lua_toboolean(L, -1);
  lua_pop(L, 1);
  SV_RegisterMessageProc(ui_handle_event);
  lua_pushcfunction(L, f_beginDraggingSession);
  lua_setfield(L, -2, "beginDraggingSession");
  
#define OVERRIDE_FROMSTRING(NAME) \
  int f_##NAME##_TextConvert(lua_State* L); \
  int f_##NAME##_New(lua_State* L); \
  lua_getfield(L, -1, #NAME); \
  lua_pushcfunction(L, f_##NAME##_TextConvert); \
  lua_setfield(L, -2, "fromstring"); \
  lua_pushcfunction(L, f_##NAME##_New); \
  lua_setfield(L, -2, "new"); \
  lua_pop(L, 1);
  OVERRIDE_FROMSTRING(Thickness)
  OVERRIDE_FROMSTRING(EdgeShorthand)
  OVERRIDE_FROMSTRING(CornerRadius)
#undef OVERRIDE_FROMSTRING
}

