#pragma once

#include "actionslink_types.h"
#include "message.pb.h"

void actionslink_requests_init(const actionslink_request_handlers_t *p_request_handlers);

void actionslink_request_handler(const ActionsLink_ToMcuRequest *p_request, const uint8_t *p_data, uint16_t data_length);
