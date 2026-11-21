#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#include <include/api.h>
#include <generated/UIKit/UIKit.h>
#include <libs/platform/ios/ios_local.h>
#include <plugins/UIKit/Accessibility.h>
#include "ios_accessibility.h"

// VoiceOver for ORCA screens (W11). The engine draws everything into one
// Metal view, which UIKit cannot see into, so after each paint this mirrors
// the screen's accessibility elements (labels, traits, hints and frames from
// plugins/UIKit/Accessibility.c) into UIAccessibilityElements held by a
// transparent container on top. Elements are reused by engine object, so
// VoiceOver keeps its place while a screen redraws; activating one taps its
// centre through the engine's own queue, so buttons act as for a finger.
// Nothing runs unless an assistive technology does.

@interface OrcaAccessibilityElement : UIAccessibilityElement
@property(nonatomic) CGPoint tapPoint;
@property(nonatomic) unsigned generation;
@end

@implementation OrcaAccessibilityElement
- (BOOL)accessibilityActivate
{
  uint32_t point = MAKEDWORD((int)CLAMP(self.tapPoint.x, 0, UINT16_MAX), (int)CLAMP(self.tapPoint.y, 0, UINT16_MAX));
  ax_pointer_t pointer = { .flags = AX_POINTER_TOUCH, .time = (uint32_t)(uint64_t)(CACurrentMediaTime() * 1000) };
  ios_post_touch(kEventLeftButtonDown, point, NULL, pointer);
  ios_post_touch(kEventLeftButtonUp, point, NULL, pointer);
  return YES;
}
@end

@interface OrcaAccessibilityContainer : UIView
@property(nonatomic, copy) NSArray *elements;
@end

@implementation OrcaAccessibilityContainer
- (BOOL)isAccessibilityElement { return NO; }
- (NSArray *)accessibilityElements { return self.elements; }
- (void)dealloc { [_elements release]; [super dealloc]; }
@end

static OrcaAccessibilityContainer *a11y_container;
static NSMutableDictionary<NSValue *, OrcaAccessibilityElement *> *a11y_elements;
static NSMutableArray *a11y_order;
static struct Object *a11y_screen;
static unsigned a11y_generation;

static UIAccessibilityTraits a11y_native_traits(int traits)
{
  switch (traits) {
    case kAccessibilityTraitsButton: return UIAccessibilityTraitButton;
    case kAccessibilityTraitsLink: return UIAccessibilityTraitLink;
    case kAccessibilityTraitsHeader: return UIAccessibilityTraitHeader;
    case kAccessibilityTraitsStaticText: return UIAccessibilityTraitStaticText;
    case kAccessibilityTraitsImage: return UIAccessibilityTraitImage;
    case kAccessibilityTraitsSummary: return UIAccessibilityTraitSummaryElement;
    default: return UIAccessibilityTraitNone;
  }
}

static NSString *a11y_string(lpcString_t text)
{
  return text && *text ? [NSString stringWithUTF8String:text] : nil;
}

static void a11y_visit(struct AccessibilityElement const *element, void *context)
{
  BOOL *changed = context;
  NSValue *key = [NSValue valueWithPointer:element->object];
  OrcaAccessibilityElement *native = a11y_elements[key];
  if (!native) {
    native = [[[OrcaAccessibilityElement alloc] initWithAccessibilityContainer:a11y_container] autorelease];
    a11y_elements[key] = native;
    *changed = YES;
  }
  // The engine's object is only a key here; it is never dereferenced.
  CGRect frame = CGRectMake(element->x, element->y, element->width, element->height);
  native.accessibilityFrameInContainerSpace = frame;
  native.tapPoint = CGPointMake(CGRectGetMidX(frame), CGRectGetMidY(frame));
  NSString *label = a11y_string(element->label), *hint = a11y_string(element->hint);
  if (![native.accessibilityLabel isEqualToString:label ?: @""]) native.accessibilityLabel = label;
  if (hint != native.accessibilityHint && ![native.accessibilityHint isEqualToString:hint]) native.accessibilityHint = hint;
  native.accessibilityTraits = a11y_native_traits(element->traits);
  if (native.generation != a11y_generation - 1) *changed = YES;
  native.generation = a11y_generation;
  [a11y_order addObject:native];
}

void ORCA_IOS_ClearAccessibility(void)
{
  [a11y_container removeFromSuperview];
  [a11y_container release];
  a11y_container = nil;
  [a11y_elements release];
  a11y_elements = nil;
  [a11y_order release];
  a11y_order = nil;
  a11y_screen = NULL;
}

void ORCA_IOS_SyncAccessibility(struct Object *screen)
{
  if (!ios_view) return;
  if (!UIAccessibilityIsVoiceOverRunning() && !UIAccessibilityIsSwitchControlRunning()) {
    if (a11y_container) ORCA_IOS_ClearAccessibility();
    return;
  }
  if (!a11y_container) {
    a11y_container = [[OrcaAccessibilityContainer alloc] initWithFrame:ios_view.bounds];
    a11y_container.userInteractionEnabled = NO; // touches still reach the engine and its scroll views
    a11y_container.backgroundColor = UIColor.clearColor;
    a11y_container.opaque = NO;
    a11y_container.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    a11y_elements = [NSMutableDictionary new];
    a11y_order = [NSMutableArray new];
  }
  if (a11y_container.superview != ios_view) [ios_view addSubview:a11y_container];
  [ios_view bringSubviewToFront:a11y_container];
  if (!CGRectEqualToRect(a11y_container.frame, ios_view.bounds)) a11y_container.frame = ios_view.bounds;

  struct Object *shown = UIKit_AccessibilityScreen(screen);
  BOOL newScreen = shown != a11y_screen, changed = newScreen;
  a11y_screen = shown;
  a11y_generation++;
  [a11y_order removeAllObjects];
  // Matrices are final once the screen has been drawn.
  UIKit_CollectAccessibility(screen, ios_view.bounds.size.width, ios_view.bounds.size.height, a11y_visit, &changed);
  for (NSValue *key in [[a11y_elements.allKeys copy] autorelease])
    if (a11y_elements[key].generation != a11y_generation) { [a11y_elements removeObjectForKey:key]; changed = YES; }
  if (!changed && [a11y_container.elements isEqualToArray:a11y_order]) return;
  a11y_container.elements = a11y_order;
  // A new screen moves VoiceOver to its first element; otherwise it stays.
  UIAccessibilityPostNotification(newScreen ? UIAccessibilityScreenChangedNotification
                                            : UIAccessibilityLayoutChangedNotification, nil);
}
