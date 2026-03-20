#include "x11_local.h"
#include "../platform.h"

#define WIDTH 640
#define HEIGHT 480

Display*   x_display    = NULL;
Window     x_window     = 0;
EGLDisplay egl_display  = EGL_NO_DISPLAY;
Atom       wm_delete_window;

static EGLConfig egl_config;

struct _WND window;

static void
create_window(struct _WND* win, int32_t width, int32_t height)
{
  int screen = DefaultScreen(x_display);
  Window root = RootWindow(x_display, screen);

  eglBindAPI(EGL_OPENGL_API);

  EGLint attributes[] = {
    EGL_RED_SIZE,     8,
    EGL_GREEN_SIZE,   8,
    EGL_BLUE_SIZE,    8,
    EGL_ALPHA_SIZE,   8,
    EGL_DEPTH_SIZE,   24,
    EGL_STENCIL_SIZE, 8,
    EGL_NONE
  };
  EGLint num_config;
  eglChooseConfig(egl_display, attributes, &egl_config, 1, &num_config);

  /* Get native visual matching EGL config */
  EGLint visual_id = 0;
  Visual* visual = NULL;
  int depth = 0;
  XVisualInfo* vi = NULL;

  eglGetConfigAttrib(egl_display, egl_config, EGL_NATIVE_VISUAL_ID, &visual_id);
  if (visual_id) {
    XVisualInfo vi_template;
    int num_visuals;
    vi_template.visualid = (VisualID)visual_id;
    vi = XGetVisualInfo(x_display, VisualIDMask, &vi_template, &num_visuals);
  }

  if (vi) {
    visual = vi->visual;
    depth = vi->depth;
  } else {
    visual = DefaultVisual(x_display, screen);
    depth = DefaultDepth(x_display, screen);
  }

  Colormap colormap = XCreateColormap(x_display, root, visual, AllocNone);
  XSetWindowAttributes swa;
  swa.colormap = colormap;
  swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
                   ButtonPressMask | ButtonReleaseMask |
                   PointerMotionMask | StructureNotifyMask |
                   FocusChangeMask;

  x_window = XCreateWindow(x_display, root,
                            0, 0, width, height, 0,
                            depth, InputOutput, visual,
                            CWColormap | CWEventMask, &swa);

  if (vi) {
    XFree(vi);
  }

  wm_delete_window = XInternAtom(x_display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(x_display, x_window, &wm_delete_window, 1);

  XMapWindow(x_display, x_window);
  XFlush(x_display);

  win->egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, NULL);
  win->egl_surface = eglCreateWindowSurface(egl_display, egl_config,
                                             (EGLNativeWindowType)x_window, NULL);
  eglMakeCurrent(egl_display, win->egl_surface, win->egl_surface, win->egl_context);

  win->width  = width;
  win->height = height;

  printf("X11 window created (%dx%d)\n", width, height);
}

static void
delete_window(struct _WND* win)
{
  if (win->egl_surface != EGL_NO_SURFACE) {
    eglDestroySurface(egl_display, win->egl_surface);
    win->egl_surface = EGL_NO_SURFACE;
  }
  if (win->egl_context != EGL_NO_CONTEXT) {
    eglDestroyContext(egl_display, win->egl_context);
    win->egl_context = EGL_NO_CONTEXT;
  }
  if (x_window) {
    XDestroyWindow(x_display, x_window);
    x_window = 0;
  }
}

void
WI_Init(void)
{
  x_display = XOpenDisplay(NULL);
  if (!x_display) {
    printf("Failed to open X display\n");
    return;
  }

  egl_display = eglGetDisplay((EGLNativeDisplayType)x_display);
  if (egl_display == EGL_NO_DISPLAY) {
    printf("Failed to get EGL display\n");
    return;
  }

  if (!eglInitialize(egl_display, NULL, NULL)) {
    printf("Failed to initialize EGL\n");
    return;
  }

  create_window(&window, WIDTH, HEIGHT);
}

void
WI_Shutdown(void)
{
  delete_window(&window);
  if (egl_display != EGL_NO_DISPLAY) {
    eglTerminate(egl_display);
    egl_display = EGL_NO_DISPLAY;
  }
  if (x_display) {
    XCloseDisplay(x_display);
    x_display = NULL;
  }
}

void
R_ReleaseIOSurface(unsigned iosurface)
{
  (void)iosurface;
}

struct _IMAGE*
R_CreatetextureFromIOSurface(unsigned surfaceID)
{
  (void)surfaceID;
  return NULL;
}

unsigned
R_CreateIOSurface(unsigned w, unsigned h, unsigned texnum)
{
  (void)w;
  (void)h;
  (void)texnum;
  return (unsigned)-1;
}
