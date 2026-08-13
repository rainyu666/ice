#include "stm32f0xx_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "i2c_freertos.h"
#include "app_assert/app_assert.h"

#define I2C_RTOS_NUM 2

struct i2c_rtos_handler
{
    I2C_HandleTypeDef *hi2c;
    SemaphoreHandle_t  mutex;
    SemaphoreHandle_t  semaphore;
#if defined(configSUPPORT_STATIC_ALLOCATION) && (configSUPPORT_STATIC_ALLOCATION == 1)
    StaticSemaphore_t mutex_buffer;
    StaticSemaphore_t semaphore_buffer;
#endif
    uint32_t async_error_code;
};

static struct i2c_rtos_handler i2c_rtos_handlers[I2C_RTOS_NUM];

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    portENTER_CRITICAL();
    BaseType_t xHigherPriorityTaskWoken               = pdFALSE;
    int        handler_index                          = hi2c->Instance == I2C1 ? 0 : 1;
    i2c_rtos_handlers[handler_index].async_error_code = hi2c->ErrorCode;
    portEXIT_CRITICAL();
    xSemaphoreGiveFromISR(i2c_rtos_handlers[handler_index].semaphore, &xHigherPriorityTaskWoken);
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    portENTER_CRITICAL();
    BaseType_t xHigherPriorityTaskWoken               = pdFALSE;
    int        handler_index                          = hi2c->Instance == I2C1 ? 0 : 1;
    i2c_rtos_handlers[handler_index].async_error_code = hi2c->ErrorCode;
    portEXIT_CRITICAL();
    xSemaphoreGiveFromISR(i2c_rtos_handlers[handler_index].semaphore, &xHigherPriorityTaskWoken);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    portENTER_CRITICAL();
    BaseType_t xHigherPriorityTaskWoken               = pdFALSE;
    int        handler_index                          = hi2c->Instance == I2C1 ? 0 : 1;
    i2c_rtos_handlers[handler_index].async_error_code = hi2c->ErrorCode;
    portEXIT_CRITICAL();
    xSemaphoreGiveFromISR(i2c_rtos_handlers[handler_index].semaphore, &xHigherPriorityTaskWoken);
}

struct i2c_rtos_handler *i2c_rtos_init(I2C_HandleTypeDef *hi2c)
{
    int handler_index                     = hi2c->Instance == I2C1 ? 0 : 1;
    i2c_rtos_handlers[handler_index].hi2c = hi2c;
    i2c_rtos_handlers[handler_index].mutex =
        xSemaphoreCreateMutexStatic(&i2c_rtos_handlers[handler_index].mutex_buffer);
    APP_ASSERT(i2c_rtos_handlers[handler_index].mutex);

    i2c_rtos_handlers[handler_index].semaphore =
        xSemaphoreCreateBinaryStatic(&i2c_rtos_handlers[handler_index].semaphore_buffer);
    APP_ASSERT(i2c_rtos_handlers[handler_index].semaphore);

    HAL_I2C_Init(hi2c);

    HAL_I2CEx_ConfigAnalogFilter(hi2c, I2C_ANALOGFILTER_ENABLE);

    return &i2c_rtos_handlers[handler_index];
}

int i2c_rtos_write_data(struct i2c_rtos_handler *i2c_h, uint8_t i2c_address, uint16_t reg_address,
                        uint16_t reg_address_size, uint8_t *p_buffer, size_t length)
{
    int error = 0;

    if (xSemaphoreTake(i2c_h->mutex, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        error = -1;
        goto error_handler;
    }

    if (HAL_I2C_Mem_Write_IT(i2c_h->hi2c, i2c_address, reg_address, reg_address_size, p_buffer, length) != HAL_OK)
    {
        error = -2;
        goto error_handler;
    }

    // Wait for transfer to finish
    if (xSemaphoreTake(i2c_h->semaphore, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        error = -3;
        goto error_handler;
    }

    if (i2c_h->async_error_code)
    {
        error = -4;
        goto error_handler;
    }

    // Unlock resource mutex
    (void) xSemaphoreGive(i2c_h->mutex);

    return error;

error_handler:
    // Deinit I2C iface?!
    xSemaphoreGive(i2c_h->mutex);
    return error;
}

int i2c_rtos_read_data(struct i2c_rtos_handler *i2c_h, uint8_t i2c_address, uint16_t reg_address,
                       uint16_t reg_address_size, uint8_t *p_buffer, size_t length)
{
    int error = 0;

    if (xSemaphoreTake(i2c_h->mutex, pdMS_TO_TICKS(1000)) != pdTRUE)
    {
        error = -1;
        goto error_handler;
    }

    if (HAL_I2C_Mem_Read_IT(i2c_h->hi2c, i2c_address, reg_address, reg_address_size, p_buffer, length) != HAL_OK)
    {
        error = -2;
        goto error_handler;
    }

    // Wait for transfer to finish
    if (xSemaphoreTake(i2c_h->semaphore, pdMS_TO_TICKS(1000)) != pdTRUE)
    {
        error = -3;
        goto error_handler;
    }

    // Check for errors
    if (i2c_h->async_error_code)
    {
        error = -4;
        goto error_handler;
    }

    // Unlock resource mutex
    (void) xSemaphoreGive(i2c_h->mutex);

    return error;

error_handler:
    // Deinit I2C iface?!
    xSemaphoreGive(i2c_h->mutex);
    return error;
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
