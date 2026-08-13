#include "stm32f0xx_hal.h"
#include "stm32f0xx.h"

extern ADC_HandleTypeDef  Adc1Handle;
extern I2C_HandleTypeDef  I2C1_Handle;
extern I2C_HandleTypeDef  I2C2_Handle;
extern UART_HandleTypeDef UART1_Handle;
extern UART_HandleTypeDef UART2_Handle;
extern volatile uint32_t g_btlink_seq_tx;
extern volatile unsigned long long g_btlink_octets_u64;

void HardFault_Handler(void)
{
    uint32_t msp  = __get_MSP();
    uint32_t psp  = __get_PSP();
    uint32_t ipsr = __get_IPSR();

    __asm("bkpt 1");

    // Reset the system
    NVIC_SystemReset();
}

void EXTI2_3_IRQHandler(void)
{
    // IO expander interrupt pin
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

void I2C1_IRQHandler(void)
{
    HAL_I2C_EV_IRQHandler(&I2C1_Handle);
    HAL_I2C_ER_IRQHandler(&I2C1_Handle);
}

void I2C2_IRQHandler(void)
{
    HAL_I2C_EV_IRQHandler(&I2C2_Handle);
    HAL_I2C_ER_IRQHandler(&I2C2_Handle);
}

void ADC1_IRQHandler(void)
{
    HAL_ADC_IRQHandler(&Adc1Handle);
}

void DMA1_Channel1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(Adc1Handle.DMA_Handle);
}

void USART1_IRQHandler(void)
{
    g_btlink_seq_tx     = 1;
    g_btlink_octets_u64 = g_btlink_octets_u64 + 1ULL;
    HAL_UART_IRQHandler(&UART1_Handle);
}

void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&UART2_Handle);
}

void bsp_bluetooth_uart_isr_rx_complete_callback(void);
void bsp_debug_uart_isr_rx_complete_callback(void);

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        bsp_bluetooth_uart_isr_rx_complete_callback();
    }
    else if (huart->Instance == USART2)
    {
        bsp_debug_uart_isr_rx_complete_callback();
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        HAL_UART_MspDeInit(&UART1_Handle);

        __HAL_UART_CLEAR_PEFLAG(&UART1_Handle);
        __HAL_UART_CLEAR_OREFLAG(&UART1_Handle);
        __HAL_UART_CLEAR_FEFLAG(&UART1_Handle);
        __HAL_UART_CLEAR_NEFLAG(&UART1_Handle);
        __HAL_UART_CLEAR_IDLEFLAG(&UART1_Handle);

        HAL_UART_MspInit(&UART1_Handle);
    }
    else if (huart->Instance == USART2)
    {
        HAL_UART_MspDeInit(&UART2_Handle);

        __HAL_UART_CLEAR_PEFLAG(&UART2_Handle);
        __HAL_UART_CLEAR_OREFLAG(&UART2_Handle);
        __HAL_UART_CLEAR_FEFLAG(&UART2_Handle);
        __HAL_UART_CLEAR_NEFLAG(&UART2_Handle);
        __HAL_UART_CLEAR_IDLEFLAG(&UART2_Handle);

        HAL_UART_MspInit(&UART2_Handle);

        static uint8_t rx_data = 0;
        HAL_UART_Receive_IT(&UART2_Handle, (uint8_t *) &rx_data, 1);
    }
}
