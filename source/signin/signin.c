#include <include/api.h>
#include <signin/signin.h>
#include <stdio.h>
#include <string.h>
#include "signin_native.h"

#if defined(__APPLE__) || defined(__ANDROID__)
#include <pthread.h>
static pthread_mutex_t signin_lock = PTHREAD_MUTEX_INITIALIZER;
#define SIGNIN_LOCK() pthread_mutex_lock(&signin_lock)
#define SIGNIN_UNLOCK() pthread_mutex_unlock(&signin_lock)
#else
#define SIGNIN_LOCK() ((void)0)
#define SIGNIN_UNLOCK() ((void)0)
#endif

#define SIGNIN_CAPACITY 16
static struct SigninEvent signin_events[SIGNIN_CAPACITY];
static unsigned signin_head, signin_count;

void Signin_Enqueue(struct SigninEvent const *event) {
  SIGNIN_LOCK();
  if (signin_count == SIGNIN_CAPACITY) {
    signin_head = (signin_head + 1) % SIGNIN_CAPACITY;
    signin_count--;
  }
  signin_events[(signin_head + signin_count++) % SIGNIN_CAPACITY] = *event;
  SIGNIN_UNLOCK();
}

void Signin_EnqueueURL(char const *type, char const *url) {
  struct SigninEvent event = {0};
  // A truncated URL would lose its secret; report it instead.
  if (!url || strlen(url) >= sizeof(event.url)) {
    Signin_EnqueueError("invalid_url", "The sign-in link is too long.");
    return;
  }
  snprintf(event.type, sizeof(event.type), "%s", type);
  snprintf(event.url, sizeof(event.url), "%s", url);
  Signin_Enqueue(&event);
}

void Signin_EnqueueError(char const *code, char const *message) {
  struct SigninEvent event = { .type = "error" };
  snprintf(event.code, sizeof(event.code), "%s", code);
  snprintf(event.message, sizeof(event.message), "%s", message);
  Signin_Enqueue(&event);
}

static int signin_dequeue(struct SigninEvent *event) {
  SIGNIN_LOCK();
  int found = signin_count > 0;
  if (found) {
    *event = signin_events[signin_head];
    signin_head = (signin_head + 1) % SIGNIN_CAPACITY;
    signin_count--;
  }
  SIGNIN_UNLOCK();
  return found;
}

#if defined(__ANDROID__)
#include <source/android/android_bridge.h>
/* A page opened by Open() returns as a callback; any other link is a url event. */
static int signin_android_pending;

void Signin_AndroidURL(char const *url) {
  SIGNIN_LOCK();
  int pending = signin_android_pending;
  signin_android_pending = 0;
  SIGNIN_UNLOCK();
  Signin_EnqueueURL(pending ? "callback" : "url", url);
}
#endif

int signin_Supported(lua_State *L) {
#if defined(__APPLE__) || defined(__ANDROID__)
  lua_pushboolean(L, 1);
#else
  lua_pushboolean(L, 0);
#endif
  return 1;
}

static int signin_valid_scheme(char const *scheme) {
  if (!scheme || !(*scheme >= 'a' && *scheme <= 'z')) return 0;
  for (char const *c = scheme; *c; c++)
    if (!((*c >= 'a' && *c <= 'z') || (*c >= '0' && *c <= '9') || *c == '+' || *c == '-' || *c == '.')) return 0;
  return strlen(scheme) <= 128;
}

int signin_Open(lua_State *L, char const *url, char const *callbackScheme) {
  if (!url || strncmp(url, "https://", 8) || strlen(url) >= 4096)
    return lua_pushnil(L), lua_pushliteral(L, "Sign-in pages must use https."), 2;
  if (!signin_valid_scheme(callbackScheme))
    return lua_pushnil(L), lua_pushliteral(L, "The callback scheme must be a lowercase URL scheme."), 2;
#if defined(__APPLE__)
  if (Signin_AppleOpen(url, callbackScheme)) return lua_pushboolean(L, 1), 1;
  return lua_pushnil(L), lua_pushliteral(L, "The sign-in page could not be opened."), 2;
#elif defined(__ANDROID__)
  SIGNIN_LOCK();
  signin_android_pending = 1;
  SIGNIN_UNLOCK();
  if (Android_OpenURL(url)) return lua_pushboolean(L, 1), 1;
  SIGNIN_LOCK();
  signin_android_pending = 0;
  SIGNIN_UNLOCK();
  return lua_pushnil(L), lua_pushliteral(L, "The sign-in page could not be opened."), 2;
#else
  return lua_pushnil(L), lua_pushliteral(L, "Browser sign-in is not available on this platform."), 2;
#endif
}

int signin_Poll(lua_State *L) {
  struct SigninEvent event;
  if (!signin_dequeue(&event)) return lua_pushnil(L), 1;
  lua_createtable(L, 0, 4);
#define PUSH_EVENT_FIELD(name) if (event.name[0]) { lua_pushstring(L, event.name); lua_setfield(L, -2, #name); }
  PUSH_EVENT_FIELD(type);
  PUSH_EVENT_FIELD(url);
  PUSH_EVENT_FIELD(code);
  PUSH_EVENT_FIELD(message);
#undef PUSH_EVENT_FIELD
  return 1;
}

#include <signin/signin_export.c>
