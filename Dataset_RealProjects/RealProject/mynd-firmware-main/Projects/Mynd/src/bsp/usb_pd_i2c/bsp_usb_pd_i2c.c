#include <stdbool.h>
#include "platform/stm32/i2c_freertos.h"
#include "app_assert/app_assert.h"
#include "bsp_usb_pd_i2c.h"
#include "board_hw.h"
#include "logger.h"

I2C_HandleTypeDef          I2C1_Handle;
static i2c_rtos_handler_t *i2c_rtos_h;

void bsp_usb_pd_i2c_init(void)
{
    I2C1_Handle.Instance              = USB_PD_I2C;
    I2C1_Handle.Init.Timing           = USB_PD_I2C_TIMING;
    I2C1_Handle.Init.OwnAddress1      = 0;
    I2C1_Handle.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    I2C1_Handle.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    I2C1_Handle.Init.OwnAddress2      = 0;
    I2C1_Handle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    I2C1_Handle.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    I2C1_Handle.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;

    i2c_rtos_h = i2c_rtos_init(&I2C1_Handle);
    APP_ASSERT(i2c_rtos_h);
}

void bsp_usb_pd_i2c_msp_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /* Enable GPIO SCL/SDA clock */
    USB_PD_I2C_SCL_GPIO_CLK_ENABLE();
    USB_PD_I2C_SDA_GPIO_CLK_ENABLE();

    /* I2C SCL GPIO pin configuration */
    GPIO_InitStruct.Pin       = USB_PD_I2C_SCL_GPIO_PIN;
    GPIO_InitStruct.Mode      = USB_PD_I2C_SCL_GPIO_MODE;
    GPIO_InitStruct.Pull      = USB_PD_I2C_SCL_GPIO_PULL;
    GPIO_InitStruct.Speed     = USB_PD_I2C_SCL_GPIO_SPEED;
    GPIO_InitStruct.Alternate = USB_PD_I2C_SCL_GPIO_AF;
    HAL_GPIO_Init(USB_PD_I2C_SCL_GPIO_PORT, &GPIO_InitStruct);

    /* I2C SDA GPIO pin configuration */
    GPIO_InitStruct.Pin       = USB_PD_I2C_SDA_GPIO_PIN;
    GPIO_InitStruct.Mode      = USB_PD_I2C_SDA_GPIO_MODE;
    GPIO_InitStruct.Pull      = USB_PD_I2C_SDA_GPIO_PULL;
    GPIO_InitStruct.Speed     = USB_PD_I2C_SDA_GPIO_SPEED;
    GPIO_InitStruct.Alternate = USB_PD_I2C_SDA_GPIO_AF;
    HAL_GPIO_Init(USB_PD_I2C_SDA_GPIO_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(USB_PD_I2C_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(USB_PD_I2C_IRQn);

    /* Enable I2C clock */
    USB_PD_I2C_CLK_ENABLE();
}

int bsp_usb_pd_i2c_write(uint8_t i2c_address, uint8_t register_address, const uint8_t *p_buffer, uint32_t length)
{
    int error;
    int retries = 3;

    do
    {
        // Write the data
        error = i2c_rtos_write_data(i2c_rtos_h, i2c_address, register_address, 1, (uint8_t *) p_buffer, length);

        if (error < 0)
        {
            log_err("I2C write failed (address 0x%02X, register 0x%02X)", i2c_address, register_address);
        }

        retries--;
    } while (error < 0 && retries > 0);
    return error;
}

int bsp_usb_pd_i2c_read(uint8_t i2c_address, uint8_t register_address, uint8_t *p_buffer, uint32_t length)
{
    int error;
    int retries = 3;

    do
    {
        error = i2c_rtos_read_data(i2c_rtos_h, i2c_address, register_address, 1, p_buffer, length);

        if (error < 0)
        {
            log_err("I2C read failed (address 0x%02X, register 0x%02X)", i2c_address, register_address);
        }

        retries--;
    } while (error < 0 && retries > 0);

    return error;
}
