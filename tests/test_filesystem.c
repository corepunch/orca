/* Focused C tests for the XML scene reader. */

#include "test_local.h"
#include "mem_tracker.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <include/orca.h>
#include <core/core.h>
#include <filesystem/filesystem.h>

extern int luaopen_orca_core(lua_State *L);
extern int luaopen_orca_geometry(lua_State *L);
extern int luaopen_orca_filesystem(lua_State *L);

static void
ensure_class_registry(void)
{
    static int initialized = 0;
    if (initialized) {
        return;
    }

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaL_requiref(L, "orca.geometry", luaopen_orca_geometry, 1);
    lua_pop(L, 1);
    luaL_requiref(L, "orca.core", luaopen_orca_core, 1);
    lua_pop(L, 1);
    luaL_requiref(L, "orca.filesystem", luaopen_orca_filesystem, 1);
    lua_pop(L, 1);
    lua_close(L);
    initialized = 1;
}

static void test_datasource_unchanged_order(void)
{
    RUN_TEST("datasource_unchanged_order_does_not_notify", {
        ensure_class_registry();
        struct Object *object = OBJ_Create(ID_XmlDataSource);
        struct XmlDataSource *source = GetXmlDataSource(object);
        PROP_SetStringValue(OBJ_FindLongProperty(object, ID_XmlDataSource_Content),
            "<response><row name='a'/><row name='b'/><row name='c'/></response>");
        EXPECT(XmlDataSource_ReloadData(source));
        struct Object *items = CMP_GetObject(source->Items);
        struct Object *a = OBJ_GetFirstChild(items);
        struct Object *b = OBJ_GetNext(a);
        struct Object *c = OBJ_GetNext(b);
        uint32_t version = GetDataObject(items)->_collectionVersion;
        EXPECT(XmlDataSource_ReloadData(source));
        EXPECT(GetDataObject(items)->_collectionVersion == version);
        EXPECT(OBJ_GetFirstChild(items) == a && OBJ_GetNext(a) == b && OBJ_GetNext(b) == c);
        PROP_SetStringValue(OBJ_FindLongProperty(object, ID_XmlDataSource_Content),
            "<response><row name='c'/><row name='b'/><row name='a'/></response>");
        EXPECT(OBJ_GetFirstChild(items) == c && OBJ_GetNext(c) == b && OBJ_GetNext(b) == a);
        EXPECT(GetDataObject(items)->_collectionVersion > version);
        OBJ_ReleaseRef(object);
    });
}

/* ------------------------------------------------------------------ */
/* Tests                                                               */
/* ------------------------------------------------------------------ */

static void test_scene_tree(void)
{
    RUN_TEST("scene_tree_preserves_names_and_children", {
        ensure_class_registry();
        const char *source = "<Object Name=\"Root\"><Object Name=\"Child &amp; &quot;quoted&quot;\"/></Object>";
        struct Object *root = FS_LoadObjectFromString(source);
        EXPECT(root != NULL);
        EXPECT_STR_EQ(OBJ_GetName(root), "Root");
        struct Object *child = root ? OBJ_FindChild(root, "Child & \"quoted\"", FALSE) : NULL;
        EXPECT(child != NULL);
        if (root) {
            OBJ_Clear(root);
            OBJ_ReleaseRef(root);
        }
    });
}

static void test_scene_rejects_malformed_input(void)
{
    RUN_TEST("scene_rejects_unterminated_and_trailing_input", {
        ensure_class_registry();
        const char *unterminated = "<Object Name=\"broken\">";
        const char *trailing = "<Object/><Object/>";
        EXPECT(FS_LoadObjectFromString(unterminated) == NULL);
        EXPECT(FS_LoadObjectFromString(trailing) == NULL);
    });
}

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */

int main(void)
{
    test_scene_tree();
    test_scene_rejects_malformed_input();
    test_datasource_unchanged_order();

    printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
    return s_tests_failed == 0 ? 0 : 1;
}
