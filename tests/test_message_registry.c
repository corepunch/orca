#include <stdio.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <include/orca.h>
#include "test_local.h"

extern int luaopen_orca(lua_State *L);
extern int luaopen_orca_core(lua_State *L);

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
  return cls ? cls->NumProperties : 0;
}

static bool_t
inherits_from_send_message_action(struct ClassDesc const *cls)
{
  struct ClassDesc const *base = OBJ_FindClass("SendMessageAction");
  if (!cls || !base) {
    return FALSE;
  }
  for (uint32_t const *parent = cls->ParentClasses; *parent; parent++) {
    if (*parent == base->ClassID) {
      return TRUE;
    }
  }
  return FALSE;
}

static void test_trigger_triggered_message_action_class(void)
{
  RUN_TEST("trigger_triggered_message_action_class", {
    lua_State *L = make_lua_state();
    struct ClassDesc const *cls = OBJ_FindClass("Trigger.Triggered");
    EXPECT(L != NULL);
    EXPECT(cls != NULL);
    EXPECT(inherits_from_send_message_action(cls));
    EXPECT(payload_property_count(cls) == 1);
    EXPECT(cls->Properties[0].Name != NULL && !strcmp(cls->Properties[0].Name, "Sender"));
  });
}

static void test_object_attached_message_action_class(void)
{
  RUN_TEST("object_attached_message_action_class", {
    lua_State *L = make_lua_state();
    struct ClassDesc const *cls = OBJ_FindClass("Object.Attached");
    EXPECT(L != NULL);
    EXPECT(cls != NULL);
    EXPECT(inherits_from_send_message_action(cls));
    EXPECT(payload_property_count(cls) == 1);
    EXPECT(cls->Properties[0].Name != NULL && !strcmp(cls->Properties[0].Name, "Sender"));
  });
}

static void test_unknown_message_action_class_lookup(void)
{
  RUN_TEST("unknown_message_action_class_lookup", {
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
