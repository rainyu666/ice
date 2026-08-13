#include <stdbool.h>
#include "bsp_shared_i2c.h"
#include "platform/stm32/i2c_freertos.h"
#include "board_hw.h"
#include "logger.h"
#include "app_assert/app_assert.h"

I2C_HandleTypeDef          I2C2_Handle;
static bool                s_is_initialized;
static i2c_rtos_handler_t *i2c_rtos_h;

void bsp_shared_i2c_init(void)
{
    if (s_is_initialized)
    {
        return;
    }

    I2C2_Handle.Instance              = SHARED_I2C;
    I2C2_Handle.Init.Timing           = SHARED_I2C_TIMING;
    I2C2_Handle.Init.OwnAddress1      = 0;
    I2C2_Handle.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    I2C2_Handle.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    I2C2_Handle.Init.OwnAddress2      = 0;
    I2C2_Handle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    I2C2_Handle.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    I2C2_Handle.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;

    i2c_rtos_h = i2c_rtos_init(&I2C2_Handle);
    APP_ASSERT(i2c_rtos_h);

    s_is_initialized = true;
}

void bsp_shared_i2c_deinit(void)
{
    if (!s_is_initialized)
    {
        return;
    }

    // I2C_Deinit(&I2C2_Handle);
    // TODO: add i2c_deinit in i2c_freertos file
}

void bsp_shared_i2c_msp_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /* Enable GPIO SCL/SDA clock */
    SHARED_I2C_SCL_GPIO_CLK_ENABLE();
    SHARED_I2C_SDA_GPIO_CLK_ENABLE();

    /* I2C SCL GPIO pin configuration */
    GPIO_InitStruct.Pin       = SHARED_I2C_SCL_GPIO_PIN;
    GPIO_InitStruct.Mode      = SHARED_I2C_SCL_GPIO_MODE;
    GPIO_InitStruct.Pull      = SHARED_I2C_SCL_GPIO_PULL;
    GPIO_InitStruct.Speed     = SHARED_I2C_SCL_GPIO_SPEED;
    GPIO_InitStruct.Alternate = SHARED_I2C_SCL_GPIO_AF;
    HAL_GPIO_Init(SHARED_I2C_SCL_GPIO_PORT, &GPIO_InitStruct);

    /* I2C SDA GPIO pin configuration */
    GPIO_InitStruct.Pin       = SHARED_I2C_SDA_GPIO_PIN;
    GPIO_InitStruct.Mode      = SHARED_I2C_SDA_GPIO_MODE;
    GPIO_InitStruct.Pull      = SHARED_I2C_SDA_GPIO_PULL;
    GPIO_InitStruct.Speed     = SHARED_I2C_SDA_GPIO_SPEED;
    GPIO_InitStruct.Alternate = SHARED_I2C_SDA_GPIO_AF;
    HAL_GPIO_Init(SHARED_I2C_SDA_GPIO_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(SHARED_I2C_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(SHARED_I2C_IRQn);

    /* Enable I2C clock */
    SHARED_I2C_CLK_ENABLE();
}

void bsp_shared_i2c_msp_deinit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    SHARED_I2C_FORCE_RESET();
    SHARED_I2C_RELEASE_RESET();

    /* Configure I2C SDA&SCL GPIO as analog pin */
    GPIO_InitStruct.Pin   = SHARED_I2C_SCL_GPIO_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SHARED_I2C_SCL_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = SHARED_I2C_SDA_GPIO_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SHARED_I2C_SDA_GPIO_PORT, &GPIO_InitStruct);

    HAL_NVIC_DisableIRQ(SHARED_I2C_IRQn);

    SHARED_I2C_CLK_DISABLE();
}

int bsp_shared_i2c_write(uint8_t i2c_address, uint8_t register_address, const uint8_t *p_buffer, uint32_t length)
{
    int error;
    int retries = 3;

    if (!s_is_initialized)
    {
        log_error("Can't write on I2C before initialization");
        return -1;
    }

    do
    {
        // Write the data
        error = i2c_rtos_write_data(i2c_rtos_h, i2c_address, register_address, 1, (uint8_t *) p_buffer, length);

        if (error < 0)
        {
            log_err("I2C write failed (address 0x%02X, register 0x%02X, error %d)", i2c_address, register_address,
                    error);
        }

        retries--;
    } while (error < 0 && retries > 0);
    return error;
}

int bsp_shared_i2c_read(uint8_t i2c_address, uint8_t register_address, uint8_t *p_buffer, uint32_t length)
{
    int error;
    int retries = 3;

    if (!s_is_initialized)
    {
        log_error("Can't read on I2C before initialization");
        return -1;
    }

    do
    {
        error = i2c_rtos_read_data(i2c_rtos_h, i2c_address, register_address, 1, p_buffer, length);

        if (error < 0)
        {
            log_err("I2C read failed (address 0x%02X, register 0x%02X, error %d)", i2c_address, register_address,
                    error);
        }

        retries--;
    } while (error < 0 && retries > 0);
    return error;
}
