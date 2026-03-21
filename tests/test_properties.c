/*
 * test_properties.c — C unit tests for the ORCA property system and p_runtime.
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
 *   - p_runtime: Token_Create / Token_Release
 *   - p_runtime: OBJ_RunProgram with constant, arithmetic, and string expressions
 *   - p_runtime: PROP_Import — importing vm_register values into properties
 *   - p_runtime: PROP_AttachProgram + PROP_Update — expression → property
 *   - p_runtime: property import by name reference (.Property path)
 *
 * Compiled and linked against liborca.so via the `test-properties` Makefile
 * target (depends on `buildlib`).
 */

#include <source/core/core_local.h>
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
/* Runtime test component (separate from the property tests)           */
/* FullIdentifiers use the path form (e.g. fnv1a32(".Count")) so that  */
/* program expressions like ".Count" resolve via OBJ_FindPropertyByPath*/
/* ------------------------------------------------------------------ */

struct RTComp {
    int         Count;
    float       Value;
    const char* Label;
};

static struct PropertyType s_rtPropCount;
static struct PropertyType s_rtPropValue;
static struct PropertyType s_rtPropLabel;
static struct PropertyType s_rtProps[3];
static struct ClassDesc    s_rtClass;

static LRESULT RTComp_Proc(lpObject_t o, void* cmp, uint32_t msg,
                            wParam_t w, lParam_t l) {
    (void)o; (void)cmp; (void)msg; (void)w; (void)l;
    return 0;
}

static void register_runtime_class(void)
{
    /*
     * FullIdentifier == fnv1a32(".PropName") matches what
     * OBJ_FindPropertyByPath resolves when the program text is ".PropName".
     */
    memset(&s_rtPropCount, 0, sizeof(s_rtPropCount));
    strncpy(s_rtPropCount.Name, "Count", sizeof(s_rtPropCount.Name) - 1);
    strncpy(s_rtPropCount.Key,  "Count", sizeof(s_rtPropCount.Key)  - 1);
    s_rtPropCount.DataType        = kDataTypeInt;
    s_rtPropCount.DataSize        = sizeof(int);
    s_rtPropCount.ShortIdentifier = fnv1a32("Count");
    s_rtPropCount.FullIdentifier  = fnv1a32(".Count");
    s_rtPropCount.Offset          = offsetof(struct RTComp, Count);

    memset(&s_rtPropValue, 0, sizeof(s_rtPropValue));
    strncpy(s_rtPropValue.Name, "Value", sizeof(s_rtPropValue.Name) - 1);
    strncpy(s_rtPropValue.Key,  "Value", sizeof(s_rtPropValue.Key)  - 1);
    s_rtPropValue.DataType        = kDataTypeFloat;
    s_rtPropValue.DataSize        = sizeof(float);
    s_rtPropValue.ShortIdentifier = fnv1a32("Value");
    s_rtPropValue.FullIdentifier  = fnv1a32(".Value");
    s_rtPropValue.Offset          = offsetof(struct RTComp, Value);

    memset(&s_rtPropLabel, 0, sizeof(s_rtPropLabel));
    strncpy(s_rtPropLabel.Name, "Label", sizeof(s_rtPropLabel.Name) - 1);
    strncpy(s_rtPropLabel.Key,  "Label", sizeof(s_rtPropLabel.Key)  - 1);
    s_rtPropLabel.DataType        = kDataTypeString;
    s_rtPropLabel.DataSize        = sizeof(const char*);
    s_rtPropLabel.ShortIdentifier = fnv1a32("Label");
    s_rtPropLabel.FullIdentifier  = fnv1a32(".Label");
    s_rtPropLabel.Offset          = offsetof(struct RTComp, Label);

    s_rtProps[0] = s_rtPropCount;
    s_rtProps[1] = s_rtPropValue;
    s_rtProps[2] = s_rtPropLabel;

    memset(&s_rtClass, 0, sizeof(s_rtClass));
    s_rtClass.ObjProc       = RTComp_Proc;
    s_rtClass.Properties    = s_rtProps;
    s_rtClass.ClassName     = "RTComp";
    s_rtClass.ClassID       = fnv1a32("RTComp");
    s_rtClass.ClassSize     = sizeof(struct RTComp);
    s_rtClass.NumProperties = 3;
    s_rtClass.DefaultName   = "rtcomp";

    OBJ_RegisterClass(&s_rtClass);
}

static lpObject_t make_rt_object(void)
{
    return OBJ_MakeNativeObject(fnv1a32("RTComp"));
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
/* p_runtime tests                                                     */
/* ------------------------------------------------------------------ */

static void test_runtime_token_create_int(void)
{
    TEST_BEGIN("Token_Create: integer constant expression");
    struct token *prog = Token_Create("42");
    EXPECT(prog != NULL);
    if (prog) Token_Release(prog);
}

static void test_runtime_token_create_float(void)
{
    TEST_BEGIN("Token_Create: float constant expression");
    struct token *prog = Token_Create("3.14");
    EXPECT(prog != NULL);
    if (prog) Token_Release(prog);
}

static void test_runtime_token_create_string(void)
{
    TEST_BEGIN("Token_Create: string literal expression");
    struct token *prog = Token_Create("\"hello\"");
    EXPECT(prog != NULL);
    if (prog) Token_Release(prog);
}

static void test_runtime_token_create_arithmetic(void)
{
    TEST_BEGIN("Token_Create: arithmetic expression");
    struct token *prog = Token_Create("ADD(2, 3)");
    EXPECT(prog != NULL);
    if (prog) Token_Release(prog);
}

static void test_runtime_run_int_constant(void)
{
    TEST_BEGIN("OBJ_RunProgram: integer constant evaluates to int");
    lpObject_t obj = make_rt_object();
    struct token *prog = Token_Create("7");
    EXPECT(prog != NULL);
    if (prog) {
        struct vm_register r = {0};
        bool_t ok = OBJ_RunProgram(obj, prog, &r);
        EXPECT(ok);
        EXPECT(r.type == kDataTypeInt || r.type == kDataTypeFloat);
        EXPECT((int)r.value[0] == 7);
        Token_Release(prog);
    }
    destroy_object(obj);
}

static void test_runtime_run_float_constant(void)
{
    TEST_BEGIN("OBJ_RunProgram: float constant evaluates to float");
    lpObject_t obj = make_rt_object();
    struct token *prog = Token_Create("2.5");
    EXPECT(prog != NULL);
    if (prog) {
        struct vm_register r = {0};
        bool_t ok = OBJ_RunProgram(obj, prog, &r);
        EXPECT(ok);
        EXPECT(r.value[0] == 2.5f);
        Token_Release(prog);
    }
    destroy_object(obj);
}

static void test_runtime_run_string_constant(void)
{
    TEST_BEGIN("OBJ_RunProgram: string literal evaluates to string");
    lpObject_t obj = make_rt_object();
    struct token *prog = Token_Create("\"world\"");
    EXPECT(prog != NULL);
    if (prog) {
        struct vm_register r = {0};
        bool_t ok = OBJ_RunProgram(obj, prog, &r);
        EXPECT(ok);
        EXPECT(r.type == kDataTypeString);
        /* String is stored as a pointer in r.value via VM_REG_SET_STR */
        const char *str = *(const char *const *)r.value;
        EXPECT_STR_EQ(str, "world");
        Token_Release(prog);
    }
    destroy_object(obj);
}

static void test_runtime_run_arithmetic(void)
{
    TEST_BEGIN("OBJ_RunProgram: ADD(10, 5) == 15");
    lpObject_t obj = make_rt_object();
    struct token *prog = Token_Create("ADD(10, 5)");
    EXPECT(prog != NULL);
    if (prog) {
        struct vm_register r = {0};
        bool_t ok = OBJ_RunProgram(obj, prog, &r);
        EXPECT(ok);
        EXPECT((int)r.value[0] == 15);
        Token_Release(prog);
    }
    destroy_object(obj);
}

static void test_runtime_import_int(void)
{
    TEST_BEGIN("PROP_Import: int register into int property");
    lpObject_t obj = make_rt_object();
    lpProperty_t prop;
    OBJ_FindShortProperty(obj, "Count", &prop);
    EXPECT(prop != NULL);
    if (prop) {
        struct vm_register r = {0};
        r.type    = kDataTypeInt;
        r.size    = sizeof(int);
        r.value[0] = 99.0f;
        bool_t ok = PROP_Import(prop, kPropertyAttributeWholeProperty, &r);
        EXPECT(ok);
        EXPECT(!PROP_IsNull(prop));
        EXPECT(*(int*)PROP_GetValue(prop) == 99);
    }
    destroy_object(obj);
}

static void test_runtime_import_float(void)
{
    TEST_BEGIN("PROP_Import: float register into float property");
    lpObject_t obj = make_rt_object();
    lpProperty_t prop;
    OBJ_FindShortProperty(obj, "Value", &prop);
    EXPECT(prop != NULL);
    if (prop) {
        struct vm_register r = {0};
        r.type    = kDataTypeFloat;
        r.size    = sizeof(float);
        r.value[0] = 1.5f;
        bool_t ok = PROP_Import(prop, kPropertyAttributeWholeProperty, &r);
        EXPECT(ok);
        EXPECT(*(float*)PROP_GetValue(prop) == 1.5f);
    }
    destroy_object(obj);
}

static void test_runtime_import_string(void)
{
    TEST_BEGIN("PROP_Import: string register into string property");
    lpObject_t obj = make_rt_object();
    lpProperty_t prop;
    OBJ_FindShortProperty(obj, "Label", &prop);
    EXPECT(prop != NULL);
    if (prop) {
        /* Build a string vm_register the way the VM does it */
        struct vm_register r = {0};
        r.type = kDataTypeString;
        r.size = sizeof(const char*);
        const char *s = "imported";
        memcpy(r.value, &s, sizeof(s));
        bool_t ok = PROP_Import(prop, kPropertyAttributeWholeProperty, &r);
        EXPECT(ok);
        EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "imported");
    }
    destroy_object(obj);
}

static void test_runtime_attach_and_update_int(void)
{
    TEST_BEGIN("PROP_AttachProgram + PROP_Update: constant int program");
    lpObject_t obj = make_rt_object();
    lpProperty_t prop;
    OBJ_FindShortProperty(obj, "Count", &prop);
    EXPECT(prop != NULL);
    if (prop) {
        struct token *prog = Token_Create("21");
        EXPECT(prog != NULL);
        if (prog) {
            PROP_AttachProgram(prop, kPropertyAttributeWholeProperty, prog, "21");
            EXPECT(PROP_HasProgram(prop));
            /* Advance frame so PROP_Update's frame-guard doesn't skip execution */
            core.frame++;
            bool_t ok = PROP_Update(prop);
            EXPECT(ok);
            EXPECT(!PROP_IsNull(prop));
            EXPECT(*(int*)PROP_GetValue(prop) == 21);
        }
    }
    destroy_object(obj);
}

static void test_runtime_attach_and_update_string(void)
{
    TEST_BEGIN("PROP_AttachProgram + PROP_Update: constant string program");
    lpObject_t obj = make_rt_object();
    lpProperty_t prop;
    OBJ_FindShortProperty(obj, "Label", &prop);
    EXPECT(prop != NULL);
    if (prop) {
        struct token *prog = Token_Create("\"bound\"");
        EXPECT(prog != NULL);
        if (prog) {
            PROP_AttachProgram(prop, kPropertyAttributeWholeProperty, prog, "\"bound\"");
            EXPECT(PROP_HasProgram(prop));
            core.frame++;
            bool_t ok = PROP_Update(prop);
            EXPECT(ok);
            EXPECT(!PROP_IsNull(prop));
            EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "bound");
        }
    }
    destroy_object(obj);
}

/*
 * Property-reference by name: the expression ".Value" references the
 * property whose FullIdentifier == fnv1a32(".Value") on the same object.
 * RTComp properties are registered with exactly those identifiers.
 *
 * Flow: set Value=5.0, attach program ".Value" to Count, call PROP_Update —
 * Count should be set to 5 (via int conversion of the float).
 */
static void test_runtime_property_reference(void)
{
    TEST_BEGIN("p_runtime: property reference via .PropName binding");
    lpObject_t obj = make_rt_object();
    lpProperty_t propCount, propValue;
    OBJ_FindShortProperty(obj, "Count", &propCount);
    OBJ_FindShortProperty(obj, "Value", &propValue);
    EXPECT(propCount != NULL);
    EXPECT(propValue != NULL);
    if (!propCount || !propValue) { destroy_object(obj); return; }

    /* Set the source property */
    float v = 5.0f;
    PROP_SetValue(propValue, &v);
    EXPECT(!PROP_IsNull(propValue));

    /* Compile a program that reads .Value and binds it to Count */
    struct token *prog = Token_Create(".Value");
    EXPECT(prog != NULL);
    if (prog) {
        PROP_AttachProgram(propCount, kPropertyAttributeWholeProperty, prog, ".Value");
        EXPECT(PROP_HasProgram(propCount));
        core.frame++;
        bool_t ok = PROP_Update(propCount);
        EXPECT(ok);
        EXPECT(!PROP_IsNull(propCount));
        /* Count is int, Value is float 5.0 → expect 5 */
        EXPECT(*(int*)PROP_GetValue(propCount) == 5);
    }
    destroy_object(obj);
}

/*
 * String concatenation: ADD("foo", "bar") should produce "foobar"
 * and a program that writes it to a string property should work.
 */
static void test_runtime_string_concat_program(void)
{
    TEST_BEGIN("p_runtime: string concat ADD(\"foo\", \"bar\")");
    lpObject_t obj = make_rt_object();
    lpProperty_t prop;
    OBJ_FindShortProperty(obj, "Label", &prop);
    EXPECT(prop != NULL);
    if (prop) {
        struct token *prog = Token_Create("ADD(\"foo\", \"bar\")");
        EXPECT(prog != NULL);
        if (prog) {
            PROP_AttachProgram(prop, kPropertyAttributeWholeProperty,
                               prog, "ADD(\"foo\", \"bar\")");
            core.frame++;
            bool_t ok = PROP_Update(prop);
            EXPECT(ok);
            EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "foobar");
        }
    }
    destroy_object(obj);
}

/* ------------------------------------------------------------------ */

int main(void)
{
    printf("ORCA property system tests\n");
    printf("==========================\n");

    register_test_class();
    register_runtime_class();

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

    printf("\n--- p_runtime tests ---\n");
    test_runtime_token_create_int();
    test_runtime_token_create_float();
    test_runtime_token_create_string();
    test_runtime_token_create_arithmetic();
    test_runtime_run_int_constant();
    test_runtime_run_float_constant();
    test_runtime_run_string_constant();
    test_runtime_run_arithmetic();
    test_runtime_import_int();
    test_runtime_import_float();
    test_runtime_import_string();
    test_runtime_attach_and_update_int();
    test_runtime_attach_and_update_string();
    test_runtime_property_reference();
    test_runtime_string_concat_program();

    printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
    return s_tests_failed == 0 ? 0 : 1;
}
