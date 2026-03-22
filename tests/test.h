/* tests/test.h — C unit-test macro infrastructure.
 *
 * Three levels of abbreviation are provided:
 *
 *  Level 0 – verbose (no macros):
 *    static void test_bool_property(void) {
 *        TEST_BEGIN("bool property set/get");
 *        long snap = MEM_SNAPSHOT();
 *        lpObject_t obj = make_object();
 *        ...
 *        destroy_object(obj);
 *        MEM_CHECK_LEAK(snap, s_current_test);
 *    }
 *
 *  Level 1 – combined begin+snapshot, xmlWith-style object scope:
 *    static void test_bool_property(void) {
 *        TEST_BEGIN_MEM("bool property set/get", snap);
 *        objWith(obj) {
 *            ...
 *        }
 *        MEM_CHECK_LEAK(snap, s_current_test);
 *    }
 *
 *  Level 2 – single open/close macro (the shortest form):
 *    static void test_bool_property(void) {
 *        testWith("bool property set/get", obj) {
 *            ...
 *        }
 *    }
 *
 * Object lifecycle macros (objWith / testWith) are only available when
 * <include/orca.h> has been included before this header.
 */

#ifndef TESTS_TEST_H
#define TESTS_TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* -----------------------------------------------------------------------
 * Test state — defined once in test_main.c, extern-declared here.
 * ----------------------------------------------------------------------- */

extern const char *s_current_test;
extern int         s_tests_run;
extern int         s_tests_failed;

/* -----------------------------------------------------------------------
 * Assertion helper
 * ----------------------------------------------------------------------- */

#define TEST_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "FAIL [%s] (%s:%d): %s\n", \
                    s_current_test ? s_current_test : "?", \
                    __FILE__, __LINE__, #cond); \
            s_tests_failed++; \
            return; \
        } \
    } while (0)

/* -----------------------------------------------------------------------
 * Memory snapshot — detects allocation leaks within a single test.
 * On Linux  mallinfo2() is used; elsewhere the check is a no-op.
 * ----------------------------------------------------------------------- */

#ifdef __linux__
#  include <malloc.h>
static inline long _mem_snapshot(void)
{
    struct mallinfo2 mi = mallinfo2();
    return (long)mi.uordblks;
}
static inline void _mem_check_leak(long snap, const char *name)
{
    long now = _mem_snapshot();
    if (now > snap + 64)
        fprintf(stderr, "POSSIBLE LEAK in [%s]: %ld bytes\n", name, now - snap);
}
#  define MEM_SNAPSHOT()             _mem_snapshot()
#  define MEM_CHECK_LEAK(snap, name) _mem_check_leak(snap, name)
#else
#  define MEM_SNAPSHOT()             0L
#  define MEM_CHECK_LEAK(snap, name) ((void)(snap))
#endif

/* -----------------------------------------------------------------------
 * Level 1a — TEST_BEGIN: set s_current_test (legacy / explicit form).
 *
 *   TEST_BEGIN("my test");
 *   long snap = MEM_SNAPSHOT();
 *   ...
 *   MEM_CHECK_LEAK(snap, s_current_test);
 * ----------------------------------------------------------------------- */

#define TEST_BEGIN(name) \
    do { s_current_test = (name); s_tests_run++; } while (0)

/* -----------------------------------------------------------------------
 * Level 1b — TEST_BEGIN_MEM: combine TEST_BEGIN + MEM_SNAPSHOT.
 *
 *   TEST_BEGIN_MEM("my test", snap);
 *   ...
 *   MEM_CHECK_LEAK(snap, s_current_test);
 * ----------------------------------------------------------------------- */

static inline long _test_begin(const char *name)
{
    s_current_test = name;
    s_tests_run++;
    return MEM_SNAPSHOT();
}

#define TEST_BEGIN_MEM(name, snap) \
    TEST_BEGIN(name); \
    long snap = MEM_SNAPSHOT()

/* -----------------------------------------------------------------------
 * TEST_WITH_MEM — for-loop scope that automatically checks for memory
 * leaks after the body (no explicit MEM_CHECK_LEAK call required).
 *
 *   TEST_WITH_MEM("my test", snap) {
 *       ...
 *   }
 * ----------------------------------------------------------------------- */

#define TEST_WITH_MEM(name, snap) \
    for (long snap = _test_begin(name); snap >= 0; \
         MEM_CHECK_LEAK(snap, s_current_test), snap = -1)

/* -----------------------------------------------------------------------
 * Token-pasting helpers for generating unique variable names.
 * ----------------------------------------------------------------------- */

#define _TEST_CAT(a, b)   a##b
#define _TEST_UNIQUE(pfx) _TEST_CAT(pfx, __LINE__)

/* -----------------------------------------------------------------------
 * objWith — RAII-style object lifecycle using the xmlWith pattern.
 *
 * Only available when <include/orca.h> has been included beforehand.
 *
 * Requires s_test_L  (lua_State*)    to be set in test_main.c.
 * Requires s_test_class (lpcClassDesc_t) to be set before calling.
 *
 *   objWith(obj) {
 *       // obj is lpObject_t, valid for the block body
 *   }
 * ----------------------------------------------------------------------- */

#ifdef __ORCA_H__

extern lua_State     *s_test_L;
extern lpcClassDesc_t s_test_class;

static inline lpObject_t _make_test_obj(void)
{
    if (!s_test_class) return NULL;
    return OBJ_Create(s_test_L, s_test_class);
}

static inline void _free_test_obj(lpObject_t obj)
{
    /* Pop from the stack (makes the userdata unreachable from Lua),
     * then run one GC cycle so __gc fires (calling OBJ_Release) exactly once.
     * This avoids the double-release that would occur if we called
     * OBJ_Release here AND let __gc fire later. */
    lua_pop(s_test_L, 1);
    lua_gc(s_test_L, LUA_GCCOLLECT, 0);
    (void)obj;
}

#define objWith(obj) \
    xmlWith(struct Object, obj, _make_test_obj(), _free_test_obj)

/* -----------------------------------------------------------------------
 * testWith — the full single open/close macro:
 *   • sets s_current_test and increments counter
 *   • takes a MEM_SNAPSHOT before the body
 *   • creates an object (obj) for use in the body via objWith
 *   • destroys the object automatically after the body
 *   • runs MEM_CHECK_LEAK automatically after the body
 *
 *   testWith("bool property set/get", obj) {
 *       bool_t val = TRUE;
 *       OBJ_SetPropertyValue(obj, "Enabled", &val);
 *       TEST_ASSERT(*(bool_t *)PROP_GetValue(...) == TRUE);
 *   }
 * ----------------------------------------------------------------------- */

#define testWith(name, obj) \
    TEST_WITH_MEM(name, _TEST_UNIQUE(_snap_)) \
    objWith(obj)

#endif /* __ORCA_H__ */

/* -----------------------------------------------------------------------
 * Test summary — call once at the end of main().
 * Returns 0 on success, 1 on any failure.
 * ----------------------------------------------------------------------- */

static inline int test_summary(void)
{
    if (s_tests_failed == 0) {
        printf("All %d C unit tests passed.\n", s_tests_run);
        return 0;
    }
    fprintf(stderr, "%d/%d C unit tests FAILED.\n",
            s_tests_failed, s_tests_run);
    return 1;
}

#endif /* TESTS_TEST_H */
