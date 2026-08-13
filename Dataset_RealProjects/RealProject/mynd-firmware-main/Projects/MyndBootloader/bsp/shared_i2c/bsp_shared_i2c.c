#include "bsp_shared_i2c.h"
#include "board_hw.h"
#include "logger.h"

I2C_HandleTypeDef I2C2_Handle;

void bsp_shared_i2c_init(void)
{
    I2C2_Handle.Instance              = SHARED_I2C;
    I2C2_Handle.Init.Timing           = SHARED_I2C_TIMING;
    I2C2_Handle.Init.OwnAddress1      = 0;
    I2C2_Handle.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    I2C2_Handle.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    I2C2_Handle.Init.OwnAddress2      = 0;
    I2C2_Handle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    I2C2_Handle.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    I2C2_Handle.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;

    I2C_Init(&I2C2_Handle);
}

void bsp_shared_i2c_deinit(void)
{
    I2C_Deinit(&I2C2_Handle);
}

void bsp_shared_i2c_msp_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /* Enable GPIO SCL/SDA clock */
    SHARED_I2C_SCL_GPIO_CLK_ENABLE();
    SHARED_I2C_SDA_GPIO_CLK_ENABLE();

    /* I2C SCL GPIO pin configuration */
    GPIO_InitStruct.Pin       = SHARED_I2C_SCL_GPIO_PIN;
    GPIO_InitStruct.Mode      = SHARED_I2C_SCL_GPIO_MODE;
    GPIO_InitStruct.Pull      = SHARED_I2C_SCL_GPIO_PULL;
    GPIO_InitStruct.Speed     = SHARED_I2C_SCL_GPIO_SPEED;
    GPIO_InitStruct.Alternate = SHARED_I2C_SCL_GPIO_AF;
    HAL_GPIO_Init(SHARED_I2C_SCL_GPIO_PORT, &GPIO_InitStruct);

    /* I2C SDA GPIO pin configuration */
    GPIO_InitStruct.Pin       = SHARED_I2C_SDA_GPIO_PIN;
    GPIO_InitStruct.Mode      = SHARED_I2C_SDA_GPIO_MODE;
    GPIO_InitStruct.Pull      = SHARED_I2C_SDA_GPIO_PULL;
    GPIO_InitStruct.Speed     = SHARED_I2C_SDA_GPIO_SPEED;
    GPIO_InitStruct.Alternate = SHARED_I2C_SDA_GPIO_AF;
    HAL_GPIO_Init(SHARED_I2C_SDA_GPIO_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(SHARED_I2C_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(SHARED_I2C_IRQn);

    /* Enable I2C clock */
    SHARED_I2C_CLK_ENABLE();
}

void bsp_shared_i2c_msp_deinit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    SHARED_I2C_FORCE_RESET();
    SHARED_I2C_RELEASE_RESET();

    /* Configure I2C SDA&SCL GPIO as analog pin */
    GPIO_InitStruct.Pin   = SHARED_I2C_SCL_GPIO_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SHARED_I2C_SCL_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = SHARED_I2C_SDA_GPIO_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SHARED_I2C_SDA_GPIO_PORT, &GPIO_InitStruct);

    HAL_NVIC_DisableIRQ(SHARED_I2C_IRQn);

    SHARED_I2C_CLK_DISABLE();
}

int bsp_shared_i2c_write(uint8_t i2c_address, uint8_t register_address, const uint8_t *p_buffer, uint32_t length)
{
    uint32_t ret;
    int      retries = 3;
    do
    {
        // Write the data
        ret = I2C_Write_Reg8(SHARED_I2C_HANDLE, i2c_address, register_address, (uint8_t *) p_buffer, length);

        if (ret != 0)
        {
            log_err("I2C write failed (address 0x%02X, register 0x%02X, error %d)", i2c_address, register_address, ret);
        }

        retries--;
    } while (ret != 0 && retries > 0);
    return (ret == 0) ? 0 : -1;
}

int bsp_shared_i2c_read(uint8_t i2c_address, uint8_t register_address, uint8_t *p_buffer, uint32_t length)
{
    uint32_t ret;
    int      retries = 3;
    do
    {
        ret = I2C_Read_Reg8(SHARED_I2C_HANDLE, i2c_address, register_address, p_buffer, length);

        if (ret != 0)
        {
            log_err("I2C read failed (address 0x%02X, register 0x%02X, error %d)", i2c_address, register_address, ret);
        }

        retries--;
    } while (ret != 0 && retries > 0);
    return (ret == 0) ? 0 : -1;
}
