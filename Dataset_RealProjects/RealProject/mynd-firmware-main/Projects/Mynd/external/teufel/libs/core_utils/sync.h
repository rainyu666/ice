#pragma once

#include <cstdint>
#include "FreeRTOS.h"
#if defined(TEUFEL_LOGGER)
#include "logger.h"
#else
#include "dbg_log.h"
#endif

struct SyncPrimitive
{
    template <typename T>
    static int await(const T &t, uint32_t timeout_ms, const char *msg)
    {
        /* FreeRTOS Stream and Message Buffers use the task notification at array index 0.
         * If you want to maintain the state of a task notification across a call to a Stream,
         * or Message Buffer API function then use a task notification at an array index greater than 0.
         * In order to avoid collisions, please consider to use indexes starting from 1. */
        if (static_cast<uint32_t >(t) == 0)
            log_warn("SyncPrimitive (%s) Task notification at array index 0!", __func__);

        uint32_t ulNotificationValue =
            ulTaskNotifyTakeIndexed(static_cast<uint32_t>(t), pdTRUE, pdMS_TO_TICKS(timeout_ms));
        if (ulNotificationValue == 1)
        {
            log_info("Task %s %s", getDesc(t), msg);
            return 0;
        }

        log_err("%s: task %s sync timeout", __func__, getDesc(t));

        return 1;
    }

    template <typename T>
    static int notify(const T &t)
    {
        static TaskHandle_t xTaskToNotifySystem = nullptr;

        if (!xTaskToNotifySystem)
        {
            xTaskToNotifySystem = xTaskGetHandle("System");
        }

        if (xTaskToNotifySystem == nullptr)
        {
            log_err("Sys sync notify failed");
            return -1;
        }

        /* FreeRTOS Stream and Message Buffers use the task notification at array index 0.
         * If you want to maintain the state of a task notification across a call to a Stream,
         * or Message Buffer API function then use a task notification at an array index greater than 0.
         * In order to avoid collisions, please consider to use indexes starting from 1. */
        if (static_cast<uint32_t >(t) == 0)
            log_warn("SyncPrimitive (%s) Task notification at array index 0!", __func__);

        xTaskNotifyGiveIndexed(xTaskToNotifySystem, static_cast<uint32_t>(t));

        return 0;
    }
};
