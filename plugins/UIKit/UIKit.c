#include <plugins/UIKit/UIKit.h>
#include <include/api.h>

#define DRAG_SESSION "__DRAG_SESSION__"
#define DRAG_THRESHOLD 4

void EdgeShorthand_Convert1(struct EdgeShorthand* self, float value) {
  self->Left = value;
  self->Right = value;
}

void BorderRadiusShorthand_Convert1(struct  BorderRadiusShorthand* self, float value) {
  self->BottomLeftRadius = value;
  self->BottomRightRadius = value;
  self->TopLeftRadius = value;
  self->TopRightRadius = value;
}

void MarginShorthand_Convert4(struct MarginShorthand* self, float left, float top, float right, float bottom) {
  self->Axis[0].Left = left;
  self->Axis[0].Right = right;
  self->Axis[1].Left = top;
  self->Axis[1].Right = bottom;
}
void MarginShorthand_Convert2(struct MarginShorthand* self, float horizontal, float vertical) {
  self->Axis[0].Left = horizontal;
  self->Axis[0].Right = horizontal;
  self->Axis[1].Left = vertical;
  self->Axis[1].Right = vertical;
}
void MarginShorthand_Convert1(struct MarginShorthand* self, float uniform) {
  self->Axis[0].Left = uniform;
  self->Axis[0].Right = uniform;
  self->Axis[1].Left = uniform;
  self->Axis[1].Right = uniform;
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
    case kMsgMouseMoved:
    case kMsgLeftMouseDown:
    case kMsgRightMouseDown:
    case kMsgOtherMouseDown:
    case kMsgLeftMouseUp:
    case kMsgRightMouseUp:
    case kMsgOtherMouseUp:
    case kMsgScrollWheel:
    case kMsgLeftMouseDragged:
    case kMsgRightMouseDragged:
    case kMsgOtherMouseDragged:
      // lua_pushnumber(L, LOWORD(e->wParam));
      // lua_pushnumber(L, HIWORD(e->wParam));
      lua_pushnumber(L, point.x);
      lua_pushnumber(L, point.y);
      lua_pushnumber(L, LOWORD((intptr_t)e->lParam));
      lua_pushnumber(L, HIWORD((intptr_t)e->lParam));
      return 4;
    case kMsgDragDrop:
    case kMsgDragEnter:
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

LRESULT
UI_HandleMouseEvent(lua_State* L, lpObject_t root, struct WI_Message* e)
{
  lpObject_t focused = core_GetFocus();
  lpObject_t sender = NULL;
  bool_t success = FALSE;
  while (OBJ_GetNext(root)) {
    root = OBJ_GetNext(root);
  }
  struct HitTestMsgArgs hitMsg = { .x = LOWORD(e->wParam), .y = HIWORD(e->wParam) };
  if (focused && OBJ_GetFlags(focused)&OF_NOACTIVATE) {
    if ((sender = (lpObject_t)OBJ_SendMessageW(focused, kMsgHitTest, 0, &hitMsg)))
      goto handle;
    if (e->message == kMsgLeftMouseDown) {
      for (lpObject_t mod = focused, p = OBJ_GetParent(focused);
           mod && (OBJ_GetFlags(mod)&OF_NOACTIVATE);
           mod = p, p = p?OBJ_GetParent(p):NULL)
      {
        if ((sender = (lpObject_t)OBJ_SendMessageW(mod, kMsgHitTest, 0, &hitMsg)))
          goto handle;
        OBJ_RemoveFromParent(L, mod);
      }
      return TRUE;
    }
  }
  if (!(sender = (lpObject_t)OBJ_SendMessageW(root, kMsgHitTest, 0, &hitMsg)))
    return FALSE;
handle:
  switch (e->message) {
    case kMsgLeftMouseDown:
    case kMsgRightMouseDown:
    case kMsgOtherMouseDown:
      OBJ_SetFocus(sender);
      break;
    case kMsgLeftMouseUp:
      OBJ_SetFocus(sender);
      if (lua_getfield(L, LUA_REGISTRYINDEX, DRAG_SESSION) == LUA_TTABLE) {
        luaX_parsefield(bool_t, active, -1, lua_toboolean);
        luaX_parsefield(lpObject_t , view, -1, luaX_checkObject);
        if (active) {
          if (GetNode(view)) {
            GetNode(view)->Visible = FALSE;
          }
          e->message = kMsgDragDrop;
        }
      }
      lua_pop(L, 1);
      break;
    case kMsgLeftMouseDragged:
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
          e->message = kMsgDragEnter;
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
    } else if (OBJ_SendMessageW(obj, e->message, 0, e)) {
      success = TRUE;
    }
  }
  
  void CORE_UpdateHover(void);
  switch (e->message) {
    case kMsgMouseMoved:
      CORE_UpdateHover();
      break;
    case kMsgDragDrop:
      lua_pushnil(L);
      lua_setfield(L, LUA_REGISTRYINDEX, DRAG_SESSION);
      break;
    case kMsgLeftMouseUp:
      lua_pushnil(L);
      lua_setfield(L, LUA_REGISTRYINDEX, DRAG_SESSION);
      break;
  }
  return success || e->message == kMsgDragEnter;
}

LRESULT ui_handle_event(lua_State* L, struct WI_Message *msg) {
  switch (msg->message) {
    case kMsgLeftMouseUp:
    case kMsgRightMouseUp:
    case kMsgOtherMouseUp:
    case kMsgLeftMouseDown:
    case kMsgRightMouseDown:
    case kMsgOtherMouseDown:
    case kMsgLeftMouseDragged:
    case kMsgRightMouseDragged:
    case kMsgOtherMouseDragged:
    case kMsgMouseMoved:
    case kMsgScrollWheel:
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
}

