#include "board_link_power_supply.h"
#include "board_hw.h"
#include "logger.h"

void board_link_power_supply_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // Configure power button pin
    POWER_BUTTON_GPIO_CLK_ENABLE();
    GPIO_InitStruct.Pin   = POWER_BUTTON_GPIO_PIN;
    GPIO_InitStruct.Mode  = POWER_BUTTON_GPIO_MODE;
    GPIO_InitStruct.Pull  = POWER_BUTTON_GPIO_PULL;
    GPIO_InitStruct.Speed = POWER_BUTTON_GPIO_SPEED;
    HAL_GPIO_Init(POWER_BUTTON_GPIO_PORT, &GPIO_InitStruct);

    // Configure power hold pin
    POWER_HOLD_GPIO_CLK_ENABLE();
    GPIO_InitStruct.Pin   = POWER_HOLD_GPIO_PIN;
    GPIO_InitStruct.Mode  = POWER_HOLD_GPIO_MODE;
    GPIO_InitStruct.Pull  = POWER_HOLD_GPIO_PULL;
    GPIO_InitStruct.Speed = POWER_HOLD_GPIO_SPEED;
    HAL_GPIO_Init(POWER_HOLD_GPIO_PORT, &GPIO_InitStruct);

    // Configure AC OK pin
    AC_OK_GPIO_CLK_ENABLE();
    GPIO_InitStruct.Pin   = AC_OK_GPIO_PIN;
    GPIO_InitStruct.Mode  = AC_OK_GPIO_MODE;
    GPIO_InitStruct.Pull  = AC_OK_GPIO_PULL;
    GPIO_InitStruct.Speed = AC_OK_GPIO_SPEED;
    HAL_GPIO_Init(AC_OK_GPIO_PORT, &GPIO_InitStruct);
}

bool board_link_power_supply_is_ac_ok(void)
{
    return HAL_GPIO_ReadPin(AC_OK_GPIO_PORT, AC_OK_GPIO_PIN) == GPIO_PIN_SET;
}

bool board_link_power_supply_button_is_pressed(void)
{
    return HAL_GPIO_ReadPin(POWER_BUTTON_GPIO_PORT, POWER_BUTTON_GPIO_PIN) == GPIO_PIN_SET;
}

bool board_link_power_supply_is_held_on(void)
{
    return HAL_GPIO_ReadPin(POWER_HOLD_GPIO_PORT, POWER_HOLD_GPIO_PIN) == GPIO_PIN_SET;
}

void board_link_power_supply_hold_on(bool hold)
{
    HAL_GPIO_WritePin(POWER_HOLD_GPIO_PORT, POWER_HOLD_GPIO_PIN, (hold) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    log_info("Power supply hold %s", hold ? "active" : "released");
}
