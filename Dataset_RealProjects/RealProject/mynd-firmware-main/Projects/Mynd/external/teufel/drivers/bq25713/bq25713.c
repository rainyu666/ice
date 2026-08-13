#include "bq25713.h"
#include "stddef.h"

#if defined(FreeRTOS) && defined(configSUPPORT_DYNAMIC_ALLOCATION) && (configSUPPORT_DYNAMIC_ALLOCATION == 1)
#include "FreeRTOS.h"
#else
#include <stdlib.h>
#endif

struct bq25713_handler
{
    bq25713_i2c_read_fn  i2c_read_fn;
    bq25713_i2c_write_fn i2c_write_fn;
    uint8_t              i2c_device_address;
};

static int rmw_reg(const struct bq25713_handler *h, uint8_t reg, uint8_t *mask, uint8_t *data)
{
    uint8_t reg_data[2];
    int            ret;

    ret = h->i2c_read_fn(h->i2c_device_address, reg, reg_data, 2);
    if (ret < 0)
    {
        return -1;
    }

    reg_data[0] &= ~mask[0];
    reg_data[1] &= ~mask[1];
    reg_data[0] |= data[0];
    reg_data[1] |= data[1];

    ret = h->i2c_write_fn(h->i2c_device_address, reg, reg_data, 2);
    if (ret < 0)
    {
        return -1;
    }

    return 0;
}


bq25713_handler_t *bq25713_init(const bq25713_config_t *p_config)
{
    if (p_config == NULL)
    {
        return NULL;
    }

    if (p_config->i2c_read_fn == NULL || p_config->i2c_write_fn == NULL)
    {
        return NULL;
    }

#if defined(FreeRTOS) && defined(configSUPPORT_DYNAMIC_ALLOCATION) && (configSUPPORT_DYNAMIC_ALLOCATION == 1)
    struct bq25713_handler *h = pvPortMalloc(sizeof(struct bq25713_handler));
#else
    struct bq25713_handler *h = malloc(sizeof(struct bq25713_handler));
#endif

    if (h == NULL)
    {
        return NULL;
    }

    h->i2c_read_fn        = p_config->i2c_read_fn;
    h->i2c_write_fn       = p_config->i2c_write_fn;
    h->i2c_device_address = p_config->i2c_device_address;
    return h;
}

int bq25713_get_device_id(const bq25713_handler_t *h, uint8_t *p_device_id)
{
    return bq25713_read_register(h, BQ25713_REG_DEVICE_ID, p_device_id, 1);
}

int bq25713_get_charger_status(const bq25713_handler_t *h, bq25713_charger_status_t *p_charger_status,
                               bq25713_fault_status_t *p_fault_status)
{
    uint8_t data[2];
    if (bq25713_read_register(h, BQ25713_REG_CHARGER_STATUS, data, 2) == 0)
    {
        *p_fault_status   = *((bq25713_fault_status_t *)(data));
        *p_charger_status = *((bq25713_charger_status_t *)data + 1);
        return 0;
    }
    return -1;
}

int bq25713_get_max_charge_voltage_mv(const bq25713_handler_t *h, uint16_t *p_voltage_mv)
{
    uint8_t data[2];
    if (bq25713_read_register(h, BQ25713_REG_CHARGE_VOLTAGE, data, 2) != 0)
    {
        return -1;
    }

    *p_voltage_mv = ((uint16_t)data[1]) << 8 | data[0];
    return 0;
}

int bq25713_set_max_charge_voltage_mv(const bq25713_handler_t *h, uint16_t voltage_mv)
{
    uint8_t data[2];

    // Lower 3 bits are reserved, the voltage can only be set in 8 mV steps
    data[0] = voltage_mv & 0xF8;

    // Top bit is reserved
    data[1] = (voltage_mv >> 8) & 0x7F;
    return bq25713_write_register(h, BQ25713_REG_CHARGE_VOLTAGE, data, 2);
}

int bq25713_get_charge_current_ma(const bq25713_handler_t *h, uint16_t *p_current_ma)
{
    uint8_t data[2];
    if (bq25713_read_register(h, BQ25713_REG_CHARGE_CURRENT, data, 2) != 0)
    {
        return -1;
    }

    *p_current_ma = ((uint16_t)data[1]) << 8 | data[0];
    return 0;
}

int bq25713_set_charge_current_ma(const bq25713_handler_t *h, uint16_t current_ma)
{
    uint8_t data[2];

    // Lower 6 bits are reserved, the current can only be set in 64 mA steps
    data[0] = current_ma & 0xC0;

    // Top 3 bits are reserved
    data[1] = (current_ma >> 8) & 0x1F;
    return bq25713_write_register(h, BQ25713_REG_CHARGE_CURRENT, data, 2);
}

int bq25713_get_battery_voltage_mv(const bq25713_handler_t *h, uint16_t *p_vbat)
{
    uint8_t data;
    if (bq25713_read_register(h, BQ25713_REG_ADC_VBAT, &data, 1) != 0)
    {
        return -1;
    }

    // Battery voltage measurement range starts at 2.88 V
    // LSB is 64 mV
    *p_vbat = 2880 + (data * 64);
    return 0;
}

int bq25713_get_system_voltage_mv(const bq25713_handler_t *h, uint16_t *p_vsys)
{
    uint8_t data;
    if (bq25713_read_register(h, BQ25713_REG_ADC_VSYS, &data, 1) != 0)
    {
        return -1;
    }

    // System voltage measurement range starts at 2.88 V
    // LSB is 64 mV
    *p_vsys = 2880 + (data * 64);
    return 0;
}

int bq25713_get_input_voltage_mv(const bq25713_handler_t *h, uint16_t *p_vbus)
{
    uint8_t data;
    if (bq25713_read_register(h, BQ25713_REG_ADC_VBUS, &data, 1) != 0)
    {
        return -1;
    }

    // Input voltage measurement range starts at 3200 mV
    // LSB is 64 mV
    *p_vbus = 3200 + (data * 64);
    return 0;
}

int bq25713_get_input_current_ma(const bq25713_handler_t *h, uint16_t *p_iin)
{
    uint8_t data;
    if (bq25713_read_register(h, BQ25713_REG_ADC_IIN, &data, 1) != 0)
    {
        return -1;
    }

    // LSB is 50 mA
    *p_iin = data * 50;
    return 0;
}

int bq25713_get_adc_charge_current_ma(const bq25713_handler_t *h, uint16_t *p_current)
{
    uint8_t data;
    if (bq25713_read_register(h, BQ25713_REG_ADC_ICHG, &data, 1) != 0)
    {
        return -1;
    }

    // LSB is 64 mA
    *p_current = data * 64;
    return 0;
}

int bq25713_get_adc_discharge_current_ma(const bq25713_handler_t *h, uint16_t *p_current)
{
    uint8_t data;
    if (bq25713_read_register(h, BQ25713_REG_ADC_IDCHG, &data, 1) != 0)
    {
        return -1;
    }

    // LSB is 256 mA
    *p_current = data * 256;
    return 0;
}

int bq25713_get_system_power_mv(const bq25713_handler_t *h, uint16_t *p_system_power)
{
    uint8_t data;
    if (bq25713_read_register(h, BQ25713_REG_ADC_PSYS, &data, 1) != 0)
    {
        return -1;
    }

    // LSB is 12 mV
    *p_system_power = data * 12;
    return 0;
}

int bq25713_set_charge_inhibit(const bq25713_handler_t *h, uint8_t charge_inhibit)
{
    uint8_t data[2];
    data[0] = charge_inhibit;
    data[1] = 0x00;

    uint8_t mask[2];
    mask[0] = 0x01;
    mask[1] = 0x00;

    return rmw_reg(h, BQ25713_REG_CHARGE_OPTION0, mask, data);
}

int bq25713_set_low_power_mode(const bq25713_handler_t *h, bool enable)
{
    uint8_t data[2];
    data[0] = 0x00;
    data[1] = enable ? 0x80 : 0x00;

    uint8_t mask[2] = {0x00, 0x80};

    return rmw_reg(h, BQ25713_REG_CHARGE_OPTION0, mask, data);
}

int bq25713_read_register(const bq25713_handler_t *h, uint8_t register_address, uint8_t *p_data, uint32_t length)
{
    return h->i2c_read_fn(h->i2c_device_address, register_address, p_data, length);
}

int bq25713_write_register(const bq25713_handler_t *h, uint8_t register_address, const uint8_t *p_data, uint32_t length)
{
    return h->i2c_write_fn(h->i2c_device_address, register_address, p_data, length);
}
