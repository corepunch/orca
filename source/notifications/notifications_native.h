#ifndef ORCA_NOTIFICATIONS_NATIVE_H
#define ORCA_NOTIFICATIONS_NATIVE_H

struct NotificationsEvent {
  char type[12], token[513], conversationId[129], messageId[129], userId[129], message[256], code[32];
};

/* Copies callback data into a bounded queue; no Lua state crosses threads. */
void Notifications_Enqueue(struct NotificationsEvent const *event);
int Notifications_Dequeue(struct NotificationsEvent *event);

#define NOTIFICATIONS_LOCAL_ID 96

#if defined(__APPLE__)
int Notifications_AppleSupported(void);
void Notifications_AppleInitialize(void);
void Notifications_AppleRequestAuthorization(void);
void Notifications_AppleToken(void *data);
void Notifications_AppleError(void *error);
void Notifications_AppleOpen(void *response);
void Notifications_AppleSetBadge(long count);
void Notifications_AppleSetActiveConversation(char const *conversationId);
/* Local notifications: identifiers are prefixed so remote ones are never touched. */
void Notifications_AppleRequestLocalAuthorization(void);
void Notifications_AppleScheduleLocal(char const *id, char const *title, char const *body, double at);
void Notifications_AppleCancelLocal(char const *id);
#endif

#if defined(ORCA_IOS)
/* Background refresh (notifications_refresh.m): register at launch, before
   application:didFinishLaunchingWithOptions: returns; NULL or an error. */
void Notifications_AppleRegisterRefresh(void);
char const *Notifications_AppleScheduleRefresh(double seconds);
/* Widgets (N3): widget.json in the ORCAAppGroup container; NULL or an error. */
char const *Notifications_AppleWriteWidget(char const *json, unsigned long length);
#endif

#endif
