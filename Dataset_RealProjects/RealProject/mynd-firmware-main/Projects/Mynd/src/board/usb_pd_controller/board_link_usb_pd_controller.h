#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef void (*board_link_usb_pd_controller_plug_connection_change_cb_t)(bool connected);
typedef void (*board_link_usb_pd_controller_power_connection_change_cb_t)(bool connected);
typedef void (*board_link_usb_pd_controller_pd_port_role_change_cb_t)(bool source);

typedef struct
{
    board_link_usb_pd_controller_plug_connection_change_cb_t  plug_connection_change_cb;
    board_link_usb_pd_controller_power_connection_change_cb_t power_connection_change_cb;
    board_link_usb_pd_controller_pd_port_role_change_cb_t     pd_port_role_change_cb;
} board_link_usb_pd_controller_callbacks_t;

typedef enum
{
    TYPEC_DRP_MODE,
    TYPEC_SOURCE_ONLY_MODE,
    TYPEC_SINK_ONLY_MODE,
    TYPEC_DISABLED_MODE
} typec_mode_t;

typedef enum
{
    USB_PD_MAX_SOURCE_CURRENT_0A,
    USB_PD_MAX_SOURCE_CURRENT_1A,
    USB_PD_MAX_SOURCE_CURRENT_2A,
    USB_PD_MAX_SOURCE_CURRENT_3A,
} usb_pd_max_source_current_t;

typedef enum
{
    USB_PD_MAX_SOURCE_VOLTAGE_5V,
    USB_PD_MAX_SOURCE_VOLTAGE_20V,
} usb_pd_max_source_voltage_t;

#if defined(__cplusplus)
extern "C"
{
#endif

    /**
     * @brief Initializes the USB PD controller.
     *
     * @param[in] p_callbacks           pointer to callbacks struct
     */
    void board_link_usb_pd_controller_init(void);

    /**
     * @brief Polls the status of the USB PD controller.
     *
     * @details This function is meant to be called periodically.
     *          The user-provided callback functions will be called when
     *          the USB status changes.
     *
     * @param[in] p_callbacks           pointer to callbacks struct
     */
    void board_link_usb_pd_controller_poll_status(const board_link_usb_pd_controller_callbacks_t *p_callbacks);

    /**
     * @brief Instructs the USB PD controller to execute a read from a specified slave address and
     *        register offset using an I2C read transaction on the I2Cm bus.
     *
     * @note  The maximum read length supported is 10 bytes.
     *
     * @param[in]  h                        pointer to handler
     * @param[in]  i2c_address              I2C device address
     * @param[in]  register_address         register address
     * @param[out] p_data                   pointer to buffer where the read data will be written to
     * @param[in]  length                   length to read (see note above)
     *
     * @return 0 if successful, -1 otherwise
     */
    int board_link_usb_pd_controller_i2c_read(uint8_t i2c_address, uint8_t register_address, uint8_t *p_data,
                                              uint32_t length);

    int board_link_usb_pd_controller_eeprom_read(uint16_t register_address, uint8_t *p_data, uint32_t length);

    int board_link_usb_pd_controller_eeprom_write(uint16_t register_address, const uint8_t *p_data, uint32_t length);

    /**
     * @brief Instructs the USB PD controller to execute a write to a specified slave address and
     *        register offset using an I2C write transaction on the I2Cm bus.
     *
     * @note  The maximum payload length supported is 10 bytes.
     *
     * @param[in] h                         pointer to handler
     * @param[in] i2c_address               I2C device address
     * @param[in] register_address          register address
     * @param[in] p_data                    pointer to data to write
     * @param[in] length                    length to write (see note above)
     *
     * @return 0 if successful, -1 otherwise
     */
    int board_link_usb_pd_controller_i2c_write(uint8_t i2c_address, uint8_t register_address, const uint8_t *p_data,
                                               uint32_t length);

    /**
     * @brief Checks if the USB PD controller is ready to operate normally.
     *
     * @return true if ready, false otherwise
     */
    bool board_link_usb_pd_controller_is_ready(void);

    /**
     * @brief Checks if there is USB cable plugged.
     *
     * @return true if cable is plugged, false otherwise
     */
    bool board_link_usb_pd_controller_is_plug_connected(void);

    /**
     * @brief Checks if the USB power is connected.
     *
     * @return true if power is connected, false otherwise
     */
    bool board_link_usb_pd_controller_is_power_connected(void);

    /**
     * @brief Checks if the USB PD controller detected a dead battery condition.
     *
     * @return true if the battery is dead, false otherwise
     */
    bool board_link_usb_pd_controller_is_battery_dead(void);

    /**
     * @brief Instructs the USB PD controller to swap to source mode.
     */
    int board_link_usb_pd_controller_swap_to_srouce(void);

    /**
     * @brief Gets the USB PD controller firmware version by reading byte 0 of the
     *        Customer Use register. The version # is written in hex, e.g. 0x21 = v2.1
     *
     * @param[in] p_pd_fw_version pointer for version to be written to.
     *
     * @return Returns 0 if successful, -1 otherwise.
     */
    int board_link_usb_pd_controller_fw_version(uint8_t *p_pd_fw_version);

    void board_link_usb_pd_controller_select_mode(typec_mode_t mode);

    /**
     * @brief Sets the maximum source current for the USB PD controller.
     *
     * @param[in] source_current  maximum source current
     *
     * @return Returns 0 if successful, -1 otherwise.
     */
    void board_link_usb_pd_controller_set_max_source_current(usb_pd_max_source_current_t source_current);

    void board_link_usb_pd_controller_set_max_source_voltage(usb_pd_max_source_voltage_t source_voltage);

#if defined(__cplusplus)
}
#endif
