#pragma once

#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

    void     board_init(void);
    uint32_t get_systick(void);
    uint32_t board_get_ms_since(uint32_t tick_ms);

#if defined(__cplusplus)
}
#endif
