#ifndef __m_local_H__
#define __m_local_H__

#include <include/api.h>
#include <include/renderer.h>

// TODO: Move to orca.h
#include <core/core.h>

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
#define MAX_PROPERTY_SHORTHANDS 1024
#define MAX_STRUCTS 256
#define MAX_STRUCT_PARSERS 256
#define MAX_FILE_LOADERS 256
#define MAX_FONT_FAMILIES 512

#define ID_PropertyChangedMessage 0xdffd83eb // PropertyChangedMessage

// Legacy fallback for C-level string-to-struct parsers registered before the
// generated struct descriptor is available.
struct struct_parser_entry {
  const char* type_name;
  structParserFn_t fn;
};

struct file_loader {
  const char* extension;
  struct Object* (*fn)(int argc, const char* argv[]);
};

struct font_registry_entry {
  fixedString_t name;
  path_t path;
};

struct game
{
  longTime_t realtime;
  uint32_t frame;
  struct Object *focus;
  struct Object *hover;
  struct Object *hover2;
  fixedString_t tags[MAX_TAGS];
  struct ClassDesc const *classes[MAX_CLASSES];
  struct StructDesc *structs[MAX_STRUCTS];
  struct PropertyType ptypes[MAX_PROPERTY_TYPES];
  struct PropertyShorthand shorthands[MAX_PROPERTY_SHORTHANDS];
  struct struct_parser_entry struct_parsers[MAX_STRUCT_PARSERS];
  struct file_loader file_loaders[MAX_FILE_LOADERS];
  struct font_registry_entry fonts[MAX_FONT_FAMILIES];
  uint64_t struct_parse_mask;
  bool_t struct_parse_mask_valid;
};

// stateman.c

void
API_PrintStackTrace(lua_State* L);

void
OBJ_Clear(struct Object *);

uint32_t
OBJ_AddRef(struct Object *);

uint32_t
OBJ_ReleaseRef(struct Object *);

void
OBJ_DetachFromParent(struct Object *);

struct Object *
OBJ_FindImmediateChild(struct Object *object, uint32_t ident);

uint32_t
OBJ_GetUniqueID(struct Object const *);

int64_t
OBJ_GetObjectCount(void);


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
PROP_Create(lua_State*, struct Object *, struct PropertyType const *);
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
PROP_Import(struct Property *, struct vm_register*);
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
                   struct token* program);

ORCA_API bool_t
PROP_SetBinding(struct Property *,
                lpcString_t expression,
                eBindingMode_t mode,
                bool_t enabled);

extern struct game core;

#endif /* n_utils_h */
