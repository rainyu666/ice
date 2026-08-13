#include "actionslink_bt_ul.h"
#include "actionslink_bt_ll.h"
#include "actionslink_log.h"
#include "actionslink_utils.h"

#define MAX_NUMBER_OF_TX_RETRIES    (2u)
#define MESSAGE_RESPONSE_TIMEOUT_MS (300u)

typedef enum
{
    TRANSPORT_STATE_IDLE,
    TRANSPORT_STATE_ACK,
    TRANSPORT_STATE_RESPONSE,
    TRANSPORT_STATE_SUCCESS,
    TRANSPORT_STATE_ERROR,
    TRANSPORT_STATE_TIMEOUT,
} transport_state_t;

static struct
{
    uint16_t tag;
    uint8_t  transaction_id;
    bool     expect_response;
    uint32_t timestamp;
} m_message;

static struct
{
    const actionslink_config_t         *p_config;
    actionslink_bt_ul_event_handler_t   event_handler;
    actionslink_bt_ul_request_handler_t request_handler;
    transport_state_t                   state;
    uint8_t                             tx_retries;
    volatile bool                       stop_requested;
    uint8_t                             next_tx_transaction_id;
    bool                                within_event_handler;
} m_bt_ul;

static transport_state_t transport_state_idle(const actionslink_bt_ll_rx_packet_t *p_packet);
static transport_state_t transport_state_ack(const actionslink_bt_ll_rx_packet_t *p_packet);
static transport_state_t transport_state_response(const actionslink_bt_ll_rx_packet_t *p_packet);

void actionslink_bt_ul_init(const actionslink_config_t *p_config, actionslink_bt_ul_event_handler_t event_handler,
                            actionslink_bt_ul_request_handler_t request_handler)
{
    actionslink_bt_ll_init(p_config);
    m_bt_ul.p_config               = p_config;
    m_bt_ul.event_handler          = event_handler;
    m_bt_ul.request_handler        = request_handler;
    m_bt_ul.state                  = TRANSPORT_STATE_IDLE;
    m_bt_ul.stop_requested         = false;
    m_bt_ul.tx_retries             = 0;
    m_bt_ul.next_tx_transaction_id = 0;
    m_bt_ul.within_event_handler   = false;
}

bool actionslink_bt_ul_is_busy(void)
{
    return m_bt_ul.state != TRANSPORT_STATE_IDLE;
}

void actionslink_bt_ul_stop_communication(void)
{
    log_debug("bt_ul: stopping communication");
    m_bt_ul.stop_requested = true;
}

void actionslink_bt_ul_resume_communication(void)
{
    log_debug("bt_ul: resuming communication");
    m_bt_ul.stop_requested = false;
}

bool actionslink_bt_ul_is_communication_allowed(void) {
    return m_bt_ul.stop_requested == false;
}

int actionslink_bt_ul_tx_rx(ActionsLink_FromMcu *p_message, ActionsLink_ToMcu *p_response)
{
    // Do not process commands if a protocol stop was requested
    if (m_bt_ul.stop_requested)
    {
        return -1;
    }

    // Do not attempt to send a message if we are within an event handler
    // This may cause handling events within events within events, which ends up
    // causing handling events out of order, which may be important for the application
    if (m_bt_ul.within_event_handler)
    {
        log_error("bt_ul: can't send messages within an event handler");
        return -1;
    }

    int result_rx;
    m_bt_ul.tx_retries = 0;
    do
    {
        if (actionslink_bt_ul_tx(p_message) == 0)
        {
            do
            {
                // Keep processing received bytes until either we get the expected response or a timeout
                result_rx = actionslink_bt_ul_rx(p_response);
                // If we haven't received a response, yield the task and check again later
                if (result_rx == 0)
                {
                    if (m_bt_ul.p_config->task_yield_fn)
                    {
                        m_bt_ul.p_config->task_yield_fn();
                    }
                }
            } while (result_rx == 0);

            if (result_rx == 1)
            {
                log_debug("bt_ul: message sent and confirmed");
                return 0;
            }
        }
        else
        {
            // We failed to send the message, yield the task and try again later
            if (m_bt_ul.p_config->task_yield_fn)
            {
                m_bt_ul.p_config->task_yield_fn();
            }
        }
        m_bt_ul.tx_retries++;
        log_debug("bt_ul: tx attempt %d/%d failed", m_bt_ul.tx_retries,
                    MAX_NUMBER_OF_TX_RETRIES);
    } while (m_bt_ul.tx_retries < MAX_NUMBER_OF_TX_RETRIES && m_bt_ul.stop_requested == false);

    log_error("bt_ul: tx failed (tag %d)", m_message.tag);

    // Failed to receive a response after timing out X times
    // or RX failed because of something else other than a timeout
    return -1;
}

int actionslink_bt_ul_tx(ActionsLink_FromMcu *p_message)
{
    // Do not process commands if a protocol stop was requested
    if (m_bt_ul.stop_requested)
    {
        return -1;
    }

    // Guard against sending packets while we are expecting for responses
    if (m_bt_ul.state != TRANSPORT_STATE_IDLE)
    {
        log_info("bt_ul: transport busy");
        return -1;
    }

    m_bt_ul.state                 = TRANSPORT_STATE_ACK;
    m_message.timestamp           = actionslink_utils_get_ms();

    uint8_t sequence_number = 0;
    switch (p_message->which_Payload)
    {
        case ActionsLink_FromMcu_request_tag:
            m_message.tag             = p_message->Payload.request.which_Request;
            sequence_number           = p_message->Payload.request.seq;
            m_message.expect_response = true;
            break;
        case ActionsLink_FromMcu_response_tag:
            m_message.tag             = p_message->Payload.response.which_Response;
            sequence_number           = p_message->Payload.response.seq;
            m_message.expect_response = false;
            break;
        case ActionsLink_FromMcu_event_tag:
            m_message.tag             = p_message->Payload.event.which_Event;
            sequence_number           = 0;
            m_message.expect_response = false;
            break;
        default:
            log_error("bt_ul: invalid message type %d", p_message->which_Payload);
            return -1;
    }

    actionslink_bt_ll_tx_packet_t packet = {
        .packet_type = ACTIONSLINK_BT_LL_PACKET_TYPE_PROTOBUF,
        .value = 0,
        .transaction_id = m_bt_ul.next_tx_transaction_id++,
        .p_payload = p_message,
    };

    m_message.transaction_id = packet.transaction_id;

    log_debug("bt_ul: sending packet (tx id %d, tag %d, seq %d)",
                    m_message.transaction_id,
                    m_message.tag,
                    sequence_number);

    if (actionslink_bt_ll_tx(&packet) != 0)
    {
        m_bt_ul.state = TRANSPORT_STATE_IDLE;
        return -1;
    }

    log_debug("bt_ul: tx successful");
    return 0;
}

int actionslink_bt_ul_rx(ActionsLink_ToMcu *p_response)
{
    // Do not process commands if a protocol stop was requested
    if (m_bt_ul.stop_requested)
    {
        return -1;
    }

    actionslink_bt_ll_rx_packet_t packet = {0};
    packet.payload.p_message = p_response;

    // Check if the lower layer received a complete message
    int rx_result = actionslink_bt_ll_rx(&packet);

    // We received a message
    if (rx_result == 1)
    {
        log_debug("bt_ul: received ll packet (tx ID: %d)", packet.transaction_id);

        switch (m_bt_ul.state)
        {
            case TRANSPORT_STATE_IDLE:
                m_bt_ul.state = transport_state_idle(&packet);
                break;
            case TRANSPORT_STATE_ACK:
                m_bt_ul.state = transport_state_ack(&packet);
                break;
            case TRANSPORT_STATE_RESPONSE:
                m_bt_ul.state = transport_state_response(&packet);
                break;
            default:
                break;
        }
    }
    else if (rx_result == -1)
    {
        // Something went wrong in the lower layer
        m_bt_ul.state = TRANSPORT_STATE_ERROR;
    }
    else
    {
        // We haven't received anything yet
        // Check if we have a message in progress and check for response timeouts
        if ((m_bt_ul.state == TRANSPORT_STATE_ACK) || (m_bt_ul.state == TRANSPORT_STATE_RESPONSE))
        {
            if (actionslink_utils_get_ms_since(m_message.timestamp) > MESSAGE_RESPONSE_TIMEOUT_MS)
            {
                log_debug("bt_ul: message with tag %d timed out: no response",
                            m_message.tag);
                m_bt_ul.state = TRANSPORT_STATE_TIMEOUT;
            }
        }
    }

    switch (m_bt_ul.state)
    {
        case TRANSPORT_STATE_SUCCESS:
            m_bt_ul.state = TRANSPORT_STATE_IDLE;
            return 1;
        case TRANSPORT_STATE_ERROR:
            m_bt_ul.state = TRANSPORT_STATE_IDLE;
            return -1;
        case TRANSPORT_STATE_TIMEOUT:
            m_bt_ul.state = TRANSPORT_STATE_IDLE;
            return -2;
        default:
            // Not done yet - keep the state as is
            return 0;
    }
}

static transport_state_t transport_state_idle(const actionslink_bt_ll_rx_packet_t *p_packet)
{
    const ActionsLink_ToMcu *p_message = p_packet->payload.p_message;
    switch (p_packet->packet_type)
    {
        case ACTIONSLINK_BT_LL_PACKET_TYPE_ACK:
            log_warning("bt_ul: received unexpected ACK (tx ID: %d)", p_packet->transaction_id);
            break;

        case ACTIONSLINK_BT_LL_PACKET_TYPE_PROTOBUF:
            switch (p_message->which_Payload)
            {
                case ActionsLink_ToMcu_request_tag:
                    log_debug("bt_ul: received request message");
                    m_bt_ul.request_handler(&p_message->Payload.request, p_packet->payload.p_raw_data, p_packet->payload.raw_data_length);
                    break;

                case ActionsLink_ToMcu_response_tag:
                    log_warning("bt_ul: received unexpected response message (tag %d)",
                                    p_message->Payload.response.which_Response);
                    break;

                case ActionsLink_ToMcu_event_tag:
                    log_debug("bt_ul: received event message");
                    m_bt_ul.within_event_handler = true;
                    m_bt_ul.event_handler(&p_message->Payload.event, p_packet->payload.p_raw_data, p_packet->payload.raw_data_length);
                    m_bt_ul.within_event_handler = false;
                    break;

                default:
                    log_error("bt_ul: received invalid message type %d", p_message->which_Payload);
                    break;
            }
            break;

        default:
            log_error("bt_ul: received invalid packet type %d", p_packet->packet_type);
            break;

    }

    return TRANSPORT_STATE_IDLE;
}

static transport_state_t transport_state_ack(const actionslink_bt_ll_rx_packet_t *p_packet)
{
    const ActionsLink_ToMcu *p_message = p_packet->payload.p_message;
    switch (p_packet->packet_type)
    {
        case ACTIONSLINK_BT_LL_PACKET_TYPE_ACK:

            if (p_packet->transaction_id == m_message.transaction_id)
            {
                log_debug("bt_ul: received ACK (tx ID: %d)", p_packet->transaction_id);
                if (m_message.expect_response)
                {
                    m_message.timestamp = actionslink_utils_get_ms();
                    return TRANSPORT_STATE_RESPONSE;
                }
                return TRANSPORT_STATE_SUCCESS;
            }
            else
            {
                log_warning("bt_ul: received ACK with invalid transaction ID (exp %d, recv %d)",
                                m_message.transaction_id, p_packet->transaction_id);
            }
            break;


        case ACTIONSLINK_BT_LL_PACKET_TYPE_PROTOBUF:
            switch (p_message->which_Payload)
            {
                case ActionsLink_ToMcu_request_tag:
                    log_warning("bt_ul: received request while waiting for ACK (tag %d)",
                                    p_message->Payload.request.which_Request);
                    break;

                case ActionsLink_ToMcu_response_tag:
                    log_warning("bt_ul: received response while waiting for ACK (tag %d)",
                                    p_message->Payload.response.which_Response);
                    break;

                case ActionsLink_ToMcu_event_tag:
                    log_debug("bt_ul: received event message");
                    m_bt_ul.within_event_handler = true;
                    m_bt_ul.event_handler(&p_message->Payload.event, p_packet->payload.p_raw_data, p_packet->payload.raw_data_length);
                    m_bt_ul.within_event_handler = false;
                    break;

                default:
                    log_error("bt_ul: received invalid message type %d", p_message->which_Payload);
                    break;
            }
            break;

        default:
            log_error("bt_ul: received invalid packet type %d", p_packet->packet_type);
            break;

    }

    return TRANSPORT_STATE_ACK;
}

static transport_state_t transport_state_response(const actionslink_bt_ll_rx_packet_t *p_packet)
{
    const ActionsLink_ToMcu *p_message = p_packet->payload.p_message;
    switch (p_packet->packet_type)
    {
        case ACTIONSLINK_BT_LL_PACKET_TYPE_ACK:
            log_warning("bt_ul: received ACK while waiting for response (tx ID: %d)", p_packet->transaction_id);
            break;

        case ACTIONSLINK_BT_LL_PACKET_TYPE_PROTOBUF:
            switch (p_message->which_Payload)
            {
                case ActionsLink_ToMcu_request_tag:
                    log_warning("bt_ul: received request while waiting for response (tag %d)",
                                    p_message->Payload.request.which_Request);
                    break;

                case ActionsLink_ToMcu_response_tag:
                    if (p_message->Payload.response.which_Response == m_message.tag)
                    {
                        log_debug("bt_ul: received response (tag %d)",
                                        p_message->Payload.response.which_Response);
                        return TRANSPORT_STATE_SUCCESS;
                    }
                    else
                    {
                        log_warning("bt_ul: received response with unexpected tag %d",
                                        p_message->Payload.response.which_Response);
                    }
                    break;

                case ActionsLink_ToMcu_event_tag:
                    log_debug("bt_ul: received event message");
                    m_bt_ul.within_event_handler = true;
                    m_bt_ul.event_handler(&p_message->Payload.event, p_packet->payload.p_raw_data, p_packet->payload.raw_data_length);
                    m_bt_ul.within_event_handler = false;
                    break;

                default:
                    log_error("bt_ul: received invalid message type %d", p_message->which_Payload);
                    break;
            }
            break;

        default:
            log_error("bt_ul: received invalid packet type %d", p_packet->packet_type);
            break;

    }

    return TRANSPORT_STATE_RESPONSE;
}
