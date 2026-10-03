/*
 * test_filesystem.c — C unit tests for the XML loader helpers in
 * source/filesystem/fs_xml_inline.h.
 *
 * The query-string loader-argument parser (_ParseLoaderArgs) that this file
 * used to cover was removed together with the .svg file loader; its tests
 * went with it.
 *
 * Compiled via the `test-filesystem` Makefile target (depends on `buildlib`).
 */

#include "test_local.h"
#include "mem_tracker.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "source/filesystem/fs_xml_inline.h"

extern int luaopen_orca_core(lua_State *L);
extern int luaopen_orca_geometry(lua_State *L);
extern int luaopen_orca_renderer(lua_State *L);

static void
ensure_class_registry(void)
{
    static int initialized = 0;
    if (initialized) {
        return;
    }

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaL_requiref(L, "orca.core", luaopen_orca_core, 1);
    lua_pop(L, 1);
    luaL_requiref(L, "orca.geometry", luaopen_orca_geometry, 1);
    lua_pop(L, 1);
    luaL_requiref(L, "orca.renderer", luaopen_orca_renderer, 1);
    lua_pop(L, 1);
    lua_close(L);
    initialized = 1;
}

/* ------------------------------------------------------------------ */
/* Tests                                                               */
/* ------------------------------------------------------------------ */

static void test_inline_xml_expansion(void)
{
    RUN_TEST("inline_xml_expansion_supports_comma_and_nested_braces", {
        ensure_class_registry();
        const char *xml = "<Material Texture={Texture Width=48, Height=24}/>";
        char *expanded = _ExpandXmlPositionalArgs(xml);
        EXPECT(expanded != NULL);
        EXPECT_STR_EQ(expanded, "<Material Texture=\"{Texture Width=48, Height=24}\"/>");
        free(expanded);
    });
}

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */

int main(void)
{
    test_inline_xml_expansion();

    printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
    return s_tests_failed == 0 ? 0 : 1;
}
