#ifndef __ORCA_H__
#define __ORCA_H__

#include "orcadef.h"

#include <libs/platform/platform.h>

typedef struct lua_State lua_State;

struct renderer;
struct game;
struct localization;
struct uniform;
struct AXbuffer;
struct ViewText;
struct ViewEntity;
struct ViewDef;
struct view_camera;
struct Package;
struct token;

struct _xmlDoc;
struct _xmlNode;

typedef uint32_t propertyID_t;
typedef uint32_t messageID_t;
typedef uint32_t classID_t;
typedef uint32_t event_t;

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "orcaerror.h"
#include "shared.h"

#define CONSOLE_WIDTH 128
#define CONSOLE_HEIGHT 256
#define CONSOLE_CHAR_WIDTH 8
#define CONSOLE_CHAR_HEIGHT 16

#define STRINGIZE(x) #x
#define TO_STRING(x) STRINGIZE(x)

enum message_type
{
  kMessageTypeNormal,
  kMessageTypeError,
  kMessageTypeWarning,
};

struct network_service
{
  char ipaddr[16];
  int port;
  int sockfd;
  int stopped;
  int last_poll_time;
  void (*read)(struct network_service* ptr);
  struct network_service* next;
};

enum token_type
{
  tok_identifier,
  tok_assign,
  tok_call,
  tok_argument,
  tok_constant,
  tok_string,
  tok_count
};

#define TOKEN_MAX_ARGS 4
struct vm_register;
struct token
{
  struct token* next;
  struct token* args[TOKEN_MAX_ARGS];
  struct Object *rootnode;
  struct {
    int component;
    struct Property *property;
    handle_t animation;
    bool_t (*func)(struct token* token,
                   struct vm_register* regs,
                   struct Object *object,
                   struct vm_register* output);
  } cache;
  uint32_t reg;
  uint32_t attr;
  bool_t oneway;
  bool_t inbrackets;
  enum token_type type;
  char text[4];
};


#define SV_PostMessage(OBJ, MSG, _W, _L) \
axPostMessageW(OBJ, fnv1a32(MSG), _W, _L);

/*
 Fowler–Noll–Vo hash function
 */

INLINE uint32_t
fnv1a32(lpcString_t str)
{
  uint32_t value = 0x811c9dc5;
  for (; *str; str++) {
    value ^= *str;
    value *= 0x1000193;
  }
  return value;
}

INLINE uint32_t
fnv1a32_range(lpcString_t from, lpcString_t to)
{
  uint32_t value = 0x811c9dc5;
  for (; from != to; from++) {
    value ^= *from;
    value *= 0x1000193;
  }
  return value;
}

/*
 Basic memory operations
 */

INLINE void*
_zero_alloc(lpcString_t file, uint32_t line, size_t size)
{
  //    Con_Error("%d", size);
  //    if (size == 6553600) {
  //        int a =0 ;
  //    }
  //    Con_Error("%s:%d %d\n", file, line, (int)size);
  //    static int a = 0;
  //    if (rand() % 2) {
  //        return NULL;
  //    }
  void* mem = malloc(size);
  memset(mem, 0, size);
  return mem;
}

/*
 Property flags
 */

enum
{
  PF_USED_IN_TRIGGER = (1 << 1),
  PF_USED_IN_STATE_MANAGER = (1 << 2),
  PF_MODIFIED = (1 << 3),
  PF_HAS_SOURCE = (1 << 4),
  PF_HASCHANGECALLBACK = (1 << 5),
  PF_SPECIALIZED = (1 << 6),
  PF_PROPERTY_TYPE = (1 << 7),
  PF_NOTIFICATION_QUEUED = (1 << 8), // notification is queued as PropertyChangedMessage
};

#define ON_CHANGED_CALLBACK "on%sChanged"
#define PROPERTY_TYPE_MASK ((1 << 8) - 1)

#define FOR_EACH_PROPERTY(VARIABLE, LIST) \
  for (struct Property *VARIABLE = LIST; VARIABLE; \
       VARIABLE = PROP_GetNext(VARIABLE))

/*
 Property data structures
 */

#include <source/core/core.h>

ORCA_API struct property_program const *
core_GetPrograms(void);

ORCA_API struct Object *
OBJ_Create(uint32_t class_id);

ORCA_API void
PROP_RunAllPrograms(void);

ORCA_API bool_t
PROP_Update(struct Property *);

ORCA_API void
PROP_SetValue(struct Property *, void const* source);

ORCA_API void
PROP_SetStringValue(struct Property *, char const* source);

ORCA_API void
PROP_SetDirty(struct Property *property);

ORCA_API bool_t
PROP_IsNull(struct Property const *);

ORCA_API struct Property *
PROP_FindByLongID(struct Property *, uint32_t identifier);

ORCA_API struct Property *
PROP_FindByShortID(struct Property *, uint32_t identifier);

ORCA_API uint32_t
PROP_GetFlags(struct Property const *);

ORCA_API void
PROP_SetFlag(struct Property *, uint32_t value);

ORCA_API void
PROP_Clear(struct Property *);

ORCA_API uint32_t
PROP_GetLongIdentifier(struct Property const *);

ORCA_API struct Property *
PROP_GetNext(struct Property const *);

ORCA_API void const*
PROP_GetValue(struct Property const *);

ORCA_API enum DataType
PROP_GetType(struct Property const *);

ORCA_API uint32_t
PROP_GetSize(struct Property const *);

ORCA_API bool_t
PROP_HasProgram(struct Property *);

ORCA_API lpcString_t
PROP_GetUserData(struct Property const *);

ORCA_API lpcString_t
PROP_GetName(struct Property const *);

ORCA_API lpcString_t
PROP_GetComponentName(struct Property const *property);

ORCA_API void
PROP_ClearSpecialized(struct Property *list);

ORCA_API struct PropertyType const *
PROP_GetDesc(struct Property const *prop);

ORCA_API void
PROP_Print(struct Property *p, LPSTR buffer, uint32_t len);

ORCA_API struct Property *
PROP_Create(lua_State*,struct Object *,struct PropertyType const *);

ORCA_API uint32_t
GetPropertyHandleType(lpcString_t szType);

typedef void (*EnumPropertyTypeProc)(lpcString_t, void*);
typedef void (*EnumAliasProc)(lpcString_t, lpcString_t, void*);
ORCA_API bool_t UI_EnumObjectAliases(struct Object *object, EnumAliasProc, void* args);

#define PROP_CopyValue(property, dest) memcpy(dest, PROP_GetValue(property), PROP_GetSize(property))
#define PROP_FindByFullName(LIST, NAME) PROP_FindByLongID(LIST, fnv1a32(NAME));

/*
 Object helper macros
 */

#define OBJ_CLASS(VAR, CLASS) (OBJ_GetClass(VAR) == ID_##CLASS)

// clang-format off

#define OBJ_GetScriptHandle(OBJECT) (*(uint32_t *)OBJ_GetPointer(OBJECT, GNP_SCRIPTHANLDE, 0))

// clang-format on

//#define KANZI_SUPPORT

#define FOR_EACH_OBJECT(var, parent)                                           \
  for (struct Object *var = OBJ_GetFirstChild(parent), \
    *next = var ? OBJ_GetNext(var) : NULL; \
    var; var = next, next = next ? OBJ_GetNext(next) : NULL)

#ifdef KANZI_SUPPORT
#define FOR_EACH_LAYOUTABLE(var, parent)                                       \
  FOR_EACH_OBJECT(var, parent)                                                 \
    if (GetNode2D(var))
#else
#define FOR_EACH_LAYOUTABLE(var, parent)                                       \
  FOR_EACH_OBJECT(var, parent)                                                 \
    if (GetNode2D(var) && !OBJ_IsHidden(var))
#endif

#define FOR_EACH_CHILD(object, func, ...)                                      \
  FOR_EACH_OBJECT(child, object) func(child, ##__VA_ARGS__)

typedef LRESULT (*objectProc_t)(struct Object *, void*, uint32_t, wParam_t, lParam_t);

#ifndef ID_Node_IsVisible
#define ID_Node_IsVisible 0xa03cfb85 // Node.IsVisible
#endif
#define OBJ_IsHidden(OBJECT) (OBJ_SendMessageW(OBJECT, ID_Node_IsVisible, 0, NULL) == FALSE)

#define kEventResumeCoroutine 0x3cc3febc
#define kEventStopCoroutine 0xf13ac065
#define kEventReadCommands 0x23d83fd3

#define EVENT_PTR(NAME) NAME##MsgPtr

//typedef struct Object ** EVENT_PTR(HitTest);

#define OF_UPDATED_ONCE (1 << 0)
#define OF_LOADED_FROM_PREFAB (1 << 2)
#define OF_ACTIVATED (1 << 3)
#define OF_TEMPLATE (1 << 4)
#define OF_DIRTY (1 << 5)
#define OF_SCROLL (1 << 6)
#define OF_RENDERED (1 << 7)
#define OF_TABSTOP (1 << 8)
#define OF_HOVERABLE (1 << 9)
#define OF_SELECTED (1 << 10)
#define OF_EXPANDED_IN_DEBUG (1 << 11)
#define OF_NOACTIVATE (1 << 12)
#define OF_ANIMATE_QUEUED (1 << 13)

struct ClassDesc
{
  objectProc_t ObjProc; // pointer to the main message handling function for this class, used for dispatching messages to objects of this class
  struct PropertyType const *Properties; // pointer to an array of property descriptors, should be sorted by long identifier for efficient lookup
  lpcString_t ClassName; // human-readable name of the class, used for debugging and editor display, should be unique across all classes
  lpcString_t DefaultName; // string used for naming objects of this class when no name is provided, should be unique across all classes
  lpcString_t ContentType; // optional string describing the type of content this class represents, used for auto-detecting packages and for editor filtering
  lpcString_t Xmlns; // optional XML namespace associated with this class, used for auto-detecting packages
  uint32_t ParentClasses[16]; // array of class IDs of parent classes, used for inheritance and type checking, should be ordered from most derived to least derived
  uint32_t NumProperties; // number of properties defined directly on this class, used for iterating over properties and for calculating offsets of properties in the object struct
  uint32_t ClassID; // hash of the class name, used for quick comparisons and lookups, should be unique across all classes
  uint32_t ClassSize; // size of the class itself excluding components, used for calculating offsets of components and properties
  uint32_t MemorySize; // total size of an instance of this class including components, used for memory allocation
  void const *Defaults; // pointer to a struct containing default values for properties of this class, used for resetting to defaults and for inheriting default values in subclasses
  bool_t IsAttachOnly; // if true, this class can only be added as a component to an existing object, not instantiated standalone
};

struct component;

ORCA_API void
OBJ_RegisterFileLoader(const char* extension, struct Object* (*fn)(int argc, const char* argv[]));

ORCA_API HRESULT
OBJ_FindShortProperty(struct Object *object, lpcString_t short_name, struct Property ** output);

ORCA_API HRESULT
OBJ_FindLongProperty(struct Object *object, uint32_t long_id, struct Property ** output);

ORCA_API HRESULT
OBJ_SetPropertyValue(struct Object *object, lpcString_t name, void const* value);

ORCA_API LRESULT
OBJ_SendMessage(struct Object *, lpcString_t, wParam_t, lParam_t);

ORCA_API LRESULT
OBJ_SendMessageW(struct Object *, uint32_t, wParam_t, lParam_t);

ORCA_API uint32_t
OBJ_GetUniforms(struct Object *, struct uniform*);

ORCA_API void*
CMP_GetUserData(struct component*);

ORCA_API bool_t
SV_RegisterMessageProc(LRESULT (*proc)(lua_State*, struct AXmessage*));

ORCA_API bool_t
SV_UnregisterMessageProc(LRESULT (*proc)(lua_State*, struct AXmessage*));

ORCA_API bool_t
SV_DispatchMessage(lua_State*, struct AXmessage*);

ORCA_API void
OBJ_Release(lua_State *L, struct Object *);

ORCA_API void*
OBJ_GetComponent(struct Object *pobj, uint32_t class_id);

ORCA_API struct Property *
OBJ_GetPropertyAtIndex(struct Object *object, uint32_t classid,
                       size_t classsize, uint32_t index);

ORCA_API struct Object *
CMP_GetObject(void const*);

ORCA_API struct component*
OBJ_AddComponent(struct Object *, uint32_t class_id);

typedef int
(*propertyParser_t)(lua_State*,struct Object *,lpcString_t,lpcString_t);

ORCA_API bool_t
OBJ_RegisterClass(struct ClassDesc const *);

ORCA_API struct ClassDesc const *
OBJ_FindClass(lpcString_t);

// Register a pure-C string parser for a named struct type.
// The parser should write the parsed value to dst (of size `sz`) and return TRUE on success.
ORCA_API void
OBJ_RegisterStructParser(const char* type_name,
                         int (*fn)(const char* str, void* dst, size_t sz));

// Parse a struct of the given type name from a string using a registered C parser.
// Returns TRUE on success, FALSE if no parser is registered or parsing fails.
ORCA_API int
OBJ_ParseStruct(const char* type_name, const char* str, void* dst, size_t sz);

ORCA_API struct ClassDesc const *
OBJ_FindClassW(uint32_t);

ORCA_API struct PropertyType const *
OBJ_FindPropertyType(uint32_t);

ORCA_API bool_t
OBJ_RegisterPropertyType(struct PropertyType const *pt);

ORCA_API void
OBJ_EnumClasses(uint32_t superclass, void (*fnProc)(struct ClassDesc const *, void*), void*);

ORCA_API void
OBJ_EnumObjectClasses(struct Object *pobj,
                      void (*fnProc)(struct ClassDesc const *, void*),
                      void* param);

ORCA_API void
OBJ_EnumClassProperties(struct Object *object,
                        void (*fnProc)(struct Object const *,
                                       struct PropertyType const *,
                                       struct ClassDesc const *,
                                       void const*,
                                       void*),
                        void* parm);

/*
 File system access
 */

#define IDMESHHEADER MAKE_FOURCC('M', 'E', 'S', 'H')

typedef char path_t[MAX_OSPATH];

struct file {
  uint32_t size;
  byte_t data[];
};

typedef struct reqArg {
  shortStr_t name;
  shortStr_t value;
} reqArg_t;

struct Package;

ORCA_API struct file* FS_LoadFile(lpcString_t);
ORCA_API HRESULT FS_FreeFile(struct file*);
//ORCA_API HRESULT FS_GetImageSize(lpcString_t, struct AXsize*);
ORCA_API lpcString_t FS_ParseArgs(LPSTR s, reqArg_t *args, size_t maxargs);

// Editor stuff
ORCA_API void FS_InitHash(void);
ORCA_API void FS_ShutdownHash(void);

/*
 Network operations
 */

typedef void (*SERVICEREADPROC)(struct network_service*);

// bool_t net_socket_has_data(int sockfd);
// int  net_open_socket(int port);
// void net_close_socket(int net_socket);
// int  net_accept(int net_socket);
// int  net_connect(lpcString_t  addr, int port);
// int  net_packet(int net_socket, struct AXbuffer* net_message);
// int  net_send_packet(int net_socket, struct AXbuffer* net_message);
// int  net_set_nonblocking(int sockfd);
// bool_t net_has_no_error(void);

struct network_service* ConnectService(lpcString_t, uint16_t, SERVICEREADPROC);
void ReleaseService(struct network_service*);

HRESULT StartService(struct network_service*, SERVICEREADPROC);
HRESULT StopService(struct network_service*);
void _ServiceRead(struct network_service*);

// void Net_Init(void);
// void Net_Shutdown(void);

/*
 Console functions
 */

ORCA_API void Con_Init(void);
ORCA_API void Con_Shutdown(void);

/*
 VM stuff
 */

#ifdef USE_VM

typedef struct VM* PVM;
typedef struct _VMDEBUG* PVMDEBUG;

typedef uint32_t (*VM_SysCall)(PVM, uint32_t);

#define VM_STACK_SIZE 64 * 1024
#define VM_HEAP_SIZE 16 * 1024 * 1024

#define ID_ORCA 0x4143524F

typedef struct _VMPROGRAM
{
  uint32_t magic;
  uint32_t progsize;
  uint32_t num_symbols;
  byte_t data[];
}* PVMPROGRAM;

void
vm_execute(PVM vm, lpcString_t symbol);
PVM
vm_create(VM_SysCall systemCalls,
          uint32_t stack_size,
          uint32_t heap_size,
          PVMPROGRAM program);
void
vm_attach_debug(PVM vm, PVMDEBUG debugs, uint32_t numdebugs);
void vm_shutdown(PVM);
// Function to initialize the memory manager
void
initialize_memory_manager(PVM vm, void* buffer, size_t buffer_size);

void*
vm_allocmem(PVM vm, size_t size);
void*
vm_reallocmem(PVM vm, void* ptr, size_t size);
void
vm_freemem(PVM vm, void* ptr);

PVM
VM_Init(void);

#endif

/*
 WIMs stuff (move to a plugin)
 */

typedef struct _WIMSCREENVIEW
{
  uint32_t id;
  lpcString_t string;
}* PWIMSCREENVIEW;

typedef void (*WIMSCREENPROC)(PWIMSCREENVIEW, void*);
void WIM_EnumScreens(WIMSCREENPROC, void*);

// #define IID_FILESYSTEM "FILESYSTEM"
//#define IID_GAME "IMODULE"
// #define IID_LOCALIZATION "ILOCALIZATION"
// #define IID_CONSOLE "CONSOLE"
// #define IID_SYSTEM "SYSTEM"

typedef enum
{
  LOC_UNDEFINED,
  LOC_RESOURCE,
  LOC_TEXT,
} LOCALE_TYPE;

ORCA_API lpcString_t
Loc_GetString(lpcString_t, LOCALE_TYPE);


ORCA_API struct token*
Token_Create(lpcString_t szSourceCode);

ORCA_API void
Token_Release(struct token* pToken);

//struct IFileSystem {
//  void* (*LoadBundle)(const char* path);
//  bool_t (*HasChangedFiles)(void*);
//  bool_t (*FileExists)(const char* path);
//  bool_t (*OpenFile)(const char* path);
//};

#endif
