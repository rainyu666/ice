#include <stdbool.h>
#include "bsp_debug_uart.h"
#include "board_hw.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_ll_usart.h"
#include "logger.h"

UART_HandleTypeDef UART2_Handle;

void bsp_debug_uart_init(void)
{
    UART2_Handle.Instance                    = DEBUG_UART;
    UART2_Handle.Init.BaudRate               = DEBUG_UART_BAUDRATE;
    UART2_Handle.Init.WordLength             = UART_WORDLENGTH_8B;
    UART2_Handle.Init.StopBits               = UART_STOPBITS_1;
    UART2_Handle.Init.Parity                 = UART_PARITY_NONE;
    UART2_Handle.Init.Mode                   = UART_MODE_TX_RX;
    UART2_Handle.Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    UART2_Handle.Init.OverSampling           = UART_OVERSAMPLING_16;
    UART2_Handle.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    UART2_Handle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    HAL_UART_Init(&UART2_Handle);
}

void bsp_debug_uart_msp_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /* Gpio clock enable */
    DEBUG_UART_TX_GPIO_CLK_ENABLE();
    DEBUG_UART_RX_GPIO_CLK_ENABLE();

    /* USART RX/TX GPIO Configuration */
    GPIO_InitStruct.Pin       = DEBUG_UART_TX_GPIO_PIN;
    GPIO_InitStruct.Mode      = DEBUG_UART_TX_GPIO_MODE;
    GPIO_InitStruct.Pull      = DEBUG_UART_TX_GPIO_PULL;
    GPIO_InitStruct.Speed     = DEBUG_UART_TX_GPIO_SPEED;
    GPIO_InitStruct.Alternate = DEBUG_UART_TX_GPIO_AF;
    HAL_GPIO_Init(DEBUG_UART_TX_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = DEBUG_UART_RX_GPIO_PIN;
    GPIO_InitStruct.Mode      = DEBUG_UART_RX_GPIO_MODE;
    GPIO_InitStruct.Pull      = DEBUG_UART_RX_GPIO_PULL;
    GPIO_InitStruct.Speed     = DEBUG_UART_RX_GPIO_SPEED;
    GPIO_InitStruct.Alternate = DEBUG_UART_RX_GPIO_AF;
    HAL_GPIO_Init(DEBUG_UART_RX_GPIO_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(DEBUG_UART_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DEBUG_UART_IRQn);

    DEBUG_UART_CLK_ENABLE();
}

int bsp_debug_uart_tx(const uint8_t *p_data, size_t length)
{
    HAL_UART_Transmit(&UART2_Handle, (uint8_t *) p_data, length, HAL_MAX_DELAY);
    return 0;
}
