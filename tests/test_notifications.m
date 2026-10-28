#import <Foundation/Foundation.h>
#import <UserNotifications/UserNotifications.h>
#include <notifications/notifications.h>
#include <assert.h>
#include <string.h>

/* The platform callback seam is private; injecting bytes avoids APNs or permission dialogs. */
extern void Notifications_AppleToken(void *data);
extern void Notifications_AppleError(void *error);
extern void Notifications_AppleOpen(void *response);
extern int luaopen_orca_notifications(lua_State *L);

@interface TestNotification : NSObject
@property(retain) UNNotificationRequest *request;
@end
@implementation TestNotification
@synthesize request;
- (void)dealloc { [request release]; [super dealloc]; }
@end

@interface TestNotificationResponse : NSObject
@property(retain) TestNotification *notification;
@property(copy) NSString *actionIdentifier;
@end
@implementation TestNotificationResponse
@synthesize notification, actionIdentifier;
- (void)dealloc { [notification release]; [actionIdentifier release]; [super dealloc]; }
@end

static void check(lua_State *L, char const *script) {
  if (luaL_dostring(L, script) != LUA_OK) {
    fprintf(stderr, "%s\n", lua_tostring(L, -1));
    abort();
  }
}

static TestNotification *notification(NSDictionary *payload) {
  UNMutableNotificationContent *content = [[[UNMutableNotificationContent alloc] init] autorelease];
  content.userInfo = payload;
  TestNotification *result = [[[TestNotification alloc] init] autorelease];
  result.request = [UNNotificationRequest requestWithIdentifier:@"test" content:content trigger:nil];
  return result;
}

int main(void) {
  @autoreleasepool {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaL_requiref(L, "orca.notifications", luaopen_orca_notifications, 0);
    lua_pop(L, 1);
    check(L, "n=require 'orca.notifications'; assert(n.poll()==nil)");
    unsigned char bytes[] = {0, 1, 15, 16, 127, 128, 254, 255};
    NSMutableData *token = [NSMutableData dataWithBytes:bytes length:sizeof(bytes)];
    Notifications_AppleToken(token);
    memset(token.mutableBytes, 0xff, token.length);
    check(L, "local e=n.poll(); assert(e.type=='token' and e.token=='00010f107f80feff'); assert(n.poll()==nil)");
    Notifications_AppleToken([NSData data]);
    check(L, "local e=n.poll(); assert(e.type=='error' and #e.message>0)");

    id<UNUserNotificationCenterDelegate> delegate = [[[NSClassFromString(@"OrcaNotificationDelegate") alloc] init] autorelease];
    assert(delegate);
    NSDictionary *payload = @{@"conversationId": @"conversation", @"messageId": @"message", @"userId": @"recipient"};
    TestNotification *message = notification(@{@"data": payload});
    __block BOOL completed = NO;
    [delegate userNotificationCenter:nil willPresentNotification:(UNNotification *)message
        withCompletionHandler:^(UNNotificationPresentationOptions options) {
      assert(options == UNNotificationPresentationOptionNone);
      completed = YES;
    }];
    assert(completed);
    check(L, "local e=n.poll(); assert(e.type=='received' and e.conversationId=='conversation' and e.messageId=='message' and e.userId=='recipient')");

    TestNotificationResponse *response = [[[TestNotificationResponse alloc] init] autorelease];
    response.notification = notification(payload);
    response.actionIdentifier = UNNotificationDefaultActionIdentifier;
    Notifications_AppleOpen(response);
    completed = NO;
    [delegate userNotificationCenter:nil didReceiveNotificationResponse:(UNNotificationResponse *)response
        withCompletionHandler:^{ completed = YES; }];
    assert(completed);
    check(L, "local e=n.poll(); assert(e.type=='open' and e.userId=='recipient')");
    check(L, "assert(n.poll()==nil, 'Scene and UN delegate startup delivery must coalesce')");
    Notifications_AppleOpen(response);
    check(L, "assert(n.poll().type=='open', 'A later tap on the same message must still work')");
    response.actionIdentifier = UNNotificationDismissActionIdentifier;
    [delegate userNotificationCenter:nil didReceiveNotificationResponse:(UNNotificationResponse *)response
        withCompletionHandler:^{}];
    check(L, "assert(n.poll()==nil)");
    response.actionIdentifier = UNNotificationDefaultActionIdentifier;
    for (NSDictionary *bad in @[@{@"conversationId": @"conversation"},
          @{@"conversationId": @42, @"messageId": @"message", @"userId": @"recipient"},
          @{@"conversationId": @"conversation", @"messageId": @"message", @"userId": @""}]) {
      response.notification = notification(bad);
      [delegate userNotificationCenter:nil didReceiveNotificationResponse:(UNNotificationResponse *)response
          withCompletionHandler:^{}];
    }
    check(L, "assert(n.poll()==nil)");
    // Flooding cannot grow the native queue without a polling Lua consumer.
    for (unsigned i = 0; i < 100; i++) Notifications_AppleError(NULL);
    check(L, "for i=1,64 do assert(n.poll().type=='error') end; assert(n.poll()==nil)");
    lua_close(L);
    puts("Native notification queue, payload and Lua event tests passed");
  }
  return 0;
}
