#pragma once

#if defined(LOGGER_USE_COLOR) && LOGGER_USE_COLOR == 1
#define logger_any_print_color(...) logger_internal_print_color(__VA_ARGS__)
#define logger_any_reset_color(...) logger_internal_reset_color()
#else
#define logger_any_print_color(...)
#define logger_any_reset_color(...)
#endif

#if defined(LOGGER_USE_FULL_LINE_COLOR) && LOGGER_USE_FULL_LINE_COLOR == 1
#define logger_any_reset_color_at_end_of_line(...) logger_any_reset_color(__VA_ARGS__)
#define logger_any_reset_color_after_log_level(...)
#else
#define logger_any_reset_color_at_end_of_line(...)
#define logger_any_reset_color_after_log_level(...) logger_any_reset_color(__VA_ARGS__)
#endif

#if defined(LOGGER_PRINT_LOG_LEVEL) && LOGGER_PRINT_LOG_LEVEL == 1
#define logger_any_print_log_level(...) logger_internal_print_log_level(__VA_ARGS__)
#else
#define logger_any_print_log_level(...)
#endif

#if defined(LOGGER_PRINT_TIMESTAMP) && LOGGER_PRINT_TIMESTAMP == 1
#define logger_any_print_timestamp(...) logger_internal_print_timestamp(__VA_ARGS__)
#else
#define logger_any_print_timestamp(...)
#endif

#if defined(LOGGER_PRINT_TIMESTAMP_BEFORE_LOG_LEVEL) && LOGGER_PRINT_TIMESTAMP_BEFORE_LOG_LEVEL == 1
#define logger_any_print_timestamp_before_log_level(...) logger_any_print_timestamp(__VA_ARGS__)
#define logger_any_print_timestamp_after_log_level(...)
#else
#define logger_any_print_timestamp_before_log_level(...)
#define logger_any_print_timestamp_after_log_level(...) logger_any_print_timestamp(__VA_ARGS__)
#endif

#if defined(LOGGER_PRINT_LOG_LOCATION) && LOGGER_PRINT_LOG_LOCATION == 1
#define logger_any_print_log_location(...) logger_internal_print_log_location(__VA_ARGS__)
#else
#define logger_any_print_log_location(...)
#endif

#if defined(LOGGER_PRINT_NEW_LINE) && LOGGER_PRINT_NEW_LINE == 1
#define logger_any_print_new_line(...) logger_internal_print_new_line(__VA_ARGS__)
#else
#define logger_any_print_new_line(...)
#endif

#define log_internal_raw(level, ...)                                                                                   \
    do                                                                                                                 \
    {                                                                                                                  \
        if (LOG_LEVEL >= level)                                                                                        \
        {                                                                                                              \
            logger_internal_print_log(__VA_ARGS__);                                                                    \
        }                                                                                                              \
    } while (0)


#define log_internal(level, ...)                                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        if (LOG_LEVEL >= level)                                                                                        \
        {                                                                                                              \
            if (logger_internal_lock() != 0)                                                                           \
                break;                                                                                                 \
            logger_any_print_timestamp_before_log_level();                                                             \
            logger_any_print_color(level);                                                                             \
            logger_any_print_log_level(level);                                                                         \
            logger_any_reset_color_after_log_level(level);                                                             \
            logger_any_print_timestamp_after_log_level();                                                              \
            logger_any_print_log_location(LOG_MODULE_NAME, __LINE__, LOGGER_LOG_LOCATION_WIDTH);                       \
            logger_internal_print_log(__VA_ARGS__);                                                                    \
            logger_any_print_new_line(LOGGER_NEW_LINE_STRING);                                                         \
            logger_any_reset_color_at_end_of_line(level);                                                              \
            logger_internal_unlock();                                                                                  \
        }                                                                                                              \
    } while (0)
