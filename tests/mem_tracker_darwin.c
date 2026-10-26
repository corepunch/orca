/* Injected only into allocation-tracked Darwin tests. dyld applies interpose
 * entries in this dylib to both the executable and its engine dependencies. */
#define TEST_MEMORY 1
#define ORCA_MEMORY_INTERPOSER 1
#include "mem_tracker.h"

void orca_test_memory_init(void)
{
    mem_init();
}

long orca_test_memory_snapshot(void)
{
    return MEM_SNAPSHOT();
}
