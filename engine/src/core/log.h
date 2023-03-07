#pragma once

#include "defines.h"

#define LOG_FATAL_ENABLED 1
#define LOG_ERROR_ENABLED 1
#define LOG_WARN_ENABLED  1
#define LOG_INFO_ENABLED  1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

// Disable debug and trace logging for release builds
#if OKO_RELEASE == 1
  #define LOG_DEBUG_ENABLED 0
  #define LOG_TRACE_ENABLED 0
#endif

typedef enum log_level {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
} log_level;

b8 log_initialize(u64* memory_requirement, void* state);
void log_shutdown(void* state);

OKO_API void log_output(log_level level, const char* message, ...);

#if LOG_FATAL_ENABLED == 1
  #define OKO_FATAL(message, ...) \
    log_output(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);
#else
  #define OKO_FATAL(message, ...)
#endif

#if LOG_ERROR_ENABLED == 1
  #define OKO_ERROR(message, ...) \
    log_output(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#else
  #define OKO_ERROR(message, ...)
#endif

#if LOG_WARN_ENABLED == 1
  #define OKO_WARN(message, ...) \
    log_output(LOG_LEVEL_WARN, message, ##__VA_ARGS__);
#else
  #define OKO_WARN(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
  #define OKO_INFO(message, ...) \
    log_output(LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#else
  #define OKO_INFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
  #define OKO_DEBUG(message, ...) \
    log_output(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#else
  #define OKO_DEBUG(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
  #define OKO_TRACE(message, ...) \
    log_output(LOG_LEVEL_TRACE, message, ##__VA_ARGS__);
#else
  #define OKO_TRACE(message, ...)
#endif