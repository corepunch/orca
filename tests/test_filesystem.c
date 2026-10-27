/* Focused C tests for the selected scene reader. */

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
#include <include/scene_format.h>
#include <core/core.h>
#include <filesystem/filesystem.h>

extern int luaopen_orca_core(lua_State *L);
extern int luaopen_orca_geometry(lua_State *L);

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
    lua_close(L);
    initialized = 1;
}

/* ------------------------------------------------------------------ */
/* Tests                                                               */
/* ------------------------------------------------------------------ */

static void test_scene_tree(void)
{
    RUN_TEST("scene_tree_preserves_names_and_children", {
        ensure_class_registry();
#if ORCA_HAS_LISP
        const char *source = "(Object Name=\"Root\" (Object Name=\"Child & \\\"quoted\\\"\"))";
#else
        const char *source = "<Object Name=\"Root\"><Object Name=\"Child &amp; &quot;quoted&quot;\"/></Object>";
#endif
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
#if ORCA_HAS_LISP
        const char *unterminated = "(Object Name=\"broken\"";
        const char *trailing = "(Object) (Object)";
#else
        const char *unterminated = "<Object Name=\"broken\">";
        const char *trailing = "<Object/><Object/>";
#endif
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

    printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
    return s_tests_failed == 0 ? 0 : 1;
}
