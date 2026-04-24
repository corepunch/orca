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

#define _GetProperties(obj)                                                    \
  (*(lpProperty_t*)OBJ_GetObjectComponent(obj, kCompProperties))

// StyleController component accessor — returns NULL if component not attached
#define _GetStyleController(obj)                                               \
  GetStyleController(obj)

#define MAX_NODE_PACKAGES 16

#define STYLE_HOVER 1
#define STYLE_FOCUS 2
#define STYLE_DARK 4
#define STYLE_SELECT 8

struct lua_State;
struct alias;

// Parsed style class selector (e.g., "button:hover/50")
struct style_class_selector
{
  struct style_class_selector* next;
  shortStr_t value;    // base class name (without pseudo-states or opacity suffix)
  byte_t flags;        // STYLE_HOVER / STYLE_FOCUS / STYLE_DARK / STYLE_SELECT bits
  byte_t opacity;      // 0–100 percentage (default 100); sourced from "/N" syntax
};

struct vm_register
{
  eDataType_t type;
  uint32_t size;
  float value[MAX_PROPERTY_STRING / sizeof(float)];
};

#define MAX_CLASSES 256
#define MAX_PROPERTY_TYPES 1024
#define MAX_STRUCT_PARSERS 256
#define MAX_FILE_LOADERS 256

// C-level string-to-struct parsers registered without a Lua state.
struct struct_parser_entry {
  const char* type_name;
  int (*fn)(const char* str, void* dst, size_t size);
};

struct file_loader {
  const char* extension;
  struct Object* (*fn)(const char* path);
};

struct game
{
  lua_State *L;
  longTime_t realtime;
  uint32_t frame;
  lpObject_t focus;
  lpObject_t hover;
  lpObject_t hover2;
  fixedString_t tags[MAX_TAGS];
  lpcClassDesc_t classes[MAX_CLASSES];
  struct PropertyType ptypes[MAX_PROPERTY_TYPES];
  struct struct_parser_entry struct_parsers[MAX_STRUCT_PARSERS];
  struct file_loader file_loaders[MAX_FILE_LOADERS];
};

// stateman.c

void
API_PrintStackTrace(lua_State* L);

ORCA_API lpObject_t
OBJ_Create(lua_State*, lpcClassDesc_t cdesc);

lpObject_t
OBJ_MakeNativeObject(uint32_t class_id);

void
OBJ_Clear(lpObject_t);

lpObject_t
OBJ_FindImmediateChild(lpObject_t object, uint32_t ident);

uint32_t
OBJ_GetUniqueID(lpcObject_t);

lpObject_t
OBJ_FindKnownPrefab(lpcString_t szFileName, lpcString_t* ppRemaining);

void
OBJ_UpdateProperties(lpObject_t);

void
OBJ_EnumStyleClasses(lpObject_t, lpcString_t, struct style_class_selector*);

uint32_t
OBJ_GetStyleFlags(lpObject_t);

bool_t
OBJ_RunProgram(lpObject_t, struct token*, struct vm_register*);

void
OBJ_MoveToFront(lpObject_t);

void
OBJ_ClearStyleClasses(lpObject_t);

void
OBJ_RequestAnimate(lpObject_t);
  
ORCA_API void
OBJ_SetAlias(lpObject_t, uint32_t);


struct timer;
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

lpProperty_t
OBJ_AddComponentProperty(lua_State*, struct component*, lpcPropertyType_t);
//lpProperty_t
//OBJ_AddComponentProperty2(lua_State*, struct component*, lpcString_t);
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
bool_t
PROP_Import(lpProperty_t, enum PropertyAttribute, struct vm_register*);
void
PROP_SetTypeSize(lpProperty_t, eDataType_t, uint32_t);
lpProperty_t
PROP_AddToList(lpProperty_t, lpProperty_t*);
void
PROP_SetValuePtr(lpProperty_t, void*);
lpObject_t
PROP_GetObject(lpcProperty_t);
bool_t 
PROP_RegisterChangedCallback(lua_State*, lpProperty_t, int);
void
OBJ_ReleaseComponents(lpObject_t);
void
OBJ_ReleaseProperties(lpObject_t);
void
CMP_Detach(void* userdata);

void
UI_ProcessCommands(struct lua_State* L, lpObject_t root);

ORCA_API void
PROP_AttachProgram(lpProperty_t,
                   enum PropertyAttribute,
                   struct token* program,
                   lpcString_t source);

extern struct game core;

#endif /* n_utils_h */
