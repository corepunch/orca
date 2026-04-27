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
  (*(struct Property **)OBJ_GetObjectComponent(obj, kCompProperties))

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

#define ID_PropertyChangedMessage 0xdffd83eb // PropertyChangedMessage

// C-level string-to-struct parsers registered without a Lua state.
struct struct_parser_entry {
  const char* type_name;
  int (*fn)(const char* str, void* dst, size_t size);
};

struct file_loader {
  const char* extension;
  struct Object* (*fn)(int argc, const char* argv[]);
};

struct property_program;

struct game
{
  lua_State *L;
  longTime_t realtime;
  uint32_t frame;
  struct Object *focus;
  struct Object *hover;
  struct Object *hover2;
  fixedString_t tags[MAX_TAGS];
  struct ClassDesc const *classes[MAX_CLASSES];
  struct PropertyType ptypes[MAX_PROPERTY_TYPES];
  struct struct_parser_entry struct_parsers[MAX_STRUCT_PARSERS];
  struct file_loader file_loaders[MAX_FILE_LOADERS];
  struct property_program* programs;
};

// stateman.c

void
API_PrintStackTrace(lua_State* L);

void
OBJ_Clear(struct Object *);

struct Object *
OBJ_FindImmediateChild(struct Object *object, uint32_t ident);

uint32_t
OBJ_GetUniqueID(struct Object const *);

int64_t
OBJ_GetObjectCount(void);


struct Object *
OBJ_FindKnownPrefab(lpcString_t szFileName, lpcString_t* ppRemaining);


void
OBJ_EnumStyleClasses(struct Object *, lpcString_t, struct style_class_selector*);

uint32_t
OBJ_GetStyleFlags(struct Object *);

bool_t
OBJ_RunProgram(struct Object *, struct token*, struct vm_register*);

void
OBJ_MoveToFront(struct Object *);

void
OBJ_ClearStyleClasses(struct Object *);

void
OBJ_RequestAnimate(struct Object *);
  
ORCA_API void
OBJ_SetAlias(struct Object *, uint32_t);


struct timer;
void
OBJ_AddTimer(struct Object *, struct timer*);

struct Property *
_CreateClassProperty(struct Object *, uint32_t);

HRESULT
_RegisterProperty(struct Object *, struct Property *);

INLINE
bool_t
OBJ_NameEquals(struct Object *object, lpcString_t name)
{
  return !strcmp(OBJ_GetName(object), name);
}

struct Property *
OBJ_AddComponentProperty(lua_State*, struct component*, struct PropertyType const *);
//struct Property *
//OBJ_AddComponentProperty2(lua_State*, struct component*, lpcString_t);
lpcString_t
CMP_GetClassName(struct component*);
struct Object *
CMP_GetOwner(struct component*);
bool_t
CMP_SetProperty(struct component*, struct Property *);
uint32_t
PROP_GetShortID(struct Property const *);
bool_t
PROP_Import(struct Property *, enum PropertyAttribute, struct vm_register*);
void
PROP_SetTypeSize(struct Property *, eDataType_t, uint32_t);
struct Property *
PROP_AddToList(struct Property *, struct Property **);
void
PROP_SetValuePtr(struct Property *, void*);
struct Object *
PROP_GetObject(struct Property const *);
bool_t 
PROP_RegisterChangedCallback(lua_State*, struct Property *, int);
void
OBJ_ReleaseComponents(struct Object *);
void
OBJ_ReleaseProperties(struct Object *);
void
CMP_Detach(void* userdata);

void
UI_ProcessCommands(struct lua_State* L, struct Object *root);

ORCA_API void
PROP_AttachProgram(struct Property *,
                   enum PropertyAttribute,
                   struct token* program,
                   lpcString_t source);

extern struct game core;

#endif /* n_utils_h */
