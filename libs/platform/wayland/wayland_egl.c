#include "wayland_local.h"

#define WIDTH 640
#define HEIGHT 480

struct wl_display* display;
struct wl_compositor* compositor = NULL;
struct wl_seat* seat = NULL;
struct xdg_wm_base* xdg_wm_base = NULL;
struct xkb_context* xkb_ctx;
struct xkb_keymap* xkb_keymap;
struct xkb_state* xkb_state;
static char running = 1;

EGLDisplay egl_display;

struct wl_seat_listener*
get_seat_listener(void);
struct xdg_surface_listener*
get_xdg_surface_listener(void);

static void
xdg_wm_base_ping(void* data, struct xdg_wm_base* wm_base, uint32_t serial)
{
  xdg_wm_base_pong(wm_base, serial);
}

static struct xdg_wm_base_listener xdg_wm_base_listener = { &xdg_wm_base_ping };

// listeners
static void
registry_add_object(void* data,
                    struct wl_registry* registry,
                    uint32_t name,
                    char const* interface,
                    uint32_t version)
{
  // printf("add_object %s\n", interface);
  if (!strcmp(interface, "wl_compositor")) {
    compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 1);
  } else if (!strcmp(interface, "xdg_wm_base")) {
    xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    xdg_wm_base_add_listener(xdg_wm_base, &xdg_wm_base_listener, NULL);
  } else if (strcmp(interface, "wl_seat") == 0) {
    seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
    wl_seat_add_listener(seat, get_seat_listener(), NULL);
  }
}
static void
registry_remove_object(void* data, struct wl_registry* registry, uint32_t name)
{
  printf("remove_object: %u\n", name);
}

static struct wl_registry_listener registry_listener = {
  &registry_add_object,
  &registry_remove_object
};

static void
shell_surface_configure(void* data,
                        struct wl_shell_surface* shell_surface,
                        uint32_t edges,
                        int32_t width,
                        int32_t height)
{
  struct _WND* window = data;
  wl_egl_window_resize(window->egl_window, width, height, 0, 0);
}

static void
create_window(struct _WND* window, int32_t width, int32_t height)
{
  if (!compositor || !xdg_wm_base) {
    printf("Compositor or xdg_wm_base is NULL!\n");
    return;
  }

  eglBindAPI(EGL_OPENGL_API);
  EGLint attributes[] = { 
    EGL_RED_SIZE, 8, 
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8, 
    EGL_ALPHA_SIZE, 8, 
    EGL_DEPTH_SIZE, 24,
    EGL_STENCIL_SIZE, 8,
    EGL_NONE 
  };
  EGLConfig config;
  EGLint num_config;
  eglChooseConfig(egl_display, attributes, &config, 1, &num_config);

  window->egl_context = eglCreateContext(egl_display, config, EGL_NO_CONTEXT, NULL);

  printf("Creating Wayland surface\n");
  window->surface = wl_compositor_create_surface(compositor);
  if (!window->surface) {
    printf("Failed to create Wayland surface!\n");
    return;
  }

  printf("Creating xdg_surface\n");
  window->xdg_surface = xdg_wm_base_get_xdg_surface(xdg_wm_base, window->surface);
  if (!window->xdg_surface) {
    printf("Failed to create xdg_surface!\n");
    return;
  }

  xdg_surface_add_listener(window->xdg_surface, get_xdg_surface_listener(), NULL);

  window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
  if (!window->xdg_toplevel) {
    printf("Failed to create xdg_toplevel!\n");
    return;
  }
  xdg_toplevel_set_title(window->xdg_toplevel, "Wayland Window");

  window->egl_window = wl_egl_window_create(window->surface, width, height);
  if (!window->egl_window) {
    printf("Failed to create EGL window!\n");
    return;
  }
  EGLint surface_attributes[] = {
    EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_SRGB,
    EGL_NONE
  };
  window->egl_surface = eglCreateWindowSurface(egl_display, config, window->egl_window, surface_attributes);
  if (!window->egl_window) {
    printf("Failed to create sRGB EGL surface, trying regular RGB!\n");
    window->egl_surface = eglCreateWindowSurface(egl_display, config, window->egl_window, NULL);
  } else {
    printf("Created sRGB EGL surface\n");
  }
  if (!window->egl_window) {
    printf("Failed to create EGL surface!\n");
    return;
  }
  eglMakeCurrent(egl_display, window->egl_surface, window->egl_surface, window->egl_context);
  printf("Window created successfully\n");

  wl_surface_commit(window->surface);

  wl_display_roundtrip(display);
}

static void
delete_window(struct _WND* window)
{
  eglDestroySurface(egl_display, window->egl_surface);
  wl_egl_window_destroy(window->egl_window);
  xdg_toplevel_destroy(window->xdg_toplevel);
  xdg_surface_destroy(window->xdg_surface);
  wl_surface_destroy(window->surface);
  eglDestroyContext(egl_display, window->egl_context);
}
static void
draw_window(struct _WND* window)
{
  glClearColor(0.0, 1.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  eglSwapBuffers(egl_display, window->egl_surface);
}

struct _WND window;

void
WI_Init(void)
{
  display = wl_display_connect(NULL);
  struct wl_registry* registry = wl_display_get_registry(display);
  wl_registry_add_listener(registry, &registry_listener, NULL);
  wl_display_roundtrip(display);

  egl_display = eglGetDisplay(display);
  eglInitialize(egl_display, NULL, NULL);

  xkb_ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  if (!xkb_ctx) {
    printf("Failed to create XKB context\n");
    return;
  }

  struct xkb_rule_names names = { .layout = "us" };
  xkb_keymap =
    xkb_keymap_new_from_names(xkb_ctx, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);

  if (!xkb_keymap) {
    printf("Failed to create XKB keymap\n");
    return;
  }

  xkb_state = xkb_state_new(xkb_keymap);
  if (!xkb_state) {
    printf("Failed to create XKB state\n");
    return;
  }

  create_window(&window, WIDTH, HEIGHT);
}

void
WI_Shutdown(void)
{
  xkb_state_unref(xkb_state);
  xkb_keymap_unref(xkb_keymap);
  xkb_context_unref(xkb_ctx);
  delete_window(&window);
  eglTerminate(egl_display);
  wl_display_disconnect(display);
}

void
R_ReleaseIOSurface(unsigned iosurface)
{
}

struct _IMAGE*
R_CreatetextureFromIOSurface(unsigned surfaceID)
{
  return NULL;
}

unsigned
R_CreateIOSurface(unsigned w, unsigned h, unsigned texnum)
{
  return -1;
}

// int main (void) {
// 	WI_Init();

// 	struct _WND window;
// 	create_window (&window, WIDTH, HEIGHT);

// 	while (running) {
// 		wl_display_dispatch_pending (display);
// 		draw_window (&window);
// 	}

// 	delete_window (&window);

// 	WI_Shutdown();
// 	return 0;
// }
