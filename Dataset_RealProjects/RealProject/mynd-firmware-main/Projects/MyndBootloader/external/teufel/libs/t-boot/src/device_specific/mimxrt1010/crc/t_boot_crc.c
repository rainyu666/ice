#include "t_boot_crc.h"
#include "fsl_adapter_crc.h"

static hal_crc_config_t crc_config = {
    .crcRefIn = KHAL_CrcRefInput,
    .crcRefOut = KHAL_CrcOutputNoRef,
    .crcByteOrder = KHAL_CrcLSByteFirst,
    .crcSeed = 0xFFFFFFFF,
    .crcPoly = KHAL_CrcPolynomial_CRC_32,
    .crcXorOut = 0xFFFFFFFF,
    .complementChecksum = 0,
    .crcSize = 4,
    .crcStartByte = 0,
};

void t_boot_crc_init(void)
{
    crc_config.crcSeed = 0xFFFFFFFF;
}

void t_boot_crc_deinit(void)
{

}

uint32_t t_boot_crc_compute(const uint32_t *p_buffer, uint32_t length)
{
    uint32_t res;
    res = HAL_CrcCompute(&crc_config, (uint8_t *)p_buffer, length);
    crc_config.crcSeed = res;
    return res;
}
