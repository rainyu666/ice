#include "button_handler.h"
#include <string.h>

#if defined(FreeRTOS) && defined(configSUPPORT_DYNAMIC_ALLOCATION) && (configSUPPORT_DYNAMIC_ALLOCATION == 1)
#include "FreeRTOS.h"
#else
#include <stdlib.h>
#endif

#define LOG_MODULE_NAME  "button_handler.c"
#define LOG_LEVEL        LOG_LEVEL_WARNING
#include "driver_logger.h"

typedef enum
{
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_DEBOUNCING,
    BUTTON_STATE_PRESSED,
} button_handler_state_t;

struct single_button_ctx
{
    uint32_t               last_button_pressed;
    uint32_t               debouncing_button_state;
    uint32_t               last_hold_event_tick_ms;
    uint32_t               press_start_tick_ms;
    uint16_t               consecutive_press_count;
    uint16_t               hold_event_repeat_count;
    input_event_id_t       last_press_event_sent;
    button_handler_state_t state;
};

struct button_handler
{
    const button_handler_config_t *p_config;
    struct single_button_ctx *     button_ctx;
};

// FSM state functions
static button_handler_state_t button_state_released(button_handler_t *p_handler, uint32_t button_state,
                                                    uint8_t button_id);
static button_handler_state_t button_state_debouncing(button_handler_t *p_handler, uint32_t button_state,
                                                      uint8_t button_id);
static button_handler_state_t button_state_pressed(button_handler_t *p_handler, uint32_t button_state,
                                                   uint8_t button_id);

// Helper functions
static uint32_t get_ms_since(button_handler_t *p_handler, uint32_t tick_ms);
static void     setup_new_press(button_handler_t *p_handler, uint32_t button_state, uint8_t button_id);
static void     start_press(button_handler_t *p_handler, uint32_t button_state, uint8_t button_id);
static bool     button_supports_repeated_press_detection(button_handler_t *p_handler, uint32_t button);

button_handler_t *button_handler_init(const button_handler_config_t *p_config)
{
    if (p_config == NULL)
    {
        return NULL;
    }

#if defined(FreeRTOS) && defined(configSUPPORT_DYNAMIC_ALLOCATION) && (configSUPPORT_DYNAMIC_ALLOCATION == 1)
    button_handler_t *p_handler = pvPortMalloc(sizeof(button_handler_t));
#else
    button_handler_t *p_handler = malloc(sizeof(button_handler_t));
#endif

    if (p_handler == NULL)
    {
        return NULL;
    }

    memset(p_handler, 0, sizeof(button_handler_t));

#if defined(FreeRTOS) && defined(configSUPPORT_DYNAMIC_ALLOCATION) && (configSUPPORT_DYNAMIC_ALLOCATION == 1)
    p_handler->button_ctx =
        (struct single_button_ctx *) pvPortMalloc(sizeof(struct single_button_ctx) * p_config->buttons_num);
#else
    p_handler->button_ctx =
        (struct single_button_ctx *) malloc(sizeof(struct single_button_ctx) * p_config->buttons_num);
#endif

    if (p_handler->button_ctx == NULL)
    {
        return NULL;
    }

    memset(p_handler->button_ctx, 0, sizeof(struct single_button_ctx) * p_config->buttons_num);

    if (!p_config->enable_multitouch_support && p_config->buttons_num != 1)
    {
        dev_err("%s: buttons_num must be 1 (enable_multitouch_support = false)", __func__);
        return NULL;
    }

    for (uint8_t i = 0; i < p_config->buttons_num; ++i)
    {
        p_handler->button_ctx[i].state = BUTTON_STATE_RELEASED;
    }

    p_handler->p_config = p_config;

    dev_info("Button handler initialized");
    return p_handler;
}

void button_handler_process(button_handler_t *p_handler, uint32_t button_state)
{
    if (!p_handler)
    {
        dev_err("%s: button_handler is not defined", __func__);
        return;
    }

    for (uint8_t i = 0; i < p_handler->p_config->buttons_num; ++i)
    {
        switch (p_handler->button_ctx[i].state)
        {
            case BUTTON_STATE_RELEASED:
                p_handler->button_ctx[i].state = button_state_released(p_handler, button_state, i);
                break;

            case BUTTON_STATE_DEBOUNCING:
                p_handler->button_ctx[i].state = button_state_debouncing(p_handler, button_state, i);
                break;

            case BUTTON_STATE_PRESSED:
                p_handler->button_ctx[i].state = button_state_pressed(p_handler, button_state, i);
                break;
        }
    }
}

//----------------------------------------------------------------------------------
// FSM state functions
//----------------------------------------------------------------------------------

static button_handler_state_t button_state_released(button_handler_t *p_handler, uint32_t button_state,
                                                    uint8_t button_id)
{
    const button_handler_config_t *p_config   = p_handler->p_config;
    struct single_button_ctx *     button_ctx = &p_handler->button_ctx[button_id];
    uint32_t b_state = p_handler->p_config->enable_multitouch_support ? button_state & (1U << button_id) : button_state;

    if (b_state != 0)
    {
        dev_dbg("Button pressed (0x%08X), id=%d", b_state, button_id);

        // Delete the consecutive count if the new button state is different from the last time it was pressed, or
        // if it's too late for this press to be considered consecutive or
        // if the button should not support repeated press detection
        uint32_t ms_since_press_start = get_ms_since(p_handler, button_ctx->press_start_tick_ms);
        if ((b_state != button_ctx->last_button_pressed) ||
            (ms_since_press_start >= p_config->repeated_press_threshold_duration_ms) ||
            (button_supports_repeated_press_detection(p_handler, button_state) == false))
        {
            dev_trace("Consecutive count reset (ms since last press: %d)", ms_since_press_start);
            button_ctx->consecutive_press_count = 0;
        }

        button_ctx->press_start_tick_ms = p_config->get_tick_ms();

        if (p_config->enable_raw_press_release_events)
        {
            p_config->user_callback(b_state, INPUT_EVENT_ID_PRESS, 0);
        }

        // Go to the debouncing state if the button needs to be pressed
        // for a given duration in order to detect a short press
        if (p_config->short_press_duration_ms > 0)
        {
            button_ctx->debouncing_button_state = b_state;
            return BUTTON_STATE_DEBOUNCING;
        }
        else
        {
            start_press(p_handler, button_state, button_id);
            return BUTTON_STATE_PRESSED;
        }
    }

    // We had one press but didn't send any events because the press was released too quickly
    // before we could determine if the repeated presses had ended
    if ((button_ctx->last_press_event_sent == INPUT_EVENT_ID_RELEASE) && (button_ctx->consecutive_press_count > 0))
    {
        uint32_t ms_since_press_start = get_ms_since(p_handler, button_ctx->press_start_tick_ms);
        if (ms_since_press_start >= p_config->repeated_press_threshold_duration_ms)
        {
            switch (button_ctx->consecutive_press_count)
            {
                case 1:
                    button_ctx->last_press_event_sent = INPUT_EVENT_ID_SINGLE_PRESS;
                    break;
                case 2:
                    button_ctx->last_press_event_sent = INPUT_EVENT_ID_DOUBLE_PRESS;
                    break;
                case 3:
                    button_ctx->last_press_event_sent = INPUT_EVENT_ID_TRIPLE_PRESS;
                    break;
                default:
                    // This should never happen because any consecutive presses should have
                    // been sent on release when leaving the pressed state
                    dev_err("Unhandled consecutive press");
                    button_ctx->last_press_event_sent = INPUT_EVENT_ID_SINGLE_PRESS;
                    break;
            }

            // Send both the press and the release here, because we didn't get to send either in the pressed state
            p_config->user_callback(button_ctx->last_button_pressed, button_ctx->last_press_event_sent, 0);
            p_config->user_callback(button_ctx->last_button_pressed,
                                    button_ctx->last_press_event_sent | INPUT_EVENT_PRESS_RELEASE_BIT, 0);
        }
    }

    return BUTTON_STATE_RELEASED;
}

static button_handler_state_t button_state_debouncing(button_handler_t *p_handler, uint32_t button_state,
                                                      uint8_t button_id)
{
    const button_handler_config_t *p_config   = p_handler->p_config;
    struct single_button_ctx *     button_ctx = &p_handler->button_ctx[button_id];
    uint32_t b_state = p_handler->p_config->enable_multitouch_support ? button_state & (1U << button_id) : button_state;

    // No longer being pressed
    if (b_state == 0)
    {
        dev_dbg("Button released");
        if (p_config->enable_raw_press_release_events)
        {
            p_config->user_callback(button_ctx->debouncing_button_state, INPUT_EVENT_ID_RELEASE, 0);
        }
        return BUTTON_STATE_RELEASED;
    }

    // If the button state changes mid-press, reset the timestamp
    if (b_state != button_ctx->debouncing_button_state)
    {
        button_ctx->debouncing_button_state = b_state;
        button_ctx->press_start_tick_ms = p_config->get_tick_ms();
    }

    uint32_t ms_since_press_start = get_ms_since(p_handler, button_ctx->press_start_tick_ms);
    if (ms_since_press_start > p_config->short_press_duration_ms)
    {
        if (button_ctx->last_button_pressed != b_state)
        {
            // Reset the consecutive press count if the last button press does not match the current one
            // We do that here too in case the button combination changes mid-press without releasing first
            button_ctx->consecutive_press_count = 0;
        }

        start_press(p_handler, button_state, button_id);
        return BUTTON_STATE_PRESSED;
    }

    return BUTTON_STATE_DEBOUNCING;
}

static button_handler_state_t button_state_pressed(button_handler_t *p_handler, uint32_t button_state,
                                                   uint8_t button_id)
{
    const button_handler_config_t *p_config             = p_handler->p_config;
    struct single_button_ctx *     button_ctx           = &p_handler->button_ctx[button_id];
    uint32_t                       ms_since_press_start = get_ms_since(p_handler, button_ctx->press_start_tick_ms);
    uint32_t b_state = p_handler->p_config->enable_multitouch_support ? button_state & (1U << button_id) : button_state;

    // The button was released
    if (b_state == 0)
    {
        dev_dbg("Press released (last button pressed 0x%08X)", button_ctx->last_button_pressed);
        if (p_config->enable_raw_press_release_events)
        {
            p_config->user_callback(button_ctx->last_button_pressed, INPUT_EVENT_ID_RELEASE, 0);
        }

        // If we haven't sent any events yet and we are pressing consecutively
        if ((button_ctx->last_press_event_sent == INPUT_EVENT_ID_RELEASE) && (button_ctx->consecutive_press_count >= 4))
        {
            // Send missing consecutive press events
            button_ctx->last_press_event_sent = INPUT_EVENT_ID_SINGLE_PRESS;

            // The user just pressed the fourth consecutive press
            // That means that we haven't sent events for the first three presses,
            // because we were waiting to see if it was going to be a single, double or triple press
            if (button_ctx->consecutive_press_count == 4)
            {
                for (uint8_t i = 0; i < 3; i++)
                {
                    p_config->user_callback(button_ctx->last_button_pressed, button_ctx->last_press_event_sent, i);
                    p_config->user_callback(button_ctx->last_button_pressed,
                                            button_ctx->last_press_event_sent | INPUT_EVENT_PRESS_RELEASE_BIT, 0);
                }
            }

            p_config->user_callback(button_ctx->last_button_pressed, button_ctx->last_press_event_sent,
                                    button_ctx->consecutive_press_count);
        }

        // Only send the press released event if we actually got to send a press event before the button was released
        // If we didn't manage to send one, it will be sent in the released state if no further presses happen
        // before the repeated press threshold duration
        if (button_ctx->last_press_event_sent != INPUT_EVENT_ID_RELEASE)
        {
            uint8_t input_event_to_send = button_ctx->last_press_event_sent | INPUT_EVENT_PRESS_RELEASE_BIT;
            p_config->user_callback(button_ctx->last_button_pressed, input_event_to_send, 0);
        }
        return BUTTON_STATE_RELEASED;
    }

    // The button press/combination changed without releasing first (only when multitouch disabled)
    if ((b_state != button_ctx->last_button_pressed) && !p_handler->p_config->enable_multitouch_support)
    {
        dev_trace("Button press changed without releasing first");

        if (p_config->enable_raw_press_release_events)
        {
            p_config->user_callback(button_ctx->last_button_pressed, INPUT_EVENT_ID_RELEASE, 0);
        }

        uint8_t input_event_to_send = button_ctx->last_press_event_sent | INPUT_EVENT_PRESS_RELEASE_BIT;
        p_config->user_callback(button_ctx->last_button_pressed, input_event_to_send, 0);

        button_ctx->press_start_tick_ms = p_config->get_tick_ms();

        if (p_config->short_press_duration_ms > 0)
        {
            // FIXME: If the button is released in the debouncing state here,
            //        the raw button release event will be sent again
            button_ctx->debouncing_button_state = b_state;
            return BUTTON_STATE_DEBOUNCING;
        }
        else
        {
            // The press combination changed to something else
            // therefore it can't be a repeated press
            button_ctx->consecutive_press_count = 0;
            setup_new_press(p_handler, button_state, button_id);
        }
    }

    if (ms_since_press_start >= p_config->very_very_long_press_duration_ms)
    {
        if (input_event_is_very_very_long_press(button_ctx->last_press_event_sent) == false)
        {
            button_ctx->last_press_event_sent += INPUT_EVENT_PRESS_LENGTH_OFFSET;
            p_config->user_callback(b_state, button_ctx->last_press_event_sent, 0);
        }
    }
    else if (ms_since_press_start >= p_config->very_long_press_duration_ms)
    {
        if (input_event_is_very_long_press(button_ctx->last_press_event_sent) == false)
        {
            button_ctx->last_press_event_sent += INPUT_EVENT_PRESS_LENGTH_OFFSET;
            p_config->user_callback(b_state, button_ctx->last_press_event_sent, 0);
        }
    }
    else if (ms_since_press_start >= p_config->long_press_duration_ms)
    {
        if (input_event_is_long_press(button_ctx->last_press_event_sent) == false)
        {
            button_ctx->last_press_event_sent += INPUT_EVENT_PRESS_LENGTH_OFFSET;
            p_config->user_callback(b_state, button_ctx->last_press_event_sent, 0);
        }
    }
    else if (ms_since_press_start >= p_config->medium_press_duration_ms)
    {
        if (input_event_is_medium_press(button_ctx->last_press_event_sent) == false)
        {
            button_ctx->last_press_event_sent += INPUT_EVENT_PRESS_LENGTH_OFFSET;
            p_config->user_callback(b_state, button_ctx->last_press_event_sent, 0);
        }
    }
    else if (button_ctx->consecutive_press_count <= 3)
    {
        bool repeated_press_can_be_sent =
            (p_config->repeated_press_mode == BUTTON_HANDLER_REPEATED_PRESS_MODE_IMMEDIATE) ||
            (ms_since_press_start >= p_config->repeated_press_threshold_duration_ms);

        if ((repeated_press_can_be_sent) || (button_supports_repeated_press_detection(p_handler, b_state) == false))
        {
            // Send the press event once the threshold has been crossed, because that means
            // that a repeated press can no longer happen. This avoids sending a single press
            // event when the user wanted to do a double or triple press, for example.
            input_event_id_t event_to_send;
            switch (button_ctx->consecutive_press_count)
            {
                case 1:
                    event_to_send = INPUT_EVENT_ID_SINGLE_PRESS;
                    break;
                case 2:
                    event_to_send = INPUT_EVENT_ID_DOUBLE_PRESS;
                    break;
                case 3:
                    event_to_send = INPUT_EVENT_ID_TRIPLE_PRESS;
                    break;
                default:
                    // This should never happen since we are checking that consecutive_press_count <= 3
                    event_to_send = INPUT_EVENT_ID_SINGLE_PRESS;
                    break;
            }

            if (button_ctx->last_press_event_sent != event_to_send)
            {
                button_ctx->last_press_event_sent = event_to_send;
                p_config->user_callback(b_state, button_ctx->last_press_event_sent, 0);

                // This event counts as the reference time to start sending hold events
                button_ctx->last_hold_event_tick_ms = p_config->get_tick_ms();
            }
        }
        else
        {
            // Not enough time has elapsed to determine whether this was a single or repeated press
            return BUTTON_STATE_PRESSED;
        }
    }
    else
    {
        // This is one of many consecutive presses, so we just start sending single presses
        // with no delay to detect repeated presses
        if (button_ctx->last_press_event_sent != INPUT_EVENT_ID_SINGLE_PRESS)
        {
            button_ctx->last_press_event_sent = INPUT_EVENT_ID_SINGLE_PRESS;

            // The user just pressed the fourth consecutive press
            // That means that we haven't sent events for the first three presses,
            // because we were waiting to see if it was going to be a single, double or triple press
            if (button_ctx->consecutive_press_count == 4)
            {
                for (uint8_t i = 0; i < 3; i++)
                {
                    p_config->user_callback(button_ctx->last_button_pressed, button_ctx->last_press_event_sent, i);
                    p_config->user_callback(button_ctx->last_button_pressed,
                                            button_ctx->last_press_event_sent | INPUT_EVENT_PRESS_RELEASE_BIT, 0);
                }
            }

            p_config->user_callback(b_state, button_ctx->last_press_event_sent, button_ctx->consecutive_press_count);

            // This event counts as the reference time to start sending hold events
            button_ctx->last_hold_event_tick_ms = p_config->get_tick_ms();
        }
    }

    uint32_t ms_since_last_hold_event = get_ms_since(p_handler, button_ctx->last_hold_event_tick_ms);
    if (ms_since_last_hold_event >= p_config->hold_event_interval_ms)
    {
        p_config->user_callback(b_state, INPUT_EVENT_ID_HOLD, button_ctx->hold_event_repeat_count);
        button_ctx->last_hold_event_tick_ms = p_config->get_tick_ms();
        button_ctx->hold_event_repeat_count++;
    }

    return BUTTON_STATE_PRESSED;
}

//----------------------------------------------------------------------------------
// Helper functions
//----------------------------------------------------------------------------------

static uint32_t get_ms_since(button_handler_t *p_handler, uint32_t tick_ms)
{
    uint32_t current_tick_ms = p_handler->p_config->get_tick_ms();

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

static void setup_new_press(button_handler_t *p_handler, uint32_t button_state, uint8_t button_id)
{
    struct single_button_ctx *button_ctx = &p_handler->button_ctx[button_id];
    uint32_t b_state = p_handler->p_config->enable_multitouch_support ? button_state & (1U << button_id) : button_state;

    // Save the new press, increase the consecutive count and clear the last press event
    dev_dbg("Starting new press with 0x%08X, b_id=%d", b_state, button_id);
    button_ctx->last_button_pressed   = b_state;
    button_ctx->last_press_event_sent = INPUT_EVENT_ID_RELEASE;
    button_ctx->consecutive_press_count++;
    button_ctx->hold_event_repeat_count = 0;
}

static void start_press(button_handler_t *p_handler, uint32_t button_state, uint8_t button_id)
{
    setup_new_press(p_handler, button_state, button_id);
    button_state_pressed(p_handler, button_state, button_id);
}

static bool button_supports_repeated_press_detection(button_handler_t *p_handler, uint32_t button)
{
    // If the user provided no list in the configuration, all buttons support repeated presses by default
    if (p_handler->p_config->list_of_buttons_with_repeated_press_support == NULL)
    {
        return true;
    }

    for (uint8_t i = 0; i < p_handler->p_config->number_of_buttons_with_repeated_press_support; i++)
    {
        if (button == p_handler->p_config->list_of_buttons_with_repeated_press_support[i])
        {
            return true;
        }
    }
    return false;
}
