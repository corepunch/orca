#include <include/api.h>

#include "core_local.h"
#include "property/property_internal.h"

struct game core={0};

void SV_Init(void);
void SV_Shutdown(void);

ORCA_API int luaopen_orca_object(lua_State* L);
ORCA_API int luaopen_orca_network(lua_State* L);

bool_t
OBJ_RegisterPropertyType(lpcPropertyType_t pt)
{
  FOR_LOOP(i, MAX_PROPERTY_TYPES) {
    if (!core.ptypes[i].Name) {
      memcpy(&core.ptypes[i], pt, sizeof(struct PropertyType));
      return TRUE;
    } else if (core.ptypes[i].FullIdentifier == pt->FullIdentifier) {
      return TRUE;
    }
  }
  Con_Error("No space left to register property type %s", pt->Name);
  return FALSE;
}

lpcPropertyType_t
OBJ_FindPropertyType(uint32_t ident)
{
  FOR_LOOP(i, MAX_PROPERTY_TYPES) {
    lpcPropertyType_t pt = &core.ptypes[i];
    if (pt && pt->FullIdentifier == ident) {
      return pt;
    }
  }
  return NULL;
}

bool_t
OBJ_RegisterClass(lpcClassDesc_t class)
{
  FOR_LOOP(i, MAX_CLASSES) {
    if (!core.classes[i]) {
      FOR_LOOP(j, class->NumProperties) {
        OBJ_RegisterPropertyType(&class->Properties[j]);
      }
//      Con_Printf("%s %d", class->ClassName, i);
      core.classes[i] = class;
      return TRUE;
    } else if (core.classes[i]->ClassID == class->ClassID) {
      Con_Error("Class %s is already registered", class->ClassName);
      return TRUE;
    }
  }
  Con_Error("No space left to register class %s", class->ClassName);
  return FALSE;
}

lpcClassDesc_t
OBJ_FindClassW(uint32_t class_id)
{
  FOR_LOOP(i, MAX_CLASSES) {
    lpcClassDesc_t cl = core.classes[i];
    if (cl && cl->ClassID == class_id) {
      return cl;
    }
  }
  return NULL;
}

lpcClassDesc_t
OBJ_FindClass(lpcString_t classname)
{
  return OBJ_FindClassW(fnv1a32(classname));
}

static bool_t
is_class_inherited(lpcClassDesc_t self, uint32_t parent)
{
  if (!self) return FALSE;
  if (!parent || self->ClassID == parent) return TRUE;
  for(uint32_t const *cd = self->ParentClasses; *cd; cd++) {
    if (is_class_inherited(OBJ_FindClassW(*cd), parent)) {
      return TRUE;
    }
  }
  return FALSE;
}

void
OBJ_EnumClasses(uint32_t superclass, void (*fnProc)(lpcClassDesc_t, void*), void* param)
{
  FOR_LOOP(i, MAX_CLASSES) {
    if (!superclass || is_class_inherited(core.classes[i], superclass)) {
      fnProc(core.classes[i], param);
    }
  }
}

int MakeLocalizedString(lua_State* L)
{
  lpcString_t string = luaL_checkstring(L, 1);
  uint32_t hashed = fnv1a32(string);
  char buf[MAX_PROPERTY_STRING] = { 0 };
  snprintf(buf, sizeof(buf), "#%08x", hashed);
  lua_pushstring(L, buf);
  return 1;
}

void
API_PrintStackTrace(lua_State* L)
{
  lua_Debug info;
  for (uint32_t level = 0; lua_getstack(L, level, &info); level++) {
    lua_getinfo(L, "nSl", &info);
    Con_Error("[%d] %s:%d -- %s [%s]", level,info.short_src,info.currentline,(info.name ? info.name : "<unknown>"),info.what);
  }
}

void
CL_Frame(void)
{
  //    lpObject_t  hRoot   = server->rootobj;
  //    float   fWidth  = Node2D_GetWidth(hRoot);
  //    float   fHeight = Node2D_GetHeight(hRoot);
  //
  //    UIScreen_UpdateLayout(hRoot, fWidth, fHeight);
  //
  //    IN_Frame();
  //
  //    SCR_UpdateScreen();
}

void UI_Shutdown(void) {
  OBJ_SetFocus(NULL);
}

// bool_t UI_GetProperty(uint32_t, PPROPDEF);

ORCA_API longTime_t
UI_GetTime(void)
{
  return core.realtime;
}

ORCA_API void
CORE_UpdateHover(void) {
  if (core.hover != core.hover2) {
    if (core.hover) {
      OBJ_SendMessage(core.hover, "KillHover", 0, core.hover2);
      axPostMessageW(core.hover, ID_StyleController_ThemeChanged, 0, 0);
    }
    if (core.hover2) {
      OBJ_SendMessage(core.hover2, "SetHover", 0, core.hover);
      axPostMessageW(core.hover2, ID_StyleController_ThemeChanged, 0, 0);
    }
  }
  core.hover = core.hover2;
  core.hover2 = NULL;
}

int f_poll(lua_State* L);

int OBJ_CreateFromLuaState(lua_State *L);

static int class_call(lua_State* L) {
  return OBJ_CreateFromLuaState(L);
}

/* class:extend{...} — Lua-side subclass of a C class.
 * Creates a NEW derived class table.  Extension methods are copied onto it.
 * The derived class metatable uses __index = base so that all base-class
 * methods are found transparently via Lua's own metamethod chain. */
static int class_extend(lua_State* L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  int base = lua_absindex(L, 1);

  lua_newtable(L); /* derived class table */
  int derived = lua_absindex(L, -1);

  /* Copy __nativeclass from base */
  lua_getfield(L, base, "__nativeclass");
  lua_setfield(L, derived, "__nativeclass");

  /* extend() on the derived class creates further subclasses */
  lua_pushcfunction(L, class_extend);
  lua_setfield(L, derived, "extend");

  /* Copy all extension fields verbatim */
  if (lua_type(L, 2) == LUA_TTABLE) {
    int ext = lua_absindex(L, 2);
    lua_pushnil(L);
    while (lua_next(L, ext) != 0) {
      if (lua_type(L, -2) == LUA_TSTRING) {
        lua_setfield(L, derived, lua_tostring(L, -2)); /* pops value */
      } else {
        lua_pop(L, 1);
      }
    }
  }

  /* Derived class metatable: __call → class_call, __index → base class table */
  lua_newtable(L);
  lua_pushcfunction(L, class_call);
  lua_setfield(L, -2, "__call");
  lua_pushvalue(L, base);
  lua_setfield(L, -2, "__index");
  lua_setmetatable(L, derived);

  return 1; /* derived is on top */
}

int lua_pushclass(lua_State* L, struct ClassDesc* cl)
{
  /* Create a plain callable table: { __nativeclass = <lightuserdata> }
   * Its metatable provides __call → class_call and __index → Object metatable. */
  lua_newtable(L); /* class table */
  lua_pushlightuserdata(L, cl);
  lua_setfield(L, -2, "__nativeclass");
  lua_pushcfunction(L, class_extend);
  lua_setfield(L, -2, "extend");

  lua_newtable(L); /* class metatable */
  lua_pushcfunction(L, class_call);
  lua_setfield(L, -2, "__call");
  luaL_getmetatable(L, API_TYPE_OBJECT);
  lua_setfield(L, -2, "__index");
  lua_setmetatable(L, -2); /* apply metatable to class table */

  OBJ_RegisterClass(cl);
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, cl->ClassName);
  return 1;
}

ORCA_API void CORE_AdvanceFrame(void) {
  core.frame++;
}

ORCA_API void luaX_pushlua_State(lua_State* L, lua_State const* other) {
  lua_pushnil(L);
}

static int predicate_always_true(lua_State* L) {
  lua_pushboolean(L, 1);
  return 1;
}

static int predicate_class_check(lua_State* L) {
  luaX_pushObject(L, core_GetFocus());
  lua_getfield(L, -1, "is");
  lua_pushvalue(L, -2);
  lua_pushvalue(L, lua_upvalueindex(1));
  lua_call(L, 2, 1);
  return 1;
}

int f_addCommands(lua_State* L) {
  lua_getfield(L, LUA_REGISTRYINDEX, CORE_COMMANDS);
  int commands_index = lua_gettop(L);

  if (lua_isnil(L, 1)) {
    lua_pushcfunction(L, predicate_always_true);
  } else if (lua_isstring(L, 1)) {
    lua_getglobal(L, "require");
    lua_pushvalue(L, 1);
    lua_call(L, 1, 1);
  } else if (lua_istable(L, 1)) {
    lua_pushvalue(L, 1);
    lua_pushcclosure(L, predicate_class_check, 1);
  } else if (!lua_isfunction(L, 1)) {
    return luaL_error(L, "invalid predicate type");
  } else {
    lua_pushvalue(L, 1);
  }
  
  int predicate_index = lua_gettop(L);
  
  lua_pushvalue(L, 2);
  lua_pushnil(L);
  while (lua_next(L, -2)) {
    const char* name = luaL_checkstring(L, -2);
    
    lua_newtable(L); // create command record
    
    lua_pushvalue(L, predicate_index);
    lua_setfield(L, -2, "predicate");
    
    lua_pushvalue(L, -2); // copy perform function/value
    lua_setfield(L, -2, "perform");
    
    lua_setfield(L, commands_index, name); // command.map[name] = record
    
    lua_pop(L, 1); // pop value for next iteration
  }
  
  return 0;
}

int f_executeCommand(lua_State *L) {
  lpcString_t cmd = luaL_checkstring(L, 1);
  
  // get CORE_COMMANDS[cmd]
  lua_getfield(L, LUA_REGISTRYINDEX, CORE_COMMANDS);
  lua_getfield(L, -1, cmd);
  if (!lua_istable(L, -1)) {
    lua_pop(L, 2);
    lua_pushboolean(L, 0);
    return 1;
  }
  
  // predicate
  lua_getfield(L, -1, "predicate");
  luaX_pushObject(L, core_GetFocus());
  if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
    Con_Error("%s", luaL_checkstring(L, -1));
    lua_pop(L, 3);
    lua_pushboolean(L, 0);
    return 1;
  }
  
  int ok = lua_toboolean(L, -1);
  lua_pop(L, 1);
  if (!ok) {
    lua_pop(L, 2);
    lua_pushboolean(L, 0);
    return 1;
  }
  
  // perform
  lua_getfield(L, -1, "perform");
  luaX_pushObject(L, core_GetFocus());
  if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
    Con_Error("%s", luaL_checkstring(L, -1));
    lua_pop(L, 2);
    lua_pushboolean(L, 0);
    return 1;
  }
  
  lua_pop(L, 2);
  lua_pushboolean(L, 1);
  return 1;
}

int f_addHotKeys(lua_State* L) {
  lua_getfield(L, LUA_REGISTRYINDEX, CORE_KEMAP);
  lua_pushvalue(L, 1);
  lua_pushnil(L);
  while (lua_next(L, -2)) {
    lua_getfield(L, -4, luaL_checkstring(L, -2));
    if (lua_isstring(L, -1)) {
      path_t buf={0};
      snprintf(buf, sizeof(buf), "%s;%s",
               luaL_checkstring(L, -1),
               luaL_checkstring(L, -2));
      lua_pop(L, 1);
      lua_pushstring(L, buf);
      lua_setfield(L, -5, luaL_checkstring(L, -3));
      lua_pop(L, 1);
    } else {
      lua_pop(L, 1);
      lua_setfield(L, -4, luaL_checkstring(L, -2));
    }
    
//    const char* key = luaL_checkstring(L, -2);
//    lua_getfield(L, -3, key);
//    if (!lua_istable(L, -1)) {
//      lua_pop(L, 1);
//      lua_newtable(L);
//      lua_pushvalue(L, -2);
//      lua_rawseti(L, -2, 1);
//      lua_setfield(L, -4, key);
//    } else {
//      lua_Integer len = lua_rawlen(L, -1);
//      lua_pushvalue(L, -2);
//      lua_rawseti(L, -2, len + 1);
//      lua_pop(L, 1);
//    }
//    lua_pop(L, 1);
  }
  return 0;
}

enum DataType luaX_checkDataType(lua_State *L, int idx);
int f_registerPropertyType(lua_State *L) {
  luaX_parsefield(lpcString_t, Name, 1, luaL_checkstring);
  luaX_parsefield(lpcString_t, Category, 1, luaL_optstring, "");
  luaX_parsefield(enum DataType, DataType, 1, luaX_checkDataType);
  fixedString_t tmp={0};
  if (*Category) {
    snprintf(tmp, sizeof(tmp), "%s.%s", Category, Name);
  } else {
    strncpy(tmp, Name, sizeof(tmp));
  }
  lpcString_t dot = strrchr(Name, '.');
  struct PropertyType type = {
    .ShortIdentifier = dot ? fnv1a32(dot + 1) : fnv1a32(Name),
    //    .ShortIdentifier = fnv1a32(type->Name),
    .FullIdentifier = fnv1a32(tmp),
    .DataType = DataType,
    .Name = Name,
    .Category =  Category,
  };
  switch (DataType) {
    case kDataTypeString: type.DataSize = sizeof(char*); break;
    case kDataTypeEvent:  type.DataSize = sizeof(void*); break;
    case kDataTypeObject: type.DataSize = sizeof(void*); break;
    case kDataTypeFloat:  type.DataSize = sizeof(float); break;
    case kDataTypeColor:  type.DataSize = sizeof(struct color); break;
    default:              type.DataSize = sizeof(int);   break;
  }
  OBJ_RegisterPropertyType(&type);
  return 0;
}

extern int
parse_property(const char* str,
               struct PropertyType const* prop,
               void* valueptr);

extern int
write_property(lua_State *L,
               struct PropertyType const* prop,
               void const* valueptr);

static int f_parse_property(lua_State* L) {
  const char* str = luaL_checkstring(L, 1);
  struct PropertyType const* pt = luaX_checkPropertyType(L, 2);
  char buf[MAX_PROPERTY_STRING] = {0};
  if (parse_property(str, pt, buf)) {
    write_property(L, pt, buf);
    return 1;
  } else {
    return 0;
  }
}

void core_AdvanceFrame(void) {
  core.realtime = axGetMilliseconds();
  core.frame++;
}

#define MAX_FPS_CACHE 64
static int _fps[MAX_FPS_CACHE]={0};
static int _counter=0;

LRESULT CORE_ProcessMessage(lua_State *L, struct AXmessage* e) {
  shortStr_t comp={0};
  if (e->wParam & AX_MOD_CTRL) strcat(comp, "ctrl+");
  if (e->wParam & AX_MOD_ALT) strcat(comp, "alt+");
  if (e->wParam & AX_MOD_SHIFT) strcat(comp, "shift+");
  if (e->wParam & AX_MOD_CMD) strcat(comp, "cmd+");
  strcat(comp, axKeynumToString(e->wParam));
  switch (e->message) {
    case kMsgPropertyChanged:
      PROP_FireNotification(L, (lpProperty_t)e->lParam, (lpObject_t)e->target);
      return FALSE;
    case kEventWindowPaint:
    case kEventWindowResized:
      _fps[_counter++%MAX_FPS_CACHE] = (int)(axGetMilliseconds() - core.realtime);
      core_AdvanceFrame();
      return FALSE;
    case kEventKeyDown:
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
            lua_pop(L, 1); // error message
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
      return FALSE;
  }
  return FALSE;
}

// --- Lua constructors for geometry structs ----------------------------------
// These provide ergonomic construction: EdgeShorthand(left, right),
// Thickness(v), Thickness(h, v), Thickness(l, r, t, b), etc.

static int f_Thickness_New(lua_State* L) {
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

static int f_EdgeShorthand_New(lua_State* L) {
  switch (lua_gettop(L)) {
    case 2:
      return (luaX_pushEdgeShorthand(L, &(struct EdgeShorthand){luaL_checknumber(L, 1), luaL_checknumber(L, 2)}), 1);
    case 1:
      return (luaX_pushEdgeShorthand(L, &(struct EdgeShorthand){luaL_checknumber(L, 1), luaL_checknumber(L, 1)}), 1);
    default:
      return 0;
  }
}

static int f_CornerRadius_New(lua_State* L) {
  switch (lua_gettop(L)) {
    case 4:
      return (luaX_pushCornerRadius(L, &(struct CornerRadius){luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4)}), 1);
    case 1:
      return (luaX_pushCornerRadius(L, &(struct CornerRadius){luaL_checknumber(L, 1), luaL_checknumber(L, 1), luaL_checknumber(L, 1), luaL_checknumber(L, 1)}), 1);
    default:
      return 0;
  }
}

// --- Pure-C struct string parsers (no Lua state required) -------------------

static int c_parse_EdgeShorthand(const char* str, void* dst, size_t sz) {
  if (!dst || sz != sizeof(struct EdgeShorthand)) return FALSE;
  float a = 0, b = 0;
  switch (sscanf(str, "%f %f", &a, &b)) {
    case 2: *(struct EdgeShorthand*)dst = (struct EdgeShorthand){a, b}; return TRUE;
    case 1: *(struct EdgeShorthand*)dst = (struct EdgeShorthand){a, a}; return TRUE;
    default: Con_Printf("EdgeShorthand: cannot parse '%s'", str); return FALSE;
  }
}

static int c_parse_Thickness(const char* str, void* dst, size_t sz) {
  if (!dst || sz != sizeof(struct Thickness)) return FALSE;
  float a = 0, b = 0, c = 0, d = 0;
  struct Thickness self = {0};
  switch (sscanf(str, "%f %f %f %f", &a, &b, &c, &d)) {
    case 4: self.Axis[0] = (struct EdgeShorthand){a, c}; self.Axis[1] = (struct EdgeShorthand){b, d}; break;
    case 3: self.Axis[0] = (struct EdgeShorthand){a, c}; self.Axis[1] = (struct EdgeShorthand){b, b}; break;
    case 2: self.Axis[0] = (struct EdgeShorthand){a, a}; self.Axis[1] = (struct EdgeShorthand){b, b}; break;
    case 1: self.Axis[0] = (struct EdgeShorthand){a, a}; self.Axis[1] = (struct EdgeShorthand){a, a}; break;
    default: Con_Printf("Thickness: cannot parse '%s'", str); return FALSE;
  }
  *(struct Thickness*)dst = self;
  return TRUE;
}

static int c_parse_CornerRadius(const char* str, void* dst, size_t sz) {
  if (!dst || sz != sizeof(struct CornerRadius)) return FALSE;
  float a = 0, b = 0, c = 0, d = 0;
  switch (sscanf(str, "%f %f %f %f", &a, &b, &c, &d)) {
    case 4: *(struct CornerRadius*)dst = (struct CornerRadius){a, b, c, d}; return TRUE;
    case 1: *(struct CornerRadius*)dst = (struct CornerRadius){a, a, a, a}; return TRUE;
    default: Con_Printf("CornerRadius: cannot parse '%s'", str); return FALSE;
  }
}

static int c_parse_vec2(const char* str, void* dst, size_t sz) {
  if (!dst || sz != sizeof(struct vec2)) return FALSE;
  float x = 0, y = 0;
  switch (sscanf(str, "%f %f", &x, &y)) {
    case 2: *(struct vec2*)dst = (struct vec2){x, y}; return TRUE;
    case 1: *(struct vec2*)dst = (struct vec2){x, x}; return TRUE;
    default: Con_Printf("vec2: cannot parse '%s'", str); return FALSE;
  }
}

static int c_parse_vec3(const char* str, void* dst, size_t sz) {
  if (!dst || sz != sizeof(struct vec3)) return FALSE;
  float x = 0, y = 0, z = 0;
  switch (sscanf(str, "%f %f %f", &x, &y, &z)) {
    case 3: *(struct vec3*)dst = (struct vec3){x, y, z}; return TRUE;
    case 1: *(struct vec3*)dst = (struct vec3){x, x, x}; return TRUE;  // broadcast scalar to all components
    default: Con_Printf("vec3: cannot parse '%s'", str); return FALSE;
  }
}

// --- Struct parser registry -------------------------------------------------

void
OBJ_RegisterFileLoader(const char* extension, struct Object* (*fn)(const char* path))
{
  FOR_LOOP(i, MAX_FILE_LOADERS) {
    if (core.file_loaders[i].extension &&
        !strcmp(core.file_loaders[i].extension, extension)) {
      core.file_loaders[i].fn = fn;   // update existing entry
      return;
    }
  }
  FOR_LOOP(i, MAX_FILE_LOADERS) {
    if (!core.file_loaders[i].extension) {
      core.file_loaders[i].extension = extension;
      core.file_loaders[i].fn = fn;
      return;
    }
  }
  Con_Error("No space to register file loader for '%s'", extension);
}

void
OBJ_RegisterStructParser(const char* type_name,
                          int (*fn)(const char* str, void* dst, size_t sz))
{
  FOR_LOOP(i, MAX_STRUCT_PARSERS) {
    if (core.struct_parsers[i].type_name &&
        !strcmp(core.struct_parsers[i].type_name, type_name)) {
      core.struct_parsers[i].fn = fn;   // update existing entry
      return;
    }
  }
  FOR_LOOP(i, MAX_STRUCT_PARSERS) {
    if (!core.struct_parsers[i].type_name) {
      core.struct_parsers[i].type_name = type_name;
      core.struct_parsers[i].fn = fn;
      return;
    }
  }
  Con_Error("No space to register struct parser for '%s'", type_name);
}

int
OBJ_ParseStruct(const char* type_name, const char* str, void* dst, size_t sz)
{
  FOR_LOOP(i, MAX_STRUCT_PARSERS) {
    struct struct_parser_entry const* e = &core.struct_parsers[i];
    if (e->type_name && !strcmp(e->type_name, type_name)) {
      return e->fn(str, dst, sz);
    }
  }
  return FALSE;
}

void
before_core_module_registered(lua_State* L)
{
  // Preserve registered classes: lua_pushclass calls OBJ_RegisterClass before
  // this function runs, so we must not zero core.classes[] here.
  lpcClassDesc_t saved_classes[MAX_CLASSES];
  memcpy(saved_classes, core.classes, sizeof(core.classes));
  memset(&core, 0, sizeof(struct game));
  memcpy(core.classes, saved_classes, sizeof(core.classes));
  core.realtime = axGetMilliseconds();
  core.L = L;
  //  lua_setfield(L, LUA_REGISTRYINDEX, IID_GAME);

  // Register pure-C struct parsers so parse_property works without a Lua state
  OBJ_RegisterStructParser("EdgeShorthand", c_parse_EdgeShorthand);
  OBJ_RegisterStructParser("Thickness",     c_parse_Thickness);
  OBJ_RegisterStructParser("CornerRadius",  c_parse_CornerRadius);
  OBJ_RegisterStructParser("vec2",          c_parse_vec2);
  OBJ_RegisterStructParser("vec3",          c_parse_vec3);
  
  void Init_KnownPrefabs(void);
  Init_KnownPrefabs();
  
  lua_pushcfunction(L, MakeLocalizedString);
  lua_setglobal(L, "L");
  
  SV_Init();
  SV_RegisterMessageProc(CORE_ProcessMessage);
  
  // Create commands table
  lua_newtable(L);
  lua_setfield(L, LUA_REGISTRYINDEX, CORE_COMMANDS);
  
  lua_newtable(L);
  lua_setfield(L, LUA_REGISTRYINDEX, CORE_KEMAP);
  
  lua_pushcfunction(L, f_addCommands);
  lua_setfield(L, -2, "addCommands");
  
  lua_pushcfunction(L, f_executeCommand);
  lua_setfield(L, -2, "executeCommand");
  
  lua_pushcfunction(L, f_addHotKeys);
  lua_setfield(L, -2, "addHotKeys");
  
  lua_pushcfunction(L, f_registerPropertyType);
  lua_setfield(L, -2, "registerPropertyType");
  
  lua_pushcfunction(L, f_parse_property);
  lua_setfield(L, -2, "parseProperty");
  
  luaX_require(L, "orca.geometry", 0);
}

extern lpObject_t static_stylesheet;
void core_AddGlobalStyleRule(lua_State* L, struct Object* rule) {
  if (!static_stylesheet) {
    luaX_require(L, "orca.core", 1);
    lua_getfield(L, -1, "StyleSheet");
    lua_call(L, 0, 1);
    static_stylesheet = luaX_checkObject(L, -1);
    lua_setglobal(L, "__STYLE");
    lua_pop(L, 1);
  }
  OBJ_AddChild(static_stylesheet, rule, FALSE);
}

// Drain all pending events from the platform queue, dispatching each one.
// This is used in headless tests to process kEventResumeCoroutine messages
// that run body() rebuild coroutines posted by rebuild() calls.
ORCA_API void core_FlushQueue(lua_State* L) {
  struct AXmessage msg;
  int top = lua_gettop(L);
  while (axPeekMessage(&msg)) {
    // Push a sentinel nil so that lua_pop(L,1) inside ui_handle_event
    // (kEventResumeCoroutine path) has something to consume.  After each
    // dispatch, restore the stack unconditionally to stay balanced.
    lua_pushnil(L);
    SV_DispatchMessage(L, &msg);
    lua_settop(L, top);
  }
}





void
after_core_module_registered(lua_State* L)
{
  int f_OBJ_newindex(lua_State* L);
  int f_object_gc(lua_State* L);
  int f_object_index(lua_State* L);

  // Override the default positional-args 'new' for struct types that have
  // special numeric-shorthand construction (e.g. Thickness(10) means all
  // sides, not just the first positional field).
#define OVERRIDE_NEW(NAME, New) \
  lua_getfield(L, -1, #NAME); \
  lua_pushcfunction(L, New); \
  lua_setfield(L, -2, "new"); \
  lua_pop(L, 1);

  OVERRIDE_NEW(Thickness,     f_Thickness_New)
  OVERRIDE_NEW(EdgeShorthand, f_EdgeShorthand_New)
  OVERRIDE_NEW(CornerRadius,  f_CornerRadius_New)

#undef OVERRIDE_NEW

  luaL_getmetatable(L, API_TYPE_OBJECT);
  lua_pushcfunction(L, f_object_gc);
  lua_setfield(L, -2, "__gc");
  lua_pushcfunction(L, f_OBJ_newindex);
  lua_setfield(L, -2, "__newindex");
  lua_pushcfunction(L, f_object_index);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);
}
