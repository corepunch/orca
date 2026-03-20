#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include "macos_keymap.h"
#include "macos_local.h"

uint32_t KEY_GetKeyName(uint32_t keycode) {
	for (struct keymap const *km = darwin_scancode_table; km->keyname; km++) {
		if (keycode == km->keycode)
		{
			return km->keyname;
		}
	}
	return -1;
}

struct
{
  struct WI_Message data[0x10000];
  uint16_t read, write;
} queue = { 0 };

void
WI_RemoveFromQueue(void* target)
{
  for (uint16_t r = queue.read; r != queue.write; r++)
    if (queue.data[r].target == target)
      memset(&queue.data[r], 0, sizeof(queue.data[r]));
}

void
WI_PostMessageW(void* obj, uint32_t Msg, wParam_t wParam, lParam_t lParam)
{
  struct WI_Message const msg = {
    .target = obj,
    .message = Msg,
    .wParam = wParam,
    .lParam = lParam
  };
  NSEvent *customEvent =
  [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                     location:NSZeroPoint
                modifierFlags:0
                    timestamp:[NSDate timeIntervalSinceReferenceDate]
                 windowNumber:0
                      context:nil
                      subtype:queue.write
                        data1:wParam
                        data2:0];
  for (uint16_t r = queue.read; ++r != queue.write;) {
    if (queue.data[r].message != Msg)
      continue;
    switch (Msg) {
      case kEventWindowResized:
      case kEventWindowPaint:
        queue.data[r] = msg;
        return;
    }
  }
  queue.data[queue.write++] = msg;
  // Post the event to the application's event queue
  [NSApp postEvent:customEvent atStart:NO];
}

void
NotifyFileDropEvent(char const *filename, float x, float y)
{
//	struct WI_Message * ev       = malloc(sizeof(EVENT));
//	ev->type               = ;
//	ev->windowNumber       = (int)windowNumber;
//	ev->next               = window_events;
//	ev->location.x = x;
//	ev->location.y = y;
//	strcpy(ev->filename, filename);
//	window_events = ev;
}

static uint32_t
modkey(NSEventModifierFlags modifierFlags)
{
	uint32_t flags=0;
	if (modifierFlags & NSEventModifierFlagShift) {
		flags |= WI_MOD_SHIFT;
	}
	if (modifierFlags & NSEventModifierFlagCommand) {
		flags |= WI_MOD_CMD;
	}
	if (modifierFlags & NSEventModifierFlagControl) {
		flags |= WI_MOD_CTRL;
	}
	if (modifierFlags & NSEventModifierFlagOption) {
		flags |= WI_MOD_ALT;
	}
	return flags;
}

static uint32_t
GetEventType(NSEvent *event)
{
	switch (event.type)
	{
    case NSEventTypeLeftMouseDown:
      return event.clickCount == 2 ? kEventLeftDoubleClick : kEventLeftMouseDown;
    case NSEventTypeRightMouseDown:
      return event.clickCount == 2 ? kEventRightDoubleClick : kEventRightMouseDown;
    case NSEventTypeOtherMouseDown:
      return event.clickCount == 2 ? kEventOtherDoubleClick : kEventOtherMouseDown;
    case NSEventTypeLeftMouseUp: return kEventLeftMouseUp;
    case NSEventTypeRightMouseUp: return kEventRightMouseUp;
    case NSEventTypeOtherMouseUp: return kEventOtherMouseUp;
    case NSEventTypeLeftMouseDragged: return kEventLeftMouseDragged;
    case NSEventTypeRightMouseDragged: return kEventRightMouseDragged;
    case NSEventTypeOtherMouseDragged: return kEventOtherMouseDragged;
    case NSEventTypeMouseMoved: return kEventMouseMoved;
    case NSEventTypeScrollWheel: return kEventScrollWheel;
    case NSEventTypeKeyDown: return kEventKeyDown;
    case NSEventTypeKeyUp: return kEventKeyUp;
    case NSEventTypeApplicationDefined:
      return queue.data[(uint16_t)event.subtype].message;
		// case NSEventTypeMouseEntered:
		// case NSEventTypeMouseExited:
		// case NSEventTypeFlagsChanged:
		// case NSEventTypeAppKitDefined:
		// case NSEventTypeSystemDefined:
		// case NSEventTypeApplicationDefined:
		// case NSEventTypePeriodic:
		// case NSEventTypeCursorUpdate:
		// case NSEventTypeTabletPoint:
		// case NSEventTypeTabletProximity:
    default:
      return 0;
	}
}

static int
GetKeyCode(NSEvent *event)
{
	switch ([event type])
	{
	case NSEventTypeKeyDown:
	case NSEventTypeKeyUp:
		return [event keyCode];
	default:
		return -1;
	}
}

int
WI_PollEvent(struct WI_Message * e)
{
  NSEvent *event;

start_over:
  event = [NSApp nextEventMatchingMask:NSEventMaskAny
                             untilDate:[NSDate date]
                                inMode:NSDefaultRunLoopMode
                               dequeue:YES];

  if (!event)
    return 0;

  if (event.type == NSEventTypeApplicationDefined) {
    queue.read = event.subtype;
    memcpy(e, &queue.data[queue.read], sizeof(struct WI_Message));
    [event release];
    if (!e->message) {
      goto start_over;
    }
    return 1;
  } else if (event.type != NSEventTypeKeyDown) {
    [NSApp sendEvent:event];
    [NSApp updateWindows];
  }
  
  if (!GetEventType(event)) {
    [event release];
    goto start_over;
  }
  
	int x = event.locationInWindow.x;
	int y = event.window.contentView.frame.size.height - event.locationInWindow.y;

	e->target = (void *)event.window;
	e->message = GetEventType(event);
	e->wParam = MAKEDWORD(x, y);
	
	switch([event type]) {
	case NSEventTypeScrollWheel:
		e->lParam = (void*)(intptr_t)MAKEDWORD((int)event.scrollingDeltaX,
                                           (int)event.scrollingDeltaY);
		break;
	case NSEventTypeLeftMouseDragged:
	case NSEventTypeRightMouseDragged:
	case NSEventTypeOtherMouseDragged:
		e->lParam = (void*)(intptr_t)MAKEDWORD((int)event.deltaX, (int)event.deltaY);
		break;
	case NSEventTypeKeyDown:
	case NSEventTypeKeyUp:
		e->wParam = KEY_GetKeyName(GetKeyCode(event)) | modkey(event.modifierFlags);
		strncpy((char *)&e->lParam, event.characters.UTF8String, sizeof(e->lParam));
		break;
	default:
		break;
	}
	
	[event release];
	
	return 1;
}

int
WI_WaitEvent(longTime_t msec)
{
  @autoreleasepool {
    NSDate *date = msec > 0
      ? [NSDate dateWithTimeIntervalSinceNow:(double)msec / 1000.0]
      : [NSDate distantFuture];
    NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                       untilDate:date
                                          inMode:NSDefaultRunLoopMode
                                         dequeue:NO];
    return event ? 1 : 0;
  }
}
