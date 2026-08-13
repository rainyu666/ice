#include "bsp_bluetooth_uart.h"
#include "board_hw.h"
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "task.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_ll_usart.h"
#include "logger.h"
#include <stdbool.h>

UART_HandleTypeDef          UART1_Handle;
static StreamBufferHandle_t sbuffer_handle_rx;
static volatile uint8_t     irq_rx_data[1] = {};
static volatile bool        missed_rx_data = false;

#define STORAGE_SIZE_BYTES 128u
static uint8_t              sbuffer_storage[STORAGE_SIZE_BYTES];
static StaticStreamBuffer_t StreamBufferStruct;

void bsp_bluetooth_uart_init(void)
{
    sbuffer_handle_rx = xStreamBufferCreateStatic(sizeof(sbuffer_storage), 0u, sbuffer_storage, &StreamBufferStruct);
    if (sbuffer_handle_rx == NULL)
    {
        log_err("Failed to create stream buffer");
        return;
    }

    UART1_Handle.Instance                    = BLUETOOTH_UART;
    UART1_Handle.Init.BaudRate               = BLUETOOTH_UART_BAUDRATE;
    UART1_Handle.Init.WordLength             = UART_WORDLENGTH_8B;
    UART1_Handle.Init.StopBits               = UART_STOPBITS_1;
    UART1_Handle.Init.Parity                 = UART_PARITY_NONE;
    UART1_Handle.Init.Mode                   = UART_MODE_TX_RX;
    UART1_Handle.Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    UART1_Handle.Init.OverSampling           = UART_OVERSAMPLING_16;
    UART1_Handle.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    UART1_Handle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    HAL_UART_Init(&UART1_Handle);

    // Trigger receiving
    HAL_UART_Receive_IT(&UART1_Handle, (uint8_t *) irq_rx_data, 1);
}

void bsp_bluetooth_uart_msp_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // Gpio clock enable
    BLUETOOTH_UART_TX_GPIO_CLK_ENABLE();
    BLUETOOTH_UART_RX_GPIO_CLK_ENABLE();

    // USART RX/TX GPIO Configuration
    GPIO_InitStruct.Pin       = BLUETOOTH_UART_TX_GPIO_PIN;
    GPIO_InitStruct.Mode      = BLUETOOTH_UART_TX_GPIO_MODE;
    GPIO_InitStruct.Pull      = BLUETOOTH_UART_TX_GPIO_PULL;
    GPIO_InitStruct.Speed     = BLUETOOTH_UART_TX_GPIO_SPEED;
    GPIO_InitStruct.Alternate = BLUETOOTH_UART_TX_GPIO_AF;
    HAL_GPIO_Init(BLUETOOTH_UART_TX_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = BLUETOOTH_UART_RX_GPIO_PIN;
    GPIO_InitStruct.Mode      = BLUETOOTH_UART_RX_GPIO_MODE;
    GPIO_InitStruct.Pull      = BLUETOOTH_UART_RX_GPIO_PULL;
    GPIO_InitStruct.Speed     = BLUETOOTH_UART_RX_GPIO_SPEED;
    GPIO_InitStruct.Alternate = BLUETOOTH_UART_RX_GPIO_AF;
    HAL_GPIO_Init(BLUETOOTH_UART_RX_GPIO_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(BLUETOOTH_UART_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(BLUETOOTH_UART_IRQn);

    BLUETOOTH_UART_CLK_ENABLE();
}

void bsp_bluetooth_uart_msp_deinit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    HAL_NVIC_DisableIRQ(BLUETOOTH_UART_IRQn);

    // Configure USART Tx as alternate function
    GPIO_InitStruct.Pin       = BLUETOOTH_UART_TX_GPIO_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = 0;
    HAL_GPIO_Init(BLUETOOTH_UART_TX_GPIO_PORT, &GPIO_InitStruct);

    // Configure USART Rx as alternate function
    GPIO_InitStruct.Pin       = BLUETOOTH_UART_RX_GPIO_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = 0;
    HAL_GPIO_Init(BLUETOOTH_UART_RX_GPIO_PORT, &GPIO_InitStruct);

    // Peripheral clock disable
    BLUETOOTH_UART_CLK_DISABLE();
}

void bsp_bluetooth_uart_clear_buffer(void)
{
    if (xStreamBufferReset(sbuffer_handle_rx) != pdPASS)
    {
        log_error("Failed to reset BT UART buffer");
    }
}

int bsp_bluetooth_uart_tx(const uint8_t *p_data, size_t length)
{
    HAL_UART_Transmit(&UART1_Handle, (uint8_t *) p_data, length, HAL_MAX_DELAY);
    return 0;
}

int bsp_bluetooth_uart_rx(uint8_t *p_data, size_t length)
{
    if (xStreamBufferBytesAvailable(sbuffer_handle_rx) < length)
    {
        return -1;
    }

    if (missed_rx_data)
    {
        missed_rx_data = false;
        log_error("Lost RX data");
    }

    size_t bytes_received = xStreamBufferReceive(sbuffer_handle_rx, (void *) p_data, length, 0);
    if (bytes_received != length)
    {
        log_error("UART RX failed: expected %d, received %d", length, bytes_received);
        return -1;
    }

    return 0;
}

void bsp_bluetooth_uart_isr_rx_complete_callback(void)
{
    if (xStreamBufferIsFull(sbuffer_handle_rx) == pdFALSE)
    {
        if (xStreamBufferSendFromISR(sbuffer_handle_rx, (uint8_t *) irq_rx_data, (size_t) 1, (BaseType_t *) pdFALSE) !=
            1)
        {
            missed_rx_data = true;
        }
    }
    else
    {
        missed_rx_data = true;
    }

    // Start receiving
    HAL_UART_Receive_IT(&UART1_Handle, (uint8_t *) irq_rx_data, 1);
}
