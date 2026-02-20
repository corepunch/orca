#ifndef __m_local_H__
#define __m_local_H__

#include <include/api.h>
#include <include/renderer.h>

// TODO: Move to orca.h
#include <source/core/core.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#define CORE_COMMANDS "__commands__"
#define CORE_KEMAP "__keymap__"

#define OBJ_API(NAME, ...) OBJ_##NAME(lua_State *L, lpObject_t self, ##__VA_ARGS__)

#define EDITOR_LIB

#define PARENT_ELEMENT 1
#define CALLBACK_OBJECT 1
#define CALLBACK_FUNCTION 2

#define CSS_APPLY 0x5102ed51 // @apply

#define lua_safecall(L, funcname, args, ret, f)                                \
  if (lua_pcall(L, args, ret, f) != LUA_OK) {                                  \
    Con_Error("Lua API (%s): %s", funcname, lua_tostring(L, -1));        \
    lua_pop(L, 1);                                                             \
  }

#define _GetComponents(obj)                                                    \
  (*(struct component**)OBJ_GetObjectComponent(obj, kCompComponents))

#define _GetAnimations(obj)                                                    \
  (*(struct property_animation**)OBJ_GetObjectComponent(obj, kCompAnimations))

#define _GetProperties(obj)                                                    \
  (*(lpProperty_t*)OBJ_GetObjectComponent(obj, kCompProperties))

#define _GetStateManager(obj)                                                  \
  (*(struct state_manager**)OBJ_GetObjectComponent(obj, kCompStateManager))

#define _GetClasses(obj)                                                       \
  (*(struct style_class**)OBJ_GetObjectComponent(obj, kCompClasses))

#define _GetStyles(obj)                                                        \
  (*(struct style_sheet**)OBJ_GetObjectComponent(obj, kCompStyles))

#define _GetTimers(obj)                                                        \
  (*(struct timer**)OBJ_GetObjectComponent(obj, kCompTimers))

#define _GetAliases(obj)                                                       \
  (*(struct alias**)OBJ_GetObjectComponent(obj, kCompAliases))

#define _GetCallbacks(obj)                                                     \
  (*(struct script_callback**)OBJ_GetObjectComponent(obj, kCompCallbacks))

#define MAX_NODE_PACKAGES 16

#define STYLE_HOVER 1
#define STYLE_FOCUS 2
#define STYLE_DARK 4
#define STYLE_SELECT 8

struct lua_State;
struct style_sheet;
struct style_class;
struct state_manager;
struct property_animation;
struct timer;
struct alias;
struct script_callback;

enum component_type
{
  kCompComponents,
  kCompAnimations,
  kCompProperties,
  kCompStateManager,
  kCompClasses,
  kCompStyles,
  kCompTimers,
  kCompAliases,
  kCompCallbacks,
  kCompCount,
};

struct vm_register
{
  eDataType_t type;
  uint32_t size;
  float value[MAX_PROPERTY_STRING / sizeof(float)];
};

#define MAX_CLASSES 256
#define MAX_PROPERTY_TYPES 4096

struct game
{
  lua_State *L;
  longTime_t realtime;
  uint32_t frame;
  lpObject_t focus;
  lpObject_t hover;
  lpObject_t hover2;
  lpcClassDesc_t classes[MAX_CLASSES];
  lpcPropertyDesc_t ptypes[MAX_PROPERTY_TYPES];
};

// stateman.c

void
API_PrintStackTrace(lua_State* L);

typedef void (*STYLEPROC)(lpObject_t, struct style_sheet*, void*);

lpObject_t
OBJ_Create(lua_State*, lpcClassDesc_t cdesc);

void
OBJ_Clear(lua_State*, lpObject_t);

lpObject_t
OBJ_FindImmediateChild(lpObject_t object, uint32_t ident);

uint32_t
OBJ_GetUniqueID(lpcObject_t);

lpObject_t
OBJ_FindKnownPrefab(lpcString_t szFileName, lpcString_t* ppRemaining);

void
OBJ_UpdateProperties(lpObject_t);

void
OBJ_ClearDirtyFlags(lpObject_t);

void
OBJ_EnumStyleClasses(lpObject_t, lpcString_t, STYLEPROC, void*);

uint32_t
OBJ_GetStyleFlags(lpObject_t);

void
OBJ_ApplyClass(lpObject_t, struct style_sheet*, void*);

bool_t
OBJ_RunProgram(lpObject_t, struct token*, struct vm_register*);

void
OBJ_MoveToFront(lpObject_t);

void
OBJ_ClearStyleClasses(lpObject_t);

void
OBJ_ProcessFunctions(lpObject_t, lpcString_t);

void
OBJ_SetLuaObject(lpObject_t , uint32_t);
  
void
OBJ_SetAlias(lpObject_t , uint32_t);

void
OBJ_AddTimer(lpObject_t, struct timer*);

lpProperty_t
_CreateClassProperty(lpObject_t, uint32_t);

HRESULT
_RegisterProperty(lpObject_t, lpProperty_t);

INLINE
bool_t
OBJ_NameEquals(lpObject_t object, lpcString_t name)
{
  return !strcmp(OBJ_GetName(object), name);
}

struct component*
CMP_Create(lpObject_t, lpcClassDesc_t);
lpProperty_t
CMP_CreateProperty(lua_State*, struct component*, lpcPropertyDesc_t);
//lpProperty_t
//CMP_CreateProperty2(lua_State*, struct component*, lpcString_t);
lpcString_t
CMP_GetClassName(struct component*);
lpObject_t
CMP_GetOwner(struct component*);
bool_t
CMP_SetProperty(struct component*, lpProperty_t);
uint32_t
PROP_GetShortID(lpcProperty_t);
void
PROP_ProcessEvents(lua_State*, lpProperty_t, lpObject_t);
void
PROP_ExecuteChangedCallback(lua_State*, lpObject_t, lpProperty_t);
lpcString_t
PROP_GetCallbackEvent(lpcProperty_t);
lpcString_t
PROP_GetShortName(lpcProperty_t);
bool_t
PROP_Import(lpProperty_t, enum property_attribute, struct vm_register*);
void
PROP_SetTypeSize(lpProperty_t, eDataType_t, uint32_t);
lpProperty_t
PROP_AddToList(lpProperty_t, lpProperty_t*);
void
PROP_SetValuePtr(lpProperty_t, void*);
lpObject_t
PROP_GetObject(lpcProperty_t);

void
SM_ReleaseStateManager(lpObject_t);
void
SM_EnsureStateManagerInitialized(lpObject_t);
void
SM_HandleControllerChange(lpObject_t, lpProperty_t);

void*
OBJ_GetObjectComponent(lpObject_t, enum component_type);
void
OBJ_ReleaseAnimations(lpObject_t);
void
OBJ_UpdateTimers(lpObject_t);
void
OBJ_ReleaseTimers(lpObject_t);
void
OBJ_ReleaseComponents(lpObject_t);
void
OBJ_ReleaseProperties(lpObject_t);
void
OBJ_ReleaseAliases(lpObject_t);

lpObject_t
OBJ_FindChildByAlias(lpObject_t, uint32_t);

void
OBJ_AddClass(lpObject_t, lpcString_t);

void
UI_ProcessCommands(struct lua_State* L, lpObject_t root);

extern struct game core;

#endif /* n_utils_h */
