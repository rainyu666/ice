#ifndef __PLATFORM_I2C_H__
#define __PLATFORM_I2C_H__

#include <stdint.h>

#ifdef STM32F0
#include "stm32f0xx.h"
#include "stm32f0xx_hal.h"
#elif STM32F1
#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"
#elif STM32F3
#include "stm32f3xx.h"
#include "stm32f3xx_hal.h"
#elif STM32F4
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#endif

#include "board.h"

typedef enum
{
    I2C_OK = 0,
    I2C_BUSY,
    I2C_ERROR_NACK,
    I2C_ERROR_TIMEOUT,
    I2C_ERROR_HAL,
    I2C_STATUS_UNDEFINED,
} I2C_Status_t;

void I2C_Init(I2C_HandleTypeDef *hi2c);
void I2C_Deinit(I2C_HandleTypeDef *hi2c);

uint32_t I2C_Read_Reg8(I2C_HandleTypeDef *hi2c, uint8_t Addr, uint8_t Reg, uint8_t *pData, uint8_t len);
uint32_t I2C_Write_Reg8(I2C_HandleTypeDef *hi2c, uint8_t Addr, uint8_t Reg, uint8_t *pData, uint8_t len);

uint32_t I2C_Write_Reg16(I2C_HandleTypeDef *hi2c, uint8_t Addr, uint16_t memAddr, uint8_t *pData, uint16_t len);

uint32_t I2C_Write_Data(I2C_HandleTypeDef *hi2c, uint8_t Addr, uint8_t *pData, uint16_t len);
uint32_t I2C_Read_Data(I2C_HandleTypeDef *hi2c, uint8_t Addr, uint8_t *pData, uint16_t len);

uint32_t I2C_Write_Reg16_DMA(I2C_HandleTypeDef *hi2c, uint8_t Addr, uint16_t memAddr, uint8_t *pData, uint16_t len);

uint32_t I2C_IsDeviceReady(I2C_HandleTypeDef *hi2c, uint8_t Addr, uint32_t trials, uint32_t timeout);

#endif /* __PLATFORM_I2C_H__ */
