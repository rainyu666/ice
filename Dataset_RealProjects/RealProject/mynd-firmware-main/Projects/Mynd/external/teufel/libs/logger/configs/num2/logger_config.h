#pragma once

#include "include/logger_defs.h"

// clang-format off

// ---------------------------------------------------------------------------------
// Logger w/FreeRTOS configuration
// - Add FreeRTOS must be defined as compile-time definitions of the build
// - The Client must define LOGGER_USE_EXTERNAL_THREAD flag from the build system
// - The Client must call logger_init function to explicitly pass stream buffer handle
// ---------------------------------------------------------------------------------

#if defined(FreeRTOS) && defined(LOGGER_USE_EXTERNAL_THREAD)
#define LOGGER_MUTEX_LOCK_TIMEOUT_MS                10u

#define LOGGER_STREAM_BUFFER_TIMEOUT_MS             0u
#endif

// Choose one of the backends defined above
#define LOGGER_OUTPUT_OPTION                        LOGGER_OUTPUT_FORMATTED

// ---------------------------------------------------------------------------------
// Logger formatting configuration
// ---------------------------------------------------------------------------------

// Set this to 1 to use a static buffer for formatting logs
// This is not thread safe and may result in corrupted logs if you use the logger a lot
// When this is set to 0, the logger uses stack-allocated buffers which are thread-safe,
// but if you're using an RTOS it will result in bloating the required stack size of each
// task that uses the logger
#define LOGGER_USE_STATIC_FORMATTING_BUFFER         1

// The size of the buffer in which the formatted string is to be stored
#define LOGGER_FORMATTING_BUFFER_SIZE               128

// ---------------------------------------------------------------------------------
// Logger global logging level configuration
// ---------------------------------------------------------------------------------

// The default log level in case it is not specified in the file using the logger
#define LOGGER_DEFAULT_LOG_LEVEL                    LOG_LEVEL_INFO

// This can be used to force the log level of every module to be set to a given log level
// It's useful when you want to increase/decrease the log level of the entire project at once
#define LOGGER_FORCE_GLOBAL_LOG_LEVEL               0

// If the log level is being forced, force it to trace to log everything everywhere
#if LOGGER_FORCE_GLOBAL_LOG_LEVEL
#define LOGGER_FORCED_LOG_LEVEL                     LOG_LEVEL_TRACE
#endif

// ---------------------------------------------------------------------------------
// Logger contents configuration
// ---------------------------------------------------------------------------------

#define LOG_FATAL_COLOR                             LOG_COLOR_BRIGHT_RED
#define LOG_ERROR_COLOR                             LOG_COLOR_BRIGHT_RED
#define LOG_WARNING_COLOR                           LOG_COLOR_BRIGHT_YELLOW
#define LOG_HIGHLIGHT_COLOR                         LOG_COLOR_BRIGHT_GREEN
#define LOG_INFO_COLOR                              LOG_COLOR_DEFAULT
#define LOG_DEBUG_COLOR                             LOG_COLOR_DEFAULT
#define LOG_TRACE_COLOR                             LOG_COLOR_DEFAULT

// clang-format on
