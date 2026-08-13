#pragma once

#include "actionslink_types.h"
#include "pb_encode.h"

// Unused code that could be useful for future reference
#if 0
bool actionslink_encode_bluetooth_single_bd_addr(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);
#endif

typedef struct {
    uint8_t *p_buffer;
    size_t size;
} actionslink_encode_bytes_t;

bool actionslink_encode_string(pb_ostream_t* stream, const pb_field_t* field, void* const* arg);

bool actionslink_encode_bytes(pb_ostream_t* stream, const pb_field_t* field, void* const* arg);
