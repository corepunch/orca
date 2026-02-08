#include <include/api.h>

#include "core_local.h"

struct game core={0};

void SV_Init(void);
void SV_Shutdown(void);

ORCA_API int luaopen_orca_object(lua_State* L);
ORCA_API int luaopen_orca_network(lua_State* L);

HANDLER(PropertyType, Attached) {
  pPropertyType->_desc.id = (struct ID *)hObject;
  pPropertyType->_desc.FullIdentifier = OBJ_GetIdentifier(hObject);
  pPropertyType->_desc.Flags = pPropertyType->DataType;
  pPropertyType->_desc.TypeString = pPropertyType->TargetType;
//  if (OBJ_CheckName(hObject, "ActiveContext")) return 0;
  OBJ_RegisterPropertyType(&pPropertyType->_desc);
  return 0;
}

bool_t
OBJ_RegisterPropertyType(lpcPropertyDesc_t pt)
{
  FOR_LOOP(i, MAX_PROPERTY_TYPES) {
    if (!core.ptypes[i]) {
      core.ptypes[i] = pt;
      return TRUE;
    } else if (core.ptypes[i]->FullIdentifier == pt->FullIdentifier) {
      return TRUE;
    }
  }
  Con_Error("No space left to register property type %s", pt->id->Name);
  return FALSE;
}

lpcPropertyDesc_t
OBJ_FindPropertyType(uint32_t ident)
{
  FOR_LOOP(i, MAX_PROPERTY_TYPES) {
    lpcPropertyDesc_t pt = core.ptypes[i];
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
      return TRUE;
    }
  }
  Con_Error("No space left to register class %s", class->ClassName);
  return FALSE;
}

lpcClassDesc_t
OBJ_FindClass(lpcString_t classname)
{
  FOR_LOOP(i, MAX_CLASSES) {
    lpcClassDesc_t cl = core.classes[i];
    if (cl && !strcmp(cl->ClassName, classname)) {
      return cl;
    }
  }
  return NULL;
}

static bool_t
is_class_inherited(lpcClassDesc_t self, lpcClassDesc_t parent)
{
  if (!self) return FALSE;
  if (!parent || self == parent) return TRUE;
  for(lpcClassDesc_t const *cd = self->ParentClasses; *cd; cd++) {
    if (is_class_inherited(*cd, parent)) {
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

int API_HandleEvent(struct lua_State* L, struct WI_Message* e);
void OBJ_LoadPrefabs(lpObject_t, lua_State*);
void OBJ_ProcesEvents(lpObject_t, lua_State*);
void OBJ_Awake(lpObject_t, lua_State*);
void OBJ_Animate(lpObject_t, lua_State*);

#define MAX_FPS_CACHE 64
static int _fps[MAX_FPS_CACHE]={0};
static int _counter=0;

int _numbindings=0;

void
CORE_Update(struct lua_State* L,
            lpObject_t root,
            uint32_t winsize,
            longTime_t time)
{
  struct WI_Size windowSize = {LOWORD(winsize), HIWORD(winsize)};
  _fps[_counter++%MAX_FPS_CACHE] = (int)(time - core.realtime);
  _numbindings = 0;

  core.realtime = time;
  core.frame++;
  
  OBJ_Awake(root, L);
  OBJ_Animate(root, L);
  OBJ_LoadPrefabs(root, L);
  OBJ_ProcesEvents(root, L);
  OBJ_UpdateProperties(root);
  
  OBJ_SendMessageW(root, kEventUpdateLayout, 0, &(UPDATELAYOUTSTRUCT){
    .Width = windowSize.width,
    .Height = windowSize.height,
    .Force = FALSE,
  });
  
  OBJ_SendMessageW(root, kEventUpdateMatrix, 0, &(UPDATEMATRIXSTRUCT){
    .parent = NULL,
    .opacity = 1,
  });
  
  OBJ_SendMessageW(root, kEventRenderScreen, 0, &(RENDERSCREENSTRUCT) {
    .width = windowSize.width,
    .height = windowSize.height,
    .stereo = 0,
    .target = 0,
    .angle = 0,
  });
  
//  int tmp = 0;
//  FOR_LOOP(i, MAX_FPS_CACHE) { tmp += _fps[i]; }
//  void DEBUG_Draw(float fps, int bindings);
//  DEBUG_Draw(MAX_FPS_CACHE*1000.f/tmp, _numbindings);

  OBJ_ClearDirtyFlags(root);
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
//  luaX_import(L, "orca.core", "Behaviour");
  lua_getglobal(L, "BEHAVIOUR");
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

bool_t CORE_HandleCoroutineEvent(lua_State *L, struct WI_Message* msg) {
  if (msg->message == kEventCoroutineResume) {
    int handler_ref = msg->wParam;
    
    // Get the handler from registry using luaL_rawgeti
    lua_rawgeti(L, LUA_REGISTRYINDEX, handler_ref);
    
    if (lua_isfunction(L, -1)) {
      // Call the resume_handler function
      if (lua_pcall(L, 0, 1, 0) == LUA_OK) {
        bool_t should_continue = lua_toboolean(L, -1);
        lua_pop(L, 1);
        
        if (should_continue) {
          // Post another kEventCoroutineResume to continue
          WI_PostMessageW(msg->hobj, kEventCoroutineResume, handler_ref, NULL);
        } else {
          // Coroutine is done, clean up handler using luaL_unref
          luaL_unref(L, LUA_REGISTRYINDEX, handler_ref);
          
          // Post stop event
          WI_PostMessageW(msg->hobj, kEventCoroutineStop, handler_ref, NULL);
        }
      } else {
        Con_Error("Coroutine resume error: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
      }
    } else {
      lua_pop(L, 1); // pop non-function value
    }
    return TRUE;
  }
  
  if (msg->message == kEventCoroutineStop) {
    // Cleanup already done in kEventCoroutineResume handler
    return TRUE;
  }
  
  return FALSE;
}

int CORE_ProcessMessage(lua_State *L, struct WI_Message* msg) {
  switch (msg->message) {
    case kEventCoroutineResume:
    case kEventCoroutineStop:
      return CORE_HandleCoroutineEvent(L, msg);
    case kEventWindowPaint:
      CORE_Update(L, msg->hobj, msg->wParam, WI_GetMilliseconds());
//      WI_PostMessageW(msg->hobj, kEventWindowPaint, msg->wParam, NULL);
      break;
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
    default:
      return CORE_HandleObjectMessage(L, msg);
  }
  return FALSE;
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

eDataType_t luaX_checkDataType(lua_State *L, int idx);
int f_registerPropertyType(lua_State *L) {
  char n[MAX_PROPERTY_STRING];
  lpcString_t name = luaL_checkstring(L, 1);
  snprintf(n, sizeof(n), "property-%s", name);
  lua_getfield(L, LUA_REGISTRYINDEX, n);
  if (!lua_isnil(L, -1)) return 0;
  lua_pop(L, 1);
  luaX_parsefield(eDataType_t, type, 2, luaX_checkDataType);
  size_t size = sizeof(PropertyDesc_t)+sizeof(struct ID)+MAX_PROPERTY_STRING;
  lpPropertyDesc_t ds = lua_newuserdata(L, size);
  memset(ds, 0, size);
  ds->id = (void*)(ds+1);
  ds->id->Name = (void*)(ds->id+1);
  ds->id->Identifier = fnv1a32(name);
  ds->FullIdentifier = fnv1a32(name);
  ds->Flags = type;
  strncpy((char*)ds->id->Name, name, MAX_PROPERTY_STRING);
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
  
  int luaopen_orca_Object(lua_State*);
  luaopen_orca_Object(L);
  lua_setglobal(L, "OBJECT");
  
  int luaopen_orca_behaviour(lua_State*);
  luaopen_orca_behaviour(L);
  //  lua_setfield(L, -2, "Behaviour");
  lua_setglobal(L, "BEHAVIOUR");
  
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

