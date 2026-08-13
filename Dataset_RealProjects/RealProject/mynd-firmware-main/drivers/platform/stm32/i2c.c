#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#ifdef FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif

#include "platform/stm32/i2c.h"

#define LOCKS_I2C_NUM 3

static int  I2C_LockInit(I2C_HandleTypeDef *hi2c);
static int  I2C_Lock(I2C_HandleTypeDef *hi2c);
static void I2C_Unlock(I2C_HandleTypeDef *hi2c);

static void I2C_Error_Handle(I2C_HandleTypeDef *hi2c);

#ifdef FreeRTOS
struct sems
{
    I2C_HandleTypeDef *hi2c;
    SemaphoreHandle_t  sem;
#if defined(configSUPPORT_STATIC_ALLOCATION) && (configSUPPORT_STATIC_ALLOCATION == 1)
    StaticSemaphore_t sem_buffer;
#endif
};

static struct sems interface_locks[LOCKS_I2C_NUM];

#endif

#ifdef FreeRTOS
static SemaphoreHandle_t get_sem_by_handler(struct sems *locks, I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL)
    {
        return NULL;
    }

    for (int i = 0; i < LOCKS_I2C_NUM; i++)
    {
        if ((locks + i)->hi2c == hi2c)
        {
            return (locks + i)->sem;
        }
    }

    return NULL;
}
#endif

#ifdef FreeRTOS
static int add_sem(struct sems *locks, I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL)
    {
        return -1;
    }

    for (int i = 0; i < LOCKS_I2C_NUM; i++)
    {
        if ((locks + i)->hi2c == NULL)
        {
#if defined(configSUPPORT_DYNAMIC_ALLOCATION) && (configSUPPORT_DYNAMIC_ALLOCATION == 1)
            (locks + i)->sem = xSemaphoreCreateMutex();
#else
            (locks + i)->sem = xSemaphoreCreateMutexStatic(&((locks + i)->sem_buffer));
#endif
            assert(locks->sem);
            (locks + i)->hi2c = hi2c;
            return 0;
        }
    }

    return 0;
}
#endif

static int I2C_LockInit(I2C_HandleTypeDef *hi2c)
{
#ifdef FreeRTOS
    SemaphoreHandle_t sem = get_sem_by_handler(&interface_locks[0], hi2c);

    if (sem == NULL)
    {
        add_sem(&interface_locks[0], hi2c);
    }
#endif

    return 0;
}

static int I2C_Lock(I2C_HandleTypeDef *hi2c)
{
#ifdef FreeRTOS
    SemaphoreHandle_t sem = get_sem_by_handler(&interface_locks[0], hi2c);
    if (sem != NULL)
    {
        if (xSemaphoreTake(sem, (TickType_t) 500) != pdTRUE)
        {
            return -1;
        }
    }
#endif

    return 0;
}

static void I2C_Unlock(I2C_HandleTypeDef *hi2c)
{
#ifdef FreeRTOS
    SemaphoreHandle_t sem = get_sem_by_handler(&interface_locks[0], hi2c);
    if (sem != NULL)
    {
        xSemaphoreGive(sem);
    }
#endif
}

#if 0
static void I2C_UnlockISR(I2C_HandleTypeDef *hi2c)
{
#ifdef FreeRTOS
    static BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;

    SemaphoreHandle_t sem = get_sem_by_handler(&interface_locks[0], hi2c);
    if (sem) {
        xSemaphoreGiveFromISR(sem, xHigherPriorityTaskWoken);
    }
#endif
}
#endif

void I2C_Init(I2C_HandleTypeDef *hi2c)
{
    if (I2C_LockInit(hi2c) == 0)
    {
        I2C_Lock(hi2c);
    }

    HAL_I2C_Init(hi2c);

    HAL_I2CEx_ConfigAnalogFilter(hi2c, I2C_ANALOGFILTER_ENABLE);

    I2C_Unlock(hi2c);
}

void I2C_Deinit(I2C_HandleTypeDef *hi2c)
{
    I2C_Lock(hi2c);
    HAL_I2C_DeInit(hi2c);
    I2C_Unlock(hi2c);
}

uint32_t I2C_Read_Reg8(I2C_HandleTypeDef *hi2c, uint8_t Addr, uint8_t Reg, uint8_t *pData, uint8_t len)
{
    HAL_StatusTypeDef status_hal;

    I2C_Lock(hi2c);

#ifdef FreeRTOS
    vTaskSuspendAll();
#endif

    status_hal = hi2c->Instance->RXDR;

    status_hal = HAL_I2C_Mem_Read(hi2c, Addr, Reg, I2C_MEMADD_SIZE_8BIT, pData, len, 100);
#ifdef FreeRTOS
    xTaskResumeAll();
#endif
    I2C_Unlock(hi2c);

    if (status_hal != HAL_OK)
    {
        // Keep it for handling errors
        I2C_Error_Handle(hi2c);
    }

    return status_hal;
}

uint32_t I2C_Write_Reg8(I2C_HandleTypeDef *hi2c, uint8_t Addr, uint8_t Reg, uint8_t *pData, uint8_t len)
{
    HAL_StatusTypeDef status_hal;

    I2C_Lock(hi2c);

#ifdef FreeRTOS
    vTaskSuspendAll();
#endif
    status_hal = HAL_I2C_Mem_Write(hi2c, Addr, Reg, I2C_MEMADD_SIZE_8BIT, pData, len, 100);
#ifdef FreeRTOS
    xTaskResumeAll();
#endif

    I2C_Unlock(hi2c);

    if (status_hal != HAL_OK)
    {
        // Keep it for handling errors
        I2C_Error_Handle(hi2c);
    }

    return status_hal;
}

uint32_t I2C_Write_Reg16(I2C_HandleTypeDef *hi2c, uint8_t Addr, uint16_t memAddr, uint8_t *pData, uint16_t len)
{
    HAL_StatusTypeDef status_hal;

    I2C_Lock(hi2c);

#ifdef FreeRTOS
    vTaskSuspendAll();
#endif
    status_hal = HAL_I2C_Mem_Write(hi2c, Addr, memAddr, I2C_MEMADD_SIZE_16BIT, pData, len, 100);
#ifdef FreeRTOS
    xTaskResumeAll();
#endif

    I2C_Unlock(hi2c);

    if (status_hal != HAL_OK)
    {
        // Keep it for handling errors
        I2C_Error_Handle(hi2c);
    }

    return status_hal;
}

uint32_t I2C_Write_Reg16_DMA(I2C_HandleTypeDef *hi2c, uint8_t Addr, uint16_t memAddr, uint8_t *pData, uint16_t len)
{
    HAL_StatusTypeDef status_hal;

    I2C_Lock(hi2c);

#ifdef FreeRTOS
    vTaskSuspendAll();
#endif
    status_hal = HAL_I2C_Mem_Write_DMA(hi2c, Addr, memAddr, I2C_MEMADD_SIZE_16BIT, pData, len);
#ifdef FreeRTOS
    xTaskResumeAll();
#endif
    if (status_hal != HAL_OK)
    {
        I2C_Error_Handle(hi2c);
    }

    uint32_t timeout = HAL_GetTick() + 100;
    while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY)
    {
        HAL_Delay(2);
        if (HAL_GetTick() > timeout)
        {
            status_hal = HAL_TIMEOUT;
            // FIXME: Do some error handling?
            break;
        }
    }

    I2C_Unlock(hi2c);

    return status_hal;
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    UNUSED(hi2c);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    UNUSED(hi2c);

    //    log_err("I2C Error Callback: %s\r\n", __func__);

    I2C_Error_Handle(hi2c);
}

uint32_t I2C_Write_Data(I2C_HandleTypeDef *hi2c, uint8_t Addr, uint8_t *pData, uint16_t len)
{
    HAL_StatusTypeDef status_hal;

    if (hi2c->State == HAL_I2C_STATE_RESET)
    {
        return HAL_ERROR;
    }

    I2C_Lock(hi2c);

    status_hal = hi2c->Instance->RXDR;
#ifdef FreeRTOS
    vTaskSuspendAll();
#endif
    status_hal = HAL_I2C_Master_Transmit(hi2c, Addr, pData, len, 100);
#ifdef FreeRTOS
    xTaskResumeAll();
#endif

    if (status_hal != HAL_OK)
    {
        I2C_Error_Handle(hi2c);
    }

    I2C_Unlock(hi2c);

    return status_hal;
}

uint32_t I2C_Read_Data(I2C_HandleTypeDef *hi2c, uint8_t Addr, uint8_t *pData, uint16_t len)
{
    HAL_StatusTypeDef status_hal;

    if (hi2c->State == HAL_I2C_STATE_RESET)
    {
        return HAL_ERROR;
    }

    I2C_Lock(hi2c);
#ifdef FreeRTOS
    vTaskSuspendAll();
#endif
    status_hal = hi2c->Instance->RXDR;

    status_hal = HAL_I2C_Master_Receive(hi2c, Addr, pData, len, 100);
#ifdef FreeRTOS
    xTaskResumeAll();
#endif

    if (status_hal != HAL_OK)
    {
        I2C_Error_Handle(hi2c);
    }

    I2C_Unlock(hi2c);

    return status_hal;
}

uint32_t I2C_IsDeviceReady(I2C_HandleTypeDef *hi2c, uint8_t Addr, uint32_t trials, uint32_t timeout)
{
    HAL_StatusTypeDef status_hal;

    if (hi2c->State == HAL_I2C_STATE_RESET)
    {
        return HAL_ERROR;
    }

    I2C_Lock(hi2c);
#ifdef FreeRTOS
    vTaskSuspendAll();
#endif

    status_hal = HAL_I2C_IsDeviceReady(hi2c, Addr, trials, timeout);
#ifdef FreeRTOS
    xTaskResumeAll();
#endif

    I2C_Unlock(hi2c);

    return status_hal;
}

__attribute__((optimize("-O0"))) static void I2C_Error_Handle(I2C_HandleTypeDef *hi2c)
{
    UNUSED(hi2c);
    // Disable the peripheral
    // The I2C SDA/SCL lines are released and the internal state machines and status
    // bits should be reset by hardware back to default values
    hi2c->Instance->CR1 &= ~0x00000001;

    // The PE bit must remain low for at least 3 APB clock cycles
    // TODO: Make this delay more accurate
    //       APB CLK is derived from SYSCLK with two prescalers that can go up to 256 * 16,
    //       but it's fairly unlikely that we'll be running the APB clock that slow
    for (int i = 0; i < 300; i++)
    {
        __NOP();
    }

    // Reenable the peripheral
    hi2c->Instance->CR1 |= 0x00000001;
}
