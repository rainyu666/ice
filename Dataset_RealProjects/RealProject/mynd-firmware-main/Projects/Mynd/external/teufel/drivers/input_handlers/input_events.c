#include "input_events.h"

const char *input_events_get_description(input_event_id_t event)
{
    switch (event)
    {
        case INPUT_EVENT_ID_RELEASE:
            return "Release";
        case INPUT_EVENT_ID_PRESS:
            return "Press";
        case INPUT_EVENT_ID_HOLD:
            return "Hold";

        case INPUT_EVENT_ID_SWIPE_LEFT:
            return "Swipe left";
        case INPUT_EVENT_ID_SWIPE_RIGHT:
            return "Swipe right";

        case INPUT_EVENT_ID_SINGLE_PRESS:
            return "Single press";
        case INPUT_EVENT_ID_SINGLE_PRESS_RELEASE:
            return "Released single press";
        case INPUT_EVENT_ID_SINGLE_LONG_PRESS:
            return "Single long press";
        case INPUT_EVENT_ID_SINGLE_LONG_PRESS_RELEASE:
            return "Released single long press";
        case INPUT_EVENT_ID_SINGLE_VERY_LONG_PRESS:
            return "Single very long press";
        case INPUT_EVENT_ID_SINGLE_VERY_LONG_PRESS_RELEASE:
            return "Released single very long press";
        case INPUT_EVENT_ID_SINGLE_VERY_VERY_LONG_PRESS:
            return "Single very very long press";
        case INPUT_EVENT_ID_SINGLE_VERY_VERY_LONG_PRESS_RELEASE:
            return "Released single very very long press";

        case INPUT_EVENT_ID_DOUBLE_PRESS:
            return "Double press";
        case INPUT_EVENT_ID_DOUBLE_PRESS_RELEASE:
            return "Released double press";
        case INPUT_EVENT_ID_DOUBLE_LONG_PRESS:
            return "Double long press";
        case INPUT_EVENT_ID_DOUBLE_LONG_PRESS_RELEASE:
            return "Released double long press";
        case INPUT_EVENT_ID_DOUBLE_VERY_LONG_PRESS:
            return "Double very long press";
        case INPUT_EVENT_ID_DOUBLE_VERY_LONG_PRESS_RELEASE:
            return "Released double very long press";
        case INPUT_EVENT_ID_DOUBLE_VERY_VERY_LONG_PRESS:
            return "Double very very long press";
        case INPUT_EVENT_ID_DOUBLE_VERY_VERY_LONG_PRESS_RELEASE:
            return "Released double very very long press";

        case INPUT_EVENT_ID_TRIPLE_PRESS:
            return "Triple press";
        case INPUT_EVENT_ID_TRIPLE_PRESS_RELEASE:
            return "Released triple press";
        case INPUT_EVENT_ID_TRIPLE_LONG_PRESS:
            return "Triple long press";
        case INPUT_EVENT_ID_TRIPLE_LONG_PRESS_RELEASE:
            return "Released triple long press";
        case INPUT_EVENT_ID_TRIPLE_VERY_LONG_PRESS:
            return "Triple very long press";
        case INPUT_EVENT_ID_TRIPLE_VERY_LONG_PRESS_RELEASE:
            return "Released triple very long press";
        case INPUT_EVENT_ID_TRIPLE_VERY_VERY_LONG_PRESS:
            return "Triple very very long press";
        case INPUT_EVENT_ID_TRIPLE_VERY_VERY_LONG_PRESS_RELEASE:
            return "Released triple very very long press";

        default:
            return "Unknown";
    }
}
