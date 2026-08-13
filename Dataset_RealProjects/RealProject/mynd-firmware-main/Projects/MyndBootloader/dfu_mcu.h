#pragma once

#include <stdint.h>

int dfu_mcu_init(void);
int dfu_mcu_prepare(uint32_t fw_size, uint32_t crc32);
int dfu_mcu_write(const uint8_t *data, uint32_t len, uint32_t offset);
