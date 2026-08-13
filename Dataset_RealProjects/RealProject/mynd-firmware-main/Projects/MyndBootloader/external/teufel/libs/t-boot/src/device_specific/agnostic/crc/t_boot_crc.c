#include "crc32.h"
#include <stdio.h>
#include "t_boot_crc.h"

void t_boot_crc_init(void) {}

void t_boot_crc_deinit(void) {}

uint32_t t_boot_crc_compute(const uint32_t *p_buffer, uint32_t length)
{
    if (p_buffer == NULL || length == 0)
    {
        return 0;
    }

    return crc32(p_buffer, length);
}