#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

struct queued_payload {
  struct StructDesc const *desc;
  max_align_t alignment;
  unsigned char bytes[];
};

static void
dispose_struct_payload(void *value)
{
  struct queued_payload *payload = (void *)((char *)value - offsetof(struct queued_payload, bytes));
  core_ClearStructPayload(payload->desc, value);
  free(payload);
}

static struct StructDesc const *
message_payload_desc(void *target, uint32_t event)
{
  struct Property *property = target ? OBJ_FindLongProperty(target, event) : NULL;
  if (property && PROP_GetType(property) == kDataTypeEvent)
    return OBJ_FindStructDesc(PROP_GetDesc(property)->TypeString);
  struct ClassDesc const *action = OBJ_FindClassW(event);
  if (!action) return NULL;
  char name[256];
  int length = snprintf(name, sizeof(name), "%sEventArgs", action->ClassName);
  if (length < 0 || (size_t)length >= sizeof(name)) return NULL;
  for (char *p = name; *p; p++) if (*p == '.') *p = '_';
  return OBJ_FindStructDesc(name);
}

ORCA_API void
axPostMessageDataW(void *target, uint32_t event, uint32_t wparam,
                   void const *value, size_t size)
{
  struct AXmessage message = { .target = target, .message = event, .wParam = wparam };
  if (!value || !size) { axPostMessageOwnedW(message); return; }
  struct StructDesc const *desc = message_payload_desc(target, event);
  if (desc) {
    if (desc->StructSize != size || size > SIZE_MAX - sizeof(struct queued_payload)) return;
    struct queued_payload *payload = calloc(1, sizeof(*payload) + size);
    if (!payload) return;
    payload->desc = desc;
    if (!core_CloneStructPayload(desc, payload->bytes, value)) { free(payload); return; }
    message.lParam = payload->bytes;
    if (!axOwnMessageData(&message, payload->bytes, dispose_struct_payload)) return;
  } else {
    void *data = malloc(size);
    if (!data) return;
    memcpy(data, value, size);
    message.lParam = data;
    if (!axOwnMessageData(&message, data, free)) return;
  }
  axPostMessageOwnedW(message);
}

#define MESSAGE_STATS 48
static struct MessageStat message_stats[MESSAGE_STATS];
static bool_t message_stats_enabled;

ORCA_API void
SV_EnableMessageStats(bool_t enable)
{
  message_stats_enabled = enable;
  if (!enable) memset(message_stats, 0, sizeof(message_stats));
}

static void
record_message(struct AXmessage const *msg)
{
  char const *className = msg->target ? OBJ_GetClassName(msg->target) : NULL;
  struct MessageStat *stat = message_stats;
  for (; stat < message_stats + MESSAGE_STATS && stat->count; stat++) {
    if (stat->message == msg->message && stat->className == className) break;
  }
  if (stat == message_stats + MESSAGE_STATS) return;   // table full: drop the rest
  stat->message = msg->message;
  stat->className = className;
  stat->count++;
}

static int
compare_message_stats(void const *a, void const *b)
{
  uint32_t x = ((struct MessageStat const *)a)->count, y = ((struct MessageStat const *)b)->count;
  return x < y ? 1 : x > y ? -1 : 0;
}

ORCA_API int
SV_TakeMessageStats(struct MessageStat *out, int max)
{
  qsort(message_stats, MESSAGE_STATS, sizeof(*message_stats), compare_message_stats);
  int n = 0;
  for (; n < max && n < MESSAGE_STATS && message_stats[n].count; n++) out[n] = message_stats[n];
  memset(message_stats, 0, sizeof(message_stats));
  return n;
}

static int
push_event_message(lua_State *L, struct AXmessage *msg)
{
  static Window_PaintMsg_t wnd;
  if (message_stats_enabled) record_message(msg);
  wnd.WindowWidth = LOWORD(msg->wParam);
  wnd.WindowHeight = HIWORD(msg->wParam);
  switch (msg->message) {
    case kEventWindowPaint:
      msg->message = ID_Window_Paint;
      msg->lParam = &wnd;
      break;
    case kEventWindowResized:
      msg->message = ID_Window_Resized;
      msg->lParam = &wnd;
      break;
    case kEventWindowChangedScreen:
      msg->message = ID_Window_ChangedScreen;
      break;
    case kEventWindowClosed:
      msg->message = ID_Window_Closed;
      break;
  }
  struct AXmessage* out = lua_newuserdata(L, sizeof(struct AXmessage));
  luaL_setmetatable(L, "Event");
  memcpy(out, msg, sizeof(struct AXmessage));
  return 1;
}

#ifdef ORCA_IOS
static int resume_get_message(lua_State *L, int status, lua_KContext context);
#endif

int f_get_message(lua_State* L)
{
  struct AXmessage msg = {0};
  if (axPeekMessage(&msg)) {
    OBJ_BeginMessage(msg.target);
    return push_event_message(L, &msg);
  }
#ifdef ORCA_IOS
  return lua_yieldk(L, 0, 0, resume_get_message);
#else
  /* Wait up to 500ms so file-change detection runs even when the window
     receives no input events (e.g. after a CSS-only save). */
  if (axWaitMessage(500) > 0 && axPeekMessage(&msg)) {
    OBJ_BeginMessage(msg.target);
    return push_event_message(L, &msg);
  }
  /* Timeout: synthesise a timer tick so the Lua loop can check
     hasChangedFiles() without needing real window activity. */
  msg.message = kEventTimer;
  return push_event_message(L, &msg);
#endif
}

#ifdef ORCA_IOS
static int resume_get_message(lua_State *L, int status, lua_KContext context)
{
  return f_get_message(L);
}
#endif

int f_peek_message(lua_State* L)
{
  struct AXmessage msg = {0};
  if (!axPeekMessage(&msg)) {
    return 0;
  }
  OBJ_BeginMessage(msg.target);
  return push_event_message(L, &msg);
}

int f_event_new(lua_State* L);
int f_dispatch_message(lua_State* L)
{
  if (lua_istable(L, 1)) {
    lua_pushcfunction(L, f_event_new);
    lua_insert(L, -2);
    lua_call(L, 1, 1);
    struct AXmessage *ev = luaL_checkudata(L, -1, "Event");
    lua_pushboolean(L, SV_DispatchMessage(L, ev));
  } else {
    struct AXmessage *event = luaL_checkudata(L, 2, "Event");
    switch (event->message) {
      case kEventLeftButtonDown:
      case kEventRightButtonDown:
      case kEventOtherButtonDown:
      case kEventLeftButtonUp:
      case kEventRightButtonUp:
      case kEventOtherButtonUp:
      case kEventLeftButtonDragged:
      case kEventRightButtonDragged:
      case kEventOtherButtonDragged:
      case kEventLeftDoubleClick:
      case kEventRightDoubleClick:
      case kEventOtherDoubleClick:
      case kEventMouseMoved:
      case kEventScrollWheel:
      case kEventKeyDown:
      case kEventKeyUp:
      case kEventChar:
      case kEventReadCommands:
//      case kEventWindowPaint:
//      case kEventWindowResized:
//      case kEventWindowChangedScreen:
//      case kEventWindowClosed:
      case ID_Window_Paint:
      case ID_Window_Resized:
      case ID_Window_ChangedScreen:
      case ID_Window_Closed:
      case ID_Window_Activated:
      case ID_Window_Deactivated:
        event->target = lua_isnil(L, 1) ? NULL : *(struct Object **)luaL_checkudata(L, 1, "Object");
        break;
    }
    lua_pushboolean(L, SV_DispatchMessage(L, event));
  }
  return 1;
}

//void
//axPostMessageW(struct Object *hobj, uint32_t Msg, wParam_t wParam, lParam_t lParam)
//{
//  static uint32_t message_id = 0;
//  struct AXmessage data = { hobj, Msg, wParam, lParam, message_id++ };
//  // HACK: unclear why this happens
//  if (Msg == kMsgWindowPaint) {
//    for (uint16_t r = queue.read; r != queue.write; r++) {
//      if (queue.data[r].message == kMsgWindowPaint &&
//          queue.data[r].hobj == hobj)
//      {
//        memset(&queue.data[r], 0, sizeof(queue.data[r]));
//        //      Con_Error("%d %d %d %08x", queue.write - queue.read, r, queue.read, queue.data[r].hobj);
//      }
//    }
//  }
//  queue.data[queue.write++] = data;
//}

//void
//SV_PostMessage(struct Object *hobj, lpcString_t Msg, wParam_t wParam, lParam_t lParam)
//{
//  axPostMessageW(hobj, fnv1a32(Msg), wParam, lParam);
//}

#define MAX_CLIENTS 256

typedef LRESULT (*message_proc_t)(lua_State*, struct AXmessage*);
static message_proc_t clients[MAX_CLIENTS];

static bool_t
dispatch_message(lua_State* L, struct AXmessage* msg) {
  if (!msg->target) {
    switch (msg->message) {
      case kEventClearReference:
        luaL_unref(L, LUA_REGISTRYINDEX, msg->wParam);
        return TRUE;
      case kEventReadCommands:
        break;
      default:
        return FALSE;
    }
  }
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i] && clients[i](L, msg)) {
      return TRUE;
    }
  }
  return FALSE;
}

ORCA_API bool_t SV_DispatchMessage(lua_State* L, struct AXmessage* msg) {
  void *target = msg->target;
  bool_t handled = dispatch_message(L, msg);
  OBJ_EndMessage(target);
  axDisposeMessage(msg);
  return handled;
}

int f_event_gc(lua_State *L)
{
  axDisposeMessage(luaL_checkudata(L, 1, "Event"));
  return 0;
}

ORCA_API void
axBuildModifiersString(wParam_t wParam, char* buf, size_t size)
{
  buf[0] = '\0';
  if (wParam & AX_MOD_CTRL)  strncat(buf, "ctrl+",  size - strlen(buf) - 1);
  if (wParam & AX_MOD_ALT)   strncat(buf, "alt+",   size - strlen(buf) - 1);
  if (wParam & AX_MOD_SHIFT) strncat(buf, "shift+", size - strlen(buf) - 1);
  if (wParam & AX_MOD_CMD)   strncat(buf, "cmd+",   size - strlen(buf) - 1);
}

lpcString_t axKeynumToString(uint32_t keynum);

/* Only ASCII printable range: translating extended/Unicode characters requires
   UTF-8 decoding, which is left to higher-level key handlers. */
static bool_t is_printable_char(int ch) {
  return ch >= 0x20 && ch <= 0x7E;
}

/* Apply shift-key transformation to a printable ASCII character. */
static char apply_shift(char ch) {
  if (ch >= '0' && ch <= '9') {
    lpcString_t sym = ")!@#$%^&*(";
    return sym[ch - '0'];
  }
  return toupper(ch);
}

int f_translate_message(lua_State* L) {
  struct AXmessage const* msg = luaL_checkudata(L, 1, "Event");
  if (msg->message == kEventKeyDown && is_printable_char(msg->wParam & 0xff)) {
    char ch = msg->wParam & 0xff;
    if (msg->wParam & AX_MOD_SHIFT)
      ch = apply_shift(ch);
    lParam_t lp = 0;
    *(char*)&lp = ch;
    axPostMessageW(msg->target, kEventChar, msg->wParam, lp);
  }
  return 0;
}

ORCA_API void
axKeyEventToText(struct AXmessage const* e, char* buf, size_t size)
{
  if (e->message == kEventChar) {
    /* lParam stores the computed character written by f_translate_message. */
    uint32_t len = 0;
    while (len < sizeof(e->lParam) && ((lpcString_t)&e->lParam)[len])
      len++;
    if (len >= size) len = (uint32_t)size - 1;
    memcpy(buf, &e->lParam, len);
    buf[len] = '\0';
  } else {
#if __linux__
    uint32_t len = 0;
    while (len < sizeof(e->lParam) && ((lpcString_t)&e->lParam)[len])
      len++;
    if (len >= size) len = (uint32_t)size - 1;
    memcpy(buf, &e->lParam, len);
    buf[len] = '\0';
#else
    snprintf(buf, size, "%s", axKeynumToString(e->wParam));
#endif
  }
}

bool_t SV_RegisterMessageProc(LRESULT (*proc)(lua_State*, struct AXmessage *)) {
  memmove(&clients[1], &clients[0], (MAX_CLIENTS - 1) * sizeof(clients[0]));
  clients[0] = proc;
  return FALSE;
}

bool_t SV_UnregisterMessageProc(LRESULT (*proc)(lua_State*, struct AXmessage *)) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i] == proc) {
      int remaining = MAX_CLIENTS - i - 1;
      if (remaining > 0) {
        memmove(&clients[i], &clients[i + 1], remaining * sizeof(clients[0]));
      }
      clients[MAX_CLIENTS - 1] = NULL; // clean up last slot
      return TRUE;
    }
  }
  return FALSE;
}

ORCA_API void SV_Init(void) {
  memset(clients, 0, sizeof(clients));
}

ORCA_API void SV_Shutdown(void) {
}
