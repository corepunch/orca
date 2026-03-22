/* tests/test_main.c — C test driver compiled into the orca binary.
 *
 * When the orca binary is built with -DORCA_C_TESTS (see 'make test-unit-c'),
 * test_main.c and test_properties.c are compiled in.  The main loop in
 * orca.c calls run_c_tests() when the test path has a '.c' extension.
 *
 * Usage:
 *   make test-unit-c            → builds and runs the C property tests
 */

#include <include/api.h>
#include <include/orca.h>
#include "test.h"

/* Global test state — extern-declared in test.h, defined here. */
const char *s_current_test = NULL;
int         s_tests_run    = 0;
int         s_tests_failed = 0;

/* Lua state and test class set by run_c_tests() before each test suite. */
lua_State     *s_test_L     = NULL;
lpcClassDesc_t s_test_class = NULL;

/* Defined in test_properties.c */
void run_property_tests(void);

/* Called from orca.c main loop when the test file ends with '.c'. */
int run_c_tests(lua_State *L)
{
    s_test_L     = L;
    s_test_class = OBJ_FindClass("Node2D");
    if (!s_test_class) {
        fprintf(stderr, "C test setup failed: Node2D class not found "
                "(UIKit plugin may not be loaded).\n");
        return 1;
    }
    run_property_tests();
    if (s_tests_failed == 0) {
        printf("All %d C property tests passed.\n", s_tests_run);
    } else {
        fprintf(stderr, "%d/%d C property tests FAILED.\n",
                s_tests_failed, s_tests_run);
    }
    return s_tests_failed;
}
