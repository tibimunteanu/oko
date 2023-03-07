#include <core/log.h>
#include <math/math.h>

#define expect_should_be(expected, actual)                \
  if (actual != expected) {                               \
    OKO_ERROR(                                            \
        "--> Expected %lld, but got: %lld. File: %s:%d.", \
        expected,                                         \
        actual,                                           \
        __FILE__,                                         \
        __LINE__                                          \
    );                                                    \
    return false;                                         \
  }

#define expect_should_not_be(expected, actual)                     \
  if (actual == expected) {                                        \
    OKO_ERROR(                                                     \
        "--> Expected %d != %d, but they are equal. File: %s:%d.", \
        expected,                                                  \
        actual,                                                    \
        __FILE__,                                                  \
        __LINE__                                                   \
    );                                                             \
    return false;                                                  \
  }

#define expect_float_to_be(expected, actual)            \
  if (oko_abs(expected - actual) > OKO_FLOAT_EPSILON) { \
    OKO_ERROR(                                          \
        "--> Expected %f, but got: %f. File: %s:%d.",   \
        expected,                                       \
        actual,                                         \
        __FILE__,                                       \
        __LINE__                                        \
    );                                                  \
    return false;                                       \
  }

#define expect_to_be_true(actual)                                             \
  if (actual != true) {                                                       \
    OKO_ERROR(                                                                \
        "--> Expected true, but got: false. File: %s:%d.", __FILE__, __LINE__ \
    );                                                                        \
    return false;                                                             \
  }

#define expect_to_be_false(actual)                                            \
  if (actual != false) {                                                      \
    OKO_ERROR(                                                                \
        "--> Expected false, but got: true. File: %s:%d.", __FILE__, __LINE__ \
    );                                                                        \
    return false;                                                             \
  }