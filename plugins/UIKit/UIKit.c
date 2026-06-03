#include <UIKit/UIKit.h>
#include <include/api.h>
#include <filesystem/filesystem.h>
#include <plugins/UIKit/ui_data.h>

struct UIData *
Object_UIData(struct Object *object)
{
  if (!object || object->super_id != SUPER_ID_NODE2D) return NULL;
  return (struct UIData *)object->typedata;
}

bool_t is_server = FALSE;

extern int f_beginDraggingSession(lua_State *L);
extern LRESULT ui_handle_event(lua_State* L, struct AXmessage *msg);

static int
c_parse_transform2(const char* str, void* dst, size_t sz)
{
  if (!dst || sz != sizeof(struct transform2)) return FALSE;
  float x = 0, y = 0, r = 0, sx = 1, sy = 1;
  struct transform2 self = { .scale = {1, 1} };
  switch (sscanf(str, "%f %f %f %f %f", &x, &y, &r, &sx, &sy)) {
    case 5:
      self.scale.y = sy;
      self.scale.x = sx;
      self.rotation = r;
      self.translation = (struct vec2){x, y};
      *(struct transform2*)dst = self;
      return TRUE;
    case 4:
      self.scale.x = sx;
      self.rotation = r;
      self.translation = (struct vec2){x, y};
      *(struct transform2*)dst = self;
      return TRUE;
    case 3:
      self.rotation = r;
      self.translation = (struct vec2){x, y};
      *(struct transform2*)dst = self;
      return TRUE;
    case 2:
      self.translation = (struct vec2){x, y};
      *(struct transform2*)dst = self;
      return TRUE;
    default: return FALSE;
  }
}

#define REG_UIDATA(NAME) \
  OBJ_SetClassTypedataOffset(ID_##NAME, (uint32_t)offsetof(struct UIData, NAME))

void on_ui_module_registered(lua_State* L) {
  luaX_require(L, "orca.core", 0);
  OBJ_RegisterStorageFamily(SUPER_ID_NODE2D, sizeof(struct UIData));
  /* Register UIData-relative offsets so defaults copy into the right slot */
  REG_UIDATA(Node2D);
  REG_UIDATA(StyleController);
  REG_UIDATA(TextRun);
  REG_UIDATA(TextBlock);
  REG_UIDATA(PrefabView2D);
  REG_UIDATA(ImageView);
  REG_UIDATA(Grid);
  REG_UIDATA(UniformGrid);
  REG_UIDATA(StackView);
  REG_UIDATA(Form);
  REG_UIDATA(RadioButton);
  REG_UIDATA(RadioGroup);
  REG_UIDATA(Tab);
  REG_UIDATA(TabBar);
  REG_UIDATA(Button);
  REG_UIDATA(Label);
  REG_UIDATA(Input);
  REG_UIDATA(Screen);
  REG_UIDATA(Popup);
  REG_UIDATA(TabView);
  REG_UIDATA(ConsoleView);
  REG_UIDATA(Page);
  REG_UIDATA(PageHost);
  REG_UIDATA(PageViewport);
  REG_UIDATA(Cinematic);
  REG_UIDATA(Control);
  REG_UIDATA(NinePatchImage);
  OBJ_RegisterStructParser("Transform2D", c_parse_transform2);
  lua_getglobal(L, "SERVER");
  is_server = lua_toboolean(L, -1);
  lua_pop(L, 1);
  SV_RegisterMessageProc(ui_handle_event);
  lua_pushcfunction(L, f_beginDraggingSession);
  lua_setfield(L, -2, "beginDraggingSession");
}

void
after_ui_module_registered(lua_State* L) {
  // Register UIKit table in package.loaded so TerminalView.lua can require it.
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "loaded");
  lua_pushvalue(L, -3); // UIKit table
  lua_setfield(L, -2, "orca.UIKit");
  lua_pop(L, 2); // pop loaded, package

  // Load TerminalView Lua extension and expose it as UIKit.TerminalView.
  // Guard against recursive require() returning package.loaded's in-progress
  // sentinel (boolean true) when LayerPrefabPlaceholder triggers require()
  // during this same luaopen call — only assign if a real table/function was returned.
  if (luaX_require(L, "orca.UIKit.TerminalView", 1) == LUA_OK) {
    if (lua_istable(L, -1) || lua_isfunction(L, -1)) {
      lua_setfield(L, -2, "TerminalView");
    } else {
      lua_pop(L, 1);
    }
  }
}

#include <UIKit/UIKit_export.c>
