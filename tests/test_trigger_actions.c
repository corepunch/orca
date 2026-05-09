#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <include/orca.h>
#include <source/core/core_local.h>
#include <source/core/core_properties.h>
#include <source/core/object/object_internal.h>

#include <stdio.h>
#include <string.h>

extern int luaopen_orca_core(lua_State *L);

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

#define SET_BOOL(obj, name, value) do { \
  bool_t _v = (value); \
  EXPECT_OK(OBJ_SetPropertyValue((obj), (name), &_v)); \
} while (0)

struct TestValueMessage {
  float Value;
};

struct TestMixedMessage {
  int Count;
  float Value;
  bool_t Enabled;
};

static uint32_t s_msg_value;
static uint32_t s_msg_mixed;
static uint32_t s_msg_no_args;

static struct
{
  bool_t Triggered;
  int NoArgsCount;
  int ValueCount;
  int MixedCount;
  float Value;
  int Count;
  bool_t Enabled;
  const char *Label;
} s_observed;

static lua_State *
make_lua_state(void)
{
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  luaopen_orca_core(L);
  lua_pop(L, 1);
  return L;
}

static void register_trigger_action_test_types(void);

static void
initialize_test_environment(void)
{
  if (s_lua_state) {
    return;
  }
  s_lua_state = make_lua_state();
  register_trigger_action_test_types();
}

static void
reset_observed(void)
{
  memset(&s_observed, 0, sizeof(s_observed));
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

  if (msg == s_msg_value) {
    struct TestValueMessage const *args = (struct TestValueMessage const *)lparam;
    s_observed.ValueCount++;
    s_observed.Value = args ? args->Value : 0.f;
    return TRUE;
  }
  if (msg == s_msg_mixed) {
    struct TestMixedMessage const *args = (struct TestMixedMessage const *)lparam;
    s_observed.MixedCount++;
    if (args) {
      s_observed.Count = args->Count;
      s_observed.Value = args->Value;
      s_observed.Enabled = args->Enabled;
    } else {
      s_observed.Count = 0;
      s_observed.Value = 0.f;
      s_observed.Enabled = FALSE;
    }
    return TRUE;
  }
  if (msg == s_msg_no_args) {
    s_observed.NoArgsCount++;
    return TRUE;
  }
  return FALSE;
}

static struct PropertyType const s_value_message_fields[] = {
  {
    .ShortIdentifier = 0xd147f96a, /* fnv1a32("Value") */
    .FullIdentifier = 0xd147f96a,
    .Name = "Value",
    .Key = "Value",
    .DataType = kDataTypeFloat,
    .DataSize = sizeof(float),
    .Offset = offsetof(struct TestValueMessage, Value),
  },
};

static struct PropertyType const s_mixed_message_fields[] = {
  {
    .ShortIdentifier = 0xe1e7b894, /* fnv1a32("Count") */
    .FullIdentifier = 0xe1e7b894,
    .Name = "Count",
    .Key = "Count",
    .DataType = kDataTypeInt,
    .DataSize = sizeof(int),
    .Offset = offsetof(struct TestMixedMessage, Count),
  },
  {
    .ShortIdentifier = 0xd147f96a, /* fnv1a32("Value") */
    .FullIdentifier = 0xd147f96a,
    .Name = "Value",
    .Key = "Value",
    .DataType = kDataTypeFloat,
    .DataSize = sizeof(float),
    .Offset = offsetof(struct TestMixedMessage, Value),
  },
  {
    .ShortIdentifier = 0x9c86e43e, /* fnv1a32("Enabled") */
    .FullIdentifier = 0x9c86e43e,
    .Name = "Enabled",
    .Key = "Enabled",
    .DataType = kDataTypeBool,
    .DataSize = sizeof(bool_t),
    .Offset = offsetof(struct TestMixedMessage, Enabled),
  },
};

struct TestSource {
  int Count;
  float Value;
  bool_t Enabled;
};

static struct PropertyType const s_source_properties[] = {
  {
    .ShortIdentifier = 0xe1e7b894, /* fnv1a32("Count") */
    .FullIdentifier = 0xe1e7b894,
    .Name = "Count",
    .Key = "Count",
    .DataType = kDataTypeInt,
    .DataSize = sizeof(int),
    .Offset = offsetof(struct TestSource, Count),
  },
  {
    .ShortIdentifier = 0xd147f96a, /* fnv1a32("Value") */
    .FullIdentifier = 0xd147f96a,
    .Name = "Value",
    .Key = "Value",
    .DataType = kDataTypeFloat,
    .DataSize = sizeof(float),
    .Offset = offsetof(struct TestSource, Value),
  },
  {
    .ShortIdentifier = 0x9c86e43e, /* fnv1a32("Enabled") */
    .FullIdentifier = 0x9c86e43e,
    .Name = "Enabled",
    .Key = "Enabled",
    .DataType = kDataTypeBool,
    .DataSize = sizeof(bool_t),
    .Offset = offsetof(struct TestSource, Enabled),
  },
};

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
  .Properties = s_source_properties,
  .ClassName = "TestEventTriggerSource",
  .DefaultName = "TestEventTriggerSource",
  .ParentClasses = { ID_EventTrigger, ID_SendMessageAction, 0 },
  .ClassID = 0xd79411fc,
  .ClassSize = sizeof(struct TestSource),
  .NumProperties = sizeof(s_source_properties) / sizeof(s_source_properties[0]),
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

  OBJ_RegisterMessagePropertyTypes("TestMessage.Value",
                                   (struct PropertyType *)s_value_message_fields,
                                   1);
  OBJ_RegisterMessagePropertyTypes("TestMessage.Mixed",
                                   (struct PropertyType *)s_mixed_message_fields,
                                   (uint32_t)(sizeof(s_mixed_message_fields) / sizeof(s_mixed_message_fields[0])));

  s_msg_no_args = fnv1a32("TestMessage.NoArgs");
  s_msg_value = fnv1a32("TestMessage.Value");
  s_msg_mixed = fnv1a32("TestMessage.Mixed");
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
              const char *routed_event,
              const char *message,
              const char *target)
{
  SET_STRING(source, "RoutedEvent", routed_event);
  SET_STRING(source, "Message", message);
  SET_STRING(source, "Target", target);
}

static bool_t
is_trigger_bound(struct Object *source)
{
  struct Property *prop = NULL;
  if (FAILED(OBJ_FindShortProperty(source, "Trigger", &prop)) || !prop) {
    return FALSE;
  }
  return *(struct Object **)PROP_GetValue(prop) == CMP_GetObject(GetTrigger(source));
}

static void
test_event_trigger_no_args(void)
{
  RUN("event_trigger_no_args", {
    reset_observed();

    struct Object *root = make_object(0xa3b95e0d, "Root");
    struct Object *source = make_object(0xd79411fc, "Source");
    struct Object *target = make_object(0xb5c4156c, "Target");

    EXPECT(root != NULL);
    EXPECT(source != NULL);
    EXPECT(target != NULL);

    OBJ_AddChild(root, source, FALSE);
    OBJ_AddChild(source, target, FALSE);

    set_up_source(source, "Node.RightButtonUp", "TestMessage.NoArgs", "Target");
    EXPECT(is_trigger_bound(source));

    OBJ_SendMessageW(source, ID_Node_RightButtonUp, 0, NULL);

    EXPECT(s_observed.Triggered);
    EXPECT(s_observed.NoArgsCount == 1);
    EXPECT(s_observed.ValueCount == 0);
    EXPECT(s_observed.MixedCount == 0);
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

    EXPECT(root != NULL);
    EXPECT(source != NULL);
    EXPECT(target != NULL);

    OBJ_AddChild(root, source, FALSE);
    OBJ_AddChild(source, target, FALSE);

    set_up_source(source, "Node.KeyDown", "TestMessage.Value", "Target");
    SET_FLOAT(source, "Value", 3.25f);
    EXPECT(is_trigger_bound(source));

    OBJ_SendMessageW(source, ID_Node_KeyDown, 0, NULL);

    EXPECT(s_observed.Triggered);
    EXPECT(s_observed.ValueCount == 1);
    EXPECT(s_observed.MixedCount == 0);
    EXPECT(s_observed.NoArgsCount == 0);
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

    EXPECT(root != NULL);
    EXPECT(source != NULL);
    EXPECT(target != NULL);

    OBJ_AddChild(root, source, FALSE);
    OBJ_AddChild(source, target, FALSE);

    set_up_source(source, "Node.RightButtonUp", "TestMessage.Mixed", "Target");
    SET_INT(source, "Count", 12);
    EXPECT(is_trigger_bound(source));

    OBJ_SendMessageW(source, ID_Node_RightButtonUp, 0, NULL);

    EXPECT(s_observed.Triggered);
    EXPECT(s_observed.MixedCount == 1);
    EXPECT(s_observed.Count == 12);
    EXPECT(fabsf(s_observed.Value) < 0.0001f);
    EXPECT(s_observed.Enabled == FALSE);
  });
}

int
main(void)
{
  initialize_test_environment();
  test_event_trigger_no_args();
  test_event_trigger_single_value();
  test_event_trigger_partial_payload_defaults_to_zero();

  if (s_lua_state) {
    lua_close(s_lua_state);
    s_lua_state = NULL;
  }

  printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
  return s_tests_failed ? 1 : 0;
}
