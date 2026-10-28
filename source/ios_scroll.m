#import <UIKit/UIKit.h>
#include <include/api.h>
#include <include/renderer.h>
#include <generated/UIKit/UIKit.h>
#include <libs/platform/ios/ios_local.h>
#include "ios_scroll.h"
#include "ios_scroll_policy.h"

@class OrcaNativeScrollView;

static NSMutableDictionary<NSValue *, OrcaNativeScrollView *> *scroll_views;
static struct Object *scroll_screen;
static unsigned scroll_generation;

static bool_t orca_scroll_native_visible(struct Object *object)
{
  if (!scroll_screen) return FALSE;
  for (struct Object *node = object; node; node = OBJ_GetParent(node)) {
    if (GetNode(node) && OBJ_IsHidden(node)) return FALSE;
    if (GetPivot(node) && GetPivot(node)->TransitionMode != kTransitionModeNone) return FALSE;
    if (node == scroll_screen) return TRUE;
  }
  return FALSE;
}

static void orca_scroll_post_touch(UITouch *touch, uint32_t event, CGPoint point, CGPoint delta)
{
  ax_pointer_t pointer = { .flags = touch.type == UITouchTypePencil ? AX_POINTER_STYLUS : AX_POINTER_TOUCH,
    .time = (uint32_t)(uint64_t)(touch.timestamp * 1000) };
  ios_post_touch(event, MAKEDWORD((int)CLAMP(point.x, 0, UINT16_MAX), (int)CLAMP(point.y, 0, UINT16_MAX)),
    event == kEventLeftButtonDragged ? (void *)(intptr_t)MAKEDWORD((int)delta.x, (int)delta.y) : NULL, pointer);
}

// Delay an engine press until a tap or horizontal drag is known. A native
// vertical pan must never arm a row button or start the C scroll integrator.
@interface OrcaScrollTouchView : UIView
@property(nonatomic, strong) UITouch *touch;
@property(nonatomic) CGPoint origin;
@property(nonatomic) BOOL horizontal, stoppedDeceleration;
- (void)cancelEngineTouch;
@end

@interface OrcaNativeScrollView : UIScrollView <UIScrollViewDelegate>
@property(nonatomic) struct Object *object;
@property(nonatomic) unsigned generation;
@property(nonatomic) BOOL syncing;
@property(nonatomic) BOOL stoppingTouch;
@property(nonatomic) BOOL offsetPending;
@property(nonatomic) CGFloat appliedOffset;
@property(nonatomic, strong) OrcaScrollTouchView *touchContent;
- (void)queueOffset;
- (void)detach;
@end

@implementation OrcaScrollTouchView
- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
  UITouch *touch = touches.anyObject;
  if (touch.type == UITouchTypePencil) { [ios_view touchesBegan:touches withEvent:event]; return; }
  if (self.touch) return;
  self.touch = touch;
  self.origin = [touch locationInView:ios_view];
  self.horizontal = NO;
  OrcaNativeScrollView *scroll = (OrcaNativeScrollView *)self.superview;
  self.stoppedDeceleration = scroll.stoppingTouch || scroll.decelerating;
  scroll.stoppingTouch = NO;
}
- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
  if (touches.anyObject.type == UITouchTypePencil) { [ios_view touchesMoved:touches withEvent:event]; return; }
  if (![touches containsObject:self.touch]) return;
  CGPoint point = [self.touch locationInView:ios_view];
  CGFloat dx = point.x - self.origin.x, dy = point.y - self.origin.y;
  if (!self.horizontal) {
    if (fabs(dx) < 8 || fabs(dx) <= fabs(dy)) return;
    self.horizontal = YES;
    orca_scroll_post_touch(self.touch, kEventLeftButtonDown, self.origin, CGPointZero);
  }
  CGPoint previous = [self.touch previousLocationInView:ios_view];
  orca_scroll_post_touch(self.touch, kEventLeftButtonDragged, point, CGPointMake(point.x - previous.x, point.y - previous.y));
}
- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
  if (touches.anyObject.type == UITouchTypePencil) { [ios_view touchesEnded:touches withEvent:event]; return; }
  if (![touches containsObject:self.touch]) return;
  CGPoint point = [self.touch locationInView:ios_view];
  if (self.horizontal || !self.stoppedDeceleration) {
    if (!self.horizontal) {
      orca_scroll_post_touch(self.touch, kEventLeftButtonDown, point, CGPointZero);
      if (self.touch.tapCount == 2) orca_scroll_post_touch(self.touch, kEventLeftDoubleClick, point, CGPointZero);
    }
    orca_scroll_post_touch(self.touch, kEventLeftButtonUp, point, CGPointZero);
  }
  self.touch = nil;
}
- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
  if (touches.anyObject.type == UITouchTypePencil) { [ios_view touchesCancelled:touches withEvent:event]; return; }
  if (![touches containsObject:self.touch]) return;
  [self cancelEngineTouch];
}
- (void)cancelEngineTouch
{
  if (self.touch && self.horizontal)
    orca_scroll_post_touch(self.touch, kEventLeftButtonUp, CGPointMake(UINT16_MAX, UINT16_MAX), CGPointZero);
  self.touch = nil;
  self.horizontal = NO;
}
- (void)dealloc { self.touch = nil; [super dealloc]; }
@end

@implementation OrcaNativeScrollView
- (BOOL)gestureRecognizerShouldBegin:(UIGestureRecognizer *)recognizer
{
  if (recognizer == self.panGestureRecognizer) {
    CGPoint velocity = [self.panGestureRecognizer velocityInView:self];
    if (!orca_scroll_native_visible(self.object) || fabs(velocity.y) <= fabs(velocity.x)) return NO;
  }
  return [super gestureRecognizerShouldBegin:recognizer];
}
- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event
{
  if (self.decelerating) self.stoppingTouch = YES;
  return orca_scroll_native_visible(self.object) ? [super hitTest:point withEvent:event] : nil;
}
- (BOOL)touchesShouldCancelInContentView:(UIView *)view { return YES; }
- (void)queueOffset
{
  if (!self.syncing && self.object && orca_scroll_native_visible(self.object)) {
    self.offsetPending = YES;
    OBJ_RequestAnimate(self.object);
  }
}
- (void)scrollViewDidScroll:(UIScrollView *)scrollView { [self queueOffset]; }
- (void)scrollViewWillBeginDragging:(UIScrollView *)scrollView { [self queueOffset]; }
- (void)scrollViewDidEndDragging:(UIScrollView *)scrollView willDecelerate:(BOOL)decelerate
{
  if (!self.object) return;
  if (!GetNode2D(self.object)->InertiaScrolling && decelerate)
    [self setContentOffset:self.contentOffset animated:NO];
  [self queueOffset];
}
- (void)scrollViewDidEndDecelerating:(UIScrollView *)scrollView { [self queueOffset]; }
- (void)detach
{
  self.delegate = nil;
  struct Node2D *node = self.object ? GetNode2D(self.object) : NULL;
  if (node) node->_scrollTracking = node->_scrollAnimating = FALSE;
  self.object = NULL;
  [self.touchContent cancelEngineTouch];
  [self setContentOffset:self.contentOffset animated:NO];
  [self removeFromSuperview];
}
- (void)dealloc { [self detach]; self.touchContent = nil; [super dealloc]; }
@end

static OrcaNativeScrollView *orca_scroll_native_scroll(struct Object *object)
{
  return scroll_views[[NSValue valueWithPointer:object]];
}

bool_t ORCA_IOS_IsNativeScroll(struct Object *object)
{
  return orca_scroll_native_scroll(object) != nil && orca_scroll_native_visible(object);
}

bool_t ORCA_IOS_IsNativeScrollActive(struct Object *object)
{
  OrcaNativeScrollView *scroll = orca_scroll_native_scroll(object);
  return scroll && orca_scroll_native_visible(object) && (scroll.tracking || scroll.dragging || scroll.decelerating);
}

bool_t ORCA_IOS_ApplyNativeScroll(struct Object *object)
{
  OrcaNativeScrollView *scroll = orca_scroll_native_scroll(object);
  if (!scroll || !orca_scroll_native_visible(object)) return FALSE;
  if (!scroll.offsetPending) return TRUE;
  scroll.offsetPending = NO;
  struct Node2D *node = GetNode2D(object);
  struct vec2 offset = node->ContentOffset;
  offset.y = ORCA_IOS_ScrollOffset(scroll.contentOffset.y);
  node->_scrollWriting = TRUE;
  PROP_SetValue(OBJ_FindLongProperty(object, ID_Node2D_ContentOffset), &offset);
  node->_scrollWriting = FALSE;
  node->_scrollTracking = scroll.tracking || scroll.dragging;
  node->_scrollAnimating = FALSE; // UIKit owns deceleration; C must not re-enqueue itself.
  node->_scrollVelocity = (struct vec2){0};
  scroll.appliedOffset = offset.y;
  return TRUE;
}

void ORCA_IOS_NativeScrollOffsetChanged(struct Object *object)
{
  OrcaNativeScrollView *scroll = orca_scroll_native_scroll(object);
  if (!scroll) return;
  struct Node2D *node = GetNode2D(object);
  scroll.syncing = YES;
  [scroll setContentOffset:CGPointMake(0, ORCA_IOS_ScrollOffset(node->ContentOffset.y)) animated:NO];
  scroll.appliedOffset = node->ContentOffset.y;
  scroll.offsetPending = NO;
  scroll.syncing = NO;
}

void ORCA_IOS_ForgetScrollView(struct Object *object)
{
  if (object == scroll_screen) { ORCA_IOS_ClearScrollViews(); return; }
  NSValue *key = [NSValue valueWithPointer:object];
  [scroll_views[key] detach];
  [scroll_views removeObjectForKey:key];
}

void ORCA_IOS_ClearScrollViews(void)
{
  for (OrcaNativeScrollView *scroll in scroll_views.allValues) [scroll detach];
  [scroll_views removeAllObjects];
  scroll_screen = NULL;
}

static CGRect orca_scroll_native_rect(struct Node2D *node)
{
  return CGRectMake(node->Matrix.v[12], node->Matrix.v[13],
    Node2D_GetFrame(node, kBox3FieldWidth), Node2D_GetFrame(node, kBox3FieldHeight));
}

static bool_t orca_scroll_axis_aligned(struct Node2D *node)
{
  float *m = node->Matrix.v;
  return fabsf(m[0] - 1) < 0.001f && fabsf(m[5] - 1) < 0.001f &&
    fabsf(m[1]) < 0.001f && fabsf(m[4]) < 0.001f && fabsf(m[3]) < 0.001f && fabsf(m[7]) < 0.001f;
}

static void orca_scroll_sync_node(struct Object *object, CGRect clip)
{
  if (GetPivot(object) && GetPivot(object)->TransitionMode != kTransitionModeNone) return;
  struct Node2D *node = GetNode2D(object);
  if (!node || OBJ_IsHidden(object) || node->_opacity <= 0 || node->IgnoreHitTest || !orca_scroll_axis_aligned(node)) return;
  CGRect rect = orca_scroll_native_rect(node);
  if (node->Overflow.y != kOverflowVisible || node->Overflow.x != kOverflowVisible)
    clip = CGRectIntersection(clip, rect);
  if (CGRectIsEmpty(clip) || CGRectIsNull(clip)) return;

  double extent = ORCA_IOS_ScrollExtent(node->_node->Size.Axis[1].Scroll, rect.size.height);
  if (node->Overflow.y == kOverflowScroll && extent > 0.5 && !GetInput(object)) {
    CGRect viewport = rect;
    viewport.origin.y += node->AdjustedContentInset.Axis[1]._RIGHT;
    viewport.size.height -= node->AdjustedContentInset.Axis[1]._RIGHT + node->AdjustedContentInset.Axis[1]._LEFT;
    viewport = CGRectIntersection(viewport, clip);
    if (CGRectIsEmpty(viewport) || CGRectIsNull(viewport)) return;
    NSValue *key = [NSValue valueWithPointer:object];
    OrcaNativeScrollView *scroll = scroll_views[key];
    if (!scroll) {
      scroll = [[OrcaNativeScrollView alloc] initWithFrame:viewport];
      scroll.object = object; // Destroy unregisters before the engine object is freed.
      node->_scrollTracking = node->_scrollAnimating = FALSE;
      node->_scrollVelocity = (struct vec2){0};
      scroll.backgroundColor = UIColor.clearColor;
      scroll.opaque = NO;
      scroll.contentInsetAdjustmentBehavior = UIScrollViewContentInsetAdjustmentNever;
      scroll.showsHorizontalScrollIndicator = scroll.showsVerticalScrollIndicator = NO;
      scroll.directionalLockEnabled = YES;
      scroll.delaysContentTouches = NO;
      scroll.canCancelContentTouches = YES;
      scroll.panGestureRecognizer.allowedTouchTypes = @[@(UITouchTypeDirect)];
      scroll.decelerationRate = ORCA_IOS_ScrollRetention(UIScrollViewDecelerationRateNormal, 1.5);
      scroll.touchContent = [[[OrcaScrollTouchView alloc] initWithFrame:CGRectZero] autorelease];
      scroll.touchContent.backgroundColor = UIColor.clearColor;
      [scroll addSubview:scroll.touchContent];
      scroll.delegate = scroll;
      scroll_views[key] = scroll;
      [ios_view addSubview:scroll];
      [scroll release];
      scroll.appliedOffset = node->ContentOffset.y;
    }
    scroll.generation = scroll_generation;
    [ios_view bringSubviewToFront:scroll];
    scroll.syncing = YES;
    if (!CGRectEqualToRect(scroll.frame, viewport)) scroll.frame = viewport;
    scroll.bounces = node->RubberBandScrolling;
    scroll.alwaysBounceVertical = node->RubberBandScrolling;
    CGSize contentSize = CGSizeMake(viewport.size.width, viewport.size.height + extent);
    if (!CGSizeEqualToSize(scroll.contentSize, contentSize)) scroll.contentSize = contentSize;
    CGRect contentFrame = CGRectMake(0, 0, contentSize.width, contentSize.height);
    if (!CGRectEqualToRect(scroll.touchContent.frame, contentFrame)) scroll.touchContent.frame = contentFrame;
    BOOL moving = scroll.tracking || scroll.dragging || scroll.decelerating;
    if (!moving || fabs(node->ContentOffset.y - scroll.appliedOffset) > 0.01)
      [scroll setContentOffset:CGPointMake(0, ORCA_IOS_ScrollOffset(node->ContentOffset.y)) animated:NO];
    scroll.appliedOffset = node->ContentOffset.y;
    scroll.syncing = NO;
  }
  FOR_EACH_OBJECT(child, object) orca_scroll_sync_node(child, clip);
}

void ORCA_IOS_SyncScrollViews(struct Object *screen)
{
  if (!ios_view || !screen) return;
  // Screen's renderer paints later modal siblings over its ordinary content.
  for (struct Object *next = OBJ_GetNext(screen); next; next = OBJ_GetNext(next))
    if (GetScreen(next) && GetScreen(next)->Visible) screen = next;
  if (scroll_screen != screen) ORCA_IOS_ClearScrollViews();
  scroll_screen = screen;
  if (!scroll_views) scroll_views = [NSMutableDictionary new];
  scroll_generation++;
  // Matrices and content extents are final after the screen has been drawn.
  orca_scroll_sync_node(screen, ios_view.bounds);
  for (NSValue *key in [[scroll_views.allKeys copy] autorelease]) {
    OrcaNativeScrollView *scroll = scroll_views[key];
    if (scroll.generation != scroll_generation) { [scroll detach]; [scroll_views removeObjectForKey:key]; }
  }
}
