#pragma once

#include "include/logger_defs.h"

// clang-format off

// Set this to 1 to produce colored output
#define LOGGER_USE_COLOR                            1

// Set this to 1 to apply the color to the entire log line,
// otherwise only the log level will be colored
#define LOGGER_USE_FULL_LINE_COLOR                  0

// Set this to 1 to prefix the output with a "[LEVEL]" string
#define LOGGER_PRINT_LOG_LEVEL                      1

// Set this to 1 to timestamp the logs
#define LOGGER_PRINT_TIMESTAMP                      1

// Set this to 1 to print the timestamp before the log level
#define LOGGER_PRINT_TIMESTAMP_BEFORE_LOG_LEVEL     1

// Set this to 1 to add the "module:line_number" to the logs
// Note that if this option is enabled
// LOG_MODULE_NAME must be defined in every file where the logger is used
#define LOGGER_PRINT_LOG_LOCATION                   1

// Set this to terminate each log with a new line string
#define LOGGER_PRINT_NEW_LINE                       1

// This defines the string to be appended to the logs
// when the LOGGER_PRINT_NEW_LINE option is set to 1
#define LOGGER_NEW_LINE_STRING                      "\r\n"

// This defines the width of the log location string that is added to the logs
// when the LOGGER_PRINT_LOG_LOCATION option is set to 1.
// It is useful to align the actual contents of the logs to the right of the "log header"
// Increase it if you have long module names or decrease it if your module names are short
#define LOGGER_LOG_LOCATION_WIDTH                   30

// clang-format on
