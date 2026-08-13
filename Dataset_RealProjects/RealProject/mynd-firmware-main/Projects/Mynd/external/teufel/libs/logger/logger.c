#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "logger_config.h"
#include "include/logger_impl.h"

#if LOGGER_FORMATTING_BUFFER_SIZE < 32
#error "Logger formatting buffer must have a size of at least 32 bytes"
#endif

#if LOGGER_USE_STATIC_FORMATTING_BUFFER == 1
static char logger_buffer[LOGGER_FORMATTING_BUFFER_SIZE];
#endif

static const char *const logger_log_level_strings[8] = {
    [LOG_LEVEL_OFF] = "OFF",      [LOG_LEVEL_FATAL] = "FATAL",    [LOG_LEVEL_ERROR] = "ERROR",
    [LOG_LEVEL_WARNING] = "WARN", [LOG_LEVEL_HIGHLIGHT] = "HIGH", [LOG_LEVEL_INFO] = "INFO",
    [LOG_LEVEL_DEBUG] = "DEBUG",  [LOG_LEVEL_TRACE] = "TRACE",
};

static const char *const logger_log_color_strings[8] = {
    [LOG_LEVEL_OFF]       = "",
    [LOG_LEVEL_FATAL]     = LOG_FATAL_COLOR,
    [LOG_LEVEL_ERROR]     = LOG_ERROR_COLOR,
    [LOG_LEVEL_WARNING]   = LOG_WARNING_COLOR,
    [LOG_LEVEL_HIGHLIGHT] = LOG_HIGHLIGHT_COLOR,
    [LOG_LEVEL_INFO]      = LOG_INFO_COLOR,
    [LOG_LEVEL_DEBUG]     = LOG_DEBUG_COLOR,
    [LOG_LEVEL_TRACE]     = LOG_TRACE_COLOR,
};

static void print_buffer(char *p_data, size_t length);
static void print_encoding_error(void);
static void print_buffer_full_error(void);

#if defined(FreeRTOS) && defined(LOGGER_USE_EXTERNAL_THREAD)

#include "FreeRTOS.h"
#include "semphr.h"
#include "stream_buffer.h"

static StreamBufferHandle_t sbuffer_logger_h = NULL;
static SemaphoreHandle_t    mutex;

int logger_init(StreamBufferHandle_t sbuf)
{
    if (!sbuf)
        return -1;

    sbuffer_logger_h = sbuf;
#if defined(configSUPPORT_DYNAMIC_ALLOCATION) && (configSUPPORT_DYNAMIC_ALLOCATION == 1)
    mutex            = xSemaphoreCreateMutex();
#else
    static StaticSemaphore_t mutex_buffer;
    mutex = xSemaphoreCreateMutexStatic(&mutex_buffer);
#endif
    if (mutex == NULL)
    {
        return -1;
    }

    return 0;
}
#endif

int logger_internal_lock(void)
{
#if defined(FreeRTOS) && defined(LOGGER_USE_EXTERNAL_THREAD)
    uint32_t IPSR_register = 0U;

    __asm volatile("MRS %0, ipsr" : "=r"(IPSR_register));

    if (0U == IPSR_register)
    {
        if (xSemaphoreTake(mutex, pdMS_TO_TICKS(LOGGER_MUTEX_LOCK_TIMEOUT_MS)) != pdTRUE)
        {
            return -1;
        }
    }
    else
    {
        if (xSemaphoreTakeFromISR(mutex, NULL) != pdTRUE)
        {
            return -1;
        }
    }
#endif
    return 0;
}

void logger_internal_unlock(void)
{
#if defined(FreeRTOS) && defined(LOGGER_USE_EXTERNAL_THREAD)

    uint32_t IPSR_register = 0U;

    __asm volatile("MRS %0, ipsr" : "=r"(IPSR_register));

    if (0U == IPSR_register)
    {
        xSemaphoreGive(mutex);
    }
    else
    {
        xSemaphoreGiveFromISR(mutex, NULL);
    }

#endif
}

void logger_internal_print_color(logger_log_level_t level)
{
#if LOGGER_USE_STATIC_FORMATTING_BUFFER == 0
    char logger_buffer[LOGGER_FORMATTING_BUFFER_SIZE];
#endif

    int string_length = snprintf(logger_buffer, LOGGER_FORMATTING_BUFFER_SIZE, "%s", logger_log_color_strings[level]);
    if (string_length >= 0)
    {
        print_buffer(logger_buffer, string_length);
    }
    else
    {
        print_encoding_error();
    }
}

void logger_internal_reset_color(void)
{
#if LOGGER_USE_STATIC_FORMATTING_BUFFER == 0
    char logger_buffer[LOGGER_FORMATTING_BUFFER_SIZE];
#endif

    print_buffer(LOG_COLOR_DEFAULT, strlen(LOG_COLOR_DEFAULT));
}

void logger_internal_print_log_level(logger_log_level_t level)
{
#if LOGGER_USE_STATIC_FORMATTING_BUFFER == 0
    char logger_buffer[LOGGER_FORMATTING_BUFFER_SIZE];
#endif

    int string_length =
        snprintf(logger_buffer, LOGGER_FORMATTING_BUFFER_SIZE, "[%-5s] ", logger_log_level_strings[level]);
    if (string_length >= 0)
    {
        print_buffer(logger_buffer, string_length);
    }
    else
    {
        print_encoding_error();
    }
}

void logger_internal_print_timestamp(void)
{
#if LOGGER_USE_STATIC_FORMATTING_BUFFER == 0
    char logger_buffer[LOGGER_FORMATTING_BUFFER_SIZE];
#endif

    int string_length = snprintf(logger_buffer, LOGGER_FORMATTING_BUFFER_SIZE, "T%08lu: ", logger_get_timestamp());
    if (string_length >= 0)
    {
        print_buffer(logger_buffer, string_length);
    }
    else
    {
        print_encoding_error();
    }
}

void logger_internal_print_log_location(const char *module_name, size_t line_number, uint8_t module_name_string_width)
{
#if LOGGER_USE_STATIC_FORMATTING_BUFFER == 0
    char logger_buffer[LOGGER_FORMATTING_BUFFER_SIZE];
#endif

    int string_length = snprintf(logger_buffer, LOGGER_FORMATTING_BUFFER_SIZE, "%s:", module_name);
    if (string_length >= 0)
    {
        print_buffer(logger_buffer, string_length);
    }
    else
    {
        print_encoding_error();
    }

    // Negative numbers are handled fine, printf will just not add any trailing spaces
    // This keeps the log aligned to the right of the log location
    int string_width = module_name_string_width - string_length;

    string_length = snprintf(logger_buffer, LOGGER_FORMATTING_BUFFER_SIZE, "%-*u ", string_width, line_number);
    if (string_length >= 0)
    {
        print_buffer(logger_buffer, string_length);
    }
    else
    {
        print_encoding_error();
    }
}

void logger_internal_print_log(const char *format_string, ...)
{
#if LOGGER_USE_STATIC_FORMATTING_BUFFER == 0
    char logger_buffer[LOGGER_FORMATTING_BUFFER_SIZE];
#endif
    va_list vArgs;

    va_start(vArgs, format_string);
    int string_length = vsnprintf(logger_buffer, LOGGER_FORMATTING_BUFFER_SIZE, format_string, vArgs);
    va_end(vArgs);

    if (string_length >= 0)
    {
        print_buffer(logger_buffer, string_length);
        if (string_length > LOGGER_FORMATTING_BUFFER_SIZE)
        {
            print_buffer_full_error();
        }
    }
    else
    {
        print_encoding_error();
    }
}

void logger_internal_print_new_line(const char *new_line_string)
{
#if LOGGER_USE_STATIC_FORMATTING_BUFFER == 0
    char logger_buffer[LOGGER_FORMATTING_BUFFER_SIZE];
#endif

    int string_length = snprintf(logger_buffer, LOGGER_FORMATTING_BUFFER_SIZE, "%s", new_line_string);
    if (string_length >= 0)
    {
        print_buffer(logger_buffer, string_length);
    }
    else
    {
        print_encoding_error();
    }
}

static void print_buffer(char *p_data, size_t length)
{
#if defined(FreeRTOS) && defined(LOGGER_USE_EXTERNAL_THREAD)

    uint32_t IPSR_register = 0U;

    __asm volatile("MRS %0, ipsr" : "=r"(IPSR_register));

    if (xStreamBufferIsFull(sbuffer_logger_h) == pdFALSE)
    {
        // We intentionally do not check what xStreamBufferSend functions return,
        // because not much to do to handle the errors.
        if (0U == IPSR_register)
            xStreamBufferSend(sbuffer_logger_h, p_data, length, pdMS_TO_TICKS(LOGGER_STREAM_BUFFER_TIMEOUT_MS));
        else
            xStreamBufferSendFromISR(sbuffer_logger_h, p_data, length, NULL);
    }
#else
    (void) length;
    printf("%s", p_data);
#endif
}

static void print_encoding_error(void)
{
    printf("<encoding error>");
}

static void print_buffer_full_error(void)
{
    printf("<truncated>");
}
