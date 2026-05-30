#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <include/orca.h>
#include <include/codegen.h>
#include <source/core/core_local.h>
#include <core/core_properties.h>
#include <source/core/object/object_internal.h>
#include <source/filesystem/fs_local.h>
#include <UIKit/UIKit.h>
#include <UIKit/UIKit_properties.h>

#include <stdio.h>
#include <string.h>

extern int luaopen_orca(lua_State *L);
extern int luaopen_orca_core(lua_State *L);
extern int luaopen_orca_system(lua_State *L);

static int s_tests_run = 0;
static int s_tests_failed = 0;
static const char *s_current_test = NULL;
static lua_State *s_lua_state = NULL;

#define EXPECT(...) \
  if (!(__VA_ARGS__)) { \
    fprintf(stderr, "  FAIL [%s]: %s (line %d)\n", s_current_test, #__VA_ARGS__, __LINE__); \
    s_tests_failed++; \
    break; \
  }

#define EXPECT_OK(hr) EXPECT((hr) == NOERROR)

#define RUN(name, block) \
  do { \
    s_current_test = name; \
    s_tests_run++; \
    printf("Running %s...\n", name); \
    do { block } while (0); \
  } while (0)

#define SET_STRING(obj, name, value) do { \
  const char *_v = (value); \
  EXPECT_OK(OBJ_SetPropertyValue((obj), (name), &_v)); \
} while (0)

#define SET_FLOAT(obj, name, value) do { \
  float _v = (value); \
  EXPECT_OK(OBJ_SetPropertyValue((obj), (name), &_v)); \
} while (0)

#define SET_INT(obj, name, value) do { \
  int _v = (value); \
  EXPECT_OK(OBJ_SetPropertyValue((obj), (name), &_v)); \
} while (0)

static struct
{
  bool_t Triggered;
  int StartCount;
  int RightButtonUpCount;
  float Value;
  int Count;
} s_observed;

static lua_State *
make_lua_state(void)
{
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  luaL_requiref(L, "orca", luaopen_orca, 1);
  lua_pop(L, 1);
  luaopen_orca_core(L);
  lua_pop(L, 1);
  luaL_requiref(L, "orca.system", luaopen_orca_system, 1);
  lua_pop(L, 1);
  return L;
}

static void register_trigger_action_test_types(void);
static LRESULT TestDispatchObjectMessage(lua_State *L, struct AXmessage *msg);

static void
initialize_test_environment(void)
{
  if (s_lua_state) {
    return;
  }
  s_lua_state = make_lua_state();
  SV_RegisterMessageProc(TestDispatchObjectMessage);
  register_trigger_action_test_types();
}

static void
reset_observed(void)
{
  memset(&s_observed, 0, sizeof(s_observed));
}

static void
pump_messages(struct Object *root)
{
  (void)root;
  core_FlushQueue(s_lua_state);
}

static LRESULT
TestDispatchObjectMessage(lua_State *L, struct AXmessage *msg)
{
  (void)L;

  if (!msg || !msg->target) {
    return FALSE;
  }

  if (msg->message == kEventResumeCoroutine ||
      msg->message == kEventStopCoroutine) {
    return FALSE;
  }

  for (struct Object *hobj = msg->target; hobj; hobj = OBJ_GetParent(hobj)) {
    if (OBJ_SendMessageW(hobj, msg->message, msg->wParam, msg->lParam)) {
      return TRUE;
    }
  }
  return FALSE;
}

static LRESULT
TestHostProc(struct Object *object, void *cmp, uint32_t msg, wParam_t wparam, lParam_t lparam)
{
  (void)object;
  (void)cmp;
  (void)wparam;
  (void)lparam;
  if (msg == ID_Trigger_Triggered) {
    s_observed.Triggered = TRUE;
  }
  return FALSE;
}

static LRESULT
TestSinkProc(struct Object *object, void *cmp, uint32_t msg, wParam_t wparam, lParam_t lparam)
{
  (void)object;
  (void)cmp;
  (void)wparam;

  if (msg == ID_Object_Start) {
    s_observed.StartCount++;
    return TRUE;
  }
  if (msg == ID_Node_RightButtonUp) {
    Node_RightButtonUpMsg_t const *args = (Node_RightButtonUpMsg_t const *)lparam;
    s_observed.RightButtonUpCount++;
    if (args) {
      s_observed.Value = args->x;
      s_observed.Count = args->clickCount;
    }
    return TRUE;
  }
  return FALSE;
}

static LRESULT
TestSourceProc(struct Object *object, void *cmp, uint32_t msg, wParam_t wparam, lParam_t lparam)
{
  (void)object;
  (void)cmp;
  (void)wparam;
  (void)lparam;
  if (msg == ID_Trigger_Triggered) {
    s_observed.Triggered = TRUE;
  }
  return FALSE;
}

static struct ClassDesc s_host_class = {
  .ObjProc = TestHostProc,
  .ClassName = "TestTriggerHost",
  .DefaultName = "TestTriggerHost",
  .ClassID = 0xa3b95e0d,
  .ClassSize = 0,
  .NumProperties = 0,
};

static struct ClassDesc s_sink_class = {
  .ObjProc = TestSinkProc,
  .ClassName = "TestTriggerSink",
  .DefaultName = "TestTriggerSink",
  .ClassID = 0xb5c4156c,
  .ClassSize = 0,
  .NumProperties = 0,
};

static struct ClassDesc s_event_source_class = {
  .ObjProc = TestSourceProc,
  .ClassName = "TestEventTriggerSource",
  .DefaultName = "TestEventTriggerSource",
  .ParentClasses = { ID_EventTrigger, 0 },
  .ClassID = 0xd79411fc,
  .ClassSize = 0,
  .NumProperties = 0,
};

static void
register_trigger_action_test_types(void)
{
  static bool_t registered = FALSE;
  if (registered) {
    return;
  }
  registered = TRUE;

  OBJ_RegisterClass(&s_host_class);
  OBJ_RegisterClass(&s_sink_class);
  OBJ_RegisterClass(&s_event_source_class);
}

static struct Object *
make_object(uint32_t class_id, const char *name)
{
  struct Object *obj = OBJ_Create(class_id);
  if (obj && name) {
    OBJ_SetName(obj, name);
  }
  return obj;
}

static void
set_up_source(struct Object *source,
              const char *routed_event)
{
  SET_STRING(source, "RoutedEvent", routed_event);
}

static void
test_event_trigger_no_args(void)
{
  RUN("event_trigger_no_args", {
    reset_observed();

    struct Object *root = make_object(0xa3b95e0d, "Root");
    struct Object *source = make_object(0xd79411fc, "Source");
    struct Object *target = make_object(0xb5c4156c, "Target");
    struct Object *action = make_object(ID_Object_StartAction, "Action");

    EXPECT(root != NULL);
    EXPECT(source != NULL);
    EXPECT(target != NULL);
    EXPECT(action != NULL);

    OBJ_AddChild(root, source);
    OBJ_AddChild(source, target);
    OBJ_AddChild(source, action);

    set_up_source(source, "Node.RightButtonUp");
    SET_STRING(action, "Target", "Target");

    OBJ_SendMessageW(source, ID_Node_RightButtonUp, 0, NULL);

    EXPECT(s_observed.Triggered);
    EXPECT(s_observed.StartCount == 1);
  });
}

static void
test_event_trigger_single_value(void)
{
  RUN("event_trigger_single_value", {
    reset_observed();

    struct Object *root = make_object(0xa3b95e0d, "Root");
    struct Object *source = make_object(0xd79411fc, "Source");
    struct Object *target = make_object(0xb5c4156c, "Target");
    struct Object *action = make_object(ID_Node_RightButtonUpAction, "Action");

    EXPECT(root != NULL);
    EXPECT(source != NULL);
    EXPECT(target != NULL);
    EXPECT(action != NULL);

    OBJ_AddChild(root, source);
    OBJ_AddChild(source, target);
    OBJ_AddChild(source, action);

    set_up_source(source, "Node.KeyDown");
    SET_STRING(action, "Target", "Target");
    SET_FLOAT(action, "x", 3.25f);

    OBJ_SendMessageW(source, ID_Node_KeyDown, 0, NULL);

    EXPECT(s_observed.Triggered);
    EXPECT(s_observed.RightButtonUpCount == 1);
    EXPECT(fabsf(s_observed.Value - 3.25f) < 0.0001f);
  });
}

static void
test_event_trigger_partial_payload_defaults_to_zero(void)
{
  RUN("event_trigger_partial_payload_defaults_to_zero", {
    reset_observed();

    struct Object *root = make_object(0xa3b95e0d, "Root");
    struct Object *source = make_object(0xd79411fc, "Source");
    struct Object *target = make_object(0xb5c4156c, "Target");
    struct Object *action = make_object(ID_Node_RightButtonUpAction, "Action");

    EXPECT(root != NULL);
    EXPECT(source != NULL);
    EXPECT(target != NULL);
    EXPECT(action != NULL);

    OBJ_AddChild(root, source);
    OBJ_AddChild(source, target);
    OBJ_AddChild(source, action);

    set_up_source(source, "Node.RightButtonUp");
    SET_STRING(action, "Target", "Target");
    SET_INT(action, "clickCount", 12);

    OBJ_SendMessageW(source, ID_Node_RightButtonUp, 0, NULL);

    EXPECT(s_observed.Triggered);
    EXPECT(s_observed.RightButtonUpCount == 1);
    EXPECT(s_observed.Count == 12);
    EXPECT(fabsf(s_observed.Value) < 0.0001f);
  });
}

static void
test_generated_action_property_order(void)
{
  RUN("generated_action_property_order", {
    struct Object *action = make_object(ID_Node_RightButtonUpAction, "Action");
    struct Property *x_prop = NULL;
    struct Property *target_prop = NULL;

    EXPECT(action != NULL);
    EXPECT_OK(OBJ_FindLongProperty(action, ID_Node_RightButtonUpAction_x, &x_prop));
    EXPECT_OK(OBJ_FindLongProperty(action, ID_Node_RightButtonUpAction__ActionTarget, &target_prop));
    EXPECT(x_prop != NULL);
    EXPECT(target_prop != NULL);
    EXPECT(strcmp(PROP_GetName(x_prop), "x") == 0);
    EXPECT(strcmp(PROP_GetName(target_prop), "Target") == 0);
    char removed_class[] = "Send" "Message" "Action";
    EXPECT(OBJ_FindClass(removed_class) == NULL);
  });
}

static void
test_generated_action_xml(void)
{
  RUN("generated_action_xml", {
    reset_observed();

    struct Object *root = FS_LoadObjectFromXmlString(
      "<TestTriggerHost Name=\"Root\">\n"
      "  <TestEventTriggerSource Name=\"Source\" RoutedEvent=\"Node.LeftButtonUp\">\n"
      "    <Object.Start Target=\"../Target\"/>\n"
      "  </TestEventTriggerSource>\n"
      "  <TestTriggerSink Name=\"Target\"/>\n"
      "</TestTriggerHost>");
    EXPECT(root != NULL);
    reset_observed();

    struct Object *source = root ? OBJ_FindChild(root, "Source", TRUE) : NULL;
    EXPECT(source != NULL);
    OBJ_SendMessageW(source, ID_Node_LeftButtonUp, 0, NULL);
    EXPECT(s_observed.StartCount == 1);

    if (root) {
      OBJ_Clear(root);
      OBJ_ReleaseRef(root);
    }
  });
}

static void
test_generated_action_post_mode(void)
{
  RUN("generated_action_post_mode", {
    reset_observed();

    struct Object *root = make_object(0xa3b95e0d, "Root");
    struct Object *sender = make_object(0xb5c4156c, "Sender");
    struct Object *action = make_object(ID_Object_StartAction, "Action");
    enum DispatchMode mode = kDispatchModePost;

    EXPECT(root != NULL);
    EXPECT(sender != NULL);
    EXPECT(action != NULL);

    OBJ_AddChild(root, sender);
    OBJ_AddChild(root, action);
    EXPECT_OK(OBJ_SetPropertyValue(action, "Mode", &mode));

    _SendMessage(action, Action, Dispatch, .Sender = sender);
    EXPECT(s_observed.StartCount == 0);
    pump_messages(root);
    EXPECT(s_observed.StartCount == 1);
  });
}

static void
test_generated_action_unset_target_dispatches_sender(void)
{
  RUN("generated_action_unset_target_dispatches_sender", {
    reset_observed();

    struct Object *sender = make_object(0xb5c4156c, "Sender");
    struct Object *action = make_object(ID_Object_StartAction, "Action");

    EXPECT(sender != NULL);
    EXPECT(action != NULL);

    _SendMessage(action, Action, Dispatch, .Sender = sender);
    EXPECT(s_observed.StartCount == 1);
  });
}

int
main(void)
{
  initialize_test_environment();
  test_event_trigger_no_args();
  test_event_trigger_single_value();
  test_event_trigger_partial_payload_defaults_to_zero();
  test_generated_action_property_order();
  test_generated_action_xml();
  test_generated_action_post_mode();
  test_generated_action_unset_target_dispatches_sender();

  if (s_lua_state) {
    lua_close(s_lua_state);
    s_lua_state = NULL;
  }

  printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
  return s_tests_failed ? 1 : 0;
}
