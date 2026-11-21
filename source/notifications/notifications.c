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

int notifications_SetBadgeCount(lua_State *L) {
  lua_Integer count = luaL_checkinteger(L, 1);
  luaL_argcheck(L, count >= 0, 1, "badge count must not be negative");
#if defined(__APPLE__)
  if (Notifications_AppleSupported()) {
    Notifications_AppleSetBadge((long)count);
    lua_pushboolean(L, 1);
    return 1;
  }
#endif
  lua_pushnil(L);
  lua_pushliteral(L, "The application icon badge requires an Apple application bundle.");
  return 2;
}

int notifications_SetActiveConversation(lua_State *L) {
  char const *conversationId = luaL_optstring(L, 1, NULL);
  luaL_argcheck(L, !conversationId || strlen(conversationId) < sizeof(((struct NotificationsEvent *)0)->conversationId), 1,
                "conversation identifier is too long");
#if defined(__APPLE__)
  if (Notifications_AppleSupported()) Notifications_AppleSetActiveConversation(conversationId);
#endif
  lua_pushboolean(L, 1);
  return 1;
}

static int notifications_unsupported(lua_State *L, char const *message) {
  lua_pushnil(L);
  lua_pushstring(L, message);
  return 2;
}

static char const *const notifications_local_unsupported = "Local notifications require an Apple application bundle.";

int notifications_RequestLocalAuthorization(lua_State *L) {
#if defined(__APPLE__)
  if (Notifications_AppleSupported()) {
    Notifications_AppleRequestLocalAuthorization();
    lua_pushboolean(L, 1);
    return 1;
  }
#endif
  return notifications_unsupported(L, notifications_local_unsupported);
}

static int notifications_valid_id(char const *id, size_t length) {
  if (!length || length > NOTIFICATIONS_LOCAL_ID) return 0;
  for (size_t i = 0; i < length; i++) {
    char c = id[i];
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || strchr("-_.:", c))) return 0;
  }
  return 1;
}

static char const *notifications_field(lua_State *L, char const *name, size_t limit) {
  lua_getfield(L, 1, name);
  size_t length = 0;
  char const *value = lua_tolstring(L, -1, &length);
  lua_pop(L, 1); // the table keeps the string alive
  if (!value) luaL_error(L, "notification %s must be a string", name);
  if (length > limit || memchr(value, '\0', length)) luaL_error(L, "notification %s is too long", name);
  return value;
}

int notifications_ScheduleLocal(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  char const *id = notifications_field(L, "id", NOTIFICATIONS_LOCAL_ID);
  luaL_argcheck(L, notifications_valid_id(id, strlen(id)), 1, "id must be 1-96 letters, digits, '-', '_', '.' or ':'");
  char const *title = notifications_field(L, "title", 256);
  char const *body = notifications_field(L, "body", 1024);
  lua_getfield(L, 1, "at");
  int ok = 0;
  double at = lua_tonumberx(L, -1, &ok);
  lua_pop(L, 1);
  luaL_argcheck(L, ok && at == at, 1, "at must be Unix seconds");
#if defined(__APPLE__)
  if (Notifications_AppleSupported()) {
    Notifications_AppleScheduleLocal(id, title, body, at);
    lua_pushboolean(L, 1);
    return 1;
  }
#endif
  return notifications_unsupported(L, notifications_local_unsupported);
}

int notifications_CancelLocal(lua_State *L) {
  size_t length = 0;
  char const *id = luaL_optlstring(L, 1, NULL, &length);
  luaL_argcheck(L, !id || notifications_valid_id(id, length), 1, "id must be 1-96 letters, digits, '-', '_', '.' or ':'");
#if defined(__APPLE__)
  if (Notifications_AppleSupported()) {
    Notifications_AppleCancelLocal(id);
    lua_pushboolean(L, 1);
    return 1;
  }
#endif
  return notifications_unsupported(L, notifications_local_unsupported);
}

int notifications_ScheduleRefresh(lua_State *L) {
  lua_Number seconds = luaL_checknumber(L, 1);
  luaL_argcheck(L, seconds >= 0 && seconds == seconds, 1, "seconds must not be negative");
#if defined(ORCA_IOS)
  char const *error = Notifications_AppleScheduleRefresh((double)seconds);
  if (!error) { lua_pushboolean(L, 1); return 1; }
  return notifications_unsupported(L, error);
#else
  return notifications_unsupported(L, "Background refresh needs an iPhone or iPad.");
#endif
}

#define NOTIFICATIONS_WIDGET_MAX 16384

int notifications_WriteWidgetSnapshot(lua_State *L) {
  size_t length = 0;
  char const *json = luaL_checklstring(L, 1, &length);
  luaL_argcheck(L, length <= NOTIFICATIONS_WIDGET_MAX, 1, "snapshot must be at most 16 KiB");
#if defined(ORCA_IOS)
  char const *error = Notifications_AppleWriteWidget(json, length);
  if (!error) { lua_pushboolean(L, 1); return 1; }
  return notifications_unsupported(L, error);
#else
  (void)json;
  return notifications_unsupported(L, "Widgets need an iPhone or iPad build with an App Group.");
#endif
}

#include <notifications/notifications_export.c>
