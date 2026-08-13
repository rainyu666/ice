#pragma once

#include "actionslink_types.h"
#include "message.pb.h"

typedef enum {
    ACTIONSLINK_BT_LL_PACKET_TYPE_ACK      = 0x00,
    ACTIONSLINK_BT_LL_PACKET_TYPE_PROTOBUF = 0x01,
} actionslink_bt_ll_packet_type_t;

typedef struct
{
    actionslink_bt_ll_packet_type_t packet_type;
    uint8_t value;
    uint8_t transaction_id;
    const ActionsLink_FromMcu *p_payload;
} actionslink_bt_ll_tx_packet_t;

typedef struct
{
    ActionsLink_ToMcu * p_message;      // Decoded payload
    const uint8_t *p_raw_data;          // Undecoded payload data
    uint16_t raw_data_length;           // Length of the undecoded payload data
} actionslink_bt_ll_rx_packet_payload_t;

typedef struct
{
    actionslink_bt_ll_packet_type_t packet_type;
    uint8_t value;
    uint8_t transaction_id;
    actionslink_bt_ll_rx_packet_payload_t payload;
} actionslink_bt_ll_rx_packet_t;

/**
 * @brief Initializes the low layer of the Actionslink transport.
 *
 * @param[in] p_config      pointer to the configuration
 */
void actionslink_bt_ll_init(const actionslink_config_t *p_config);

/**
 * @brief Resets the low layer of the Actionslink transport
 */
void actionslink_bt_ll_reset(void);

/**
 * @brief Sends a packet to the Actions module.
 * @note  This function reuses the buffer passed to it to build a frame around
 *        the payload. If the buffer is not big enough to construct the frame,
 *        this function will return error.
 *
 * @param[in] p_packet          pointer to packet to send
 * @param[in] p_tx_buffer       pointer to buffer to use for constructing the frame
 * @param[in] tx_buffer_size    size of the TX buffer
 *
 * @return 0 if successful, -1 otherwise
 */
int actionslink_bt_ll_tx(const actionslink_bt_ll_tx_packet_t *p_packet);

/**
 * @brief Receives bytes from the Actions module and processes them to extract a received payload.
 *
 * @param[out] p_packet         pointer to where the pointer to the received packet will be written to
 *
 * @return  0 if no packets were received
 *          1 if a packet was received
 *         -1 if a communication error occurred
 */
int actionslink_bt_ll_rx(actionslink_bt_ll_rx_packet_t *p_packet);
