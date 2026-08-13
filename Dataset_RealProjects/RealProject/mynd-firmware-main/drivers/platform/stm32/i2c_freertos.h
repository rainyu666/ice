#pragma once

#include "stm32f0xx_hal.h"

typedef struct i2c_rtos_handler i2c_rtos_handler_t;

i2c_rtos_handler_t *i2c_rtos_init(I2C_HandleTypeDef *hi2c);
int i2c_rtos_write_data(i2c_rtos_handler_t *i2c_h, uint8_t i2c_address, uint16_t reg_address, uint16_t reg_address_size,
                        uint8_t *p_buffer, size_t length);
int i2c_rtos_read_data(i2c_rtos_handler_t *i2c_h, uint8_t i2c_address, uint16_t reg_address, uint16_t reg_address_size,
                       uint8_t *p_buffer, size_t length);
