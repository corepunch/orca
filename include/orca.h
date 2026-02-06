#ifndef __ORCA_H__
#define __ORCA_H__

#include "orcadef.h"

#include <libs/platform/platform.h>

typedef struct lua_State lua_State;

typedef unsigned int bool_t;
typedef unsigned char byte_t;
typedef void* handle_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef uint32_t LRESULT;
typedef signed int HRESULT;
typedef uint32_t wParam_t;
typedef void* lParam_t;
typedef long long objectTags_t;
typedef unsigned long longTime_t;
typedef char shortStr_t[MAX_NAMELEN];
typedef const char *lpcString_t;
typedef char *LPSTR;
typedef int *lpint;

struct renderer;
struct game;
struct localization;
struct uniform;
struct font;
struct WI_Buffer;
struct view_text;
struct view_entity;
struct view_def;
struct view_camera;
struct Package;
struct token;

struct _xmlDoc;
struct _xmlNode;

FWD_STRUCT(Property);
FWD_STRUCT(Object);
FWD_STRUCT(ClassDesc);
FWD_STRUCT(PropertyDesc);

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
//#include <stdbool.h>
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

enum ipo_type
{
  IPO_LINEAR,
  IPO_CONST,
  IPO_BACK,
  IPO_BOUNCE,
  IPO_CIRC,
  IPO_CUBIC,
  IPO_ELASTIC,
  IPO_EXPO,
  IPO_QUAD,
  IPO_QUART,
  IPO_QUINT,
  IPO_SINE,
};

enum easing
{
  IPO_EASE_IN_OUT,
  IPO_EASE_IN,
  IPO_EASE_OUT,
};

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
  lpObject_t rootnode;
  struct {
    int component;
    lpProperty_t property;
    handle_t animation;
    bool_t (*func)(struct token* token,
                   struct vm_register* regs,
                   lpObject_t object,
                   struct vm_register* output);
  } cache;
  uint32_t reg;
  uint32_t attr;
  bool_t oneway;
  bool_t inbrackets;
  void* userdata;
  enum token_type type;
  char text[4];
};

enum keyframe_mode {
  Key_Free,
  Key_Auto,
  Key_Linear,
  Key_Constant,
  Key_ClampedAuto,
};

struct keyframe
{
  float value[4];
  float inSlope[4];
  float outSlope[4];
  float inWeight[4];
  float outWeight[4];
  float time;
  int tangentMode;
  int weightedMode;
};

struct curve
{
  shortStr_t path;
  shortStr_t property;
  uint32_t num_keyframes;
  struct curve *next;
  struct keyframe keyframes[];
};

enum animation_mode {
  kAnimationPlayOnce,
  kAnimationLoop,
  kAnimationPingPong,
};

FWD_STRUCT(KeyframeAnim);

struct KeyframeAnim
{
  longTime_t timer;
  float start_time;
  float stop_time;
  enum animation_mode mode;
  LPSTR name;
  LPSTR filename;
  struct curve *curves;
  lpKeyframeAnim_t next;
};

struct ID
{
  lpcString_t Name;
  uint32_t Identifier;
};

ORCA_API lpKeyframeAnim_t
ANIM_Load(struct _xmlDoc*);

void
ANIM_Release(lpKeyframeAnim_t);

#define SV_PostMessage(OBJ, MSG, _W, _L) \
WI_PostMessageW(OBJ, fnv1a32(MSG), _W, _L);

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
  PF_NIL = (1 << 0),
  PF_USED_IN_TRIGGER = (1 << 1),
  PF_USED_IN_STATE_MANAGER = (1 << 2),
  PF_MODIFIED = (1 << 3),
  PF_HAS_SOURCE = (1 << 4),
  PF_HASCHANGECALLBACK = (1 << 5),
  PF_SPECIALIZED = (1 << 6),
  PF_PROPERTY_TYPE = (1 << 7),
};

#define ON_CHANGED_CALLBACK "on%sChanged"
#define PROPERTY_TYPE_MASK ((1 << 8) - 1)

#define FOR_EACH_PROPERTY(VARIABLE, LIST) \
  for (lpProperty_t VARIABLE = LIST; VARIABLE; \
       VARIABLE = PROP_GetNext(VARIABLE))

/*
 Property data structures
 */

#define PROPERTY_SINGLE 4
#define MAX_PROPERTY_STRING 256
#define MAX_PROPERTIES 65536
#define MAX_BINDING_ARGS 16

#define PF_IS_BOOLEAN 256

typedef char fixedString_t[MAX_PROPERTY_STRING];

#define T_TYPEMASK (((uint32_t)-1) >> 4)
#define T_PRIVATE ~(((uint32_t)-1) >> 1)

enum property_attribute
{
  ATTR_WHOLE_PROPERTY,
  ATTR_COLOR_R, // = ATTR_VECTOR_X,
  ATTR_COLOR_G, // = ATTR_VECTOR_Y,
  ATTR_COLOR_B, // = ATTR_VECTOR_Z,
  ATTR_COLOR_A, // = ATTR_VECTOR_W,
  ATTR_VECTOR_X,
  ATTR_VECTOR_Y,
  ATTR_VECTOR_Z,
  ATTR_VECTOR_W,
  ATTR_COUNT
};

typedef enum
{
  BIND_EXPRESSION,
  BING_ONEWAY,
} BINDING_MODE;

struct PropertyDesc
{
  uint32_t FullIdentifier;
  uint32_t Offset;
  uint32_t Flags;
  lpcString_t TypeString;
  uint32_t NumComponents;
  struct ID *id;
};

#include <source/core/core.h>

ORCA_API bool_t
PROP_Update(lpProperty_t);

ORCA_API void
PROP_SetValue(lpProperty_t, void const* source);

ORCA_API bool_t
PROP_IsNull(lpcProperty_t);

ORCA_API lpProperty_t
PROP_FindByLongID(lpProperty_t, uint32_t identifier);

ORCA_API lpProperty_t
PROP_FindByShortID(lpProperty_t, uint32_t identifier);

ORCA_API uint32_t
PROP_GetFlags(lpcProperty_t);

ORCA_API void
PROP_SetFlag(lpProperty_t, uint32_t value);

ORCA_API uint32_t
PROP_GetLongIdentifier(lpcProperty_t);

ORCA_API lpProperty_t
PROP_GetNext(lpcProperty_t);

ORCA_API void const*
PROP_GetValue(lpcProperty_t);

ORCA_API enum DataType
PROP_GetType(lpcProperty_t);

ORCA_API uint32_t
PROP_GetSize(lpcProperty_t);

ORCA_API bool_t
PROP_HasProgram(lpProperty_t);

ORCA_API lpcString_t
PROP_GetUserData(lpcProperty_t);

ORCA_API lpcString_t
PROP_GetName(lpcProperty_t);

ORCA_API lpcString_t
PROP_GetComponentName(lpcProperty_t property);

ORCA_API void
PROP_GetFullName(lpcProperty_t property, LPSTR buf, int size);

ORCA_API void
PROP_Parse(lpProperty_t, lpcString_t);

ORCA_API void
PROP_ClearSpecialized(lpProperty_t list);

ORCA_API lpcPropertyDesc_t 
PROP_GetDesc(lpcProperty_t prop);

ORCA_API void
PROP_Print(lpProperty_t p, LPSTR buffer, uint32_t len);

ORCA_API lpProperty_t
PROP_Create(lua_State*,lpObject_t,lpcString_t,enum DataType,lpcString_t udata);

ORCA_API void
PROP_AttachProgram(lpProperty_t,
                   enum property_attribute,
                   struct token* program,
                   lpcString_t source);

ORCA_API uint32_t
GetPropertyHandleType(lpcString_t szType);

#define PROP_CopyValue(property, dest) memcpy(dest, PROP_GetValue(property), PROP_GetSize(property))
#define PROP_FindByFullName(LIST, NAME) PROP_FindByLongID(LIST, fnv1a32(NAME));

/*
 Object helper macros
 */

#define OBJ_CLASS(VAR, CLASS) (OBJ_GetClass(VAR) == ID_##CLASS)

// clang-format off

#define OBJ_GetScriptHandle(OBJECT) (*(uint32_t *)OBJ_GetPointer(OBJECT, GNP_SCRIPTHANLDE, 0))
#define OBJ_IsHidden(OBJECT) (OBJ_SendMessageW(OBJECT, kEventIsVisible, 0, NULL) == FALSE)

// clang-format on

//#define KANZI_SUPPORT

#define MESSAGE_HANDLER(PARM, TYPE)                                            \
  LRESULT NAME##_##PARM(                                                       \
    lpObject_t hObject, TYPE##Ptr NAME, P##PARM##STRUCT param)

#define FOR_EACH_OBJECT(var, parent)                                           \
  for (lpObject_t var = OBJ_GetFirstChild(parent), \
    next = var ? OBJ_GetNext(var) : NULL; \
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

typedef LRESULT (*objectProc_t)(lpObject_t, void*, uint32_t, wParam_t, lParam_t);

#define kEventAwake 0x7f460f7c

#define EVENT_PTR(NAME) NAME##EventPtr

typedef lpObject_t* EVENT_PTR(HitTest);
typedef struct lua_State* PushPropertyEventPtr;
typedef lpObject_t EVENT_PTR(Object);
typedef struct lua_State* EVENT_PTR(LoadView);

typedef struct
{
  lua_State* L;
  lpProperty_t hProperty;
} PROPERTYCHANGEDSTRUCT, *EVENT_PTR(PropertyChanged);

typedef struct
{
  union
  {
    struct
    {
      float Width, Height;
    };
    float Size[2];
  };
  bool_t Force;
} UPDATELAYOUTSTRUCT, *EVENT_PTR(UpdateLayout);

typedef struct
{
  lua_State* L;
  lpcString_t EventName;
  uint32_t FirstArg;
  uint32_t NumArgs;
} HANDLEMESSAGESTRUCT, *EVENT_PTR(HandleMessage);

typedef struct
{
  void* Trigger;
  EVENT_PTR(HandleMessage) message;
} TRIGGEREDSTRUCT, *EVENT_PTR(Triggered);

typedef struct _RENDERSCREENSTRUCT
{
  uint32_t width;
  uint32_t height;
  float stereo;
  float angle;
  handle_t target;
  bool_t only_paint;
} RENDERSCREENSTRUCT, *EVENT_PTR(RenderScreen);

typedef struct _UPDATEMATRIXSTRUCT
{
  struct mat4 const* parent;
  float opacity;
  bool_t force;
} UPDATEMATRIXSTRUCT, *EVENT_PTR(UpdateMatrix);

// Node2D messages

typedef void* EVENT_PTR(UpdateGeometry);

typedef struct _FOREGROUNDCONTENTSTRUCT
{
  handle_t result;
} FOREGROUNDCONTENTSTRUCT, *EVENT_PTR(ForegroundContent);

// Node3D messages

typedef struct view_def* EVENT_PTR(Render);

typedef struct _ARRANGECHILDRENSTRUCT
{
  /*enum Direction*/ uint32_t axis;
  struct bounds const* bounds;
} ARRANGECHILDRENSTRUCT, *EVENT_PTR(ArrangeChildren);

#define OF_UPDATED_ONCE (1 << 0)
#define OF_STATEMANAGER_INITIALIZED (1 << 1)
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

struct ClassDesc
{
  objectProc_t ObjProc;
  lpcPropertyDesc_t Properties;
  void const *Defaults;
  lpcString_t ClassName;
  lpcString_t DefaultName;
  lpcString_t ContentType;
  lpcString_t Xmlns;
  lpcClassDesc_t ParentClasses[16];
  uint32_t NumProperties;
  uint32_t ClassID;
  uint32_t ClassSize;
  uint32_t MemorySize;
};

struct component;

ORCA_API HRESULT
OBJ_FindShortProperty(lpObject_t object, lpcString_t short_name, lpProperty_t* output);

ORCA_API HRESULT
OBJ_FindLongProperty(lpObject_t object, lpcString_t long_name, lpProperty_t* output);

ORCA_API HRESULT
OBJ_SetPropertyValue(lpObject_t object, lpcString_t name, void const* value);

ORCA_API LRESULT
OBJ_SendMessage(lpObject_t, lpcString_t, wParam_t, lParam_t);

ORCA_API LRESULT
OBJ_SendMessageW(lpObject_t, uint32_t, wParam_t, lParam_t);

ORCA_API uint32_t
OBJ_GetUniforms(lpObject_t, struct uniform*);

ORCA_API void*
CMP_GetUserData(struct component*);

ORCA_API bool_t
SV_RegisterMessageProc(int (*proc)(lua_State*, struct WI_Message*));

ORCA_API bool_t
SV_UnregisterMessageProc(int (*proc)(lua_State*, struct WI_Message*));

ORCA_API void
OBJ_Release(lua_State *L, lpObject_t);

ORCA_API void*
OBJ_GetComponent(lpObject_t pobj, uint32_t class_id);

ORCA_API lpProperty_t
OBJ_GetPropertyAtIndex(lpObject_t object, uint32_t classid,
                       size_t classsize, uint32_t index);

ORCA_API lpObject_t
CMP_GetObject(void const*);

ORCA_API struct component*
OBJ_AddComponent(lpObject_t, lpcClassDesc_t);

ORCA_API void
OBJ_AddStyleClass(lpObject_t,
                  lpcString_t name,
                  lpcString_t prop,
                  lpcString_t val,
                  uint32_t flags);

typedef int
(*propertyParser_t)(lua_State*,lpObject_t,lpcString_t,lpcString_t);

ORCA_API HRESULT
SM_ReadStateManager(lua_State*, lpObject_t, struct _xmlDoc*, propertyParser_t);

ORCA_API bool_t
OBJ_RegisterClass(lpcClassDesc_t);

ORCA_API lpcClassDesc_t
OBJ_FindClass(lpcString_t);

ORCA_API lpcPropertyDesc_t
OBJ_FindPropertyType(uint32_t);

ORCA_API bool_t
OBJ_RegisterPropertyType(lpcPropertyDesc_t pt);

ORCA_API void
OBJ_EnumClasses(lpcClassDesc_t, void (*fnProc)(lpcClassDesc_t, void*), void*);

ORCA_API void
OBJ_EnumObjectClasses(lpObject_t pobj,
                      void (*fnProc)(lpcClassDesc_t, void*),
                      void* param);

ORCA_API void
OBJ_EnumClassProperties(lpObject_t object,
                        void (*fnProc)(lpcObject_t,
                                       lpcPropertyDesc_t,
                                       lpcClassDesc_t,
                                       void const*,
                                       void*),
                        void* parm);

/*
 File system access
 */

#define MAX_RESOURCEPATH 256
#define MAX_OSPATH 1024
#define MAX_SEARCHPATHS 32
#define MAX_ENVVARS 32
#define MAKE_FOURCC(a, b, c, d)                                                \
  ((int)(d) << 24 | (int)(c) << 16 | (b) << 8 | (a))
#define PACK_FLAT 0
#define PACK_LZ4 1
#define PACK_ZIP 2
#define CHUNK_SIZE 16384 // 16 KB buffer size, you can adjust this as needed

#define IDPAKHEADER MAKE_FOURCC('P', 'A', 'C', 'K')
#define IDMESHHEADER MAKE_FOURCC('M', 'E', 'S', 'H')

typedef char path_t[MAX_OSPATH];
typedef char ospathfmt_t[MAX_OSPATH + MAX_RESOURCEPATH];

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
//ORCA_API HRESULT FS_GetImageSize(lpcString_t, struct WI_Size*);
//ORCA_API struct _xmlDoc* FS_LoadXML(lpcString_t);
ORCA_API struct _xmlDoc* FS_LoadXML(lpcString_t);
ORCA_API struct _xmlDoc* FS_InitProject(lpcString_t);
ORCA_API lpcString_t FS_ParseArgs(LPSTR s, reqArg_t *args, size_t maxargs);
//ORCA_API void FS_RegisterObject(lpObject_t object, lpcString_t path);

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
// int  net_packet(int net_socket, struct WI_Buffer* net_message);
// int  net_send_packet(int net_socket, struct WI_Buffer* net_message);
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

#endif
