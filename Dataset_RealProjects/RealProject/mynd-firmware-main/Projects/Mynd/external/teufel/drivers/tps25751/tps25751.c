#include "tps25751.h"
#include <string.h>

#define LOG_MODULE_NAME "tps25751.c"
#define LOG_LEVEL       LOG_LEVEL_INFO
#include "driver_logger.h"

#if defined(FreeRTOS) && defined(configSUPPORT_DYNAMIC_ALLOCATION) && (configSUPPORT_DYNAMIC_ALLOCATION == 1)
#include "FreeRTOS.h"
#else
#include <stdlib.h>
#endif

#define TPS25751_REG_MODE                (0x03)
#define TPS25751_REG_CUSTOMER_USE        (0x06)
#define TPS25751_REG_CMD1                (0x08)
#define TPS25751_REG_DATA1               (0x09)
#define TPS25751_REG_INT_EVENT1          (0x14)
#define TPS25751_REG_INT_MASK1           (0x16)
#define TPS25751_REG_INT_CLEAR1          (0x18)
#define TPS25751_REG_STATUS              (0x1A)
#define TPS25751_REG_POWER_PATH_STATUS   (0x26)
#define TPS25751_REG_PORT_CONFIGURATION  (0x28)
#define TPS25751_REG_PORT_CONTROL        (0x29)
#define TPS25751_REG_BOOT_FLAGS          (0x2D)
#define TPS25751_REG_RX_SOURCE_CAPS      (0x30)
#define TPS25751_REG_RX_SINK_CAPS        (0x31)
#define TPS25751_REG_TX_SOURCE_CAPS      (0x32)
#define TPS25751_REG_TX_SINK_CAPS        (0x33)
#define TPS25751_REG_ACTIVE_PDO_CONTRACT (0x34)
#define TPS25751_REG_ACTIVE_RDO_CONTRACT (0x35)
#define TPS25751_REG_AUTONEGOTIATE_SINK  (0x37)
#define TPS25751_REG_SPM_CLIENT_CONTROL  (0x3C)
#define TPS25751_REG_POWER_STATUS        (0x3F)
#define TPS25751_REG_PD_STATUS           (0x40)
#define TPS25751_REG_TYPEC_STATE         (0x69)
#define TPS25751_REG_SLEEP_CONTROL       (0x70)
#define TPS25751_REG_GPIO_STATUS         (0x72)
#define TPS25751_REG_MOISTURE_DETECTION  (0x98)

struct tps25751_handler
{
    tps25751_i2c_read_fn     i2c_read_fn;
    tps25751_i2c_write_fn    i2c_write_fn;
    tps25751_thread_sleep_fn thread_sleep_fn;
    uint8_t                  i2c_device_address;
};

static int tps25751_read_register(const tps25751_handler_t *h, uint8_t register_address, uint8_t *p_data,
                                  uint32_t length);
static int tps25751_write_register(const tps25751_handler_t *h, uint8_t register_address, const uint8_t *p_data,
                                   uint32_t length);

tps25751_handler_t *tps25751_init(const tps25751_config_t *p_config)
{
    if (p_config == NULL)
    {
        return NULL;
    }

    if (p_config->i2c_read_fn == NULL || p_config->i2c_write_fn == NULL || p_config->thread_sleep_fn == NULL)
    {
        return NULL;
    }

#if defined(FreeRTOS) && defined(configSUPPORT_DYNAMIC_ALLOCATION) && (configSUPPORT_DYNAMIC_ALLOCATION == 1)
    struct tps25751_handler *h = pvPortMalloc(sizeof(struct tps25751_handler));
#else
    struct tps25751_handler *h = malloc(sizeof(struct tps25751_handler));
#endif

    if (h == NULL)
    {
        return NULL;
    }

    h->i2c_read_fn        = p_config->i2c_read_fn;
    h->i2c_write_fn       = p_config->i2c_write_fn;
    h->thread_sleep_fn    = p_config->thread_sleep_fn;
    h->i2c_device_address = p_config->i2c_device_address;
    return h;
}

int tps25751_get_device_mode(const tps25751_handler_t *h, tps25751_device_mode_t *p_mode)
{
    // 1 byte for byte count, 4 bytes for register, 1 byte to null-terminate the string
    uint8_t  data[6]         = {0};
    uint8_t *p_register_data = &data[1];

    if (tps25751_read_register(h, TPS25751_REG_MODE, data, 5) != 0)
    {
        return -E_TPS25751_IO;
    }

    if (strncmp((const char *) p_register_data, "APP ", 4) == 0)
    {
        *p_mode = TPS25751_DEVICE_MODE_APP;
    }
    else if (strncmp((const char *) p_register_data, "BOOT", 4) == 0)
    {
        *p_mode = TPS25751_DEVICE_MODE_BOOT;
    }
    else if (strncmp((const char *) p_register_data, "PTCH", 4) == 0)
    {
        *p_mode = TPS25751_DEVICE_MODE_PATCH;
    }
    else
    {
        *p_mode = TPS25751_DEVICE_MODE_LIMITED;
    }

    log_debug("Device mode[%d]: %s", *p_mode, &data[1]);

    return E_TPS25751_OK;
}

int tps25751_get_device_status(const tps25751_handler_t *h, tps25751_status_t *p_status)
{
    // 1 byte for byte count, 5 bytes for register
    uint8_t  data[6]         = {0};
    uint8_t *p_register_data = &data[1];

    if (tps25751_read_register(h, TPS25751_REG_STATUS, data, 6) != 0)
    {
        return -E_TPS25751_IO;
    }

    p_status->is_plug_connected          = (p_register_data[0] & 0x01) == 1;
    p_status->connection_state           = (tps25751_connection_state_t) ((p_register_data[0] >> 1) & 0x03);
    p_status->plug_orientation           = (tps25751_plug_orientation_t) ((p_register_data[0] >> 4) & 0x01);
    p_status->port_role                  = (tps25751_port_role_t) ((p_register_data[0] >> 5) & 0x01);
    p_status->data_role                  = (tps25751_data_role_t) ((p_register_data[0] >> 6) & 0x01);
    p_status->vbus_status                = (tps25751_vbus_status_t) ((p_register_data[2] >> 4) & 0x03);
    p_status->usb_host_attachment_status = (tps25751_usb_host_attachment_status_t) ((p_register_data[2] >> 6) & 0x03);
    p_status->legacy_status              = (tps25751_legacy_status_t) (p_register_data[3] & 0x03);
    p_status->is_bist_procedure_in_progress = ((p_register_data[3] >> 3) & 0x01) == 1;
    p_status->soc_ack_timeout               = ((p_register_data[3] >> 6) & 0x01) == 1;

    log_debug("Device status: [ %02X %02X %02X %02X %02X ]", p_register_data[0], p_register_data[1], p_register_data[2],
              p_register_data[3], p_register_data[4]);

    return E_TPS25751_OK;
}

int tps25751_get_boot_flags(const tps25751_handler_t *h, tps25751_boot_flags_t *p_flags)
{
    // 1 byte for byte count, 5 bytes for register
    uint8_t  data[6]         = {0};
    uint8_t *p_register_data = &data[1];

    if (tps25751_read_register(h, TPS25751_REG_BOOT_FLAGS, data, 6) != 0)
    {
        return -E_TPS25751_IO;
    }

    p_flags->patch_header_error_detected       = (p_register_data[0] & 0x01) == 1;
    p_flags->dead_battery_indicated            = ((p_register_data[0] >> 2) & 0x01) == 1;
    p_flags->i2c_eeprom_present                = ((p_register_data[0] >> 3) & 0x01) == 1;
    p_flags->patch_download_error_detected     = ((p_register_data[1] >> 2) & 0x01) == 1;
    p_flags->system_thermal_shutdown_indicated = ((p_register_data[2] >> 3) & 0x01) == 1;
    p_flags->patch_config_source               = (tps25751_patch_config_source_t) ((p_register_data[3] >> 5) & 0x07);
    p_flags->pd_controller_revision_id         = p_register_data[4];

    log_debug("Boot flags: [ %02X %02X %02X %02X %02X ]", p_register_data[0], p_register_data[1], p_register_data[2],
              p_register_data[3], p_register_data[4]);

    return E_TPS25751_OK;
}

int tps25751_get_power_status(const tps25751_handler_t *h, tps25751_power_status_t *p_status)
{
    // 1 byte for byte count, 2 bytes for register
    uint8_t  data[3]         = {0};
    uint8_t *p_register_data = &data[1];

    if (tps25751_read_register(h, TPS25751_REG_POWER_STATUS, data, 3) != 0)
    {
        return -E_TPS25751_IO;
    }

    p_status->connection_present       = (p_register_data[0] & 0x01) == 1;
    p_status->pd_role                  = (tps25751_pd_role_t) ((p_register_data[0] >> 1) & 0x01);
    p_status->type_c_current           = (tps25751_type_c_current_t) ((p_register_data[0] >> 2) & 0x03);
    p_status->charger_detect_status    = (tps25751_charger_detect_status_t) ((p_register_data[0] >> 4) & 0x0F);
    p_status->charger_advertise_status = (tps25751_charger_advertise_status_t) (p_register_data[1] & 0x03);

    log_debug("Power status: [ %02X %02X ]", p_register_data[0], p_register_data[1]);

    return E_TPS25751_OK;
}

int tps25751_load_patch_bundle(const tps25751_handler_t *h, const uint8_t *p_patch_data, uint32_t patch_length)
{
    uint8_t data[12] = {0};
    int     retries;

    // Send 'GO2P' 4CC command
    if (tps25751_send_4cc_command(h, "GO2P", 5000) != 0)
    {
        return -E_TPS25751_IO;
    }

    // Read output data of 'GO2P' 4CC command
    if (tps25751_read_register(h, TPS25751_REG_DATA1, data, 2) != 0)
    {
        log_error("Failed to read output data of PBMs command");
        return -E_TPS25751_IO;
    }

    // Check that the output data has status "Success"
    if ((data[1] & 0x0F) != 0x00)
    {
        log_error("Invalid go to patch mode status: 0x%02X", data[1]);
        return -E_TPS25751_STATE;
    }

    // Wait until the PD controller is ready to be patched
    retries = 3;
    do
    {
        if (tps25751_read_register(h, TPS25751_REG_INT_EVENT1, data, 12) == 0)
        {
            bool ready_for_patch = ((data[11] >> 1) & 0x01) == 1;
            if (ready_for_patch)
            {
                break;
            }
        }
        h->thread_sleep_fn(10);
        retries--;
    } while (retries > 0);

    if (retries == 0)
    {
        log_error("PD controller is not ready for patch");
        return -E_TPS25751_STATE;
    }

    // Make sure the device is in patch mode
    retries = 3;
    do
    {
        tps25751_device_mode_t mode;
        if (tps25751_get_device_mode(h, &mode) == 0)
        {
            if (mode == TPS25751_DEVICE_MODE_PATCH)
            {
                break;
            }
        }
        h->thread_sleep_fn(10);
        retries--;
    } while (retries > 0);

    if (retries == 0)
    {
        log_error("PD controller is not in patch mode");
        return -E_TPS25751_STATE;
    }

    // Write input data for 'PBMs' 4CC command
    data[0] = 6;
    data[1] = (uint8_t) (patch_length & 0xFF);
    data[2] = (uint8_t) ((patch_length >> 8) & 0xFF);
    data[3] = (uint8_t) ((patch_length >> 16) & 0xFF);
    data[4] = (uint8_t) ((patch_length >> 24) & 0xFF);
    data[5] = h->i2c_device_address;
    data[6] = 50; // 5 seconds timeout in 100 ms units
    if (tps25751_write_register(h, TPS25751_REG_DATA1, data, 7) != 0)
    {
        log_error("Failed to write input data for PBMs command");
        return -E_TPS25751_IO;
    }

    // Send 'PBMs' 4CC command
    if (tps25751_send_4cc_command(h, "PBMs", 5000) != 0)
    {
        return -E_TPS25751_IO;
    }

    // Read output data of 'PBMs' 4CC command
    if (tps25751_read_register(h, TPS25751_REG_DATA1, data, 2) != 0)
    {
        log_error("Failed to read output data of PBMs command");
        return -E_TPS25751_IO;
    }

    // Check that the output data has status "Patch start success"
    if (data[1] != 0x00)
    {
        log_error("Invalid patch start status: 0x%02X", data[1]);
        return -E_TPS25751_STATE;
    }

    if (tps25751_write_register(h, TPS25751_REG_DATA1, p_patch_data, patch_length) != 0)
    {
        log_error("Failed to write patch data");

        // End the download sequence
        tps25751_send_4cc_command(h, "PBMe", 5000);
        return -E_TPS25751_IO;
    }

    // Delay at least 500 us according to reference manual
    h->thread_sleep_fn(10);

    if (tps25751_send_4cc_command(h, "PBMc", 5000) != 0)
    {
        // End the download sequence
        tps25751_send_4cc_command(h, "PBMe", 5000);
        return -E_TPS25751_IO;
    }

    // Delay 20 ms according to reference manual
    h->thread_sleep_fn(20);

    // Read output data of 'PBMc' 4CC command
    if (tps25751_read_register(h, TPS25751_REG_DATA1, data, 2) != 0)
    {
        log_error("Failed to read output data of PBMc command");
        return -E_TPS25751_IO;
    }

    // Make sure that the return code of the 'PBMc' command is success
    if (data[1] != 0x00)
    {
        log_error("Invalid PBMc return code: 0x%02X", data[1]);
        return -E_TPS25751_STATE;
    }

    // Make sure the patch is loaded
    retries = 3;
    do
    {
        if (tps25751_read_register(h, TPS25751_REG_INT_EVENT1, data, 12) == 0)
        {
            bool patch_loaded = (data[11] & 0x01) == 1;
            if (patch_loaded)
            {
                break;
            }
        }
        h->thread_sleep_fn(10);
        retries--;
    } while (retries > 0);

    if (retries == 0)
    {
        log_error("Patch loading timed out");
        return -E_TPS25751_TIMEOUT;
    }

    // Make sure the device is in app mode
    retries = 3;
    do
    {
        tps25751_device_mode_t mode;
        if (tps25751_get_device_mode(h, &mode) == 0)
        {
            if (mode == TPS25751_DEVICE_MODE_APP)
            {
                break;
            }
        }
        h->thread_sleep_fn(10);
        retries--;
    } while (retries > 0);

    if (retries == 0)
    {
        log_error("Switching to app mode timed out");
        return -E_TPS25751_TIMEOUT;
    }

    log_info("PD controller patch downloaded successfully");
    return 0;
}

int tps25751_clear_dead_battery_flag(const tps25751_handler_t *h)
{
    return tps25751_send_4cc_command(h, "DBfg", 1000);
}

int tps25751_send_4cc_command(const tps25751_handler_t *h, const char *command, uint32_t timeout_ms)
{
    uint8_t data[5] = {4, command[0], command[1], command[2], command[3]};

    if (tps25751_write_register(h, TPS25751_REG_CMD1, data, 5) == 0)
    {
        // It takes around 500-800 us for the command to be processed
        h->thread_sleep_fn(2);

        // Divided by 4 because we wait 4 ms between retries
        uint32_t retries_left = timeout_ms / 4;
        do
        {
            if (tps25751_read_register(h, TPS25751_REG_CMD1, data, 5) != 0)
            {
                log_error("Failed to read CMD1 register");
                break;
            }

            uint8_t zeroed_buffer[4] = {0, 0, 0, 0};
            if (memcmp(&data[1], zeroed_buffer, 4) == 0)
            {
                return E_TPS25751_OK;
            }

            if (memcmp(&data[1], "!CMD", 4) == 0)
            {
                log_error("Command rejected");
                break;
            }

            // Wait 4 ms before retrying
            h->thread_sleep_fn(4);
            retries_left--;
        } while (retries_left > 0);

        if (retries_left == 0)
        {
            log_error("Command timed out");
            return -E_TPS25751_TIMEOUT;
        }
    }

    log_error("Command %s was not processed successfully", command);
    return -E_TPS25751_IO;
}

int tps25751_read_customer_use_reg(const tps25751_handler_t *h, uint8_t *p_register_data)
{
    // 1 byte for byte count, 8 bytes for register
    uint8_t data[9] = {0};

    if (tps25751_read_register(h, TPS25751_REG_CUSTOMER_USE, data, 9) != 0)
    {
        log_error("Customer Use register read failed.");
        return -E_TPS25751_IO;
    }

    for (uint8_t i = 0; i < 8; ++i)
    {
        p_register_data[i] = data[i + 1];
    }

    log_debug("Customer Use register: [ %02X %02X %02X %02X %02X %02X %02X %02X ]", data[1], data[2], data[3], data[4],
              data[5], data[6], data[7], data[8]);

    return E_TPS25751_OK;
}

int tps25751_set_port_configuration_typec_state_machine(const tps25751_handler_t           *h,
                                                        tps25751_port_typec_state_machine_t sm)
{
    uint8_t data[16] = {0};

    if (tps25751_read_register(h, TPS25751_REG_PORT_CONFIGURATION, data, 16) != 0)
    {
        log_error("Register read failed.");
        return -E_TPS25751_IO;
    }

    data[1] = (uint8_t) sm;

    tps25751_write_register(h, TPS25751_REG_PORT_CONFIGURATION, data, 16);

    return E_TPS25751_OK;
}

int tps25751_set_max_source_current(const tps25751_handler_t *h, tps25751_pd_max_source_current_t source_current)
{
    uint8_t data[12] = {0};

    if (tps25751_read_register(h, TPS25751_REG_TX_SOURCE_CAPS, data, 12) != 0)
    {
        log_error("Register read failed.");
        return -E_TPS25751_IO;
    }

    switch (source_current)
    {
        case TPS25751_PD_MAX_SOURCE_CURRENT_0A:
            data[4] = 0x0;
            data[5] &= 0xFC;
            break;
        case TPS25751_PD_MAX_SOURCE_CURRENT_1A:
            data[4] = 0x64;
            data[5] &= 0xFC;
            break;
        case TPS25751_PD_MAX_SOURCE_CURRENT_2A:
            data[4] = 0xC8;
            data[5] &= 0xFC;
            break;
        case TPS25751_PD_MAX_SOURCE_CURRENT_3A:
            data[4] = 0x2C;
            data[5] &= 0xFD;
            break;
        default:
            log_error("Invalid source current value.");
            return -E_TPS25751_PARAM;
    }

    if (tps25751_write_register(h, TPS25751_REG_TX_SOURCE_CAPS, data, 12) != 0)
    {
        log_error("Register write failed.");
        return -E_TPS25751_IO;
    }

    // Apply source capabilities
    if (tps25751_send_4cc_command(h, "SSrC", 5000) != 0)
    {
        log_error("Failed to send SSrC command");
        return -E_TPS25751_IO;
    }

    // Read output data of 'SSrC' 4CC command
    if (tps25751_read_register(h, TPS25751_REG_DATA1, data, 2) != 0)
    {
        log_error("Failed to read output data of SSrC command");
        return -E_TPS25751_IO;
    }

    // Check the return code
    if (data[1] != 0x00)
    {
        log_error("Invalid SSrC return code: 0x%02X", data[1]);
        return -E_TPS25751_STATE;
    }

    return E_TPS25751_OK;
}

int tps25751_set_max_charger_voltage(const tps25751_handler_t *h, tps25751_sink_max_voltage_t voltage)
{
    uint8_t data[8] = {0x07, 0x0B, 0xC0, 0x12, 0x41, 0x65, 0x7C, 0x01}; // 5V default

    if (voltage == TPS25751_SINK_MAX_VOLTAGE_20V)
    {
        data[1] = 0x3B;
        data[5] = 0x90;
        data[6] = 0x7D;
    }

    if (tps25751_write_register(h, TPS25751_REG_AUTONEGOTIATE_SINK, data, sizeof(data)) != 0)
    {
        log_error("Register write failed.");
        return -E_TPS25751_IO;
    }

    // Confirm with GSrC command
    if (tps25751_send_4cc_command(h, "GSrC", 5000) != 0)
    {
        log_error("Failed to send GSrC command");
        return -E_TPS25751_IO;
    }

    // Read output data of 'GSrC' 4CC command
    if (tps25751_read_register(h, TPS25751_REG_DATA1, data, 2) != 0)
    {
        log_error("Failed to read output data of GSrC command");
        return -E_TPS25751_IO;
    }

    // Check the return code
    if (data[1] != 0x00)
    {
        log_error("Invalid GSrC return code: 0x%02X", data[1]);
        return -E_TPS25751_STATE;
    }

    return E_TPS25751_OK;
}

int tps25751_eeprom_read(const tps25751_handler_t *h, uint16_t address, uint8_t *p_data, uint32_t length)
{
    uint8_t data[17];

    // Note: This restriction doesn't actually exist in the TPS25751 datasheet
    // This is an intentional decision made in order to not leak driver internals through
    // the API, because otherwise we would need to specify in the driver documentation that the
    // buffer provided to this function would need to be 2 bytes longer than actually needed in order
    // because of the two bytes for the length and the task return code of the 4CC commands
    if (length > 16)
    {
        log_error("Maximum I2C read length exceeded (%d > 16)", length);
        return -E_TPS25751_PARAM;
    }

    // Write input data for 'FLrd' 4CC command
    data[0] = 2;
    data[1] = address & 0xFF;
    data[2] = (address >> 8) & 0xFF;
    if (tps25751_write_register(h, TPS25751_REG_DATA1, data, 3) != 0)
    {
        log_error("Failed to write input data for I2Cr command");
        return -E_TPS25751_IO;
    }

    // Send 'FLrd' 4CC command
    if (tps25751_send_4cc_command(h, "FLrd", 50) != 0)
    {
        return -E_TPS25751_IO;
    }

    // Read output data of 'FLrd' 4CC command
    if (tps25751_read_register(h, TPS25751_REG_DATA1, data, length + 1) != 0)
    {
        log_error("Failed to read output data of FLrd command");
        return -E_TPS25751_IO;
    }

    // Make sure that the return code of the 'FLrd' command is success
    // TODO: needs to be clarified with TI

    // Copy the register data to the user-provided buffer
    memcpy(p_data, &data[1], length);
    return E_TPS25751_OK;
}

int tps25751_eeprom_write(const tps25751_handler_t *h, uint16_t address, const uint8_t *p_data, uint32_t length)
{
    uint8_t data[34];

    if (length > 32)
    {
        log_error("Maximum I2C write length exceeded (%d > 32)", length);
        return -E_TPS25751_PARAM;
    }

    // Write input data for 'FLad' 4CC command
    data[0] = 2;
    data[1] = address & 0xFF;
    data[2] = (address >> 8) & 0xFF;
    if (tps25751_write_register(h, TPS25751_REG_DATA1, data, 3) != 0)
    {
        log_error("Failed to write input data for I2Cr command");
        return -E_TPS25751_IO;
    }

    // Send 'FLrd' 4CC command
    if (tps25751_send_4cc_command(h, "FLad", 50) != 0)
    {
        return -E_TPS25751_IO;
    }

    // Read output data of 'FLad' 4CC command
    if (tps25751_read_register(h, TPS25751_REG_DATA1, data, 2) != 0)
    {
        log_error("Failed to read output data of FLad command");
        return -E_TPS25751_IO;
    }

    // Make sure that the return code of the 'FLad' command is success
    if (data[1] != 0x00)
    {
        log_error("Invalid FLad return code: 0x%02X", data[1]);
        return -E_TPS25751_STATE;
    }

    // Write input data for 'FLwd' 4CC command
    data[0] = length;

    memcpy(&data[1], p_data, length);

    if (tps25751_write_register(h, TPS25751_REG_DATA1, data, length + 1) != 0)
    {
        log_error("Failed to write input data for FLwd command");
        return -E_TPS25751_IO;
    }

    // Send 'FLwd' 4CC command
    if (tps25751_send_4cc_command(h, "FLwd", 50) != 0)
    {
        return -E_TPS25751_IO;
    }

    // Read output data of 'FLwd' 4CC command
    if (tps25751_read_register(h, TPS25751_REG_DATA1, data, 2) != 0)
    {
        log_error("Failed to read output data of FLwd command");
        return -E_TPS25751_IO;
    }

    // Make sure that the return code of the 'FLwd' command is success
    if (data[1] != 0x00)
    {
        log_error("Invalid FLwd return code: 0x%02X", data[1]);
        return -E_TPS25751_STATE;
    }

    return E_TPS25751_OK;
}

int tps25751_i2c_read(const tps25751_handler_t *h, uint8_t i2c_address, uint8_t register_address, uint8_t *p_data,
                      uint32_t length)
{
    uint8_t data[12];

    // Note: This restriction doesn't actually exist in the TPS25751 datasheet
    // This is an intentional decision made in order to not leak driver internals through
    // the API, because otherwise we would need to specify in the driver documentation that the
    // buffer provided to this function would need to be 2 bytes longer than actually needed in order
    // because of the two bytes for the length and the task return code of the 4CC commands
    if (length > 10)
    {
        log_error("Maximum I2C read length exceeded (%d > 10)", length);
        return -E_TPS25751_PARAM;
    }

    // Write input data for 'I2Cr' 4CC command
    data[0] = 3;
    data[1] = i2c_address;
    data[2] = register_address;
    data[3] = length;
    if (tps25751_write_register(h, TPS25751_REG_DATA1, data, 4) != 0)
    {
        log_error("Failed to write input data for I2Cr command");
        return -E_TPS25751_IO;
    }

    // Send 'I2Cr' 4CC command
    if (tps25751_send_4cc_command(h, "I2Cr", 50) != 0)
    {
        return -E_TPS25751_IO;
    }

    // Read output data of 'I2Cr' 4CC command
    if (tps25751_read_register(h, TPS25751_REG_DATA1, data, length + 2) != 0)
    {
        log_error("Failed to read output data of I2Cr command");
        return -E_TPS25751_IO;
    }

    // Make sure that the return code of the 'I2Cr' command is success
    if (data[1] != 0x00)
    {
        log_error("Invalid I2Cr return code: 0x%02X", data[1]);
        return -E_TPS25751_STATE;
    }

    // Copy the register data to the user-provided buffer
    memcpy(p_data, &data[2], length);
    return E_TPS25751_OK;
}

int tps25751_i2c_write(const tps25751_handler_t *h, uint8_t i2c_address, uint8_t register_address,
                       const uint8_t *p_data, uint32_t length)
{
    if (length > 10)
    {
        log_error("Maximum I2C write length exceeded (%d > 10)", length);
        return -E_TPS25751_PARAM;
    }

    uint8_t data[15];

    // Write input data for 'I2Cw' 4CC command
    data[0] = length + 4;
    data[1] = i2c_address;
    data[2] = length + 1; // +1 because the register address byte also counts as payload data

    // Note: The TPS25751 reference manual specifies that a reserved byte should be written after the length byte,
    // however, this is confirmed by TI to be a mistake in the reference manual and that byte should not be written.

    data[3] = register_address;
    memcpy(&data[4], p_data, length);

    if (tps25751_write_register(h, TPS25751_REG_DATA1, data, length + 4) != 0)
    {
        log_error("Failed to write input data for I2Cw command");
        return -E_TPS25751_IO;
    }

    // Send 'I2Cw' 4CC command
    if (tps25751_send_4cc_command(h, "I2Cw", 50) != 0)
    {
        return -E_TPS25751_IO;
    }

    // Read output data of 'I2Cw' 4CC command
    if (tps25751_read_register(h, TPS25751_REG_DATA1, data, 2) != 0)
    {
        log_error("Failed to read output data of I2Cw command");
        return -E_TPS25751_IO;
    }

    // Make sure that the return code of the 'I2Cw' command is success
    if (data[1] != 0x00)
    {
        log_error("Invalid I2Cw return code: 0x%02X", data[1]);
        return -E_TPS25751_STATE;
    }

    return E_TPS25751_OK;
}

static int tps25751_read_register(const tps25751_handler_t *h, uint8_t register_address, uint8_t *p_data,
                                  uint32_t length)
{
    return (h->i2c_read_fn(h->i2c_device_address, register_address, p_data, length) == 0) ? E_TPS25751_OK
                                                                                          : -E_TPS25751_IO;
}

static int tps25751_write_register(const tps25751_handler_t *h, uint8_t register_address, const uint8_t *p_data,
                                   uint32_t length)
{
    return (h->i2c_write_fn(h->i2c_device_address, register_address, p_data, length) == 0) ? E_TPS25751_OK
                                                                                           : -E_TPS25751_IO;
}

int tps25751_swap_pd_role_to_source(const tps25751_handler_t *h)
{
    if (tps25751_send_4cc_command(h, "SWSr", 5000) != 0)
    {
        log_error("Failed to send SSrC command");
        return -E_TPS25751_IO;
    }

    return E_TPS25751_OK;
}
