#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    CHARGER_STATUS_CONNECTED     = 0, /*!< Charger is connected but not active(not fully initialized or complete) */
    CHARGER_STATUS_NOT_CONNECTED = 1, /*!< Charger is not connected */
    CHARGER_STATUS_UNDEFINED     = 2, /*!< I2C IO error, invalid IC, etc. */
} board_link_charger_status_t;

#if defined(__cplusplus)
extern "C"
{
#endif

    /**
     * @brief Initializes the battery charger.
     */
    void board_link_charger_init(void);

    /**
     * @brief Enables or disables low power mode of the charger.
     *
     * @param[in] enable    true to enable lower power mode, false to disable
     */
    void board_link_charger_enable_low_power_mode(bool enable);

    /**
     * @brief Sets up the battery charger.
     *
     * @return 0 if successful, -1 otherwise
     */
    int board_link_charger_setup(void);

    /**
     * @brief Enables or disables fast charge mode (2.5 A or 5 A charge current).
     *
     * @param[in] enable    true to enable fast charge, false to disable
     */
    void board_link_charger_enable_fast_charge(bool enable);

    /**
     * @brief Disables the charger (sets the charge current to 0 mA).
     */
    void board_link_charger_disable(void);

    /**
     * @brief Checks if fast charge mode is enabled.
     *
     * @return true if fast charge is enabled, false otherwise
     */
    bool board_link_charger_is_fast_charge_enabled(void);

    /**
     * @brief Gets the battery voltage in mV.
     *
     * @param[out] p_voltage_mv     pointer to where the voltage should be written
     *
     * @return 0 if successful, -1 otherwise
     */
    int board_link_charger_get_battery_voltage(uint16_t *p_voltage_mv);

    /**
     * @brief Gets the charge state.
     * @param[out] p_state     pointer to where the charge status should be written
     * @return 0 if successful, -1 otherwise
     */
    int board_link_charger_get_status(board_link_charger_status_t *p_charger_status);

    /**
     * @brief Gets the AC plugged status.
     * @param[out] p_ac_plugged     pointer to where the AC plugged status should be written
     * @return 0 if successful, -1 otherwise
     */
    int board_link_charger_get_ac_plugged_status(bool *p_ac_plugged);

    /**
     * @brief Disables or enables battery charging
     * @param[in] disable, true to disable charging, false to enable
     * @return 0 if successful, -1 otherwise
     */
    int board_link_charger_disable_charging(bool disable);

#if defined(__cplusplus)
}
#endif
