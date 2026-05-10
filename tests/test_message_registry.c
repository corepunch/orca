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

static void test_trigger_triggered_registry(void)
{
  RUN("trigger_triggered_registry", {
    lua_State *L = make_lua_state();
    uint32_t count = 0;
    struct PropertyType *props = OBJ_FindMessagePropertyTypes("Trigger.Triggered", &count);
    EXPECT(L != NULL);
    EXPECT(props != NULL);
    EXPECT(count == 2);
    EXPECT(props[0].Name != NULL && !strcmp(props[0].Name, "Trigger"));
    EXPECT(props[1].Name != NULL && !strcmp(props[1].Name, "Sender"));
  });
}

static void test_object_attached_registry(void)
{
  RUN("object_attached_registry", {
    lua_State *L = make_lua_state();
    uint32_t count = 0;
    struct PropertyType *props = OBJ_FindMessagePropertyTypes("Object.Attached", &count);
    EXPECT(L != NULL);
    EXPECT(props != NULL);
    EXPECT(count == 1);
    EXPECT(props[0].Name != NULL && !strcmp(props[0].Name, "Sender"));
  });
}

static void test_unknown_message_registry_lookup(void)
{
  RUN("unknown_message_registry_lookup", {
    lua_State *L = make_lua_state();
    uint32_t count = 123;
    struct PropertyType *props = OBJ_FindMessagePropertyTypes("Nope.Unknown", &count);
    EXPECT(L != NULL);
    EXPECT(props == NULL);
    EXPECT(count == 0);
  });
}

int main(void)
{
  test_trigger_triggered_registry();
  test_object_attached_registry();
  test_unknown_message_registry_lookup();

  if (s_lua_state) {
    lua_close(s_lua_state);
  }

  printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
  return s_tests_failed ? 1 : 0;
}
