#ifndef ORCA_NOTIFICATIONS_NATIVE_H
#define ORCA_NOTIFICATIONS_NATIVE_H

struct NotificationsEvent {
  char type[12], token[513], conversationId[129], messageId[129], userId[129], message[256], code[32];
};

/* Copies callback data into a bounded queue; no Lua state crosses threads. */
void Notifications_Enqueue(struct NotificationsEvent const *event);
int Notifications_Dequeue(struct NotificationsEvent *event);

#if defined(__APPLE__)
int Notifications_AppleSupported(void);
void Notifications_AppleInitialize(void);
void Notifications_AppleRequestAuthorization(void);
void Notifications_AppleToken(void *data);
void Notifications_AppleError(void *error);
void Notifications_AppleOpen(void *response);
#endif

#endif
