#import <Foundation/Foundation.h>
#import <UIKit/UIApplication.h>
#import <UIKit/UIScene.h>
#import <UIKit/UISceneSession.h>
#import <UIKit/UISceneConfiguration.h>
#import <UIKit/UIWindowScene.h>
#import <QuartzCore/CADisplayLink.h>
#import <OpenGLES/ES3/gl.h>

#include <include/api.h>
#include <core/core.h>
#include <generated/UIKit/UIKit.h>
#include <libs/platform/ios/ios_local.h>
#include <unistd.h>

int luaopen_orca(lua_State *L);

@interface OrcaSceneDelegate : UIResponder <UIWindowSceneDelegate> {
  lua_State *_lua, *_coroutine;
  CADisplayLink *_displayLink;
  BOOL _stepping;
}
@end

@implementation OrcaSceneDelegate
- (void)stop
{
  [_displayLink invalidate];
  [_displayLink release];
  _displayLink = nil;
  if (_lua) lua_close(_lua);
  _lua = _coroutine = NULL;
}

- (BOOL)start
{
  NSString *resources = [NSBundle mainBundle].resourcePath;
  NSString *project = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"ORCAProject"] ?: @"project";
  project = [resources stringByAppendingPathComponent:project];
  if (chdir(project.fileSystemRepresentation)) {
    NSLog(@"ORCA: cannot open project at %@", project);
    return NO;
  }
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
  axPostMessageW(NULL, kEventWindowPaint, axGetSize(NULL), NULL);
  int nresults = 0;
  int status = lua_resume(_coroutine, _lua, 0, &nresults);
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
}

- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session
      options:(UISceneConnectionOptions *)options
{
  if (![scene isKindOfClass:[UIWindowScene class]] || ![self start]) return;
  _displayLink = [[CADisplayLink displayLinkWithTarget:self selector:@selector(frame:)] retain];
  [_displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
}

- (void)sceneWillResignActive:(UIScene *)scene
{
  _displayLink.paused = YES;
  if (_lua) { axMakeCurrentContext(); glFinish(); }
}

- (void)sceneDidBecomeActive:(UIScene *)scene { _displayLink.paused = NO; }
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
