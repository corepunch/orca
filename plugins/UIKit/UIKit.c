#include <UIKit/UIKit.h>
#include <include/api.h>
#include <filesystem/filesystem.h>
#include <ctype.h>
#include <strings.h>

bool_t is_server = FALSE;

extern int f_beginDraggingSession(lua_State *L);
extern LRESULT ui_handle_event(lua_State* L, struct AXmessage *msg);
extern const char *_Overflow[];
extern const char *_FontWeight[];
extern const char *_FontStyle[];

static bool_t
ui_parse_float_token(const char *token, float *out)
{
  char *end = NULL;
  float value = strtof(token, &end);
  if (end == token) return FALSE;
  while (*end && isspace((unsigned char)*end)) end++;
  if (*end && strcasecmp(end, "px") && strcasecmp(end, "pt")) return FALSE;
  *out = value;
  return TRUE;
}

static bool_t
ui_parse_enum_token(const char **values, const char *token, int *out)
{
  for (int i = 0; values && values[i]; i++) {
    if (!strcasecmp(values[i], token)) {
      *out = i;
      return TRUE;
    }
  }
  return FALSE;
}

static int
c_parse_BrushShorthand(const char* str, void* dst, size_t sz)
{
  if (!dst || sz != sizeof(struct BrushShorthand)) return FALSE;
  struct BrushShorthand self = {0};
  self.Color = COLOR_Parse(str);
  *(struct BrushShorthand*)dst = self;
  return TRUE;
}

static int
c_parse_FontShorthand(const char* str, void* dst, size_t sz)
{
  if (!dst || sz != sizeof(struct FontShorthand)) return FALSE;
  char buf[MAX_PROPERTY_STRING] = {0};
  struct FontShorthand self = {0};
  bool_t any = FALSE;
  snprintf(buf, sizeof(buf), "%s", str);
  for (char *tok = strtok(buf, " \t\r\n"); tok; tok = strtok(NULL, " \t\r\n")) {
    float f = 0;
    int value = 0;
    if (ui_parse_float_token(tok, &f)) {
      self.Size = f;
      any = TRUE;
    } else if (ui_parse_enum_token(_FontWeight, tok, &value)) {
      self.Weight = (enum FontWeight)value;
      any = TRUE;
      if (!strcasecmp(tok, "Normal") &&
          ui_parse_enum_token(_FontStyle, tok, &value)) {
        self.Style = (enum FontStyle)value;
      }
    } else if (ui_parse_enum_token(_FontStyle, tok, &value)) {
      self.Style = (enum FontStyle)value;
      any = TRUE;
    }
  }
  *(struct FontShorthand*)dst = self;
  return any;
}

static int
c_parse_UnderlineShorthand(const char* str, void* dst, size_t sz)
{
  if (!dst || sz != sizeof(struct UnderlineShorthand)) return FALSE;
  char buf[MAX_PROPERTY_STRING] = {0};
  struct UnderlineShorthand self = {0};
  int number = 0;
  bool_t any = FALSE;
  snprintf(buf, sizeof(buf), "%s", str);
  for (char *tok = strtok(buf, " \t\r\n"); tok; tok = strtok(NULL, " \t\r\n")) {
    float f = 0;
    if (ui_parse_float_token(tok, &f)) {
      if (number++ == 0) self.Width = f;
      else self.Offset = f;
      any = TRUE;
    } else {
      self.Color = COLOR_Parse(tok);
      any = TRUE;
    }
  }
  *(struct UnderlineShorthand*)dst = self;
  return any;
}

static int
c_parse_ShadowShorthand(const char* str, void* dst, size_t sz)
{
  if (!dst || sz != sizeof(struct ShadowShorthand)) return FALSE;
  char buf[MAX_PROPERTY_STRING] = {0};
  float values[4] = {0};
  int n = 0;
  struct ShadowShorthand self = {0};
  bool_t any = FALSE;
  snprintf(buf, sizeof(buf), "%s", str);
  for (char *tok = strtok(buf, " \t\r\n"); tok; tok = strtok(NULL, " \t\r\n")) {
    float f = 0;
    if (n < 4 && ui_parse_float_token(tok, &f)) {
      values[n++] = f;
      any = TRUE;
    } else {
      self.Color = COLOR_Parse(tok);
      any = TRUE;
    }
  }
  if (n > 0) self.Offset.x = values[0];
  if (n > 1) self.Offset.y = values[1];
  if (n > 2) self.BlurRadius = values[2];
  if (n > 3) self.SpreadRadius = values[3];
  *(struct ShadowShorthand*)dst = self;
  return any;
}

static int
c_parse_RingShorthand(const char* str, void* dst, size_t sz)
{
  if (!dst || sz != sizeof(struct RingShorthand)) return FALSE;
  char buf[MAX_PROPERTY_STRING] = {0};
  struct RingShorthand self = {0};
  int number = 0;
  bool_t any = FALSE;
  snprintf(buf, sizeof(buf), "%s", str);
  for (char *tok = strtok(buf, " \t\r\n"); tok; tok = strtok(NULL, " \t\r\n")) {
    float f = 0;
    if (ui_parse_float_token(tok, &f)) {
      if (number++ == 0) self.Width = f;
      else self.Offset = f;
      any = TRUE;
    } else {
      self.Color = COLOR_Parse(tok);
      any = TRUE;
    }
  }
  *(struct RingShorthand*)dst = self;
  return any;
}

static int
c_parse_OverflowShorthand(const char* str, void* dst, size_t sz)
{
  if (!dst || sz != sizeof(struct OverflowShorthand)) return FALSE;
  char buf[MAX_PROPERTY_STRING] = {0};
  struct OverflowShorthand self = {0};
  int value = 0;
  int n = 0;
  snprintf(buf, sizeof(buf), "%s", str);
  for (char *tok = strtok(buf, " \t\r\n"); tok && n < 2; tok = strtok(NULL, " \t\r\n")) {
    if (!ui_parse_enum_token(_Overflow, tok, &value)) return FALSE;
    if (n++ == 0) {
      self.x = (enum Overflow)value;
      self.y = (enum Overflow)value;
    } else {
      self.y = (enum Overflow)value;
    }
  }
  *(struct OverflowShorthand*)dst = self;
  return n > 0;
}

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

void on_ui_module_registered(lua_State* L) {
  luaX_require(L, "orca.core", 0);
  OBJ_RegisterStructParser("BrushShorthand", c_parse_BrushShorthand);
  OBJ_RegisterStructParser("FontShorthand", c_parse_FontShorthand);
  OBJ_RegisterStructParser("UnderlineShorthand", c_parse_UnderlineShorthand);
  OBJ_RegisterStructParser("ShadowShorthand", c_parse_ShadowShorthand);
  OBJ_RegisterStructParser("RingShorthand", c_parse_RingShorthand);
  OBJ_RegisterStructParser("OverflowShorthand", c_parse_OverflowShorthand);
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
