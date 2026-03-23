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

/* Small bootstrap buffer used *only* while dlsym() resolves the real
 * allocator pointers.  dlsym itself calls calloc() once internally (on
 * glibc/macOS), so we need enough space for that one allocation.
 * All other pre-main allocations (GnuTLS, libEGL, …) go to the real
 * libc once the lazy initialiser fires. */
#define BOOTSTRAP_BUF_SIZE 1024
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

/* Real allocator function pointers resolved lazily on first use. */
typedef void *(*malloc_fn_t)(size_t);
typedef void *(*calloc_fn_t)(size_t, size_t);
typedef void *(*realloc_fn_t)(void *, size_t);
typedef void  (*free_fn_t)(void *);

static malloc_fn_t  s_real_malloc  = NULL;
static calloc_fn_t  s_real_calloc  = NULL;
static realloc_fn_t s_real_realloc = NULL;
static free_fn_t    s_real_free    = NULL;

/* Recursion guard: set to 1 while dlsym() is running so that the
 * calloc() invoked internally by dlsym uses the bootstrap buffer
 * instead of trying to re-enter init_alloc_funcs(). */
static int s_init_in_progress = 0;

/* Tracking is enabled only after mem_init() is called from main().
 * Allocations that happen before that point (library constructors,
 * dlsym overhead) are served by the real libc but NOT counted. */
static int s_tracking_active = 0;

/* Resolve the real allocator function pointers via dlsym(RTLD_NEXT).
 * Safe to call from any intercepted allocator; the recursion guard
 * prevents re-entrancy.  Aborts if dlsym fails (indicates a broken
 * runtime environment where tests cannot run meaningfully). */
static void init_alloc_funcs(void) {
    s_init_in_progress = 1;
    s_real_malloc  = (malloc_fn_t)  dlsym(RTLD_NEXT, "malloc");
    s_real_calloc  = (calloc_fn_t)  dlsym(RTLD_NEXT, "calloc");
    s_real_realloc = (realloc_fn_t) dlsym(RTLD_NEXT, "realloc");
    s_real_free    = (free_fn_t)    dlsym(RTLD_NEXT, "free");
    s_init_in_progress = 0;
    if (!s_real_malloc || !s_real_calloc || !s_real_realloc || !s_real_free) {
        /* Cannot locate libc allocators — tests cannot run. */
        abort();
    }
}

/* Live allocation counter: incremented by malloc/calloc, decremented
 * by free.  Only active after mem_init() enables tracking. */
static volatile long s_alloc_count = 0;
static volatile long s_alloc_total = 0;  /* monotonic total, for diagnostics */

/* mem_init() must be called at the start of main(). From this point
 * onward all allocations from both the test binary and liborca.so are
 * tracked.  Allocations that occurred before mem_init() (library
 * constructors, dlsym overhead) are NOT counted. */
static void mem_init(void) {
    if (!s_real_malloc) init_alloc_funcs();
    s_alloc_count = 0;
    s_alloc_total = 0;
    s_tracking_active = 1;
}

/* ---- replacement allocators ---- */

void *malloc(size_t size) {
    if (!s_real_malloc) {
        if (s_init_in_progress) return bootstrap_alloc(size);
        init_alloc_funcs();
    }
    void *p = s_real_malloc(size);
    if (p && s_tracking_active) {
        __atomic_fetch_add(&s_alloc_count, 1, __ATOMIC_RELAXED);
        __atomic_fetch_add(&s_alloc_total, 1, __ATOMIC_RELAXED);
    }
    return p;
}

void *calloc(size_t n, size_t size) {
    if (!s_real_calloc) {
        if (s_init_in_progress) {
            /* Called from dlsym — use the bootstrap buffer (zeroed by BSS). */
            void *p = bootstrap_alloc(n * size);
            return p;
        }
        init_alloc_funcs();
    }
    void *p = s_real_calloc(n, size);
    if (p && s_tracking_active) {
        __atomic_fetch_add(&s_alloc_count, 1, __ATOMIC_RELAXED);
        __atomic_fetch_add(&s_alloc_total, 1, __ATOMIC_RELAXED);
    }
    return p;
}

void *realloc(void *ptr, size_t size) {
    if (is_bootstrap_ptr(ptr)) {
        /* Bootstrap pointers cannot be passed to the real realloc.
         * Copy to heap; in practice this path is never taken. */
        if (!s_real_malloc) {
            if (s_init_in_progress) return bootstrap_alloc(size);
            init_alloc_funcs();
        }
        void *np = s_real_malloc(size);
        if (np) {
            size_t old_cap = s_bootstrap_pos;
            memcpy(np, ptr, old_cap < size ? old_cap : size);
            if (s_tracking_active) {
                __atomic_fetch_add(&s_alloc_count, 1, __ATOMIC_RELAXED);
                __atomic_fetch_add(&s_alloc_total, 1, __ATOMIC_RELAXED);
            }
        }
        return np;
    }
    if (!s_real_realloc) {
        if (s_init_in_progress) return NULL;
        init_alloc_funcs();
    }
    void *p = s_real_realloc(ptr, size);
    /* realloc(NULL, size) is equivalent to malloc(size): count it. */
    if (!ptr && p && s_tracking_active) {
        __atomic_fetch_add(&s_alloc_count, 1, __ATOMIC_RELAXED);
        __atomic_fetch_add(&s_alloc_total, 1, __ATOMIC_RELAXED);
    }
    return p;
}

void free(void *ptr) {
    if (!ptr) return;
    if (is_bootstrap_ptr(ptr)) return; /* bootstrap: silently ignore */
    if (!s_real_free) init_alloc_funcs(); /* aborts if dlsym fails */
    s_real_free(ptr);
    if (s_tracking_active) {
        __atomic_fetch_sub(&s_alloc_count, 1, __ATOMIC_RELAXED);
    }
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

#define EXPECT(...) \
    if (!(__VA_ARGS__)) { \
        fprintf(stderr, "  FAIL [%s]: %s (line %d)\n", \
                s_current_test, #__VA_ARGS__, __LINE__); \
        s_tests_failed++; \
        break; \
    } 

#define EXPECT_OK(hr) EXPECT((hr) == NOERROR)
#define EXPECT_STR_EQ(a, b) EXPECT((a) && (b) && !strcmp(a, b))

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
    return OBJ_MakeNativeObject(fnv1a32("RTComp"));
}

/* ------------------------------------------------------------------ */
/* Object lifecycle helper                                            */
/* ------------------------------------------------------------------ */

/* Allocate a bare TestComp object without a Lua state */
static lpObject_t make_object(void) {
    return OBJ_MakeNativeObject(fnv1a32("TestComp"));
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
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Value", &prop));
        EXPECT(PROP_GetType(prop) == kDataTypeFloat);
        EXPECT(PROP_IsNull(prop));

        float f = 3.14f;
        PROP_SetValue(prop, &f);
        EXPECT(!PROP_IsNull(prop));
        EXPECT(*(float*)PROP_GetValue(prop) == 3.14f);
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
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));
        EXPECT(PROP_GetType(prop) == kDataTypeString);
        EXPECT(PROP_IsNull(prop));

        PROP_SetValue(prop, "hello");
        EXPECT(!PROP_IsNull(prop));
        EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "hello");
    }
}

static void test_string_property_reassign(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));

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

static void test_string_property_clear(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));

        PROP_SetValue(prop, "some string");
        EXPECT(!PROP_IsNull(prop));

        /* After clear the property should be null/reset */
        EXPECT(PROP_Clear(prop), PROP_IsNull(prop));

        /* Setting after clear must not double-free */
        PROP_SetValue(prop, "after clear");
        EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "after clear");
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
        PROP_SetValue(prop, "will be freed");
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

        EXPECT_OK(OBJ_SetPropertyValue(obj, "Label", "via api"));
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));
        EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "via api");
    }
}

static void test_property_state_string(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));

        /* Set the hover state value */
        PROP_SetValue(prop, "normal");
        EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "normal");

        /* Clear all states — should free heap strings without crashing */
        EXPECT(PROP_Clear(prop), PROP_IsNull(prop));

        /* Re-set after full clear */
        PROP_SetValue(prop, "reset");
        EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "reset");
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
        PROP_SetValue(pLabel, "hello");

        /* Verify they don't alias each other */
        EXPECT(*(int*)PROP_GetValue(pCount) == 5);
        EXPECT_STR_EQ((const char*)PROP_GetValue(pLabel), "hello");

        PROP_SetValue(pLabel, "world");
        EXPECT(*(int*)PROP_GetValue(pCount) == 5); /* Count unchanged */
        EXPECT_STR_EQ((const char*)PROP_GetValue(pLabel), "world");
    }
}

static void test_string_empty(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));

        PROP_SetValue(prop, "");
        EXPECT(!PROP_IsNull(prop));
        EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "");
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
        WITH(struct token, prog, Token_Create("7"), Token_Release) {
            EXPECT(prog != NULL);
            struct vm_register r = {0};
            EXPECT(OBJ_RunProgram(obj, prog, &r));
            EXPECT(r.type == kDataTypeInt || r.type == kDataTypeFloat);
            EXPECT((int)r.value[0] == 7);
        }
    }
}

static void test_runtime_run_float_constant(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        WITH(struct token, prog, Token_Create("2.5"), Token_Release) {
            EXPECT(prog != NULL);
            struct vm_register r = {0};
            EXPECT(OBJ_RunProgram(obj, prog, &r));
            EXPECT(r.value[0] == 2.5f);
        }
    }
}

static void test_runtime_run_string_constant(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        WITH(struct token, prog, Token_Create("\"world\""), Token_Release) {
            EXPECT(prog != NULL);
            struct vm_register r = {0};
            EXPECT(OBJ_RunProgram(obj, prog, &r));
            EXPECT(r.type == kDataTypeString);
            const char *str = *(const char *const *)r.value;
            EXPECT_STR_EQ(str, "world");
        }
    }
}

static void test_runtime_run_arithmetic(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        WITH(struct token, prog, Token_Create("ADD(10, 5)"), Token_Release) {
            EXPECT(prog != NULL);
            struct vm_register r = {0};
            EXPECT(OBJ_RunProgram(obj, prog, &r));
            EXPECT((int)r.value[0] == 15);
        }
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
        EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "imported");
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
        EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "bound");
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
        EXPECT_STR_EQ((const char*)PROP_GetValue(prop), "foobar");
    }
}

/*
 * IF operator: IF(cond, true-value, false-value) works like Excel IF.
 * When condition is non-zero, returns true-value; otherwise false-value.
 */
static void test_runtime_if_true_branch(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        WITH(struct token, prog, Token_Create("IF(1, 10, 20)"), Token_Release) {
            EXPECT(prog != NULL);
            struct vm_register r = {0};
            EXPECT(OBJ_RunProgram(obj, prog, &r));
            EXPECT((int)r.value[0] == 10);
        }
    }
}

static void test_runtime_if_false_branch(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        WITH(struct token, prog, Token_Create("IF(0, 10, 20)"), Token_Release) {
            EXPECT(prog != NULL);
            struct vm_register r = {0};
            EXPECT(OBJ_RunProgram(obj, prog, &r));
            EXPECT((int)r.value[0] == 20);
        }
    }
}

static void test_runtime_if_string_branch(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        WITH(struct token, prog, Token_Create("IF(0, \"yes\", \"no\")"), Token_Release) {
            EXPECT(prog != NULL);
            struct vm_register r = {0};
            EXPECT(OBJ_RunProgram(obj, prog, &r));
            EXPECT(r.type == kDataTypeString);
            const char *str = *(const char *const *)r.value;
            EXPECT_STR_EQ(str, "no");
        }
    }
}

/* ------------------------------------------------------------------ */
/* Memory-leak tests (active when compiled with -DTEST_MEMORY)         */
/* Each test takes a snapshot before and checks for leaks after.       */
/* ------------------------------------------------------------------ */

/* Leak test: set a string property then destroy the object — the
 * strdup'd string must be freed by OBJ_ReleaseProperties. */
static void test_memleak_string_set_release(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));
        PROP_SetValue(prop, "hello leak test");
    }
}

/* Leak test: reassign a string multiple times — old strings must be freed. */
static void test_memleak_string_multiple_sets(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));
        PROP_SetValue(prop, "first");
        PROP_SetValue(prop, "second");   /* frees "first" */
        PROP_SetValue(prop, "third");    /* frees "second" */
    }                                    /* destroy_object frees "third" */
}

/* Leak test: PROP_Clear must free the heap string and leave no leak. */
static void test_memleak_prop_clear(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));
        PROP_SetValue(prop, "to be cleared");
        PROP_Clear(prop);
    }
}

/* Leak test: PROP_Clear on a never-set string must not crash or leak. */
static void test_memleak_prop_clear_never_set(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));
        PROP_Clear(prop);
    }
}

/* Leak test: set/clear/reset one string property and set a second scalar
 * property, then destroy — no heap string or Property struct leaks. */
static void test_memleak_multiple_string_props(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        /* String property: set, reassign, clear, set again */
        EXPECT_OK(OBJ_SetPropertyValue(obj, "Label", "alpha"));
        lpProperty_t strprop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &strprop));
        PROP_SetValue(strprop, "alpha2");
        PROP_Clear(strprop);
        PROP_SetValue(strprop, "final");
        /* Scalar property: set (no heap allocation for value) */
        EXPECT_OK(OBJ_SetPropertyValue(obj, "Count", &(int){7}));
    }
}

/* Leak test: Token_Create / Token_Release must not leak. */
static void test_memleak_token_create_release(void) {
    WITH(struct token, prog, Token_Create("ADD(1, 2)"), Token_Release) {
        (void)prog;
    }
}

/* Leak test: Token_Create for a string literal must not leak. */
static void test_memleak_token_string_literal(void) {
    WITH(struct token, prog, Token_Create("\"test string\""), Token_Release) {
        (void)prog;
    }
}

/* Leak test: OBJ_RunProgram with a string must not leak VM temporaries. */
static void test_memleak_run_string_program(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        WITH(struct token, prog, Token_Create("\"vm string\""), Token_Release) {
            struct vm_register r = {0};
            EXPECT(OBJ_RunProgram(obj, prog, &r));
        }
    }
}

/* Leak test: PROP_AttachProgram + PROP_Update + object destroy, no leak.
 * After PROP_AttachProgram the property owns the token; destroy_object
 * calls OBJ_ReleaseProperties which runs Token_Release + frees code strings. */
static void test_memleak_attach_update_release(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        lpProperty_t prop;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Label", &prop));
        struct token *prog = Token_Create("\"attached\"");
        EXPECT(prog != NULL);
        /* Property takes ownership of prog and strdup's the code. */
        PROP_AttachProgram(prop, kPropertyAttributeWholeProperty,
                           prog, "\"attached\"");
        core.frame++;
        PROP_Update(prop);
        /* destroy_object → OBJ_ReleaseProperties → Token_Release +
           free(programSources) + free(heap string value). */
    }
}

/* Leak test: set int, float, bool properties — no heap allocations expected. */
static void test_memleak_scalar_properties(void) {
    WITH(struct Object, obj, make_object(), destroy_object) {
        lpProperty_t pi, pf, pb;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Count",  &pi));
        EXPECT_OK(OBJ_FindShortProperty(obj, "Value",  &pf));
        EXPECT_OK(OBJ_FindShortProperty(obj, "Active", &pb));
        int    iv = 99;
        float  fv = 1.5f;
        bool_t bv = true;
        PROP_SetValue(pi, &iv);
        PROP_SetValue(pf, &fv);
        PROP_SetValue(pb, &bv);
    }
}

/* Leak test: property reference program (reads one prop, writes another). */
static void test_memleak_property_reference_program(void) {
    WITH(struct Object, obj, make_rt_object(), destroy_object) {
        lpProperty_t propCount, propValue;
        EXPECT_OK(OBJ_FindShortProperty(obj, "Count", &propCount));
        EXPECT_OK(OBJ_FindShortProperty(obj, "Value", &propValue));
        float v = 7.0f;
        PROP_SetValue(propValue, &v);
        struct token *prog = Token_Create("{./Value}");
        EXPECT(prog != NULL);
        /* Property owns prog after this call. */
        PROP_AttachProgram(propCount, kPropertyAttributeWholeProperty,
                           prog, "{./Value}");
        core.frame++;
        PROP_Update(propCount);
        /* destroy_object will call Token_Release on prog. */
    }
}

/* ------------------------------------------------------------------ */

#define RUN(func) \
long snap_##func = MEM_SNAPSHOT(); \
s_current_test = #func; \
s_tests_run++; \
puts("Running " #func "..."); \
func(); \
MEM_CHECK_LEAK(snap_##func, #func)

int main(void) {
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
    RUN(test_runtime_if_true_branch);
    RUN(test_runtime_if_false_branch);
    RUN(test_runtime_if_string_branch);

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
