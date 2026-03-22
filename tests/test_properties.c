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
 *   - Memory leak tests (compiled with -DTEST_MEMORY): string property lifecycle,
 *     PROP_Clear, OBJ_ReleaseProperties, Token_Create/Release, and attach+update
 *
 * Compiled and linked against liborca.so via the `test-properties` Makefile
 * target (depends on `buildlib`).
 *
 * Build with -DTEST_MEMORY to enable malloc/free/strdup interception and
 * per-test allocation tracking.
 */

/* ------------------------------------------------------------------ */
/* TEST_MEMORY: malloc/free/strdup interception for leak detection     */
/*                                                                     */
/* When -DTEST_MEMORY is defined, this file provides its own malloc,   */
/* calloc, realloc, free, and strdup symbols.  Because the test binary */
/* is linked dynamically against liborca.so, the dynamic linker will   */
/* prefer the definitions in the main executable, so liborca's         */
/* allocations are also counted.                                       */
/*                                                                     */
/* A small static bootstrap buffer handles allocations that arrive     */
/* before mem_init() (called at the top of main) resolves the real     */
/* libc functions via RTLD_NEXT.  Bootstrap pointers are silently      */
/* ignored by free().                                                  */
/* ------------------------------------------------------------------ */
#if TEST_MEMORY
#include <dlfcn.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>  /* memset, memcpy, strlen — provided by libc directly */

/* Static bootstrap buffer for very early allocations (before mem_init).
 * 128 KB is enough for library initialization of all linked modules. */
#define BOOTSTRAP_BUF_SIZE (128 * 1024)
static char   s_bootstrap_buf[BOOTSTRAP_BUF_SIZE];
static size_t s_bootstrap_pos = 0;

static int is_bootstrap_ptr(const void *ptr) {
    return (const char *)ptr >= s_bootstrap_buf &&
           (const char *)ptr <  s_bootstrap_buf + BOOTSTRAP_BUF_SIZE;
}

static void *bootstrap_alloc(size_t size) {
    size = (size + 15u) & ~(size_t)15u; /* 16-byte alignment */
    if (s_bootstrap_pos + size > BOOTSTRAP_BUF_SIZE) return NULL;
    void *p = s_bootstrap_buf + s_bootstrap_pos;
    s_bootstrap_pos += size;
    return p;
}

/* Real allocator function pointers, set by mem_init(). */
typedef void *(*malloc_fn_t)(size_t);
typedef void *(*calloc_fn_t)(size_t, size_t);
typedef void *(*realloc_fn_t)(void *, size_t);
typedef void  (*free_fn_t)(void *);

static malloc_fn_t  s_real_malloc  = NULL;
static calloc_fn_t  s_real_calloc  = NULL;
static realloc_fn_t s_real_realloc = NULL;
static free_fn_t    s_real_free    = NULL;

/* Live allocation counter: incremented by malloc/calloc/strdup,
 * decremented by free.  Snapshot before/after a test to detect leaks. */
static volatile long s_alloc_count = 0;
static volatile long s_alloc_total = 0;  /* monotonic total, for diagnostics */

/* mem_init() must be called at the start of main(), after which all
 * allocations from both the test binary and liborca.so are tracked. */
static void mem_init(void) {
    s_real_malloc  = (malloc_fn_t) dlsym(RTLD_NEXT, "malloc");
    s_real_calloc  = (calloc_fn_t) dlsym(RTLD_NEXT, "calloc");
    s_real_realloc = (realloc_fn_t)dlsym(RTLD_NEXT, "realloc");
    s_real_free    = (free_fn_t)   dlsym(RTLD_NEXT, "free");
    /* Reset counter: any bootstrap allocations that survive to this point
     * live in the static buffer and will never be freed through s_real_free,
     * so they don't affect future delta checks. */
    s_alloc_count = 0;
    s_alloc_total = 0;
}

/* ---- replacement allocators ---- */

void *malloc(size_t size) {
    if (!s_real_malloc) return bootstrap_alloc(size);
    void *p = s_real_malloc(size);
    if (p) {
        __atomic_fetch_add(&s_alloc_count, 1, __ATOMIC_RELAXED);
        __atomic_fetch_add(&s_alloc_total, 1, __ATOMIC_RELAXED);
    }
    return p;
}

void *calloc(size_t n, size_t size) {
    if (!s_real_calloc) {
        void *p = bootstrap_alloc(n * size);
        if (p) memset(p, 0, n * size);
        return p;
    }
    void *p = s_real_calloc(n, size);
    if (p) {
        __atomic_fetch_add(&s_alloc_count, 1, __ATOMIC_RELAXED);
        __atomic_fetch_add(&s_alloc_total, 1, __ATOMIC_RELAXED);
    }
    return p;
}

void *realloc(void *ptr, size_t size) {
    if (is_bootstrap_ptr(ptr)) {
        /* Bootstrap pointers can't be passed to real realloc; copy to heap.
         * The old allocation size is unknown, so we copy up to the bootstrap
         * buffer's used capacity as a safe upper bound.  In practice, realloc
         * of bootstrap pointers does not occur in this test binary. */
        if (!s_real_malloc) return bootstrap_alloc(size);
        void *np = s_real_malloc(size);
        if (np) {
            size_t old_cap = s_bootstrap_pos; /* upper bound on old size */
            memcpy(np, ptr, old_cap < size ? old_cap : size);
            __atomic_fetch_add(&s_alloc_count, 1, __ATOMIC_RELAXED);
            __atomic_fetch_add(&s_alloc_total, 1, __ATOMIC_RELAXED);
        }
        return np;
    }
    if (!s_real_realloc) return NULL;
    void *p = s_real_realloc(ptr, size);
    /* realloc with non-NULL ptr replaces in-place: count unchanged. */
    if (!ptr && p) {
        __atomic_fetch_add(&s_alloc_count, 1, __ATOMIC_RELAXED);
        __atomic_fetch_add(&s_alloc_total, 1, __ATOMIC_RELAXED);
    }
    return p;
}

void free(void *ptr) {
    if (!ptr) return;
    if (is_bootstrap_ptr(ptr)) return; /* bootstrap: silently ignore */
    if (!s_real_free) return;
    s_real_free(ptr);
    __atomic_fetch_sub(&s_alloc_count, 1, __ATOMIC_RELAXED);
}

/* Note: strdup is NOT overridden here — it internally calls malloc, so
 * strdup allocations are automatically counted through our malloc wrapper. */

/* ---- snapshot / check macros ---- */

#define MEM_SNAPSHOT()   ((long)s_alloc_count)

#define MEM_CHECK_LEAK(snap, label)                                          \
    do {                                                                     \
        long _now  = (long)s_alloc_count;                                    \
        long _snap = (snap);                                                 \
        if (_now > _snap) {                                                 \
            fprintf(stderr,                                                  \
                    "  LEAK [%s]: %ld outstanding allocation(s)"             \
                    " (before=%ld after=%ld)\n",                             \
                    (label), _now - _snap, _snap, _now);                     \
            s_tests_failed++;                                                \
        }                                                                    \
    } while (0)

#else /* !TEST_MEMORY */

static void mem_init(void) {}
#define MEM_SNAPSHOT()             (0L)
#define MEM_CHECK_LEAK(snap, label) ((void)(snap))

#endif /* TEST_MEMORY */

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

#define TEST_BEGIN(name) 

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

/* Declare a memory-leak test function.  Pair with MEMLEAK_TEST_END.
 * Takes a snapshot before the body and checks for outstanding
 * allocations after it.  The macro opens an inner scope block `{` that
 * MEMLEAK_TEST_END closes with `}` before running MEM_CHECK_LEAK — this
 * scoping ensures all local variables (including xmlWith-managed objects)
 * are destroyed before the leak check fires.  Usage:
 *   MEMLEAK_TEST(test_memleak_foo, "memleak: ...")
 *       // body — use xmlWith for object/token lifetimes
 *   MEMLEAK_TEST_END
 */
#define MEMLEAK_TEST(fn, label) \
    static void fn(void) { \
        TEST_BEGIN(label); \
        long snap = MEM_SNAPSHOT(); \
        {

#define MEMLEAK_TEST_END \
        } \
        MEM_CHECK_LEAK(snap, s_current_test); \
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
/* FullIdentifiers use the plain property name (e.g. fnv1a32("Count")) so  */
/* that bindings like "{./Count}" resolve via OBJ_FindPropertyByPath.       */
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
     * FullIdentifier == fnv1a32("PropName") matches what
     * OBJ_FindPropertyByPath resolves for a binding like "{./PropName}":
     * the "./" prefix is consumed as "same object", leaving "PropName"
     * as the leaf passed to PROP_FindByFullName.
     */
    memset(&s_rtPropCount, 0, sizeof(s_rtPropCount));
    strncpy(s_rtPropCount.Name, "Count", sizeof(s_rtPropCount.Name) - 1);
    strncpy(s_rtPropCount.Key,  "Count", sizeof(s_rtPropCount.Key)  - 1);
    s_rtPropCount.DataType        = kDataTypeInt;
    s_rtPropCount.DataSize        = sizeof(int);
    s_rtPropCount.ShortIdentifier = fnv1a32("Count");
    s_rtPropCount.FullIdentifier  = fnv1a32("Count");
    s_rtPropCount.Offset          = offsetof(struct RTComp, Count);

    memset(&s_rtPropValue, 0, sizeof(s_rtPropValue));
    strncpy(s_rtPropValue.Name, "Value", sizeof(s_rtPropValue.Name) - 1);
    strncpy(s_rtPropValue.Key,  "Value", sizeof(s_rtPropValue.Key)  - 1);
    s_rtPropValue.DataType        = kDataTypeFloat;
    s_rtPropValue.DataSize        = sizeof(float);
    s_rtPropValue.ShortIdentifier = fnv1a32("Value");
    s_rtPropValue.FullIdentifier  = fnv1a32("Value");
    s_rtPropValue.Offset          = offsetof(struct RTComp, Value);

    memset(&s_rtPropLabel, 0, sizeof(s_rtPropLabel));
    strncpy(s_rtPropLabel.Name, "Label", sizeof(s_rtPropLabel.Name) - 1);
    strncpy(s_rtPropLabel.Key,  "Label", sizeof(s_rtPropLabel.Key)  - 1);
    s_rtPropLabel.DataType        = kDataTypeString;
    s_rtPropLabel.DataSize        = sizeof(const char*);
    s_rtPropLabel.ShortIdentifier = fnv1a32("Label");
    s_rtPropLabel.FullIdentifier  = fnv1a32("Label");
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
 * OBJ_ReleaseProperties, frees all component blocks, and finally
 * frees the object itself.
 * Does NOT call OBJ_Release (which needs a Lua state and message dispatch).
 */
static void destroy_object(lpObject_t obj)
{
    if (!obj) return;
    OBJ_ReleaseProperties(obj);
    /* Walk the component linked list and free each block.
     * struct component has `next` as its first field (see component.c),
     * so we can walk without knowing the full struct layout. */
    void *cmp = *(void **)OBJ_GetObjectComponent(obj, kCompComponents);
    while (cmp) {
        void *next = *(void **)cmp; /* next pointer is first field */
        free(cmp);
        cmp = next;
    }
    free(obj);
}

/* ------------------------------------------------------------------ */
/* Tests                                                               */
/* ------------------------------------------------------------------ */

static void test_int_property(void)
{
    TEST_BEGIN("int property set/get");
    xmlWith(struct Object, obj, make_object(), destroy_object) {
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
    }
}

static void test_float_property(void)
{
    TEST_BEGIN("float property set/get");
    xmlWith(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        HRESULT hr = OBJ_FindShortProperty(obj, "Value", &prop);
        EXPECT(SUCCEEDED(hr));
        EXPECT(PROP_GetType(prop) == kDataTypeFloat);
        EXPECT(PROP_IsNull(prop));

        float f = 3.14f;
        PROP_SetValue(prop, &f);
        EXPECT(!PROP_IsNull(prop));
        EXPECT(*(float*)PROP_GetValue(prop) == 3.14f);
    }
}

static void test_bool_property(void)
{
    TEST_BEGIN("bool property set/get");
    xmlWith(struct Object, obj, make_object(), destroy_object) {
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
    }
}

static void test_string_property_basic(void)
{
    TEST_BEGIN("string property set/get");
    xmlWith(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        OBJ_FindShortProperty(obj, "Label", &prop);
        EXPECT(prop != NULL);
        EXPECT(PROP_GetType(prop) == kDataTypeString);
        EXPECT(PROP_IsNull(prop));

        PROP_SetValue(prop, "hello");
        EXPECT(!PROP_IsNull(prop));
        EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "hello");
    }
}

static void test_string_property_reassign(void)
{
    TEST_BEGIN("string property multiple reassignments");
    xmlWith(struct Object, obj, make_object(), destroy_object) {
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
    }
}

static void test_string_property_clear(void)
{
    TEST_BEGIN("string property PROP_Clear");
    xmlWith(struct Object, obj, make_object(), destroy_object) {
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
    }
}

static void test_string_property_clear_without_set(void)
{
    TEST_BEGIN("string property PROP_Clear without prior set");
    xmlWith(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        OBJ_FindShortProperty(obj, "Label", &prop);

        /* Clearing a never-set string property must not crash */
        PROP_Clear(prop);
        EXPECT(PROP_IsNull(prop));
    }
}

static void test_release_properties_frees_strings(void)
{
    TEST_BEGIN("OBJ_ReleaseProperties frees string memory");
    xmlWith(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        OBJ_FindShortProperty(obj, "Label", &prop);
        PROP_SetValue(prop, "will be freed");
        /* destroy_object (called by xmlWith) calls OBJ_ReleaseProperties which must
           free the strdup'd string without crashing. */
    }
}

static void test_set_property_value_api(void)
{
    TEST_BEGIN("OBJ_SetPropertyValue (demand creation)");
    xmlWith(struct Object, obj, make_object(), destroy_object) {
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
    }
}

static void test_property_state_string(void)
{
    TEST_BEGIN("per-state string property set/clear");
    xmlWith(struct Object, obj, make_object(), destroy_object) {
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
    }
}

static void test_struct_property(void)
{
    TEST_BEGIN("struct (vec2) property set/get");
    xmlWith(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        OBJ_FindShortProperty(obj, "Position", &prop);
        EXPECT(prop != NULL);
        EXPECT(PROP_GetType(prop) == kDataTypeStruct);

        float pos[2] = {10.0f, 20.0f};
        PROP_SetValue(prop, pos);
        float *result = (float*)PROP_GetValue(prop);
        EXPECT(result[0] == 10.0f);
        EXPECT(result[1] == 20.0f);
    }
}

static void test_find_property_unknown(void)
{
    TEST_BEGIN("OBJ_FindShortProperty for unknown property returns error");
    xmlWith(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop = NULL;
        HRESULT hr = OBJ_FindShortProperty(obj, "NonExistentProp", &prop);
        EXPECT(FAILED(hr));
    }
}

static void test_multiple_properties_independent(void)
{
    TEST_BEGIN("multiple properties on same object are independent");
    xmlWith(struct Object, obj, make_object(), destroy_object) {
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
    }
}

static void test_string_empty(void)
{
    TEST_BEGIN("string property with empty string");
    xmlWith(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        OBJ_FindShortProperty(obj, "Label", &prop);

        PROP_SetValue(prop, "");
        EXPECT(!PROP_IsNull(prop));
        EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "");
    }
}

static void test_release_without_string_set(void)
{
    TEST_BEGIN("OBJ_ReleaseProperties with no string set");
    xmlWith(struct Object, obj, make_object(), destroy_object) {
        /* Only set a non-string property */
        lpProperty_t prop;
        OBJ_FindShortProperty(obj, "Count", &prop);
        int val = 1;
        PROP_SetValue(prop, &val);
        /* destroy_object (called by xmlWith) calls OBJ_ReleaseProperties which must
           not crash even though the Label string property was never set */
    }
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
    xmlWith(struct Object, obj, make_rt_object(), destroy_object) {
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
    }
}

static void test_runtime_run_float_constant(void)
{
    TEST_BEGIN("OBJ_RunProgram: float constant evaluates to float");
    xmlWith(struct Object, obj, make_rt_object(), destroy_object) {
        struct token *prog = Token_Create("2.5");
        EXPECT(prog != NULL);
        if (prog) {
            struct vm_register r = {0};
            bool_t ok = OBJ_RunProgram(obj, prog, &r);
            EXPECT(ok);
            EXPECT(r.value[0] == 2.5f);
            Token_Release(prog);
        }
    }
}

static void test_runtime_run_string_constant(void)
{
    TEST_BEGIN("OBJ_RunProgram: string literal evaluates to string");
    xmlWith(struct Object, obj, make_rt_object(), destroy_object) {
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
    }
}

static void test_runtime_run_arithmetic(void)
{
    TEST_BEGIN("OBJ_RunProgram: ADD(10, 5) == 15");
    xmlWith(struct Object, obj, make_rt_object(), destroy_object) {
        struct token *prog = Token_Create("ADD(10, 5)");
        EXPECT(prog != NULL);
        if (prog) {
            struct vm_register r = {0};
            bool_t ok = OBJ_RunProgram(obj, prog, &r);
            EXPECT(ok);
            EXPECT((int)r.value[0] == 15);
            Token_Release(prog);
        }
    }
}

static void test_runtime_import_int(void)
{
    TEST_BEGIN("PROP_Import: int register into int property");
    xmlWith(struct Object, obj, make_rt_object(), destroy_object) {
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
    }
}

static void test_runtime_import_float(void)
{
    TEST_BEGIN("PROP_Import: float register into float property");
    xmlWith(struct Object, obj, make_rt_object(), destroy_object) {
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
    }
}

static void test_runtime_import_string(void)
{
    TEST_BEGIN("PROP_Import: string register into string property");
    xmlWith(struct Object, obj, make_rt_object(), destroy_object) {
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
    }
}

static void test_runtime_attach_and_update_int(void)
{
    TEST_BEGIN("PROP_AttachProgram + PROP_Update: constant int program");
    xmlWith(struct Object, obj, make_rt_object(), destroy_object) {
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
    }
}

static void test_runtime_attach_and_update_string(void)
{
    TEST_BEGIN("PROP_AttachProgram + PROP_Update: constant string program");
    xmlWith(struct Object, obj, make_rt_object(), destroy_object) {
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
static void test_runtime_property_reference(void)
{
    TEST_BEGIN("p_runtime: property reference via {./PropName} binding");
    xmlWith(struct Object, obj, make_rt_object(), destroy_object) {
        lpProperty_t propCount, propValue;
        OBJ_FindShortProperty(obj, "Count", &propCount);
        OBJ_FindShortProperty(obj, "Value", &propValue);
        EXPECT(propCount != NULL);
        EXPECT(propValue != NULL);
        if (propCount && propValue) {
            /* Set the source property */
            float v = 5.0f;
            PROP_SetValue(propValue, &v);
            EXPECT(!PROP_IsNull(propValue));

            /* Compile a program that reads {./Value} and binds it to Count */
            struct token *prog = Token_Create("{./Value}");
            EXPECT(prog != NULL);
            if (prog) {
                PROP_AttachProgram(propCount, kPropertyAttributeWholeProperty, prog, "{./Value}");
                EXPECT(PROP_HasProgram(propCount));
                core.frame++;
                bool_t ok = PROP_Update(propCount);
                EXPECT(ok);
                EXPECT(!PROP_IsNull(propCount));
                /* Count is int, Value is float 5.0 → expect 5 */
                EXPECT(*(int*)PROP_GetValue(propCount) == 5);
            }
        }
    }
}

/*
 * String concatenation: ADD("foo", "bar") should produce "foobar"
 * and a program that writes it to a string property should work.
 */
static void test_runtime_string_concat_program(void)
{
    TEST_BEGIN("p_runtime: string concat ADD(\"foo\", \"bar\")");
    xmlWith(struct Object, obj, make_rt_object(), destroy_object) {
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
    }
}

/* ------------------------------------------------------------------ */
/* Memory-leak tests (active when compiled with -DTEST_MEMORY)         */
/* Each test takes a snapshot before and checks for leaks after.       */
/* ------------------------------------------------------------------ */

/* Leak test: set a string property then destroy the object — the
 * strdup'd string must be freed by OBJ_ReleaseProperties. */
MEMLEAK_TEST(test_memleak_string_set_release, "memleak: string property set then object released")
    xmlWith(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        OBJ_FindShortProperty(obj, "Label", &prop);
        if (prop) PROP_SetValue(prop, "hello leak test");
    }
MEMLEAK_TEST_END

/* Leak test: reassign a string multiple times — old strings must be freed. */
MEMLEAK_TEST(test_memleak_string_multiple_sets, "memleak: string property multiple reassignments")
    xmlWith(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        OBJ_FindShortProperty(obj, "Label", &prop);
        if (prop) {
            PROP_SetValue(prop, "first");
            PROP_SetValue(prop, "second");   /* frees "first" */
            PROP_SetValue(prop, "third");    /* frees "second" */
        }
    }                                        /* destroy_object frees "third" */
MEMLEAK_TEST_END

/* Leak test: PROP_Clear must free the heap string and leave no leak. */
MEMLEAK_TEST(test_memleak_prop_clear, "memleak: PROP_Clear frees string")
    xmlWith(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        OBJ_FindShortProperty(obj, "Label", &prop);
        if (prop) {
            PROP_SetValue(prop, "to be cleared");
            PROP_Clear(prop);
        }
    }
MEMLEAK_TEST_END

/* Leak test: PROP_Clear on a never-set string must not crash or leak. */
MEMLEAK_TEST(test_memleak_prop_clear_never_set, "memleak: PROP_Clear on never-set string property")
    xmlWith(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        OBJ_FindShortProperty(obj, "Label", &prop);
        if (prop) PROP_Clear(prop);
    }
MEMLEAK_TEST_END

/* Leak test: set/clear/reset one string property and set a second scalar
 * property, then destroy — no heap string or Property struct leaks. */
MEMLEAK_TEST(test_memleak_multiple_string_props, "memleak: multiple property set/clear/release no leak")
    xmlWith(struct Object, obj, make_object(), destroy_object) {
        /* String property: set, reassign, clear, set again */
        OBJ_SetPropertyValue(obj, "Label", "alpha");
        lpProperty_t strprop;
        OBJ_FindShortProperty(obj, "Label", &strprop);
        if (strprop) {
            PROP_SetValue(strprop, "alpha2");
            PROP_Clear(strprop);
            PROP_SetValue(strprop, "final");
        }
        /* Scalar property: set (no heap allocation for value) */
        OBJ_SetPropertyValue(obj, "Count", &(int){7});
    }
MEMLEAK_TEST_END

/* Leak test: Token_Create / Token_Release must not leak. */
MEMLEAK_TEST(test_memleak_token_create_release, "memleak: Token_Create/Token_Release no leak")
    /* Create a token and let xmlWith auto-release it — body is empty because
     * the test only verifies the create/release lifecycle has no leaks. */
    xmlWith(struct token, prog, Token_Create("ADD(1, 2)"), Token_Release) {
        (void)prog;
    }
MEMLEAK_TEST_END

/* Leak test: Token_Create for a string literal must not leak. */
MEMLEAK_TEST(test_memleak_token_string_literal, "memleak: Token_Create string literal no leak")
    /* Create a string-literal token and let xmlWith auto-release it. */
    xmlWith(struct token, prog, Token_Create("\"test string\""), Token_Release) {
        (void)prog;
    }
MEMLEAK_TEST_END

/* Leak test: OBJ_RunProgram with a string must not leak VM temporaries. */
MEMLEAK_TEST(test_memleak_run_string_program, "memleak: OBJ_RunProgram string constant no leak")
    xmlWith(struct Object, obj, make_rt_object(), destroy_object) {
        xmlWith(struct token, prog, Token_Create("\"vm string\""), Token_Release) {
            struct vm_register r = {0};
            OBJ_RunProgram(obj, prog, &r);
        }
    }
MEMLEAK_TEST_END

/* Leak test: PROP_AttachProgram + PROP_Update + object destroy, no leak.
 * After PROP_AttachProgram the property owns the token; destroy_object
 * calls OBJ_ReleaseProperties which runs Token_Release + frees code strings. */
MEMLEAK_TEST(test_memleak_attach_update_release, "memleak: PROP_AttachProgram+Update+destroy_object no leak")
    xmlWith(struct Object, obj, make_rt_object(), destroy_object) {
        lpProperty_t prop;
        OBJ_FindShortProperty(obj, "Label", &prop);
        if (prop) {
            struct token *prog = Token_Create("\"attached\"");
            if (prog) {
                /* Property takes ownership of prog and strdup's the code. */
                PROP_AttachProgram(prop, kPropertyAttributeWholeProperty,
                                   prog, "\"attached\"");
                core.frame++;
                PROP_Update(prop);
                /* destroy_object → OBJ_ReleaseProperties → Token_Release +
                   free(programSources) + free(heap string value). */
            }
        }
    }
MEMLEAK_TEST_END

/* Leak test: set int, float, bool properties — no heap allocations expected. */
MEMLEAK_TEST(test_memleak_scalar_properties, "memleak: scalar (int/float/bool) properties no leak")
    xmlWith(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t pi, pf, pb;
        OBJ_FindShortProperty(obj, "Count",  &pi);
        OBJ_FindShortProperty(obj, "Value",  &pf);
        OBJ_FindShortProperty(obj, "Active", &pb);
        int   iv = 99;
        float fv = 1.5f;
        bool_t bv = true;
        if (pi) PROP_SetValue(pi, &iv);
        if (pf) PROP_SetValue(pf, &fv);
        if (pb) PROP_SetValue(pb, &bv);
    }
MEMLEAK_TEST_END

/* Leak test: property reference program (reads one prop, writes another). */
MEMLEAK_TEST(test_memleak_property_reference_program, "memleak: property reference program no leak")
    xmlWith(struct Object, obj, make_rt_object(), destroy_object) {
        lpProperty_t propCount, propValue;
        OBJ_FindShortProperty(obj, "Count", &propCount);
        OBJ_FindShortProperty(obj, "Value", &propValue);
        if (propCount && propValue) {
            float v = 7.0f;
            PROP_SetValue(propValue, &v);
            struct token *prog = Token_Create("{./Value}");
            if (prog) {
                /* Property owns prog after this call. */
                PROP_AttachProgram(propCount, kPropertyAttributeWholeProperty,
                                   prog, "{./Value}");
                core.frame++;
                PROP_Update(propCount);
                /* destroy_object will call Token_Release on prog. */
            }
        }
    }
MEMLEAK_TEST_END

/* ------------------------------------------------------------------ */

#define RUN(func) \
s_current_test = #func; \
s_tests_run++ \
puts("Running " #func "..."); \
func()

int main(void)
{
    mem_init(); /* must be first: resolves real malloc/free via RTLD_NEXT */

    printf("ORCA property system tests\n");
    printf("==========================\n");

    register_test_class();
    register_runtime_class();

    RUN(test_int_property);
    RUN(test_float_property);
    RUN(test_bool_property);
    RUN(test_string_property_basic);
    RUN(test_string_property_reassign);
    RUN(test_string_property_clear);
    RUN(test_string_property_clear_without_set);
    RUN(test_release_properties_frees_strings);
    RUN(test_set_property_value_api);
    RUN(test_property_state_string);
    RUN(test_struct_property);
    RUN(test_find_property_unknown);
    RUN(test_multiple_properties_independent);
    RUN(test_string_empty);
    RUN(test_release_without_string_set);

    RUN(test_runtime_token_create_int);
    RUN(test_runtime_token_create_float);
    RUN(test_runtime_token_create_string);
    RUN(test_runtime_token_create_arithmetic);
    RUN(test_runtime_run_int_constant);
    RUN(test_runtime_run_float_constant);
    RUN(test_runtime_run_string_constant);
    RUN(test_runtime_run_arithmetic);
    RUN(test_runtime_import_int);
    RUN(test_runtime_import_float);
    RUN(test_runtime_import_string);
    RUN(test_runtime_attach_and_update_int);
    RUN(test_runtime_attach_and_update_string);
    RUN(test_runtime_property_reference);
    RUN(test_runtime_string_concat_program);

    RUN(test_memleak_string_set_release);
    RUN(test_memleak_string_multiple_sets);
    RUN(test_memleak_prop_clear);
    RUN(test_memleak_prop_clear_never_set);
    RUN(test_memleak_multiple_string_props);
    RUN(test_memleak_token_create_release);
    RUN(test_memleak_token_string_literal);
    RUN(test_memleak_run_string_program);
    RUN(test_memleak_attach_update_release);
    RUN(test_memleak_scalar_properties);
    RUN(test_memleak_property_reference_program);

#if TEST_MEMORY
    printf("\nMemory tracking: %ld total allocation(s) made, "
           "%ld outstanding at exit\n",
           s_alloc_total, s_alloc_count);
#endif

    printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
    return s_tests_failed == 0 ? 0 : 1;
}
