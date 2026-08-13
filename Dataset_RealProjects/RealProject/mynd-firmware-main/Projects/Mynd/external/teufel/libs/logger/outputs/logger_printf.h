#pragma once

#include "stdio.h"

#define log_internal_raw(level, ...)                                                                                   \
    do                                                                                                                 \
    {                                                                                                                  \
        if (LOG_LEVEL >= level)                                                                                        \
        {                                                                                                              \
            printf(__VA_ARGS__);                                                                                       \
        }                                                                                                              \
    } while (0)

#define log_internal(level, ...)                                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        if (LOG_LEVEL >= level)                                                                                        \
        {                                                                                                              \
            printf(__VA_ARGS__);                                                                                       \
        }                                                                                                              \
    } while (0)
