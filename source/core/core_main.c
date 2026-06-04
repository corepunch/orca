#include <include/api.h>
#include <strings.h>

#include "core_local.h"
#include "property/property_internal.h"

struct game core={0};

void SV_Init(void);
void SV_Shutdown(void);

ORCA_API int luaopen_orca_object(lua_State* L);
ORCA_API int luaopen_orca_network(lua_State* L);

static bool_t
CORE_FindFontFamilyIndex(lpcString_t name, int *index)
{
  if (!name || !*name) return FALSE;
  FOR_LOOP(i, MAX_FONT_FAMILIES) {
    if (core.fonts[i].name[0] && !strcasecmp(core.fonts[i].name, name)) {
      if (index) *index = i;
      return TRUE;
    }
  }
  return FALSE;
}

void
CORE_RegisterFontFamily(lpcString_t name, lpcString_t path)
{
  int index = -1;
  if (!name || !*name || !path || !*path) return;
  if (CORE_FindFontFamilyIndex(name, &index)) {
    snprintf(core.fonts[index].path, sizeof(core.fonts[index].path), "%s", path);
    return;
  }
  FOR_LOOP(i, MAX_FONT_FAMILIES) {
    if (!core.fonts[i].name[0]) {
      snprintf(core.fonts[i].name, sizeof(core.fonts[i].name), "%s", name);
      snprintf(core.fonts[i].path, sizeof(core.fonts[i].path), "%s", path);
      return;
    }
  }
  Con_Error("No space left to register font family '%s'", name);
}

lpcString_t
CORE_FindFontFamily(lpcString_t name)
{
  int index = -1;
  return CORE_FindFontFamilyIndex(name, &index) ? core.fonts[index].path : NULL;
}

bool_t
OBJ_RegisterPropertyType(struct PropertyType const *pt)
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

struct PropertyType const *
OBJ_FindPropertyType(uint32_t ident)
{
  FOR_LOOP(i, MAX_PROPERTY_TYPES) {
    struct PropertyType const *pt = &core.ptypes[i];
    if (pt && pt->FullIdentifier == ident) {
      return pt;
    }
  }
  return NULL;
}

bool_t
OBJ_RegisterPropertyShorthand(struct PropertyShorthand const *sh)
{
  FOR_LOOP(i, MAX_PROPERTY_SHORTHANDS) {
    if (!core.shorthands[i].Name) {
      memcpy(&core.shorthands[i], sh, sizeof(struct PropertyShorthand));
      return TRUE;
    } else if (core.shorthands[i].FullIdentifier == sh->FullIdentifier) {
      return TRUE;
    }
  }
  Con_Error("No space left to register property shorthand %s.%s",
            sh->Category ? sh->Category : "",
            sh->Name ? sh->Name : "");
  return FALSE;
}

struct PropertyShorthand const *
OBJ_FindPropertyShorthand(uint32_t ident)
{
  FOR_LOOP(i, MAX_PROPERTY_SHORTHANDS) {
    struct PropertyShorthand const *sh = &core.shorthands[i];
    if (sh->Name && sh->FullIdentifier == ident) {
      return sh;
    }
  }
  return NULL;
}

bool_t
OBJ_RegisterClass(struct ClassDesc const *class)
{
  FOR_LOOP(i, MAX_CLASSES) {
    if (!core.classes[i]) {
      FOR_LOOP(j, class->NumProperties) {
        OBJ_RegisterPropertyType(&class->Properties[j]);
      }
      FOR_LOOP(j, class->NumShorthands) {
        OBJ_RegisterPropertyShorthand(&class->Shorthands[j]);
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

bool_t
OBJ_RegisterStructDesc(struct StructDesc *desc)
{
  FOR_LOOP(i, MAX_STRUCTS) {
    if (!core.structs[i]) {
      FOR_LOOP(j, MAX_STRUCT_PARSERS) {
        if (core.struct_parsers[j].type_name &&
            !strcmp(core.struct_parsers[j].type_name, desc->StructName)) {
          desc->Parser = core.struct_parsers[j].fn;
          break;
        }
      }
      core.structs[i] = desc;
      return TRUE;
    } else if (!strcmp(core.structs[i]->StructName, desc->StructName)) {
      return TRUE;
    }
  }
  Con_Error("No space left to register struct %s", desc->StructName);
  return FALSE;
}

struct StructDesc const *
OBJ_FindStructDesc(lpcString_t name)
{
  FOR_LOOP(i, MAX_STRUCTS) {
    struct StructDesc const *desc = core.structs[i];
    if (desc && !strcmp(desc->StructName, name)) {
      return desc;
    }
  }
  return NULL;
}

struct ClassDesc const *
OBJ_FindClassW(uint32_t class_id)
{
  FOR_LOOP(i, MAX_CLASSES) {
    struct ClassDesc const *cl = core.classes[i];
    if (cl && cl->ClassID == class_id) {
      return cl;
    }
  }
  return NULL;
}

struct ClassDesc const *
OBJ_FindClass(lpcString_t classname)
{
  return OBJ_FindClassW(fnv1a32(classname));
}

static bool_t
is_class_inherited(struct ClassDesc const *self, uint32_t parent)
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
OBJ_EnumClasses(uint32_t superclass, void (*fnProc)(struct ClassDesc const *, void*), void* param)
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
  //    struct Object * hRoot   = server->rootobj;
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

static int class_table_call(lua_State* L) {
  if (lua_type(L, 1) != LUA_TTABLE) {
    return luaL_error(L, "invalid class table");
  }
  return OBJ_CreateFromLuaState(L);
}

int lua_pushclass(lua_State* L, struct ClassDesc* cl)
{
  OBJ_RegisterClass(cl);

  /* Expose each C class as a callable table. */
  lua_newtable(L);
  lua_pushlightuserdata(L, cl);
  lua_setfield(L, -2, "__nativeclass");

  /* Add short-name -> full-name aliases (e.g. LeftButtonDown -> Node.LeftButtonDown)
   * so Lua can use self:send(Node.LeftButtonDown) transparently. */
  FOR_LOOP(i, cl->NumProperties) {
    struct PropertyType const *pt = &cl->Properties[i];
    if (!pt->Name || !*pt->Name) {
      continue;
    }
    fixedString_t full = {0};
    if (pt->Category && *pt->Category) {
      snprintf(full, sizeof(full), "%s.%s", pt->Category, pt->Name);
    } else {
      strncpy(full, pt->Name, sizeof(full) - 1);
    }
    lua_pushstring(L, full);
    lua_setfield(L, -2, pt->Name);
  }

  lua_newtable(L);
  lua_pushcfunction(L, class_table_call);
  lua_setfield(L, -2, "__call");
  lua_setmetatable(L, -2);

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
  switch (e->message) {
    case kEventWindowPaint:
    case kEventWindowResized:
      _fps[_counter++%MAX_FPS_CACHE] = (int)(axGetMilliseconds() - core.realtime);
      core_AdvanceFrame();
      return FALSE;
    case ID_PropertyChangedMessage: {
      struct Object *object = e->target;
      struct Property *property = e->lParam;
      if (!property && object && e->wParam) {
        property = PROP_FindByShortID(OBJ_GetProperties(object), e->wParam);
      }
      if (!property) {
        return FALSE;
      }
      PROP_FireNotification(L, property, property->object ? property->object : object);
      return TRUE;
    }
    case kEventResumeCoroutine: {
      int tmp;
      switch (lua_resume(e->target, L, LOWORD(e->wParam), &tmp)) {
        case LUA_OK:
          if (tmp > 0) lua_pop(e->target, tmp);
          axPostMessageW(e->target, kEventStopCoroutine, e->wParam, NULL);
          break;
        case LUA_YIELD:
          if (tmp > 0) lua_pop(e->target, tmp);
          axPostMessageW(e->target, kEventResumeCoroutine, MAKEDWORD(0, HIWORD(e->wParam)), NULL);
          break;
        default:
          axPostMessageW(e->target, kEventStopCoroutine, e->wParam, NULL);
          if (!lua_isnil(e->target, -1)) {
            lpcString_t err = lua_tostring(e->target, -1);
            if (err) fprintf(stderr, "co.resume(): %s\n", err);
          }
          lua_pop(e->target, 1);
          break;
      }
      /* Return FALSE so kEventStopCoroutine (posted above) can still trigger
         a repaint via axPostMessageW; returning TRUE would short-circuit
         SV_DispatchMessage and the Lua event loop would issue a redundant
         second Window.Paint via its `result` check. */
      return FALSE;
    }
    case kEventStopCoroutine:
      luaL_unref(L, LUA_REGISTRYINDEX, HIWORD(e->wParam));
      axRemoveFromQueue(e->target);
      axPostMessageW(NULL, kEventWindowPaint, axGetSize(NULL), 0);
      /* Return FALSE: the coroutine is fully cleaned up; the Window.Paint
         posted above will reach CORE_HandleObjectMessage on its own pass. */
      return FALSE;
    case kEventKeyDown:
    {
      shortStr_t comp={0};
      if (e->wParam & AX_MOD_CTRL) strcat(comp, "ctrl+");
      if (e->wParam & AX_MOD_ALT) strcat(comp, "alt+");
      if (e->wParam & AX_MOD_SHIFT) strcat(comp, "shift+");
      if (e->wParam & AX_MOD_CMD) strcat(comp, "cmd+");
      strcat(comp, axKeynumToString(e->wParam));
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

// --- Struct parser registry -------------------------------------------------

void
OBJ_RegisterFileLoader(const char* extension, struct Object* (*fn)(int argc, const char* argv[]))
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
                          structParserFn_t fn)
{
  FOR_LOOP(i, MAX_STRUCTS) {
    struct StructDesc *desc = core.structs[i];
    if (desc && !strcmp(desc->StructName, type_name)) {
      desc->Parser = fn;
      return;
    }
  }
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

void
OBJ_SetStructParseMask(uint64_t mask)
{
  core.struct_parse_mask = mask;
  core.struct_parse_mask_valid = TRUE;
}

uint64_t
OBJ_GetStructParseMask(bool_t *valid)
{
  if (valid) *valid = core.struct_parse_mask_valid;
  return core.struct_parse_mask;
}

int
OBJ_ParseStruct(const char* type_name, const char* str, void* dst, size_t sz)
{
  int parsed = FALSE;
  core.struct_parse_mask = 0;
  core.struct_parse_mask_valid = FALSE;
  struct StructDesc const *desc = OBJ_FindStructDesc(type_name);
  if (desc && desc->Parser) {
    parsed = desc->Parser(str, dst, sz);
    if (!parsed) core.struct_parse_mask_valid = FALSE;
    return parsed;
  }
  FOR_LOOP(i, MAX_STRUCT_PARSERS) {
    struct struct_parser_entry const* e = &core.struct_parsers[i];
    if (e->type_name && !strcmp(e->type_name, type_name)) {
      parsed = e->fn(str, dst, sz);
      if (!parsed) core.struct_parse_mask_valid = FALSE;
      return parsed;
    }
  }
  return FALSE;
}

void
before_core_module_registered(lua_State* L)
{
  // Preserve registered classes: lua_pushclass calls OBJ_RegisterClass before
  // this function runs, so we must not zero core.classes[] here.
  struct ClassDesc const *saved_classes[MAX_CLASSES];
  struct file_loader saved_file_loaders[MAX_FILE_LOADERS];
  struct font_registry_entry saved_fonts[MAX_FONT_FAMILIES];
  memcpy(saved_classes, core.classes, sizeof(core.classes));
  memcpy(saved_file_loaders, core.file_loaders, sizeof(core.file_loaders));
  memcpy(saved_fonts, core.fonts, sizeof(core.fonts));
  memset(&core, 0, sizeof(struct game));
  memcpy(core.classes, saved_classes, sizeof(core.classes));
  memcpy(core.file_loaders, saved_file_loaders, sizeof(core.file_loaders));
  memcpy(core.fonts, saved_fonts, sizeof(core.fonts));
  core.realtime = axGetMilliseconds();
  //  lua_setfield(L, LUA_REGISTRYINDEX, IID_GAME);

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

extern struct Object *static_stylesheet;
void core_AddGlobalStyleRule(lua_State* L, struct Object* rule) {
  if (!static_stylesheet) {
    luaX_require(L, "orca.core", 1);
    lua_getfield(L, -1, "StyleSheet");
    lua_call(L, 0, 1);
    static_stylesheet = luaX_checkObject(L, -1);
    lua_setglobal(L, "__STYLE");
    lua_pop(L, 1);
  }
  OBJ_AddChild(static_stylesheet, rule);
}

ORCA_API void core_FlushQueue(lua_State* L) {
  struct AXmessage msg;
  int top = lua_gettop(L);
  while (axPeekMessage(&msg)) {
    SV_DispatchMessage(L, &msg);
    lua_settop(L, top);
  }
}

ORCA_API int core_GetObjectCount(lua_State* L) {
  lua_pushinteger(L, OBJ_GetObjectCount());
  return 1;
}





void
after_core_module_registered(lua_State* L)
{
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
}

#include <core/core_export.c>
