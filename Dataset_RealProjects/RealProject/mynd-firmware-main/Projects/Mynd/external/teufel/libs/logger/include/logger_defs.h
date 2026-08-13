#pragma once

#define LOG_COLOR_DEFAULT        "\x1B[0m"
#define LOG_COLOR_BLACK          "\x1B[2;30m"
#define LOG_COLOR_RED            "\x1B[2;31m"
#define LOG_COLOR_GREEN          "\x1B[2;32m"
#define LOG_COLOR_YELLOW         "\x1B[2;33m"
#define LOG_COLOR_BLUE           "\x1B[2;34m"
#define LOG_COLOR_MAGENTA        "\x1B[2;35m"
#define LOG_COLOR_CYAN           "\x1B[2;36m"
#define LOG_COLOR_WHITE          "\x1B[2;37m"
#define LOG_COLOR_BRIGHT_BLACK   "\x1B[1;30m"
#define LOG_COLOR_BRIGHT_RED     "\x1B[1;31m"
#define LOG_COLOR_BRIGHT_GREEN   "\x1B[1;32m"
#define LOG_COLOR_BRIGHT_YELLOW  "\x1B[1;33m"
#define LOG_COLOR_BRIGHT_BLUE    "\x1B[1;34m"
#define LOG_COLOR_BRIGHT_MAGENTA "\x1B[1;35m"
#define LOG_COLOR_BRIGHT_CYAN    "\x1B[1;36m"
#define LOG_COLOR_BRIGHT_WHITE   "\x1B[1;37m"
#define LOG_COLOR_GREY           "\x1B[1;90m"

// ---------------------------------------------------------------------------------
// Logger output configuration
// ---------------------------------------------------------------------------------

#define LOGGER_OUTPUT_RAW       0 // Logs using printf without any formatting
#define LOGGER_OUTPUT_FORMATTED 1 // Generic output, which invokes printf call, with defined formatting

typedef enum
{
    LOG_LEVEL_OFF,
    LOG_LEVEL_FATAL,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_HIGHLIGHT,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,
} logger_log_level_t;

typedef enum
{
    LOG_OUTPUT_OPTION_NORMAL,
    LOG_OUTPUT_OPTION_RAW,
} logger_log_output_option_t;
