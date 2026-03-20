#include "macos_local.h"

#import <OpenGL/gl.h>

#define USE_SINGLE_WINDOW

#define MIN_WINDOW_WIDTH 256
#define MIN_WINDOW_HEIGHT 256

WI_API uint32_t _IOSurface = -1;

//#define API_TYPE_WINDOW "Window"

//// Private notifications that are reliably dispatched when a window is moved
/// by dragging its titlebar. / The object of the notification is the window
/// being dragged. / Available in macOS 10.12+
// static NSString* const NSWindowWillStartDraggingNotification =
// @"NSWindowWillStartDraggingNotification"; static NSString* const
// NSWindowDidEndDraggingNotification = @"NSWindowDidEndDraggingNotification";

NSOpenGLPixelFormatAttribute attributes [] = {
	NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
	NSOpenGLPFADepthSize, 24,
	NSOpenGLPFAStencilSize, 8,
	NSOpenGLPFAColorSize, 24,
	NSOpenGLPFAAlphaSize, 8,
	NSOpenGLPFADoubleBuffer,
	NSOpenGLPFAAccelerated,
	NSOpenGLPFANoRecovery,
	0
};

struct wstate {
  NSWindow *Window;
  CGLContextObj ctx;
  IOSurfaceRef surf;
  GLuint texnum, framebuffer;
  GLuint width, height;
  GLfloat backingScale;
} wstate = {0};

void
WI_NotifySizeChanged(uint32_t width, uint32_t height)
{
  wstate.width = width;
  wstate.height = height;
  
  WI_PostMessageW(NULL, kEventWindowResized, MAKEDWORD(width, height), 0);
  WI_PostMessageW(NULL, kEventWindowPaint, MAKEDWORD(width, height), 0);
}

static NSRect
GetScreenFrame(uint32_t width)
{
	NSArray   *screens    = [NSScreen screens];
//	NSUInteger numScreens = [screens count];
//	return [[screens objectAtIndex:(numScreens-1)] frame];
  return [[screens objectAtIndex:0] frame];
	//    FOR_LOOP(index, (int)numScreens) {
	//        NSScreen *screen = [screens objectAtIndex:index];
	//        NSRect    frame  = [screen visibleFrame];
	//        if (frame.size.width == width) {
	//            return frame;
	//        }
	//    }
	//    return [[NSScreen mainScreen] frame];
}

static NSRect
CenterOnScreen(uint32_t width, uint32_t height)
{
	width  = MAX(MIN_WINDOW_WIDTH, width);
	height = MAX(MIN_WINDOW_HEIGHT, height);
	NSRect screenRect = GetScreenFrame(width);
	float  x = (screenRect.size.width - width) / 2 + screenRect.origin.x;
	float  y = (screenRect.size.height - height) / 2 + screenRect.origin.y;
	return CGRectMake(x, y, width, height);
}

static bool IsDarkMode(NSWindow *window) {
  NSAppearance *appearance = [window effectiveAppearance];
  NSAppearanceName match = [appearance bestMatchFromAppearancesWithNames:@[NSAppearanceNameDarkAqua, NSAppearanceNameAqua]];
  return [match isEqualToString:NSAppearanceNameDarkAqua];
}

static void ListenForDarkModeChanges(NSWindow *window) {
  [[NSDistributedNotificationCenter defaultCenter]
   addObserverForName:@"AppleInterfaceThemeChangedNotification"
   object:nil
   queue:[NSOperationQueue mainQueue]
   usingBlock:^(NSNotification * _Nonnull note) {
    if (IsDarkMode(window)) {
      NSLog(@"Switched to Dark Mode");
    } else {
      NSLog(@"Switched to Light Mode");
    }
  }];
}

static NSWindow *MakeWindow(NSRect windowRect) {
  int mask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
  return [[NSWindow alloc] initWithContentRect:windowRect
                                     styleMask:mask
                                       backing:NSBackingStoreBuffered
                                         defer:NO];
}

static NSOpenGLContext *MakeOpenGLContext(void) {
  NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
  NSOpenGLContext *context = [[NSOpenGLContext alloc] initWithFormat: pixelFormat shareContext: nil];
  [pixelFormat release];
  return context;
}

static void ConfigureOpenGLView(NSOpenGLView *openglView) {
  GLint            interval = 0;
  NSOpenGLContext *context  = MakeOpenGLContext();

  [openglView setOpenGLContext:context];
  [openglView setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
  [openglView setWantsBestResolutionOpenGLSurface:YES];
  
  [context makeCurrentContext];
  [context setValues:&interval
        forParameter:NSOpenGLContextParameterSwapInterval];
  
  // Under manual retain/release, NSOpenGLView retains the context when set.
  // Release our local ownership to avoid a leak reported by the analyzer.
  [context release];
}

bool_t
WI_CreateWindow(char const *title, uint32_t width, uint32_t height, uint32_t flags)
{
  /*
  NSOpenGLPixelFormatAttribute attrs[] = {
    NSOpenGLPFAAccelerated,
    NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
    0
  };
  
  NSOpenGLPixelFormat *pf =
  [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
  
  NSOpenGLContext *ctx =
  [[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil];
  
  // No window/view needed
  [ctx makeCurrentContext];
  
  return TRUE;
  */
  if (wstate.Window) {
//    [g_window setContentSize:NSMakeSize(width, height)];
//    [g_window setFrameOrigin:CenterOnScreen(width, height).origin];
    NSOpenGLView *view = [wstate.Window contentView];
    [[view openGLContext] makeCurrentContext];
    return TRUE;
  }
	NSRect           windowRect  = CenterOnScreen(width, height);
	NSRect           viewRect    = CGRectMake(0, 0, width, height);
	NSOpenGLView    *openGLView  = [[NSOpenGLView alloc] initWithFrame:viewRect];
	NSString        *windowTitle = [[NSString alloc] initWithUTF8String:title];
	NSWindow        *window      = MakeWindow(windowRect);
	WindowDelegate  *delegate    = [[WindowDelegate alloc] init];

  [delegate setWindow:window];
  
	[window setFrameOrigin:windowRect.origin];
	[window setTitle:windowTitle];
	[window orderFront:nil];
	[window setAcceptsMouseMovedEvents:YES];
	[window setContentView:openGLView];
	[window setDelegate:delegate];
	[window setInitialFirstResponder:openGLView];
	[window setContentSize:NSMakeSize(width, height)];
	[window orderOut:nil];
	[window display];
	[window setFrameOrigin:CenterOnScreen(width, height).origin];
	[window makeKeyAndOrderFront:nil];      // Bring window to the front and give it focus
	[window registerForDraggedTypes:[NSArray arrayWithObject:NSPasteboardTypeFileURL]];

  ConfigureOpenGLView(openGLView);
  ListenForDarkModeChanges(window);
  
  if (wstate.width != width || wstate.height != height) {
    WI_NotifySizeChanged(width, height);
  }

  assert(!wstate.surf);
  
  wstate.Window = window;
  
  if ([openGLView wantsBestResolutionOpenGLSurface]) {
    wstate.backingScale = [wstate.Window backingScaleFactor];
  } else {
    wstate.backingScale = 1;
  }
  
  //    [NSApp activateIgnoringOtherApps:YES];
  
	return FALSE;
}

void WI_Shutdown(void) {
#ifndef USE_SINGLE_WINDOW
	NSWindow *window = hWnd->window;
  [window setContentView:nil];
  [window setDelegate:nil];
	[window close];
	[window release];
  free(hWnd);
#endif
}

float
WI_GetScaling(void)
{
  return MAX(wstate.backingScale, 1);
}

uint32_t WI_GetSize(struct WI_Size * pSize) {
  if (pSize) {
    pSize->width = wstate.width;
    pSize->height = wstate.height;
  }
  return MAKEDWORD(MAX(MIN_WINDOW_WIDTH, wstate.width), MAX(MIN_WINDOW_HEIGHT, wstate.height));
}

bool_t WI_SetSize(uint32_t width, uint32_t height, bool_t centered) {
  if (wstate.width == width && wstate.height == height) {
    return TRUE;
  }
  if (wstate.Window) {
    wstate.width = MAX(MIN_WINDOW_WIDTH, width);
    wstate.height = MAX(MIN_WINDOW_HEIGHT, height);
    [wstate.Window setContentSize:NSMakeSize(wstate.width, wstate.height)];
    if (centered) {
      [wstate.Window setFrameOrigin:CenterOnScreen(wstate.width, wstate.height).origin];
      [wstate.Window makeKeyAndOrderFront:nil];
    }
    return TRUE;
  } else {
    return FALSE;
  }
}

void WI_BindFramebuffer(void) {
  if (wstate.Window) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  } else if (wstate.surf) {
    glBindFramebuffer(GL_FRAMEBUFFER, wstate.framebuffer);
  }
}

void WI_BeginPaint(void) {
  if (wstate.Window) {
    NSOpenGLView *view = [wstate.Window contentView];
    [[view openGLContext] makeCurrentContext];
  } else if (wstate.surf) {
    CGLSetCurrentContext(wstate.ctx);
  }
  WI_BindFramebuffer();
}

void WI_EndPaint(void) {
  if (wstate.Window) {
    NSOpenGLView *view = [wstate.Window contentView];
    [[view openGLContext] flushBuffer];
  }
}

void WI_MakeCurrentContext(void) {
  if (wstate.Window) {
    NSOpenGLView *view = [wstate.Window contentView];
    [[view openGLContext] makeCurrentContext];
  }
}

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>

BOOL
IOSurface_Create(uint32_t w, uint32_t h)
{
  NSDictionary* props = [NSDictionary dictionaryWithObjectsAndKeys:
                        [NSNumber numberWithBool:YES], kIOSurfaceIsGlobal,
                        [NSNumber numberWithInt:w], kIOSurfaceWidth,
                        [NSNumber numberWithInt:h], kIOSurfaceHeight,
                        [NSNumber numberWithInt:4], kIOSurfaceBytesPerElement,
                        nil];
  
  wstate.surf = IOSurfaceCreate((__bridge CFDictionaryRef)props);
  if (!wstate.surf) {
    NSLog(@"Can't create IOSurface");
    return NO;
  }
  CGLContextObj ctx = CGLGetCurrentContext();
  glGenTextures(1, &wstate.texnum);
  glGenFramebuffers(1, &wstate.framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, wstate.framebuffer);
  // Attach IOSurface to OpenGL texture
  glBindTexture(GL_TEXTURE_RECTANGLE_EXT, wstate.texnum);
  CGLTexImageIOSurface2D(ctx, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA, w, h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, wstate.surf, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_EXT, wstate.texnum, 0);
  glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 0);
  
  wstate.width = w;
  wstate.height = h;
  wstate.backingScale = 1;

  WI_PostMessageW(NULL, kEventWindowPaint, MAKEDWORD(w, h), 0);
  
  _IOSurface = IOSurfaceGetID(wstate.surf);
  fprintf(stderr, "IOSurface ID: %u\n", _IOSurface);  // Share this ID with the other app
  return YES;
}

void
IOSurface_Release(uint32_t iosurface)
{
  if (iosurface) {
    IOSurfaceRef sharedSurface = IOSurfaceLookup(iosurface);
    if (sharedSurface) {
      CFRelease(sharedSurface);
    }
  }
}

bool_t
WI_CreateSurface(uint32_t width, uint32_t height)
{
  if (wstate.surf) {
    return TRUE;
  }
  CGLPixelFormatObj pix;
  GLint npix;
  CGLChoosePixelFormat(attributes, &pix, &npix);
  CGLCreateContext(pix, NULL, &wstate.ctx);
  CGLDestroyPixelFormat(pix);
  CGLSetCurrentContext(wstate.ctx);
 
  assert(!wstate.Window);
  
  IOSurface_Create(width, height);
  
  return TRUE;
}

@implementation WindowDelegate {}

- (void) windowWillClose:(NSNotification *)aNotification {
  WI_PostMessageW(NULL, kEventWindowClosed, 0, 0);
}
//- (bool)validateMenuItem:(NSMenuItem *)menuItem {
//    NSLog(@"%@", menuItem.title);
//    return YES;
//}
- (void) windowDidResize:(NSNotification *)notification {
  uint32_t width = self.window.contentView.frame.size.width;
  uint32_t height = self.window.contentView.frame.size.height;
  //  WI_PostMessageW(NULL, kEventWindowResized, MAKEDWORD(width, height));
  WI_NotifySizeChanged(width, height);
}
-(void) windowDidChangeScreen:(NSNotification *)notification {
  WI_PostMessageW(NULL, kEventWindowChangedScreen, 0, 0);
}
- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender {
  if (([sender draggingSourceOperationMask] & NSDragOperationGeneric) == NSDragOperationGeneric) {
    return NSDragOperationGeneric;
  } else {
    return NSDragOperationNone; // no idea what to do with this, reject it.
  }
}
- (bool)performDragOperation:(id <NSDraggingInfo>)sender {
  NSPasteboard *pasteboard = [sender draggingPasteboard];
  NSArray *array = [pasteboard propertyListForType:@"NSFilenamesPboardType"];
  NSPoint mouse = [sender draggingLocation];
  float mouse_y = self.window.contentView.frame.size.height - mouse.y;
  for (NSString *path in array) {
    NSURL *fileURL = [NSURL fileURLWithPath:path];
    NSNumber *isAlias = nil;
    [fileURL getResourceValue:&isAlias
                       forKey:NSURLIsAliasFileKey
                        error:nil];
    /* If the URL is an alias, resolve it. */
    if ([isAlias boolValue]) {
      NSURLBookmarkResolutionOptions opts = NSURLBookmarkResolutionWithoutMounting | NSURLBookmarkResolutionWithoutUI;
      NSData *bookmark = [NSURL bookmarkDataWithContentsOfURL:fileURL
                                                        error:nil];
      if (bookmark != nil) {
        NSURL *resolvedURL = [NSURL URLByResolvingBookmarkData:bookmark
                                                       options:opts
                                                 relativeToURL:nil
                                           bookmarkDataIsStale:nil
                                                         error:nil];
        if (resolvedURL != nil) {
          fileURL = resolvedURL;
        }
      }
    }
    
    NotifyFileDropEvent(fileURL.path.UTF8String, mouse.x, mouse_y);
  }
  return YES;
}

@end
