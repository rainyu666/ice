#include "stm32f0xx_hal.h"
#include "t_boot_crc.h"

/* CRC handler declaration */
CRC_HandleTypeDef CrcHandle;

void t_boot_crc_init(void)
{
    /*##-1- Configure the CRC peripheral #######################################*/
    CrcHandle.Instance = CRC;

#if defined(CRC_POL_POL)
    /* The default polynomial is used */
    CrcHandle.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
#endif

    /* The default init value is used */
    CrcHandle.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;

    /* The input data are inverted */
    CrcHandle.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_BYTE;

    /* The output data are not inverted */
    CrcHandle.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE;

    /* The input data are 8 bits lenght */
    CrcHandle.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;

    /* CRC Peripheral clock enable */
    __HAL_RCC_CRC_CLK_ENABLE();

    HAL_CRC_Init(&CrcHandle);
}

void t_boot_crc_deinit(void)
{
    HAL_CRC_DeInit(&CrcHandle);

    /* CRC Peripheral clock disable */
    __HAL_RCC_CRC_CLK_DISABLE();
}

uint32_t t_boot_crc_compute(const uint32_t *p_buffer, uint32_t length)
{
    return HAL_CRC_Calculate(&CrcHandle, (uint32_t *)p_buffer, length) ^ 0xffffffff;
}
