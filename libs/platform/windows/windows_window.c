#include "windows_local.h"
#include "../platform.h"

#define DEFAULT_WIDTH  640
#define DEFAULT_HEIGHT 480

HWND  g_hwnd = NULL;
HDC   g_hdc  = NULL;
HGLRC g_hrc  = NULL;

int g_win_width  = DEFAULT_WIDTH;
int g_win_height = DEFAULT_HEIGHT;

static HINSTANCE g_hinstance = NULL;

void
WI_Init(void)
{
  g_hinstance = GetModuleHandleA(NULL);

  WNDCLASSEXA wc;
  ZeroMemory(&wc, sizeof(wc));
  wc.cbSize        = sizeof(wc);
  wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc   = WndProc;
  wc.hInstance     = g_hinstance;
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.lpszClassName = PLATFORM_WNDCLASS;

  if (!RegisterClassExA(&wc)) {
    fprintf(stderr, "Failed to register window class\n");
    return;
  }

  /* Compute window rect that gives the desired client area */
  RECT rect = { 0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT };
  AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_APPWINDOW);
  int w = rect.right  - rect.left;
  int h = rect.bottom - rect.top;

  g_hwnd = CreateWindowExA(
    WS_EX_APPWINDOW, PLATFORM_WNDCLASS, "",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, w, h,
    NULL, NULL, g_hinstance, NULL);

  if (!g_hwnd) {
    fprintf(stderr, "Failed to create window\n");
    return;
  }

  g_hdc = GetDC(g_hwnd);

  /* Choose and set a pixel format that supports OpenGL */
  PIXELFORMATDESCRIPTOR pfd;
  ZeroMemory(&pfd, sizeof(pfd));
  pfd.nSize      = sizeof(pfd);
  pfd.nVersion   = 1;
  pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 24;
  pfd.cStencilBits = 8;
  pfd.iLayerType = PFD_MAIN_PLANE;

  int fmt = ChoosePixelFormat(g_hdc, &pfd);
  if (!fmt || !SetPixelFormat(g_hdc, fmt, &pfd)) {
    fprintf(stderr, "Failed to set pixel format\n");
    return;
  }

  g_hrc = wglCreateContext(g_hdc);
  if (!g_hrc) {
    fprintf(stderr, "Failed to create WGL context\n");
    return;
  }

  wglMakeCurrent(g_hdc, g_hrc);

  ShowWindow(g_hwnd, SW_SHOW);
  UpdateWindow(g_hwnd);

  printf("Windows window created (%dx%d)\n", DEFAULT_WIDTH, DEFAULT_HEIGHT);
}

void
WI_Shutdown(void)
{
  if (g_hrc) {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_hrc);
    g_hrc = NULL;
  }
  if (g_hdc && g_hwnd) {
    ReleaseDC(g_hwnd, g_hdc);
    g_hdc = NULL;
  }
  if (g_hwnd) {
    DestroyWindow(g_hwnd);
    g_hwnd = NULL;
  }
  if (g_hinstance) {
    UnregisterClassA(PLATFORM_WNDCLASS, g_hinstance);
    g_hinstance = NULL;
  }
}

bool_t
WI_CreateWindow(char const *title, uint32_t width, uint32_t height, uint32_t flags)
{
  (void)flags;
  if (!g_hwnd) {
    return FALSE;
  }
  if (title) {
    SetWindowTextA(g_hwnd, title);
  }
  if (width > 0 && height > 0 &&
      ((int)width != g_win_width || (int)height != g_win_height)) {
    RECT rect = { 0, 0, (LONG)width, (LONG)height };
    AdjustWindowRectEx(&rect, (DWORD)GetWindowLongA(g_hwnd, GWL_STYLE),
                       FALSE, (DWORD)GetWindowLongA(g_hwnd, GWL_EXSTYLE));
    SetWindowPos(g_hwnd, NULL, 0, 0,
                 rect.right - rect.left, rect.bottom - rect.top,
                 SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
    g_win_width  = (int)width;
    g_win_height = (int)height;
  }
  return TRUE;
}

void
WI_MakeCurrentContext(void)
{
  if (g_hdc && g_hrc) {
    wglMakeCurrent(g_hdc, g_hrc);
  }
}

void
WI_BeginPaint(void)
{
  WI_MakeCurrentContext();
}

void
WI_EndPaint(void)
{
  /* Ensure alpha is fully opaque before presenting */
  glColorMask(0, 0, 0, 1);
  glClearColor(1, 1, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  glColorMask(1, 1, 1, 1);

  if (g_hdc) {
    SwapBuffers(g_hdc);
  }
}

void
WI_BindFramebuffer(void)
{
  /* WGL uses the default framebuffer; nothing to bind */
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
  if (!g_hdc) {
    return 1.0f;
  }
  int dpi = GetDeviceCaps(g_hdc, LOGPIXELSX);
  return (dpi > 0) ? ((float)dpi / 96.0f) : 1.0f;
}

uint32_t
WI_GetSize(struct WI_Size *size)
{
  if (size) {
    size->width  = (uint32_t)g_win_width;
    size->height = (uint32_t)g_win_height;
  }
  return MAKEDWORD(g_win_width, g_win_height);
}

bool_t
WI_SetSize(uint32_t width, uint32_t height, bool_t centered)
{
  if (!g_hwnd) {
    return FALSE;
  }
  UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
  int x = 0, y = 0;
  if (centered) {
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    x = (sw - (int)width)  / 2;
    y = (sh - (int)height) / 2;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
  } else {
    flags |= SWP_NOMOVE;
  }
  RECT rect = { 0, 0, (LONG)width, (LONG)height };
  AdjustWindowRectEx(&rect, (DWORD)GetWindowLongA(g_hwnd, GWL_STYLE),
                     FALSE, (DWORD)GetWindowLongA(g_hwnd, GWL_EXSTYLE));
  SetWindowPos(g_hwnd, NULL, x, y,
               rect.right - rect.left, rect.bottom - rect.top, flags);
  g_win_width  = (int)width;
  g_win_height = (int)height;
  return TRUE;
}

/* IOSurface stubs (macOS concept, not applicable on Windows) */
void
R_ReleaseIOSurface(unsigned iosurface)
{
  (void)iosurface;
}

struct _IMAGE *
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
