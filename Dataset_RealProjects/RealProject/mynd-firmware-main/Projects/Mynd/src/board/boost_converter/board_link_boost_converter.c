#include "board_link_boost_converter.h"
#include "board_hw.h"
#define LOG_LEVEL LOG_LEVEL_ERROR
#include "logger.h"

void board_link_boost_converter_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // Configure boost enable pin
    BOOST_ENABLE_GPIO_CLK_ENABLE();
    board_link_boost_converter_enable(false);
    GPIO_InitStruct.Pin   = BOOST_ENABLE_GPIO_PIN;
    GPIO_InitStruct.Mode  = BOOST_ENABLE_GPIO_MODE;
    GPIO_InitStruct.Pull  = BOOST_ENABLE_GPIO_PULL;
    GPIO_InitStruct.Speed = BOOST_ENABLE_GPIO_SPEED;
    HAL_GPIO_Init(BOOST_ENABLE_GPIO_PORT, &GPIO_InitStruct);
}

void board_link_boost_converter_enable(bool enable)
{
    HAL_GPIO_WritePin(BOOST_ENABLE_GPIO_PORT, BOOST_ENABLE_GPIO_PIN, (enable) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    log_info("Boost converter %s", enable ? "enabled" : "disabled");
}
