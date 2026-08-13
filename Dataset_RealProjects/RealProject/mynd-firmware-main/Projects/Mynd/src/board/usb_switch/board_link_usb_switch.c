#define LOG_LEVEL LOG_LEVEL_ERROR
#include "board_link_usb_switch.h"
#include "board_hw.h"
#include "logger.h"

void board_link_usb_switch_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    USB_SELECT0_GPIO_CLK_ENABLE();
    USB_SELECT1_GPIO_CLK_ENABLE();

    GPIO_InitStruct.Pin   = USB_SELECT0_GPIO_PIN;
    GPIO_InitStruct.Mode  = USB_SELECT0_GPIO_MODE;
    GPIO_InitStruct.Pull  = USB_SELECT0_GPIO_PULL;
    GPIO_InitStruct.Speed = USB_SELECT0_GPIO_SPEED;
    HAL_GPIO_Init(USB_SELECT0_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = USB_SELECT1_GPIO_PIN;
    GPIO_InitStruct.Mode  = USB_SELECT1_GPIO_MODE;
    GPIO_InitStruct.Pull  = USB_SELECT1_GPIO_PULL;
    GPIO_InitStruct.Speed = USB_SELECT1_GPIO_SPEED;
    HAL_GPIO_Init(USB_SELECT1_GPIO_PORT, &GPIO_InitStruct);

    log_debug("USB switch initialized");
}

void board_link_usb_switch_to_mcu(void)
{
    HAL_GPIO_WritePin(USB_SELECT0_GPIO_PORT, USB_SELECT0_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(USB_SELECT1_GPIO_PORT, USB_SELECT1_GPIO_PIN, GPIO_PIN_SET);
    log_info("USB switched to MCU");
}

void board_link_usb_switch_to_bluetooth(void)
{
    HAL_GPIO_WritePin(USB_SELECT0_GPIO_PORT, USB_SELECT0_GPIO_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(USB_SELECT1_GPIO_PORT, USB_SELECT1_GPIO_PIN, GPIO_PIN_SET);
    log_info("USB switched to Bluetooth module");
}

void board_link_usb_switch_to_usb_pd(void)
{
    HAL_GPIO_WritePin(USB_SELECT0_GPIO_PORT, USB_SELECT0_GPIO_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(USB_SELECT1_GPIO_PORT, USB_SELECT1_GPIO_PIN, GPIO_PIN_RESET);
    log_info("USB switched to Bluetooth module");
}

void board_link_usb_switch_to_uart_debug(void)
{
    HAL_GPIO_WritePin(USB_SELECT1_GPIO_PORT, USB_SELECT1_GPIO_PIN, GPIO_PIN_RESET);
    log_info("USB switched to UART debug");
}
