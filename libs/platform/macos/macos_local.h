#ifndef __MACOS_LOCAL_H__
#define __MACOS_LOCAL_H__

#import <AppKit/AppKit.h>

#include "../platform.h"

@interface WindowDelegate : NSObject<NSWindowDelegate>
@property (assign) NSWindow* window;
@end

//@interface StatusToolBarItem: NSView
//@end

struct tracking_area *FindTrackingArea(float x, float y);

#endif
