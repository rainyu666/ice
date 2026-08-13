#pragma once

#include <stdbool.h>
#include <stdint.h>

#define E_TPS25751_OK      0
#define E_TPS25751_IO      1 // I/O operation failed
#define E_TPS25751_PARAM   2 // Invalid param
#define E_TPS25751_STATE   3 // Invalid state
#define E_TPS25751_TIMEOUT 4 // Timeout

typedef int (*tps25751_i2c_read_fn)(uint8_t i2c_address, uint8_t register_address, uint8_t *p_data, uint32_t length);
typedef int (*tps25751_i2c_write_fn)(uint8_t i2c_address, uint8_t register_address, const uint8_t *p_data,
                                     uint32_t length);
typedef void (*tps25751_thread_sleep_fn)(uint32_t ms);

typedef struct tps25751_handler tps25751_handler_t;

typedef struct
{
    tps25751_i2c_read_fn     i2c_read_fn;
    tps25751_i2c_write_fn    i2c_write_fn;
    tps25751_thread_sleep_fn thread_sleep_fn;
    uint8_t                  i2c_device_address;
} tps25751_config_t;

typedef enum
{
    TPS25751_DEVICE_MODE_APP,
    TPS25751_DEVICE_MODE_BOOT,
    TPS25751_DEVICE_MODE_PATCH,
    TPS25751_DEVICE_MODE_LIMITED,
} tps25751_device_mode_t;

typedef enum
{
    TPS25751_CONNECTION_STATE_NO_CONNECTION,
    TPS25751_CONNECTION_STATE_PORT_DISABLED,
    TPS25751_CONNECTION_STATE_AUDIO_CONNECTION,
    TPS25751_CONNECTION_STATE_DEBUG_CONNECTION,
    TPS25751_CONNECTION_STATE_NO_RD_CONNECTION,
    TPS25751_CONNECTION_STATE_RESERVED,
    TPS25751_CONNECTION_STATE_CONNECTION_PRESENT_NO_RA,
    TPS25751_CONNECTION_STATE_CONNECTION_PRESENT,
} tps25751_connection_state_t;

typedef enum
{
    TPS25751_PLUG_ORIENTATION_UPSIDE_UP,
    TPS25751_PLUG_ORIENTATION_UPSIDE_DOWN,
} tps25751_plug_orientation_t;

typedef enum
{
    TPS25751_PORT_ROLE_SINK,
    TPS25751_PORT_ROLE_SOURCE,
} tps25751_port_role_t;

typedef enum
{
    TPS25751_DATA_ROLE_UPWARD_FACING_PORT,
    TPS25751_DATA_ROLE_DOWNWARD_FACING_PORT,
} tps25751_data_role_t;

typedef enum
{
    TPS25751_VBUS_STATUS_AT_SAFE_0V,
    TPS25751_VBUS_STATUS_AT_SAFE_5V,
    TPS25751_VBUS_STATUS_WITHIN_EXPECTED_LIMITS,
    TPS25751_VBUS_STATUS_OUTSIDE_SPECIFIED_RANGES,
} tps25751_vbus_status_t;

typedef enum
{
    TPS25751_USB_HOST_ATTACHMENT_STATUS_NO_HOST,
    TPS25751_USB_HOST_ATTACHMENT_STATUS_PD_YES_USB_NO,
    TPS25751_USB_HOST_ATTACHMENT_STATUS_PD_NO_USB_YES,
    TPS25751_USB_HOST_ATTACHMENT_STATUS_PD_YES_USB_YES,
} tps25751_usb_host_attachment_status_t;

typedef enum
{
    TPS25751_LEGACY_STATUS_NOT_IN_LEGACY_MODE,
    TPS25751_LEGACY_STATUS_LEGACY_SINK,
    TPS25751_LEGACY_STATUS_LEGACY_SOURCE,
    TPS25751_LEGACY_STATUS_LEGACY_SINK_DUE_TO_DEAD_BATTERY,
} tps25751_legacy_status_t;

typedef enum
{
    TPS25751_SINK_SM_ONLY,
    TPS25751_SOURCE_SM_ONLY,
    TPS25751_DRP_SM,
    TPS25751_DISABLED,
} tps25751_port_typec_state_machine_t;

typedef enum
{
    TPS25751_PD_MAX_SOURCE_CURRENT_0A,
    TPS25751_PD_MAX_SOURCE_CURRENT_1A,
    TPS25751_PD_MAX_SOURCE_CURRENT_2A,
    TPS25751_PD_MAX_SOURCE_CURRENT_3A,
} tps25751_pd_max_source_current_t;

typedef enum
{
    TPS25751_SINK_MAX_VOLTAGE_5V,
    TPS25751_SINK_MAX_VOLTAGE_20V,
} tps25751_sink_max_voltage_t;

typedef struct
{
    bool                                  is_plug_connected;
    tps25751_connection_state_t           connection_state;
    tps25751_plug_orientation_t           plug_orientation;
    tps25751_port_role_t                  port_role;
    tps25751_data_role_t                  data_role;
    tps25751_vbus_status_t                vbus_status;
    tps25751_usb_host_attachment_status_t usb_host_attachment_status;
    tps25751_legacy_status_t              legacy_status;
    bool                                  is_bist_procedure_in_progress;
    bool                                  soc_ack_timeout;
} tps25751_status_t;

typedef enum
{
    TPS25751_PATCH_CONFIG_SOURCE_NONE,
    TPS25751_PATCH_CONFIG_SOURCE_RESERVED1,
    TPS25751_PATCH_CONFIG_SOURCE_RESERVED2,
    TPS25751_PATCH_CONFIG_SOURCE_RESERVED3,
    TPS25751_PATCH_CONFIG_SOURCE_RESERVED4,
    TPS25751_PATCH_CONFIG_SOURCE_EEPROM,
    TPS25751_PATCH_CONFIG_SOURCE_I2C,
    TPS25751_PATCH_CONFIG_SOURCE_RESERVED7,
} tps25751_patch_config_source_t;

typedef struct
{
    bool                           patch_header_error_detected;
    bool                           dead_battery_indicated;
    bool                           i2c_eeprom_present;
    bool                           patch_download_error_detected;
    bool                           system_thermal_shutdown_indicated;
    tps25751_patch_config_source_t patch_config_source;
    uint8_t                        pd_controller_revision_id;
} tps25751_boot_flags_t;

typedef enum
{
    TPS25751_CHARGER_ADVERTISE_STATUS_DISABLED,
    TPS25751_CHARGER_ADVERTISE_STATUS_IN_PROCESS,
    TPS25751_CHARGER_ADVERTISE_STATUS_COMPLETE,
    TPS25751_CHARGER_ADVERTISE_STATUS_RESERVED,
} tps25751_charger_advertise_status_t;

typedef enum
{
    TPS25751_CHARGER_DETECT_STATUS_DISABLED,
    TPS25751_CHARGER_DETECT_STATUS_IN_PROGRESS,
    TPS25751_CHARGER_DETECT_STATUS_NONE_DETECTED,
    TPS25751_CHARGER_DETECT_STATUS_SDP_DETECTED,
    TPS25751_CHARGER_DETECT_STATUS_BC_1_2_CDP_DETECTED,
    TPS25751_CHARGER_DETECT_STATUS_BC_1_2_DCP_DETECTED,
    TPS25751_CHARGER_DETECT_STATUS_DIVIDER1_DCP_DETECTED,
    TPS25751_CHARGER_DETECT_STATUS_DIVIDER2_DCP_DETECTED,
    TPS25751_CHARGER_DETECT_STATUS_DIVIDER3_DCP_DETECTED,
    TPS25751_CHARGER_DETECT_STATUS_1_2V_DCP_DETECTED,
    TPS25751_CHARGER_DETECT_STATUS_RESERVED_A,
    TPS25751_CHARGER_DETECT_STATUS_RESERVED_B,
    TPS25751_CHARGER_DETECT_STATUS_RESERVED_C,
    TPS25751_CHARGER_DETECT_STATUS_RESERVED_D,
    TPS25751_CHARGER_DETECT_STATUS_RESERVED_E,
    TPS25751_CHARGER_DETECT_STATUS_RESERVED_F,
} tps25751_charger_detect_status_t;

typedef enum
{
    TPS25751_TYPE_C_CURRENT_DEFAULT,
    TPS25751_TYPE_C_CURRENT_1_5_AMP,
    TPS25751_TYPE_C_CURRENT_3_0_AMP,
    TPS25751_TYPE_C_CURRENT_SET_BY_PD_CONTRACT,
} tps25751_type_c_current_t;

typedef enum
{
    TPS25751_PD_ROLE_SINK,
    TPS25751_PD_ROLE_SOURCE,
} tps25751_pd_role_t;

typedef struct
{
    bool                                connection_present;
    tps25751_pd_role_t                  pd_role;
    tps25751_type_c_current_t           type_c_current;
    tps25751_charger_detect_status_t    charger_detect_status;
    tps25751_charger_advertise_status_t charger_advertise_status;
} tps25751_power_status_t;

/**
 * @brief Initializes the TPS25751 driver.
 *
 * @param[in] p_config      pointer to configuration struct
 *
 * @return pointer to handler if successful, NULL otherwise
 */
tps25751_handler_t *tps25751_init(const tps25751_config_t *p_config);

/**
 * @brief Gets the current device mode.
 *
 * @param[in]  h            pointer to handler
 * @param[out] p_mode       pointer to variable where the mode will be written to
 *
 * @return 0 if successful, -1 otherwise
 */
int tps25751_get_device_mode(const tps25751_handler_t *h, tps25751_device_mode_t *p_mode);

/**
 * @brief Gets the device status.
 *
 * @param[in]  h            pointer to handler
 * @param[out] p_status     pointer to variable where the status will be written to
 *
 * @return 0 if successful, -1 otherwise
 */
int tps25751_get_device_status(const tps25751_handler_t *h, tps25751_status_t *p_status);

/**
 * @brief Gets the boot status.
 *
 * @param[in]  h            pointer to handler
 * @param[out] p_status     pointer to variable where the status will be written to
 *
 * @return 0 if successful, -1 otherwise
 */
int tps25751_get_boot_flags(const tps25751_handler_t *h, tps25751_boot_flags_t *p_flags);

/**
 * @brief Gets the power status.
 *
 * @param[in]  h            pointer to handler
 * @param[out] p_status     pointer to variable where the status will be written to
 *
 * @return 0 if successful, -1 otherwise
 */
int tps25751_get_power_status(const tps25751_handler_t *h, tps25751_power_status_t *p_status);

/**
 * @brief Loads a patch bundle to the PD controller.
 *
 * @details This is a long running blocking call. It will put the current thread to sleep
 *          while waiting for the PD controller to process the patch update.
 *
 * @param[in] h                 pointer to handler
 * @param[in] p_patch_data      pointer to patch data
 * @param[in] patch_length      patch length in bytes
 *
 * @return 0 if successful, -1 otherwise
 */
int tps25751_load_patch_bundle(const tps25751_handler_t *h, const uint8_t *p_patch_data, uint32_t patch_length);

/**
 * @brief Clears the dead battery flag.
 *
 * @param[in] h             pointer to handler
 *
 * @return 0 if successful, -1 otherwise
 */
int tps25751_clear_dead_battery_flag(const tps25751_handler_t *h);

/**
 * @brief Sends a 4CC command and verifies that it got processed successfully.
 *
 * @param[in] h                         pointer to handler
 * @param[in] command                   4-character command string
 * @param[in] timeout_ms                timeout in milliseconds
 *
 * @return 0 if successful, -1 otherwise
 */
int tps25751_send_4cc_command(const tps25751_handler_t *h, const char *command, uint32_t timeout_ms);

/**
 * @brief Reads the 64-bit contents of the 'Customer Use' register
 *
 * @param[in] h                         pointer to handler
 * @param[in] p_register_data           pointer to data
 *
 * @return 0 if successful, -1 otherwise
 */
int tps25751_read_customer_use_reg(const tps25751_handler_t *h, uint8_t *p_register_data);

/**
 * @brief Set TypeC state machine configuration
 *
 * @param[in] h                         pointer to handler
 * @param[in] sm                        state machine (Sink only, Source only, DRP, Disabled)
 *
 * @return 0 if successful, -1 otherwise
 */
int tps25751_set_port_configuration_typec_state_machine(const tps25751_handler_t           *h,
                                                        tps25751_port_typec_state_machine_t sm);

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
int tps25751_i2c_read(const tps25751_handler_t *h, uint8_t i2c_address, uint8_t register_address, uint8_t *p_data,
                      uint32_t length);

/**
 * @brief Set MAX source current (Simplified version with only 0, 1, 2, 3A options)
 *
 * @param[in] h                         pointer to handler
 * @param[in] source_current            max source current
 *
 * @return 0 if successful, -1 otherwise
 */
int tps25751_set_max_source_current(const tps25751_handler_t *h, tps25751_pd_max_source_current_t source_current);

/**
 * @brief Set MAX charger voltage
 * @param[in] h                         pointer to handler
 * @param[in] voltage                   max charger voltage
 * @return 0 if successful, -1 otherwise
 */
int tps25751_set_max_charger_voltage(const tps25751_handler_t *h, tps25751_sink_max_voltage_t voltage);

/**
 * @brief Instructs the USB PD controller to execute a read from the EEPROM which is connected to it.
 *        The EEPROM is connected to the USB PD controller via I2C.
 *        There is no need to specify the I2C address of the EEPROM, as it is known to the USB PD controller.
 *
 * @param[in]  h                        pointer to handler
 * @param[in]  address                  address to read from
 * @param[out] p_data                   pointer to buffer where the read data will be written to
 * @param[in]  length                   length to read (max 16 bytes)
 *
 * @return 0 if successful, -1 otherwise
 */
int tps25751_eeprom_read(const tps25751_handler_t *h, uint16_t address, uint8_t *p_data, uint32_t length);

/**
 * @brief Instructs the USB PD controller to execute a write to the EEPROM which is connected to it.
 *        The EEPROM is connected to the USB PD controller via I2C.
 *        There is no need to specify the I2C address of the EEPROM, as it is known to the USB PD controller.
 * @param[in]  h                        pointer to handler
 * @param[in]  address                  address to write to
 * @param[in]  p_data                   pointer to data to write
 * @param[in]  length                   length to write (max 32 bytes)
 *
 * @return 0 if successful, -1 otherwise
 */
int tps25751_eeprom_write(const tps25751_handler_t *h, uint16_t address, const uint8_t *p_data, uint32_t length);

/**
 * @brief Instructs the USB PD controller to 'try' to swap to source PD role.
 * @param[in] h                         pointer to handler
 *
 * @return 0 if successful, -1 otherwise
 */
int tps25751_swap_pd_role_to_source(const tps25751_handler_t *h);

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
int tps25751_i2c_write(const tps25751_handler_t *h, uint8_t i2c_address, uint8_t register_address,
                       const uint8_t *p_data, uint32_t length);
