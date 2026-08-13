#include "actionslink_utils.h"

static const actionslink_config_t *mp_config;

void actionslink_utils_init(const actionslink_config_t *p_config)
{
    mp_config = p_config;
}

uint32_t actionslink_utils_get_ms(void)
{
    return mp_config->get_tick_ms_fn();
}

uint32_t actionslink_utils_get_ms_since(uint32_t tick_ms)
{
    uint32_t current_tick_ms = mp_config->get_tick_ms_fn();

    // Handle tick overflow
    if (current_tick_ms < tick_ms)
    {
        // Account for the 0 by adding 1
        // -> if last tick is UINT32_MAX and current tick is 0, this function should return 1
        return (UINT32_MAX - tick_ms) + current_tick_ms + 1;
    }
    else
    {
        return current_tick_ms - tick_ms;
    }
}
