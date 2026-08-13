#include "actionslink_encoders.h"
#include "actionslink_log.h"

// Check the following link for a very nice encoding/decoding example with nested repeated messages
// https://github.com/BlockWorksCo/Playground/tree/c968e22cd923ee184fe9362905a4d58e9f69cb27/ProtobufTest

// Unused code that could be useful for future reference
#if 0
static void log_encoder_error(const char *fn, const char *error)
{
    log_error("encoder: %s failed [%s]", fn, error);
}

bool actionslink_encode_bluetooth_single_bd_addr(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    actionslink_bluetooth_address_t *p_bd_addr = *((actionslink_bluetooth_address_t **) arg);

    if (!pb_encode_tag_for_field(stream, field))
    {
        log_encoder_error(__FUNCTION__, stream->errmsg);
        return false;
    }

    if (!pb_encode_string(stream, p_bd_addr->buffer, 6))
    {
        log_encoder_error(__FUNCTION__, stream->errmsg);
        return false;
    }

    return true;
}
#endif

bool actionslink_encode_string(pb_ostream_t* stream, const pb_field_t* field, void* const* arg)
{
    const char* str = (const char*)(*arg);

    if (!pb_encode_tag_for_field(stream, field))
        return false;

    return pb_encode_string(stream, (uint8_t*)str, strlen(str));
}

bool actionslink_encode_bytes(pb_ostream_t* stream, const pb_field_t* field, void* const* arg)
{
    actionslink_encode_bytes_t* data = (actionslink_encode_bytes_t*)(*arg);

    if (!pb_encode_tag_for_field(stream, field))
        return false;

    return pb_encode_string(stream, data->p_buffer, data->size);
}
