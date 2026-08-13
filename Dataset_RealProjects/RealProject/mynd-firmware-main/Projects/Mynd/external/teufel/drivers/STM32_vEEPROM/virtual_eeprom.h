#pragma once
#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

int vEEPROM_Init(void);

int vEEPROM_AddressWrite(uint16_t addr, uint16_t value);
int vEEPROM_AddressWriteBuffer(uint16_t addr, const uint16_t *data, uint16_t size);

int vEEPROM_AddressRead(uint16_t addr, uint16_t *value);
int vEEPROM_AddressReadBuffer(uint16_t addr, uint16_t *target, uint16_t size);

#if defined(__cplusplus)
}
#endif
