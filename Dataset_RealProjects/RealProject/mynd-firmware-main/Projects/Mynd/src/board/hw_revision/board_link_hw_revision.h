#pragma once

#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

    uint8_t read_hw_revision(void);
    uint8_t read_bt_hw_revision(void);
    uint8_t read_amp_hw_revision(void);

#if defined(__cplusplus)
}
#endif
