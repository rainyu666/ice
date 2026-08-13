#include "board_link_moisture_detection.h"
#include "board_hw.h"

void board_link_moisture_detection_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    MOISTURE_DETECTION_GPIO_CLK_ENABLE();

    GPIO_InitStruct.Pin   = MOISTURE_DETECTION_GPIO_PIN;
    GPIO_InitStruct.Mode  = MOISTURE_DETECTION_GPIO_MODE;
    GPIO_InitStruct.Pull  = MOISTURE_DETECTION_GPIO_PULL;
    GPIO_InitStruct.Speed = MOISTURE_DETECTION_GPIO_SPEED;
    HAL_GPIO_Init(MOISTURE_DETECTION_GPIO_PORT, &GPIO_InitStruct);
}

bool board_link_moisture_detection_is_detected(void)
{
    return HAL_GPIO_ReadPin(MOISTURE_DETECTION_GPIO_PORT, MOISTURE_DETECTION_GPIO_PIN) == GPIO_PIN_SET;
}
