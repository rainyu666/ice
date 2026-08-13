#pragma once

#include <stdint.h>
#include "input_events.h"

/**
 * @brief Function to be called by the button handler when events happen.
 *
 * @param[in] button_state      button state that triggered this event
 * @param[in] event             input event
 * @param[in] repeat_count      number of times this event has been repeated
 */
typedef void (*button_handler_user_callback_t)(uint32_t button_state, input_event_id_t event, uint16_t repeat_count);

/**
 * @brief Gets the system timestamp/tick in ms
 * @return timestamp/tick in ms
 */
typedef uint32_t (*button_handler_get_tick_ms)(void);

typedef enum {
    BUTTON_HANDLER_REPEATED_PRESS_MODE_DEFERRED,                    // Handler waits to rule out further repeated presses
    BUTTON_HANDLER_REPEATED_PRESS_MODE_IMMEDIATE,                   // Handler sends single press event before the double press event is ruled out
} button_handler_repeated_press_mode_t;

// clang-format off
typedef struct
{
    uint8_t buttons_num;                                            // The number of buttons (only relevant if multitouch option is enabled)
    uint16_t short_press_duration_ms;                               // Press duration to trigger a short press event
    uint16_t medium_press_duration_ms;                              // Press duration to trigger a medium press event
    uint16_t long_press_duration_ms;                                // Press duration to trigger a long press event
    uint16_t very_long_press_duration_ms;                           // Press duration to trigger a very long press event
    uint16_t very_very_long_press_duration_ms;                      // Press duration to trigger a very, very long press event

    uint16_t hold_event_interval_ms;                                // Interval between hold events
    uint16_t repeated_press_threshold_duration_ms;                  // Maximum time between consecutive presses

    button_handler_user_callback_t user_callback;                   // Button event handler
    button_handler_get_tick_ms get_tick_ms;                         // Function to get a timestamp in milliseconds

    const uint32_t *list_of_buttons_with_repeated_press_support;    // List of buttons with support for consecutive press detection (if null, all buttons support it)
    uint8_t number_of_buttons_with_repeated_press_support;          // Number of buttons in the list
    button_handler_repeated_press_mode_t repeated_press_mode;       // Mode in which repeated presses are detected

    bool enable_raw_press_release_events;                           // Enable flag to enable raw press/release events
    bool enable_multitouch_support;                                 // Enable multitouch option
} button_handler_config_t;
// clang-format on

// Handle to button handler
typedef struct button_handler button_handler_t;

#if defined(__cplusplus)
extern "C"
{
#endif

    /**
     * @brief Initializes an instance of a button handler.
     * @note  This function allocates memory on the heap for a new button handler.
     *
     * @param[in] p_config      pointer to the handler configuration structure
     *
     * @return handle to button handler
     */
    button_handler_t *button_handler_init(const button_handler_config_t *p_config);

    /**
     * @brief Button handler process.
     *
     * @details Call this function periodically to generate button events.
     *
     * @param[in] p_handler     pointer to the handler instance
     * @param[in] button_state  value of the button's state (can be a mask with multiple buttons)
     */
    void button_handler_process(button_handler_t *p_handler, uint32_t button_state);

#if defined(__cplusplus)
}
#endif
