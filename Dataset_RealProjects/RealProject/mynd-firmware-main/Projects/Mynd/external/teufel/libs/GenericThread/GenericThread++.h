#pragma once

#include <cstdint>
#include <variant>
#include <cassert>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#ifdef TEUFEL_LOGGER
#include "logger.h"
#else
#include "dbg_log.h"
#endif

namespace Teufel::GenericThread
{

template <typename T>
struct QueueMessage
{
    uint8_t mid;
    T       payload;
};


template <typename T>
struct Config
{
    const char   *Name;
    uint32_t      StackSize;
    unsigned long Priority;
    uint32_t      IdleMs;
    void (*Callback_Idle)();
    void (*Callback_Init)();

    uint8_t QueueSize;

    void (*Callback)(uint8_t mid, T msg);

#if defined(configSUPPORT_STATIC_ALLOCATION) && (configSUPPORT_STATIC_ALLOCATION == 1)
    StackType_t *StackBuffer;
    StaticTask_t *StaticTask;

    StaticQueue_t *StaticQueue;
    uint8_t *QueueBuffer;
#endif
};

template <>
struct Config<void>
{
    const char   *Name;
    uint32_t      StackSize;
    unsigned long Priority;
    uint32_t      IdleMs;
    void (*Callback_Idle)();
    void (*Callback_Init)();
};

template <typename T>
struct GenericThread
{
    const Config<T> *config;
    TaskHandle_t     task;
    QueueHandle_t    queue;
    uint32_t         idle_ms;

#if defined(GENERIC_THREAD_ENABLE_STATS)
    struct
    {
        uint8_t  MaxQueueSize;
        uint16_t MinStackSize_bytes;
    } stats;
#endif
};

template <typename T>
[[noreturn]] static void task_loop(void *pvParameters)
{
    // TODO: add  static_assert(is_variant....

    auto             gthread = (GenericThread<T> *) pvParameters;
    const Config<T> *config  = gthread->config;

    if (config->Callback_Init)
    {
        log_trace("[%s] Init\n", config->Name);
        config->Callback_Init();
    }

    if constexpr (std::is_same_v<T, void>)
    {
        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(gthread->idle_ms));
            if (config->Callback_Idle)
            {
                config->Callback_Idle();
            }
        }
    }
    else
    {
        QueueMessage<T> msg;

        while (true)
        {
            if (xQueueReceive(gthread->queue, (void *) &(msg), pdMS_TO_TICKS(gthread->idle_ms)))
            {
                config->Callback(msg.mid, msg.payload);
            }
            else
            {
                if (config->Callback_Idle)
                {
                    config->Callback_Idle();
                }
#if defined(GENERIC_THREAD_ENABLE_STATS)
                gthread->stats.MinStackSize_bytes = uxTaskGetStackHighWaterMark2(gthread->task) * 4;
#endif
            }
        }
    }
}

template <typename T>
GenericThread<T> *create(const Config<T> *config)
{
    // TODO: add  static_assert(is_variant....

    auto gthread = (GenericThread<T> *) new GenericThread<T>;
    assert(gthread != nullptr);

    gthread->config  = config;
    gthread->idle_ms = config->IdleMs;
    gthread->task    = nullptr;
    gthread->queue   = nullptr;
#if defined(GENERIC_THREAD_ENABLE_STATS)
    GenThread->stats.MaxQueueSize       = 0;
    GenThread->stats.MinStackSize_bytes = config->StackSize * 4;
#endif

    log_trace("[%s] Create Message Queue\n", config->Name);
    if constexpr (!std::is_same_v<T, void>)
    {
        QueueMessage<T> msg;

#if defined(configSUPPORT_DYNAMIC_ALLOCATION) && (configSUPPORT_DYNAMIC_ALLOCATION == 1)
        gthread->queue = xQueueCreate(config->QueueSize, sizeof(QueueMessage<T>));
#else
        gthread->queue = xQueueCreateStatic(config->QueueSize, sizeof(QueueMessage<T>), config->QueueBuffer, config->StaticQueue);
#endif
        assert(gthread->queue != nullptr);
    }

    /* Note: If your code broke when I moved this, you were relying on uninitialized values. */

#if defined(configSUPPORT_DYNAMIC_ALLOCATION) && (configSUPPORT_DYNAMIC_ALLOCATION == 1)
    BaseType_t xErr;
    xErr =
        xTaskCreate(task_loop<T>, config->Name, config->StackSize, (void *) gthread, config->Priority, &gthread->task);

    if (xErr == pdPASS)
    {
        log_dbg("Create thread: %s (%dB, prio: %u): OK", config->Name, config->StackSize * 4, config->Priority);
    }
    else
    {
        log_dbg("Create thread: %s (%dB, prio: %u): Fail", config->Name, config->StackSize * 4, config->Priority);
        assert(false);
    }

#else
    TaskHandle_t xHandle = NULL;
    xHandle = xTaskCreateStatic(task_loop<T>, config->Name, config->StackSize, (void *) gthread, config->Priority,
                             config->StackBuffer, config->StaticTask);
    assert(xHandle);
#endif

    return gthread;
}

template <typename T>
int PostMsg(GenericThread<T> *gthread, uint8_t mid, T msg)
{
    if constexpr (std::is_same_v<T, void>)
        return 0;

    int        error                    = 0;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if ((!gthread) || (!gthread->queue))
    {
        return -1;
    }

    QueueMessage<T> txmsg = {
        .mid     = mid,
        .payload = msg,
    };

    uint32_t IPSR_register;
    __asm volatile("MRS %0, ipsr" : "=r"(IPSR_register));

    if (0U == IPSR_register)
    {
        if (xQueueSend(gthread->queue, (void *) &txmsg, (TickType_t) 100) != pdPASS)
        {
            log_err("{%s} Post Msg from failed (mem left: %d)", pcTaskGetName(gthread->task),
                    uxQueueSpacesAvailable(gthread->queue));
            error = -1;
        }
    }
    else
    {
        if (xQueueSendFromISR(gthread->queue, (void *) &txmsg, &xHigherPriorityTaskWoken) != pdTRUE)
        {
            // log_err("[ISR] Post Msg failed for \"%s\" with event:%d", pcTaskGetName(gen_thread->Task), event);
            error = -2;
        }
        // Switch context if necessary.
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

#if defined(GENERIC_THREAD_ENABLE_STATS)
    uint32_t queueSize          = gthread->config->QueueSize - uxQueueSpacesAvailable(gthread->queue);
    gthread->stats.MaxQueueSize = MAX(queueSize, gthread->stats.MaxQueueSize);
#endif

    return error;
}

#if defined(GENERIC_THREAD_ENABLE_STATS)
uint16_t getMinStackSize(GenericThread_t *gthread);
uint8_t  getMaxQueueSize(GenericThread_t *gthread);
#endif

}
