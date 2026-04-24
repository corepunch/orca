/*
 * test_styles.c — C unit tests for the ORCA style system (StyleController).
 *
 * Tests cover:
 *   - StyleController attachment and class parsing
 *   - OBJ_AddClass: runtime class addition via AddClass/AddClasses messages
 *   - OBJ_GetStyleFlags: returns 0 when no pseudo-state is active
 *   - StyleController.ThemeChanged (via _SendMessage): applies a float rule to a property
 *     using native StyleRule/StyleSheet objects
 *   - Memory leak checks for all operations (with -DTEST_MEMORY)
 *
 * Compiled via the `test-styles` Makefile target (depends on `buildlib`).
 */

#include "mem_tracker.h"

#include <source/core/core_local.h>
#include <source/core/core.h>
#include <source/core/core_properties.h>
#include <source/core/object/object_internal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 * Engine-internal functions not declared in the public header but exported
 * from the shared library (no -fvisibility=hidden in the build).
 */
extern lpObject_t OBJ_Create(uint32_t class_id);
extern void OBJ_ReleaseProperties(lpObject_t);
extern void OBJ_ReleaseComponents(lpObject_t);

/* ------------------------------------------------------------------ */
/* Minimal test harness                                                */
/* ------------------------------------------------------------------ */

static int s_tests_run    = 0;
static int s_tests_failed = 0;
static const char* s_current_test = NULL;

#define EXPECT(...)                                                         \
    if (!(__VA_ARGS__)) {                                                   \
        fprintf(stderr, "  FAIL [%s]: %s (line %d)\n",                    \
                s_current_test, #__VA_ARGS__, __LINE__);                   \
        s_tests_failed++;                                                   \
        break;                                                              \
    }

#define EXPECT_OK(hr) EXPECT((hr) == NOERROR)

/* ------------------------------------------------------------------ */
/* Test class: a minimal class whose parent is StyleController         */
/* ------------------------------------------------------------------ */

/*
 * TestStyledComp: a trivially small component that inherits StyleController
 * so GetStyleController() returns a valid pointer.  Has one float field
 * "Width" to test property-based style application.
 */
struct TestStyledComp {
    float Width;
};

static LRESULT TestStyledCompProc(struct Object* o, void* c,
                                   uint32_t msg, wParam_t wp, lParam_t lp)
{
    (void)o; (void)c; (void)msg; (void)wp; (void)lp;
    return FALSE;
}

/*
 * ID chosen to avoid collision with FNV1a32 hashes of existing class names.
 * The value 0xaabb1122 is not produced by fnv1a32 for any registered ORCA
 * class name and is used solely within this test binary.
 */
#define ID_TestStyledComp 0xaabb1122u
/* Property descriptor for "Width" (fnv1a32("Width") = 0x3b42dfbf) */
static struct PropertyType const TestStyledCompProperties[] = {
    {
        .ShortIdentifier = 0x3b42dfbfu, /* fnv1a32("Width") */
        .FullIdentifier  = 0xbfbb7a5bu, /* fnv1a32("TestStyledComp.Width") */
        .Name            = "Width",
        .DataType        = kDataTypeFloat,
        .DataSize        = sizeof(float),
        .Offset          = offsetof(struct TestStyledComp, Width),
    },
};
#define kTestStyledCompNumProperties 1

static struct TestStyledComp TestStyledCompDefaults = { .Width = 0.f };

/* _StyleController is exported from liborca (core_export.c).          */
extern struct ClassDesc _StyleController;
extern struct ClassDesc _StyleSheet;
extern struct ClassDesc _StyleRule;

static struct ClassDesc s_testStyledClass = {
    .ClassName       = "TestStyledComp",
    .DefaultName     = "TestStyledComp",
    .ClassID         = ID_TestStyledComp,
    .ClassSize       = sizeof(struct TestStyledComp),
    .Properties      = TestStyledCompProperties,
    .ObjProc         = TestStyledCompProc,
    .Defaults        = &TestStyledCompDefaults,
    .NumProperties   = kTestStyledCompNumProperties,
    .ParentClasses   = { ID_StyleController, 0 },
};

static void register_styled_class(void) {
    OBJ_RegisterClass(&_StyleSheet);
    OBJ_RegisterClass(&_StyleRule);
    OBJ_RegisterClass(&_StyleController);
    OBJ_RegisterClass(&s_testStyledClass);
}

/* ------------------------------------------------------------------ */
/* Object lifecycle helper                                             */
/* ------------------------------------------------------------------ */

static lpObject_t make_styled_object(void) {
    return OBJ_Create(ID_TestStyledComp);
}

/*
 * Frees all properties, all component data, then the object itself.
 * Does NOT call OBJ_Release (which needs a Lua state for Lua-owned objects).
 * We dispatch Object.Release manually so attach-only components
 * (StyleController) can clean up before the component memory is freed.
 */
static void destroy_object(lpObject_t obj) {
    if (!obj) return;
    /* Dispatch Object.Release → StyleController.Release frees classes + sheet */
    OBJ_SendMessageW(obj, ID_Object_Release, 0, NULL);
    OBJ_ReleaseComponents(obj);
    OBJ_ReleaseProperties(obj);
    free(obj->Name);
    free(obj);
}

/*
 * Add a float property-override rule to obj's per-object StyleSheet.
 * className: the base class name (without leading '.'), e.g. "btn".
 * propFullID: FullIdentifier hash of the target property (e.g. fnv1a32("TestStyledComp.Width")).
 * value: float value to set for the override.
 */
static void _test_add_float_rule(lpObject_t obj, lpcString_t className,
                                  uint32_t propFullID, float value)
{
    struct StyleController* sc = GetStyleController(obj);
    if (!sc) return;
    if (!sc->StyleSheet) {
        lpObject_t s = OBJ_Create(ID_StyleSheet);
        sc->StyleSheet = GetStyleSheet(s);
        sc->owned_sheet = TRUE;
    }
    lpObject_t sheetObj = CMP_GetObject(sc->StyleSheet);

    lpObject_t ruleObj = OBJ_Create(ID_StyleRule);
    struct StyleRule* sr = GetStyleRule(ruleObj);
    sr->class_id = fnv1a32(className);

    /* Set ClassName property on the rule */
    lpProperty_t clsProp = NULL;
    if (SUCCEEDED(OBJ_FindLongProperty(ruleObj, ID_StyleRule_ClassName, &clsProp))) {
        PROP_SetStringValue(clsProp, className);
    }

    /* Find the property type by FullIdentifier and create an override */
    lpcPropertyType_t pt = OBJ_FindPropertyType(propFullID);
    if (pt) {
        lpProperty_t rp = PROP_Create(NULL, ruleObj, pt);
        PROP_SetValue(rp, &value);
    }

    OBJ_AddChild(sheetObj, ruleObj, FALSE);
}

/* ------------------------------------------------------------------ */
/* Tests                                                               */
/* ------------------------------------------------------------------ */

/* GetStyleController returns non-NULL after component instantiation */
static void test_stylecontroller_attached(void) {
    WITH(struct Object, obj, make_styled_object(), destroy_object) {
        EXPECT(GetStyleController(obj) != NULL);
    }
}

/* Parsing a simple class name stores it (verified via OF_HOVERABLE flag) */
static void test_parse_class_hover_flag(void) {
    WITH(struct Object, obj, make_styled_object(), destroy_object) {
        /* Before parsing: no HOVERABLE flag */
        EXPECT(!(OBJ_GetFlags(obj) & OF_HOVERABLE));
        _SendMessage(obj, StyleController, AddClasses, "btn:hover");
        /* After parsing: the :hover pseudo-class sets OF_HOVERABLE */
        EXPECT(OBJ_GetFlags(obj) & OF_HOVERABLE);
    }
}

/* Parsing multiple space-separated classes works */
static void test_parse_multiple_classes(void) {
    WITH(struct Object, obj, make_styled_object(), destroy_object) {
        /* Only the class with :hover should flip OF_HOVERABLE */
        _SendMessage(obj, StyleController, AddClasses, "base secondary:hover extra");
        EXPECT(OBJ_GetFlags(obj) & OF_HOVERABLE);
    }
}

/* Parsing a class with no pseudo-state does not set OF_HOVERABLE */
static void test_parse_class_no_hover(void) {
    WITH(struct Object, obj, make_styled_object(), destroy_object) {
        _SendMessage(obj, StyleController, AddClasses, "primary");
        EXPECT(!(OBJ_GetFlags(obj) & OF_HOVERABLE));
    }
}

/* OBJ_AddClass: adding a :hover class at runtime sets OF_HOVERABLE */
static void test_add_class_hover(void) {
    WITH(struct Object, obj, make_styled_object(), destroy_object) {
        EXPECT(!(OBJ_GetFlags(obj) & OF_HOVERABLE));
        _SendMessage(obj, StyleController, AddClass, "card:hover");
        EXPECT(OBJ_GetFlags(obj) & OF_HOVERABLE);
    }
}

/* _test_add_float_rule is null-safe when StyleController is absent */
static void test_add_style_rule_no_component(void) {
    do {
    /*
     * Build a bare object with no StyleController. _test_add_float_rule
     * checks GetStyleController(obj) and must not crash when it's NULL.
     */
    lpObject_t obj = calloc(1, sizeof(struct Object));
    EXPECT(obj != NULL);
    /* Must not crash — GetStyleController returns NULL for bare objects */
    _test_add_float_rule(obj, "btn", 0xbfbb7a5bu /* TestStyledComp.Width */, 100.f);
    free(obj);
    } while(0);
}

/* After Object.Release dispatch, StyleController cleans up class list and stylesheet */
static void test_release_clears_classes_and_sheet(void) {
    WITH(struct Object, obj, make_styled_object(), destroy_object) {
        _SendMessage(obj, StyleController, AddClasses, "btn:hover secondary");
        _test_add_float_rule(obj, "btn", 0xbfbb7a5bu /* TestStyledComp.Width */, 50.f);
        /* Verify state before release */
        struct StyleController* sc = GetStyleController(obj);
        EXPECT(sc != NULL);
        EXPECT(sc->classes    != NULL);
        EXPECT(sc->StyleSheet != NULL);
        /* Dispatch Object.Release → StyleController.Release frees classes + sheet */
        OBJ_SendMessageW(obj, ID_Object_Release, 0, NULL);
        EXPECT(sc->classes    == NULL);
        EXPECT(sc->StyleSheet == NULL);
        /* Re-dispatch must be safe (idempotent) */
        OBJ_SendMessageW(obj, ID_Object_Release, 0, NULL);
    }
}

/* OBJ_GetStyleFlags returns 0 when no pseudo-state is active */
static void test_get_style_flags_none(void) {
    WITH(struct Object, obj, make_styled_object(), destroy_object) {
        EXPECT(OBJ_GetStyleFlags(obj) == 0);
    }
}

/*
 * ThemeChanged via _SendMessage (replaces OBJ_ApplyStyles):
 * Add a "btn" → Width=42 rule via native StyleRule/StyleSheet objects,
 * parse the "btn" class, then apply ThemeChanged.
 * Verify that the Width property was updated to 42.
 */
static void test_apply_styles_float_property(void) {
    WITH(struct Object, obj, make_styled_object(), destroy_object) {
        /* Register the rule: "btn { Width: 42 }" via native objects */
        _test_add_float_rule(obj, "btn",
                             0xbfbb7a5bu /* fnv1a32("TestStyledComp.Width") */, 42.f);
        /* Apply the "btn" class */
        _SendMessage(obj, StyleController, AddClasses, "btn");
        /* Apply styles — sends StyleController.ThemeChanged message */
        _SendMessage(obj, StyleController, ThemeChanged, .recursive = FALSE);
        /* Verify the Width property was set to 42 */
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Width", &prop));
        EXPECT(!PROP_IsNull(prop));
        EXPECT(*(float *)PROP_GetValue(prop) == 42.f);
    }
}

/*
 * ThemeChanged is silent on objects without StyleController.
 * Sending the message to a bare object must not crash.
 */
static void test_apply_styles_no_component(void) {
    do {
    lpObject_t obj = calloc(1, sizeof(struct Object));
    EXPECT(obj != NULL);
    _SendMessage(obj, StyleController, ThemeChanged, .recursive = FALSE);  /* must not crash */
    free(obj);
    } while(0);
}

/* Multi-pseudo-state "btn:hover:focus" — both flags must be set */
static void test_parse_class_multiple_pseudo_states(void) {
    WITH(struct Object, obj, make_styled_object(), destroy_object) {
        _SendMessage(obj, StyleController, AddClasses, "btn:hover:focus");
        struct StyleController* sc = GetStyleController(obj);
        EXPECT(sc != NULL);
        EXPECT(sc->classes != NULL);
        /* Both STYLE_HOVER and STYLE_FOCUS must be set */
        EXPECT((sc->classes->flags & STYLE_HOVER) != 0);
        EXPECT((sc->classes->flags & STYLE_FOCUS) != 0);
    }
}

/* _ParseClass must store only the base name, not pseudo-states */
static void test_parse_class_base_name_only(void) {
    WITH(struct Object, obj, make_styled_object(), destroy_object) {
        _SendMessage(obj, StyleController, AddClasses, "myclass:hover");
        struct StyleController* sc = GetStyleController(obj);
        EXPECT(sc != NULL);
        EXPECT(sc->classes != NULL);
        /* cls->value must be "myclass", not "myclass:hover" */
        EXPECT(!strcmp(sc->classes->value, "myclass"));
    }
}

/* _ParseClass with opacity: base name is the part before '/' */
static void test_parse_class_opacity_extraction(void) {
    WITH(struct Object, obj, make_styled_object(), destroy_object) {
        _SendMessage(obj, StyleController, AddClasses, "blue/50");
        struct StyleController* sc = GetStyleController(obj);
        EXPECT(sc != NULL);
        EXPECT(sc->classes != NULL);
        EXPECT(!strcmp(sc->classes->value, "blue"));
        EXPECT(sc->classes->opacity == 50);
    }
}

/*
 * Rule added with className "btn" (no leading dot) must match parsed class "btn".
 * Uses native StyleRule/StyleSheet objects for rule creation.
 */
static void test_add_style_rule_dot_selector_matches(void) {
    WITH(struct Object, obj, make_styled_object(), destroy_object) {
        /* Rule registered without leading dot */
        _test_add_float_rule(obj, "btn",
                             0xbfbb7a5bu /* fnv1a32("TestStyledComp.Width") */, 77.f);
        /* Class parsed without dot */
        _SendMessage(obj, StyleController, AddClasses, "btn");
        _SendMessage(obj, StyleController, ThemeChanged, .recursive = FALSE);
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Width", &prop));
        EXPECT(!PROP_IsNull(prop));
        EXPECT(*(float *)PROP_GetValue(prop) == 77.f);
    }
}

/* ------------------------------------------------------------------ */
/* Test runner                                                         */
/* ------------------------------------------------------------------ */

#define DECL_TEST(func) { #func, func }

int main(void) {
    mem_init();

    printf("ORCA style system tests\n");
    printf("=======================\n");

    register_styled_class();

    static const struct { const char *name; void (*fn)(void); } tests[] = {
        DECL_TEST(test_stylecontroller_attached),
        DECL_TEST(test_parse_class_hover_flag),
        DECL_TEST(test_parse_multiple_classes),
        DECL_TEST(test_parse_class_no_hover),
        DECL_TEST(test_add_class_hover),
        DECL_TEST(test_add_style_rule_no_component),
        DECL_TEST(test_release_clears_classes_and_sheet),
        DECL_TEST(test_get_style_flags_none),
        DECL_TEST(test_apply_styles_float_property),
        DECL_TEST(test_apply_styles_no_component),
        DECL_TEST(test_parse_class_multiple_pseudo_states),
        DECL_TEST(test_parse_class_base_name_only),
        DECL_TEST(test_parse_class_opacity_extraction),
        DECL_TEST(test_add_style_rule_dot_selector_matches),
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
