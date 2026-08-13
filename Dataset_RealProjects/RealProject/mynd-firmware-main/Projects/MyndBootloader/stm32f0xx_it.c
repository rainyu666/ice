#include "stm32f0xx_hal.h"
#include "stm32f0xx.h"
#include "stm32f0xx_it.h"

extern PCD_HandleTypeDef hpcd;
extern I2C_HandleTypeDef I2C2_Handle;

/**
 * @brief  This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void) {}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void)
{
    __BKPT(0);
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles SVCall exception.
 * @param  None
 * @retval None
 */
void SVC_Handler(void) {}

/**
 * @brief  This function handles PendSVC exception.
 * @param  None
 * @retval None
 */
void PendSV_Handler(void) {}

/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}

/**
 * @brief  This function handles USB Handler.
 * @param  None
 * @retval None
 */
void USB_IRQHandler(void)
{
    HAL_PCD_IRQHandler(&hpcd);
}

void DMA1_Channel4_5_6_7_IRQHandler(void)
{
    // HAL_DMA_IRQHandler(I2C2_Handle.hdmarx);
    HAL_DMA_IRQHandler(I2C2_Handle.hdmatx);
}

/**
 * @brief  This function handles I2C event and error interrupt request.
 * @param  None
 * @retval None
 * @Note   This function is redefined in "main.h" and related to I2C data transmission
 */
void I2C2_IRQHandler(void)
{
    HAL_I2C_EV_IRQHandler(&I2C2_Handle);
    HAL_I2C_ER_IRQHandler(&I2C2_Handle);
}
