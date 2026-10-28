#import <Foundation/Foundation.h>
#import <UserNotifications/UserNotifications.h>
#include <TargetConditionals.h>
#include <string.h>
#include "notifications_native.h"

#if TARGET_OS_IOS
#import <UIKit/UIKit.h>
#else
#import <AppKit/AppKit.h>
#endif

@interface OrcaNotificationDelegate : NSObject <UNUserNotificationCenterDelegate
#if !TARGET_OS_IOS
    , NSApplicationDelegate
#endif
    >
@end

static OrcaNotificationDelegate *notificationDelegate;
static struct NotificationsEvent latestToken;

static void notification_error(char const *code, char const *message) {
  struct NotificationsEvent event = { .type = "error" };
  snprintf(event.code, sizeof(event.code), "%s", code);
  snprintf(event.message, sizeof(event.message), "%s", message);
  Notifications_Enqueue(&event);
}

static int notification_identifier(id value, char *target, size_t size) {
  if (![value isKindOfClass:[NSString class]]) return 0;
  NSData *bytes = [value dataUsingEncoding:NSUTF8StringEncoding];
  if (!bytes.length || bytes.length >= size || memchr(bytes.bytes, '\0', bytes.length)) return 0;
  memcpy(target, bytes.bytes, bytes.length);
  target[bytes.length] = '\0';
  return 1;
}

static void notification_payload(NSDictionary *payload, char const *type) {
  if (![payload isKindOfClass:[NSDictionary class]]) return;
  id nested = payload[@"data"];
  NSDictionary *data = [nested isKindOfClass:[NSDictionary class]] ? nested : payload;
  struct NotificationsEvent event = {0};
  if (!notification_identifier(data[@"conversationId"], event.conversationId, sizeof(event.conversationId)) ||
      !notification_identifier(data[@"messageId"], event.messageId, sizeof(event.messageId)) ||
      !notification_identifier(data[@"userId"], event.userId, sizeof(event.userId))) return;
  snprintf(event.type, sizeof(event.type), "%s", type);
  Notifications_Enqueue(&event);
}

int Notifications_AppleSupported(void) {
  NSBundle *bundle = [NSBundle mainBundle];
  return bundle.bundleIdentifier.length && [[bundle objectForInfoDictionaryKey:@"CFBundlePackageType"] isEqual:@"APPL"];
}

void Notifications_AppleToken(void *value) {
  NSData *data = (NSData *)value;
  if (![data isKindOfClass:[NSData class]] || !data.length || data.length > 256) {
    notification_error("invalid_token", "APNs returned an invalid device token.");
    return;
  }
  struct NotificationsEvent event = { .type = "token" };
  unsigned char const *bytes = data.bytes;
  char const *hex = "0123456789abcdef";
  for (NSUInteger i = 0; i < data.length; i++) {
    event.token[i * 2] = hex[bytes[i] >> 4];
    event.token[i * 2 + 1] = hex[bytes[i] & 15];
  }
  // Apple delivers application-delegate callbacks on the main thread.
  latestToken = event;
  Notifications_Enqueue(&event);
}

void Notifications_AppleError(void *error) {
  (void)error;
  notification_error("registration_failed", "APNs registration failed. Check the app's push entitlement, signing profile and network.");
}

void Notifications_AppleOpen(void *value) {
  UNNotificationResponse *response = (UNNotificationResponse *)value;
  if (response && ![response.actionIdentifier isEqual:UNNotificationDismissActionIdentifier])
    notification_payload(response.notification.request.content.userInfo, "open");
}

void Notifications_AppleInitialize(void) {
  if (!Notifications_AppleSupported() || notificationDelegate) return;
  UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
  if (center.delegate) return;
  notificationDelegate = [[OrcaNotificationDelegate alloc] init];
  center.delegate = notificationDelegate;
#if !TARGET_OS_IOS
  // The desktop platform has no application delegate; preserve one supplied by a host.
  if (!NSApp.delegate) NSApp.delegate = notificationDelegate;
#endif
}

void Notifications_AppleRequestAuthorization(void) {
  dispatch_async(dispatch_get_main_queue(), ^{
    Notifications_AppleInitialize();
    if (!notificationDelegate || [UNUserNotificationCenter currentNotificationCenter].delegate != notificationDelegate) {
      notification_error("unsupported", "The application already has another notification delegate.");
      return;
    }
#if !TARGET_OS_IOS
    if (NSApp.delegate != notificationDelegate) {
      notification_error("unsupported", "The application delegate cannot receive APNs registration callbacks.");
      return;
    }
#endif
    UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
    [center requestAuthorizationWithOptions:UNAuthorizationOptionAlert | UNAuthorizationOptionBadge | UNAuthorizationOptionSound
                         completionHandler:^(BOOL granted, NSError *error) {
      if (error || !granted) {
        notification_error(error ? "authorization_failed" : "permission_denied",
                           "Notification permission was not granted. Enable notifications in system settings.");
        return;
      }
      dispatch_async(dispatch_get_main_queue(), ^{
        if (latestToken.token[0]) Notifications_Enqueue(&latestToken);
#if TARGET_OS_IOS
        [[UIApplication sharedApplication] registerForRemoteNotifications];
#else
        [NSApp registerForRemoteNotifications];
#endif
      });
    }];
  });
}

@implementation OrcaNotificationDelegate
#if !TARGET_OS_IOS
+ (void)load {
  // Install before launch responses are delivered, including a notification cold start.
  [[NSNotificationCenter defaultCenter] addObserverForName:NSApplicationWillFinishLaunchingNotification
      object:nil queue:nil usingBlock:^(NSNotification *note) {
    (void)note;
    Notifications_AppleInitialize();
  }];
}
- (void)application:(NSApplication *)application didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)token {
  (void)application;
  Notifications_AppleToken(token);
}
- (void)application:(NSApplication *)application didFailToRegisterForRemoteNotificationsWithError:(NSError *)error {
  (void)application;
  Notifications_AppleError(error);
}
#endif

- (void)userNotificationCenter:(UNUserNotificationCenter *)center
      willPresentNotification:(UNNotification *)notification
        withCompletionHandler:(void (^)(UNNotificationPresentationOptions options))completionHandler {
  (void)center;
  notification_payload(notification.request.content.userInfo, "received");
  // The active application refreshes its conversation without showing a second alert.
  completionHandler(UNNotificationPresentationOptionNone);
}

- (void)userNotificationCenter:(UNUserNotificationCenter *)center
didReceiveNotificationResponse:(UNNotificationResponse *)response
        withCompletionHandler:(void (^)(void))completionHandler {
  (void)center;
  Notifications_AppleOpen(response);
  completionHandler();
}
@end
