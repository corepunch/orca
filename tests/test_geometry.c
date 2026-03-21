/* tests/test_geometry.c — C unit tests for the geometry (math) module.
 *
 * These tests are standalone: they link against geometry_ops.c and require
 * no Lua state, no renderer, and no windowing system.
 *
 * Demonstrates TEST_WITH_MEM and TEST_BEGIN_MEM macros from tests/test.h.
 */

#include "test.h"

#include <include/shared.h>

/* helper: approximate float equality */
static int feq(float a, float b) { return (a - b < 0.0001f) && (b - a < 0.0001f); }

/* ---------------------------------------------------------------------- */

static void test_vec2_add(void)
{
    TEST_BEGIN_MEM("vec2 add", snap);

    struct vec2 a = { 1.0f, 2.0f };
    struct vec2 b = { 3.0f, 4.0f };
    struct vec2 r = VEC2_Add(&a, &b);

    TEST_ASSERT(feq(r.x, 4.0f));
    TEST_ASSERT(feq(r.y, 6.0f));

    MEM_CHECK_LEAK(snap, s_current_test);
}

static void test_vec2_sub(void)
{
    TEST_BEGIN_MEM("vec2 sub", snap);

    struct vec2 a = { 5.0f, 7.0f };
    struct vec2 b = { 2.0f, 3.0f };
    struct vec2 r = VEC2_Sub(&a, &b);

    TEST_ASSERT(feq(r.x, 3.0f));
    TEST_ASSERT(feq(r.y, 4.0f));

    MEM_CHECK_LEAK(snap, s_current_test);
}

static void test_vec2_scale(void)
{
    /* TEST_WITH_MEM — for-loop form: no explicit MEM_CHECK_LEAK needed */
    TEST_WITH_MEM("vec2 scale", snap) {
        struct vec2 v = { 2.0f, 3.0f };
        struct vec2 r = VEC2_Scale(&v, 2.0f);
        TEST_ASSERT(feq(r.x, 4.0f));
        TEST_ASSERT(feq(r.y, 6.0f));
    }
}

static void test_vec2_dot(void)
{
    TEST_WITH_MEM("vec2 dot product", snap) {
        struct vec2 a = { 1.0f, 0.0f };
        struct vec2 b = { 0.0f, 1.0f };
        TEST_ASSERT(feq(VEC2_Dot(&a, &b), 0.0f)); /* perpendicular */

        struct vec2 c = { 1.0f, 0.0f };
        TEST_ASSERT(feq(VEC2_Dot(&a, &c), 1.0f)); /* parallel unit vectors */
    }
}

static void test_vec2_length(void)
{
    TEST_WITH_MEM("vec2 length", snap) {
        struct vec2 v = { 3.0f, 4.0f };
        TEST_ASSERT(feq(VEC2_Len(&v), 5.0f)); /* 3-4-5 right triangle */
    }
}

static void test_vec3_add(void)
{
    TEST_WITH_MEM("vec3 add", snap) {
        struct vec3 a = { 1.0f, 2.0f, 3.0f };
        struct vec3 b = { 4.0f, 5.0f, 6.0f };
        struct vec3 r = VEC3_Add(&a, &b);
        TEST_ASSERT(feq(r.x, 5.0f));
        TEST_ASSERT(feq(r.y, 7.0f));
        TEST_ASSERT(feq(r.z, 9.0f));
    }
}

static void test_vec3_cross(void)
{
    TEST_WITH_MEM("vec3 cross product", snap) {
        struct vec3 x = { 1.0f, 0.0f, 0.0f };
        struct vec3 y = { 0.0f, 1.0f, 0.0f };
        struct vec3 r = VEC3_Cross(&x, &y); /* should be (0,0,1) */
        TEST_ASSERT(feq(r.x, 0.0f));
        TEST_ASSERT(feq(r.y, 0.0f));
        TEST_ASSERT(feq(r.z, 1.0f));
    }
}

static void test_vec3_dot(void)
{
    TEST_WITH_MEM("vec3 dot product", snap) {
        struct vec3 a = { 1.0f, 0.0f, 0.0f };
        struct vec3 b = { 0.0f, 1.0f, 0.0f };
        TEST_ASSERT(feq(VEC3_Dot(&a, &b), 0.0f)); /* perpendicular */

        struct vec3 c = { 2.0f, 0.0f, 0.0f };
        TEST_ASSERT(feq(VEC3_Dot(&a, &c), 2.0f));
    }
}

static void test_vec3_length(void)
{
    TEST_WITH_MEM("vec3 length", snap) {
        struct vec3 v = { 0.0f, 3.0f, 4.0f }; /* 3-4-5 in y-z plane */
        TEST_ASSERT(feq(VEC3_Len(&v), 5.0f));
    }
}

/* ---------------------------------------------------------------------- */

void run_geometry_tests(void)
{
    test_vec2_add();
    test_vec2_sub();
    test_vec2_scale();
    test_vec2_dot();
    test_vec2_length();
    test_vec3_add();
    test_vec3_cross();
    test_vec3_dot();
    test_vec3_length();
}
