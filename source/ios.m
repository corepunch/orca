#import <Foundation/Foundation.h>
#import <UIKit/UIApplication.h>
#import <UIKit/UIScene.h>
#import <UIKit/UISceneSession.h>
#import <UIKit/UISceneConfiguration.h>
#import <UIKit/UIWindowScene.h>
#import <QuartzCore/CADisplayLink.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/ES3/gl.h>

#include <include/api.h>
#include <core/core.h>
#include <generated/UIKit/UIKit.h>
#include <libs/platform/ios/ios_local.h>
#include <float.h>
#include <unistd.h>

int luaopen_orca(lua_State *L);

struct OrcaFrameProfile {
  BOOL enabled;
  unsigned callbacks, gaps, paints;
  double start, last, lastTimestamp;
  double resumeTotal, resumeMax, callbackTotal, callbackMax;
  double gapTotal, gapMax, timestampGapTotal, timestampGapMax;
  double durationTotal, intervalTotal, targetLeadTotal, targetLeadMin;
};

static void profile_frame(struct OrcaFrameProfile *p, CADisplayLink *link,
                           double started, double resumeDuration, double finished)
{
  if (!p->start) {
    p->start = started;
    p->targetLeadMin = link.targetTimestamp - started;
  }
  if (p->last) {
    double gap = started - p->last, timestampGap = link.timestamp - p->lastTimestamp;
    p->gapTotal += gap;
    p->gapMax = MAX(p->gapMax, gap);
    p->timestampGapTotal += timestampGap;
    p->timestampGapMax = MAX(p->timestampGapMax, timestampGap);
    p->gaps++;
  }
  p->last = started;
  p->lastTimestamp = link.timestamp;
  p->callbacks++;
  p->resumeTotal += resumeDuration;
  p->resumeMax = MAX(p->resumeMax, resumeDuration);
  p->callbackTotal += finished - started;
  p->callbackMax = MAX(p->callbackMax, finished - started);
  p->durationTotal += link.duration;
  p->intervalTotal += link.targetTimestamp - link.timestamp;
  p->targetLeadTotal += link.targetTimestamp - started;
  p->targetLeadMin = MIN(p->targetLeadMin, link.targetTimestamp - started);
  double elapsed = finished - p->start;
  if (elapsed < 1) return;
  double count = p->callbacks, gaps = MAX(p->gaps, 1u);
  unsigned paints = ios_paint_count();
  fprintf(stderr, "[ios frames] callbacks=%u paints=%u elapsed=%.3fs rate=%.2f/s paint_rate=%.2f/s "
    "resume_ms=%.2f/max%.2f callback_ms=%.2f/max%.2f "
    "wall_gap_ms=%.2f/max%.2f display_gap_ms=%.2f/max%.2f "
    "duration_ms=%.2f target_interval_ms=%.2f target_lead_ms=%.2f/min%.2f preferred_fps=%ld\n",
    p->callbacks, paints - p->paints, elapsed, count / elapsed, (paints - p->paints) / elapsed,
    p->resumeTotal * 1000 / count, p->resumeMax * 1000,
    p->callbackTotal * 1000 / count, p->callbackMax * 1000,
    p->gapTotal * 1000 / gaps, p->gapMax * 1000,
    p->timestampGapTotal * 1000 / gaps, p->timestampGapMax * 1000,
    p->durationTotal * 1000 / count, p->intervalTotal * 1000 / count,
    p->targetLeadTotal * 1000 / count, p->targetLeadMin * 1000,
    (long)link.preferredFramesPerSecond);
  *p = (struct OrcaFrameProfile){ .enabled = YES, .start = finished, .paints = paints,
    .last = started, .lastTimestamp = link.timestamp, .targetLeadMin = DBL_MAX };
}

@interface OrcaSceneDelegate : UIResponder <UIWindowSceneDelegate> {
  lua_State *_lua, *_coroutine;
  CADisplayLink *_displayLink;
  CFRunLoopObserverRef _wakeObserver;
  BOOL _stepping, _booted, _active;
  struct OrcaFrameProfile _frameProfile;
}
- (void)wake;
@end

// Rendering is event-driven: the display link only paces queued messages and
// pauses while the queue is empty. Messages wake the main run loop, and this
// observer resumes the link when one is waiting.
static void wake_display(CFRunLoopObserverRef observer, CFRunLoopActivity activity, void *delegate)
{
  (void)observer; (void)activity;
  [(OrcaSceneDelegate *)delegate wake];
}

@implementation OrcaSceneDelegate
- (void)wake
{
  // Never widen the batch of a running frame; nested run loops can call this.
  if (_stepping || !_active || !_displayLink.paused) return;
  ios_begin_message_frame();
  if (ios_messages_ready()) _displayLink.paused = NO;
}

- (void)stop
{
  if (_wakeObserver) {
    CFRunLoopRemoveObserver(CFRunLoopGetMain(), _wakeObserver, kCFRunLoopCommonModes);
    CFRelease(_wakeObserver);
    _wakeObserver = NULL;
  }
  [_displayLink invalidate];
  [_displayLink release];
  _displayLink = nil;
  if (_lua) lua_close(_lua);
  _lua = _coroutine = NULL;
}

- (BOOL)start
{
  char const *profile = getenv("ORCA_PROFILE_FRAMES");
  _frameProfile.enabled = profile && *profile && strcmp(profile, "0");
  NSString *resources = [NSBundle mainBundle].resourcePath;
  NSString *project = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"ORCAProject"] ?: @"project";
  project = [resources stringByAppendingPathComponent:project];
  if (chdir(project.fileSystemRepresentation)) {
    NSLog(@"ORCA: cannot open project at %@", project);
    return NO;
  }
  _booted = NO;
  _lua = luaL_newstate();
  if (!_lua) return NO;
  *(void **)lua_getextraspace(_lua) = NULL;
  luaL_openlibs(_lua);
  lua_pushstring(_lua, [NSBundle mainBundle].executablePath.fileSystemRepresentation);
  lua_setglobal(_lua, "EXENAME");
  lua_pushstring(_lua, [resources stringByAppendingPathComponent:@"share"].fileSystemRepresentation);
  lua_setglobal(_lua, "SHAREDIR");
  lua_pushstring(_lua, resources.fileSystemRepresentation);
  lua_setglobal(_lua, "LIBDIR");
  lua_pushstring(_lua, resources.fileSystemRepresentation);
  lua_setglobal(_lua, "PLUGDIR");
  lua_pushstring(_lua, project.fileSystemRepresentation);
  lua_setglobal(_lua, "PROJECTDIR");
  lua_pushstring(_lua, project.fileSystemRepresentation);
  lua_setglobal(_lua, "DATADIR");
  luaL_requiref(_lua, "orca", luaopen_orca, 0);
  lua_pop(_lua, 1);
  ios_set_paint_message(ID_Window_Paint);
  char const *bootstrap =
    "package.path = SHAREDIR..'/?.lua;'..SHAREDIR..'/?/init.lua;'..package.path\n"
    "local Application = require 'orca.core.application'\n"
    "local Startup = require 'orca.core.startup'\n"
    "local app = assert(Startup.open(Application, DATADIR))\n"
    "return app:run()\n";
  _coroutine = lua_newthread(_lua);
  if (luaL_loadstring(_coroutine, bootstrap) != LUA_OK) {
    NSLog(@"ORCA: %s", lua_tostring(_coroutine, -1));
    [self stop];
    return NO;
  }
  return YES;
}

- (void)frame:(CADisplayLink *)sender
{
  if (_stepping || !_coroutine) return;
  _stepping = YES;
  ios_begin_message_frame();
  // Nothing changed: draw nothing and sleep until the next message.
  if (_booted && !ios_messages_ready()) {
    _displayLink.paused = YES;
    _stepping = NO;
    return;
  }
  _booted = YES;
  double started = _frameProfile.enabled ? CACurrentMediaTime() : 0;
  int nresults = 0;
  double resumeStarted = _frameProfile.enabled ? CACurrentMediaTime() : 0;
  int status = lua_resume(_coroutine, _lua, 0, &nresults);
  double resumed = _frameProfile.enabled ? CACurrentMediaTime() : 0;
  if (status == LUA_YIELD) {
    lua_pop(_coroutine, nresults);
    struct Object *focus = core_GetFocus();
    struct Input *input = focus ? GetInput(focus) : NULL;
    UIKeyboardType keyboard = UIKeyboardTypeDefault;
    if (input) switch (input->Type) {
      case kInputTypeNumber: keyboard = UIKeyboardTypeNumberPad; break;
      case kInputTypePhone: keyboard = UIKeyboardTypePhonePad; break;
      case kInputTypeEmail: keyboard = UIKeyboardTypeEmailAddress; break;
      case kInputTypeUrl: keyboard = UIKeyboardTypeURL; break;
      case kInputTypeDecimal: keyboard = UIKeyboardTypeDecimalPad; break;
      default: break;
    }
    axIOSShowKeyboard(input && input->Type != kInputTypeCheckbox && input->Type != kInputTypeRadio,
                      keyboard, input && input->Type == kInputTypePassword);
  } else {
    BOOL reload = status == LUA_OK && lua_type(_coroutine, -1) == LUA_TSTRING;
    if (status != LUA_OK) NSLog(@"ORCA: %s", lua_tostring(_coroutine, -1));
    if (reload) {
      lua_close(_lua);
      _lua = _coroutine = NULL;
      if (![self start]) [self stop];
    } else [self stop];
  }
  _stepping = NO;
  if (_frameProfile.enabled) profile_frame(&_frameProfile, sender, started, resumed - resumeStarted, CACurrentMediaTime());
}

- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session
      options:(UISceneConnectionOptions *)options
{
  if (![scene isKindOfClass:[UIWindowScene class]] || ![self start]) return;
  _displayLink = [[CADisplayLink displayLinkWithTarget:self selector:@selector(frame:)] retain];
  // Ask for the panel's full rate on ProMotion devices; iPhones also need
  // CADisableMinimumFrameDurationOnPhone in Info.plist, or they stay at 60 Hz.
  if (@available(iOS 15.0, *)) _displayLink.preferredFrameRateRange = CAFrameRateRangeMake(80, 120, 120);
  [_displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
  CFRunLoopObserverContext context = { .info = self };
  _wakeObserver = CFRunLoopObserverCreate(NULL, kCFRunLoopBeforeWaiting | kCFRunLoopAfterWaiting,
                                          true, 0, wake_display, &context);
  CFRunLoopAddObserver(CFRunLoopGetMain(), _wakeObserver, kCFRunLoopCommonModes);
}

- (void)sceneWillResignActive:(UIScene *)scene
{
  axPostMessageW(NULL, ID_Window_Deactivated, 0, NULL);
  _active = NO;
  _displayLink.paused = YES;
  if (_lua) { axMakeCurrentContext(); glFinish(); }
}

- (void)sceneDidBecomeActive:(UIScene *)scene
{
  axPostMessageW(NULL, ID_Window_Activated, 0, NULL);
  _active = YES;
  _displayLink.paused = NO;
}
- (void)sceneDidDisconnect:(UIScene *)scene { [self stop]; }
- (void)dealloc { [self stop]; [super dealloc]; }
@end

@interface OrcaAppDelegate : UIResponder <UIApplicationDelegate>
@end

@implementation OrcaAppDelegate
- (UISceneConfiguration *)application:(UIApplication *)application
    configurationForConnectingSceneSession:(UISceneSession *)session
                                  options:(UISceneConnectionOptions *)options
{
  UISceneConfiguration *config = [[[UISceneConfiguration alloc]
      initWithName:@"ORCA" sessionRole:session.role] autorelease];
  config.delegateClass = [OrcaSceneDelegate class];
  return config;
}
@end

int main(int argc, char **argv)
{
  @autoreleasepool {
    return UIApplicationMain(argc, argv, nil, NSStringFromClass([OrcaAppDelegate class]));
  }
}
