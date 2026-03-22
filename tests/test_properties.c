/* tests/test_properties.c — C unit tests for the engine property system.
 *
 * Compiled into the orca binary when built with -DORCA_C_TESTS.
 * Run via:  $(TESTBIN_C) -test=tests/test_properties.c
 *
 * Demonstrates the three levels of test macro brevity from tests/test.h:
 *
 *   Level 0 — verbose (original):
 *     TEST_BEGIN("bool property set/get");
 *     long snap = MEM_SNAPSHOT();
 *     lpObject_t obj = make_object();
 *     ...
 *     destroy_object(obj);
 *     MEM_CHECK_LEAK(snap, s_current_test);
 *
 *   Level 1 — combined begin+snapshot (TEST_BEGIN_MEM):
 *     TEST_BEGIN_MEM("bool property set/get", snap);
 *     objWith(obj) { ... }
 *     MEM_CHECK_LEAK(snap, s_current_test);
 *
 *   Level 2 — single open/close macro (testWith):
 *     testWith("bool property set/get", obj) { ... }
 */

#include <include/api.h>
#include <include/orca.h>
#include "test.h"

/* -----------------------------------------------------------------------
 * Helpers
 * ----------------------------------------------------------------------- */

static lpProperty_t find_prop(lpObject_t obj, const char *name)
{
    lpProperty_t prop = NULL;
    OBJ_FindShortProperty(obj, name, &prop);
    return prop;
}

/* -----------------------------------------------------------------------
 * Bool property — testWith (Level 2: shortest form)
 * ----------------------------------------------------------------------- */

static void test_bool_property(void)
{
    testWith("bool property set/get", obj) {
        bool_t val = TRUE;
        OBJ_SetPropertyValue(obj, "Visible", &val);

        lpProperty_t prop = find_prop(obj, "Visible");
        TEST_ASSERT(prop != NULL);
        TEST_ASSERT(*(bool_t *)PROP_GetValue(prop) == TRUE);

        val = FALSE;
        OBJ_SetPropertyValue(obj, "Visible", &val);
        TEST_ASSERT(*(bool_t *)PROP_GetValue(prop) == FALSE);
    }
}

/* -----------------------------------------------------------------------
 * Float property — TEST_BEGIN_MEM + objWith (Level 1)
 * ----------------------------------------------------------------------- */

static void test_float_property(void)
{
    TEST_BEGIN_MEM("float property set/get", snap);
    objWith(obj) {
        float w = 123.0f;
        OBJ_SetPropertyValue(obj, "Width", &w);

        lpProperty_t prop = find_prop(obj, "Width");
        TEST_ASSERT(prop != NULL);
        TEST_ASSERT(*(float *)PROP_GetValue(prop) == 123.0f);

        w = 0.0f;
        OBJ_SetPropertyValue(obj, "Width", &w);
        TEST_ASSERT(*(float *)PROP_GetValue(prop) == 0.0f);
    }
    MEM_CHECK_LEAK(snap, s_current_test);
}

/* -----------------------------------------------------------------------
 * Multiple properties on a single object — testWith (Level 2)
 * ----------------------------------------------------------------------- */

static void test_multiple_properties(void)
{
    testWith("multiple property set/get", obj) {
        float h = 200.0f;
        bool_t snap = FALSE;

        OBJ_SetPropertyValue(obj, "Height", &h);
        OBJ_SetPropertyValue(obj, "SnapToPixel", &snap);

        lpProperty_t ph = find_prop(obj, "Height");
        lpProperty_t ps = find_prop(obj, "SnapToPixel");

        TEST_ASSERT(ph != NULL && ps != NULL);
        TEST_ASSERT(*(float *)PROP_GetValue(ph) == 200.0f);
        TEST_ASSERT(*(bool_t *)PROP_GetValue(ps) == FALSE);
    }
}

/* -----------------------------------------------------------------------
 * Property types — TEST_WITH_MEM (Level 1b, no manual leak check needed)
 * ----------------------------------------------------------------------- */

static void test_property_type(void)
{
    TEST_WITH_MEM("property DataType reported correctly", snap) {
        objWith(obj) {
            /* Trigger property creation by setting a value */
            bool_t val = TRUE;
            OBJ_SetPropertyValue(obj, "ClipChildren", &val);

            lpProperty_t prop = find_prop(obj, "ClipChildren");
            TEST_ASSERT(prop != NULL);
            TEST_ASSERT(PROP_GetType(prop) == kDataTypeBool);
            TEST_ASSERT(PROP_GetSize(prop) == sizeof(bool_t));
        }
    }
}

/* -----------------------------------------------------------------------
 * Missing property — must not crash
 * ----------------------------------------------------------------------- */

static void test_missing_property(void)
{
    testWith("missing property is NULL", obj) {
        lpProperty_t prop = find_prop(obj, "DoesNotExist_XYZ");
        TEST_ASSERT(prop == NULL);
    }
}

/* -----------------------------------------------------------------------
 * Suite entry point
 * ----------------------------------------------------------------------- */

void run_property_tests(void)
{
    test_bool_property();
    test_float_property();
    test_multiple_properties();
    test_property_type();
    test_missing_property();
}
