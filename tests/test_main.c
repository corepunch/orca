/* tests/test_main.c — entry point for the standalone C unit-test binary.
 *
 * Build with:
 *   make test-unit
 *
 * The binary links only against geometry_ops.c and libc — no Lua, no
 * renderer, no windowing system.  Object-property tests (which need a
 * Lua state and UIKit) are run via the normal orca binary; see
 * 'make test'.
 */

#include "test.h"

/* Global test state — extern-declared in test.h, defined here. */
const char *s_current_test = NULL;
int         s_tests_run    = 0;
int         s_tests_failed = 0;

void run_geometry_tests(void);

int main(void)
{
    run_geometry_tests();
    return test_summary();
}
