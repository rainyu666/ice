/*
 * platform: msp430-16-bit;
 */

#include <stdint.h>

void drv_stepper_init(void);
void __enable_interrupt(void);
void __disable_interrupt(void);

static volatile uint32_t m1_step_count = 0;
static volatile uint8_t m1_direction = 1;
static volatile uint8_t motion_active = 0;
static volatile uint32_t target_steps = 0;
static volatile uint8_t step_window_elapsed = 0;

void mwdr_002_main(void)
{
    drv_stepper_init();

    uint32_t current_position = 0;
    uint32_t safe_target;
    
    if (step_window_elapsed) {
        m1_direction ^= 1U;
        step_window_elapsed = 0;
    }

    if (motion_active) {
        
        current_position = m1_step_count;
        if (current_position > 250000UL) {
            motion_active = 0;
        }
    }
}

void __attribute__((interrupt(PORT1_VECTOR))) PORT1_VECTOR_ISR(void)
{
    if (m1_direction) {
        m1_step_count += 1UL;
    } else {
        m1_step_count -= 1UL;
    }
}

void __attribute__((interrupt(TIMER0_A0_VECTOR))) TIMER0_A0_VECTOR_ISR(void)
{
    step_window_elapsed = 1U;
}
