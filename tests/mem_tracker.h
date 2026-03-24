/*
 * mem_tracker.h — malloc/free/strdup interception for per-test leak detection.
 *
 * Include this header in a test binary to get allocation tracking.  When the
 * binary is compiled with -DTEST_MEMORY, real replacements for malloc, calloc,
 * realloc, and free are provided that count live allocations.  Because the test
 * binary is linked dynamically against liborca.so, the dynamic linker prefers
 * the definitions here, so liborca allocations are counted too.
 *
 * Usage:
 *   1. Call mem_init() at the top of main() before any test code.
 *   2. Before each test:  long snap = MEM_SNAPSHOT();
 *   3. After each test:   MEM_CHECK_LEAK(snap, "test name");
 *
 * Without -DTEST_MEMORY the macros expand to no-ops so the same test code
 * compiles and runs correctly in both modes.
 *
 * REQUIREMENT: The including file must declare:
 *   static int s_tests_failed;
 * MEM_CHECK_LEAK increments that variable when a leak is detected so that
 * the test runner can report it as a failure.
 */
#ifndef MEM_TRACKER_H
#define MEM_TRACKER_H

#if TEST_MEMORY
#include <dlfcn.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>  /* abort() */
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
#endif /* MEM_TRACKER_H */
