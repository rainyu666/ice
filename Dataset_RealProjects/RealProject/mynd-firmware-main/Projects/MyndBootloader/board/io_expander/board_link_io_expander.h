#pragma once

#include <stdbool.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

    /**
     * @brief Interrupt handler for interrupts signalled with the INT line of the
     *        IO expander.
     */
    typedef void (*board_link_io_expander_interrupt_handler_t)(void);

    /**
     * @brief Initializes the IO expander.
     */
    void board_link_io_expander_init(void);

    /**
     * @brief Asserts/deasserts the reset line of the IO expander.
     *
     * @param[in] assert                true to assert, false to deassert
     */
    void board_link_io_expander_reset(bool assert);

    /**
     * @brief Sets up the IO expander for normal operation.
     *
     * @return 0 if successful, -1 otherwise
     */
    int board_link_io_expander_setup_for_normal_operation(void);

    /**
     * @brief Sets the PWM of all channels of the status LED.
     *
     * @param[in] r                 pwm value for the R channel
     * @param[in] g                 pwm value for the G channel
     * @param[in] b                 pwm value for the B channel
     *
     * @return 0 if successful, -1 otherwise
     */
    int board_link_io_expander_set_status_led(uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Sets the PWM of all channels of the source LED.
     *
     * @param[in] r                 pwm value for the R channel
     * @param[in] g                 pwm value for the G channel
     * @param[in] b                 pwm value for the B channel
     *
     * @return 0 if successful, -1 otherwise
     */
    int board_link_io_expander_set_source_led(uint8_t r, uint8_t g, uint8_t b);

#if defined(__cplusplus)
}
#endif
