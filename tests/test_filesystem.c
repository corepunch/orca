/* Focused C tests for the Lisp scene reader. */

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

static void test_lisp_tree(void)
{
    RUN_TEST("lisp_tree_preserves_names_and_children", {
        ensure_class_registry();
        struct Object *root = FS_LoadObjectFromLispString(
            "(Object Name=\"Root\" (Object Name=\"Child & \\\"quoted\\\"\"))");
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

static void test_lisp_rejects_malformed_input(void)
{
    RUN_TEST("lisp_rejects_unterminated_and_trailing_input", {
        ensure_class_registry();
        EXPECT(FS_LoadObjectFromLispString("(Object Name=\"broken\"") == NULL);
        EXPECT(FS_LoadObjectFromLispString("(Object) (Object)") == NULL);
    });
}

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */

int main(void)
{
    test_lisp_tree();
    test_lisp_rejects_malformed_input();

    printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
    return s_tests_failed == 0 ? 0 : 1;
}
