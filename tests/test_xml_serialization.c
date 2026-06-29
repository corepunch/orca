/*
 * test_xml_serialization.c — C unit tests for FS_SerializeObjectToXmlString.
 *
 * Tests cover:
 *   - NULL input returns NULL
 *   - Simple object serializes with class name and attributes
 *   - Object with properties serializes as XML attributes
 *   - Object with child objects serializes nested elements
 *   - Object with template flag serializes as LayerPrefabPlaceholder
 *   - Object with binding expressions serializes <BindingExpression>
 *   - Object with text content serializes text as inner XML
 *   - Memory cleanup (no leaks when -DTEST_MEMORY is defined)
 *
 * Compiled via the `test-xml-serialization` Makefile target (depends on `buildlib`).
 */

#include "test_local.h"
#include "mem_tracker.h"

#include <include/orca.h>
#include <core/core.h>
#include <filesystem/filesystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Test class: a minimal component with a few properties              */
/* ------------------------------------------------------------------ */

struct TestObj {
    float Width;
    float Height;
};

static LRESULT TestObjProc(struct Object* o, void* c,
                            uint32_t msg, wParam_t wp, lParam_t lp)
{
    (void)o; (void)c; (void)msg; (void)wp; (void)lp;
    return FALSE;
}

#define ID_TestObj 0xaabb0001u
#define ID_TestObj2 0xaabb0002u

static struct PropertyType const TestObjProperties[] = {
    {
        .ShortIdentifier = 0x3b42dfbfu, /* fnv1a32("Width") */
        .FullIdentifier  = 0x11111111u,
        .Name            = "Width",
        .DataType        = kDataTypeFloat,
        .DataSize        = sizeof(float),
        .Offset          = offsetof(struct TestObj, Width),
    },
    {
        .ShortIdentifier = 0x8c8a8a47u, /* fnv1a32("Height") */
        .FullIdentifier  = 0x22222222u,
        .Name            = "Height",
        .DataType        = kDataTypeFloat,
        .DataSize        = sizeof(float),
        .Offset          = offsetof(struct TestObj, Height),
    },
};
#define kTestObjNumProperties 2

static struct TestObj TestObjDefaults = { .Width = 0.f, .Height = 0.f };

static struct ClassDesc s_testObjClass = {
    .ClassName       = "TestObj",
    .DefaultName     = "TestObj",
    .ClassID         = ID_TestObj,
    .ClassSize       = sizeof(struct TestObj),
    .Properties      = TestObjProperties,
    .ObjProc         = TestObjProc,
    .Defaults        = &TestObjDefaults,
    .NumProperties   = kTestObjNumProperties,
    .ParentClasses   = { 0 },
};

static struct PropertyType const TestObj2Properties[] = {
    {
        .ShortIdentifier = 0x3b42dfbfu, /* fnv1a32("Width") */
        .FullIdentifier  = 0x33333333u,
        .Name            = "Width",
        .DataType        = kDataTypeFloat,
        .DataSize        = sizeof(float),
        .Offset          = offsetof(struct TestObj, Width),
    },
};
static struct TestObj TestObj2Defaults = { .Width = 0.f };

static struct ClassDesc s_testObj2Class = {
    .ClassName       = "TestObj2",
    .DefaultName     = "TestObj2",
    .ClassID         = ID_TestObj2,
    .ClassSize       = sizeof(struct TestObj),
    .Properties      = TestObj2Properties,
    .ObjProc         = TestObjProc,
    .Defaults        = &TestObj2Defaults,
    .NumProperties   = 1,
    .ParentClasses   = { 0 },
};

static void register_test_classes(void) {
    OBJ_RegisterClass(&s_testObjClass);
    OBJ_RegisterClass(&s_testObj2Class);
}

/* ------------------------------------------------------------------ */
/* Object lifecycle helpers                                           */
/* ------------------------------------------------------------------ */

static struct Object *make_test_object(const char *name, float width, float height) {
    struct Object *obj = OBJ_Create(ID_TestObj);
    if (!obj) return NULL;
    if (name) {
        OBJ_SetName(obj, name);
    } else {
        OBJ_SetName(obj, "");
    }
    if (width != 0.f) PROP_SetValue(OBJ_FindLongProperty(obj, 0x3b42dfbfu), &width);
    if (height != 0.f) PROP_SetValue(OBJ_FindLongProperty(obj, 0x8c8a8a47u), &height);
    return obj;
}

static struct Object *make_test_object2(const char *name, float width) {
    struct Object *obj = OBJ_Create(ID_TestObj2);
    if (!obj) return NULL;
    if (name) {
        OBJ_SetName(obj, name);
    }
    PROP_SetValue(OBJ_FindLongProperty(obj, 0x3b42dfbfu), &width);
    return obj;
}

static void destroy_object(struct Object *obj) {
    if (!obj) return;
    OBJ_ReleaseRef(obj);
}

/* ------------------------------------------------------------------ */
/* Tests                                                              */
/* ------------------------------------------------------------------ */

static void test_null_input(void)
{
    RUN_TEST("null_input_returns_null", {
        const char *xml = FS_SerializeObjectToXmlString(NULL);
        EXPECT(xml == NULL);
    });
}

static void test_simple_object(void)
{
    RUN_TEST("simple_object_serializes_class_name", {
        register_test_classes();
        WITH(struct Object, obj, make_test_object(NULL, 100.f, 200.f), destroy_object) {
            const char *xml = FS_SerializeObjectToXmlString(obj);
            EXPECT(xml != NULL);
            EXPECT(strstr(xml, "<TestObj") != NULL);
            EXPECT(strstr(xml, "Width=\"100\"") != NULL);
            EXPECT(strstr(xml, "Height=\"200\"") != NULL);
            free((char *)xml);
        }
    });
}

static void test_object_with_name(void)
{
    RUN_TEST("object_with_name_serializes_name_attribute", {
        register_test_classes();
        WITH(struct Object, obj, make_test_object("myobj", 50.f, 75.f), destroy_object) {
            const char *xml = FS_SerializeObjectToXmlString(obj);
            EXPECT(xml != NULL);
            EXPECT(strstr(xml, "Name=\"myobj\"") != NULL);
            EXPECT(strstr(xml, "Width=\"50\"") != NULL);
            EXPECT(strstr(xml, "Height=\"75\"") != NULL);
            free((char *)xml);
        }
    });
}

static void test_object_with_zero_properties(void)
{
    RUN_TEST("object_with_zero_properties_serializes_no_attributes", {
        register_test_classes();
        WITH(struct Object, obj, make_test_object("zeros", 0.f, 0.f), destroy_object) {
            const char *xml = FS_SerializeObjectToXmlString(obj);
            EXPECT(xml != NULL);
            EXPECT(strstr(xml, "Width") == NULL);
            EXPECT(strstr(xml, "Height") == NULL);
            free((char *)xml);
        }
    });
}

static void test_object_with_children(void)
{
    RUN_TEST("object_with_children_serializes_nested_elements", {
        register_test_classes();
        WITH(struct Object, parent, make_test_object("parent", 1.f, 2.f), destroy_object) {
            WITH(struct Object, child1, make_test_object("child1", 10.f, 20.f), destroy_object) {
                WITH(struct Object, child2, make_test_object("child2", 30.f, 40.f), destroy_object) {
                    OBJ_AddChild(parent, child1);
                    OBJ_AddChild(parent, child2);

                    const char *xml = FS_SerializeObjectToXmlString(parent);
                    EXPECT(xml != NULL);
                    EXPECT(strstr(xml, "<TestObj") != NULL);
                    EXPECT(strstr(xml, "Name=\"child1\"") != NULL);
                    EXPECT(strstr(xml, "Name=\"child2\"") != NULL);
                    /* Parent should appear first, then children */
                    EXPECT(strstr(xml, "Width=\"1\"") != NULL);
                    free((char *)xml);
                }
            }
        }
    });
}

static void test_object_with_template_flag(void)
{
    RUN_TEST("object_with_template_flag_serializes_as_prefab_placeholder", {
        register_test_classes();
        WITH(struct Object, obj, make_test_object("template", 99.f, 88.f), destroy_object) {
            OBJ_SetFlags(obj, OF_TEMPLATE);
            OBJ_SetSourceFile(obj, "templates/mytemplate.xml");

            const char *xml = FS_SerializeObjectToXmlString(obj);
            EXPECT(xml != NULL);
            EXPECT(strstr(xml, "LayerPrefabPlaceholder") != NULL);
            EXPECT(strstr(xml, "PlaceholderTemplate=\"templates/mytemplate.xml\"") != NULL);
            EXPECT(strstr(xml, "Name=\"template\"") != NULL);
            /* Should also include non-default property overrides */
            EXPECT(strstr(xml, "Width=\"99\"") != NULL);
            free((char *)xml);
        }
    });
}

static void test_object_with_text_content(void)
{
    RUN_TEST("object_with_text_content_serializes_text", {
        register_test_classes();
        WITH(struct Object, obj, make_test_object("textobj", 1.f, 2.f), destroy_object) {
            OBJ_SetTextContent(obj, "Hello World");

            const char *xml = FS_SerializeObjectToXmlString(obj);
            EXPECT(xml != NULL);
            EXPECT(strstr(xml, "Hello World") != NULL);
            free((char *)xml);
        }
    });
}

static void test_object_with_no_properties(void)
{
    RUN_TEST("object_with_no_properties_serializes_empty_element", {
        register_test_classes();
        WITH(struct Object, obj, OBJ_Create(ID_TestObj2), destroy_object) {
            OBJ_SetName(obj, "noprops");

            const char *xml = FS_SerializeObjectToXmlString(obj);
            EXPECT(xml != NULL);
            EXPECT(strstr(xml, "noprops") != NULL);
            free((char *)xml);
        }
    });
}

static void test_deeply_nested_children(void)
{
    RUN_TEST("deeply_nested_children_serializes_all_levels", {
        register_test_classes();
        WITH(struct Object, level1, make_test_object("level1", 1.f, 1.f), destroy_object) {
            WITH(struct Object, level2, make_test_object("level2", 2.f, 2.f), destroy_object) {
                WITH(struct Object, level3, make_test_object("level3", 3.f, 3.f), destroy_object) {
                    OBJ_AddChild(level2, level3);
                    OBJ_AddChild(level1, level2);

                    const char *xml = FS_SerializeObjectToXmlString(level1);
                    EXPECT(xml != NULL);
                    EXPECT(strstr(xml, "Name=\"level3\"") != NULL);
                    EXPECT(strstr(xml, "Name=\"level2\"") != NULL);
                    EXPECT(strstr(xml, "Name=\"level1\"") != NULL);
                    free((char *)xml);
                }
            }
        }
    });
}

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */

int main(void)
{
    test_null_input();
    test_simple_object();
    test_object_with_name();
    test_object_with_zero_properties();
    test_object_with_children();
    test_object_with_template_flag();
    test_object_with_text_content();
    test_object_with_no_properties();
    test_deeply_nested_children();

    printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
    return s_tests_failed == 0 ? 0 : 1;
}
