#include "x11_local.h"
#include "../platform.h"

bool_t
WI_CreateWindow(char const* title, uint32_t width, uint32_t height, uint32_t flags)
{
  extern struct _WND window;
  (void)flags;

  if (!x_display || !x_window) {
    return FALSE;
  }

  XStoreName(x_display, x_window, title);

  if (width > 0 && height > 0 &&
      (width != (uint32_t)window.width || height != (uint32_t)window.height)) {
    XResizeWindow(x_display, x_window, width, height);
    window.width  = (int)width;
    window.height = (int)height;
  }

  XFlush(x_display);
  WI_PostMessageW(NULL, kEventWindowPaint, 0, NULL);
  return TRUE;
}

void
WI_MakeCurrentContext(void)
{
  extern struct _WND window;
  eglMakeCurrent(egl_display, window.egl_surface,
                 window.egl_surface, window.egl_context);
}

void
WI_BeginPaint(void)
{
  WI_MakeCurrentContext();
}

void
WI_EndPaint(void)
{
  extern struct _WND window;
  glColorMask(0, 0, 0, 1);
  glClearColor(1, 1, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  glColorMask(1, 1, 1, 1);
  eglSwapBuffers(egl_display, window.egl_surface);
}

void
WI_BindFramebuffer(void)
{
  /* EGL uses the default framebuffer; nothing to bind */
}

bool_t
WI_CreateSurface(uint32_t width, uint32_t height)
{
  /* Off-screen surfaces are not implemented */
  (void)width;
  (void)height;
  return FALSE;
}

float
WI_GetScaling(void)
{
  return 1.0f;
}

uint32_t
WI_GetSize(struct WI_Size* size)
{
  extern struct _WND window;
  if (size) {
    size->width  = (uint32_t)window.width;
    size->height = (uint32_t)window.height;
  }
  return MAKEDWORD(window.width, window.height);
}

bool_t
WI_SetSize(uint32_t width, uint32_t height, bool_t centered)
{
  extern struct _WND window;
  (void)centered;

  if (!x_display || !x_window) {
    return FALSE;
  }

  XResizeWindow(x_display, x_window, width, height);
  window.width  = (int)width;
  window.height = (int)height;
  XFlush(x_display);
  return TRUE;
}
