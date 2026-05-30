#include <stdio.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <include/orca.h>

extern int luaopen_orca(lua_State *L);
extern int luaopen_orca_core(lua_State *L);

static int s_tests_run = 0;
static int s_tests_failed = 0;
static const char *s_current_test = NULL;

#define EXPECT(...) \
  if (!(__VA_ARGS__)) { \
    fprintf(stderr, "  FAIL [%s]: %s (line %d)\n", s_current_test, #__VA_ARGS__, __LINE__); \
    s_tests_failed++; \
    break; \
  }

#define RUN(name, block) \
  do { \
    s_current_test = name; \
    s_tests_run++; \
    printf("Running %s...\n", name); \
    do { block } while (0); \
  } while (0)

static lua_State *s_lua_state = NULL;

static lua_State *make_lua_state(void)
{
  if (!s_lua_state) {
    s_lua_state = luaL_newstate();
    luaL_openlibs(s_lua_state);
    luaL_requiref(s_lua_state, "orca", luaopen_orca, 1);
    lua_pop(s_lua_state, 1);
    luaopen_orca_core(s_lua_state);
    lua_pop(s_lua_state, 1);
  }
  return s_lua_state;
}

static uint32_t
payload_property_count(struct ClassDesc const *cls)
{
  return (cls && cls->NumProperties >= 2) ? cls->NumProperties - 2 : 0;
}

static void test_trigger_triggered_message_action_class(void)
{
  RUN("trigger_triggered_message_action_class", {
    lua_State *L = make_lua_state();
    struct ClassDesc const *cls = OBJ_FindClass("Trigger.Triggered");
    EXPECT(L != NULL);
    EXPECT(cls != NULL);
    EXPECT(payload_property_count(cls) == 1);
    EXPECT(cls->Properties[0].Name != NULL && !strcmp(cls->Properties[0].Name, "Sender"));
  });
}

static void test_object_attached_message_action_class(void)
{
  RUN("object_attached_message_action_class", {
    lua_State *L = make_lua_state();
    struct ClassDesc const *cls = OBJ_FindClass("Object.Attached");
    EXPECT(L != NULL);
    EXPECT(cls != NULL);
    EXPECT(payload_property_count(cls) == 1);
    EXPECT(cls->Properties[0].Name != NULL && !strcmp(cls->Properties[0].Name, "Sender"));
  });
}

static void test_unknown_message_action_class_lookup(void)
{
  RUN("unknown_message_action_class_lookup", {
    lua_State *L = make_lua_state();
    EXPECT(L != NULL);
    EXPECT(OBJ_FindClass("Nope.Unknown") == NULL);
  });
}

int main(void)
{
  test_trigger_triggered_message_action_class();
  test_object_attached_message_action_class();
  test_unknown_message_action_class_lookup();

  if (s_lua_state) {
    lua_close(s_lua_state);
  }

  printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
  return s_tests_failed ? 1 : 0;
}
