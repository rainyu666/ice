#pragma once

#include <stdbool.h>
#include <stdint.h>

#define BQ25713_REG_CHARGE_OPTION0         (0x00)
#define BQ25713_REG_CHARGE_CURRENT         (0x02)
#define BQ25713_REG_CHARGE_VOLTAGE         (0x04)
#define BQ25713_REG_OTG_VOLTAGE            (0x06)
#define BQ25713_REG_OTG_CURRENT            (0x08)
#define BQ25713_REG_INPUT_VOLTAGE          (0x0A)
#define BQ25713_REG_VSYS_MIN               (0x0C)
#define BQ25713_REG_IIN_HOST               (0x0E)
#define BQ25713_REG_CHARGER_STATUS         (0x20)
#define BQ25713_REG_PROCHOT_STATUS         (0x22)
#define BQ25713_REG_IIN_DPM                (0x24)
#define BQ25713_REG_ADC_PSYS               (0x26)
#define BQ25713_REG_ADC_VBUS               (0x27)
#define BQ25713_REG_ADC_IDCHG              (0x28)
#define BQ25713_REG_ADC_ICHG               (0x29)
#define BQ25713_REG_ADC_CMPIN              (0x2A)
#define BQ25713_REG_ADC_IIN                (0x2B)
#define BQ25713_REG_ADC_VBAT               (0x2C)
#define BQ25713_REG_ADC_VSYS               (0x2D)
#define BQ25713_REG_MANUFACTURER_ID        (0x2E)
#define BQ25713_REG_DEVICE_ID              (0x2F)
#define BQ25713_REG_CHARGE_OPTION1_LSB     (0x30)
#define BQ25713_REG_CHARGE_OPTION1_MSB     (0x31)
#define BQ25713_REG_CHARGE_OPTION2_LSB     (0x32)
#define BQ25713_REG_CHARGE_OPTION2_MSB     (0x33)
#define BQ25713_REG_CHARGE_OPTION3_LSB     (0x34)
#define BQ25713_REG_CHARGE_OPTION3_MSB     (0x35)
#define BQ25713_REG_PROCHOT_OPTION0        (0x36)
#define BQ25713_REG_PROCHOT_OPTION1        (0x38)
#define BQ25713_REG_ADC_OPTION_LSB         (0x3A)
#define BQ25713_REG_ADC_OPTION_MSB         (0x3B)
#define BQ25713_REG_CHARGE_OPTION4         (0x3C)
#define BQ25713_REG_VMIN_ACTIVE_PROTECTION (0x3E)

typedef int (*bq25713_i2c_read_fn)(uint8_t i2c_address, uint8_t register_address, uint8_t *p_data, uint32_t length);
typedef int (*bq25713_i2c_write_fn)(uint8_t i2c_address, uint8_t register_address, const uint8_t *p_data,
                                    uint32_t length);

typedef struct bq25713_handler bq25713_handler_t;

typedef struct
{
    bq25713_i2c_read_fn  i2c_read_fn;
    bq25713_i2c_write_fn i2c_write_fn;
    uint8_t              i2c_device_address;
} bq25713_config_t;

typedef struct __attribute__((__packed__))
{
    uint8_t in_otg : 1;
    uint8_t in_precharge : 1;
    uint8_t in_fast_charge : 1;
    uint8_t in_iin_dynamic_power_mgmt : 1;
    uint8_t in_vin_dynamic_power_mgmt : 1;
    uint8_t in_vmin_active_protection_mode : 1;
    uint8_t input_current_optimization_complete : 1;
    uint8_t is_input_present : 1;
} bq25713_charger_status_t;

typedef struct __attribute__((__packed__))
{
    uint8_t otg_undervoltage_fault : 1;
    uint8_t otg_overvoltage_fault : 1;
    uint8_t latchoff_fault : 1;
    uint8_t system_short_fault : 1;
    uint8_t system_overvoltage_fault : 1;
    uint8_t input_overcurrent_fault : 1;
    uint8_t battery_overcurrent_fault : 1;
    uint8_t input_overvoltage_fault : 1;
} bq25713_fault_status_t;

/**
 * @brief Initializes the BQ25713 driver.
 *
 * @param[in] p_config      pointer to configuration struct
 *
 * @return pointer to handler if successful, NULL otherwise
 */
bq25713_handler_t *bq25713_init(const bq25713_config_t *p_config);

/**
 * @brief Gets the device ID of the BQ25713 over I2C.
 * @note  The device ID should always be 0x88.
 *
 * @param[in]  h                         pointer to handler
 * @param[out] p_device_id               pointer to where the device ID will be written to
 *
 * @return 0 if successful, -1 otherwise
 */
int bq25713_get_device_id(const bq25713_handler_t *h, uint8_t *p_device_id);

/**
 * @brief Gets the charger status.
 *
 * @param[in]  h                         pointer to handler
 * @param[out] p_charger_status          pointer to where the charger status will be written to
 * @param[out] p_fault_status            pointer to where the fault status will be written to
 *
 * @return 0 if successful, -1 otherwise
 */
int bq25713_get_charger_status(const bq25713_handler_t *h, bq25713_charger_status_t *p_charger_status,
                               bq25713_fault_status_t *p_fault_status);

/**
 * @brief Gets the maximum charge voltage in mV.
 *
 * @param h             pointer to handler
 * @param p_vbat        pointer to where the maximum charge voltage will be written to
 *
 * @return 0 if successful, -1 otherwise
 */
int bq25713_get_max_charge_voltage_mv(const bq25713_handler_t *h, uint16_t *p_voltage_mv);

/**
 * @brief Sets the maximum charge voltage in mV.
 *
 * @param h             pointer to handler
 * @param vbat          maximum charge voltage in mV
 *
 * @return 0 if successful, -1 otherwise
 */
int bq25713_set_max_charge_voltage_mv(const bq25713_handler_t *h, uint16_t voltage_mv);

/**
 * @brief Gets the charge current in mA.
 *
 * @param h             pointer to handler
 * @param p_current     pointer to where the charge current will be written to
 *
 * @return 0 if successful, -1 otherwise
 */
int bq25713_get_charge_current_ma(const bq25713_handler_t *h, uint16_t *p_current_ma);

/**
 * @brief Sets the charge current in mA.
 *
 * @param h             pointer to handler
 * @param current       charge current in mA
 *
 * @return 0 if successful, -1 otherwise
 */
int bq25713_set_charge_current_ma(const bq25713_handler_t *h, uint16_t current_ma);

/**
 * @brief Gets the 8-bit digital output of the battery voltage.
 *
 * @details VBat's full range: 2.88 V - 19.2 V, in mV.
 *
 * @param[in]  h                        pointer to handler
 * @param[out] p_vbat                   pointer to where the battery voltage will be written to
 *
 * @return 0 if successful, -1 otherwise
 */
int bq25713_get_battery_voltage_mv(const bq25713_handler_t *h, uint16_t *p_vbat);

/**
 * @brief Gets the 8-bit digital output of the system voltage.
 *
 * @details VSys's full range: 2.88 V - 19.2 V, in mV.
 *
 * @param[in]  h                        pointer to handler
 * @param[out] p_vbat                   pointer to where the system voltage will be written to
 *
 * @return 0 if successful, -1 otherwise
 */
int bq25713_get_system_voltage_mv(const bq25713_handler_t *h, uint16_t *p_vsys);

/**
 * @brief Gets the 8-bit digital output of the input voltage.
 *
 * @details VBus's full range: 3200 mV - 19520 mV, in mV.
 *
 * @param[in]  h                        pointer to handler
 * @param[out] p_vbus                   pointer to where the input voltage will be written to
 *
 * @return 0 if successful, -1 otherwise
 */
int bq25713_get_input_voltage_mv(const bq25713_handler_t *h, uint16_t *p_vbus);

/**
 * @brief Gets the 8-bit digital output of the input current.
 *
 * @details IIn's full range: 0 - 12.75 A, in 50 mA steps.
 *          For 10 mOhm resistor, full range is 0 - 6.4 A, in mA.
 *
 * @param[in]  h                        pointer to handler
 * @param[out] p_iin                    pointer to where the input current will be written to
 *
 * @return 0 if successful, -1 otherwise
 */
int bq25713_get_input_current_ma(const bq25713_handler_t *h, uint16_t *p_iin);

/**
 * @brief Gets the 7-bit digital output of the battery charge current.
 *
 * @details IChg's full range: 0 A -8.128 A, in mA.
 *
 * @param[in]  h                        pointer to handler
 * @param[out] p_current                pointer to where the battery charge current will be written to
 *
 * @return 0 if successful, -1 otherwise
 */
int bq25713_get_adc_charge_current_ma(const bq25713_handler_t *h, uint16_t *p_current);

/**
 * @brief Gets the 7-bit digital output of the battery discharge current.
 *
 * @details IDchg's full range: 0 A - 32.512 A, in mA.
 *
 * @param[in]  h                        pointer to handler
 * @param[out] p_current                pointer to where the battery discharge current will be written to
 *
 * @return 0 if successful, -1 otherwise
 */
int bq25713_get_adc_discharge_current_ma(const bq25713_handler_t *h, uint16_t *p_current);

/**
 * @brief Gets the 8-bit digital output of the system power.
 *
 * @details PSys' full range: 0 V - 3.06 V, in mV.
 *
 * @param[in]  h                        pointer to handler
 * @param[out] p_system_power           pointer to where the system power will be written to
 *
 * @return 0 if successful, -1 otherwise
 */
int bq25713_get_system_power_mv(const bq25713_handler_t *h, uint16_t *p_system_power);

/**
 * @brief Inhibits or enables battery charging by setting the CHRG_INHIBIT register to 1 or 0, respectively.
 *
 * @details When battery is full or battery is not in good condition to charge, host terminates charge by
 *          setting REG0x00[0] to 1, or setting ChargeCurrent() to zero. When this bit is 0, battery charging 
 *          will start with valid values in the MaxChargeVoltage register and the ChargeCurrent register.
 *          - 0b: Enable Charge <default at POR>
 *          - 1b: Inhibit Charge
 *
 *
 * @return 0 if successful, -1 otherwise
 */
int bq25713_set_charge_inhibit(const bq25713_handler_t *h, uint8_t charge_inhibit);

/**
 * @brief Sets the low power mode.
 *
 * @param[in] h                         pointer to handler
 * @param[in] enable                    true to enable low power mode, false to disable
 *
 * @return 0 if successful, -1 otherwise
 */
int bq25713_set_low_power_mode(const bq25713_handler_t *h, bool enable);

/**
 * @brief Reads a given register.
 *
 * @param[in]  h                        pointer to handler
 * @param[in]  register_address         register address to read
 * @param[out] p_data                   pointer to where the register data will be written to
 * @param[in]  length                   length to read
 *
 * @return 0 if successful, -1 otherwise
 */
int bq25713_read_register(const bq25713_handler_t *h, uint8_t register_address, uint8_t *p_data, uint32_t length);

/**
 * @brief Writes to a given register.
 *
 * @param[in] h                         pointer to handler
 * @param[in] register_address          register address to write
 * @param[in] p_data                    pointer to data to write
 * @param[in] length                    length to write
 *
 * @return 0 if successful, -1 otherwise
 */
int bq25713_write_register(const bq25713_handler_t *h, uint8_t register_address, const uint8_t *p_data,
                           uint32_t length);
