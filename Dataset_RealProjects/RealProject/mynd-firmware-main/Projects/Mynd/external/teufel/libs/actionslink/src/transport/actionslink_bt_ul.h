#pragma once

#include "actionslink_types.h"
#include "message.pb.h"

/**
 * @brief Handler for events sent by the Actions module.
 *
 * @param[in] p_event       pointer to received event
 * @param[in] p_data        pointer to the raw data that generated this event
 * @param[in] data_length   length of the raw data
 */
typedef void (*actionslink_bt_ul_event_handler_t)(const ActionsLink_ToMcuEvent *p_event, const uint8_t *p_data, uint16_t data_length);

/**
 * @brief Handler for requests sent by the Actions module.
 *
 * @param[in] p_request     pointer to received request
 * @param[in] p_data        pointer to the raw data that generated this response
 * @param[in] data_length   length of the raw data
 */
typedef void (*actionslink_bt_ul_request_handler_t)(const ActionsLink_ToMcuRequest *p_request, const uint8_t *p_data, uint16_t data_length);

/**
 * @brief Initializes the upper layer of the Actionslink transport.
 *
 * @param[in] p_config          pointer to the driver configuration
 * @param[in] event_handler     function to call to handle events
 */
void actionslink_bt_ul_init(const actionslink_config_t *p_config, actionslink_bt_ul_event_handler_t event_handler,
                            actionslink_bt_ul_request_handler_t request_handler);

/**
 * @brief Checks if the Actions upper transport layer is busy.
 *
 * @return true if busy, false otherwise
 */
bool actionslink_bt_ul_is_busy(void);

/**
 * @brief Requests the Actions upper transport layer to stop processing TX/RX.
 */
void actionslink_bt_ul_stop(void);

/**
 * @brief Sends a message and waits for the Actions module to send the ACK/confirmation response.
 * @note  This function validates that the response corresponds to the sent message.
 *
 * @param[in]  p_message        pointer to message to send
 * @param[out] p_response       pointer to struct where the response should be written to
 *
 * @return 0 if successful, -1 otherwise
 */
int actionslink_bt_ul_tx_rx(ActionsLink_FromMcu *p_message, ActionsLink_ToMcu *p_response);

/**
 * @brief Sends a command to the Actions module, expecting a response.
 *
 * @param[in] p_message         pointer to message to send
 *
 * @return 0 if successful, -1 otherwise
 */
int actionslink_bt_ul_tx(ActionsLink_FromMcu *p_message);

/**
 * @brief Processes received data and gets a response, if any.
 * @note  This function must be called periodically.
 *        It also parses and triggers events to be handled by the application.
 *
 * @param[out] p_response       pointer to struct where the response should be written to
 *
 * @return  0 if nothing was received
 *          1 if a message was expected and received
 *         -1 if a message was expected, but something went wrong
 */
int actionslink_bt_ul_rx(ActionsLink_ToMcu *p_response);
