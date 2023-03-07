#include "test_manager.h"

#include "memory/linear_allocator_tests.h"

#include <core/log.h>

int main() {
    test_manager_init();

    // add test registrations here.
    linear_allocator_register_tests();

    OKO_DEBUG("Starting tests...");

    test_manager_run_tests();

    return 0;
}