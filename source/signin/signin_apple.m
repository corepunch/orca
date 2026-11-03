#import <Foundation/Foundation.h>
#import <AuthenticationServices/AuthenticationServices.h>
#include <TargetConditionals.h>
#include "signin_native.h"

#if TARGET_OS_IOS
#import <UIKit/UIKit.h>
#else
#import <AppKit/AppKit.h>
#endif

// Manual reference counting, like the rest of the Apple runtime sources.
@interface OrcaSigninPresenter : NSObject <ASWebAuthenticationPresentationContextProviding>
@end

static OrcaSigninPresenter *signinPresenter;
static ASWebAuthenticationSession *signinSession;

static ASPresentationAnchor signin_anchor(void) {
#if TARGET_OS_IOS
  for (UIScene *scene in [UIApplication sharedApplication].connectedScenes) {
    if (![scene isKindOfClass:[UIWindowScene class]]) continue;
    for (UIWindow *window in ((UIWindowScene *)scene).windows) if (window.isKeyWindow) return window;
    UIWindow *first = ((UIWindowScene *)scene).windows.firstObject;
    if (first) return first;
  }
  return nil;
#else
  return NSApp.keyWindow ?: NSApp.mainWindow ?: NSApp.windows.firstObject;
#endif
}

@implementation OrcaSigninPresenter
- (ASPresentationAnchor)presentationAnchorForWebAuthenticationSession:(ASWebAuthenticationSession *)session {
  (void)session;
  return signin_anchor();
}
#if !TARGET_OS_IOS
+ (void)load {
  // Links opened from Mail or a browser arrive as Apple Events, before or after launch.
  [[NSNotificationCenter defaultCenter] addObserverForName:NSApplicationWillFinishLaunchingNotification
      object:nil queue:nil usingBlock:^(NSNotification *note) {
    (void)note;
    if (!signinPresenter) signinPresenter = [[OrcaSigninPresenter alloc] init];
    [[NSAppleEventManager sharedAppleEventManager] setEventHandler:signinPresenter
        andSelector:@selector(handleURLEvent:withReplyEvent:) forEventClass:kInternetEventClass andEventID:kAEGetURL];
  }];
}
- (void)handleURLEvent:(NSAppleEventDescriptor *)event withReplyEvent:(NSAppleEventDescriptor *)reply {
  (void)reply;
  NSString *text = [[event paramDescriptorForKeyword:keyDirectObject] stringValue];
  if (text) Signin_AppleOpenURL([NSURL URLWithString:text]);
}
#endif
@end

void Signin_AppleOpenURL(void *value) {
  NSURL *url = (NSURL *)value;
  if (![url isKindOfClass:[NSURL class]] || !url.absoluteString.length) return;
  Signin_EnqueueURL("url", url.absoluteString.UTF8String);
}

static void signin_finish(ASWebAuthenticationSession *session, NSURL *url, NSError *error) {
  if (session != signinSession) return;  // A newer session replaced this one.
  [signinSession release];
  signinSession = nil;
  if (url) Signin_EnqueueURL("callback", url.absoluteString.UTF8String);
  else if ([error.domain isEqual:ASWebAuthenticationSessionErrorDomain] &&
           error.code == ASWebAuthenticationSessionErrorCodeCanceledLogin)
    Signin_EnqueueError("cancelled", "Sign-in was cancelled.");
  else Signin_EnqueueError("failed", "The sign-in page could not finish. Please try again.");
}

int Signin_AppleOpen(char const *text, char const *scheme) {
  NSURL *url = [NSURL URLWithString:[NSString stringWithUTF8String:text]];
  NSString *callback = [NSString stringWithUTF8String:scheme];
  if (!url || !callback) return 0;
  void (^start)(void) = ^{
    if (!signinPresenter) signinPresenter = [[OrcaSigninPresenter alloc] init];
    if (signinSession) {
      ASWebAuthenticationSession *previous = signinSession;
      signinSession = nil;
      [previous cancel];
      [previous release];
    }
    __block ASWebAuthenticationSession *session = nil;
    session = [[ASWebAuthenticationSession alloc] initWithURL:url callbackURLScheme:callback
        completionHandler:^(NSURL *result, NSError *error) {
      dispatch_async(dispatch_get_main_queue(), ^{ signin_finish(session, result, error); });
    }];
    session.presentationContextProvider = signinPresenter;
    // No shared cookies: each sign-in starts clean and leaves nothing in Safari.
    session.prefersEphemeralWebBrowserSession = YES;
    signinSession = session;
    if (![session start]) signin_finish(session, nil, nil);
  };
  if ([NSThread isMainThread]) start();
  else dispatch_async(dispatch_get_main_queue(), start);
  return 1;
}
