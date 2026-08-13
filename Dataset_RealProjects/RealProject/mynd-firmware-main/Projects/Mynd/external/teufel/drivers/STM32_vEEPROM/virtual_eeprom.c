#include <assert.h>

#define LOG_LEVEL LOG_LEVEL_ERROR
#include "driver_logger.h"

#ifdef FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif

#include "eeprom.h"

#ifdef FreeRTOS
static SemaphoreHandle_t xSemaphore = NULL;
#if defined(configSUPPORT_STATIC_ALLOCATION) && (configSUPPORT_STATIC_ALLOCATION == 1)
static StaticSemaphore_t sem_buffer;
#endif
#endif

static void vEEPROM_LockInit(void);
static void vEEPROM_Lock(void);
static void vEEPROM_Unlock(void);

static void vEEPROM_LockInit(void)
{
#ifdef FreeRTOS
    if (xSemaphore == NULL)
    {
#if defined(configSUPPORT_DYNAMIC_ALLOCATION) && (configSUPPORT_DYNAMIC_ALLOCATION == 1)
        xSemaphore = xSemaphoreCreateBinary();
#else
        xSemaphore = xSemaphoreCreateBinaryStatic(&sem_buffer);
#endif
        assert(xSemaphore);
        xSemaphoreGive(xSemaphore);
    }
#endif
}

static void vEEPROM_Lock(void)
{
#ifdef FreeRTOS
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
    {
        return;
    }

    assert(xSemaphore);
    xSemaphoreTake(xSemaphore, (TickType_t) portMAX_DELAY);
#endif
}

static void vEEPROM_Unlock(void)
{
#ifdef FreeRTOS
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
    {
        return;
    }

    assert(xSemaphore);
    xSemaphoreGive(xSemaphore);
#endif
}

int vEEPROM_Init(void)
{
    vEEPROM_LockInit();

    HAL_FLASH_Lock();
    HAL_FLASH_Unlock();

    if (EE_Init() != 0)
    {
        prompt_driver_init_failed("vEEprom");
        return -1;
    }

    HAL_FLASH_Lock();

    prompt_driver_init_ok("vEEprom");

    return 0;
}

int vEEPROM_AddressWrite(uint16_t addr, uint16_t value)
{
    int      err = 0;
    uint16_t tmp;
    uint8_t  update = 1;

    vEEPROM_Lock();

    if (EE_ReadVariable(addr, &tmp) == 0)
    {
        if (tmp == value)
        {
            update = 0;
        }
    }

    if (update)
    {
        dev_dbg("[vEEprom] W 0x%04x: 0x%04x", addr, value);
        HAL_FLASH_Unlock();
        err = EE_WriteVariable(addr, value);
        HAL_FLASH_Lock();
        if (err)
        {
            dev_err("[vEEprom] W Failed: 0x%04x", addr);
        }
    }

    vEEPROM_Unlock();

    return err;
}

int vEEPROM_AddressWriteBuffer(uint16_t addr, const uint16_t *data, uint16_t size)
{
    int err = 0;

    vEEPROM_Lock();
    for (uint16_t i = 0U; i < size; ++i)
    {
        uint16_t tmp = 0U;
        int      update = 1;
        if (EE_ReadVariable(addr + i, &tmp) == 0)
        {
            if (tmp == data[i])
            {
                update = 0;
            }
        }

        if (update)
        {
            dev_dbg("[vEEprom] W 0x%04x: 0x%04x", addr + i, tmp);
            HAL_FLASH_Unlock();
            err = EE_WriteVariable(addr + i, data[i]);
            HAL_FLASH_Lock();
            if (err)
            {
                dev_err("[vEEprom] W Failed: 0x%04x", addr + i);
                break;
            }
        }
    }
    vEEPROM_Unlock();

    return err;
}

int vEEPROM_AddressRead(uint16_t addr, uint16_t *value)
{
    int err = 0;

    vEEPROM_Lock();

    err = EE_ReadVariable(addr, value);
    if (err)
    {
        dev_err("[vEEprom] R Failed: 0x%04x", addr);
    }

    vEEPROM_Unlock();

    return err;
}

int vEEPROM_AddressReadBuffer(uint16_t addr, uint16_t *target, uint16_t size)
{
    int err = 0;
    vEEPROM_Lock();
    for (uint16_t i = 0U; i < size; ++i)
    {
        err = EE_ReadVariable(addr + i, &target[i]);
        if (err)
        {
            dev_err("[vEEprom] R Failed: 0x%04x", addr + i);
            break;
        }
    }
    vEEPROM_Unlock();

    return err;
}
