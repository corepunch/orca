#include <include/api.h>

#include "core_local.h"

struct game core={0};

void SV_Init(void);
void SV_Shutdown(void);

ORCA_API int luaopen_orca_object(lua_State* L);
ORCA_API int luaopen_orca_network(lua_State* L);

bool_t
OBJ_RegisterPropertyType(lpcPropertyType_t pt)
{
  FOR_LOOP(i, MAX_PROPERTY_TYPES) {
    if (!core.ptypes[i]) {
      core.ptypes[i] = pt;
      return TRUE;
    } else if (core.ptypes[i]->FullIdentifier == pt->FullIdentifier) {
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
    lpcPropertyType_t pt = core.ptypes[i];
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
    if (cl->ClassID == class_id) {
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
is_class_inherited(lpcClassDesc_t self, lpcClassDesc_t parent)
{
  if (!self) return FALSE;
  if (!parent || self == parent) return TRUE;
  for(uint32_t const *cd = self->ParentClasses; *cd; cd++) {
    if (is_class_inherited(OBJ_FindClassW(*cd), parent)) {
      return TRUE;
    }
  }
  return FALSE;
}

void
OBJ_EnumClasses(lpcClassDesc_t p, void (*fnProc)(lpcClassDesc_t, void*), void* param)
{
  FOR_LOOP(i, MAX_CLASSES) {
    if (is_class_inherited(core.classes[i], p)) {
      fnProc(core.classes[i], param);
    }
  }
}

void
OBJ_EnumClassesW(uint32_t classID,
                 void (*fnProc)(lpcClassDesc_t, void*),
                 void* param)
{
  FOR_LOOP(i, MAX_CLASSES) {
    lpcClassDesc_t cl = core.classes[i];
    if (!cl) continue;
    for (uint32_t const *cd = cl->ParentClasses; *cd; cd++) {
      if (*cd == classID) {
        fnProc(cl, param);
        break;
      }
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

#define MAX_FPS_CACHE 64
static int _fps[MAX_FPS_CACHE]={0};
static int _counter=0;

void OBJ_UpdateLayout(lpObject_t obj, int width, int height) {
  OBJ_SendMessage(obj, "Measure", 0, &(struct Size){ .width = width, .height = height });
  OBJ_SendMessage(obj, "Arrange", 0, &(struct rect){ .width = width, .height = height });
}

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
      SV_PostMessage(core.hover, "ThemeChanged", 0, 0);
    }
    if (core.hover2) {
      OBJ_SendMessage(core.hover2, "SetHover", 0, core.hover);
      SV_PostMessage(core.hover2, "ThemeChanged", 0, 0);
    }
  }
  core.hover = core.hover2;
  core.hover2 = NULL;
}

int f_poll(lua_State* L);

int lua_pushclass(lua_State* L, struct ClassDesc* cl)
{
  lua_getglobal(L, "require");
  lua_pushstring(L, "orca");
  lua_call(L, 1, 1);
  lua_getfield(L, -1, "Behaviour");
  lua_remove(L, -2); // remove orca
  lua_getfield(L, -1, "extend");
  lua_pushvalue(L, -2);
  // Create the args table
  lua_newtable(L);
  lua_pushlightuserdata(L, cl);
  lua_setfield(L, -2, "__nativeclass");
  lua_pcall(L, 2, 1, 0);
  lua_remove(L, -2); // pop Behaviour
  // Register class globally
  // lua_pushlightuserdata(L, cl);
  OBJ_RegisterClass(cl);
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, cl->ClassName);
  return 1;
}

bool_t CORE_HandleObjectMessage(lua_State *L, struct WI_Message* msg);
bool_t CORE_HandleKeyEvent(lua_State *L, struct WI_Message* msg);

int CORE_ProcessMessage(lua_State *L, struct WI_Message* msg) {
  int tmp=0;
  switch (msg->message) {
    case kEventWindowPaint:
    case kEventWindowResized:
      _fps[_counter++%MAX_FPS_CACHE] = (int)(WI_GetMilliseconds() - core.realtime);
      core.realtime = WI_GetMilliseconds();
      core.frame++;
      if (CORE_HandleObjectMessage(L, msg)) {
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
      return FALSE;
    case kEventKeyDown:
    case kEventKeyUp:
      return CORE_HandleKeyEvent(L, msg);
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
  char n[MAX_PROPERTY_STRING];
  lpcString_t name = luaL_checkstring(L, 1);
  snprintf(n, sizeof(n), "property-%s", name);
  lua_getfield(L, LUA_REGISTRYINDEX, n);
  if (!lua_isnil(L, -1)) return 0;
  lua_pop(L, 1);
  luaX_parsefield(enum DataType, type, 2, luaX_checkDataType);
  size_t size = sizeof(PropertyType_t)+MAX_PROPERTY_STRING;
  lpPropertyType_t ds = lua_newuserdata(L, size);
  memset(ds, 0, size);
  strncpy(ds->Name, name, MAX_PROPERTY_STRING);
  ds->ShortIdentifier = fnv1a32(name);
  ds->FullIdentifier = fnv1a32(name);
  ds->DataType = type;
  lua_setfield(L, LUA_REGISTRYINDEX, n);
  OBJ_RegisterPropertyType(ds);
  return 0;
}

int f_registerPropertyTypes(lua_State *L) {
  lua_pushvalue(L, 1);
  lua_pushnil(L);
  while (lua_next(L, -2)) {
    lua_pushcfunction(L, f_registerPropertyType);
    lua_pushvalue(L, -3);
    lua_pushvalue(L, -3);
    lua_call(L, 2, 0);
    lua_pop(L, 1);
  }
  return 0;
}

void
on_core_module_registered(lua_State* L)
{
  memset(&core, 0, sizeof(struct game));
  core.realtime = WI_GetMilliseconds();
  core.L = L;
  //  lua_setfield(L, LUA_REGISTRYINDEX, IID_GAME);
  
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
  
  lua_pushcfunction(L, f_registerPropertyTypes);
  lua_setfield(L, -2, "registerPropertyTypes");
}

