#ifndef __WINDOWS_LOCAL__
#define __WINDOWS_LOCAL__

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <GL/gl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define APPNAME "highperf"
#define ZeroAlloc(size) calloc(1, size)

typedef char PATHSTR[1024];
typedef struct WI_Size *PSIZE2;
typedef struct WI_Size const *PCSIZE2;
typedef struct WI_Message EVENT, *PEVENT;

/* Window-class name used for RegisterClassEx / CreateWindowEx */
#define PLATFORM_WNDCLASS "PlatformWindow"

extern HWND  g_hwnd;
extern HDC   g_hdc;
extern HGLRC g_hrc;

extern int g_win_width;
extern int g_win_height;

/* Window procedure defined in windows_event.c */
extern LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

#endif /* __WINDOWS_LOCAL__ */
