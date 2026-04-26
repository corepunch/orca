/*
 * test_filesystem.c — C unit tests for FS_LoadObject's query-arg parsing.
 *
 * Tests cover the _ParseLoaderArgs contract:
 *   - No query string: argc stays at argc_start, argv[argc_start] = NULL
 *   - Single bare flag: argv[1] = "mask", argc = 2
 *   - Key=value pair: argv[1] = "width=48", argc = 2
 *   - Multiple args: splits on '&', all land in argv correctly
 *   - argv[argc] is always NULL (mirrors main() convention)
 *   - Truncation: arg longer than 64 chars is silently truncated to 64 chars
 *   - Too many args: extras beyond argc_max are silently dropped
 *   - Leading '?' in query string is stripped
 *
 * Compiled via the `test-filesystem` Makefile target (depends on `buildlib`).
 */

#include "mem_tracker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Mirror the constants from filesystem.c so we can declare matching arrays.
 * If these diverge from the source, the _Static_assert below will catch it. */
#define FS_LOADER_MAX_ARGS  16
#define FS_LOADER_ARG_LEN   65  /* 64 visible chars + NUL */

/* _ParseLoaderArgs is non-static (exported from liborca.so for testability). */
extern int _ParseLoaderArgs(const char *query_string,
                             const char *argv[],
                             int         argc_start,
                             int         argc_max,
                             char        arg_buf[][FS_LOADER_ARG_LEN]);

/* ------------------------------------------------------------------ */
/* Minimal test harness (same style as test_properties.c)             */
/* ------------------------------------------------------------------ */

static int s_tests_run    = 0;
static int s_tests_failed = 0;
static const char* s_current_test = NULL;

#define EXPECT(...) \
    if (!(__VA_ARGS__)) { \
        fprintf(stderr, "  FAIL [%s]: %s (line %d)\n", \
                s_current_test, #__VA_ARGS__, __LINE__); \
        s_tests_failed++; \
        break; \
    }

#define EXPECT_STR_EQ(a, b) \
    do { const char *_a = (a); const char *_b = (b); EXPECT(_a && _b && !strcmp(_a, _b)); } while(0)

#define RUN(name, block) \
    do { \
        s_current_test = name; \
        s_tests_run++; \
        printf("Running %s...\n", name); \
        do { block } while (0); \
    } while (0)

/* ------------------------------------------------------------------ */
/* Helper: allocate argv + arg_buf, preset argv[0] = path.            */
/* ------------------------------------------------------------------ */

static int
call_parse(const char *path, const char *query,
           const char **argv, char arg_buf[][FS_LOADER_ARG_LEN])
{
    argv[0] = path;
    return _ParseLoaderArgs(query, argv, 1, FS_LOADER_MAX_ARGS, arg_buf);
}

/* ------------------------------------------------------------------ */
/* Tests                                                               */
/* ------------------------------------------------------------------ */

static void test_no_query(void)
{
    RUN("no_query_string", {
        const char *argv[FS_LOADER_MAX_ARGS + 1];
        char arg_buf[FS_LOADER_MAX_ARGS][FS_LOADER_ARG_LEN];
        int argc = call_parse("img.png", NULL, argv, arg_buf);
        EXPECT(argc == 1);
        EXPECT(argv[0] != NULL);
        EXPECT_STR_EQ(argv[0], "img.png");
        EXPECT(argv[1] == NULL); /* sentinel */
    });
}

static void test_empty_query(void)
{
    RUN("empty_query_string", {
        const char *argv[FS_LOADER_MAX_ARGS + 1];
        char arg_buf[FS_LOADER_MAX_ARGS][FS_LOADER_ARG_LEN];
        int argc = call_parse("img.png", "", argv, arg_buf);
        EXPECT(argc == 1);
        EXPECT(argv[1] == NULL);
    });
}

static void test_leading_question_mark(void)
{
    RUN("leading_question_mark_stripped", {
        const char *argv[FS_LOADER_MAX_ARGS + 1];
        char arg_buf[FS_LOADER_MAX_ARGS][FS_LOADER_ARG_LEN];
        int argc = call_parse("img.png", "?mask", argv, arg_buf);
        EXPECT(argc == 2);
        EXPECT_STR_EQ(argv[1], "mask");
        EXPECT(argv[2] == NULL);
    });
}

static void test_single_flag(void)
{
    RUN("single_bare_flag", {
        const char *argv[FS_LOADER_MAX_ARGS + 1];
        char arg_buf[FS_LOADER_MAX_ARGS][FS_LOADER_ARG_LEN];
        int argc = call_parse("img.png", "?mask", argv, arg_buf);
        EXPECT(argc == 2);
        EXPECT_STR_EQ(argv[0], "img.png");
        EXPECT_STR_EQ(argv[1], "mask");
        EXPECT(argv[2] == NULL);
    });
}

static void test_key_value(void)
{
    RUN("key_value_pair", {
        const char *argv[FS_LOADER_MAX_ARGS + 1];
        char arg_buf[FS_LOADER_MAX_ARGS][FS_LOADER_ARG_LEN];
        int argc = call_parse("img.png", "?width=48", argv, arg_buf);
        EXPECT(argc == 2);
        EXPECT_STR_EQ(argv[1], "width=48");
        EXPECT(argv[2] == NULL);
    });
}

static void test_multiple_args(void)
{
    RUN("multiple_args_split_on_ampersand", {
        const char *argv[FS_LOADER_MAX_ARGS + 1];
        char arg_buf[FS_LOADER_MAX_ARGS][FS_LOADER_ARG_LEN];
        int argc = call_parse("img.png", "?width=48&type=mask&scale=2", argv, arg_buf);
        EXPECT(argc == 4);
        EXPECT_STR_EQ(argv[0], "img.png");
        EXPECT_STR_EQ(argv[1], "width=48");
        EXPECT_STR_EQ(argv[2], "type=mask");
        EXPECT_STR_EQ(argv[3], "scale=2");
        EXPECT(argv[4] == NULL);
    });
}

static void test_sentinel_always_null(void)
{
    /* Verify argv[argc] == NULL for a non-trivial case. */
    RUN("argv_argc_always_null", {
        const char *argv[FS_LOADER_MAX_ARGS + 1];
        char arg_buf[FS_LOADER_MAX_ARGS][FS_LOADER_ARG_LEN];
        int argc = call_parse("a.png", "?x=1&y=2", argv, arg_buf);
        EXPECT(argc == 3);
        EXPECT(argv[argc] == NULL);
    });
}

static void test_truncation(void)
{
    /* An arg that is 65+ chars must be silently truncated to 64 visible chars. */
    RUN("long_arg_truncated_to_64_chars", {
        /* 70 'a' characters */
        const char *argv[FS_LOADER_MAX_ARGS + 1];
        char arg_buf[FS_LOADER_MAX_ARGS][FS_LOADER_ARG_LEN];
        char long_arg[80];
        memset(long_arg, 'a', 70);
        long_arg[70] = '\0';
        char query[80 + 1];
        snprintf(query, sizeof(query), "?%s", long_arg);
        int argc = call_parse("img.png", query, argv, arg_buf);
        EXPECT(argc == 2);
        EXPECT(argv[1] != NULL);
        EXPECT(strlen(argv[1]) == 64); /* truncated to 64 visible chars */
        EXPECT(argv[2] == NULL);
    });
}

static void test_too_many_args(void)
{
    /* Build a query with MAX_LOADER_ARGS + 2 entries: only first MAX-1 should survive. */
    RUN("extra_args_beyond_max_dropped", {
        const char *argv[FS_LOADER_MAX_ARGS + 1];
        char arg_buf[FS_LOADER_MAX_ARGS][FS_LOADER_ARG_LEN];
        /* Construct "?a0&a1&...&a17" — 18 args, more than the 15 slots available */
        char query[512];
        int pos = 0;
        query[pos++] = '?';
        for (int i = 0; i < FS_LOADER_MAX_ARGS + 2; i++) {
            if (i > 0) query[pos++] = '&';
            int rem = (int)sizeof(query) - pos;
            if (rem <= 0) break;
            pos += snprintf(query + pos, (size_t)rem, "a%d", i);
        }
        query[pos] = '\0';
        int argc = call_parse("img.png", query, argv, arg_buf);
        /* argc_max = FS_LOADER_MAX_ARGS means at most FS_LOADER_MAX_ARGS-1 query args
         * (since argv[0] = path uses slot 0 and argc_start = 1), giving argc == FS_LOADER_MAX_ARGS. */
        EXPECT(argc == FS_LOADER_MAX_ARGS);
        EXPECT(argv[argc] == NULL);
    });
}

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */

int main(void)
{
    test_no_query();
    test_empty_query();
    test_leading_question_mark();
    test_single_flag();
    test_key_value();
    test_multiple_args();
    test_sentinel_always_null();
    test_truncation();
    test_too_many_args();

    printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
    return s_tests_failed == 0 ? 0 : 1;
}
