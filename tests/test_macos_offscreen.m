#import <AppKit/AppKit.h>
#import <OpenGL/gl3.h>
#include <libs/platform/platform.h>
#include <stdio.h>

static int check_background_state(void)
{
  if (NSApp.activationPolicy != NSApplicationActivationPolicyProhibited || NSApp.isActive ||
      NSApp.windows.count || NSApp.keyWindow || NSApp.mainWindow) {
    fprintf(stderr, "FAIL: offscreen rendering must remain in the background (policy=%ld, active=%d, windows=%lu, key=%p, main=%p)\n",
      (long)NSApp.activationPolicy, NSApp.isActive, (unsigned long)NSApp.windows.count,
      NSApp.keyWindow, NSApp.mainWindow);
    return 1;
  }
  return 0;
}

static int check_offscreen_rendering(void)
{
  axInit();
  if ((NSInteger)NSApp.activationPolicy < 0) {
    fputs("UNAVAILABLE: test-macos-offscreen requires AppKit registration in an unrestricted desktop session\n", stderr);
    return 77;
  }
  if (check_background_state()) return 1;
  if (!axCreateSurface(160, 120)) {
    fputs("FAIL: could not create the offscreen rendering surface\n", stderr);
    return 1;
  }
  struct AXsize size;
  axGetSize(&size);
  if (size.width != 160 || size.height != 120) return 1;
  axBeginPaint();
  axBindFramebuffer();
  glViewport(0, 0, 160, 120);
  glClearColor(1, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  unsigned char pixel[4] = {0};
  glReadPixels(80, 60, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
  if (glGetError() != GL_NO_ERROR || pixel[0] != 255 || pixel[1] || pixel[2] || pixel[3] != 255) {
    fputs("FAIL: the background surface must render readable Preview pixels\n", stderr);
    return 1;
  }
  axEndPaint();
  if (check_background_state()) return 1;
  puts("PASS: macOS offscreen rendering produces pixels without a window or foreground activation");
  return 0;
}

int main(void)
{
  @autoreleasepool {
    int result = check_offscreen_rendering();
    axShutdown();
    return result;
  }
}
