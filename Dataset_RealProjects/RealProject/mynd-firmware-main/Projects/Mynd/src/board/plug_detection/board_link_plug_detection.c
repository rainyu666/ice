#include "board_link_plug_detection.h"
#include "board_hw.h"

void board_link_plug_detection_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // Configure power button pin
    PLUG_DETECTION_GPIO_CLK_ENABLE();
    GPIO_InitStruct.Pin   = PLUG_DETECTION_GPIO_PIN;
    GPIO_InitStruct.Mode  = PLUG_DETECTION_GPIO_MODE;
    GPIO_InitStruct.Pull  = PLUG_DETECTION_GPIO_PULL;
    GPIO_InitStruct.Speed = PLUG_DETECTION_GPIO_SPEED;
    HAL_GPIO_Init(PLUG_DETECTION_GPIO_PORT, &GPIO_InitStruct);
}

bool board_link_plug_detection_is_jack_connected(void)
{
    return HAL_GPIO_ReadPin(PLUG_DETECTION_GPIO_PORT, PLUG_DETECTION_GPIO_PIN) == GPIO_PIN_RESET;
}
