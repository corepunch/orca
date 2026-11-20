// Background refresh on iOS: a BGAppRefreshTask runs the project's
// background.lua in a Lua state of its own, best effort. The state has no
// screens and never loads the engine's object system (orca.core and
// orca.filesystem are small stand-ins), so it cannot disturb an application
// state suspended beside it; it shares only the main thread, between frames.
#include <TargetConditionals.h>

#if TARGET_OS_IOS && defined(ORCA_IOS)
#import <UIKit/UIKit.h>
#import <BackgroundTasks/BackgroundTasks.h>
#include <include/api.h>
#include <core/core.h>
#include <sys/stat.h>
#include <errno.h>
#include "notifications_native.h"

int luaopen_orca(lua_State *L);

#define REFRESH_STEP_SECONDS 0.1

static NSString *refresh_identifier(void) {
  NSString *bundle = [NSBundle mainBundle].bundleIdentifier;
  return bundle.length ? [bundle stringByAppendingString:@".refresh"] : nil;
}

// BGTaskScheduler throws for an identifier the bundle does not permit.
static BOOL refresh_permitted(void) {
  NSArray *permitted = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"BGTaskSchedulerPermittedIdentifiers"];
  NSString *identifier = refresh_identifier();
  return identifier && [permitted isKindOfClass:[NSArray class]] && [permitted containsObject:identifier];
}

static NSString *refresh_project(void) {
  NSString *resources = [NSBundle mainBundle].resourcePath;
  NSString *project = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"ORCAProject"] ?: @"project";
  return [resources stringByAppendingPathComponent:project];
}

// orca.core without the object system: the device's region only.
static int refresh_region(lua_State *L) { lua_pushstring(L, core_GetRegion()); return 1; }
static int refresh_measurement(lua_State *L) { lua_pushstring(L, core_GetMeasurementSystem()); return 1; }
static int refresh_clock(lua_State *L) { lua_pushboolean(L, core_Uses24HourClock()); return 1; }
static int refresh_language(lua_State *L) { lua_pushstring(L, core_GetPreferredLanguage()); return 1; }
static int refresh_core(lua_State *L) {
  luaL_newlib(L, ((luaL_Reg[]){ { "getRegion", refresh_region }, { "getMeasurementSystem", refresh_measurement },
    { "uses24HourClock", refresh_clock }, { "getPreferredLanguage", refresh_language }, { NULL, NULL } }));
  return 1;
}

// orca.filesystem without packages: plain paths only.
static int refresh_exists(lua_State *L) {
  struct stat info;
  lua_pushboolean(L, stat(luaL_checkstring(L, 1), &info) == 0);
  return 1;
}
static int refresh_mkdir(lua_State *L) {
  lua_pushboolean(L, mkdir(luaL_checkstring(L, 1), 0700) == 0 || errno == EEXIST);
  return 1;
}
static int refresh_filesystem(lua_State *L) {
  luaL_newlib(L, ((luaL_Reg[]){ { "fileExists", refresh_exists }, { "makeDirectory", refresh_mkdir }, { NULL, NULL } }));
  return 1;
}

@interface OrcaRefreshRun : NSObject {
@public
  lua_State *lua, *thread;
  BGTask *task;
  NSTimer *timer;
}
- (void)finish:(BOOL)success;
@end

static OrcaRefreshRun *refresh_running;

@implementation OrcaRefreshRun
- (BOOL)start {
  NSString *project = refresh_project();
  if (![[NSFileManager defaultManager] fileExistsAtPath:[project stringByAppendingPathComponent:@"background.lua"]])
    return NO;
  NSString *resources = [NSBundle mainBundle].resourcePath;
  lua = luaL_newstate();
  if (!lua) return NO;
  *(void **)lua_getextraspace(lua) = NULL;
  luaL_openlibs(lua);
  lua_pushstring(lua, [resources stringByAppendingPathComponent:@"share"].fileSystemRepresentation);
  lua_setglobal(lua, "SHAREDIR");
  lua_pushstring(lua, project.fileSystemRepresentation);
  lua_setglobal(lua, "PROJECTDIR");
  lua_pushstring(lua, project.fileSystemRepresentation);
  lua_setglobal(lua, "DATADIR");
  lua_pushliteral(lua, "ios");
  lua_setglobal(lua, "PLATFORM");
  lua_pushboolean(lua, 1);
  lua_setglobal(lua, "BACKGROUND");
  luaL_requiref(lua, "orca", luaopen_orca, 0);
  lua_pop(lua, 1);
  luaL_getsubtable(lua, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  lua_pushcfunction(lua, refresh_core); lua_setfield(lua, -2, "orca.core");
  lua_pushcfunction(lua, refresh_filesystem); lua_setfield(lua, -2, "orca.filesystem");
  lua_pop(lua, 1);
  // "<package Name>/x" resolves to the project's x.lua, as the application's
  // file system does; the hook is project/background.lua returning a function.
  char const *bootstrap =
    "package.path = SHAREDIR..'/?.lua;'..SHAREDIR..'/?/init.lua;'..package.path\n"
    "local env = {}\n"
    "local package_lua = loadfile(PROJECTDIR..'/package.lua', 't', env)\n"
    "if package_lua then pcall(package_lua) end\n"
    "local prefix = type(env.Name) == 'string' and env.Name..'/' or nil\n"
    "table.insert(package.searchers, 2, function(name)\n"
    "  if not prefix or name:sub(1, #prefix) ~= prefix then return nil end\n"
    "  local path = PROJECTDIR..'/'..name:sub(#prefix + 1)..'.lua'\n"
    "  local loader, err = loadfile(path)\n"
    "  if not loader then return '\\n\\t'..tostring(err) end\n"
    "  return loader, path\n"
    "end)\n"
    "local hook = dofile(PROJECTDIR..'/background.lua')\n"
    "assert(type(hook) == 'function', 'background.lua must return a function')\n"
    "return hook()\n";
  thread = lua_newthread(lua);
  if (luaL_loadstring(thread, bootstrap) != LUA_OK) {
    NSLog(@"ORCA background: %s", lua_tostring(thread, -1));
    return NO;
  }
  timer = [[NSTimer scheduledTimerWithTimeInterval:REFRESH_STEP_SECONDS target:self selector:@selector(step:)
                                           userInfo:nil repeats:YES] retain];
  [self step:timer];
  return YES;
}

// Resumes the hook until it returns; network.fetch and other yields poll
// their work on each resume.
- (void)step:(NSTimer *)sender {
  (void)sender;
  if (!thread) return;
  int results = 0, status = lua_resume(thread, lua, 0, &results);
  if (status == LUA_YIELD) { lua_pop(thread, results); return; }
  if (status != LUA_OK) NSLog(@"ORCA background: %s", lua_tostring(thread, -1));
  BOOL success = status == LUA_OK && !(results > 0 && lua_isboolean(thread, -1) && !lua_toboolean(thread, -1));
  [self finish:success];
}

- (void)finish:(BOOL)success {
  [timer invalidate];
  [timer release];
  timer = nil;
  if (lua) {
    // The network module's shutdown would cancel every request in the
    // process, the application's too: detach it before closing.
    lua_getfield(lua, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
    lua_getfield(lua, -1, "orca.network");
    if (lua_istable(lua, -1)) { lua_pushnil(lua); lua_setmetatable(lua, -2); }
    lua_pop(lua, 2);
    lua_close(lua);
  }
  lua = thread = NULL;
  [task setTaskCompletedWithSuccess:success];
  [task release];
  task = nil;
  if (refresh_running == self) {
    [[self retain] autorelease];
    [refresh_running release];
    refresh_running = nil;
  }
}
@end

static void refresh_run(BGTask *task) {
  // The next one is asked for first, so a refresh that fails still repeats.
  Notifications_AppleScheduleRefresh(3 * 3600);
  if (refresh_running) { [task setTaskCompletedWithSuccess:NO]; return; }
  OrcaRefreshRun *run = [[OrcaRefreshRun alloc] init];
  run->task = [task retain];
  refresh_running = run;
  task.expirationHandler = ^{ if (refresh_running == run) [run finish:NO]; };
  if (![run start]) [run finish:NO];
}

void Notifications_AppleRegisterRefresh(void) {
  if (!refresh_permitted()) return;
  [[BGTaskScheduler sharedScheduler] registerForTaskWithIdentifier:refresh_identifier()
                                                        usingQueue:dispatch_get_main_queue()
                                                     launchHandler:^(BGTask *task) { refresh_run(task); }];
}

char const *Notifications_AppleScheduleRefresh(double seconds) {
  if (!refresh_permitted())
    return "Background refresh is not declared: add \"background-refresh\" to Capabilities in package.lua.";
  BGAppRefreshTaskRequest *request = [[BGAppRefreshTaskRequest alloc] initWithIdentifier:refresh_identifier()];
  request.earliestBeginDate = [NSDate dateWithTimeIntervalSinceNow:seconds];
  NSError *error = nil;
  BOOL submitted = [[BGTaskScheduler sharedScheduler] submitTaskRequest:request error:&error];
  [request release];
  // The simulator has no scheduler (BGTaskSchedulerErrorCodeUnavailable).
  return submitted ? NULL : "The system did not accept a background refresh.";
}
#endif
