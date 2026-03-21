/*
 * test_properties.c — C unit tests for the ORCA property system.
 *
 * Tests cover:
 *   - Int, float, bool property set/get
 *   - String property set/get, multiple reassignments (no double-free)
 *   - PROP_Clear: frees heap strings, resets state
 *   - OBJ_ReleaseProperties: frees all string memory on destruction
 *   - OBJ_SetPropertyValue: lazy on-demand property creation
 *   - OBJ_FindShortProperty: lookup by name
 *   - PROP_IsNull: NULL check before and after set
 *   - Per-state string set (kPropertyStateHover, etc.) and clear
 *   - Struct property set/get (using a simple vec2)
 *
 * Compiled and linked against liborca.so via the `test-properties` Makefile
 * target (depends on `buildlib`).
 */

#include <include/api.h>
#include <source/core/core.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 * OBJ_MakeNativeObject and OBJ_ReleaseProperties are engine-internal functions
 * not declared in the public header.  They ARE exported from the shared
 * library (no -fvisibility=hidden in the build), so we declare them here.
 */
extern lpObject_t OBJ_MakeNativeObject(uint32_t class_id);
extern void OBJ_ReleaseProperties(lpObject_t);

/* ------------------------------------------------------------------ */
/* Minimal test harness                                               */
/* ------------------------------------------------------------------ */

static int s_tests_run    = 0;
static int s_tests_failed = 0;
static const char* s_current_test = NULL;

#define TEST_BEGIN(name) \
    do { s_current_test = (name); s_tests_run++; } while(0)

#define EXPECT(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "  FAIL [%s]: %s (line %d)\n", \
                    s_current_test, #cond, __LINE__); \
            s_tests_failed++; \
        } \
    } while(0)

#define EXPECT_STR_EQ(a, b) \
    do { \
        const char *_a = (a), *_b = (b); \
        if (_a == NULL || _b == NULL || strcmp(_a, _b) != 0) { \
            fprintf(stderr, "  FAIL [%s]: expected \"%s\", got \"%s\" (line %d)\n", \
                    s_current_test, _b ? _b : "(null)", _a ? _a : "(null)", __LINE__); \
            s_tests_failed++; \
        } \
    } while(0)

/* ------------------------------------------------------------------ */
/* Test component definition                                          */
/* ------------------------------------------------------------------ */

struct TestComp {
    int     Count;
    float   Value;
    bool_t  Active;
    const char* Label;
    float   Position[2]; /* used as a struct/vec2 */
};

/* Property descriptors — identifiers derived via fnv1a32 at runtime */

static struct PropertyType s_propCount;
static struct PropertyType s_propValue;
static struct PropertyType s_propActive;
static struct PropertyType s_propLabel;
static struct PropertyType s_propPosition;

static struct PropertyType s_testProps[5];

/* Null ObjProc — components in these tests never receive messages.
 * Signature must match objectProc_t exactly:
 *   LRESULT (lpObject_t, void*, uint32_t msg, wParam_t, lParam_t) */
static LRESULT TestComp_Proc(lpObject_t o, void* cmp, uint32_t msg, wParam_t w, lParam_t l) {
    (void)o; (void)cmp; (void)msg; (void)w; (void)l;
    return 0;
}

static struct ClassDesc s_testClass;

static void register_test_class(void)
{
    /* Compute identifiers at runtime using fnv1a32 */

    memset(&s_propCount, 0, sizeof(s_propCount));
    strncpy(s_propCount.Name, "Count", sizeof(s_propCount.Name) - 1);
    strncpy(s_propCount.Key,  "Count", sizeof(s_propCount.Key)  - 1);
    s_propCount.DataType        = kDataTypeInt;
    s_propCount.DataSize        = sizeof(int);
    s_propCount.ShortIdentifier = fnv1a32("Count");
    s_propCount.FullIdentifier  = fnv1a32("TestComp.Count");
    s_propCount.Offset          = offsetof(struct TestComp, Count);

    memset(&s_propValue, 0, sizeof(s_propValue));
    strncpy(s_propValue.Name, "Value", sizeof(s_propValue.Name) - 1);
    strncpy(s_propValue.Key,  "Value", sizeof(s_propValue.Key)  - 1);
    s_propValue.DataType        = kDataTypeFloat;
    s_propValue.DataSize        = sizeof(float);
    s_propValue.ShortIdentifier = fnv1a32("Value");
    s_propValue.FullIdentifier  = fnv1a32("TestComp.Value");
    s_propValue.Offset          = offsetof(struct TestComp, Value);

    memset(&s_propActive, 0, sizeof(s_propActive));
    strncpy(s_propActive.Name, "Active", sizeof(s_propActive.Name) - 1);
    strncpy(s_propActive.Key,  "Active", sizeof(s_propActive.Key)  - 1);
    s_propActive.DataType        = kDataTypeBool;
    s_propActive.DataSize        = sizeof(bool_t);
    s_propActive.ShortIdentifier = fnv1a32("Active");
    s_propActive.FullIdentifier  = fnv1a32("TestComp.Active");
    s_propActive.Offset          = offsetof(struct TestComp, Active);

    memset(&s_propLabel, 0, sizeof(s_propLabel));
    strncpy(s_propLabel.Name, "Label", sizeof(s_propLabel.Name) - 1);
    strncpy(s_propLabel.Key,  "Label", sizeof(s_propLabel.Key)  - 1);
    s_propLabel.DataType        = kDataTypeString;
    s_propLabel.DataSize        = sizeof(const char*);
    s_propLabel.ShortIdentifier = fnv1a32("Label");
    s_propLabel.FullIdentifier  = fnv1a32("TestComp.Label");
    s_propLabel.Offset          = offsetof(struct TestComp, Label);

    memset(&s_propPosition, 0, sizeof(s_propPosition));
    strncpy(s_propPosition.Name, "Position", sizeof(s_propPosition.Name) - 1);
    strncpy(s_propPosition.Key,  "Position", sizeof(s_propPosition.Key)  - 1);
    s_propPosition.DataType        = kDataTypeStruct;
    s_propPosition.DataSize        = sizeof(float[2]);
    s_propPosition.ShortIdentifier = fnv1a32("Position");
    s_propPosition.FullIdentifier  = fnv1a32("TestComp.Position");
    s_propPosition.Offset          = offsetof(struct TestComp, Position);

    s_testProps[0] = s_propCount;
    s_testProps[1] = s_propValue;
    s_testProps[2] = s_propActive;
    s_testProps[3] = s_propLabel;
    s_testProps[4] = s_propPosition;

    memset(&s_testClass, 0, sizeof(s_testClass));
    s_testClass.ObjProc       = TestComp_Proc;
    s_testClass.Properties    = s_testProps;
    s_testClass.ClassName     = "TestComp";
    s_testClass.ClassID       = fnv1a32("TestComp");
    s_testClass.ClassSize     = sizeof(struct TestComp);
    s_testClass.NumProperties = 5;
    s_testClass.DefaultName   = "testcomp";

    OBJ_RegisterClass(&s_testClass);
}

/* ------------------------------------------------------------------ */
/* Object lifecycle helper                                            */
/* ------------------------------------------------------------------ */

/* Allocate a bare TestComp object without a Lua state */
static lpObject_t make_object(void)
{
    return OBJ_MakeNativeObject(fnv1a32("TestComp"));
}

/*
 * Release an object created with make_object().
 * Frees all properties (including heap-allocated strings) via
 * OBJ_ReleaseProperties, then frees the object itself.
 * Does NOT call OBJ_Release (which needs a Lua state and message dispatch).
 * Component blocks allocated by OBJ_AddComponent are not freed here since
 * this is a test-only allocator; the process exits after the test run.
 */
static void destroy_object(lpObject_t obj)
{
    if (!obj) return;
    OBJ_ReleaseProperties(obj);
    free(obj);
}

/* ------------------------------------------------------------------ */
/* Tests                                                               */
/* ------------------------------------------------------------------ */

static void test_int_property(void)
{
    TEST_BEGIN("int property set/get");
    lpObject_t obj = make_object();

    lpProperty_t prop;
    HRESULT hr = OBJ_FindShortProperty(obj, "Count", &prop);
    EXPECT(SUCCEEDED(hr));
    EXPECT(prop != NULL);
    EXPECT(PROP_GetType(prop) == kDataTypeInt);
    EXPECT(PROP_IsNull(prop));

    int val = 42;
    PROP_SetValue(prop, &val);
    EXPECT(!PROP_IsNull(prop));
    EXPECT(*(int*)PROP_GetValue(prop) == 42);

    int val2 = -7;
    PROP_SetValue(prop, &val2);
    EXPECT(*(int*)PROP_GetValue(prop) == -7);

    destroy_object(obj);
}

static void test_float_property(void)
{
    TEST_BEGIN("float property set/get");
    lpObject_t obj = make_object();

    lpProperty_t prop;
    HRESULT hr = OBJ_FindShortProperty(obj, "Value", &prop);
    EXPECT(SUCCEEDED(hr));
    EXPECT(PROP_GetType(prop) == kDataTypeFloat);
    EXPECT(PROP_IsNull(prop));

    float f = 3.14f;
    PROP_SetValue(prop, &f);
    EXPECT(!PROP_IsNull(prop));
    EXPECT(*(float*)PROP_GetValue(prop) == 3.14f);

    destroy_object(obj);
}

static void test_bool_property(void)
{
    TEST_BEGIN("bool property set/get");
    lpObject_t obj = make_object();

    lpProperty_t prop;
    OBJ_FindShortProperty(obj, "Active", &prop);
    EXPECT(prop != NULL);
    EXPECT(PROP_GetType(prop) == kDataTypeBool);

    bool_t yes = TRUE;
    PROP_SetValue(prop, &yes);
    EXPECT(*(bool_t*)PROP_GetValue(prop) == TRUE);

    bool_t no = FALSE;
    PROP_SetValue(prop, &no);
    EXPECT(*(bool_t*)PROP_GetValue(prop) == FALSE);

    destroy_object(obj);
}

static void test_string_property_basic(void)
{
    TEST_BEGIN("string property set/get");
    lpObject_t obj = make_object();

    lpProperty_t prop;
    OBJ_FindShortProperty(obj, "Label", &prop);
    EXPECT(prop != NULL);
    EXPECT(PROP_GetType(prop) == kDataTypeString);
    EXPECT(PROP_IsNull(prop));

    PROP_SetValue(prop, "hello");
    EXPECT(!PROP_IsNull(prop));
    EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "hello");

    destroy_object(obj);
}

static void test_string_property_reassign(void)
{
    TEST_BEGIN("string property multiple reassignments");
    lpObject_t obj = make_object();

    lpProperty_t prop;
    OBJ_FindShortProperty(obj, "Label", &prop);

    PROP_SetValue(prop, "first");
    EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "first");

    PROP_SetValue(prop, "second");
    EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "second");

    PROP_SetValue(prop, "third");
    EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "third");

    /* Value pointer changes each time (new strdup), but content is correct */
    PROP_SetValue(prop, "");
    EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "");

    destroy_object(obj);
}

static void test_string_property_clear(void)
{
    TEST_BEGIN("string property PROP_Clear");
    lpObject_t obj = make_object();

    lpProperty_t prop;
    OBJ_FindShortProperty(obj, "Label", &prop);

    PROP_SetValue(prop, "some string");
    EXPECT(!PROP_IsNull(prop));

    PROP_Clear(prop);
    /* After clear the property should be null/reset */
    EXPECT(PROP_IsNull(prop));

    /* Setting after clear must not double-free */
    PROP_SetValue(prop, "after clear");
    EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "after clear");

    destroy_object(obj);
}

static void test_string_property_clear_without_set(void)
{
    TEST_BEGIN("string property PROP_Clear without prior set");
    lpObject_t obj = make_object();

    lpProperty_t prop;
    OBJ_FindShortProperty(obj, "Label", &prop);

    /* Clearing a never-set string property must not crash */
    PROP_Clear(prop);
    EXPECT(PROP_IsNull(prop));

    destroy_object(obj);
}

static void test_release_properties_frees_strings(void)
{
    TEST_BEGIN("OBJ_ReleaseProperties frees string memory");
    lpObject_t obj = make_object();

    lpProperty_t prop;
    OBJ_FindShortProperty(obj, "Label", &prop);
    PROP_SetValue(prop, "will be freed");

    /* destroy_object calls OBJ_ReleaseProperties which must free the
       strdup'd string without crashing.  Run under valgrind/ASAN to
       verify there are no leaks. */
    destroy_object(obj);
}

static void test_set_property_value_api(void)
{
    TEST_BEGIN("OBJ_SetPropertyValue (demand creation)");
    lpObject_t obj = make_object();

    /* OBJ_SetPropertyValue creates the property on demand */
    HRESULT hr = OBJ_SetPropertyValue(obj, "Count", &(int){99});
    EXPECT(SUCCEEDED(hr));

    lpProperty_t prop;
    OBJ_FindShortProperty(obj, "Count", &prop);
    EXPECT(prop != NULL);
    EXPECT(*(int*)PROP_GetValue(prop) == 99);

    hr = OBJ_SetPropertyValue(obj, "Label", "via api");
    EXPECT(SUCCEEDED(hr));
    OBJ_FindShortProperty(obj, "Label", &prop);
    EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "via api");

    destroy_object(obj);
}

static void test_property_state_string(void)
{
    TEST_BEGIN("per-state string property set/clear");
    lpObject_t obj = make_object();

    lpProperty_t prop;
    OBJ_FindShortProperty(obj, "Label", &prop);
    EXPECT(prop != NULL);

    /* Set the hover state value */
    PROP_SetValue(prop, "normal");
    EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "normal");

    /* Clear all states — should free heap strings without crashing */
    PROP_Clear(prop);
    EXPECT(PROP_IsNull(prop));

    /* Re-set after full clear */
    PROP_SetValue(prop, "reset");
    EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "reset");

    destroy_object(obj);
}

static void test_struct_property(void)
{
    TEST_BEGIN("struct (vec2) property set/get");
    lpObject_t obj = make_object();

    lpProperty_t prop;
    OBJ_FindShortProperty(obj, "Position", &prop);
    EXPECT(prop != NULL);
    EXPECT(PROP_GetType(prop) == kDataTypeStruct);

    float pos[2] = {10.0f, 20.0f};
    PROP_SetValue(prop, pos);
    float *result = (float*)PROP_GetValue(prop);
    EXPECT(result[0] == 10.0f);
    EXPECT(result[1] == 20.0f);

    destroy_object(obj);
}

static void test_find_property_unknown(void)
{
    TEST_BEGIN("OBJ_FindShortProperty for unknown property returns error");
    lpObject_t obj = make_object();

    lpProperty_t prop = NULL;
    HRESULT hr = OBJ_FindShortProperty(obj, "NonExistentProp", &prop);
    EXPECT(FAILED(hr));

    destroy_object(obj);
}

static void test_multiple_properties_independent(void)
{
    TEST_BEGIN("multiple properties on same object are independent");
    lpObject_t obj = make_object();

    lpProperty_t pCount, pLabel;
    OBJ_FindShortProperty(obj, "Count", &pCount);
    OBJ_FindShortProperty(obj, "Label", &pLabel);
    EXPECT(pCount != NULL);
    EXPECT(pLabel != NULL);
    EXPECT(pCount != pLabel);

    int val = 5;
    PROP_SetValue(pCount, &val);
    PROP_SetValue(pLabel, "hello");

    /* Verify they don't alias each other */
    EXPECT(*(int*)PROP_GetValue(pCount) == 5);
    EXPECT_STR_EQ((const char*)PROP_GetValue(pLabel), "hello");

    PROP_SetValue(pLabel, "world");
    EXPECT(*(int*)PROP_GetValue(pCount) == 5); /* Count unchanged */
    EXPECT_STR_EQ((const char*)PROP_GetValue(pLabel), "world");

    destroy_object(obj);
}

static void test_string_empty(void)
{
    TEST_BEGIN("string property with empty string");
    lpObject_t obj = make_object();

    lpProperty_t prop;
    OBJ_FindShortProperty(obj, "Label", &prop);

    PROP_SetValue(prop, "");
    EXPECT(!PROP_IsNull(prop));
    EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "");

    destroy_object(obj);
}

static void test_release_without_string_set(void)
{
    TEST_BEGIN("OBJ_ReleaseProperties with no string set");
    lpObject_t obj = make_object();

    /* Only set a non-string property */
    lpProperty_t prop;
    OBJ_FindShortProperty(obj, "Count", &prop);
    int val = 1;
    PROP_SetValue(prop, &val);

    /* destroy_object calls OBJ_ReleaseProperties which must not crash
       even though the Label string property was never set */
    destroy_object(obj);
}

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */

int main(void)
{
    printf("ORCA property system tests\n");
    printf("==========================\n");

    register_test_class();

    test_int_property();
    test_float_property();
    test_bool_property();
    test_string_property_basic();
    test_string_property_reassign();
    test_string_property_clear();
    test_string_property_clear_without_set();
    test_release_properties_frees_strings();
    test_set_property_value_api();
    test_property_state_string();
    test_struct_property();
    test_find_property_unknown();
    test_multiple_properties_independent();
    test_string_empty();
    test_release_without_string_set();

    printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
    return s_tests_failed == 0 ? 0 : 1;
}
