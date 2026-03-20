#include <EGL/egl.h>
#include <GLES/gl.h>
#include <ctype.h>
#include <screen/screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qnx_local.h"

screen_context_t screen_ctx;
screen_window_t screen_win;

EGLDisplay egl_display;
EGLContext egl_ctx;
EGLSurface egl_surface;

static GLint samplerLoc;
static GLuint textureId;
static GLuint programObject;

static int
initScreen()
{
  int rc;

  printf("screen_create_context\n");
  // Create the screen context
  rc = screen_create_context(&screen_ctx, SCREEN_APPLICATION_CONTEXT);
  if (rc) {
    perror("screen_create_window failed");
    return EXIT_FAILURE;
  }

  printf("screen_create_window\n");
  // Create the screen window that will be render onto
  rc = screen_create_window(&screen_win, screen_ctx);
  if (rc) {
    perror("screen_create_window failed");
    return EXIT_FAILURE;
  }

  printf("screen_set_window_property_iv\n");
  screen_set_window_property_iv(screen_win,
                                SCREEN_PROPERTY_FORMAT,
                                (int const[]){ SCREEN_FORMAT_RGBX8888 });

  printf("screen_set_window_property_iv\n");
  screen_set_window_property_iv(screen_win,
                                SCREEN_PROPERTY_USAGE,
                                (int const[]){ SCREEN_USAGE_OPENGL_ES3 });

  printf("screen_create_window_buffers\n");
  rc = screen_create_window_buffers(screen_win, 2);
  if (rc) {
    perror("screen_create_window_buffers failed");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

static int
initEGL(void)
{
  EGLBoolean rc;
  EGLConfig egl_conf = (EGLConfig)0;
  EGLint num_confs = 0;
  EGLint const egl_ctx_attr[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

  EGLint const egl_attrib_list[] = { EGL_RED_SIZE,
                                     8,
                                     EGL_GREEN_SIZE,
                                     8,
                                     EGL_BLUE_SIZE,
                                     8,
                                     EGL_ALPHA_SIZE,
                                     0,
                                     EGL_DEPTH_SIZE,
                                     0,
                                     EGL_SURFACE_TYPE,
                                     EGL_WINDOW_BIT,
                                     EGL_RENDERABLE_TYPE,
                                     EGL_OPENGL_ES3_BIT,
                                     EGL_NONE };
  printf("eglGetDisplay\n");
  egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (egl_display == EGL_NO_DISPLAY) {
    fprintf(stderr, "eglGetDisplay failed\n");
    return EXIT_FAILURE;
  }

  printf("eglInitialize\n");
  rc = eglInitialize(egl_display, NULL, NULL);
  if (rc != EGL_TRUE) {
    fprintf(stderr, "eglInitialize failed\n");
    return EXIT_FAILURE;
  }

  printf("eglChooseConfig\n");
  rc = eglChooseConfig(egl_display, egl_attrib_list, &egl_conf, 1, &num_confs);
  if ((rc != EGL_TRUE) || (num_confs == 0)) {
    fprintf(stderr, "eglChooseConfig failed\n");
    return EXIT_FAILURE;
  }

  printf("eglCreateContext\n");
  egl_ctx = eglCreateContext(
    egl_display, egl_conf, EGL_NO_CONTEXT, (EGLint*)&egl_ctx_attr);
  if (egl_ctx == EGL_NO_CONTEXT) {
    fprintf(stderr, "eglCreateContext failed\n");
    return EXIT_FAILURE;
  }

  printf("eglCreateWindowSurface\n");
  // Create the EGL surface from the screen window
  egl_surface = eglCreateWindowSurface(egl_display, egl_conf, screen_win, NULL);
  if (egl_surface == EGL_NO_SURFACE) {
    fprintf(stderr, "eglCreateWindowSurface failed\n");
    return EXIT_FAILURE;
  }

  printf("glMakeCurrent\n");
  rc = eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_ctx);
  if (rc != EGL_TRUE) {
    fprintf(stderr, "eglMakeCurrent failed\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void
WI_Init(void)
{
  int rc;

  rc = initScreen();
  if (rc != EXIT_SUCCESS) {
    fprintf(stderr, "WI_Init: initScreen failed\n");
    return;
  }

  rc = initEGL();
  if (rc != EXIT_SUCCESS) {
    fprintf(stderr, "WI_Init: initEGL failed\n");
    return;
  }
}

void
WI_Shutdown(void)
{
  eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  eglDestroySurface(egl_display, egl_surface);
  eglDestroyContext(egl_display, egl_ctx);
  eglTerminate(egl_display);
  eglReleaseThread();

  screen_destroy_window(screen_win);
  screen_destroy_context(screen_ctx);
}

bool_t
WI_CreateWindow(char const *name, uint32_t width, uint32_t height, uint32_t flags)
{
  (void)name;
  (void)flags;
  if (width > 0 && height > 0) {
    WI_PostMessageW(NULL, kEventWindowResized, MAKEDWORD(width, height), NULL);
  }
  WI_PostMessageW(NULL, kEventWindowPaint, 0, NULL);
  return TRUE;
}

void
DestroyWindow(HWND hWnd)
{
}

void
BeginPaint(HWND hWnd)
{
  eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_ctx);
}

void
EndPaint(HWND hWnd, LPMETRICS lpMetrics)
{
  eglSwapBuffers(egl_display, egl_surface);
}

float
GetWindowScale(HWND hWnd)
{
  return 1;
}

void
GetWindowSize(HWND hWnd, LPSIZE2 lpSize)
{
  EGLint surface_width;
  EGLint surface_height;

  eglQuerySurface(egl_display, egl_surface, EGL_WIDTH, &surface_width);
  eglQuerySurface(egl_display, egl_surface, EGL_HEIGHT, &surface_height);

  lpSize->width = surface_width;
  lpSize->height = surface_height;
}

char const *
WI_GetPlatform(void)
{
  return "qnx";
}

int
PollEvent(struct event* ev)
{
  // if (screen_get_event(qnx.screen_ctx, qnx.screen_ev, 0))
  // {
  return 0;
  // }
  // int val, rc;
  // rc = screen_get_event_property_iv(
  //     qnx.screen_ev, SCREEN_PROPERTY_TYPE, &val
  // );
  // if (rc || val == SCREEN_EVENT_NONE)
  // {
  //     return 0;
  // }
  // return 1;
}

char const *
WI_LibDirectory(void)
{
  return ".";
}

char const *
WI_SettingsDirectory(void)
{
  return ".";
}

char const *
WI_ShareDirectory(void)
{
  return ".";
}
