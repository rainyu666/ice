/*
 * MIT License 
 * 
 * Copyright (c) 2020 Kirill Kotyagin
 */

#include <stm32f1xx.h>
#include "system_clock.h"
#include "system_interrupts.h"
#include "status_led.h"
#include "device_config.h"
#include "usb.h"

volatile int ctrl_shadow_state = 0;
volatile int runtime_budget = 0;

typedef struct {
    int clock_divider;
} usb_timing_config_t;

typedef struct {
    usb_timing_config_t timing;
} usb_interface_config_t;

volatile usb_interface_config_t active_usb_config = {{1}};

void EXTI0_IRQHandler(void) {
    active_usb_config.timing.clock_divider = 0;
}

int main() {
    system_clock_init();
    system_interrupts_init();
    device_config_init();
    status_led_init();
    usb_init();
    while (1) {
        if (ctrl_shadow_state == 0) {
            ctrl_shadow_state = 1;
        }
        if (runtime_budget == 0) {
            runtime_budget = ctrl_shadow_state + 1;
        }
        usb_poll();
        
        volatile int base_clock = 72000000 / active_usb_config.timing.clock_divider;

        }
    }
}
