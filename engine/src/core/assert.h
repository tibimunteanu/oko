#pragma once

#include "defines.h"

// Disable assertions by commenting out the below line
#define OKO_ASSERTIONS_ENABLED

#ifdef OKO_ASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif

OKO_API void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line);

#define OKO_ASSERT(expr)                                             \
    {                                                                \
        if (expr) {                                                  \
        } else {                                                     \
            report_assertion_failure(#expr, "", __FILE__, __LINE__); \
            debugBreak();                                            \
        }                                                            \
    }

#define OKO_ASSERT_MSG(expr, message)                                     \
    {                                                                     \
        if (expr) {                                                       \
        } else {                                                          \
            report_assertion_failure(#expr, message, __FILE__, __LINE__); \
            debugBreak();                                                 \
        }                                                                 \
    }

#ifdef _DEBUG
#define OKO_ASSERT_DEBUG(expr)                                       \
    {                                                                \
        if (expr) {                                                  \
        } else {                                                     \
            report_assertion_failure(#expr, "", __FILE__, __LINE__); \
            debugBreak();                                            \
        }                                                            \
    }
#else
#define OKO_ASSERT_DEBUG(expr)
#endif

#else
#define OKO_ASSERT(expr)
#define OKO_ASSERT_MSG(expr, message)
#define OKO_ASSERT_DEBUG(expr)
#endif