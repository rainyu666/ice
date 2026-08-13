#pragma once

#include <stddef.h>
#include <stdint.h>

#include "include/logger_impl.h"

#include "logger_config.h"
#include "logger_format.h"

// --------------------------------------------------------------------------
// Logger checks for required definitions
// --------------------------------------------------------------------------

#if LOGGER_PRINT_LOG_LOCATION

#ifndef LOG_MODULE_NAME
#if defined(__GNUC__) && (__GNUC__ > 11)
#define LOG_MODULE_NAME __FILE_NAME__
#else
#error "Logger is configured to print the log location but the module name is not defined in this file.\n" \
 "Add #define LOG_MODULE_NAME "module_name" before #include logger.h"
#endif
#endif

#else // LOGGER_PRINT_LOG_LOCATION

#ifndef LOG_MODULE_NAME
#define LOG_MODULE_NAME ""
#endif

#endif // LOGGER_PRINT_LOG_LOCATION

#ifndef LOG_LEVEL
#define LOG_LEVEL LOGGER_DEFAULT_LOG_LEVEL
#endif

// If the user configuration is to force the log level globally
// undefine the module's log level and redefine it with the user default
#if LOGGER_FORCE_GLOBAL_LOG_LEVEL
#undef LOG_LEVEL
#define LOG_LEVEL LOGGER_FORCED_LOG_LEVEL
#endif

// --------------------------------------------------------------------------
// Logger internal macro definitions
// --------------------------------------------------------------------------

#if LOGGER_OUTPUT_OPTION == LOGGER_OUTPUT_RAW
#include "outputs/logger_printf.h"
#else

#if defined __cplusplus
extern "C"
{
#endif

    void logger_internal_print_color(logger_log_level_t level);

    void logger_internal_reset_color(void);

    void logger_internal_print_log_level(logger_log_level_t level);

    void logger_internal_print_timestamp();

    void logger_internal_print_log_location(const char *module_name, size_t line_number,
                                            uint8_t module_name_string_width);

    void logger_internal_print_log(const char *format_string, ...);

    void logger_internal_print_new_line(const char *new_line_string);

    int logger_internal_lock(void);
    void logger_internal_unlock(void);

#include "outputs/logger_any.h"

#if defined __cplusplus
}
#endif

#endif // LOGGER_OUTPUT_OPTION == LOGGER_OUTPUT_RAW

// --------------------------------------------------------------------------
// Logger API macro definitions
// (see logger_config.h for macro names)
// --------------------------------------------------------------------------

#define log_init()  logger_init()
#define log_flush() logger_flush()

#define log_fatal(...)     log_internal(LOG_LEVEL_FATAL, __VA_ARGS__)
#define log_err(...)       log_internal(LOG_LEVEL_ERROR, __VA_ARGS__)
#define log_error(...)     log_internal(LOG_LEVEL_ERROR, __VA_ARGS__)
#define log_warn(...)      log_internal(LOG_LEVEL_WARNING, __VA_ARGS__)
#define log_warning(...)   log_internal(LOG_LEVEL_WARNING, __VA_ARGS__)
#define log_high(...)      log_internal(LOG_LEVEL_HIGHLIGHT, __VA_ARGS__)
#define log_highlight(...) log_internal(LOG_LEVEL_HIGHLIGHT, __VA_ARGS__)
#define log_info(...)      log_internal(LOG_LEVEL_INFO, __VA_ARGS__)
#define log_dbg(...)       log_internal(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define log_debug(...)     log_internal(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define log_trace(...)     log_internal(LOG_LEVEL_TRACE, __VA_ARGS__)

#define log_fatal_raw(...)     log_internal_raw(LOG_LEVEL_FATAL, __VA_ARGS__)
#define log_err_raw(...)       log_internal_raw(LOG_LEVEL_ERROR, __VA_ARGS__)
#define log_error_raw(...)     log_internal_raw(LOG_LEVEL_ERROR, __VA_ARGS__)
#define log_warn_raw(...)      log_internal_raw(LOG_LEVEL_WARNING, __VA_ARGS__)
#define log_warning_raw(...)   log_internal_raw(LOG_LEVEL_WARNING, __VA_ARGS__)
#define log_high_raw(...)      log_internal_raw(LOG_LEVEL_HIGHLIGHT, __VA_ARGS__)
#define log_highlight_raw(...) log_internal_raw(LOG_LEVEL_HIGHLIGHT, __VA_ARGS__)
#define log_info_raw(...)      log_internal_raw(LOG_LEVEL_INFO, __VA_ARGS__)
#define log_dbg_raw(...)       log_internal_raw(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define log_debug_raw(...)     log_internal_raw(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define log_trace_raw(...)     log_internal_raw(LOG_LEVEL_TRACE, __VA_ARGS__)
