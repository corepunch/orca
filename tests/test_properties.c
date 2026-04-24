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
 *   - project-defined (dynamic) string property type: set/get with correct DataSize
 *   - kDataTypeColor: set/get, channel import/export, COLOR4 binding
 *
 * Compiled and linked against liborca.so via the `test-properties` Makefile
 * target (depends on `buildlib`).
 *
 * Build with -DTEST_MEMORY to enable malloc/free/strdup interception and
 * per-test allocation tracking.  Memory leak coverage for all functional
 * tests is provided automatically through that mechanism — there are no
 * separate memleak test functions.
 */

#include "mem_tracker.h"

#include <source/core/core_local.h>
#include <source/core/core.h>
#include <source/core/object/object_internal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 * OBJ_Create and OBJ_ReleaseProperties are engine-internal functions
 * not declared in the public header.  They ARE exported from the shared
 * library (no -fvisibility=hidden in the build), so we declare them here.
 */
extern lpObject_t OBJ_Create(uint32_t class_id);
extern void OBJ_ReleaseProperties(lpObject_t);
extern struct color COLOR_Parse(lpcString_t);

/* ------------------------------------------------------------------ */
/* Minimal test harness                                               */
/* ------------------------------------------------------------------ */

static int s_tests_run    = 0;
static int s_tests_failed = 0;
static const char* s_current_test = NULL;

#define TEST_BEGIN(name)

#define EXPECT(...) \
    if (!(__VA_ARGS__)) { \
        fprintf(stderr, "  FAIL [%s]: %s (line %d)\n", \
                s_current_test, #__VA_ARGS__, __LINE__); \
        s_tests_failed++; \
        break; \
    } 

#define EXPECT_OK(hr) EXPECT((hr) == NOERROR)
#define EXPECT_STR_EQ(a, b) EXPECT((a) && (b) && !strcmp(a, b))

/*
 * PROP_STR — wrap a string literal as a char** suitable for PROP_SetValue.
 * String properties store a char* internally, so the API expects void const*
 * pointing to a char* (i.e. char**).  Use PROP_STR("literal") wherever a
 * string literal would otherwise be passed directly.
 */
#define PROP_STR(s) (&(const char*){(s)})

/*
 * PROP_TEST — shorthand for the common find→type→null→set→not-null→value
 * pattern shared by simple scalar/string property tests.  _p names the
 * lpProperty_t used inside cmpval so callers can write e.g.
 *   PROP_TEST(obj, "Value", kDataTypeFloat, &(float){3.14f},
 *             *(float*)PROP_GetValue(_p) == 3.14f);
 * The block (not do…while) lets EXPECT's break reach the enclosing WITH loop.
 */
#define PROP_TEST(obj, name, dtype, setval, cmpval)                          \
    {                                                                        \
        lpProperty_t _p;                                                     \
        EXPECT_OK(OBJ_FindShortProperty(obj, name, &_p));                    \
        EXPECT(PROP_GetType(_p) == dtype);                                   \
        EXPECT(PROP_IsNull(_p));                                             \
        PROP_SetValue(_p, setval);                                           \
        EXPECT(!PROP_IsNull(_p));                                            \
        EXPECT(cmpval);                                                      \
    }

/*
 * RUN_PROG — shorthand for create-token → run-program → release-token.
 * Wraps the three inner lines that appear in every "run a constant/arithmetic
 * expression" test.  EXPECT inside uses break which exits the WITH loop.
 */
#define RUN_PROG(obj, expr, reg)                                             \
    WITH(struct token, _t, Token_Create(expr), Token_Release) {              \
        EXPECT(_t != NULL);                                                  \
        EXPECT(OBJ_RunProgram(obj, _t, reg));                                \
    }

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

/* Property descriptors — identifiers pre-computed from fnv1a32 at build time,
 * matching what register_test_class previously computed at runtime. */

/* fnv1a32("Count")=0xe1e7b894  fnv1a32("TestComp.Count")=0x1be61373  */
/* fnv1a32("Value")=0xd147f96a  fnv1a32("TestComp.Value")=0xa6d2bd25  */
/* fnv1a32("Active")=0x1f89134f fnv1a32("TestComp.Active")=0xcc3a31be */
/* fnv1a32("Label")=0x9eccf29d  fnv1a32("TestComp.Label")=0x7707c43a  */
/* fnv1a32("Position")=0xe27f342a fnv1a32("TestComp.Position")=0x77494c8f */
static struct PropertyType s_testProps[5] = {
    { .Name = "Count",    .Key = "Count",    .DataType = kDataTypeInt,    .DataSize = sizeof(int),           .ShortIdentifier = 0xe1e7b894, .FullIdentifier = 0x1be61373, .Offset = offsetof(struct TestComp, Count)    },
    { .Name = "Value",    .Key = "Value",    .DataType = kDataTypeFloat,  .DataSize = sizeof(float),         .ShortIdentifier = 0xd147f96a, .FullIdentifier = 0xa6d2bd25, .Offset = offsetof(struct TestComp, Value)    },
    { .Name = "Active",   .Key = "Active",   .DataType = kDataTypeBool,   .DataSize = sizeof(bool_t),        .ShortIdentifier = 0x1f89134f, .FullIdentifier = 0xcc3a31be, .Offset = offsetof(struct TestComp, Active)   },
    { .Name = "Label",    .Key = "Label",    .DataType = kDataTypeString, .DataSize = sizeof(const char*),   .ShortIdentifier = 0x9eccf29d, .FullIdentifier = 0x7707c43a, .Offset = offsetof(struct TestComp, Label)    },
    { .Name = "Position", .Key = "Position", .DataType = kDataTypeStruct, .DataSize = sizeof(float[2]),      .ShortIdentifier = 0xe27f342a, .FullIdentifier = 0x77494c8f, .Offset = offsetof(struct TestComp, Position) },
};

/* Null ObjProc — components in these tests never receive messages.
 * Signature must match objectProc_t exactly:
 *   LRESULT (lpObject_t, void*, uint32_t msg, wParam_t, lParam_t) */
static LRESULT TestComp_Proc(lpObject_t o, void* cmp, uint32_t msg, wParam_t w, lParam_t l) {
    (void)o; (void)cmp; (void)msg; (void)w; (void)l;
    return 0;
}

static struct ClassDesc s_testClass = {
    .ObjProc       = TestComp_Proc,
    .Properties    = s_testProps,
    .ClassName     = "TestComp",
    .ClassID       = 0xc87d9b4a, /* fnv1a32("TestComp") */
    .ClassSize     = sizeof(struct TestComp),
    .NumProperties = 5,
    .DefaultName   = "testcomp",
};

static void register_test_class(void)
{
    OBJ_RegisterClass(&s_testClass);
}

/* ------------------------------------------------------------------ */
/* Runtime test component (separate from the property tests)           */
/* FullIdentifiers use the plain property name (e.g. fnv1a32("Count")) so  */
/* that bindings like "{./Count}" resolve via OBJ_FindPropertyByPath.       */
/* ------------------------------------------------------------------ */

struct RTComp {
    int         Count;
    float       Value;
    const char* Label;
};

/* fnv1a32("Count")=0xe1e7b894 (ShortIdentifier == FullIdentifier for runtime bindings) */
/* fnv1a32("Value")=0xd147f96a */
/* fnv1a32("Label")=0x9eccf29d */
static struct PropertyType s_rtProps[3] = {
    { .Name = "Count", .Key = "Count", .DataType = kDataTypeInt,    .DataSize = sizeof(int),         .ShortIdentifier = 0xe1e7b894, .FullIdentifier = 0xe1e7b894, .Offset = offsetof(struct RTComp, Count) },
    { .Name = "Value", .Key = "Value", .DataType = kDataTypeFloat,  .DataSize = sizeof(float),       .ShortIdentifier = 0xd147f96a, .FullIdentifier = 0xd147f96a, .Offset = offsetof(struct RTComp, Value) },
    { .Name = "Label", .Key = "Label", .DataType = kDataTypeString, .DataSize = sizeof(const char*), .ShortIdentifier = 0x9eccf29d, .FullIdentifier = 0x9eccf29d, .Offset = offsetof(struct RTComp, Label) },
};

static LRESULT RTComp_Proc(lpObject_t o, void* cmp, uint32_t msg,
                            wParam_t w, lParam_t l) {
    (void)o; (void)cmp; (void)msg; (void)w; (void)l;
    return 0;
}

static struct ClassDesc s_rtClass = {
    .ObjProc       = RTComp_Proc,
    .Properties    = s_rtProps,
    .ClassName     = "RTComp",
    .ClassID       = 0xfc2d6a10, /* fnv1a32("RTComp") */
    .ClassSize     = sizeof(struct RTComp),
    .NumProperties = 3,
    .DefaultName   = "rtcomp",
};

static void register_runtime_class(void)
{
    OBJ_RegisterClass(&s_rtClass);
}

static lpObject_t make_rt_object(void) {
    return OBJ_Create(fnv1a32("RTComp"));
}

/* ------------------------------------------------------------------ */
/* RTString2Comp — two string properties for source→target binding    */
/* tests.  FullIdentifiers are plain fnv1a32(name) so {./Source} and  */
/* {./Target} resolve correctly via OBJ_FindPropertyByPath.           */
/* ------------------------------------------------------------------ */

struct RTString2Comp {
    const char* Source;
    const char* Target;
};

/* fnv1a32("Source") = 0x5b67e082  fnv1a32("Target") = 0x6d9978b7 */
static struct PropertyType s_rtString2Props[2] = {
    { .Name = "Source", .Key = "Source", .DataType = kDataTypeString,
      .DataSize = sizeof(const char*),
      .Offset = offsetof(struct RTString2Comp, Source) },
    { .Name = "Target", .Key = "Target", .DataType = kDataTypeString,
      .DataSize = sizeof(const char*),
      .Offset = offsetof(struct RTString2Comp, Target) },
};

static LRESULT RTString2Comp_Proc(lpObject_t o, void* cmp, uint32_t msg,
                                   wParam_t w, lParam_t l) {
    (void)o; (void)cmp; (void)msg; (void)w; (void)l;
    return 0;
}

static struct ClassDesc s_rtString2Class = {
    .ObjProc       = RTString2Comp_Proc,
    .Properties    = s_rtString2Props,
    .ClassName     = "RTString2Comp",
    .ClassSize     = sizeof(struct RTString2Comp),
    .NumProperties = 2,
    .DefaultName   = "rtstring2comp",
};

static void register_rtstring2_class(void) {
    s_rtString2Props[0].ShortIdentifier = s_rtString2Props[0].FullIdentifier = fnv1a32("Source");
    s_rtString2Props[1].ShortIdentifier = s_rtString2Props[1].FullIdentifier = fnv1a32("Target");
    s_rtString2Class.ClassID = fnv1a32("RTString2Comp");
    OBJ_RegisterClass(&s_rtString2Class);
}

static lpObject_t make_rtstring2_object(void) {
    return OBJ_Create(fnv1a32("RTString2Comp"));
}

/* ------------------------------------------------------------------ */
/* Color test component                                                */
/* Basic set/get/import/export of kDataTypeColor properties.          */
/* ------------------------------------------------------------------ */

struct ColorComp {
    struct color Tint; /* kDataTypeColor */
};

/* fnv1a32("ColorComp")      = 0x1628afcf */
/* fnv1a32("ColorComp.Tint") = 0x7a0bf43e */
/* fnv1a32("Tint")           = 0xdedc93ac */
static struct PropertyType s_colorProps[1] = {
    { .Name = "Tint", .Key = "Tint", .DataType = kDataTypeColor,
      .DataSize = sizeof(struct color),
      .ShortIdentifier = 0xdedc93ac, .FullIdentifier = 0x7a0bf43e,
      .Offset = offsetof(struct ColorComp, Tint) },
};

static LRESULT ColorComp_Proc(lpObject_t o, void* cmp, uint32_t msg,
                              wParam_t w, lParam_t l) {
    (void)o; (void)cmp; (void)msg; (void)w; (void)l;
    return 0;
}

static struct ClassDesc s_colorClass = {
    .ObjProc       = ColorComp_Proc,
    .Properties    = s_colorProps,
    .ClassName     = "ColorComp",
    .ClassID       = 0x1628afcf, /* fnv1a32("ColorComp") */
    .ClassSize     = sizeof(struct ColorComp),
    .NumProperties = 1,
    .DefaultName   = "colorcomp",
};

static void register_color_class(void) {
    OBJ_RegisterClass(&s_colorClass);
}

static lpObject_t make_color_object(void) {
    return OBJ_Create(fnv1a32("ColorComp"));
}

/* ------------------------------------------------------------------ */
/* Runtime color component                                             */
/* Used for binding tests: Color (source) + Alpha (float sink).       */
/* FullIdentifiers are plain fnv1a32(name) so {./Color}.COLORR works. */
/* ------------------------------------------------------------------ */

struct RTColorComp {
    struct color Color; /* kDataTypeColor — binding source */
    float        Alpha; /* kDataTypeFloat — float sink for channel binding */
};

/* fnv1a32("RTColorComp")       = 0x179ee081                          */
/* fnv1a32("RTColorComp.Color") = 0xa40d18f0 (not used for binding)  */
/* fnv1a32("RTColorComp.Alpha") = 0xe4dc94f3 (not used for binding)  */
/* fnv1a32("Color")             = 0xe5b43cf8  ← FullIdentifier        */
/* fnv1a32("Alpha")             = 0x0348724b  ← FullIdentifier        */
static struct PropertyType s_rtColorProps[2] = {
    { .Name = "Color", .Key = "Color", .DataType = kDataTypeColor,
      .DataSize = sizeof(struct color),
      .ShortIdentifier = 0xe5b43cf8, .FullIdentifier = 0xe5b43cf8,
      .Offset = offsetof(struct RTColorComp, Color) },
    { .Name = "Alpha", .Key = "Alpha", .DataType = kDataTypeFloat,
      .DataSize = sizeof(float),
      .ShortIdentifier = 0x0348724b, .FullIdentifier = 0x0348724b,
      .Offset = offsetof(struct RTColorComp, Alpha) },
};

static LRESULT RTColorComp_Proc(lpObject_t o, void* cmp, uint32_t msg,
                                wParam_t w, lParam_t l) {
    (void)o; (void)cmp; (void)msg; (void)w; (void)l;
    return 0;
}

static struct ClassDesc s_rtColorClass = {
    .ObjProc       = RTColorComp_Proc,
    .Properties    = s_rtColorProps,
    .ClassName     = "RTColorComp",
    .ClassID       = 0x179ee081, /* fnv1a32("RTColorComp") */
    .ClassSize     = sizeof(struct RTColorComp),
    .NumProperties = 2,
    .DefaultName   = "rtcolorcomp",
};

static void register_rt_color_class(void) {
    OBJ_RegisterClass(&s_rtColorClass);
}

static lpObject_t make_rt_color_object(void) {
    return OBJ_Create(fnv1a32("RTColorComp"));
}

/* ------------------------------------------------------------------ */
/* Object lifecycle helper                                            */
/* ------------------------------------------------------------------ */

/* Allocate a bare TestComp object without a Lua state */
static lpObject_t make_object(void) {
    return OBJ_Create(fnv1a32("TestComp"));
}

/*
 * Release an object created with make_object().
 * Frees all properties (including heap-allocated strings) via
 * OBJ_ReleaseProperties, frees all component blocks, and finally
 * frees the object itself.
 * Does NOT call OBJ_Release (which needs a Lua state and message dispatch).
 */
static void destroy_object(lpObject_t obj) {
    if (!obj) return;
    OBJ_ReleaseProperties(obj);
    OBJ_ReleaseComponents(obj);
    free(obj->Name);
    free(obj);
}

/* ------------------------------------------------------------------ */
/* Tests                                                               */
/* ------------------------------------------------------------------ */

static void test_int_property(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Count", &prop));
        EXPECT(PROP_GetType(prop) == kDataTypeInt);
        EXPECT(PROP_IsNull(prop));

        int val = 42;
        PROP_SetValue(prop, &val);
        EXPECT(!PROP_IsNull(prop));
        EXPECT(*(int*)PROP_GetValue(prop) == 42);

        int val2 = -7;
        PROP_SetValue(prop, &val2);
        EXPECT(*(int*)PROP_GetValue(prop) == -7);
    }
}

static void test_float_property(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        PROP_TEST(obj, "Value", kDataTypeFloat, &(float){3.14f},
                  *(float*)PROP_GetValue(_p) == 3.14f);
    }
}

static void test_bool_property(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Active", &prop));
        EXPECT(PROP_GetType(prop) == kDataTypeBool);

        bool_t yes = TRUE;
        PROP_SetValue(prop, &yes);
        EXPECT(*(bool_t*)PROP_GetValue(prop) == TRUE);

        bool_t no = FALSE;
        PROP_SetValue(prop, &no);
        EXPECT(*(bool_t*)PROP_GetValue(prop) == FALSE);
    }
}

static void test_string_property_basic(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        PROP_TEST(obj, "Label", kDataTypeString, PROP_STR("hello"),
                  !strcmp(*(const char**)PROP_GetValue(_p), "hello"));
    }
}

static void test_string_property_reassign(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));

        PROP_SetValue(prop, PROP_STR("first"));
        EXPECT_STR_EQ(*(const char**)PROP_GetValue(prop), "first");

        PROP_SetValue(prop, PROP_STR("second"));
        EXPECT_STR_EQ(*(const char**)PROP_GetValue(prop), "second");

        PROP_SetValue(prop, PROP_STR("third"));
        EXPECT_STR_EQ(*(const char**)PROP_GetValue(prop), "third");

        /* Value pointer changes each time (new strdup), but content is correct */
        PROP_SetValue(prop, PROP_STR(""));
        EXPECT_STR_EQ(*(const char**)PROP_GetValue(prop), "");
    }
}

static void test_string_property_clear(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));

        PROP_SetValue(prop, PROP_STR("some string"));
        EXPECT(!PROP_IsNull(prop));

        /* After clear the property should be null/reset */
        EXPECT(PROP_Clear(prop), PROP_IsNull(prop));

        /* Setting after clear must not double-free */
        PROP_SetValue(prop, PROP_STR("after clear"));
        EXPECT_STR_EQ(*(const char**)PROP_GetValue(prop), "after clear");
    }
}

static void test_string_property_clear_without_set(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));

        /* Clearing a never-set string property must not crash */
        EXPECT(PROP_Clear(prop), PROP_IsNull(prop));
    }
}

static void test_release_properties_frees_strings(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));
        PROP_SetValue(prop, PROP_STR("will be freed"));
        /* destroy_object (called by WITH) calls OBJ_ReleaseProperties which must
           free the strdup'd string without crashing. */
    }
}

static void test_set_property_value_api(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        /* OBJ_SetPropertyValue creates the property on demand */
        EXPECT_OK(OBJ_SetPropertyValue(obj, "Count", &(int){99}));

        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Count", &prop));
        EXPECT(*(int*)PROP_GetValue(prop) == 99);

        EXPECT_OK(OBJ_SetPropertyValue(obj, "Label", PROP_STR("via api")));
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));
        EXPECT_STR_EQ(*(const char**)PROP_GetValue(prop), "via api");
    }
}

static void test_property_state_string(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));

        /* Set the hover state value */
        PROP_SetValue(prop, PROP_STR("normal"));
        EXPECT_STR_EQ(*(const char**)PROP_GetValue(prop), "normal");

        /* Clear all states — should free heap strings without crashing */
        EXPECT(PROP_Clear(prop), PROP_IsNull(prop));

        /* Re-set after full clear */
        PROP_SetValue(prop, PROP_STR("reset"));
        EXPECT_STR_EQ(*(const char**)PROP_GetValue(prop), "reset");
    }
}

static void test_struct_property(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Position", &prop));
        EXPECT(PROP_GetType(prop) == kDataTypeStruct);

        float pos[2] = {10.0f, 20.0f};
        PROP_SetValue(prop, pos);
        float *result = (float*)PROP_GetValue(prop);
        EXPECT(result[0] == 10.0f);
        EXPECT(result[1] == 20.0f);
    }
}

static void test_find_property_unknown(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop = NULL;
        EXPECT(FAILED(OBJ_FindShortProperty(obj, "NonExistentProp", &prop)));
    }
}

static void test_multiple_properties_independent(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t pCount, pLabel;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Count", &pCount));
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &pLabel));
        EXPECT(pCount != pLabel);

        int val = 5;
        PROP_SetValue(pCount, &val);
        PROP_SetValue(pLabel, PROP_STR("hello"));

        /* Verify they don't alias each other */
        EXPECT(*(int*)PROP_GetValue(pCount) == 5);
        EXPECT_STR_EQ(*(const char**)PROP_GetValue(pLabel), "hello");

        PROP_SetValue(pLabel, PROP_STR("world"));
        EXPECT(*(int*)PROP_GetValue(pCount) == 5); /* Count unchanged */
        EXPECT_STR_EQ(*(const char**)PROP_GetValue(pLabel), "world");
    }
}

static void test_string_empty(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));

        PROP_SetValue(prop, PROP_STR(""));
        EXPECT(!PROP_IsNull(prop));
        EXPECT_STR_EQ(*(const char**)PROP_GetValue(prop), "");
    }
}

static void test_release_without_string_set(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        /* Only set a non-string property */
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Count", &prop));
        int val = 1;
        PROP_SetValue(prop, &val);
        /* destroy_object (called by WITH) calls OBJ_ReleaseProperties which must
           not crash even though the Label string property was never set */
    }
}

/* ------------------------------------------------------------------ */
/* p_runtime tests                                                     */
/* ------------------------------------------------------------------ */

static void test_runtime_token_create_int(void) {
    WITH(struct token, prog, Token_Create("42"), Token_Release) {
        EXPECT(prog != NULL);
    }
}

static void test_runtime_token_create_float(void) {
    WITH(struct token, prog, Token_Create("3.14"), Token_Release) {
        EXPECT(prog != NULL);
    }
}

static void test_runtime_token_create_string(void) {
    WITH(struct token, prog, Token_Create("\"hello\""), Token_Release) {
        EXPECT(prog != NULL);
    }
}

static void test_runtime_token_create_arithmetic(void) {
    WITH(struct token, prog, Token_Create("ADD(2, 3)"), Token_Release) {
        EXPECT(prog != NULL);
    }
}

static void test_runtime_run_int_constant(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        struct vm_register r = {0};
        RUN_PROG(obj, "7", &r);
        EXPECT(r.type == kDataTypeInt || r.type == kDataTypeFloat);
        EXPECT((int)r.value[0] == 7);
    }
}

static void test_runtime_run_float_constant(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        struct vm_register r = {0};
        RUN_PROG(obj, "2.5", &r);
        EXPECT(r.value[0] == 2.5f);
    }
}

static void test_runtime_run_string_constant(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        struct vm_register r = {0};
        RUN_PROG(obj, "\"world\"", &r);
        EXPECT(r.type == kDataTypeString);
        const char *str = *(const char *const *)r.value;
        EXPECT_STR_EQ(str, "world");
    }
}

static void test_runtime_run_arithmetic(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        struct vm_register r = {0};
        RUN_PROG(obj, "ADD(10, 5)", &r);
        EXPECT((int)r.value[0] == 15);
    }
}

static void test_runtime_import_int(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Count", &prop));
        struct vm_register r = {0};
        r.type     = kDataTypeInt;
        r.value[0] = 99.0f;
        EXPECT(PROP_Import(prop, kPropertyAttributeWholeProperty, &r));
        EXPECT(!PROP_IsNull(prop));
        EXPECT(*(int*)PROP_GetValue(prop) == 99);
    }
}

static void test_runtime_import_float(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Value", &prop));
        struct vm_register r = {0};
        r.type     = kDataTypeFloat;
        r.size     = sizeof(float);
        r.value[0] = 1.5f;
        EXPECT(PROP_Import(prop, kPropertyAttributeWholeProperty, &r));
        EXPECT(*(float*)PROP_GetValue(prop) == 1.5f);
    }
}

static void test_runtime_import_string(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));
        struct vm_register r = {0};
        r.type = kDataTypeString;
        r.size = sizeof(const char*);
        const char *s = "imported";
        memcpy(r.value, &s, sizeof(s));
        EXPECT(PROP_Import(prop, kPropertyAttributeWholeProperty, &r));
        EXPECT_STR_EQ(*(const char**)PROP_GetValue(prop), "imported");
    }
}

static void test_runtime_attach_and_update_int(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Count", &prop));
        struct token *prog = Token_Create("21");
        EXPECT(prog != NULL);
        PROP_AttachProgram(prop, kPropertyAttributeWholeProperty, prog, "21");
        EXPECT(PROP_HasProgram(prop));
        core.frame++;
        EXPECT(PROP_Update(prop));
        EXPECT(!PROP_IsNull(prop));
        EXPECT(*(int*)PROP_GetValue(prop) == 21);
    }
}

static void test_runtime_attach_and_update_string(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));
        struct token *prog = Token_Create("\"bound\"");
        EXPECT(prog != NULL);
        PROP_AttachProgram(prop, kPropertyAttributeWholeProperty, prog, "\"bound\"");
        EXPECT(PROP_HasProgram(prop));
        core.frame++;
        EXPECT(PROP_Update(prop));
        EXPECT(!PROP_IsNull(prop));
        EXPECT_STR_EQ(*(const char**)PROP_GetValue(prop), "bound");
    }
}

/*
 * Property-reference by name: the expression "{./Value}" references the
 * property whose FullIdentifier == fnv1a32("Value") on the same object.
 * RTComp properties are registered with exactly those identifiers.
 *
 * Flow: set Value=5.0, attach program "{./Value}" to Count, call PROP_Update —
 * Count should be set to 5 (via int conversion of the float).
 */
static void test_runtime_property_reference(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        lpProperty_t propCount, propValue;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Count", &propCount));
        EXPECT_OK(OBJ_FindShortProperty(obj, "Value", &propValue));

        float v = 5.0f;
        PROP_SetValue(propValue, &v);
        EXPECT(!PROP_IsNull(propValue));

        struct token *prog = Token_Create("{./Value}");
        EXPECT(prog != NULL);
        PROP_AttachProgram(propCount, kPropertyAttributeWholeProperty, prog, "{./Value}");
        EXPECT(PROP_HasProgram(propCount));
        core.frame++;
        EXPECT(PROP_Update(propCount));
        EXPECT(!PROP_IsNull(propCount));
        EXPECT(*(int*)PROP_GetValue(propCount) == 5);
    }
}

/*
 * String concatenation: ADD("foo", "bar") should produce "foobar"
 * and a program that writes it to a string property should work.
 */
static void test_runtime_string_concat_program(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));
        struct token *prog = Token_Create("ADD(\"foo\", \"bar\")");
        EXPECT(prog != NULL);
        PROP_AttachProgram(prop, kPropertyAttributeWholeProperty,
                           prog, "ADD(\"foo\", \"bar\")");
        core.frame++;
        EXPECT(PROP_Update(prop));
        EXPECT_STR_EQ(*(const char**)PROP_GetValue(prop), "foobar");
    }
}

/*
 * IF operator: IF(cond, true-value, false-value) works like Excel IF.
 * When condition is non-zero, returns true-value; otherwise false-value.
 */
static void test_runtime_if_true_branch(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        struct vm_register r = {0};
        RUN_PROG(obj, "IF(1, 10, 20)", &r);
        EXPECT((int)r.value[0] == 10);
    }
}

static void test_runtime_if_false_branch(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        struct vm_register r = {0};
        RUN_PROG(obj, "IF(0, 10, 20)", &r);
        EXPECT((int)r.value[0] == 20);
    }
}

static void test_runtime_if_string_branch(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        struct vm_register r = {0};
        RUN_PROG(obj, "IF(0, \"yes\", \"no\")", &r);
        EXPECT(r.type == kDataTypeString);
        const char *str = *(const char *const *)r.value;
        EXPECT_STR_EQ(str, "no");
    }
}

/* ------------------------------------------------------------------ */
/* String property binding tests                                       */
/*                                                                     */
/* These cover the scenario from the problem report:                   */
/*   *(char**)PROP_GetValue(p)  -- outputs (null)                      */
/*                                                                     */
/* A never-set string component property holds NULL in its char*       */
/* field.  PROP_Export must convert that NULL to an empty string ""    */
/* (via vm_strtmp), not crash or propagate NULL to the target.        */
/* ------------------------------------------------------------------ */

/*
 * Running a binding expression {./Label} on an object whose Label
 * string property was never set must produce an empty-string register,
 * not a NULL pointer or a crash.
 */
static void test_string_export_unset_produces_empty(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        /* Label was never set — the RTComp.Label char* field is NULL. */
        struct vm_register r = {0};
        RUN_PROG(obj, "{./Label}", &r);
        EXPECT(r.type == kDataTypeString);
        const char *str = *(const char *const *)r.value;
        EXPECT(str != NULL);        /* vm_strtmp(NULL) returns "" not NULL */
        EXPECT_STR_EQ(str, "");
    }
}

/*
 * PROP_AttachProgram + PROP_Update with an unset string source:
 * the target string property must be set to "" after the update,
 * never left as NULL.
 */
static void test_string_binding_unset_source_gives_empty(void) {
    WITH(struct Object, obj, make_rtstring2_object(), destroy_object) {
        lpProperty_t target;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Target", &target));
        /* Source is never set — its char* is NULL. */
        struct token *prog = Token_Create("{./Source}");
        EXPECT(prog != NULL);
        PROP_AttachProgram(target, kPropertyAttributeWholeProperty,
                           prog, "{./Source}");
        EXPECT(PROP_HasProgram(target));
        core.frame++;
        EXPECT(PROP_Update(target));
        EXPECT(!PROP_IsNull(target));
        EXPECT_STR_EQ(*(const char**)PROP_GetValue(target), "");
    }
}

/*
 * When the source string is set, PROP_Update must propagate its value
 * to the target property.
 */
static void test_string_binding_value_propagates(void) {
    WITH(struct Object, obj, make_rtstring2_object(), destroy_object) {
        lpProperty_t source, target;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Source", &source));
        EXPECT_OK(OBJ_FindShortProperty(obj, "Target", &target));

        PROP_SetValue(source, PROP_STR("hello"));
        EXPECT_STR_EQ(*(const char**)PROP_GetValue(source), "hello");

        struct token *prog = Token_Create("{./Source}");
        EXPECT(prog != NULL);
        PROP_AttachProgram(target, kPropertyAttributeWholeProperty,
                           prog, "{./Source}");
        core.frame++;
        EXPECT(PROP_Update(target));
        EXPECT(!PROP_IsNull(target));
        EXPECT_STR_EQ(*(const char**)PROP_GetValue(target), "hello");
    }
}

/*
 * Parent-child binding path {../Label}: OBJ_RunProgram resolves ".."
 * to the parent object and reads its string property.
 */
static void test_string_binding_parent_label(void) {
    WITH(struct Object, parent, make_rt_object(), destroy_object) {
        WITH(struct Object, child, make_rt_object(), destroy_object) {
            child->parent = parent;

            lpProperty_t parentLabel;
            EXPECT_OK(OBJ_FindShortProperty(parent, "Label", &parentLabel));
            PROP_SetValue(parentLabel, PROP_STR("world"));

            struct vm_register r = {0};
            RUN_PROG(child, "{../Label}", &r);
            EXPECT(r.type == kDataTypeString);
            EXPECT_STR_EQ(*(const char *const *)r.value, "world");

            child->parent = NULL;
        }
    }
}

/*
 * Parent-child binding when the parent's string property is unset:
 * the binding must produce "" not NULL or a crash.
 */
static void test_string_binding_parent_label_unset(void) {
    WITH(struct Object, parent, make_rt_object(), destroy_object) {
        WITH(struct Object, child, make_rt_object(), destroy_object) {
            child->parent = parent;

            /* Parent's Label is never set — char* is NULL. */
            struct vm_register r = {0};
            RUN_PROG(child, "{../Label}", &r);
            EXPECT(r.type == kDataTypeString);
            const char *str = *(const char *const *)r.value;
            EXPECT(str != NULL);
            EXPECT_STR_EQ(str, "");

            child->parent = NULL;
        }
    }
}

/* ------------------------------------------------------------------ */
/* kDataTypeColor — General property tests                             */
/* ------------------------------------------------------------------ */

/* Basic: DataType is kDataTypeColor, IsNull before set, set/get. */
static void test_color_property_basic(void) {
    WITH(struct Object, obj, make_color_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Tint", &prop));
        EXPECT(PROP_GetType(prop) == kDataTypeColor);
        EXPECT(PROP_GetSize(prop) == sizeof(struct color));
        EXPECT(PROP_IsNull(prop));

        struct color c = { .r = 1.0f, .g = 0.5f, .b = 0.25f, .a = 1.0f };
        PROP_SetValue(prop, &c);
        EXPECT(!PROP_IsNull(prop));

        struct color *got = (struct color *)PROP_GetValue(prop);
        EXPECT(got->r == 1.0f);
        EXPECT(got->g == 0.5f);
        EXPECT(got->b == 0.25f);
        EXPECT(got->a == 1.0f);
    }
}

/* Reassignment: setting a color property twice should update in place. */
static void test_color_property_reassign(void) {
    WITH(struct Object, obj, make_color_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Tint", &prop));

        struct color c1 = { .r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f };
        PROP_SetValue(prop, &c1);
        EXPECT(((struct color *)PROP_GetValue(prop))->r == 1.0f);

        struct color c2 = { .r = 0.0f, .g = 0.0f, .b = 1.0f, .a = 0.5f };
        PROP_SetValue(prop, &c2);
        struct color *got = (struct color *)PROP_GetValue(prop);
        EXPECT(got->r == 0.0f);
        EXPECT(got->b == 1.0f);
        EXPECT(got->a == 0.5f);
    }
}

/* Clear: PROP_Clear resets a color property back to null. */
static void test_color_property_clear(void) {
    WITH(struct Object, obj, make_color_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Tint", &prop));

        struct color c = { .r = 0.2f, .g = 0.4f, .b = 0.6f, .a = 0.8f };
        PROP_SetValue(prop, &c);
        EXPECT(!PROP_IsNull(prop));

        PROP_Clear(prop);
        EXPECT(PROP_IsNull(prop));

        /* Re-set after clear must work without corruption. */
        PROP_SetValue(prop, &c);
        EXPECT(!PROP_IsNull(prop));
        EXPECT(((struct color *)PROP_GetValue(prop))->g == 0.4f);
    }
}

/* Parsing: COLOR_Parse converts a hex color string to struct color. */
static void test_color_parse_hex_rgb(void) {
    for (int _pass = 1; _pass; _pass = 0) {
        struct color c = COLOR_Parse("#ff8000");
        EXPECT(c.r == 1.0f);
        EXPECT(c.g > 0.49f && c.g < 0.51f); /* 0x80/0xff ≈ 0.502 */
        EXPECT(c.b == 0.0f);
        EXPECT(c.a == 1.0f); /* default alpha */
    }
}

static void test_color_parse_hex_rgba(void) {
    for (int _pass = 1; _pass; _pass = 0) {
        struct color c = COLOR_Parse("#ff000080"); /* red, half opacity */
        EXPECT(c.r == 1.0f);
        EXPECT(c.g == 0.0f);
        EXPECT(c.b == 0.0f);
        EXPECT(c.a > 0.49f && c.a < 0.51f); /* 0x80/0xff ≈ 0.502 */
    }
}

/* Parsing: set a color property from a parsed string. */
static void test_color_property_from_parse(void) {
    WITH(struct Object, obj, make_color_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Tint", &prop));

        struct color c = COLOR_Parse("#00ff00"); /* green */
        PROP_SetValue(prop, &c);
        EXPECT(!PROP_IsNull(prop));

        struct color *got = (struct color *)PROP_GetValue(prop);
        EXPECT(got->r == 0.0f);
        EXPECT(got->g == 1.0f);
        EXPECT(got->b == 0.0f);
        EXPECT(got->a == 1.0f);
    }
}

/* ------------------------------------------------------------------ */
/* kDataTypeColor — PROP_Import / PROP_Export (channel bindings)      */
/* ------------------------------------------------------------------ */

/* Import whole color from a float-typed vm_register (4 floats). */
static void test_color_import_whole(void) {
    WITH(struct Object, obj, make_rt_color_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Color", &prop));

        struct vm_register r = {0};
        r.type     = kDataTypeFloat;
        r.size     = sizeof(struct color);
        r.value[0] = 0.1f; /* r */
        r.value[1] = 0.2f; /* g */
        r.value[2] = 0.3f; /* b */
        r.value[3] = 0.9f; /* a */
        EXPECT(PROP_Import(prop, kPropertyAttributeWholeProperty, &r));
        EXPECT(!PROP_IsNull(prop));

        struct color *got = (struct color *)PROP_GetValue(prop);
        EXPECT(got->r == 0.1f);
        EXPECT(got->g == 0.2f);
        EXPECT(got->b == 0.3f);
        EXPECT(got->a == 0.9f);
    }
}

/* Import individual RGBA channels via PROP_Import with attribute. */
static void test_color_import_channels(void) {
    WITH(struct Object, obj, make_rt_color_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Color", &prop));

        /* Initialize the color to known values first. */
        struct color init = { .r = 0.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f };
        PROP_SetValue(prop, &init);

        struct vm_register r = {0};
        r.type = kDataTypeFloat;
        r.size = sizeof(float);

        r.value[0] = 0.8f;
        EXPECT(PROP_Import(prop, kPropertyAttributeColorR, &r));
        EXPECT(((struct color *)PROP_GetValue(prop))->r == 0.8f);

        r.value[0] = 0.6f;
        EXPECT(PROP_Import(prop, kPropertyAttributeColorG, &r));
        EXPECT(((struct color *)PROP_GetValue(prop))->g == 0.6f);

        r.value[0] = 0.4f;
        EXPECT(PROP_Import(prop, kPropertyAttributeColorB, &r));
        EXPECT(((struct color *)PROP_GetValue(prop))->b == 0.4f);

        r.value[0] = 0.5f;
        EXPECT(PROP_Import(prop, kPropertyAttributeColorA, &r));
        EXPECT(((struct color *)PROP_GetValue(prop))->a == 0.5f);

        /* Ensure other channels were not clobbered. */
        struct color *got = (struct color *)PROP_GetValue(prop);
        EXPECT(got->r == 0.8f);
        EXPECT(got->g == 0.6f);
        EXPECT(got->b == 0.4f);
        EXPECT(got->a == 0.5f);
    }
}

/* Export a color channel via OBJ_RunProgram with {./Color}.COLORR. */
static void test_color_export_channel_program(void) {
    WITH(struct Object, obj, make_rt_color_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Color", &prop));

        struct color c = { .r = 0.75f, .g = 0.25f, .b = 0.5f, .a = 1.0f };
        PROP_SetValue(prop, &c);

        /* {./Color}.COLORR reads the R channel. */
        struct vm_register r = {0};
        RUN_PROG(obj, "{./Color}.COLORR", &r);
        EXPECT(r.type == kDataTypeFloat);
        EXPECT(r.value[0] == 0.75f);

        /* {./Color}.COLORG reads the G channel. */
        r = (struct vm_register){0};
        RUN_PROG(obj, "{./Color}.COLORG", &r);
        EXPECT(r.value[0] == 0.25f);
    }
}

/* COLOR4 function: COLOR4(r, g, b, a) produces a float[4] register. */
static void test_color_color4_function(void) {
    WITH(struct Object, obj, make_rt_color_object(), destroy_object) {
        struct vm_register r = {0};
        RUN_PROG(obj, "COLOR4(1.0, 0.5, 0.25, 0.75)", &r);
        EXPECT(r.size == sizeof(struct color));
        EXPECT(r.value[0] == 1.0f);
        EXPECT(r.value[1] == 0.5f);
        EXPECT(r.value[2] == 0.25f);
        EXPECT(r.value[3] == 0.75f);
    }
}

/* Bind COLOR4 to a color property via PROP_AttachProgram + PROP_Update. */
static void test_color_bind_color4_to_property(void) {
    WITH(struct Object, obj, make_rt_color_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Color", &prop));

        struct token *prog = Token_Create("COLOR4(0.1, 0.2, 0.3, 1.0)");
        EXPECT(prog != NULL);
        PROP_AttachProgram(prop, kPropertyAttributeWholeProperty,
                           prog, "COLOR4(0.1, 0.2, 0.3, 1.0)");
        EXPECT(PROP_HasProgram(prop));
        core.frame++;
        EXPECT(PROP_Update(prop));
        EXPECT(!PROP_IsNull(prop));

        struct color *got = (struct color *)PROP_GetValue(prop);
        EXPECT(got->r == 0.1f);
        EXPECT(got->g == 0.2f);
        EXPECT(got->b == 0.3f);
        EXPECT(got->a == 1.0f);
    }
}

/* Bind a color channel to a float property: {./Color}.COLORR → Alpha. */
static void test_color_bind_channel_to_float(void) {
    WITH(struct Object, obj, make_rt_color_object(), destroy_object) {
        /* Set source color. */
        lpProperty_t colorProp;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Color", &colorProp));
        struct color src = { .r = 0.9f, .g = 0.3f, .b = 0.0f, .a = 1.0f };
        PROP_SetValue(colorProp, &src);

        /* Bind {./Color}.COLORA to Alpha float property. */
        lpProperty_t alphaProp;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Alpha", &alphaProp));
        struct token *prog = Token_Create("{./Color}.COLORA");
        EXPECT(prog != NULL);
        PROP_AttachProgram(alphaProp, kPropertyAttributeWholeProperty,
                           prog, "{./Color}.COLORA");
        core.frame++;
        EXPECT(PROP_Update(alphaProp));
        EXPECT(!PROP_IsNull(alphaProp));
        /* Alpha channel of src is 1.0; float property should be 1.0. */
        EXPECT(*(float *)PROP_GetValue(alphaProp) == 1.0f);
    }
}

/* Bind a color channel write: import from {./Color}.COLORR into colorProp.COLORR. */
static void test_color_bind_channel_to_channel(void) {
    WITH(struct Object, obj, make_rt_color_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Color", &prop));

        /* Initialize to a known color. */
        struct color c = { .r = 0.3f, .g = 0.7f, .b = 0.5f, .a = 1.0f };
        PROP_SetValue(prop, &c);

        /* Attach {./Color}.COLORG to the R channel of the same property.
         * After update, R should equal original G (0.7). */
        struct token *prog = Token_Create("{./Color}.COLORG");
        EXPECT(prog != NULL);
        PROP_AttachProgram(prop, kPropertyAttributeColorR,
                           prog, "{./Color}.COLORG");
        core.frame++;
        EXPECT(PROP_Update(prop));

        struct color *got = (struct color *)PROP_GetValue(prop);
        EXPECT(got->r == 0.7f); /* was 0.3, now updated from G=0.7 */
        EXPECT(got->g == 0.7f); /* unchanged */
    }
}

/* ------------------------------------------------------------------ */
/* Project-defined (dynamic) property type tests                       */
/* These simulate property types registered from package.xml via       */
/* OBJ_RegisterPropertyType, as _InitPropertyTypes() does.             */
/* ------------------------------------------------------------------ */

/*
 * A project-defined string property type (e.g. from package.xml
 * <PropertyType Name="Title" DataType="String" Category="Card"/>)
 * must have DataSize == sizeof(char*) so that the heap pointer fits
 * in the property's storage buffer.  Using DataSize=4 on 64-bit
 * corrupts the pointer and causes crashes in jwPropertyExport.
 */
static struct PropertyType s_projStringPropType = {
    .Name            = "ProjTitle",
    .Category        = "Card",
    .DataType        = kDataTypeString,
    .DataSize        = sizeof(char *),   /* must be pointer-sized, not 4 */
    /* identifiers computed from "Card.ProjTitle" */
};

static void register_project_types(void) {
    s_projStringPropType.ShortIdentifier = fnv1a32("ProjTitle");
    s_projStringPropType.FullIdentifier  = fnv1a32("Card.ProjTitle");
    OBJ_RegisterPropertyType(&s_projStringPropType);
}

static void test_project_string_property_set_get(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        /*
         * Project properties are looked up by full name ("Category.Name"),
         * which is used as the FullIdentifier.  OBJ_FindShortProperty first
         * checks component properties (by ShortIdentifier), then falls back
         * to OBJ_FindLongProperty which calls _CreateProjectProperty with the
         * same identifier — so we must pass the full "Card.ProjTitle" name.
         */
        EXPECT_OK(OBJ_FindShortProperty(obj, "Card.ProjTitle", &prop));
        EXPECT(PROP_GetType(prop) == kDataTypeString);
        EXPECT(PROP_IsNull(prop));

        PROP_SetValue(prop, PROP_STR("Hello"));
        EXPECT(!PROP_IsNull(prop));
        EXPECT_STR_EQ(*(const char**)PROP_GetValue(prop), "Hello");

        PROP_SetValue(prop, PROP_STR("World"));
        EXPECT_STR_EQ(*(const char**)PROP_GetValue(prop), "World");
    }
}

/* ------------------------------------------------------------------ */

#define DECL_TEST(func) { #func, func }

int main(void) {
    mem_init(); /* must be first: resolves real malloc/free via RTLD_NEXT */

    printf("ORCA property system tests\n");
    printf("==========================\n");

    register_test_class();
    register_runtime_class();
    register_rtstring2_class();
    register_color_class();
    register_rt_color_class();
    register_project_types();

    /*
     * Table-driven test runner.  Each entry is a { "display name", fn } pair.
     * To add a test: define the function above and add one line here.
     *
     * When compiled with -DTEST_MEMORY, mem_tracker.h intercepts malloc/free
     * so MEM_CHECK_LEAK catches any allocation the functional test leaks —
     * there is no need for a separate set of "memleak" test functions.
     */
    static const struct { const char *name; void (*fn)(void); } tests[] = {
        DECL_TEST(test_int_property),
        DECL_TEST(test_float_property),
        DECL_TEST(test_bool_property),
        DECL_TEST(test_string_property_basic),
        DECL_TEST(test_string_property_reassign),
        DECL_TEST(test_string_property_clear),
        DECL_TEST(test_string_property_clear_without_set),
        DECL_TEST(test_release_properties_frees_strings),
        DECL_TEST(test_set_property_value_api),
        DECL_TEST(test_property_state_string),
        DECL_TEST(test_struct_property),
        DECL_TEST(test_find_property_unknown),
        DECL_TEST(test_multiple_properties_independent),
        DECL_TEST(test_string_empty),
        DECL_TEST(test_release_without_string_set),
        DECL_TEST(test_runtime_token_create_int),
        DECL_TEST(test_runtime_token_create_float),
        DECL_TEST(test_runtime_token_create_string),
        DECL_TEST(test_runtime_token_create_arithmetic),
        DECL_TEST(test_runtime_run_int_constant),
        DECL_TEST(test_runtime_run_float_constant),
        DECL_TEST(test_runtime_run_string_constant),
        DECL_TEST(test_runtime_run_arithmetic),
        DECL_TEST(test_runtime_import_int),
        DECL_TEST(test_runtime_import_float),
        DECL_TEST(test_runtime_import_string),
        DECL_TEST(test_runtime_attach_and_update_int),
        DECL_TEST(test_runtime_attach_and_update_string),
        DECL_TEST(test_runtime_property_reference),
        DECL_TEST(test_runtime_string_concat_program),
        DECL_TEST(test_runtime_if_true_branch),
        DECL_TEST(test_runtime_if_false_branch),
        DECL_TEST(test_runtime_if_string_branch),
        DECL_TEST(test_string_export_unset_produces_empty),
        DECL_TEST(test_string_binding_unset_source_gives_empty),
        DECL_TEST(test_string_binding_value_propagates),
        DECL_TEST(test_string_binding_parent_label),
        DECL_TEST(test_string_binding_parent_label_unset),
        DECL_TEST(test_project_string_property_set_get),
        DECL_TEST(test_color_property_basic),
        DECL_TEST(test_color_property_reassign),
        DECL_TEST(test_color_property_clear),
        DECL_TEST(test_color_parse_hex_rgb),
        DECL_TEST(test_color_parse_hex_rgba),
        DECL_TEST(test_color_property_from_parse),
        DECL_TEST(test_color_import_whole),
        DECL_TEST(test_color_import_channels),
        DECL_TEST(test_color_export_channel_program),
        DECL_TEST(test_color_color4_function),
        DECL_TEST(test_color_bind_color4_to_property),
        DECL_TEST(test_color_bind_channel_to_float),
        DECL_TEST(test_color_bind_channel_to_channel),
    };

    for (size_t i = 0; i < sizeof(tests)/sizeof(*tests); i++) {
        long snap = MEM_SNAPSHOT();
        s_current_test = tests[i].name;
        s_tests_run++;
        printf("Running %s...\n", tests[i].name);
        tests[i].fn();
        MEM_CHECK_LEAK(snap, tests[i].name);
    }

#if TEST_MEMORY
    printf("\nMemory tracking: %ld total allocation(s) made, "
           "%ld outstanding at exit\n",
           s_alloc_total, s_alloc_count);
#endif

    printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
    return s_tests_failed == 0 ? 0 : 1;
}
