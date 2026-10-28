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

static void check_owned_message_payload_copy(void)
{
  do {
    make_lua_state();
    struct StructDesc const *strings = OBJ_FindStructDesc("Object_LoadControllerEventArgs");
    EXPECT(strings != NULL);
    char path[] = "Pages/東京.xml";
    struct Object_LoadControllerEventArgs source = { .Path = path }, copy = {0};
    EXPECT(core_CloneStructPayload(strings, &copy, &source));
    EXPECT(copy.Path != path);
    path[0] = 'X';
    EXPECT(!strcmp(copy.Path, "Pages/東京.xml"));
    core_ClearStructPayload(strings, &copy);
    EXPECT(copy.Path == NULL);
    core_ClearStructPayload(strings, &copy);

    struct StructDesc const *objects = OBJ_FindStructDesc("DataObject_CollectionChangedEventArgs");
    EXPECT(objects != NULL);
    struct Object *data = OBJ_Create(ID_DataObject);
    OBJ_SetName(data, "Retained source");
    struct DataObject_CollectionChangedEventArgs objectSource = { .Source = data, .Version = UINT32_MAX };
    struct DataObject_CollectionChangedEventArgs objectCopy = {0};
    EXPECT(core_CloneStructPayload(objects, &objectCopy, &objectSource));
    OBJ_ReleaseRef(data);
    EXPECT(!strcmp(OBJ_GetName(objectCopy.Source), "Retained source"));
    EXPECT(objectCopy.Version == UINT32_MAX);
    core_ClearStructPayload(objects, &objectCopy);
    EXPECT(objectCopy.Source == NULL);

    struct StructDesc const *mouse = OBJ_FindStructDesc("Node_LeftButtonUpEventArgs");
    EXPECT(mouse != NULL);
    bool_t hasUnsignedTimestamp = FALSE;
    for (uint32_t i = 0; i < mouse->NumProperties; i++) {
      if (!strcmp(mouse->Properties[i].Name, "timestamp"))
        hasUnsignedTimestamp = mouse->Properties[i].IsUnsigned;
    }
    EXPECT(hasUnsignedTimestamp);
  } while (0);
}

static void test_owned_message_payload_copy(void)
{
  RUN_TEST("owned_message_payload_copy", { check_owned_message_payload_copy(); });
}

struct PayloadLink {
  char *text;
  struct PayloadLink *next;
};

static void check_payload_graph_limits(void)
{
  do {
    static struct PropertyType const fields[] = {
      { .Name = "text", .Offset = offsetof(struct PayloadLink, text),
        .DataType = kDataTypeString, .DataSize = sizeof(char *), .IsPointer = TRUE },
      { .Name = "next", .Offset = offsetof(struct PayloadLink, next),
        .DataType = kDataTypeStruct, .DataSize = sizeof(void *), .IsPointer = TRUE,
        .TypeString = "TestPayloadLink" },
    };
    static struct StructDesc desc = {
      .StructName = "TestPayloadLink", .Properties = fields,
      .NumProperties = 2, .StructSize = sizeof(struct PayloadLink),
    };
    OBJ_RegisterStructDesc(&desc);
    struct PayloadLink cycle = { .text = "cyclic" }, copy = {0};
    cycle.next = &cycle;
    EXPECT(!core_CloneStructPayload(&desc, &copy, &cycle));
    EXPECT(copy.text == NULL && copy.next == NULL);
    core_ClearStructPayload(&desc, &copy);

    struct PayloadLink chain[40] = {0};
    for (uint32_t i = 0; i < 40; i++) {
      chain[i].text = "nested";
      chain[i].next = i + 1 < 40 ? chain + i + 1 : NULL;
    }
    EXPECT(!core_CloneStructPayload(&desc, &copy, chain));
    EXPECT(copy.text == NULL && copy.next == NULL);
    chain[1].next = NULL;
    EXPECT(core_CloneStructPayload(&desc, &copy, chain));
    EXPECT(copy.next != chain + 1 && !strcmp(copy.next->text, "nested"));
    core_ClearStructPayload(&desc, &copy);
  } while (0);
}

static void test_payload_graph_limits(void)
{
  RUN_TEST("payload_graph_limits", { check_payload_graph_limits(); });
}

int main(void)
{
  test_trigger_triggered_message_action_class();
  test_object_attached_message_action_class();
  test_unknown_message_action_class_lookup();
  test_owned_message_payload_copy();
  test_payload_graph_limits();

  if (s_lua_state) {
    lua_close(s_lua_state);
  }

  printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
  return s_tests_failed ? 1 : 0;
}
