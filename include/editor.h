#ifndef __editor_h__
#define __editor_h__

#include <include/orca.h>
#include <include/renderer.h>

#include "../source/filesystem/fs_local.h"

typedef char CHAR;
typedef char SHORTSTR[64];
typedef char PATHSTR[256];
typedef float FLOAT;
typedef bool_t BOOL;
typedef uint32_t DWORD;
typedef uint32_t wParam_t;
typedef uint32_t HANDLE;
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef longTime_t TIME;
typedef void *lParam_t, *LPVOID;
typedef struct state_manager *HSTATEMAN;
typedef struct WI_Size SIZE2;
typedef struct rect RECT;
typedef struct rect RECT, *LPRECT;
typedef struct rect const *LPCRECT;
typedef struct vec2 VECTOR2, *LPVECTOR2;
typedef struct vec3 VECTOR3, *LPVECTOR3;
typedef struct vec4 VECTOR4, *LPVECTOR4;
typedef struct mat4 MATRIX4, *LPMATRIX4;
typedef struct color COLOR, *LPCOLOR;
typedef lpProperty_t HPROP;
typedef lpObject_t HOBJ;
typedef lpcPropertyDesc_t LPCPDESC;
typedef unsigned char *LPXMLSTR;
typedef unsigned char const *LPCXMLSTR;
typedef struct _xmlNode *LPXMLNODE;
typedef struct _xmlDoc *LPXMLDOC;
typedef eDataType_t PROP_TYPE;

#define EVT_USER 0x1111

typedef enum {
  ED_EDIT,
  ED_PLAY,
  ED_PAUSE,
} EDSTATE;

typedef struct _EDIMPORT {
  bool_t (*GetOpenFileName)(struct _WI_OpenFileName const *);
  bool_t (*GetSaveFileName)(struct _WI_OpenFileName const *);
  bool_t (*GetFolderName)(struct _WI_OpenFileName const *);

} EDIMPORT;

void ED_Init(EDIMPORT imp);
void ED_Shutdown(void);
LRESULT ED_DispatchMessage(DWORD msg, wParam_t wparam, lParam_t lparam);
void ED_Draw(void);
void ED_OpenProject(lpcString_t, EDSTATE);
BOOL ED_IsRunning(void);

typedef struct _OBJDEF {
  lpcString_t     szName;
  lpcString_t     szFullPath;
  lpcString_t     szSuperclass;
  lpcString_t     szClass;
  HANDLE     hIdentifier;
  HANDLE     hParent;
  DWORD      dwItemType;
  DWORD      dwStyle;
  VECTOR3    actualPosition;
  VECTOR3    actualSize;
  HSTATEMAN stateman;
  DWORD     *lpFlags;
  DWORD     *lpUserData;
  void      *lpObjectItself;
  BOOL       bHasChildren:1;
  BOOL       bHasAddButton;
} *LPOBJDEF;

typedef void (*EnumChildProc)(LPOBJDEF, LPVOID);

//typedef EDEXPORT (*GetRefAPI_t)(EDIMPORT);

#endif
