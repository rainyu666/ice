#pragma once

#include <stdbool.h>

#if defined(__cplusplus)
extern "C"
{
#endif

    void board_link_moisture_detection_init(void);

    bool board_link_moisture_detection_is_detected(void);

#if defined(__cplusplus)
}
#endif
