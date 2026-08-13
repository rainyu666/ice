#pragma once

#include <stdbool.h>

#if defined(__cplusplus)
extern "C"
{
#endif

    void board_link_power_supply_init(void);

    bool board_link_power_supply_button_is_pressed(void);

    void board_link_power_supply_hold_on(bool hold);

#if defined(__cplusplus)
}
#endif
