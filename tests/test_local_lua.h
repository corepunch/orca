// test_local_lua.h - Lua-specific test utilities
// Include this in test files that need to test Lua integration

#ifndef TEST_LOCAL_LUA_H
#define TEST_LOCAL_LUA_H

#include "../src/modules.h"     // for lua_State
#include "../include/orca.h"    // for lua_pushcfunction, etc.

/* ------------------------------------------------------------------ */
/* Lua test helper                                                     */
/* ------------------------------------------------------------------ */

static void test_with_lua(const char *name, void (*test_fn)(struct lua_State *))
{
    printf("Testing Lua: %s...\n", name);
    struct lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    lua_pushcfunction(L, luaopen_orca);
    lua_pushstring(L, "orca");
    lua_call(L, 1, 0);
    test_fn(L);
    lua_close(L);
}

#endif // TEST_LOCAL_LUA_H
