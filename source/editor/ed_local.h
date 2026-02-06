#ifndef ed_local_h
#define ed_local_h

#include <include/editor.h>
#include <include/renderer.h>
#include <include/module.h>
#include <include/api.h>

#include <libs/platform/platform.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

#define MAX_EDITOR_PANELS 64
#define MAX_TEXT_EDIT 64
#define MAX_ASSET_PATH 1024
#define DROP_ITEM_LENGTH 64
#define DRAG_N_DROP_DISTANCE 3

#define HEDWND_BOARDCAST ((HEDWND)0xffff)

//#define FMT_SET_CURSOR "\033[%d;%df"
#define FMT_COMMAND "\033[%dc"
#define FMT_NEXT_LINE "\n"
#define FMT_SET_CHARPOS "\033[%dG"

#define CELL_WIDTH 9
#define NAME_WIDTH 24
#define DROPDOWN_WIDTH (CELL_WIDTH * 2 + 1)

#define MAX_MENU_HEIGHT 16
#define T_ADD_PROPERTY -1
#define WINDOW_BORDER_COLOR 8
#define DRAG_THRESHOLD 1

//#define STYLE_HEADER "\033[0;7m"
//#define STYLE_HEADER_INACTIVE "\033[6;7m"
//#define STYLE_HEADER_BUTTON "\033[7;0m"
#define STYLE_HEADER "\033[0;15m"
#define STYLE_HEADER_INACTIVE "\033[8;15m"
#define STYLE_LABEL "\033[7;0m"
#define STYLE_MENUBAR "\033[0;11m"
#define STYLE_MENU "\033[0;15m"
#define STYLE_HIDDEN "\033[8;0m"
#define STYLE_TREEVIEW_STYLE(STYLE) "\033[%d;0m", STYLE
#define STYLE_HAS_BINDING "\033[11;0m"
#define STYLE_USED_IN_BINDING "\033[13;0m"
#define STYLE_IS_MODIFIED "\033[15;0m"
#define STYLE_TEXT_EDIT "\033[9;1m"
#define STYLE_TEXT_PLACEHOLDER "\033[11;1m"

#define STYLE_FIELD_FLOAT      STYLE_LABEL
#define STYLE_FIELD_DROPDOWN   "\033[15;8m"
#define STYLE_FIELD_DROPDOWN_INV "\033[8;15m"
#define STYLE_FIELD_HANDLE     "\033[14;0m"
#define STYLE_FIELD_HANDLE_PROJECT "\033[0;14m"
#define STYLE_FIELD_STRING     "\033[11;0m"
#define STYLE_FIELD_TOGGLE_ON  "\033[10;0m"
#define STYLE_FIELD_TOGGLE_OFF STYLE_HIDDEN

#define EXPAND_BUTTON_WIDTH 2
#define SCROLL_SENSIVITY 1.2f

#define TITLEBAR_HEIGHT CONSOLE_CHAR_HEIGHT

#define TOOLBAR_HEIGHT 38
#define TOOLBAR_BUTTON_SIZE 32
#define TOOLBAR_ICON_SIZE 24

#define EDWF_NODRAW 1
#define EDWF_TITLEBAR 2
#define EDWF_NOCLEAR 4
#define EDWF_CASTSHADOW 8
#define EDWF_NOFOCUS 16
#define EDWF_TOOLBAR 32
#define EDWF_DISABLED 64

#define EDWINPROC(NAME) LRESULT ED_##NAME(HEDWND wnd, DWORD msg, wParam_t wparm, lParam_t lparm)

#define ED_Echo(wnd, ...) Echo(ED_GetClient(wnd), __VA_ARGS__)

#define ED_TrackPopupMenu(menu, _x, _y, wnd) \
  ED_SetFocusedPanel(ED_CreateWindow(ED_Menu, 0, &(RECT) { \
    ED_GetClientRect(wnd).x + _x, \
    ED_GetClientRect(wnd).y + _y \
  }, wnd, menu))

enum {
  EVT_PRINT = EVT_USER,
  EVT_PAINT,
  EVT_TITLEBAR,
  EVT_CREATE,
  EVT_DESTROY,
  EVT_HOTKEY,
  EVT_COMMAND,
  EVT_CCLICK,
  EVT_CDCLICK,
  EVT_CONTEXTMENU,
  EVT_CGRAB,
  EVT_CDRAG,
  EVT_CDROP,
  EVT_MOUSEDRAG,
  EVT_WINDOWADDED,
  EVT_GETTEXT,
  EVT_MOVE,
  EVT_SIZE,
  EVT_EXTRASPACE,
  EVT_FILE_SELECTED,
  EVT_OBJECT_SELECTED,

  TB_ADDBUTTONS,
  TB_CHECKBUTTON,
};

enum {
  CBN_SELCHANGE = 0x1000,
  TREE_ON_SELECT,
  TREE_EXPAND,
  TREE_SELECT,
  GAME_SELECT_SCREEN,
};

typedef enum {
  TEXTEDIT_INPUT,
  TEXTEDIT_CANCEL,
  TEXTEDIT_ACCEPT,
  TEXTEDIT_UP,
  TEXTEDIT_DOWN,
  TEXTEDIT_ERASE,
} TEXTEDITEVENT;

enum {
  PROJITEM_SCENE,
  PROJITEM_SHADER,
  PROJITEM_MATERIAL,
  PROJITEM_PROJECT,
};

enum {
  ID_EMPTY,
  
  ID_FILE,
  ID_EDIT,
  ID_VIEW,
  ID_OBJECT,
  ID_ASSET,

  ID_FILE_NEW_SCENE,
  ID_FILE_OPEN_SCENE,
  ID_FILE_SAVE,
  ID_FILE_SAVE_AS,
  ID_FILE_RUN,
  ID_FILE_QUIT,

  ID_EDIT_UNDO,
  ID_EDIT_REDO,
  ID_EDIT_CUT,
  ID_EDIT_COPY,
  ID_EDIT_PASTE,
  ID_EDIT_DELETE,
  ID_EDIT_RENAME,
  ID_EDIT_DUPLICATE,
  ID_EDIT_TOGGLE,
  ID_EDIT_FILTER,
  
  ID_OBJECT_LABEL,
  ID_OBJECT_IMAGE,
  ID_OBJECT_STACK,
  ID_OBJECT_GRID,
  ID_OBJECT_CINEMATIC,
  
  ID_ASSET_SCENE,
  ID_ASSET_MATERIAL,
  ID_ASSET_SHADER,
  ID_ASSET_PROJECT_REFERENCE,
  ID_ASSET_PROJECT_REFERENCE_EXISTING,

  ID_NAV_NEXTCOL,
  ID_NAV_PREVCOL,
  ID_NAV_NEXTROW,
  ID_NAV_PREVROW,
  ID_NAV_NEXTTAB,
  ID_NAV_PREVTAB,
  ID_NAV_ACCEPT,
  ID_NAV_CANCEL,
  ID_NAV_TOGGLE,
  ID_NAV_NEXT,
  ID_NAV_SEL_HIERARCHY,
  ID_NAV_SEL_INSPECTOR,
  ID_NAV_MENU,
  
  // TEXT EDITOR
  ID_TEXT_NEXTWORD,
  ID_TEXT_PREVWORD,
  ID_TEXT_SELECT_NEXTWORD,
  ID_TEXT_SELECT_PREVWORD,
  ID_TEXT_SELECT_NEXTCOL,
  ID_TEXT_SELECT_PREVCOL,
  ID_TEXT_DELETE,
  ID_TEXT_BACKSPACE,
  ID_TEXT_END,
  ID_TEXT_HOME,
  ID_TEXT_NEWLINE,
  
  // SCENEVIEW
  ID_SELECT,
  ID_MOVE,
  ID_ROTATE,
  ID_SCALE,
};

typedef struct _EDWND EDWND, *HEDWND;
typedef LRESULT (*EDPANELPROC)(HEDWND, DWORD, wParam_t, lParam_t);
typedef void (*TEXTEDITCALLBACK)(TEXTEDITEVENT, lParam_t);

typedef struct _TERMINALCHAR {
  BYTE character;
  BYTE foreground:4;
  BYTE background:4;
  WORD item;
} TERMINALCHAR, *LPTERMINALCHAR;

typedef struct _TERMINAL {
  DWORD width;
  DWORD height;
  DWORD cursor;
  DWORD contentheight;
  TERMINALCHAR status;
  TERMINALCHAR buffer[CONSOLE_WIDTH * CONSOLE_HEIGHT];
} *LPTERMINAL;

//EASY_STRUCT(EDRECT) {
//    DWORD x, y, width, height;
//};

typedef struct _EDTEXTEDIT {
  BOOL             active;
  LPVOID           dest;
  LPVOID           parm;
  lpcString_t           placeholder;
  DWORD            pindex;
  DWORD            bounds;
  PROP_TYPE        type;
  TEXTEDITCALLBACK callback;
} EDTEXTEDIT, *LPEDTEXTEDIT;

typedef struct _EDDRAGITEM {
  BOOL    active;
  CHAR    name[DROP_ITEM_LENGTH];
  CHAR    data[MAX_ASSET_PATH];
  LPVOID  object;
  HEDWND  window;
  DWORD   dragtype;
  DWORD   indent;
  VECTOR2 offset;
  VECTOR2 startdrag;
} EDDRAGITEM, *LPEDDRAGITEM;

#define ED_ToggleToolbar(wnd, wparm, cmd) \
ED_SendMessage(wnd, TB_CHECKBUTTON, cmd, LOWORD(wparm)==cmd?wnd:NULL)

#define TBSTATE_ENABLED 1
#define TBSTATE_CHECKED 2

#define ED_CMD_LEN CONSOLE_WIDTH
#define ED_MAX_COMMANDS 1024

typedef struct _EDTBBTN {
  DWORD iBitmap;
  DWORD idCommand;
  BYTE  fsState;
  BYTE  fsStyle;
//  BYTE      bReserved[2];
//  DWORD_PTR dwData;
//  INT_PTR   iString;
} EDTBBTN, *LPEDTBBTN;

typedef struct _EDCMD {
  DWORD dwKey;
  DWORD szCommand;
  lpcString_t szName;
  struct _EDCMD *subcommands;
} EDCMD, *LPEDCMD;

#define MENU_BUFFER_SIZE 256
typedef EDCMD MENU[MENU_BUFFER_SIZE];

typedef struct _EDCACHE {
  struct _EDCACHE *next;
  PATHSTR path;
  LPXMLDOC doc;
} EDCACHE, *LPEDCACHE;

typedef struct _EDPAK {
  struct _EDPAK *next;
  SHORTSTR name;
  PATHSTR folder;
  PATHSTR startupScreen;
  LPXMLDOC doc;
} EDPAK, *LPEDPAK;

typedef struct _EDPROJECT {
  LPXMLNODE scene;
  LPEDPAK packages;
  LPEDCACHE cache;
} EDPROJECT, LPEDPROJECT;

struct lua_State;
typedef struct _EDITOR {
  lua_State *L;
  HEDWND     root;
  EDSTATE    state;
  DWORD      selectedFile;
  VECTOR2    mouseLocation;
  EDDRAGITEM dragItem;
  HEDWND     focus;
  HEDWND     inspector;
  CHAR       tooltip[ED_CMD_LEN];
  EDCMD      commands[ED_MAX_COMMANDS];
  HEDWND     menuBar;
  EDTEXTEDIT textEdit;
  BYTE       textEditCursor;
  CHAR       textEditBuffer[MAX_TEXT_EDIT];
  PATHSTR    currentProject;
  BOOL       bHasFinished;
  EDPROJECT  project;
  lpObject_t screen;
} EDITOR;

typedef struct _EDCREATE {
  EDPANELPROC proc;
  DWORD flags;
  LPVOID parm;
} EDCREATE;

typedef struct {
  EDCREATE left, right;
  FLOAT split;
  BOOL horizontal;
} SPLITVIEWDESC;

typedef struct _EDDROPDOWN {
  DWORD dwNumItems;
  DWORD dwSelected;
  DWORD dwUserData;
  LPVOID pArg;
  HEDWND pParent;
  void (*pSelectedProc)(DWORD dwSelected, LPVOID pArg);
  shortStr_t pTitle;
  shortStr_t pStrings[64];
} EDDROPDOWN;

extern EDIMPORT eimp;

typedef void (*EnumChildProc)(LPOBJDEF, LPVOID);

typedef struct {
  void (*EnumChildObjects)(LPVOID, EnumChildProc, LPVOID);
  void (*FilterObjects)(LPVOID, lpcString_t, EnumChildProc, LPVOID);
  BOOL (*GetObjectItem)(LPVOID, LPOBJDEF);
  BOOL (*GetObjectProperty)(LPVOID, lpcString_t, LPPROPDEF);
  BOOL (*SetObjectName)(LPVOID, lpcString_t);
  DWORD (*NewObject)(LPVOID, lpcString_t, DWORD);
} TREEVIEWDATA, *LPTREEVIEWDATA;

void TreeView_CreateItem(HEDWND wnd, DWORD type);

extern EDITOR editor;

HEDWND ED_InitLayout(void);
LPVOID ED_AllocUserData(HEDWND, DWORD size);
void ED_ReleaseUserData(HEDWND);
void ED_SetUserData(HEDWND, LPVOID);
LPVOID ED_GetUserData(HEDWND);
void ED_SetWindowRect(HEDWND, LPCRECT);
RECT ED_GetWindowRect(HEDWND);
RECT ED_GetClientRect(HEDWND);
RECT ED_GetTitleBarRect(HEDWND hWnd);
RECT ED_GetToolBarRect(HEDWND hWnd);
VECTOR2 ED_GetScroll(HEDWND);
BOOL ED_FunctionEquals(HEDWND wnd, void *ptr);
int ED_RegisterCommands(HEDWND, LPEDCMD);
LRESULT ED_SendMessage(HEDWND panel, DWORD msg, wParam_t, lParam_t);
void ED_StoreUndo(LPVOID dest, LPVOID value, BYTE size);
void ED_SetFocusedPanel(HEDWND p);
void ED_MoveToFront(HEDWND wnd);
void ED_SplitView_SetContent(HEDWND wnd, EDPANELPROC proc, lParam_t param);
HEDWND ED_SplitView_GetContent(HEDWND wnd);
HEDWND ED_SplitView_GetSideBar(HEDWND wnd);
void ED_CancelInput(void);
EDPANELPROC ED_GetWindowProc(HEDWND);
HEDWND ED_FindWindow(EDPANELPROC);
HEDWND ED_FindWindowInChildren(HEDWND wnd, EDPANELPROC proc);
HEDWND ED_OpenDocument(lpcString_t szFilename);
lpcString_t ED_GetWindowDocument(HEDWND wnd);
void ED_SetMenuItems(lpcString_t *items);
LRESULT ED_TreeView(HEDWND p, DWORD msg, wParam_t wparm, lParam_t lparm);
void ED_InvalidateWindow(HEDWND panel);
BOOL ED_AppendMenu(MENU menu, DWORD cmd_id);
LPTERMINAL ED_CreateConsole(DWORD dwWidth, DWORD dwHeight);
INLINE DWORD ED_GetColumn(LPTERMINAL lpTerminal, DWORD cursor) {
  return cursor % lpTerminal->width;
}
INLINE DWORD ED_GetRow(LPTERMINAL lpTerminal, DWORD cursor) {
  return cursor / lpTerminal->width;
}

#define ED_SameColumn(t, a, b) ED_GetColumn(t, a) == ED_GetColumn(t, b)
#define ED_SameRow(t, a, b) ED_GetRow(t, a) == ED_GetRow(t, b)

HEDWND ED_GetParent(HEDWND);
void ED_SetTextInput(LPEDTEXTEDIT lpTextEdit);
void ED_ObjectField(LPTERMINAL p, lpObject_t* value, DWORD type, DWORD width);
void ED_ColorField(LPTERMINAL p, LPCOLOR v, DWORD width);
void ED_FloatField(LPTERMINAL p, FLOAT *value, DWORD width);
void ED_IntField(LPTERMINAL p, int *value, DWORD width);
void ED_StringField(LPTERMINAL p, lpcString_t value, DWORD width);
void ED_EnumField(LPTERMINAL p, DWORD *value, lpcString_t values, DWORD width);
void ED_BooleanField(LPTERMINAL p, BOOL *value, lpcString_t name, DWORD width);
void ED_SetDragData(DWORD  dwType, lpcString_t szTitle, LPVOID lpObject, BYTE indent);
void ED_PrintStatus(lpcString_t szFormat, ...);
void ED_HandleTextInput(wParam_t wparam);
WORD ED_GetSelectedItem(HEDWND wnd);
void ED_SetSelectedItem(HEDWND wnd, WORD itemIndex);
void ED_SetHoveredItem(HEDWND wnd, WORD itemIndex);
void ED_ResetScroll(HEDWND);
void ED_ScrollToLocation(HEDWND wnd, FLOAT location);
void ED_TextField(LPTERMINAL lpTerminal);
TERMINALCHAR Con_GetPoint(LPTERMINAL, VECTOR2);
void ED_ClearConsole(HEDWND wnd);
void ED_InvalidateEditor(void);
HEDWND ED_CreateWindow(EDPANELPROC, DWORD, LPCRECT, HEDWND, lParam_t);
void ED_DestroyWindow(HEDWND hWnd);
DWORD ED_GetWindowFlags(HEDWND);
void ED_SetWindowFlags(HEDWND hWnd, DWORD dwFlags);
void ED_RegisterRootCommands(void);
LRESULT ED_Menu(HEDWND wnd, DWORD msg, wParam_t wparm, lParam_t lparm);
LPVOID TreeView_GetItem(HEDWND wnd, WORD item);

LPTERMINAL ED_GetClient(HEDWND wnd);

void Echo(LPTERMINAL, lpcString_t, ...);
void Con_PutChar(LPTERMINAL lpTerminal, BYTE c);
void Con_Clear(LPTERMINAL);
void Con_FillForegroundColor(LPTERMINAL);
void Con_ClearItems(LPTERMINAL);
DWORD Con_GetItemPosition2(LPTERMINAL con, DWORD i);
DWORD Con_GetItemPosition(LPTERMINAL con, DWORD i);
//DWORD Con_GetCurrentPosition(LPTERMINAL);
TERMINALCHAR Con_GetPoint(LPTERMINAL, VECTOR2);
DWORD Con_GetCurrentLine(LPTERMINAL lpTerminal);
BOOL Con_CheckOverflow(LPTERMINAL);
void ED_Printf(lpcString_t format, ...);
void Con_SetTextAttribute(LPTERMINAL lpTerminal, LPTERMINALCHAR lpChar);
void Con_FillLineWithColor(LPTERMINAL lpTerminal, DWORD background);

void ED_RunProject(void);
void ED_StopProject(void);

BOOL ED_Undo(void);
BOOL ED_Redo(void);

LPVOID ED_AllocateTreeViewData(LPTREEVIEWDATA, LPVOID);
BOOL XML_FindNode(HANDLE, LPXMLNODE *);
LPEDPAK ED_LoadPackage(lpcString_t szPath);
LPXMLNODE ED_LoadXML(lpcString_t szFileName);

BOOL FS_GetFileName(HANDLE ident, LPSTR path, BOOL fullpath);
void FS_EnumDir(lpcString_t, EnumChildProc, LPVOID);
void FS_FilterFiles(lpcString_t szFilter, EnumChildProc, LPVOID);
BOOL FS_FindFileByHandle(HANDLE, LPOBJDEF);
uint32_t FS_FindLibraryType(lpcString_t libname);
BOOL FS_RenameFile(HANDLE, lpcString_t);
DWORD FS_NewFile(HANDLE, lpcString_t, DWORD);
BOOL FS_GetProjectReference(lpcString_t szName, LPSTR pOut, DWORD nMaxLen);

bool_t OBJ_SaveDocument(lpObject_t object);
xmlNodePtr ED_ConvertNode(lpObject_t object, xmlNodePtr parent);
HOBJ UI_FindObjectByUniqueID(HANDLE hHandle, HOBJ root);
HOBJ XML_ParseObjectNode(lua_State* L, xmlNodePtr xml, HOBJ root);

#endif
