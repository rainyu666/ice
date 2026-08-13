#pragma once

#if defined(__cplusplus)
extern "C"
{
#endif

    /**
     * @brief Initializes the hardware needed to interface the USB switch.
     * @note  The USB is switched to the MCU after initialization.
     */
    void board_link_usb_switch_init(void);

    /**
     * @brief Switches USB to the MCU.
     */
    void board_link_usb_switch_to_mcu(void);

    /**
     * @brief Switches USB to the Bluetooth module.
     */
    void board_link_usb_switch_to_bluetooth(void);

    /**
     * @brief Switches USB to the USB PD controller IC.
     */
    void board_link_usb_switch_to_usb_pd(void);

#if defined(__cplusplus)
}
#endif
