#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/ES3/gl.h>

#include <libs/platform/ios/ios_local.h>
#include "ios_page_transition.h"
#include "ios_scroll.h"

@interface OrcaPageSnapshot : NSObject <CAAnimationDelegate>
@property(nonatomic, retain) UIView *overlay;
@property(nonatomic, retain) CALayer *page;
@property(nonatomic) uint64_t generation;
@property(nonatomic) BOOL hold;
@property(nonatomic) BOOL presentsWithTransaction;
@property(nonatomic) OrcaIOSPageTransitionCompletion completion;
@end

static OrcaPageSnapshot *page_snapshot;

@implementation OrcaPageSnapshot
- (void)animationDidStop:(CAAnimation *)animation finished:(BOOL)finished
{
  (void)animation;
  if (page_snapshot != self || !self.completion) return;
  OrcaIOSPageTransitionCompletion completion = self.completion;
  self.completion = NULL;
  uint64_t generation = self.generation;
  // Keep an exit's opaque cover until the next page has been rendered. An
  // entrance exposes the current live framebuffer when it reaches identity.
  if (!self.hold || !finished) {
    [self retain];
    page_snapshot = nil;
    [self.overlay removeFromSuperview];
    ((CAEAGLLayer *)ios_view.layer).presentsWithTransaction = self.presentsWithTransaction;
    ORCA_IOS_SetScrollViewsEnabled(TRUE);
    [self release]; // the global's ownership
    completion(generation, finished);
    [self release]; // this callback's ownership
  } else {
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    self.page.opacity = 0;
    [CATransaction commit];
    completion(generation, TRUE);
  }
}
- (void)dealloc
{
  [_overlay release];
  [_page release];
  [super dealloc];
}
@end

bool_t ORCA_IOS_PageTransitionActive(void) { return page_snapshot != nil; }

bool_t ORCA_IOS_CanPageTransition(void)
{
  char const *setting = getenv("ORCA_NATIVE_TRANSITIONS");
  return (!setting || strcmp(setting, "0")) && [NSThread isMainThread] &&
    ios_context && ios_view.superview && ios_external_lifecycle();
}

void ORCA_IOS_ClearPageTransition(void)
{
  OrcaPageSnapshot *old = page_snapshot;
  if (!old) return;
  page_snapshot = nil;
  OrcaIOSPageTransitionCompletion completion = old.completion;
  old.completion = NULL; // removing animations can invoke the delegate
  uint64_t generation = old.generation;
  [old.page removeAllAnimations];
  [old.overlay removeFromSuperview];
  ((CAEAGLLayer *)ios_view.layer).presentsWithTransaction = old.presentsWithTransaction;
  ORCA_IOS_SetScrollViewsEnabled(TRUE);
  if (completion) completion(generation, FALSE);
  [old release];
}

static void Snapshot_ReleasePixels(void *info, void const *data, size_t size)
{
  (void)info; (void)size;
  free((void *)data);
}

static CGImageRef Snapshot_CopyFramebuffer(void)
{
  size_t width = (size_t)llround(ios_view.bounds.size.width * ios_view.contentScaleFactor);
  size_t height = (size_t)llround(ios_view.bounds.size.height * ios_view.contentScaleFactor);
  if (!width || !height || width > SIZE_MAX / 4 / height) return NULL;
  size_t stride = width * 4, size = stride * height;
  unsigned char *pixels = malloc(size), *row = malloc(stride);
  if (!pixels || !row) { free(pixels); free(row); return NULL; }
  GLint alignment;
  glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  axBindFramebuffer();
  glReadPixels(0, 0, (GLsizei)width, (GLsizei)height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  glPixelStorei(GL_PACK_ALIGNMENT, alignment);
  // GL's origin is the bottom left; CGImage contents use top-left rows.
  for (size_t y = 0; y < height / 2; y++) {
    unsigned char *top = pixels + y * stride, *bottom = pixels + (height - y - 1) * stride;
    memcpy(row, top, stride); memcpy(top, bottom, stride); memcpy(bottom, row, stride);
  }
  free(row);
  CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, pixels, size, Snapshot_ReleasePixels);
  if (!provider) { free(pixels); return NULL; }
  CGColorSpaceRef color = CGColorSpaceCreateDeviceRGB();
  CGImageRef image = CGImageCreate(width, height, 8, 32, stride, color,
    kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast, provider, NULL, false,
    kCGRenderingIntentDefault);
  CGColorSpaceRelease(color);
  CGDataProviderRelease(provider);
  return image;
}

static CATransform3D Snapshot_Transform(float angle, float perspective)
{
  CATransform3D transform = CATransform3DIdentity;
  transform.m34 = -1 / MAX(perspective, 1);
  return CATransform3DRotate(transform, angle * M_PI / 180, 0, 1, 0);
}

bool_t ORCA_IOS_StartPageTransition(struct OrcaIOSPageTransition const *spec,
  uint64_t generation, OrcaIOSPageTransitionCompletion completion)
{
  if (!spec || !completion || !ORCA_IOS_CanPageTransition()) return FALSE;
  CGRect rect = CGRectIntersection(ios_view.bounds,
    CGRectMake(spec->x, spec->y, spec->width, spec->height));
  if (CGRectIsEmpty(rect) || spec->duration <= 0) return FALSE;
  CGImageRef image = Snapshot_CopyFramebuffer();
  if (!image) return FALSE;
  CGFloat scale = ios_view.contentScaleFactor;
  CGImageRef cropped = CGImageCreateWithImageInRect(image,
    CGRectMake(rect.origin.x * scale, rect.origin.y * scale, rect.size.width * scale, rect.size.height * scale));
  CGImageRelease(image);
  if (!cropped) return FALSE;

  // Replace the held exit only after the new page's capture is ready.
  ORCA_IOS_ClearPageTransition();
  ORCA_IOS_SetScrollViewsEnabled(FALSE);
  OrcaPageSnapshot *snapshot = [OrcaPageSnapshot new];
  snapshot.generation = generation;
  snapshot.completion = completion;
  snapshot.hold = spec->hold;
  snapshot.presentsWithTransaction = ((CAEAGLLayer *)ios_view.layer).presentsWithTransaction;
  // Presenting the new GL page and committing its covering CA snapshot must
  // share a transaction, otherwise the flat incoming page can flash first.
  ((CAEAGLLayer *)ios_view.layer).presentsWithTransaction = YES;
  snapshot.overlay = [[[UIView alloc] initWithFrame:ios_view.frame] autorelease];
  snapshot.overlay.userInteractionEnabled = NO;
  snapshot.overlay.clipsToBounds = YES;
  snapshot.overlay.backgroundColor = UIColor.clearColor;
  CALayer *cover = [CALayer layer];
  cover.frame = rect;
  cover.backgroundColor = [UIColor colorWithRed:spec->background[0] green:spec->background[1]
    blue:spec->background[2] alpha:1].CGColor;
  [snapshot.overlay.layer addSublayer:cover];
  snapshot.page = [CALayer layer];
  snapshot.page.contents = (id)cropped;
  snapshot.page.contentsScale = scale;
  snapshot.page.bounds = CGRectMake(0, 0, rect.size.width, rect.size.height);
  snapshot.page.anchorPoint = CGPointMake(-40 / rect.size.width, .5);
  snapshot.page.position = CGPointMake(rect.origin.x - 40, CGRectGetMidY(rect));
  CGImageRelease(cropped);
  [snapshot.overlay.layer addSublayer:snapshot.page];

  CATransform3D from = Snapshot_Transform(spec->fromAngle, spec->perspective);
  CATransform3D to = Snapshot_Transform(spec->toAngle, spec->perspective);
  [CATransaction begin];
  [CATransaction setDisableActions:YES];
  snapshot.page.transform = to;
  snapshot.page.opacity = 1;
  [ios_view.superview addSubview:snapshot.overlay];
  CABasicAnimation *turn = [CABasicAnimation animationWithKeyPath:@"transform"];
  turn.fromValue = [NSValue valueWithCATransform3D:from];
  turn.toValue = [NSValue valueWithCATransform3D:to];
  turn.duration = spec->duration;
  turn.timingFunction = [CAMediaTimingFunction functionWithControlPoints:
    spec->curve[0] :spec->curve[1] :spec->curve[2] :spec->curve[3]];
  turn.delegate = snapshot;
  if (@available(iOS 15.0, *)) turn.preferredFrameRateRange = CAFrameRateRangeMake(80, 120, 120);
  page_snapshot = snapshot;
  [snapshot.page addAnimation:turn forKey:@"orca.page-transition"];
  [CATransaction commit];
  return TRUE;
}
