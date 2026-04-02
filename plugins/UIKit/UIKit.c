#include <plugins/UIKit/UIKit.h>
#include <include/api.h>

#define DRAG_SESSION "__DRAG_SESSION__"

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

// ---------------------------------------------------------------------------
// UIKit-specific callbacks registered with the core event router.
// ---------------------------------------------------------------------------

// Perform a spatial hit-test: find the deepest Node under (x, y) within
// obj's subtree.  Returns the hit object, or NULL if nothing was found.
static lpObject_t ui_hit_test(lpObject_t obj, int x, int y) {
  return (lpObject_t)_SendMessage(obj, Node, HitTest, x, y);
}

// Show or hide the drag-ghost view that follows the cursor during a drag
// session.  Called by the core drag-state machine; UIKit provides the actual
// Node / Node2D manipulation.
static void ui_drag_ghost(lua_State* L, lpObject_t view, bool_t visible, int x, int y) {
  if (!view) return;
  if (GetNode(view))
    GetNode(view)->Visible = visible;
  if (visible && GetNode2D(view))
    GetNode2D(view)->LayoutTransform.translation = (vec2_t){ x, y };
}

bool_t is_server = FALSE;

static int f_beginDraggingSession(lua_State *L) {
  lua_pushvalue(L, 1);
  lua_setfield(L, LUA_REGISTRYINDEX, DRAG_SESSION);
  return 0;
}

void on_ui_module_registered(lua_State* L) {
  API_CallRequire(L, "orca.core", 0);
  lua_getglobal(L, "SERVER");
  is_server = lua_toboolean(L, -1);
  lua_pop(L, 1);
  CORE_RegisterHitTestProc(ui_hit_test);
  CORE_RegisterDragGhostProc(ui_drag_ghost);
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

