#pragma once

#include "actionslink_types.h"

#define log_internal(level, ...)                                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        if (ACTIONSLINK_LOG_LEVEL >= level)                                                                            \
        {                                                                                                              \
            actionslink_log(level, __VA_ARGS__);                                                                       \
        }                                                                                                              \
    } while (0)

#define log_error(...)     log_internal(ACTIONSLINK_LOG_LEVEL_ERROR, __VA_ARGS__)
#define log_warning(...)   log_internal(ACTIONSLINK_LOG_LEVEL_WARN, __VA_ARGS__)
#define log_info(...)      log_internal(ACTIONSLINK_LOG_LEVEL_INFO, __VA_ARGS__)
#define log_debug(...)     log_internal(ACTIONSLINK_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define log_trace(...)     log_internal(ACTIONSLINK_LOG_LEVEL_TRACE, __VA_ARGS__)

/**
 * @brief Passes log messages to user-defined log function.
 *
 * @param[in] lvl       log level
 * @param[in] args      log format string
 * @param ...           format arguments
 */
void actionslink_log(actionslink_log_level_t lvl, const char *args, ...);

/**
 * @brief Registers a function to pass logs to.
 *
 * @param[in] log_fn        log function
 */
void actionslink_log_register(actionslink_log_fn_t log_fn);
