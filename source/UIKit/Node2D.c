#include <include/api.h>
#include <include/orca.h>
#include <include/plugapi.h>

#include <source/UIKit/UIKit.h>

#define DRAG_SESSION "__DRAG_SESSION__"
#define DRAG_THRESHOLD 4

HANDLER(Node2D, UpdateGeometry) {
  float const w = Node2D_GetFrame(pNode2D, kBox3FieldWidth);
  float const h = Node2D_GetFrame(pNode2D, kBox3FieldHeight);
  int const data = ((int)w << 16) | (int)h;
  pNode2D->_userdata = data;
  pNode2D->_rect = (struct rect){0,0,w,h};
  return TRUE;
}

static bool_t _ContainsPoint(Node2DPtr pNode2D, float x, float y) {
  struct mat4 inv = MAT4_Inverse(&pNode2D->Matrix);
	struct vec3 point = {x,y,0};
  float w = Node2D_GetFrame(pNode2D, kBox3FieldWidth);
  float h = Node2D_GetFrame(pNode2D, kBox3FieldHeight);
  struct vec3 out = MAT4_MultiplyVector3D(&inv, &point);
  return RECT_Contains(&(struct rect){0,0,w,h}, (struct vec2 const*)&out);
}

HANDLER(Node2D, HitTest) {
  if (OBJ_IsHidden(hObject) || pNode2D->IgnoreHitTest) {
    return FALSE;
  }
  int16_t x = LOWORD(wParam);
  int16_t y = HIWORD(wParam);
  int16_t lx = x - pNode2D->ContentOffset.x;
  int16_t ly = y - pNode2D->ContentOffset.y;
  bool_t success = FALSE;
  FOR_EACH_OBJECT(hChild, hObject) {
    lpObject_t hittest = NULL;
    if (OBJ_SendMessageW(hChild, kEventHitTest, MAKEDWORD(lx, ly), &hittest)) {
      success = TRUE;
      *pHitTest = hittest;
    }
  }
  if (success) {
    return TRUE;
  } else if (_ContainsPoint(pNode2D, x, y)) {
    *pHitTest = hObject;
    return TRUE;
  } else {
    return FALSE;
  }
}


HANDLER(Node2D, Create) {
  pNode2D->_object = hObject;
  pNode2D->_node = GetNode(hObject);
  return FALSE;
}

// static float
// Node2D_GetWidth(Node2DPtr pNode)
//{
//     float *pwidth = &NODE2D_FRAME(pNode, Size, 0);
//	lpProperty_t p = Node_GetProperty(pNode->_object, kNodeWidth);
//     PROP_Update(p);
//     return MAX(640, *pwidth);
// }
//
// static float
// Node2D_GetHeight(Node2DPtr pNode)
//{
//     float *pheight = &NODE2D_FRAME(pNode, Size, 1);
//	lpProperty_t p = Node_GetProperty(pNode->_object, kNodeHeight);
//     PROP_Update(p);
//     return MAX(480, *pheight);
// }

// int stereoSeparation = 0;

ORCA_API struct rect
Node2D_GetRect(Node2DPtr node)
{
  return (struct rect){
    .x = Node2D_GetFrame(node, kBox3FieldX),
    .y = Node2D_GetFrame(node, kBox3FieldY),
    .width = Node2D_GetFrame(node, kBox3FieldWidth),
    .height = Node2D_GetFrame(node, kBox3FieldHeight),
  };
}

bool_t
Node2D_IsFrameSet(Node2DPtr pNode2D, enum Box3Field parm)
{
  return (pNode2D->_bbox_flags & (1 << parm)) != 0;
}

void
Node2D_SetFrame(Node2DPtr pNode2D, enum Box3Field parm, float value)
{
  switch (parm) {
    case kBox3FieldWidth:
    case kBox3FieldHeight:
    case kBox3FieldDepth:
      NODE2D_FRAME(pNode2D, Size, parm - kBox3FieldWidth).Actual =
        MAX(NODE2D_FRAME(pNode2D, Size, parm - kBox3FieldWidth).Min, value);
      break;
    default:
      pNode2D->_actual_pos[parm] = value;
      break;
  }
  pNode2D->_bbox_flags |= 1 << parm;
}

float
Node2D_GetFrame(Node2DPtr pNode2D, enum Box3Field parm)
{
  if (Node2D_IsFrameSet(pNode2D, parm)) {
    switch (parm) {
      case kBox3FieldWidth:
      case kBox3FieldHeight:
      case kBox3FieldDepth:
        return MAX(NODE2D_FRAME(pNode2D, Size, parm - kBox3FieldWidth).Actual, 0);
      default:
        return pNode2D->_actual_pos[parm];
    }
  } else {
    return 0;
  }
}

// static uint32_t _IndexInParent(lpObject_t  hObject) {
//	if (!OBJ_GetParent(hObject)) {
//		return 0;
//	}
//	uint32_t dwCounter = 0;
//	lpObject_t  p = OBJ_GetParent(hObject);
//	FOR_EACH_OBJECT(hChild, p) {
//		if (hChild == hObject) {
//			return dwCounter;
//		}
//		dwCounter++;
//	}
//	return dwCounter;
// }

HANDLER(Node2D, UpdateMatrix)
{
  struct mat4 Matrix;
  struct vec2 ContentOffset = pNode2D->ContentOffset;
  bool_t bInvalidate =
    pUpdateMatrix->force || (OBJ_GetFlags(hObject) & OF_DIRTY);

  if (1) // bInvalidate)
  {
    struct mat4 renderMatrix;
    struct mat4 layoutMatrix;
    struct transform2 const* RenderTransform = &pNode2D->RenderTransform;
    struct transform2 const* LayoutTransform = &pNode2D->LayoutTransform;
    struct rect frame = Node2D_GetRect(pNode2D);
    struct vec2 size = { frame.width, frame.height };
    struct vec2 pivot = VEC2_Mul(&size, &pNode2D->RenderTransformOrigin);
    struct vec2 center = VEC2_Scale(&size, 0.5f);

    layoutMatrix = transform2_ToMatrix3D(
      &(struct transform2){ .translation = { frame.x, frame.y },
                            .rotation = LayoutTransform->rotation,
                            .scale = LayoutTransform->scale },
      &center);

    renderMatrix = transform2_ToMatrix3D(RenderTransform, &pivot);

    Matrix = MAT4_Multiply(&layoutMatrix, &renderMatrix);

    lpProperty_t matprop = Node2D_GetProperty(hObject, kNode2DMatrix);

    if (matprop && (PROP_GetFlags(matprop) & PF_MODIFIED)) {
      // skip if set from outside
    } else if (pUpdateMatrix->parent) {
      pNode2D->Matrix = MAT4_Multiply(pUpdateMatrix->parent, &Matrix);
    } else {
      pNode2D->Matrix = Matrix;
    }

    pNode2D->_opacity = GetNode(hObject)->Opacity * pUpdateMatrix->opacity;
  }

  Matrix = pNode2D->Matrix;

  if (!pNode2D->RenderTarget) {
    MAT4_Translate(&Matrix, &(struct vec3){
      .x = ContentOffset.x,
      .y = ContentOffset.y,
      .z = 0
    });
  }

  FOR_EACH_CHILD(hObject, OBJ_SendMessageW, kEventUpdateMatrix, 0,
                 &(UPDATEMATRIXSTRUCT){
                   .parent = &Matrix,
                   .opacity = pNode2D->_opacity,
                   .force = bInvalidate,
                 });

  return TRUE;
}

// struct vec3 UI_GetActualSize(lpObject_t  hObject) {
//     Node2DPtr view = GetNode2D(hObject);
//     if (view) {
//         struct vec3 v = {
//             NODE2D_FRAME(hObject, ActualSize, 0),
//             NODE2D_FRAME(hObject, ActualSize, 1),
//             view->_actual_pos[1],
//         };
//         return v;
//     } else {
//         return (struct vec3){0};
//     }
// }

// HACK: Move?

HANDLER(Node, ThemeChanged)
{
	OBJ_ApplyStyles(hObject, FALSE);
  return TRUE;
}

HANDLER(Node, GetSize)
{
  return MAKEDWORD(pNode->Size.Axis[0].Actual, pNode->Size.Axis[1].Actual);
}
HANDLER(Node, IsVisible)
{
  PROP_Update(Node_GetProperty(hObject, kNodeVisible));
  return pNode->Visible && !GetNode(hObject)->QuickHide;
}
HANDLER(Node2D, Destroy)
{
  if (!pNode2D->OffscreenRendering) {
    SafeDelete(pNode2D->RenderTarget, Texture_Release);
  }
  return FALSE;
}

HANDLER(Node2D, HandleMessage)
{
  if (!OBJ_GetLuaObject(hObject))
    return FALSE;
  lua_State* L = pHandleMessage->L;
  shortStr_t pszHandler;
  sprintf(pszHandler, "on%s", pHandleMessage->EventName);
  lua_geti(L, LUA_REGISTRYINDEX, OBJ_GetLuaObject(hObject));
  lua_getfield(L, -1, pszHandler);
  if (lua_type(L, -1) == LUA_TFUNCTION) {
    lua_pop(L, 2);
    for (uint32_t i = 0; i < pHandleMessage->NumArgs; i++) {
      lua_pushvalue(L, pHandleMessage->FirstArg + i);
    }
    return luaX_executecallback(
      L, hObject, pszHandler, pHandleMessage->NumArgs);
  } else {
    lua_pop(L, 2);
  }
  return FALSE;
}

HANDLER(Node2D, ScrollWheel)
{
  NodePtr node = pNode2D->_node;
  if (pNode2D->Overflow.y == kOverflowScroll) {
    struct vec2 Offset = pNode2D->ContentOffset;
    float Scroll = MIN(0, node->Size.Axis[1].Actual - node->Size.Axis[1].Scroll);
    Offset.y =
      MAX(MIN(Offset.y + (int16_t)HIWORD((intptr_t)pScrollWheel->lParam), 0),
          Scroll);
    pNode2D->ContentOffset = Offset;
    OBJ_SetDirty(hObject);
    return TRUE;
  }
  return FALSE;
}

HANDLER(Node2D, MouseMoved)
{
  if (OBJ_GetFlags(hObject) & OF_HOVERABLE) {
    OBJ_SetHover(hObject);
    return TRUE;
  }
  return FALSE;
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
//      lua_pushnumber(L, LOWORD(e->wParam));
//      lua_pushnumber(L, HIWORD(e->wParam));
      lua_pushnumber(L, point.x);
      lua_pushnumber(L, point.y);
      lua_pushnumber(L, LOWORD((intptr_t)e->lParam));
      lua_pushnumber(L, HIWORD((intptr_t)e->lParam));
      return 4;
    case kEventDragDrop:
    case kEventDragEnter:
      lua_getfield(L, LUA_REGISTRYINDEX, DRAG_SESSION);
//      lua_pushnumber(L, LOWORD(e->wParam));
//      lua_pushnumber(L, HIWORD(e->wParam));
      lua_pushnumber(L, point.x);
      lua_pushnumber(L, point.y);
      return 3;
    default:
      return 0;
  }
}

lua_State *g_L=0;

//static bool_t ignore_upcoming_click = FALSE;

bool_t
UI_HandleMouseEvent(lua_State* L, lpObject_t root, struct WI_Message* e)
{
  lpObject_t focused = core_GetFocus();
  lpObject_t sender = NULL;
  bool_t success = FALSE;
  if (focused && OBJ_GetFlags(focused)&OF_NOACTIVATE) {
    if (OBJ_SendMessageW(focused, kEventHitTest, e->wParam, &sender))
      goto handle;
    if (e->message == kEventLeftMouseDown) {
      for (lpObject_t mod = focused, p = OBJ_GetParent(focused);
           mod && (OBJ_GetFlags(mod)&OF_NOACTIVATE);
           mod = p, p = p?OBJ_GetParent(p):NULL)
      {
        if (OBJ_SendMessageW(mod, kEventHitTest, e->wParam, &sender))
          goto handle;
        OBJ_RemoveFromParent(L, mod, FALSE);
//        ignore_upcoming_click = TRUE;
      }
      return TRUE;
    }
  }
  if (!OBJ_SendMessageW(root, kEventHitTest, e->wParam, &sender))
    return FALSE;
handle:
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
          e->message = kEventDragDrop;
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
          e->message = kEventDragEnter;
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
  return success || e->message == kEventDragEnter;
}

int ui_handle_event(lua_State* L, struct WI_Message *msg) {
  switch (msg->message) {
    case kEventLeftMouseUp:
    case kEventRightMouseUp:
    case kEventOtherMouseUp:
//      if (ignore_upcoming_click) {
//        ignore_upcoming_click = FALSE;
//        return FALSE;
//      }
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
  g_L = L;
  API_CallRequire(L, "orca.core", 0);
  lua_getglobal(L, "SERVER");
  is_server = lua_toboolean(L, -1);
  lua_pop(L, 1);
  SV_RegisterMessageProc(ui_handle_event);
  lua_pushcfunction(L, f_beginDraggingSession);
  lua_setfield(L, -2, "beginDraggingSession");
}

float Node2D_GetSize(Node2DPtr pNode2D, eDirection_t axis, eSizing_t sizing) {
  float const size = Node2D_GetFrame(pNode2D, kBox3FieldWidth + axis);
  switch (sizing) {
    case kSizingPlusMargin:
      return size + TOTAL_MARGIN(pNode2D, axis);
    case kSizingSizeOnly:
      return size;
    case kSizingMinusPadding:
      return size - TOTAL_PADDING(pNode2D, axis);
  }
  return 0;
}

