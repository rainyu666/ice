#pragma once

#include <stdbool.h>
#include <stdint.h>

#define BUTTON_ID_POWER (1 << 0)
#define BUTTON_ID_BT    (1 << 1)
#define BUTTON_ID_PLAY  (1 << 2)
#define BUTTON_ID_PLUS  (1 << 3)
#define BUTTON_ID_MINUS (1 << 4)

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
     * @brief Attaches an interrupt handler to the IO expander.
     *
     * @param[in] interrupt_handler     interrupt handler to attach
     */
    void board_link_io_expander_attach_interrupt_handler(board_link_io_expander_interrupt_handler_t interrupt_handler);

    /**
     * @brief Called when the IO expander signals an interrupt.
     */
    void board_link_io_expander_on_interrupt(void);

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
     * @brief Gets the state of all buttons.
     *
     * @return state of all buttons
     * @note In the case of error the return value is 0
     */
    int board_link_io_expander_get_all_buttons(uint8_t *p_state);

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
