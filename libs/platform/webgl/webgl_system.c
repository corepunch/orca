#include "../platform.h"
#include "webgl_local.h"

void
WI_Init(void)
{
  /* Size the canvas to CSS size × devicePixelRatio BEFORE creating the WebGL
   * context.  iOS Safari triggers a context-loss event when the canvas is
   * resized after context creation, silently killing the rendering surface. */
  double css_width = 0.0, css_height = 0.0;
  emscripten_get_element_css_size("#canvas", &css_width, &css_height);
  if (css_width > 0.0 && css_height > 0.0) {
    float dpr = WI_GetScaling();
    int px_w = (int)(css_width  * dpr + 0.5);
    int px_h = (int)(css_height * dpr + 0.5);
    emscripten_set_canvas_element_size("#canvas", px_w, px_h);
    g_canvas_width  = (int)css_width;
    g_canvas_height = (int)css_height;
  }

  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.majorVersion = 2;
  attrs.minorVersion = 0;
  attrs.alpha = EM_FALSE; /* opaque canvas — avoids premultiplied-alpha compositing issues on iOS Safari */
  attrs.depth = EM_TRUE;
  attrs.stencil = EM_TRUE;
  attrs.antialias = EM_FALSE;

  g_webgl_ctx = emscripten_webgl_create_context("#canvas", &attrs);
  if (g_webgl_ctx <= 0) {
    /* Fall back to WebGL 1 if WebGL 2 is not available */
    attrs.majorVersion = 1;
    g_webgl_ctx = emscripten_webgl_create_context("#canvas", &attrs);
  }

  emscripten_webgl_make_context_current(g_webgl_ctx);
  webgl_register_callbacks();
}

void
WI_Shutdown(void)
{
  emscripten_webgl_destroy_context(g_webgl_ctx);
  g_webgl_ctx = 0;
}

char const *
WI_GetPlatform(void)
{
  return "webgl";
}

char const *
WI_SettingsDirectory(void)
{
  return "/";
}

char const *
WI_ShareDirectory(void)
{
  return "/";
}

char const *
WI_LibDirectory(void)
{
  return "/";
}

longTime_t
WI_GetMilliseconds(void)
{
  return (longTime_t)emscripten_get_now();
}

void
WI_Sleep(longTime_t msec)
{
  (void)msec;
  /* Blocking sleep is not practical in a browser event loop.
   * Enable ASYNCIFY in emcc flags and call emscripten_sleep(msec) if needed. */
}

bool_t
WI_IsDarkTheme(void)
{
  /* Color-scheme detection would require a JS interop call; return FALSE for now. */
  return FALSE;
}

bool_t
WI_GetOpenFileName(WI_OpenFileName const *ofn)
{
  (void)ofn;
  return FALSE;
}

bool_t
WI_GetSaveFileName(WI_OpenFileName const *ofn)
{
  (void)ofn;
  return FALSE;
}

bool_t
WI_GetFolderName(WI_OpenFileName const *ofn)
{
  (void)ofn;
  return FALSE;
}

char const *
WI_KeynumToString(uint32_t keynum)
{
  static char tinystr[2];
  if (keynum == (uint32_t)-1)
    return "<KEY NOT FOUND>";
  keynum = keynum & 0xff;
  if (keynum > 32 && keynum < 127) {
    tinystr[0] = (char)keynum;
    tinystr[1] = 0;
    return tinystr;
  }
  return "<UNKNOWN KEYNUM>";
}
