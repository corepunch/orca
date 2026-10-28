#import <QuartzCore/QuartzCore.h>
#include <libs/platform/ios/ios_local.h>
#include <source/core/animation.h>
#include "ios_animation.h"

// Bound the float's range so a long session retains sub-millisecond precision.
#define ANIMATION_CLOCK_PERIOD 3600.0

@interface OrcaAnimationClockLayer : CALayer
@property(nonatomic) float timeline;
@end

@implementation OrcaAnimationClockLayer
@dynamic timeline;
- (instancetype)initWithLayer:(id)layer
{
  if ((self = [super initWithLayer:layer]) && [layer isKindOfClass:[OrcaAnimationClockLayer class]])
    self.timeline = ((OrcaAnimationClockLayer *)layer).timeline;
  return self;
}
+ (BOOL)needsDisplayForKey:(NSString *)key
{
  return [key isEqualToString:@"timeline"] || [super needsDisplayForKey:key];
}
// No backing image: every animation is evaluated and drawn by ORCA.
- (void)display {}
@end

static OrcaAnimationClockLayer *animation_layer;
static CFTimeInterval animation_begin, animation_last;

static uint32_t Animation_NativeTime(void *context)
{
  OrcaAnimationClockLayer *layer = context;
  double elapsed = MAX([layer convertTime:CACurrentMediaTime() fromLayer:nil] - animation_begin, 0);
  OrcaAnimationClockLayer *presentation = (OrcaAnimationClockLayer *)layer.presentationLayer;
  if (presentation) {
    double sample = floor(elapsed / ANIMATION_CLOCK_PERIOD) * ANIMATION_CLOCK_PERIOD + presentation.timeline;
    // Presentation may still be the previous frame at a repeat boundary.
    if (sample - elapsed > ANIMATION_CLOCK_PERIOD / 2) sample -= ANIMATION_CLOCK_PERIOD;
    else if (elapsed - sample > ANIMATION_CLOCK_PERIOD / 2) sample += ANIMATION_CLOCK_PERIOD;
    elapsed = sample;
  }
  // Installing the presentation tree must not move a just-started clock back.
  animation_last = MAX(animation_last, elapsed);
  return (uint32_t)(uint64_t)(animation_last * 1000);
}

void ORCA_IOS_SyncAnimationClock(void)
{
  if (!ios_view || animation_layer.superlayer == ios_view.layer) return;
  ORCA_IOS_ClearAnimationClock();
  animation_layer = [OrcaAnimationClockLayer new];
  animation_layer.name = @"ORCA animation timeline";
  animation_layer.bounds = CGRectMake(0, 0, 1, 1);
  animation_begin = CACurrentMediaTime();
  animation_last = 0;
  [CATransaction begin];
  [CATransaction setDisableActions:YES];
  animation_layer.timeline = ANIMATION_CLOCK_PERIOD;
  [ios_view.layer addSublayer:animation_layer];
  CABasicAnimation *clock = [CABasicAnimation animationWithKeyPath:@"timeline"];
  clock.fromValue = @0;
  clock.toValue = @(ANIMATION_CLOCK_PERIOD);
  clock.duration = ANIMATION_CLOCK_PERIOD;
  clock.beginTime = animation_begin;
  clock.repeatCount = HUGE_VALF;
  clock.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionLinear];
  if (@available(iOS 15.0, *)) clock.preferredFrameRateRange = CAFrameRateRangeMake(80, 120, 120);
  [animation_layer addAnimation:clock forKey:@"timeline"];
  [CATransaction commit];
  ANIM_SetClock(Animation_NativeTime, animation_layer);
}

void ORCA_IOS_ClearAnimationClock(void)
{
  if (!animation_layer) return;
  ANIM_SetClock(NULL, NULL);
  [animation_layer removeAllAnimations];
  [animation_layer removeFromSuperlayer];
  [animation_layer release];
  animation_layer = nil;
}

void ORCA_IOS_PauseAnimationClock(bool_t paused)
{
  ANIM_SetPaused(paused);
  if (!animation_layer || (animation_layer.speed == 0) == !!paused) return;
  if (paused) {
    CFTimeInterval time = [animation_layer convertTime:CACurrentMediaTime() fromLayer:nil];
    animation_layer.speed = 0;
    animation_layer.timeOffset = time;
  } else {
    CFTimeInterval time = animation_layer.timeOffset;
    animation_layer.speed = 1;
    animation_layer.timeOffset = 0;
    animation_layer.beginTime = 0;
    animation_layer.beginTime = [animation_layer convertTime:CACurrentMediaTime() fromLayer:nil] - time;
  }
}
