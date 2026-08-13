#include "board.h"
#include "stm32f0xx_hal.h"

extern I2C_HandleTypeDef I2C2_Handle;

void board_init(void)
{
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
    return HAL_GetTick();
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
