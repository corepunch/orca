#ifndef __WAYLAND_LOCAL__
#define __WAYLAND_LOCAL__

#include <wayland-client.h>
#include <wayland-egl.h>
#include <xkbcommon/xkbcommon.h>

#include <EGL/egl.h>
#include <GL/gl.h>
#include <string.h>
#include <stdlib.h>

#include "xdg-shell-client.h"

#define ZeroAlloc(size) calloc(1, size)

typedef struct _WND
{
  EGLContext egl_context;
  EGLSurface egl_surface;
  struct wl_surface* surface;
  struct xdg_surface* xdg_surface;
  struct xdg_toplevel* xdg_toplevel;
  struct wl_egl_window* egl_window;
  struct _WND* next;
  int width;
  int height;
}* PWND;

struct vec2 {
  float x;
  float y;
};

typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef char const *PCSTR;
typedef char PATHSTR[1024];
typedef struct WI_Size *PSIZE2;
typedef struct WI_Size const *PCSIZE2;
typedef struct WI_Message EVENT, *PEVENT;
typedef struct vec2 VECTOR2D;
typedef unsigned long TIME;

extern EGLDisplay egl_display;
extern struct wl_display* display;
extern struct wl_compositor* compositor;
extern struct wl_shell* shell;
extern struct wl_shell_surface_listener shell_surface_listener;

#endif
