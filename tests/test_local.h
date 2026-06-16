// test_local.h - Common test utilities
// Include this in all test files to avoid duplicating macros

#ifndef TEST_LOCAL_H
#define TEST_LOCAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Minimal test harness                                                */
/* ------------------------------------------------------------------ */

static char const *s_current_test = NULL;
static int s_tests_run = 0;
static int s_tests_failed = 0;

#define EXPECT(...)                                                        \
    do {                                                                   \
        if (!(__VA_ARGS__)) {                                              \
            fprintf(stderr, "  FAIL [%s]: %s (line %d)\n",                 \
                    s_current_test, #__VA_ARGS__, __LINE__);               \
            s_tests_failed++;                                              \
        }                                                                  \
    } while (0)

#define EXPECT_OK(hr) EXPECT((hr) == NOERROR)

#define RUN(name, block)                                                   \
    do {                                                                   \
        s_current_test = name;                                             \
        s_tests_run++;                                                     \
        printf("Running %s...\n", name);                                   \
        do { block } while (0);                                            \
        if (s_tests_failed) {                                              \
            fprintf(stderr, "  FAILED [%s] (see %d error(s))\n",           \
                    name, s_tests_failed);                                 \
        }                                                                  \
    } while (0)

#endif // TEST_LOCAL_H
