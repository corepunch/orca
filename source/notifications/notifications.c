#include <include/api.h>
#include <notifications/notifications.h>
#include <string.h>
#include "notifications_native.h"

#if defined(__APPLE__)
#include <pthread.h>
static pthread_mutex_t notifications_lock = PTHREAD_MUTEX_INITIALIZER;
#define NOTIFICATIONS_LOCK() pthread_mutex_lock(&notifications_lock)
#define NOTIFICATIONS_UNLOCK() pthread_mutex_unlock(&notifications_lock)
#else
#define NOTIFICATIONS_LOCK() ((void)0)
#define NOTIFICATIONS_UNLOCK() ((void)0)
#endif

#define NOTIFICATIONS_CAPACITY 64
static struct NotificationsEvent notifications_events[NOTIFICATIONS_CAPACITY];
static unsigned notifications_head, notifications_count;

void Notifications_Enqueue(struct NotificationsEvent const *event) {
  NOTIFICATIONS_LOCK();
  // Scene connection and UN delegate delivery can describe the same cold-start tap.
  if (!strcmp(event->type, "open")) {
    for (unsigned i = 0; i < notifications_count; i++) {
      struct NotificationsEvent const *queued = &notifications_events[(notifications_head + i) % NOTIFICATIONS_CAPACITY];
      if (!strcmp(queued->type, "open") && !strcmp(queued->conversationId, event->conversationId) &&
          !strcmp(queued->messageId, event->messageId) && !strcmp(queued->userId, event->userId)) {
        NOTIFICATIONS_UNLOCK();
        return;
      }
    }
  }
  if (notifications_count == NOTIFICATIONS_CAPACITY) {
    notifications_head = (notifications_head + 1) % NOTIFICATIONS_CAPACITY;
    notifications_count--;
  }
  notifications_events[(notifications_head + notifications_count++) % NOTIFICATIONS_CAPACITY] = *event;
  NOTIFICATIONS_UNLOCK();
}

int Notifications_Dequeue(struct NotificationsEvent *event) {
  NOTIFICATIONS_LOCK();
  if (!notifications_count) { NOTIFICATIONS_UNLOCK(); return 0; }
  *event = notifications_events[notifications_head];
  notifications_head = (notifications_head + 1) % NOTIFICATIONS_CAPACITY;
  notifications_count--;
  NOTIFICATIONS_UNLOCK();
  return 1;
}

int notifications_Supported(lua_State *L) {
#if defined(__APPLE__)
  lua_pushboolean(L, Notifications_AppleSupported());
#else
  lua_pushboolean(L, 0);
#endif
  return 1;
}

int notifications_RequestAuthorization(lua_State *L) {
#if defined(__APPLE__)
  if (Notifications_AppleSupported()) {
    Notifications_AppleRequestAuthorization();
    lua_pushboolean(L, 1);
    return 1;
  }
#endif
  lua_pushnil(L);
  lua_pushliteral(L, "Remote notifications require an Apple application bundle.");
  return 2;
}

int notifications_Poll(lua_State *L) {
  struct NotificationsEvent event;
  if (!Notifications_Dequeue(&event)) { lua_pushnil(L); return 1; }
  lua_createtable(L, 0, 6);
#define PUSH_EVENT_FIELD(name) if (event.name[0]) { lua_pushstring(L, event.name); lua_setfield(L, -2, #name); }
  PUSH_EVENT_FIELD(type);
  PUSH_EVENT_FIELD(token);
  PUSH_EVENT_FIELD(conversationId);
  PUSH_EVENT_FIELD(messageId);
  PUSH_EVENT_FIELD(userId);
  PUSH_EVENT_FIELD(message);
  PUSH_EVENT_FIELD(code);
#undef PUSH_EVENT_FIELD
  return 1;
}

#include <notifications/notifications_export.c>
