#pragma once

#include <stdbool.h>

#if defined(__cplusplus)
extern "C"
{
#endif

    /**
     * @brief Initializes GPIOs necessary to control the Bluetooth module.
     */
    void board_link_bluetooth_init(void);

    /**
     * @brief Powers the Bluetooth module on/off.
     *
     * @param[in] on        power option
     */
    void board_link_bluetooth_set_power(bool on);

    /**
     * @brief Asserts/deasserts the reset line of the Bluetooth module.
     *
     * @param[in] assert    assert option
     */
    void board_link_bluetooth_reset(bool assert);

#if defined(__cplusplus)
}
#endif
