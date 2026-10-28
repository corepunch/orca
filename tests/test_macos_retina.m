#import <AppKit/AppKit.h>
#import <OpenGL/gl3.h>
#include <math.h>
#include <assert.h>
#include <include/api.h>
#include <include/renderer.h>
#include <renderer/renderer.h>
#include <UIKit/UIKit.h>

extern int luaopen_orca(lua_State *L);
extern int luaopen_orca_core(lua_State *L);
extern int luaopen_orca_renderer(lua_State *L);
extern int luaopen_orca_UIKit(lua_State *L);

static void check_drawable(NSOpenGLView *view, uint32_t width, uint32_t height, float scale)
{
  struct AXsize logical;
  axGetSize(&logical);
  assert(logical.width == width && logical.height == height);
  assert(fabs(view.bounds.size.width - width) < 0.01);
  assert(fabs(view.bounds.size.height - height) < 0.01);
  assert(fabsf(axGetScaling() - scale) < 0.01f);
  assert(R_BeginFrame((struct color){0, 0, 0, 1}) >= 0);
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  assert(viewport[0] == 0 && viewport[1] == 0);
  assert(viewport[2] == lroundf(width * scale));
  assert(viewport[3] == lroundf(height * scale));
  assert(glGetError() == GL_NO_ERROR);
  assert(R_EndFrame() >= 0);
}

static void check_icon(struct Object *icon, float scale)
{
  struct Node2D_MeasureEventArgs measure = { .Width = INFINITY, .Height = INFINITY };
  LRESULT size = OBJ_SendMessageW(icon, ID_Node2D_MeasureOverride, 0, &measure);
  assert(LOWORD(size) == 24 && HIWORD(size) == 24);
  struct Texture *texture = (void *)OBJ_SendMessageW(icon, ID_Node2D_ForegroundContent, 0, NULL);
  assert(texture && texture->Scale == lroundf(scale));
  assert(texture->Width == 24 && texture->Height == 24);
}

static unsigned drain_events(uint32_t width, uint32_t height)
{
  struct AXmessage event;
  unsigned paints = 0;
  while (axPeekMessage(&event)) {
    if (event.message == kEventWindowPaint) {
      assert(LOWORD(event.wParam) == width && HIWORD(event.wParam) == height);
      paints++;
    }
    axDisposeMessage(&event);
  }
  return paints;
}

int main(void)
{
  @autoreleasepool {
    if (![NSScreen screens].count) {
      fprintf(stderr, "UNAVAILABLE: test-macos-retina requires a macOS desktop display service\n");
      return 77;
    }
    lua_State *L = luaL_newstate();
    assert(L);
    luaL_openlibs(L);
    luaL_requiref(L, "orca", luaopen_orca, 1); lua_pop(L, 1);
    luaopen_orca_core(L); lua_pop(L, 1);
    luaopen_orca_renderer(L); lua_pop(L, 1);
    luaopen_orca_UIKit(L); lua_pop(L, 1);
    axInit();
    assert(renderer_Init(400, 300, FALSE) >= 0);

    NSWindow *window = nil;
    for (NSWindow *candidate in NSApp.windows) {
      if (![candidate.contentView isKindOfClass:NSOpenGLView.class]) continue;
      window = candidate;
      break;
    }
    assert(window && (window.styleMask & NSWindowStyleMaskResizable));
    NSOpenGLView *view = (NSOpenGLView *)window.contentView;
    assert(view.wantsBestResolutionOpenGLSurface);
    assert(view.openGLContext && glGetString(GL_VERSION));
    float backing = window.backingScaleFactor;
    check_drawable(view, 400, 300, backing);
    drain_events(400, 300);
    struct Object *icon = OBJ_Create(ID_VectorView);
    struct Property *source = OBJ_FindLongProperty(icon, ID_VectorView_Source);
    char *fixture = realpath("apps/chat/assets/icons/chevron-left.svg", NULL);
    assert(fixture && source);
    PROP_SetStringValue(source, fixture);
    free(fixture);
    struct Object_PropertyChangedEventArgs sourceChanged = { .Property = source };
    OBJ_SendMessageW(icon, ID_Object_PropertyChanged, 0, &sourceChanged);
    check_icon(icon, backing);
    puts("PASS: the default renderer window requests Retina resolution and uses physical viewport pixels");

    id<NSWindowDelegate> delegate = window.delegate;
    assert([delegate respondsToSelector:@selector(windowDidChangeBackingProperties:)]);
    NSNotification *changed = [NSNotification notificationWithName:NSWindowDidChangeBackingPropertiesNotification object:window];
    for (unsigned i = 0; i < 2; i++) {
      view.wantsBestResolutionOpenGLSurface = i != 0;
      [delegate windowDidChangeBackingProperties:changed];
      check_drawable(view, 400, 300, i ? backing : 1.f);
      check_icon(icon, i ? backing : 1.f);
      unsigned paints = drain_events(400, 300);
      if (backing > 1.f) assert(paints > 0);
    }
    puts("PASS: backing changes refresh viewport and SVG density while logical dimensions stay stable");

    assert(axSetSize(480, 360, FALSE));
    [delegate windowDidResize:[NSNotification notificationWithName:NSWindowDidResizeNotification object:window]];
    check_drawable(view, 480, 360, backing);
    assert(drain_events(480, 360) > 0);
    printf("PASS: resizing preserves logical sizing and a %.1fx physical viewport\n", backing);
    if (backing <= 1.f)
      puts("NOTE: this display is 1x; run on a Retina display to verify a multiplier above one");

    OBJ_ReleaseRef(icon);
    renderer_Shutdown();
    lua_close(L);
    axShutdown();
    return 0;
  }
}
