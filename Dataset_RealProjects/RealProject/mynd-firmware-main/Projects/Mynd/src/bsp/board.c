#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f0xx_hal.h"

extern I2C_HandleTypeDef I2C2_Handle;

void board_init(void)
{
    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI2_3_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

    /* Enable all GPIO clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
}

void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}

uint32_t get_systick(void)
{
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
        return 0;

    uint32_t IPSR_register;
    __asm volatile("MRS %0, ipsr" : "=r"(IPSR_register));

    if (0U == IPSR_register)
        return xTaskGetTickCountFromISR();

    return xTaskGetTickCount();
}

uint32_t board_get_ms_since(uint32_t tick_ms)
{
    uint32_t current_tick_ms = get_systick();

    // Handle overflow
    if (current_tick_ms < tick_ms)
    {
        // Note: if last tick is UINT32_MAX and current tick is 0, this function should return 1
        return (UINT32_MAX - tick_ms) + current_tick_ms + 1;
    }
    else
    {
        return current_tick_ms - tick_ms;
    }
}
