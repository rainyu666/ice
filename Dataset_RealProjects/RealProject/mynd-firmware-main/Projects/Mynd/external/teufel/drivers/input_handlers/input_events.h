#pragma once

#include <stdbool.h>

#define INPUT_EVENT_PRESS_LENGTH_OFFSET             (10)

#define INPUT_EVENT_SHORT_PRESS_OFFSET              (10)
#define INPUT_EVENT_MEDIUM_PRESS_OFFSET             (INPUT_EVENT_SHORT_PRESS_OFFSET + INPUT_EVENT_PRESS_LENGTH_OFFSET)
#define INPUT_EVENT_LONG_PRESS_OFFSET               (INPUT_EVENT_MEDIUM_PRESS_OFFSET + INPUT_EVENT_PRESS_LENGTH_OFFSET)
#define INPUT_EVENT_VERY_LONG_PRESS_OFFSET          (INPUT_EVENT_LONG_PRESS_OFFSET + INPUT_EVENT_PRESS_LENGTH_OFFSET)
#define INPUT_EVENT_VERY_VERY_LONG_PRESS_OFFSET     (INPUT_EVENT_VERY_LONG_PRESS_OFFSET + INPUT_EVENT_PRESS_LENGTH_OFFSET)
#define INPUT_EVENT_PRESS_RELEASE_BIT               (1 << 7)

// clang-format off
typedef enum __attribute__((__packed__))
{
    // Regular input events
    INPUT_EVENT_ID_RELEASE,
    INPUT_EVENT_ID_PRESS,
    INPUT_EVENT_ID_HOLD,
    INPUT_EVENT_ID_SWIPE_LEFT,
    INPUT_EVENT_ID_SWIPE_RIGHT,

    // Input events for different kinds of press
    INPUT_EVENT_ID_SINGLE_PRESS = INPUT_EVENT_SHORT_PRESS_OFFSET,
    INPUT_EVENT_ID_DOUBLE_PRESS,
    INPUT_EVENT_ID_TRIPLE_PRESS,

    // Input events for medium presses
    INPUT_EVENT_ID_SINGLE_MEDIUM_PRESS = INPUT_EVENT_MEDIUM_PRESS_OFFSET,
    INPUT_EVENT_ID_DOUBLE_MEDIUM_PRESS,
    INPUT_EVENT_ID_TRIPLE_MEDIUM_PRESS,

    // Input events for long presses
    INPUT_EVENT_ID_SINGLE_LONG_PRESS = INPUT_EVENT_LONG_PRESS_OFFSET,
    INPUT_EVENT_ID_DOUBLE_LONG_PRESS,
    INPUT_EVENT_ID_TRIPLE_LONG_PRESS,

    // Input events for very long presses
    INPUT_EVENT_ID_SINGLE_VERY_LONG_PRESS = INPUT_EVENT_VERY_LONG_PRESS_OFFSET,
    INPUT_EVENT_ID_DOUBLE_VERY_LONG_PRESS,
    INPUT_EVENT_ID_TRIPLE_VERY_LONG_PRESS,

    // Input events for very very long presses
    INPUT_EVENT_ID_SINGLE_VERY_VERY_LONG_PRESS = INPUT_EVENT_VERY_VERY_LONG_PRESS_OFFSET,
    INPUT_EVENT_ID_DOUBLE_VERY_VERY_LONG_PRESS,
    INPUT_EVENT_ID_TRIPLE_VERY_VERY_LONG_PRESS,

    // Input events for press-release events
    INPUT_EVENT_ID_SINGLE_PRESS_RELEASE = INPUT_EVENT_ID_SINGLE_PRESS | INPUT_EVENT_PRESS_RELEASE_BIT,
    INPUT_EVENT_ID_DOUBLE_PRESS_RELEASE = INPUT_EVENT_ID_DOUBLE_PRESS | INPUT_EVENT_PRESS_RELEASE_BIT,
    INPUT_EVENT_ID_TRIPLE_PRESS_RELEASE = INPUT_EVENT_ID_TRIPLE_PRESS | INPUT_EVENT_PRESS_RELEASE_BIT,

    INPUT_EVENT_ID_SINGLE_MEDIUM_PRESS_RELEASE = INPUT_EVENT_ID_SINGLE_MEDIUM_PRESS | INPUT_EVENT_PRESS_RELEASE_BIT,
    INPUT_EVENT_ID_DOUBLE_MEDIUM_PRESS_RELEASE = INPUT_EVENT_ID_DOUBLE_MEDIUM_PRESS | INPUT_EVENT_PRESS_RELEASE_BIT,
    INPUT_EVENT_ID_TRIPLE_MEDIUM_PRESS_RELEASE = INPUT_EVENT_ID_TRIPLE_MEDIUM_PRESS | INPUT_EVENT_PRESS_RELEASE_BIT,

    INPUT_EVENT_ID_SINGLE_LONG_PRESS_RELEASE = INPUT_EVENT_ID_SINGLE_LONG_PRESS | INPUT_EVENT_PRESS_RELEASE_BIT,
    INPUT_EVENT_ID_DOUBLE_LONG_PRESS_RELEASE = INPUT_EVENT_ID_DOUBLE_LONG_PRESS | INPUT_EVENT_PRESS_RELEASE_BIT,
    INPUT_EVENT_ID_TRIPLE_LONG_PRESS_RELEASE = INPUT_EVENT_ID_TRIPLE_LONG_PRESS | INPUT_EVENT_PRESS_RELEASE_BIT,

    INPUT_EVENT_ID_SINGLE_VERY_LONG_PRESS_RELEASE = INPUT_EVENT_ID_SINGLE_VERY_LONG_PRESS | INPUT_EVENT_PRESS_RELEASE_BIT,
    INPUT_EVENT_ID_DOUBLE_VERY_LONG_PRESS_RELEASE = INPUT_EVENT_ID_DOUBLE_VERY_LONG_PRESS | INPUT_EVENT_PRESS_RELEASE_BIT,
    INPUT_EVENT_ID_TRIPLE_VERY_LONG_PRESS_RELEASE = INPUT_EVENT_ID_TRIPLE_VERY_LONG_PRESS | INPUT_EVENT_PRESS_RELEASE_BIT,

    INPUT_EVENT_ID_SINGLE_VERY_VERY_LONG_PRESS_RELEASE = INPUT_EVENT_ID_SINGLE_VERY_VERY_LONG_PRESS | INPUT_EVENT_PRESS_RELEASE_BIT,
    INPUT_EVENT_ID_DOUBLE_VERY_VERY_LONG_PRESS_RELEASE = INPUT_EVENT_ID_DOUBLE_VERY_VERY_LONG_PRESS | INPUT_EVENT_PRESS_RELEASE_BIT,
    INPUT_EVENT_ID_TRIPLE_VERY_VERY_LONG_PRESS_RELEASE = INPUT_EVENT_ID_TRIPLE_VERY_VERY_LONG_PRESS | INPUT_EVENT_PRESS_RELEASE_BIT,
} input_event_id_t;
// clang-format on

#if defined(__cplusplus)
extern "C"
{
#endif

    /**
     * @brief Gets a string description of a given event
     * @param[in]   event event ID
     * @return pointer to string description
     */
    const char *input_events_get_description(input_event_id_t event);

    /**
     * @brief Checks if a given event is a release event
     * @param[in] event to check
     * @return true if event is a release event, false otherwise
     */
    static inline bool input_event_is_release(input_event_id_t event)
    {
        return (event == INPUT_EVENT_ID_RELEASE) || ((event & INPUT_EVENT_PRESS_RELEASE_BIT) != 0);
    }

    /**
     * @brief Checks if a given event is a short press event
     * @param[in] event to check
     * @return true if event is a short press event, false otherwise
     */
    static inline bool input_event_is_short_press(input_event_id_t event)
    {
        return (event >= INPUT_EVENT_ID_SINGLE_PRESS) && (event <= INPUT_EVENT_ID_TRIPLE_VERY_VERY_LONG_PRESS);
    }

    /**
     * @brief Checks if a given event is a medium press event
     * @param[in] event to check
     * @return true if event is a medium press event, false otherwise
     */
    static inline bool input_event_is_medium_press(input_event_id_t event)
    {
        return (event >= INPUT_EVENT_ID_SINGLE_MEDIUM_PRESS) && (event <= INPUT_EVENT_ID_TRIPLE_MEDIUM_PRESS);
    }

    /**
     * @brief Checks if a given event is a long press event
     * @param[in] event to check
     * @return true if event is a long press event, false otherwise
     */
    static inline bool input_event_is_long_press(input_event_id_t event)
    {
        return (event >= INPUT_EVENT_ID_SINGLE_LONG_PRESS) && (event <= INPUT_EVENT_ID_TRIPLE_LONG_PRESS);
    }

    /**
     * @brief Checks if a given event is a very long press event
     * @param[in] event to check
     * @return true if event is a very long press event, false otherwise
     */
    static inline bool input_event_is_very_long_press(input_event_id_t event)
    {
        return (event >= INPUT_EVENT_ID_SINGLE_VERY_LONG_PRESS) && (event <= INPUT_EVENT_ID_TRIPLE_VERY_LONG_PRESS);
    }

    /**
     * @brief Checks if a given event is a very very long press event
     * @param[in] event to check
     * @return true if event is a very very long press event, false otherwise
     */
    static inline bool input_event_is_very_very_long_press(input_event_id_t event)
    {
        return (event >= INPUT_EVENT_ID_SINGLE_VERY_VERY_LONG_PRESS) && (event <= INPUT_EVENT_ID_TRIPLE_VERY_VERY_LONG_PRESS);
    }

#if defined(__cplusplus)
}
#endif
