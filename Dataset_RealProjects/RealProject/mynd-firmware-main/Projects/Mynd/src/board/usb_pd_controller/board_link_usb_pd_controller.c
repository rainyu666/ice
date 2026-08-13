#include "board_link_usb_pd_controller.h"
#include "board_hw.h"
#include "bsp_usb_pd_i2c.h"
#include "tps25751.h"
#define LOG_LEVEL LOG_LEVEL_ERROR
#include "logger.h"
#include "FreeRTOS.h"
#include "task.h"
#include "external/teufel/libs/app_assert/app_assert.h"

static struct
{
    tps25751_handler_t *tps25751;
    bool                is_ready;
    bool                is_plug_connection_present;
    bool                is_power_connection_present;
    bool                is_dead_battery_indicated;
    bool                is_pd_port_role_source; // false = sink, true = source
} s_usb_pd;

static void thread_sleep_ms(uint32_t ms);

static const tps25751_config_t tps25751_config = {
    .i2c_device_address = TPS25751_I2C_ADDRESS,
    .i2c_read_fn        = bsp_usb_pd_i2c_read,
    .i2c_write_fn       = bsp_usb_pd_i2c_write,
    .thread_sleep_fn    = thread_sleep_ms,
};

void board_link_usb_pd_controller_init(void)
{
    s_usb_pd.tps25751 = tps25751_init(&tps25751_config);
    if (s_usb_pd.tps25751 == NULL)
    {
        log_error("Failed to init TPS25751");
    }

    tps25751_clear_dead_battery_flag(s_usb_pd.tps25751);
}

void board_link_usb_pd_controller_poll_status(const board_link_usb_pd_controller_callbacks_t *p_callbacks)
{
    APP_ASSERT(p_callbacks != NULL, "Callbacks are NULL");

    // Device and power status are only available if the device is ready
    if (s_usb_pd.is_ready)
    {
        tps25751_status_t device_status;
        if (tps25751_get_device_status(s_usb_pd.tps25751, &device_status) == 0)
        {
            if (s_usb_pd.is_plug_connection_present != device_status.is_plug_connected)
            {
                s_usb_pd.is_plug_connection_present = device_status.is_plug_connected;
                if (p_callbacks->plug_connection_change_cb)
                {
                    p_callbacks->plug_connection_change_cb(s_usb_pd.is_plug_connection_present);
                }
            }
        }
        else
        {
            log_error("Failed to get status");
        }

        tps25751_power_status_t power_status;
        if (tps25751_get_power_status(s_usb_pd.tps25751, &power_status) == 0)
        {
            if (s_usb_pd.is_power_connection_present != power_status.connection_present)
            {
                s_usb_pd.is_power_connection_present = power_status.connection_present;
                if (p_callbacks->power_connection_change_cb)
                {
                    p_callbacks->power_connection_change_cb(s_usb_pd.is_power_connection_present);
                }
            }
        }
        else
        {
            log_error("Failed to get power status");
        }

#if 0
        // TODO: add PD role change callback (maybe will be needed for power bank mode)
        // Get power delivery status
        tps25751_pd_status_t power_delivery_status;
        if (tps25751_get_pd_status(s_usb_pd.tps25751, &power_delivery_status) == 0)
        {
            if (s_usb_pd.is_pd_port_role_source != (power_delivery_status.pd_role == TPS25751_PD_ROLE_SOURCE))
            {
                s_usb_pd.is_pd_port_role_source = (power_delivery_status.pd_role == TPS25751_PD_ROLE_SOURCE);
                if (p_callbacks->pd_port_role_change_cb)
                {
                    p_callbacks->pd_port_role_change_cb(s_usb_pd.is_pd_port_role_source);
                }
            }
        }
        else
        {
            log_error("Failed to get pd status");
        }
#endif
    }
    else
    {
        tps25751_device_mode_t device_mode;
        if (tps25751_get_device_mode(s_usb_pd.tps25751, &device_mode) == 0)
        {
            s_usb_pd.is_ready = (device_mode == TPS25751_DEVICE_MODE_APP);

            if (s_usb_pd.is_ready == false)
            {
                tps25751_boot_flags_t boot_flags;
                if (tps25751_get_boot_flags(s_usb_pd.tps25751, &boot_flags) == 0)
                {
                    s_usb_pd.is_dead_battery_indicated = boot_flags.dead_battery_indicated;
                }
                else
                {
                    log_error("Failed to get boot flags");
                }
            }
        }
        else
        {
            log_error("Failed to get device mode");
        }
    }
}

int board_link_usb_pd_controller_i2c_read(uint8_t i2c_address, uint8_t register_address, uint8_t *p_data,
                                          uint32_t length)
{
    if (tps25751_i2c_read(s_usb_pd.tps25751, i2c_address, register_address, p_data, length) != 0)
    {
        log_warn("RECOVER");
        // tps25751_warm_reset(s_usb_pd.tps25751);

        log_error("I2C read failed (address 0x%02X, register 0x%02X)", i2c_address, register_address);
        return -1;
    }
    return 0;
}

int board_link_usb_pd_controller_i2c_write(uint8_t i2c_address, uint8_t register_address, const uint8_t *p_data,
                                           uint32_t length)
{
    if (tps25751_i2c_write(s_usb_pd.tps25751, i2c_address, register_address, p_data, length) != 0)
    {
        log_error("I2C write failed (address 0x%02X, register 0x%02X)", i2c_address, register_address);

        log_warn("RECOVER");
        // tps25751_warm_reset(s_usb_pd.tps25751);

        return -1;
    }
    return 0;
}

int board_link_usb_pd_controller_eeprom_read(uint16_t register_address, uint8_t *p_data, uint32_t length)
{
    if (tps25751_eeprom_read(s_usb_pd.tps25751, register_address, p_data, length) != 0)
    {
        log_error("EEPROM read failed (register 0x%04X)", register_address);
        return -1;
    }
    return 0;
}

int board_link_usb_pd_controller_eeprom_write(uint16_t register_address, const uint8_t *p_data, uint32_t length)
{
    if (tps25751_eeprom_write(s_usb_pd.tps25751, register_address, p_data, length) != 0)
    {
        log_error("EEPROM write failed (register 0x%04X)", register_address);
        return -1;
    }
    return 0;
}

bool board_link_usb_pd_controller_is_ready(void)
{
    return s_usb_pd.is_ready;
}

bool board_link_usb_pd_controller_is_plug_connected(void)
{
    return s_usb_pd.is_plug_connection_present;
}

bool board_link_usb_pd_controller_is_power_connected(void)
{
    return s_usb_pd.is_power_connection_present;
}

bool board_link_usb_pd_controller_is_battery_dead(void)
{
    return s_usb_pd.is_dead_battery_indicated;
}

int board_link_usb_pd_controller_swap_to_srouce(void)
{
    log_debug("USB PD: Swap to source");
    if (tps25751_swap_pd_role_to_source(s_usb_pd.tps25751) < 0)
    {
        return -1;
    }
    return 0;
}

static const char *typec_mode_to_string(typec_mode_t mode)
{
    switch (mode)
    {
        case TYPEC_DRP_MODE:
            return "DRP";
        case TYPEC_SINK_ONLY_MODE:
            return "Sink Only";
        case TYPEC_SOURCE_ONLY_MODE:
            return "Source Only";
        case TYPEC_DISABLED_MODE:
            return "Disabled";
        default:
            return "Unknown";
    }
}

void board_link_usb_pd_controller_select_mode(typec_mode_t mode)
{
    log_debug("USB PD: select mode: %s", typec_mode_to_string(mode));
    switch (mode)
    {
        case TYPEC_DRP_MODE:
            tps25751_set_port_configuration_typec_state_machine(s_usb_pd.tps25751, TPS25751_DRP_SM);
            break;
        case TYPEC_SINK_ONLY_MODE:
            tps25751_set_port_configuration_typec_state_machine(s_usb_pd.tps25751, TPS25751_SINK_SM_ONLY);
            break;
        case TYPEC_SOURCE_ONLY_MODE:
            tps25751_set_port_configuration_typec_state_machine(s_usb_pd.tps25751, TPS25751_SOURCE_SM_ONLY);
            break;
        case TYPEC_DISABLED_MODE:
            tps25751_set_port_configuration_typec_state_machine(s_usb_pd.tps25751, TPS25751_DISABLED);
            break;
    }
}

void board_link_usb_pd_controller_set_max_source_current(usb_pd_max_source_current_t source_current)
{
    // Default to 1A
    tps25751_pd_max_source_current_t pd_max_source_current = TPS25751_PD_MAX_SOURCE_CURRENT_1A;

    switch (source_current)
    {
        case USB_PD_MAX_SOURCE_CURRENT_0A:
            pd_max_source_current = TPS25751_PD_MAX_SOURCE_CURRENT_0A;
            break;
        default:
            // Keep default value in any other cases
            break;
    }

    log_debug("USB PD: set max source current: %dA", source_current == USB_PD_MAX_SOURCE_CURRENT_0A ? 0 : 1);
    tps25751_set_max_source_current(s_usb_pd.tps25751, pd_max_source_current);
}

void board_link_usb_pd_controller_set_max_source_voltage(usb_pd_max_source_voltage_t source_voltage)
{
    // Default to 5V
    tps25751_sink_max_voltage_t sink_max_voltage = TPS25751_SINK_MAX_VOLTAGE_5V;

    if (source_voltage == USB_PD_MAX_SOURCE_VOLTAGE_20V)
    {
        sink_max_voltage = TPS25751_SINK_MAX_VOLTAGE_20V;
    }

    log_debug("USB PD: set max source voltage: %dV", source_voltage == USB_PD_MAX_SOURCE_VOLTAGE_20V ? 20 : 5);
    tps25751_set_max_charger_voltage(s_usb_pd.tps25751, sink_max_voltage);
}

int board_link_usb_pd_controller_fw_version(uint8_t *p_pd_fw_version)
{
    uint8_t customer_use_reg[8] = {0};
    if (tps25751_read_customer_use_reg(s_usb_pd.tps25751, customer_use_reg) != 0)
    {
        log_error("Failed to get pd fw version");
        return -1;
    }

    // PD FW version is written to byte 0, ex. 0x21 = v2.1
    *p_pd_fw_version = customer_use_reg[0];
    return 0;
}

static void thread_sleep_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}
