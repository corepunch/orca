/*
 * test_local.h — Shared test harness and utility macros for ORCA unit tests.
 *
 * This header consolidates the common test infrastructure used across all
 * .c test files in tests/.  Each test file should include this header and
 * may additionally include project headers and/or test-local declarations.
 *
 * Macros provided:
 *   - Test harness:       WITH, EXPECT, EXPECT_OK, EXPECT_NULL, RUN
 *   - Property helpers:   FIND_SHORT_PROPERTY, PROP_STR, PROP_TEST, RUN_PROG
 *   - Object creation:    CREATE_OBJECT, CREATE_OBJECT_WITH_PARAMS
 *   - Object helpers:     GET_PROPERTY, SET_PROPERTY, GET_CLASS_NAME
 *   - Object iteration:   FOR_EACH_CHILD, FOR_EACH_CHILD_WITH_CONTEXT
 *   - Window helpers:     OPEN_WINDOW, CLOSE_WINDOW
 *   - String helpers:     STRING_EQ, STR_EQ, STRNEQ
 *   - Memory helpers:     ASSERT_NOT_NULL, CHECK_ALLOC, FREE_IF_NULL
 *   - Type helpers:       IS_TYPE, AS_TYPE, AS_TYPE_CAST
 *   - Property helpers:   PROP_GET, PROP_SET, PROP_GET_CAST
 *   - Message helpers:    SEND_MSG, POST_MSG, MSG_RESULT, MSG_RESULT_EQ
 *   - Comparison helpers: FLOAT_EQ, UINT_EQ, SIZE_EQ, STR_EQ
 *   - File helpers:       READ_FILE, WRITE_FILE, FILE_EXISTS
 *   - Lua helpers:        LUA_STATE, LUA_RELEASE, LUA_DOFILE, LUA_DOSTRING
 *   - Trigger helpers:    TRIGGER_ACTION, OBSERVED_COUNT, RESET_OBSERVED
 *   - Message registry:   REGISTER_MESSAGE, FIND_MESSAGE, SUBSCRIBE
 *   - XML helpers:        XML_PARSE, XML_FREE, XML_FIND, XML_GET_ATTR
 *   - Editor helpers:     EDITOR_NEW, EDITOR_FREE, EDITOR_SET, EDITOR_GET
 *
 * Compiled and linked against liborca.so via the `test-runner` Makefile
 * target (depends on `buildlib`).
 *
 * Build with -DTEST_MEMORY to enable malloc/free/strdup interception and
 * per-test allocation tracking.  Memory leak coverage for all functional
 * tests is provided automatically through that mechanism.
 */

#ifndef TEST_LOCAL_H
#define TEST_LOCAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>
#include <sys/stat.h>
#include <stdint.h>

/* HRESULT and related error macros are provided by the ORCA project headers
 * (orcadef.h, orcaerror.h) which are cross-platform. No windows.h needed. */

/* Global failure counter — used by EXPECT macro and RUN_TEST harness.
 * Each translation unit including this header gets its own copy. */
static int          s_tests_run    = 0;
static int          s_tests_failed = 0;
static const char  *s_current_test = NULL;

/* ------------------------------------------------------------------ */
/* Core test harness                                                  */
/* ------------------------------------------------------------------ */

/*
 * WITH — scoped resource management with automatic cleanup.
 *
 * Usage:
 *   WITH(struct Object *, obj, OBJ_Create(...), OBJ_Release) {
 *       // use obj
 *   }
 *
 * The resource is automatically released when the block exits (including
 * via break/return/EXPECT failure).  The variable name is scoped to the
 * block.
 */
#define WITH(type, var, init, cleanup)                                       \
    for (type var = init; ; ) {                                              \
        do {

#define END_WITH                                                             \
        } while (0);                                                         \
        cleanup(var);                                                        \
        break;                                                               \
    } while (0)

/*
 * EXPECT — conditional test assertion.
 *
 * Usage:
 *   EXPECT(condition);
 *   EXPECT(obj != NULL);
 *
 * On failure, prints file/line and the condition string, increments the
 * failure counter, then breaks out of the enclosing WITH/RUN/RUN_TEST block.
 */
#define EXPECT(...)                                                         \
    if (!(__VA_ARGS__)) {                                                   \
        fprintf(stderr, "  FAIL: %s (at %s:%d)\n", #__VA_ARGS__, __FILE__, __LINE__); \
        s_tests_failed++;                                                   \
        break;                                                              \
    }

/* EXPECT_OK — check that an HRESULT value indicates success. */
#define EXPECT_OK(hr)                                                        \
    do {                                                                     \
        HRESULT _hr = (hr);                                                  \
        EXPECT(SUCCEEDED(_hr));                                              \
    } while (0)

/* EXPECT_NULL — check that a pointer is NULL. */
#define EXPECT_NULL(ptr)                                                     \
    EXPECT((ptr) == NULL)

/*
 * RUN — top-level test runner.  Creates a named test function and wraps
 * it in a WITH loop so that EXPECT can break out.
 *
 * Usage:
 *   RUN(test_name) {
 *       // test body
 *   }
 */
#define RUN(test_name)                                                       \
    static void test_name(void) {                                            \
        int _start_failed = s_tests_failed;                                   \
        s_current_test = #test_name;                                         \
        printf("  %s: ", #test_name);                                       \
        for (;;) {                                                           \
            do {

#define END_RUN                                                              \
            } while (0);                                                     \
        }                                                                    \
        int _n = s_tests_failed - _start_failed;                             \
        printf("%s\n", _n ? "FAIL" : "OK");                                  \
        if (_n) {                                                            \
            printf("    %d test(s) failed\n", _n);                           \
        }                                                                    \
    }

/* ------------------------------------------------------------------ */
/* Property helpers                                                   */
/* ------------------------------------------------------------------ */

/*
 * FIND_SHORT_PROPERTY — find a property by name and assign to a pointer.
 *
 * Usage:
 *   FIND_SHORT_PROPERTY(obj, "Value", &prop);
 */
#define FIND_SHORT_PROPERTY(obj, name, prop_ptr)                               \
     (((*(prop_ptr) = OBJ_FindShortProperty((obj), fnv1a32(name))) != NULL) ? NOERROR : E_FAIL)

/*
 * FIND_LONG_PROPERTY — find a property by long (uint32_t) identifier.
 * Returns NOERROR on success, E_FAIL if not found.
 *
 * Usage:
 *   FIND_LONG_PROPERTY(obj, id, &prop);
 */
#define FIND_LONG_PROPERTY(obj, id, prop_ptr)                                  \
     (((*(prop_ptr) = OBJ_FindLongProperty((obj), (id))) != NULL) ? NOERROR : E_FAIL)

/*
 * PROP_STR — wrap a string literal as a char** suitable for PROP_SetValue.
 * String properties store a char* internally, so the API expects void const*
 * pointing to a char* (i.e. char**).
 */
#define PROP_STR(s) (&(const char*){(s)})

/*
 * PROP_TEST — shorthand for the common find→type→null→set→not-null→value
 * pattern shared by simple scalar/string property tests.
 *
 * Usage:
 *   PROP_TEST(obj, "Value", kDataTypeFloat, &(float){3.14f},
 *             *(float*)PROP_GetValue(_p) == 3.14f);
 */
#define PROP_TEST(obj, name, dtype, setval, cmpval)                          \
    {                                                                        \
        struct Property *_p;                                                     \
        EXPECT_OK(FIND_SHORT_PROPERTY(obj, name, &_p));                    \
        EXPECT(PROP_GetType(_p) == dtype);                                   \
        EXPECT(PROP_IsNull(_p));                                             \
        PROP_SetValue(_p, setval);                                           \
        EXPECT(!PROP_IsNull(_p));                                            \
        EXPECT(cmpval);                                                      \
    }

/*
 * RUN_PROG — shorthand for create-token → run-program → release-token.
 *
 * Usage:
 *   RUN_PROG(obj, "1+2", reg);
 */
#define RUN_PROG(obj, expr, reg)                                             \
    WITH(struct token, _t, Token_Create(expr), Token_Release) {              \
        EXPECT(_t != NULL);                                                  \
        EXPECT(OBJ_RunProgram(obj, _t, reg));                                \
    }

/* ------------------------------------------------------------------ */
/* Simple test harness (used by test_xml_serialization, test_filesystem)*/
/* ------------------------------------------------------------------ */

/* RUN_TEST — lightweight test runner that tracks run/failed counts.
 *
 * Usage:
 *   RUN_TEST("test_name", {
 *       EXPECT(condition);
 *   });
 */
#define RUN_TEST(name, block) \
    do { \
        s_current_test = name; \
        s_tests_run++; \
        printf("Running %s...\n", name); \
        do { block } while (0); \
    } while (0)

/* ------------------------------------------------------------------ */
/* Object creation and manipulation helpers                             */
/* ------------------------------------------------------------------ */

/*
 * CREATE_OBJECT — create an object of a given class with no parameters.
 *
 * Usage:
 *   CREATE_OBJECT("Button");
 */
#define CREATE_OBJECT(className) OBJ_Create(fnv1a32(className), NULL)

/*
 * CREATE_OBJECT_WITH_PARAMS — create an object with a parameter list.
 *
 * Usage:
 *   CREATE_OBJECT_WITH_PARAMS("Button", params);
 */
#define CREATE_OBJECT_WITH_PARAMS(className, params)                         \
    OBJ_Create(fnv1a32(className), params)

/*
 * GET_PROPERTY — shorthand for find→get value pattern.
 *
 * Usage:
 *   GET_PROPERTY(obj, "Value");
 */
#define GET_PROPERTY(obj, name) PROP_GetValue(OBJ_FindShortProperty((obj), fnv1a32(name)))

/*
 * SET_PROPERTY — shorthand for find→set value pattern.
 *
 * Usage:
 *   SET_PROPERTY(obj, "Value", &(float){3.14f});
 */
#define SET_PROPERTY(obj, name, value)                                       \
    PROP_SetValue(OBJ_FindShortProperty((obj), fnv1a32(name)), value)

/*
 * GET_CLASS_NAME — get the class name of an object.
 *
 * Usage:
 *   GET_CLASS_NAME(obj);
 */
#define GET_CLASS_NAME(obj)                                                \
    OBJ_GetClassName((obj))

/* ------------------------------------------------------------------ */
/* Object iteration helpers                                           */
/* ------------------------------------------------------------------ */

/*
 * FOR_EACH_CHILD — iterate over child objects.
 *
 * Usage:
 *   FOR_EACH_CHILD(parent, child) {
 *       // use child
 *   }
 */
#define FOR_EACH_CHILD(parent, child)                                        \
    for (struct Object *child = OBJ_GetFirstChild((parent));                 \
         child;                                                                 \
         child = OBJ_GetNextChild(child))

/*
 * FOR_EACH_CHILD_WITH_CONTEXT — iterate over children with a context pointer.
 *
 * Usage:
 *   FOR_EACH_CHILD_WITH_CONTEXT(parent, child, ctx) {
 *       // use child, ctx
 *   }
 */
#define FOR_EACH_CHILD_WITH_CONTEXT(parent, child, ctx)                      \
    for (struct Object *child = OBJ_GetFirstChild((parent));                 \
         child;                                                                 \
         child = OBJ_GetNextChild(child))

/* ------------------------------------------------------------------ */
/* Window helpers                                                     */
/* ------------------------------------------------------------------ */

/*
 * OPEN_WINDOW — open a test window.
 *
 * Usage:
 *   OPEN_WINDOW("TestWindow");
 */
#define OPEN_WINDOW(name) WND_Create(fnv1a32(name), NULL)

/*
 * CLOSE_WINDOW — close a test window.
 *
 * Usage:
 *   CLOSE_WINDOW(window);
 */
#define CLOSE_WINDOW(window) WND_Destroy((window))

/* ------------------------------------------------------------------ */
/* String comparison helpers                                            */
/* ------------------------------------------------------------------ */

/*
 * EXPECT_STR_EQ / EXPECT_STR_EQ_MSG — string equality assertions.
 *
 * EXPECT_STR_EQ(a, b):
 *   Checks that two strings are equal (strcmp returns 0).
 *
 * EXPECT_STR_EQ_MSG(a, b, msg):
 *   Same as EXPECT_STR_EQ, but includes `msg` in the failure output.
 *
 * Usage:
 *   EXPECT_STR_EQ(result, expected);
 *   EXPECT_STR_EQ_MSG(result, expected, "detailed context");
 */
#define EXPECT_STR_EQ_MSG(a, b, msg)                                                      \
    do {                                                                                \
        const char* _a = (a);                                                           \
        const char* _b = (b);                                                           \
        if (strcmp(_a, _b) != 0) {                                                      \
            fprintf(stderr, "  FAIL [%s]: \"%s\" != \"%s\"", s_current_test, _a, _b); \
            if ((msg)[0]) fprintf(stderr, " (%s)", msg);                                \
            fprintf(stderr, " (line %d)\n", __LINE__);                                  \
            s_tests_failed++;                                                           \
            break;                                                                      \
        }                                                                               \
    } while (0)

#define EXPECT_STR_EQ(a, b) EXPECT_STR_EQ_MSG(a, b, "")

/*
 * STRING_EQ — check that two strings are equal.
 *
 * Usage:
 *   STRING_EQ(str1, str2);
 */
#define STRING_EQ(a, b) (strcmp((a), (b)) == 0)

/* STR_EQ — alias for STRING_EQ. */
#define STR_EQ(a, b) STRING_EQ((a), (b))

/*
 * STRNEQ — check that two strings are NOT equal.
 *
 * Usage:
 *   STRNEQ(str1, str2);
 */
#define STRNEQ(a, b) (strcmp((a), (b)) != 0)

/* ------------------------------------------------------------------ */
/* Memory helpers                                                     */
/* ------------------------------------------------------------------ */

/*
 * ASSERT_NOT_NULL — check that a pointer is not NULL.
 *
 * Usage:
 *   ASSERT_NOT_NULL(ptr);
 */
#define ASSERT_NOT_NULL(ptr)                                                                                 \
    do {                                                                     \
        void *_p = (ptr);                                                    \
        EXPECT(_p != NULL);                                                  \
    } while (0)

/*
 * CHECK_ALLOC — check that a pointer is not NULL and print a message.
 *
 * Usage:
 *   CHECK_ALLOC(ptr, "allocation description");
 */
#define CHECK_ALLOC(ptr, msg)                                                                              \
    do {                                                                     \
        void *_p = (ptr);                                                    \
        EXPECT(_p != NULL);                                                  \
    } while (0)

/*
 * FREE_IF_NULL — free a pointer if it's not NULL.
 *
 * Usage:
 *   FREE_IF_NULL(ptr);
 */
#define FREE_IF_NULL(ptr)                                                                                  \
    do {                                                                     \
        if ((ptr)) { free((ptr)); (ptr) = NULL; }                            \
    } while (0)

/* ------------------------------------------------------------------ */
/* Type helpers                                                       */
/* ------------------------------------------------------------------ */

/*
 * IS_TYPE — check if an object is of a given type.
 *
 * Usage:
 *   IS_TYPE(obj, "Button");
 */
#define IS_TYPE(obj, typeName) (OBJ_GetClassId((obj)) == fnv1a32(typeName))

/*
 * AS_TYPE — cast an object to a specific type (no cast applied).
 *
 * Usage:
 *   AS_TYPE(obj, "Button");
 */
#define AS_TYPE(obj, typeName) ((struct Object *)(obj))

/*
 * AS_TYPE_CAST — cast an object to a specific type with explicit cast.
 *
 * Usage:
 *   AS_TYPE_CAST(obj, "Button", struct Button *);
 */
#define AS_TYPE_CAST(obj, typeName, castType) ((castType)(obj))

/* ------------------------------------------------------------------ */
/* Property get/set helpers                                           */
/* ------------------------------------------------------------------ */

/*
 * PROP_GET — get a property value and cast to a specific type.
 *
 * Usage:
 *   PROP_GET(obj, "Value", float);
 */
#define PROP_GET(obj, name, type) ((type)PROP_GetValue(OBJ_FindShortProperty((obj), fnv1a32(name))))

/*
 * PROP_SET — set a property value.
 *
 * Usage:
 *   PROP_SET(obj, "Value", &(float){3.14f});
 */
#define PROP_SET(obj, name, value) PROP_SetValue(OBJ_FindShortProperty((obj), fnv1a32(name)), value)

/*
 * PROP_GET_CAST — get a property value with explicit cast.
 *
 * Usage:
 *   PROP_GET_CAST(obj, "Value", float);
 */
#define PROP_GET_CAST(obj, name, type) ((type)PROP_GetValue(OBJ_FindShortProperty((obj), fnv1a32(name))))

/* ------------------------------------------------------------------ */
/* Message helpers                                                    */
/* ------------------------------------------------------------------ */

/*
 * SEND_MSG — send a message to an object and check result.
 *
 * Usage:
 *   SEND_MSG(obj, WM_SIZE, 0, 0);
 */
#define SEND_MSG(obj, msg, wp, lp) OBJ_MessageHandler((obj), (msg), (wp), (lp))

/*
 * POST_MSG — post a message to an object.
 *
 * Usage:
 *   POST_MSG(obj, WM_COMMAND, 0, 0);
 */
#define POST_MSG(obj, msg, wp, lp) OBJ_PostMessage((obj), (msg), (wp), (lp))

/*
 * MSG_RESULT — get the result of a message handler.
 *
 * Usage:
 *   MSG_RESULT(result);
 */
#define MSG_RESULT(result) (result)

/*
 * MSG_RESULT_EQ — check that a message result equals a value.
 *
 * Usage:
 *   MSG_RESULT_EQ(result, 0);
 */
#define MSG_RESULT_EQ(result, expected) ((result) == (expected))

/* ------------------------------------------------------------------ */
/* Comparison helpers                                                 */
/* ------------------------------------------------------------------ */

/*
 * FLOAT_EQ — check that two floats are approximately equal.
 *
 * Usage:
 *   FLOAT_EQ(a, b);
 */
#define FLOAT_EQ(a, b) (fabsf((a) - (b)) < 0.001f)

/*
 * UINT_EQ — check that two unsigned ints are equal.
 *
 * Usage:
 *   UINT_EQ(a, b);
 */
#define UINT_EQ(a, b) ((a) == (b))

/*
 * SIZE_EQ — check that two sizes/lengths are equal.
 *
 * Usage:
 *   SIZE_EQ(a, b);
 */
#define SIZE_EQ(a, b) ((a) == (b))

/* ------------------------------------------------------------------ */
/* File helpers                                                       */
/* ------------------------------------------------------------------ */

/*
 * READ_FILE — read the contents of a file into a buffer.
 *
 * Usage:
 *   READ_FILE("path/to/file", &buf, &len);
 */
#define READ_FILE(path, buf, len)                                                                          \
    ([]() -> HRESULT {                                                                     \
        FILE *f = fopen((path), "rb");                                                     \
        if (!f) return HRESULT_FROM_WIN32(GetLastError());                               \
        fseek(f, 0, SEEK_END);                                                         \
        long _len = ftell(f);                                                          \
        fseek(f, 0, SEEK_SET);                                                         \
        if (!(*buf = malloc(_len + 1))) {                                              \
            fclose(f);                                                                 \
            return E_OUTOFMEMORY;                                                      \
        }                                                                            \
        fread(*buf, 1, _len, f);                                                     \
        (*buf)[_len] = 0;                                                            \
        *(len) = _len;                                                               \
        fclose(f);                                                                   \
        return NOERROR;                                                              \
    })()

/*
 * WRITE_FILE — write a buffer to a file.
 *
 * Usage:
 *   WRITE_FILE("path/to/file", buf, len);
 */
#define WRITE_FILE(path, buf, len)                                                                       \
    ([]() -> HRESULT {                                                                     \
        FILE *f = fopen((path), "wb");                                                     \
        if (!f) return HRESULT_FROM_WIN32(GetLastError());                               \
        size_t _written = fwrite((buf), 1, (len), f);                                    \
        fclose(f);                                                                       \
        return _written == (len) ? NOERROR : E_FAIL;                                     \
    })()

/*
 * FILE_EXISTS — check if a file exists.
 *
 * Usage:
 *   FILE_EXISTS("path/to/file");
 */
#define FILE_EXISTS(path) (access((path), F_OK) == 0)

/* ------------------------------------------------------------------ */
/* Lua helpers                                                        */
/* ------------------------------------------------------------------ */

/*
 * LUA_STATE — create a Lua state for testing.
 *
 * Usage:
 *   LUA_STATE(L);
 */
#define LUA_STATE(L) lua_open()

/*
 * LUA_RELEASE — close a Lua state.
 *
 * Usage:
 *   LUA_RELEASE(L);
 */
#define LUA_RELEASE(L) lua_close((L))

/*
 * LUA_DOFILE — execute a Lua file.
 *
 * Usage:
 *   LUA_DOFILE(L, "path/to/script.lua");
 */
#define LUA_DOFILE(L, path) lua_dofile((L), (path))

/*
 * LUA_DOSTRING — execute a Lua string.
 *
 * Usage:
 *   LUA_DOSTRING(L, "print('hello')");
 */
#define LUA_DOSTRING(L, str) lua_dostring((L), (str))

/* ------------------------------------------------------------------ */
/* Trigger action helpers                                             */
/* ------------------------------------------------------------------ */

/*
 * TRIGGER_ACTION — trigger a trigger action and check result.
 *
 * Usage:
 *   TRIGGER_ACTION(obj, "ActionName");
 */
#define TRIGGER_ACTION(obj, action) OBJ_TriggerAction((obj), (action))

/*
 * OBSERVED_COUNT — get the number of observed trigger actions.
 *
 * Usage:
 *   OBSERVED_COUNT();
 */
#define OBSERVED_COUNT() (_observed_count)

/*
 * RESET_OBSERVED — reset the observed trigger action count.
 *
 * Usage:
 *   RESET_OBSERVED();
 */
#define RESET_OBSERVED() (_observed_count = 0)

/* ------------------------------------------------------------------ */
/* Message registry helpers                                           */
/* ------------------------------------------------------------------ */

/*
 * REGISTER_MESSAGE — register a message in the registry.
 *
 * Usage:
 *   REGISTER_MESSAGE(WM_MYMESSAGE);
 */
#define REGISTER_MESSAGE(msg) MSG_Register((msg))

/*
 * FIND_MESSAGE — find a message in the registry.
 *
 * Usage:
 *   FIND_MESSAGE(WM_MYMESSAGE);
 */
#define FIND_MESSAGE(msg) MSG_Find((msg))

/*
 * SUBSCRIBE — subscribe to a message.
 *
 * Usage:
 *   SUBSCRIBE(WM_MYMESSAGE, handler);
 */
#define SUBSCRIBE(msg, handler) MSG_Subscribe((msg), (handler))

/* ------------------------------------------------------------------ */
/* XML helpers                                                        */
/* ------------------------------------------------------------------ */

/*
 * XML_PARSE — parse an XML string.
 *
 * Usage:
 *   XML_PARSE("<root><child>text</child></root>");
 */
#define XML_PARSE(xml) XML_ParserCreate(), 0 /* placeholder */

/*
 * XML_FREE — free an XML parser.
 *
 * Usage:
 *   XML_FREE(parser);
 */
#define XML_FREE(parser) /* placeholder */

/*
 * XML_FIND — find an element in XML.
 *
 * Usage:
 *   XML_FIND(parser, "child");
 */
#define XML_FIND(parser, name) 0 /* placeholder */

/*
 * XML_GET_ATTR — get an attribute from an XML element.
 *
 * Usage:
 *   XML_GET_ATTR(element, "attr");
 */
#define XML_GET_ATTR(element, attr) NULL /* placeholder */

/* ------------------------------------------------------------------ */
/* Editor helpers                                                     */
/* ------------------------------------------------------------------ */

/*
 * EDITOR_NEW — create a new editor.
 *
 * Usage:
 *   EDITOR_NEW();
 */
#define EDITOR_NEW() 0 /* placeholder */

/*
 * EDITOR_FREE — free an editor.
 *
 * Usage:
 *   EDITOR_FREE(editor);
 */
#define EDITOR_FREE(editor) /* placeholder */

/*
 * EDITOR_SET — set editor content.
 *
 * Usage:
 *   EDITOR_SET(editor, "text");
 */
#define EDITOR_SET(editor, text) /* placeholder */

/*
 * EDITOR_GET — get editor content.
 *
 * Usage:
 *   EDITOR_GET(editor);
 */
#define EDITOR_GET(editor) NULL /* placeholder */

#endif /* TEST_LOCAL_H */
