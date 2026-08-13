// Due to the flash size limit, only the WARNING level is available
// for use in the complete firmware (including the bootloader).
#if defined(BOOTLOADER)
#define LOG_LEVEL LOG_LEVEL_ERROR
#else
#define LOG_LEVEL LOG_LEVEL_INFO
#endif

#include "board_link_charger.h"
#include "board_link_usb_pd_controller.h"
#include "board_hw.h"
#include "bq25713.h"
#include <string.h>
#include "logger.h"
#include "external/teufel/libs/app_assert/app_assert.h"

#define ZERO_CHARGE_CURRENT_MA (0U)
#define SLOW_CHARGE_CURRENT_MA (2500U)
#define FAST_CHARGE_CURRENT_MA (5000U)

#define SLOW_CHARGE_VOLTAGE_MV (8200U)
#define FAST_CHARGE_VOLTAGE_MV (8400U)

static const bq25713_config_t bq25713_config = {
    .i2c_device_address = BQ25713_I2C_ADDRESS,
    .i2c_write_fn       = board_link_usb_pd_controller_i2c_write,
    .i2c_read_fn        = board_link_usb_pd_controller_i2c_read,
};

static struct
{
    bq25713_handler_t *bq25713;
} s_charger;

void board_link_charger_init(void)
{
    s_charger.bq25713 = bq25713_init(&bq25713_config);
    if (s_charger.bq25713 == NULL)
    {
        log_error("Failed to initialize charger");
    }
}

void board_link_charger_enable_low_power_mode(bool enable)
{
    // Enable/disable low power mode
    bq25713_set_low_power_mode(s_charger.bq25713, enable);
}

int board_link_charger_setup(void)
{
    uint8_t data[2];

    if (bq25713_get_device_id(s_charger.bq25713, &data[0]) == 0)
    {
        if (data[0] == 0x88)
        {
            log_trace("Device ID OK");
        }
        else
        {
            log_error("Unexpected device ID");
            return -1;
        }
    }
    else
    {
        log_error("Failed to get device ID");
        return -1;
    }

    // Disable low power mode
    board_link_charger_enable_low_power_mode(false);

    // Enable PSys sensing circuit
    data[0] = 0x12;
    if (bq25713_write_register(s_charger.bq25713, BQ25713_REG_CHARGE_OPTION1_MSB, data, 1) != 0)
    {
        log_error("Failed to enable PSys");
        return -1;
    }

    // Enable ADCs for VBat, IChg, IDchg and PSys
    data[0] = 0x2D;

    // Set ADC full scale range to 0-2.04V and resolution of 8 mV/bit
    // Start continuous ADC conversions every one second
    data[1] = 0xC0;
    if (bq25713_write_register(s_charger.bq25713, BQ25713_REG_ADC_OPTION_LSB, data, 2) != 0)
    {
        log_error("Failed to configure ADCs");
        return -1;
    }

    return 0;
}

void board_link_charger_enable_fast_charge(bool enable)
{
    log_debug("Fast charge: %s", enable ? "enabled" : "disabled");

    uint16_t charge_current_ma = enable ? FAST_CHARGE_CURRENT_MA : SLOW_CHARGE_CURRENT_MA;
    uint16_t charge_voltage_mv = enable ? FAST_CHARGE_VOLTAGE_MV : SLOW_CHARGE_VOLTAGE_MV;
    bq25713_set_charge_current_ma(s_charger.bq25713, charge_current_ma);
    bq25713_set_max_charge_voltage_mv(s_charger.bq25713, charge_voltage_mv);
}

void board_link_charger_disable(void)
{
    log_debug("Disable charger");
    bq25713_set_charge_current_ma(s_charger.bq25713, ZERO_CHARGE_CURRENT_MA);
}

bool board_link_charger_is_fast_charge_enabled(void)
{
    uint16_t charge_current_ma;
    if (bq25713_get_charge_current_ma(s_charger.bq25713, &charge_current_ma) != 0)
    {
        return false;
    }

    return charge_current_ma > SLOW_CHARGE_CURRENT_MA;
}

int board_link_charger_get_battery_voltage(uint16_t *p_voltage_mv)
{
    return bq25713_get_battery_voltage_mv(s_charger.bq25713, p_voltage_mv);
}

int board_link_charger_get_status(board_link_charger_status_t *p_charger_status)
{
    // TODO: the API is ambiguous in regards of the charger status. The bit 2 (IN_FCHRG) in the register 0x21 is
    // always set to 1, even if the charger is not charging the battery or the current is very low.
    // Because of that, we cannot rely on this register to determine the charger status. For example, when the battery
    // is fully charged, the charger status is still "charging" because the IN_FCHRG bit is set to 1.

    bq25713_charger_status_t status;
    bq25713_fault_status_t   fault_status;
    if (bq25713_get_charger_status(s_charger.bq25713, &status, &fault_status) != 0)
    {
        *p_charger_status = CHARGER_STATUS_UNDEFINED;
        return -1;
    }

    /*
    if (*(uint8_t *) &(fault_status) != 0x0)
    {
        log_err("Charger fault status: 0x%02X", *(uint8_t *) &(fault_status));
        *p_charger_status = CHARGER_STATUS_FAULT;

        // TODO: recover from fault(SYSOVP_STAT and SYS_SHORT recoverable)
        return 0;
    }
    */

    if (status.is_input_present)
    {
        *p_charger_status = CHARGER_STATUS_CONNECTED;
        return 0;
    }

    *p_charger_status = CHARGER_STATUS_NOT_CONNECTED;

    return 0;
}

int board_link_charger_get_ac_plugged_status(bool *p_ac_plugged)
{
    bq25713_charger_status_t status;
    bq25713_fault_status_t   fault_status;
    if (bq25713_get_charger_status(s_charger.bq25713, &status, &fault_status) != 0)
    {
        return -1;
    }

    *p_ac_plugged = status.is_input_present;
    return 0;
}

int board_link_charger_disable_charging(bool disable)
{
    uint8_t charge_inhibit = (disable) ? 1 : 0;
    return bq25713_set_charge_inhibit(s_charger.bq25713, charge_inhibit);
}
