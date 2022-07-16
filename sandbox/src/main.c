#include <core/logger.h>
#include <core/asserts.h>

int main(void) {
    OKO_FATAL("A test message: %f", 3.14f);
    OKO_ERROR("A test message: %f", 3.14f);
    OKO_WARN("A test message: %f", 3.14f);
    OKO_INFO("A test message: %f", 3.14f);
    OKO_DEBUG("A test message: %f", 3.14f);
    OKO_TRACE("A test message: %f", 3.14f);

    OKO_ASSERT(1 == 0);

    return 0;
}