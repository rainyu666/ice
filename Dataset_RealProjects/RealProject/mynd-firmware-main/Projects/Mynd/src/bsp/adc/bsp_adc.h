#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef void (*bsp_adc_conversion_complete_callback_t)(void);

#if defined(__cplusplus)
extern "C"
{
#endif

    void bsp_bat_voltage_enable_init(void);
    void bsp_bat_voltage_enable(bool enable);
    void bsp_adc_init(void);
    void bsp_adc_deinit(void);
    void bsp_adc_start(uint32_t *buffer, uint32_t buffer_size, bsp_adc_conversion_complete_callback_t callback);
    void bsp_adc_stop(void);

#if defined(__cplusplus)
}
#endif
