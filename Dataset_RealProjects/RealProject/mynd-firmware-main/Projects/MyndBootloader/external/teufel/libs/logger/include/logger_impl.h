#pragma once
#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

/**
 * @brief Initializes the logger.
 */
#if defined(FreeRTOS) && defined(LOGGER_USE_EXTERNAL_THREAD)

#include "FreeRTOS.h"
#include "semphr.h"
#include "stream_buffer.h"

int logger_init(StreamBufferHandle_t sbuf);
#else
void logger_init(void);
#endif

/**
 * @brief Flushes the logger.
 */
void logger_flush(void);

/**
 * @brief Gets a system timestamp.
 * @return timestamp
 */
uint32_t logger_get_timestamp();

#if defined(__cplusplus)
}
#endif
