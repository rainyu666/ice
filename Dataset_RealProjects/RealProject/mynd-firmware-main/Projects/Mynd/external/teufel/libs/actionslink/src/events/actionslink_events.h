#pragma once

#include "actionslink_types.h"
#include "message.pb.h"

void actionslink_events_init(const actionslink_event_handlers_t *p_event_handlers);

bool actionslink_events_has_received_system_ready(void);

void actionslink_event_handler(const ActionsLink_ToMcuEvent *p_event, const uint8_t *p_data, uint16_t data_length);
