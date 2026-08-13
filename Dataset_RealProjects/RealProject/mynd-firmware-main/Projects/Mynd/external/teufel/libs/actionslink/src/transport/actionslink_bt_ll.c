#include "actionslink_bt_ll.h"
#include "actionslink_log.h"
#include "actionslink_utils.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include <string.h>

// We build the frames according to the HDLC protocol
// see https://en.wikipedia.org/wiki/High-Level_Data_Link_Control
#define HDLC_FRAME_DELIMITER            (0x7Eu)
#define HDLC_ESCAPE_CHARACTER           (0x7Du)
#define HDLC_ESCAPE_MASK                (0x20u)

#define INITIAL_CRC8_VALUE              (0x00u)

#define PACKET_HEADER_SIZE              (8u)
#define PACKET_START_MAGIC_BYTE         (0x55u)

#define PACKET_INDEX_START_BYTE         (0u)
#define PACKET_INDEX_PACKET_TYPE        (1u)
#define PACKET_INDEX_TRANSACTION_ID     (2u)
#define PACKET_INDEX_PAYLOAD_LENGTH_LSB (3u)
#define PACKET_INDEX_PAYLOAD_LENGTH_MSB (4u)
#define PACKET_INDEX_PAYLOAD_CRC        (5u)
#define PACKET_INDEX_RESERVED           (6u)
#define PACKET_INDEX_HEADER_CRC         (7u)
#define PACKET_INDEX_PAYLOAD_START      (8u)

#define UART_TX_TIMEOUT_MS          (100u)
#define UART_RX_TIMEOUT_MS          (100u)
#define EXPECTED_BYTE_RX_TIMEOUT_MS (1u)

#define PROCESS_FRAME_INCOMPLETE    (-1)
#define PROCESS_FRAME_ERROR         (-2)

#define NACK_REASON_BAD_PACKET      (1u)
#define NACK_REASON_BAD_CRC         (2u)
#define NACK_REASON_INVALID_LENGTH  (3u)
#define NACK_REASON_BUSY            (4u)

typedef enum
{
    TRANSPORT_STATE_DATA,
    TRANSPORT_STATE_ESCAPED_DATA,
} transport_state_t;

static struct
{
    const actionslink_config_t *p_config;
    transport_state_t       state;
    size_t                  received_data_length;
    size_t                  buffered_data_length;
    uint32_t                last_rx_timestamp;
} m_bt_ll;

static void    reset_transport_state(void);
static size_t  get_number_of_escaped_chars(const uint8_t *p_buffer, size_t length);
static bool    is_escape_required(uint8_t byte);
static uint8_t calculate_crc8(uint8_t crc, const uint8_t *p_buffer, size_t length);
static int     process_received_byte(uint8_t byte, actionslink_bt_ll_rx_packet_t *p_packet);
static int     validate_received_data(actionslink_bt_ll_rx_packet_t *p_packet);
static int     send_ack(uint8_t transaction_id);
static int     send_nack(uint8_t transaction_id, uint8_t nack_reason);

void actionslink_bt_ll_init(const actionslink_config_t *p_config)
{
    m_bt_ll.p_config = p_config;
    actionslink_bt_ll_reset();
}

void actionslink_bt_ll_reset(void)
{
    reset_transport_state();
    log_debug("bt_ll: transport state reset");
}

int actionslink_bt_ll_tx(const actionslink_bt_ll_tx_packet_t *p_packet)
{
    uint8_t *p_tx_buffer                     = m_bt_ll.p_config->p_tx_buffer;
    uint16_t tx_buffer_size                  = m_bt_ll.p_config->tx_buffer_size;
    p_tx_buffer[PACKET_INDEX_START_BYTE]     = PACKET_START_MAGIC_BYTE;
    p_tx_buffer[PACKET_INDEX_PACKET_TYPE]    = (p_packet->value << 3) | p_packet->packet_type;
    p_tx_buffer[PACKET_INDEX_TRANSACTION_ID] = p_packet->transaction_id;
    p_tx_buffer[PACKET_INDEX_RESERVED]       = 0x00;

    size_t payload_length;
    if (p_packet->p_payload != NULL)
    {
        if (!pb_get_encoded_size(&payload_length, ActionsLink_FromMcu_fields, p_packet->p_payload))
        {
            log_error("bt_ll: failed to calculate encoded payload size");
            return -1;
        }

        pb_ostream_t stream_out = pb_ostream_from_buffer(&p_tx_buffer[PACKET_INDEX_PAYLOAD_START], tx_buffer_size - PACKET_HEADER_SIZE);
        if (!pb_encode(&stream_out, ActionsLink_FromMcu_fields, p_packet->p_payload))
        {
            log_error("bt_ll: failed to encode payload");
            return -1;
        }

        uint8_t payload_crc = calculate_crc8(INITIAL_CRC8_VALUE, &p_tx_buffer[PACKET_INDEX_PAYLOAD_START], payload_length);
        p_tx_buffer[PACKET_INDEX_PAYLOAD_LENGTH_LSB] = payload_length & 0xFF;
        p_tx_buffer[PACKET_INDEX_PAYLOAD_LENGTH_MSB] = (payload_length >> 8) & 0xFF;
        p_tx_buffer[PACKET_INDEX_PAYLOAD_CRC] = payload_crc;
    }
    else
    {
        payload_length = 0;
        p_tx_buffer[PACKET_INDEX_PAYLOAD_LENGTH_LSB] = 0;
        p_tx_buffer[PACKET_INDEX_PAYLOAD_LENGTH_MSB] = 0;
        p_tx_buffer[PACKET_INDEX_PAYLOAD_CRC] = INITIAL_CRC8_VALUE;
    }

    uint8_t header_crc = calculate_crc8(INITIAL_CRC8_VALUE, p_tx_buffer, PACKET_HEADER_SIZE - 1);
    p_tx_buffer[PACKET_INDEX_HEADER_CRC] = header_crc;

    size_t escaped_chars_in_header  = get_number_of_escaped_chars(p_tx_buffer, PACKET_HEADER_SIZE);
    size_t escaped_chars_in_payload = get_number_of_escaped_chars(&p_tx_buffer[PACKET_INDEX_PAYLOAD_START], payload_length);

    // The amount of data to send is header + payload,
    // plus one extra byte per escaped byte
    // plus 2 bytes for start/end frame delimiters,
    size_t total_length = PACKET_HEADER_SIZE + payload_length + escaped_chars_in_header + escaped_chars_in_payload + 2;
    if (total_length > tx_buffer_size)
    {
        log_error("bt_ll: tx buffer is not large enough for tx (%d required)", total_length);
        return -1;
    }

    // We reuse the same buffer to build the HDLC frame
    // We start writing to the end of the buffer,
    // moving the bytes from the end of the original payload
    // to their final positions in the tx frame.
    // The index of any given byte in the original payload is always
    // lower than its index in the final tx frame, so this is safe
    // i.e. payload[0] is always frame[1], payload[1] is always frame[2] or frame[3], ...
    size_t index_in_escaped_array         = total_length - 1;
    p_tx_buffer[index_in_escaped_array--] = HDLC_FRAME_DELIMITER;

    uint32_t length_before_escaping_data = PACKET_HEADER_SIZE + payload_length;
    for (int i = length_before_escaping_data - 1; i >= 0; i--)
    {
        if (is_escape_required(p_tx_buffer[i]))
        {
            p_tx_buffer[index_in_escaped_array--] = p_tx_buffer[i] ^ HDLC_ESCAPE_MASK;
            p_tx_buffer[index_in_escaped_array--] = HDLC_ESCAPE_CHARACTER;
        }
        else
        {
            p_tx_buffer[index_in_escaped_array--] = p_tx_buffer[i];
        }
    }

    // After moving each byte in the payload to its respective index,
    // we should have ended with index 0, where the frame delimiter goes
    if (index_in_escaped_array != 0)
    {
        log_error("bt_ll: failed to construct tx frame");
        return -1;
    }

    p_tx_buffer[0] = HDLC_FRAME_DELIMITER;

    int ret_val = m_bt_ll.p_config->write_buffer_fn(p_tx_buffer, total_length, UART_TX_TIMEOUT_MS);
    if (ret_val != 0)
    {
        log_error("bt_ll: failed to send data over UART");
    }

    return ret_val;
}

int actionslink_bt_ll_rx(actionslink_bt_ll_rx_packet_t *p_packet)
{
    uint8_t byte;
    while (m_bt_ll.p_config->read_buffer_fn(&byte, 1, EXPECTED_BYTE_RX_TIMEOUT_MS) == 0)
    {
        m_bt_ll.last_rx_timestamp = actionslink_utils_get_ms();

        // Process the next byte and if it completes a command propagate the data to the caller
        int rx_result = process_received_byte(byte, p_packet);
        if (rx_result >= 0)
        {
            reset_transport_state();
            return 1;
        }

        // Return if there were any frame errors
        if (rx_result == PROCESS_FRAME_ERROR)
        {
            reset_transport_state();
            return -1;
        }
    }

    // Check for RX timeouts
    if (actionslink_utils_get_ms_since(m_bt_ll.last_rx_timestamp) > UART_RX_TIMEOUT_MS)
    {
        if (m_bt_ll.received_data_length > 0)
        {
            log_debug("bt_ll: timeout -> discarding partial frame (%d bytes)",
                        m_bt_ll.buffered_data_length);
            reset_transport_state();
        }
    }

    // No command has been completely received yet
    return 0;
}

static void reset_transport_state(void)
{
    m_bt_ll.received_data_length = 0;
    m_bt_ll.buffered_data_length = 0;
    m_bt_ll.state                = TRANSPORT_STATE_DATA;
}

/**
 * @brief This function processes received bytes into frames.
 *
 * @param[in] byte      byte to process
 *
 * @return 0 if successful
 *         PROCESS_FRAME_INCOMPLETE if a complete frame hasn't been received yet
 *         PROCESS_FRAME_ERROR if something went wrong while building a frame
 */
static int process_received_byte(uint8_t byte, actionslink_bt_ll_rx_packet_t *p_packet)
{
    log_trace("bt_ll: rx 0x%02X", byte);
    if (byte == HDLC_FRAME_DELIMITER)
    {
        log_trace("bt_ll: received frame delimiter");
        if (m_bt_ll.received_data_length > 0)
        {
            // Valid frames should contain at least the header
            // Frames with no payload are allowed
            if (m_bt_ll.received_data_length >= PACKET_HEADER_SIZE)
            {
                // An entire frame was received and buffered all the data received
                if (m_bt_ll.received_data_length == m_bt_ll.buffered_data_length)
                {
                    return validate_received_data(p_packet);
                }
                else
                {
                    log_error("bt_ll: rx buffer is not large enough - received %d bytes, buffered %d",
                                m_bt_ll.received_data_length, m_bt_ll.buffered_data_length);
                    send_nack(0, NACK_REASON_BUSY);
                }
            }
            else
            {
                log_warning("bt_ll: invalid rx frame (too short: %d bytes)",
                            m_bt_ll.received_data_length);
                send_nack(0, NACK_REASON_INVALID_LENGTH);
            }
            return PROCESS_FRAME_ERROR;
        }
    }
    else if (byte == HDLC_ESCAPE_CHARACTER)
    {
        // Received the escape character, next byte will need to be "unescaped"
        m_bt_ll.state = TRANSPORT_STATE_ESCAPED_DATA;
    }
    else
    {
        if (m_bt_ll.buffered_data_length < m_bt_ll.p_config->rx_buffer_size)
        {
            if (m_bt_ll.state == TRANSPORT_STATE_ESCAPED_DATA)
            {
                // Escaped bytes must invert bit 5 according to the HDLC protocol
                byte ^= HDLC_ESCAPE_MASK;
                // Next byte should be normal data again
                m_bt_ll.state = TRANSPORT_STATE_DATA;
            }
            m_bt_ll.p_config->p_rx_buffer[m_bt_ll.buffered_data_length] = byte;
            m_bt_ll.buffered_data_length++;
        }

        m_bt_ll.received_data_length++;
    }

    // Frame hasn't been received completely yet
    return PROCESS_FRAME_INCOMPLETE;
}

/**
 * @brief This function validates the received data and builds a packet from it.
 *
 * @param[out] p_packet      pointer to container where the received packet will be stored
 *
 * @return 0 if successful
 *         -1 if the received data is invalid
 */
static int validate_received_data(actionslink_bt_ll_rx_packet_t *p_packet) {
    const uint8_t *p_rx_data = m_bt_ll.p_config->p_rx_buffer;

    uint8_t calculated_header_crc = calculate_crc8(INITIAL_CRC8_VALUE, p_rx_data, PACKET_HEADER_SIZE - 1);
    if (calculated_header_crc != p_rx_data[PACKET_INDEX_HEADER_CRC])
    {
        log_error("bt_ll: invalid header crc (exp 0x%08X, recv 0x%08X)",
                        calculated_header_crc, p_rx_data[PACKET_INDEX_HEADER_CRC]);
        send_nack(0, NACK_REASON_BAD_CRC);
        return PROCESS_FRAME_ERROR;
    }

    if (p_rx_data[PACKET_INDEX_START_BYTE] != PACKET_START_MAGIC_BYTE)
    {
        log_error("bt_ll: invalid start byte (exp 0x55, recv 0x%02X)", p_rx_data[0]);
        send_nack(0, NACK_REASON_BAD_PACKET);
        return PROCESS_FRAME_ERROR;
    }

    uint8_t packet_type = p_rx_data[PACKET_INDEX_PACKET_TYPE] & 0x07;
    if (packet_type > ACTIONSLINK_BT_LL_PACKET_TYPE_PROTOBUF)
    {
        log_error("bt_ll: invalid packet type 0x%02X", packet_type);
        send_nack(0, NACK_REASON_BAD_PACKET);
        return PROCESS_FRAME_ERROR;
    }

    uint8_t value_for_packet_type = p_rx_data[PACKET_INDEX_PACKET_TYPE] >> 3;
    uint8_t transaction_id = p_rx_data[PACKET_INDEX_TRANSACTION_ID];
    uint16_t payload_length = (p_rx_data[PACKET_INDEX_PAYLOAD_LENGTH_MSB] << 8) | p_rx_data[PACKET_INDEX_PAYLOAD_LENGTH_LSB];

    uint8_t calculated_payload_crc = calculate_crc8(INITIAL_CRC8_VALUE, &p_rx_data[PACKET_INDEX_PAYLOAD_START], payload_length);
    if (calculated_payload_crc != p_rx_data[PACKET_INDEX_PAYLOAD_CRC])
    {
        log_error("bt_ll: invalid payload crc (exp 0x%08X, recv 0x%08X)",
                        calculated_payload_crc, p_rx_data[PACKET_INDEX_PAYLOAD_CRC]);
        send_nack(0, NACK_REASON_BAD_CRC);
        return PROCESS_FRAME_ERROR;
    }

    if (packet_type == ACTIONSLINK_BT_LL_PACKET_TYPE_PROTOBUF)
    {
        if (payload_length == 0)
        {
            log_error("bt_ll: received protobuf packet without payload");
            send_nack(0, NACK_REASON_BAD_PACKET);
            return PROCESS_FRAME_ERROR;
        }

        pb_istream_t stream_in = pb_istream_from_buffer(&p_rx_data[PACKET_INDEX_PAYLOAD_START], payload_length);

        if (!pb_decode(&stream_in, ActionsLink_ToMcu_fields, p_packet->payload.p_message))
        {
            log_error("bt_ll: failed to decode payload");
            send_nack(0, NACK_REASON_BAD_PACKET);
            return PROCESS_FRAME_ERROR;
        }
    }
    else
    {
        if (payload_length > 0)
        {
            log_error("bt_ll: received ack packet with payload");
            send_nack(0, NACK_REASON_BAD_PACKET);
            return PROCESS_FRAME_ERROR;
        }
    }

    p_packet->packet_type = packet_type;
    p_packet->value = value_for_packet_type;
    p_packet->transaction_id = transaction_id;
    p_packet->payload.p_raw_data = &p_rx_data[PACKET_INDEX_PAYLOAD_START];
    p_packet->payload.raw_data_length = payload_length;

    const char * const packet_type_str = (packet_type == ACTIONSLINK_BT_LL_PACKET_TYPE_ACK) ? "ack" : "protobuf";
    log_debug("bt_ll: received %s packet with value %d (tx ID: %d)",
                    packet_type_str, value_for_packet_type, transaction_id);

    if (packet_type == ACTIONSLINK_BT_LL_PACKET_TYPE_PROTOBUF)
    {
        send_ack(p_packet->transaction_id);
    }
    return 0;
}

static size_t get_number_of_escaped_chars(const uint8_t *p_buffer, size_t length)
{
    if (!p_buffer || length == 0)
    {
        return 0;
    }

    size_t escaped_chars = 0;
    for (size_t i = 0; i < length; ++i)
    {
        if (p_buffer[i] == HDLC_FRAME_DELIMITER || p_buffer[i] == HDLC_ESCAPE_CHARACTER)
        {
            escaped_chars++;
        }
    }
    return escaped_chars;
}

static bool is_escape_required(uint8_t byte)
{
    return (byte == HDLC_FRAME_DELIMITER || byte == HDLC_ESCAPE_CHARACTER);
}

// Lookup table for CRC-8 calculation
// - POLY: 0x07
// - INIT: 0x00
// - REFIN: false
// - REFOUT: false
// - XOROUT: 0x00
static const uint8_t crc8_table[] = {
    0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15,
    0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
    0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65,
    0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
    0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5,
    0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
    0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85,
    0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
    0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2,
    0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
    0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2,
    0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
    0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32,
    0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
    0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42,
    0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
    0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c,
    0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
    0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec,
    0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
    0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c,
    0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
    0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c,
    0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
    0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b,
    0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63,
    0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b,
    0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
    0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb,
    0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
    0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb,
    0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3,
};

static uint8_t calculate_crc8(uint8_t crc, const uint8_t *p_buffer, size_t length) {
    while (length--)
    {
        crc = crc8_table[crc ^ *p_buffer++];
    }
    return crc;
}

static int send_ack(uint8_t transaction_id)
{
    actionslink_bt_ll_tx_packet_t packet = {
        .packet_type = ACTIONSLINK_BT_LL_PACKET_TYPE_ACK,
        .value = 0,
        .transaction_id = transaction_id,
        .p_payload = NULL,
    };
    log_debug("bt_ll: sending ack packet (tx ID: %d)", transaction_id);
    return actionslink_bt_ll_tx(&packet);
}

static int send_nack(uint8_t transaction_id, uint8_t reason)
{
    actionslink_bt_ll_tx_packet_t packet = {
        .packet_type    = ACTIONSLINK_BT_LL_PACKET_TYPE_ACK,
        .value          = reason,
        .transaction_id = transaction_id,
        .p_payload      = NULL,
    };
    log_debug("bt_ll: sending nack packet (tx ID: %d)", transaction_id);
    return actionslink_bt_ll_tx(&packet);
}
