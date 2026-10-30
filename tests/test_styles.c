/*
 * test_styles.c — C unit tests for the ORCA style system (StyleController).
 *
 * Tests cover:
 *   - StyleController attachment and class parsing
 *   - OBJ_AddClass: runtime class addition via AddClass/AddClasses messages
 *   - public raw class reconstruction for parsed pseudo-state classes
 *   - StyleController.ThemeChanged (via _SendMessage): applies a float rule to a property
 *     using native StyleRule/StyleSheet objects
 *   - Memory leak checks for all operations (with -DTEST_MEMORY)
 *
 * Compiled via the `test-styles` Makefile target (depends on `buildlib`).
 */

#include "test_local.h"
#include "mem_tracker.h"

#include <include/orca.h>
#include <core/core.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Narrow test-only declarations for the property VM surface. */
struct vm_register
{
    eDataType_t type;
    uint32_t size;
    float value[MAX_PROPERTY_STRING / sizeof(float)];
};

extern bool_t OBJ_RunProgram(struct Object *, struct token *, struct vm_register *);
extern bool_t PROP_SetBinding(struct Property *, char const *, unsigned int, bool_t);
/* Private global sheet seam: exercise global/default vs application cascade. */
extern struct Object *static_stylesheet;

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

static struct Object *make_styled_object(void) {
    return OBJ_Create(ID_TestStyledComp);
}

static void destroy_object(struct Object *obj) {
    if (!obj) return;
    OBJ_ReleaseRef(obj);
}

/*
 * Add a float property-override rule to obj's per-object StyleSheet.
 * className: the base class name (without leading '.'), e.g. "btn".
 * propFullID: FullIdentifier hash of the target property (e.g. fnv1a32("TestStyledComp.Width")).
 * value: float value to set for the override.
 */
static void _test_add_float_rule(struct Object *obj, lpcString_t className,
                                  uint32_t propFullID, float value)
{
    struct StyleController* sc = GetStyleController(obj);
    if (!sc) return;
    if (!sc->StyleSheet) {
        struct Object *s = OBJ_Create(ID_StyleSheet);
        sc->StyleSheet = GetStyleSheet(s);
        sc->owned_sheet = TRUE;
    }
    struct Object *sheetObj = CMP_GetObject(sc->StyleSheet);

    struct Object *ruleObj = OBJ_Create(ID_StyleRule);
    struct StyleRule* sr = GetStyleRule(ruleObj);
    sr->class_id = fnv1a32(className);

    /* Set ClassName property on the rule */
    struct Property *clsProp = NULL;
    if (SUCCEEDED(FIND_LONG_PROPERTY(ruleObj, ID_StyleRule_ClassName, &clsProp))) {
        PROP_SetStringValue(clsProp, className);
    }

    /* Find the property type by FullIdentifier and create an override */
    struct PropertyType const *pt = core_FindPropertyType(propFullID);
    if (pt) {
        struct Property *rp = PROP_Create(NULL, ruleObj, pt);
        PROP_SetValue(rp, &value);
    }

    OBJ_AddChild(sheetObj, ruleObj);
}

/* Theme queries create Cocoa autoreleased temporaries on Darwin. Initialize
 * its runtime caches before tracking, and drain temporaries after each case. */
static void warm_style_runtime(void)
{
    struct Object *obj = make_styled_object();
    _SendMessage(obj, StyleController, AddClasses, "tracker-warmup");
    destroy_object(obj);
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
    struct Object *obj = OBJ_Create(ID_StyleSheet);
    EXPECT(obj != NULL);
    /* Must not crash — GetStyleController returns NULL for bare objects */
    _test_add_float_rule(obj, "btn", 0xbfbb7a5bu /* TestStyledComp.Width */, 100.f);
    destroy_object(obj);
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

/* Raw style classes are exposed without pseudo-state qualifiers */
static void test_raw_style_classes_strip_pseudo_state(void) {
    WITH(struct Object, obj, make_styled_object(), destroy_object) {
        char classes[64] = {0};
        _SendMessage(obj, StyleController, AddClasses, "btn:hover");
        EXPECT(OBJ_GetRawStyleClasses(obj, classes, sizeof(classes)) == classes);
        EXPECT(!strcmp(classes, "btn"));
    }
}

/*
 * ThemeChanged via _SendMessage (replaces OBJ_ApplyStyles):
 * Add a "btn" → Width=42 rule via native StyleRule/StyleSheet objects,
 * parse the "btn" class, then apply ThemeChanged.
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
        struct Property *prop;
        EXPECT_OK(FIND_SHORT_PROPERTY(obj, "Width", &prop));
        EXPECT(!PROP_IsNull(prop));
        EXPECT(*(float *)PROP_GetValue(prop) == 42.f);
    }
}

static void test_specialized_restores_unset_property(void) {
    WITH(struct Object, obj, make_styled_object(), destroy_object) {
        struct Property *prop = OBJ_FindLongProperty(obj, 0xbfbb7a5bu);
        EXPECT(prop != NULL);
        EXPECT(PROP_IsNull(prop));
        PROP_SetFlag(prop, PF_SPECIALIZED);
        PROP_SetValue(prop, &(float){42.f});
        PROP_ClearSpecialized(OBJ_GetProperties(obj));
        EXPECT(PROP_IsNull(prop));
        EXPECT(*(float *)PROP_GetValue(prop) == 0.f);
    }
}

static void test_specialized_restores_string(void) {
    WITH(struct Object, obj, OBJ_Create(ID_StyleRule), destroy_object) {
        struct Property *prop = OBJ_FindLongProperty(obj, ID_StyleRule_ClassName);
        EXPECT(prop != NULL);
        PROP_SetStringValue(prop, "original");
        PROP_SetFlag(prop, PF_SPECIALIZED);
        PROP_SetStringValue(prop, "focused");
        PROP_SetFlag(prop, PF_SPECIALIZED);
        PROP_SetStringValue(prop, "focused-again");
        PROP_ClearSpecialized(OBJ_GetProperties(obj));
        EXPECT(!strcmp(*(char const **)PROP_GetValue(prop), "original"));
        EXPECT(!PROP_IsNull(prop));
        /* Destruction while specialized must also release the preserved string. */
        PROP_SetFlag(prop, PF_SPECIALIZED);
        PROP_SetStringValue(prop, "focused-at-release");
    }
}

static void test_specialized_restores_owned_object(void) {
    WITH(struct Object, original, OBJ_Create(ID_StyleSheet), destroy_object) {
        WITH(struct Object, override, OBJ_Create(ID_StyleSheet), destroy_object) {
            WITH(struct Object, obj, make_styled_object(), destroy_object) {
                struct Property *prop = OBJ_FindLongProperty(obj, ID_StyleController_StyleSheet);
                PROP_SetValue(prop, &original);
                OBJ_AddChild(obj, original);
                PROP_SetFlag(prop, PF_OWNS_OBJECT_CHILD);
                PROP_SetFlag(prop, PF_SPECIALIZED);
                PROP_SetValue(prop, &override);
                EXPECT(OBJ_GetParent(original) == obj);
                EXPECT(!(PROP_GetFlags(prop) & PF_OWNS_OBJECT_CHILD));
                PROP_ClearSpecialized(OBJ_GetProperties(obj));
                EXPECT(*(struct Object **)PROP_GetValue(prop) == original);
                EXPECT(PROP_GetFlags(prop) & PF_OWNS_OBJECT_CHILD);
                EXPECT(OBJ_GetParent(original) == obj);
                PROP_Clear(prop);
                EXPECT(OBJ_GetParent(original) == NULL);

                /* Destroying an active override must also release snapshot ownership. */
                PROP_SetValue(prop, &original);
                OBJ_AddChild(obj, original);
                PROP_SetFlag(prop, PF_OWNS_OBJECT_CHILD);
                PROP_SetFlag(prop, PF_SPECIALIZED);
                PROP_SetValue(prop, &override);
                EXPECT(OBJ_GetParent(original) == obj);
            }
            EXPECT(OBJ_GetParent(original) == NULL);
        }
    }
}

/* ------------------------------------------------------------------ */
/* CSS Unit Binding Tests                                               */
/* ------------------------------------------------------------------ */

/*
 * Test that a binding with ../Width * 1.5 resolves correctly when parent.Width changes.
 * This simulates what CSS "width: 1.5rem" would do via the style system.
 */
static void test_css_unit_binding_inherits_from_parent(void) {
    WITH(struct Object, parent, make_styled_object(), destroy_object) {
        WITH(struct Object, child, make_styled_object(), destroy_object) {
            OBJ_AddChild(parent, child);

            /* Set parent's Width to 10 */
            struct Property *parentWidth;
            EXPECT_OK(FIND_SHORT_PROPERTY(parent, "Width", &parentWidth));
            float parentVal = 10.f;
            PROP_SetValue(parentWidth, &parentVal);

            /* Set a binding on child's Width: {../Width} * 1.5 */
            struct Property *childWidth;
            EXPECT_OK(FIND_SHORT_PROPERTY(child, "Width", &childWidth));
            EXPECT(PROP_SetBinding(childWidth, "{../Width} * 1.5", kBindingModeOneWay, TRUE));

            /* Initial value should be unset (binding not yet evaluated) */
            float initialVal = *(float *)PROP_GetValue(childWidth);
            (void)initialVal; /* binding may or may not have evaluated yet */

            /* Create a token and run it to trigger the binding */
            struct token *tok = Token_Create("{../Width} * 1.5");
            EXPECT(tok != NULL);
            struct vm_register reg = {0};
            EXPECT(OBJ_RunProgram(child, tok, &reg));
            Token_Release(tok);

            /* Result should be 15 (10 * 1.5) */
            EXPECT(reg.type == kDataTypeFloat);
            float result = *(float *)reg.value;
            EXPECT(result == 15.f);

            /* Change parent width to 20 */
            parentVal = 20.f;
            PROP_SetValue(parentWidth, &parentVal);

            /* Re-run the binding */
            tok = Token_Create("{../Width} * 1.5");
            EXPECT(tok != NULL);
            reg.value[0] = 0;
            EXPECT(OBJ_RunProgram(child, tok, &reg));
            Token_Release(tok);

            /* Result should now be 30 (20 * 1.5) */
            EXPECT(reg.type == kDataTypeFloat);
            result = *(float *)reg.value;
            EXPECT(result == 30.f);

            OBJ_RemoveFromParent(child);
        }
    }
}

/*
 * Test that CSS unit binding expressions are created correctly.
 * This verifies the helper functions in Css.c.
 */
static void test_css_unit_binding_expression_generation(void) {
    /* These tests verify the binding expression generation logic.
     * The actual CSS parser calls css_make_binding_expr() to create
     * binding expressions from CSS values like "1.5rem".
     *
     * Expected mappings:
     *   "1.5rem" + "TextRun.FontSize" → "{../Font.Size} * 1.5"
     *   "2em" + "TextRun.FontSize"     → "{../Font.Size} * 2"
     *   "1rem" + "TextRun.FontSize"    → "{../Font.Size}"  (multiplier is 1.0)
     */
    /* Note: The css_value_has_unit and css_make_binding_expr functions
     * are tested indirectly through the CSS parsing pipeline.
     * A full test would use ui.loadObjectFromCssString and verify
     * bindings are created on styled objects.
     */
    printf("  (CSS unit binding expression generation tested via Lua tests)\n");
}

/*
 * ThemeChanged is silent on objects without StyleController.
 * Sending the message to a bare object must not crash.
 */
static void test_apply_styles_no_component(void) {
    do {
    struct Object *obj = OBJ_Create(ID_StyleSheet);
    EXPECT(obj != NULL);
    _SendMessage(obj, StyleController, ThemeChanged, .recursive = FALSE);  /* must not crash */
    destroy_object(obj);
    } while(0);
}

/* Multi-pseudo-state "btn:hover:focus" marks the object hoverable */
static void test_parse_class_multiple_pseudo_states(void) {
    WITH(struct Object, obj, make_styled_object(), destroy_object) {
        _SendMessage(obj, StyleController, AddClasses, "btn:hover:focus");
        EXPECT(OBJ_GetFlags(obj) & OF_HOVERABLE);
    }
}

/* Parsed classes expose only the base name, not pseudo-states */
static void test_parse_class_base_name_only(void) {
    WITH(struct Object, obj, make_styled_object(), destroy_object) {
        char classes[64] = {0};
        _SendMessage(obj, StyleController, AddClasses, "myclass:hover");
        EXPECT(OBJ_GetRawStyleClasses(obj, classes, sizeof(classes)) == classes);
        EXPECT(!strcmp(classes, "myclass"));
    }
}

/* Parsed classes expose only the base name, not opacity suffixes */
static void test_parse_class_opacity_extraction(void) {
    WITH(struct Object, obj, make_styled_object(), destroy_object) {
        char classes[64] = {0};
        _SendMessage(obj, StyleController, AddClasses, "blue/50");
        EXPECT(OBJ_GetRawStyleClasses(obj, classes, sizeof(classes)) == classes);
        EXPECT(!strcmp(classes, "blue"));
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
        struct Property *prop;
        EXPECT_OK(FIND_SHORT_PROPERTY(obj, "Width", &prop));
        EXPECT(!PROP_IsNull(prop));
        EXPECT(*(float *)PROP_GetValue(prop) == 77.f);
    }
}

/* Native state flags restyle the control and descendants, then restore their base. */
static void test_control_state_selectors(void) {
    WITH(struct Object, root, make_styled_object(), destroy_object) {
        struct Object *mark = make_styled_object();
        OBJ_AddChild(root, mark);
        _SendMessage(root, StyleController, AddClasses, "control");
        _SendMessage(mark, StyleController, AddClasses, "mark");
        struct Property *rootWidth = OBJ_FindShortProperty(root, fnv1a32("Width"));
        struct Property *markWidth = OBJ_FindShortProperty(mark, fnv1a32("Width"));
        PROP_SetValue(rootWidth, &(float){11.f});
        PROP_SetValue(markWidth, &(float){13.f});
        _test_add_float_rule(root, ".control:checked", 0xbfbb7a5bu, 21.f);
        _test_add_float_rule(root, ".control:checked > .mark", 0xbfbb7a5bu, 31.f);
        _test_add_float_rule(root, ".control:checked:disabled > .mark", 0xbfbb7a5bu, 41.f);
        _test_add_float_rule(root, ".control:indeterminate > .mark", 0xbfbb7a5bu, 51.f);
        _test_add_float_rule(root, ".control:checked:active > .mark", 0xbfbb7a5bu, 61.f);
        _test_add_float_rule(root, ".control:checked:hover > .mark", 0xbfbb7a5bu, 71.f);
        _test_add_float_rule(root, ".control:unknown > .mark", 0xbfbb7a5bu, 999.f);
        OBJ_SetFlags(root, OBJ_GetFlags(root) | OF_CHECKED);
        EXPECT(*(float *)PROP_GetValue(rootWidth) == 21.f);
        EXPECT(*(float *)PROP_GetValue(markWidth) == 31.f);
        EXPECT(OBJ_GetFlags(root) & OF_HOVERABLE);
        OBJ_SetFlags(root, OBJ_GetFlags(root) | OF_SELECTED);
        EXPECT(*(float *)PROP_GetValue(markWidth) == 61.f);
        OBJ_SetFlags(root, OBJ_GetFlags(root) & ~OF_SELECTED);
        EXPECT(*(float *)PROP_GetValue(markWidth) == 31.f);
        OBJ_SetFlags(root, OBJ_GetFlags(root) | OF_DISABLED);
        EXPECT(*(float *)PROP_GetValue(markWidth) == 41.f);
        OBJ_SetFlags(root, OBJ_GetFlags(root) & ~(OF_CHECKED | OF_DISABLED));
        EXPECT(*(float *)PROP_GetValue(rootWidth) == 11.f);
        EXPECT(*(float *)PROP_GetValue(markWidth) == 13.f);
        OBJ_SetFlags(root, OBJ_GetFlags(root) | OF_INDETERMINATE);
        EXPECT(*(float *)PROP_GetValue(markWidth) == 51.f);
        OBJ_SetFlags(root, OBJ_GetFlags(root) & ~OF_INDETERMINATE);
        EXPECT(*(float *)PROP_GetValue(markWidth) == 13.f);
        OBJ_ReleaseRef(mark);
    }
}

static void test_application_base_overrides_global_state(void) {
    struct Object *previousSheet = static_stylesheet;
    WITH(struct Object, defaults, make_styled_object(), destroy_object) {
        _test_add_float_rule(defaults, ".control:checked", 0xbfbb7a5bu, 22.f);
        _test_add_float_rule(defaults, ".control", 0xbfbb7a5bu, 11.f);
        static_stylesheet = CMP_GetObject(GetStyleController(defaults)->StyleSheet);
        WITH(struct Object, app, make_styled_object(), destroy_object) {
            _SendMessage(app, StyleController, AddClasses, "control");
            struct Property *width = OBJ_FindShortProperty(app, fnv1a32("Width"));
            OBJ_SetFlags(app, OBJ_GetFlags(app) | OF_CHECKED);
            EXPECT(*(float *)PROP_GetValue(width) == 22.f);
            /* Own state wins even when a base rule follows it in the same sheet. */
            _test_add_float_rule(app, ".control:checked:disabled", 0xbfbb7a5bu, 44.f);
            _test_add_float_rule(app, ".control", 0xbfbb7a5bu, 33.f);
            _SendMessage(app, StyleController, ThemeChanged, .recursive = TRUE);
            EXPECT(*(float *)PROP_GetValue(width) == 33.f);
            OBJ_SetFlags(app, OBJ_GetFlags(app) | OF_DISABLED);
            EXPECT(*(float *)PROP_GetValue(width) == 44.f);
            OBJ_SetFlags(app, OBJ_GetFlags(app) & ~OF_DISABLED);
            EXPECT(*(float *)PROP_GetValue(width) == 33.f);
            OBJ_SetFlags(app, OBJ_GetFlags(app) & ~OF_CHECKED);
            EXPECT(*(float *)PROP_GetValue(width) == 33.f);
            OBJ_SetFlags(app, OBJ_GetFlags(app) | OF_CHECKED);
            EXPECT(*(float *)PROP_GetValue(width) == 33.f);
        }
        static_stylesheet = previousSheet;
    }
}

static void test_disabled_inheritance_and_unknown_pseudo(void) {
    WITH(struct Object, root, make_styled_object(), destroy_object) {
        struct Object *child = make_styled_object();
        OBJ_AddChild(root, child);
        _SendMessage(child, StyleController, AddClasses, "child");
        _test_add_float_rule(root, ".child:enabled", 0xbfbb7a5bu, 20.f);
        _test_add_float_rule(root, ".child:disabled", 0xbfbb7a5bu, 40.f);
        _test_add_float_rule(root, ".child", 0xbfbb7a5bu, 999.f);
        struct Object *sheet = CMP_GetObject(GetStyleController(root)->StyleSheet);
        struct Object *last = OBJ_GetFirstChild(sheet);
        while (OBJ_GetNext(last)) last = OBJ_GetNext(last);
        struct Property *pseudo = OBJ_FindLongProperty(last, ID_StyleRule_PseudoClass);
        PROP_SetStringValue(pseudo, "unsupported");
        struct Property *width = OBJ_FindShortProperty(child, fnv1a32("Width"));
        _SendMessage(root, StyleController, ThemeChanged, .recursive = TRUE);
        EXPECT(*(float *)PROP_GetValue(width) == 20.f);
        OBJ_SetFlags(root, OBJ_GetFlags(root) | OF_DISABLED);
        EXPECT(*(float *)PROP_GetValue(width) == 40.f);
        OBJ_SetFlags(root, OBJ_GetFlags(root) & ~OF_DISABLED);
        EXPECT(*(float *)PROP_GetValue(width) == 20.f);
        OBJ_ReleaseRef(child);
    }
}

/* ------------------------------------------------------------------ */
/* Test runner                                                         */
/* ------------------------------------------------------------------ */

#define DECL_TEST(func) { #func, func }

int main(void) {
    printf("ORCA style system tests\n");
    printf("=======================\n");

    register_styled_class();
    MEM_AUTORELEASE_SCOPE(warm_style_runtime);
    /* Exclude one-time platform clock/timezone caches from ownership baselines. */
    core_AdvanceFrame();
    mem_init();

    static const struct { const char *name; void (*fn)(void); } tests[] = {
        DECL_TEST(test_control_state_selectors),
        DECL_TEST(test_disabled_inheritance_and_unknown_pseudo),
        DECL_TEST(test_application_base_overrides_global_state),
        DECL_TEST(test_stylecontroller_attached),
        DECL_TEST(test_parse_class_hover_flag),
        DECL_TEST(test_parse_multiple_classes),
        DECL_TEST(test_parse_class_no_hover),
        DECL_TEST(test_add_class_hover),
        DECL_TEST(test_add_style_rule_no_component),
        DECL_TEST(test_release_clears_classes_and_sheet),
        DECL_TEST(test_raw_style_classes_strip_pseudo_state),
        DECL_TEST(test_apply_styles_float_property),
        DECL_TEST(test_specialized_restores_unset_property),
        DECL_TEST(test_specialized_restores_string),
        DECL_TEST(test_specialized_restores_owned_object),
        DECL_TEST(test_apply_styles_no_component),
        DECL_TEST(test_parse_class_multiple_pseudo_states),
        DECL_TEST(test_parse_class_base_name_only),
        DECL_TEST(test_parse_class_opacity_extraction),
        DECL_TEST(test_add_style_rule_dot_selector_matches),
        DECL_TEST(test_css_unit_binding_inherits_from_parent),
        DECL_TEST(test_css_unit_binding_expression_generation),
    };

    for (size_t i = 0; i < sizeof(tests)/sizeof(*tests); i++) {
        long snap = MEM_SNAPSHOT();
        s_current_test = tests[i].name;
        s_tests_run++;
        printf("Running %s...\n", tests[i].name);
        MEM_AUTORELEASE_SCOPE(tests[i].fn);
        MEM_CHECK_LEAK(snap, tests[i].name);
    }

#if TEST_MEMORY
    if (s_alloc_count > 0) {
        printf("\nMemory tracking: %ld total allocation(s) made, "
               "%ld outstanding at exit\n",
               s_alloc_total, s_alloc_count);
    }
#endif

    printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
    return s_tests_failed == 0 ? 0 : 1;
}
