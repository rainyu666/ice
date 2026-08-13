#include <stdbool.h>
#include "stm32f0xx_hal.h"
#include "board_hw.h"
#include "external/teufel/libs/app_assert/app_assert.h"
#include "bsp_debug_uart.h"
#include "bsp_shared_i2c.h"

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == BLUETOOTH_UART)
    {
        // bsp_bluetooth_uart_msp_init();
    }
    else if (huart->Instance == DEBUG_UART)
    {
        bsp_debug_uart_msp_init();
    }
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == USB_PD_I2C)
    {
        // bsp_usb_pd_i2c_msp_init();
    }
    else if (hi2c->Instance == SHARED_I2C)
    {
        bsp_shared_i2c_msp_init();
    }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == USB_PD_I2C)
    {
        GPIO_InitTypeDef GPIO_InitStruct;

        USB_PD_I2C_FORCE_RESET();
        USB_PD_I2C_RELEASE_RESET();

        /* Configure I2C SDA&SCL GPIO as analog pin */
        GPIO_InitStruct.Pin   = USB_PD_I2C_SCL_GPIO_PIN;
        GPIO_InitStruct.Mode  = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull  = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(USB_PD_I2C_SCL_GPIO_PORT, &GPIO_InitStruct);

        GPIO_InitStruct.Pin   = USB_PD_I2C_SDA_GPIO_PIN;
        GPIO_InitStruct.Mode  = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull  = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(USB_PD_I2C_SDA_GPIO_PORT, &GPIO_InitStruct);

        HAL_NVIC_DisableIRQ(USB_PD_I2C_IRQn);

        USB_PD_I2C_CLK_DISABLE();
    }
    else if (hi2c->Instance == SHARED_I2C)
    {
        bsp_shared_i2c_msp_deinit();
    }
    else
    {
        APP_ASSERT(false);
    }
}

/**
 * @brief  Initializes the PCD MSP.
 * @param  hpcd: PCD handle
 * @retval None
 */
void HAL_PCD_MspInit(PCD_HandleTypeDef *hpcd)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    USB_D_P_GPIO_CLK_ENABLE();
    USB_D_M_GPIO_CLK_ENABLE();

    /* Configure USB Plus Pin */
    GPIO_InitStruct.Pin       = USB_D_P_GPIO_PIN;
    GPIO_InitStruct.Mode      = USB_D_P_GPIO_MODE;
    GPIO_InitStruct.Pull      = USB_D_P_GPIO_PULL;
    GPIO_InitStruct.Alternate = USB_D_P_GPIO_AF;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(USB_D_P_GPIO_PORT, &GPIO_InitStruct);

    /* Configure USB Minus Pin */
    GPIO_InitStruct.Pin       = USB_D_M_GPIO_PIN;
    GPIO_InitStruct.Mode      = USB_D_M_GPIO_MODE;
    GPIO_InitStruct.Pull      = USB_D_M_GPIO_PULL;
    GPIO_InitStruct.Alternate = USB_D_M_GPIO_AF;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(USB_D_M_GPIO_PORT, &GPIO_InitStruct);

    /* Enable USB FS Clock */
    __HAL_RCC_USB_CLK_ENABLE();

    /* Set USB FS Interrupt priority */
    HAL_NVIC_SetPriority(USB_IRQn, 3, 0);

    /* Enable USB FS Interrupt */
    HAL_NVIC_EnableIRQ(USB_IRQn);
}

/**
 * @brief  De-Initializes the PCD MSP.
 * @param  hpcd: PCD handle
 * @retval None
 */
void HAL_PCD_MspDeInit(PCD_HandleTypeDef *hpcd)
{
    /* Disable USB FS Clock */
    __HAL_RCC_USB_CLK_DISABLE();
}
