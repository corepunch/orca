#include "../platform.h"
#include "webgl_local.h"

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE g_webgl_ctx = 0;
int g_canvas_width = 0;
int g_canvas_height = 0;

bool_t
WI_CreateWindow(PCSTR title, DWORD width, DWORD height, DWORD flags)
{
  (void)title;
  (void)flags;
  (void)width;
  (void)height;

  /* WI_Init already sized the canvas when the CSS layout was ready.
   * Only fall back to resizing here if the layout was not yet available
   * at init time (g_canvas_width/height remain 0). */
  if (g_canvas_width == 0 || g_canvas_height == 0) {
    double css_width, css_height;
    emscripten_get_element_css_size("#canvas", &css_width, &css_height);
    g_canvas_width  = (int)css_width;
    g_canvas_height = (int)css_height;
    emscripten_set_canvas_element_size("#canvas",
      (int)(css_width  * WI_GetScaling() + 0.5),
      (int)(css_height * WI_GetScaling() + 0.5));
  }

  emscripten_webgl_make_context_current(g_webgl_ctx);
  printf("Created window with size %dx%d\n", g_canvas_width, g_canvas_height);
  WI_PostMessageW(NULL, kEventWindowPaint, MAKEDWORD(g_canvas_width, g_canvas_height), NULL);
  return TRUE;
}

uint32_t
WI_GetSize(struct WI_Size *size)
{
  if (size) {
    size->width = (uint32_t)g_canvas_width;
    size->height = (uint32_t)g_canvas_height;
  }
  return MAKEDWORD(g_canvas_width, g_canvas_height);
}

bool_t
WI_SetSize(uint32_t width, uint32_t height, bool_t centered)
{
  (void)centered;
  g_canvas_width = (int)width;
  g_canvas_height = (int)height;
  emscripten_set_canvas_element_size("#canvas", 
    (int)(width * WI_GetScaling() + 0.5), 
    (int)(height * WI_GetScaling() + 0.5));
  WI_PostMessageW(NULL, kEventWindowResized, MAKEDWORD(width, height), NULL);
  return TRUE;
}

#include <math.h>
float
WI_GetScaling(void)
{
  return fmaxf((float)emscripten_get_device_pixel_ratio(), 1.0f);
}

void
WI_MakeCurrentContext(void)
{
  emscripten_webgl_make_context_current(g_webgl_ctx);
}

void
WI_BeginPaint(void)
{
  emscripten_webgl_make_context_current(g_webgl_ctx);
}

void
WI_EndPaint(void)
{
  // WebGL swapping is handled by the browser automatically
}

void
WI_BindFramebuffer(void)
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool_t
WI_CreateSurface(uint32_t width, uint32_t height)
{
  (void)width;
  (void)height;
  return FALSE;
}
