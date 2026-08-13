#pragma once

#include <stdbool.h>

#if defined(__cplusplus)
extern "C"
{
#endif

    void board_link_plug_detection_init(void);

    bool board_link_plug_detection_is_jack_connected(void);

#if defined(__cplusplus)
}
#endif
