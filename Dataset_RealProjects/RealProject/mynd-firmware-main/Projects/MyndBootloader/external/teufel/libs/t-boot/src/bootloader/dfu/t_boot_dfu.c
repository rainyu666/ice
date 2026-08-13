#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "t_boot_config.h"
#include "t_boot_crc.h"
#include "t_boot_dfu.h"
#include "t_boot_encryption.h"

#define LOG_MODULE_NAME "t_boot_dfu.c"
#define LOG_LEVEL       T_BOOT_LOG_LEVEL
#include "t_boot_logger.h"

// All packets have the same header length
#define DFU_HEADER_LEN sizeof(t_boot_dfu_header_t)

#define DFU_PACKET_TYPE_INIT      0
#define DFU_PACKET_TYPE_FW_HEADER 1
#define DFU_PACKET_TYPE_DATA      2

static const uint8_t key[] = T_BOOT_ENCRYPTION_KEY;

#if T_BOOT_DFU_SKIP_SIGNATURE == 0

#include "crypto.h"

/* buffer required for internal allocation of memory */
static uint8_t preallocated_buffer[1024];

/* Inject RSA private key here */
extern const uint8_t T1_Modulus[128];
extern const uint8_t T1_pubExp[3];

static const RSApubKey_stt PubKey_st = {
    .mExponentSize = sizeof(T1_pubExp),
    .mModulusSize  = sizeof(T1_Modulus),
    .pmExponent    = (uint8_t *) T1_pubExp,
    .pmModulus     = (uint8_t *) T1_Modulus,
};

static uint8_t signature[128]; /* Buffer that will contain the signature */

/* The SHA1 context */
static SHA1ctx_stt SHA1ctx_st;
/* Buffer that will contain the SHA-1 digest of the message */
static uint8_t Digest[CRL_SHA1_SIZE];
/* structure that will contain the preallocated buffer */
static membuf_stt mb_st;

#endif /* T_BOOT_DFU_SKIP_SIGNATURE == 0 */

typedef struct __attribute__((__packed__))
{
    uint32_t magic; // 0xBEEFCAFE
    uint8_t  packet_type;
    uint8_t  number_of_dfu_components;
    uint8_t  reserved; // reserved for future needs

    struct dfu_header_flags
    {
        // flags
        uint8_t flag_reserved1 : 1; // former dry-run
        uint8_t flag_reserved2 : 1; // former verify crc for chunks
        uint8_t force_update : 1;
        uint8_t reboot_after : 1;
        uint8_t encryption : 4;
    } flags;

    uint32_t product_type;
    uint32_t product_id;
} t_boot_dfu_header_t;

typedef struct __attribute__((__packed__))
{
    uint32_t magic;        // 0xBEEFCAFE
    uint8_t  packet_type;  // DFU_PACKET_TYPE_FW_HEADER
    uint8_t  component_id; // current component
    uint16_t chunks;       // number of total amount of chunks
    uint32_t size;         // bytes of fw component
    uint32_t fw_crc32;     // crc32 for current component firmware
} t_boot_dfu_fw_header_t;

typedef struct __attribute__((__packed__))
{
    uint32_t magic;        // 0xBEEFCAFE
    uint8_t  packet_type;  // DFU_PACKET_TYPE_DATA
    uint8_t  component_id; // current component
    uint16_t chunk_number; // current chunk number
    uint32_t length;       // bytes of data in chunk
    uint32_t chunk_crc32;  // crc sum for current chunk
} t_boot_dfu_chunk_header_t;

typedef enum
{
    DFU_PROCESS_STATE_WAIT_INIT,
    DFU_PROCESS_STATE_WAIT_FW_HEADER,
    DFU_PROCESS_STATE_WAIT_CHUNK,
    DFU_PROCESS_STATE_DONE,
    DFU_PROCESS_STATE_FAILURE,
} dfu_process_state_t;

static void                         dump_init_packet(t_boot_dfu_header_t *h) __attribute__((unused));
static void                         dump_fw_header(t_boot_dfu_fw_header_t *h) __attribute__((unused));
__attribute__((unused)) static void dump_chunk_header(t_boot_dfu_chunk_header_t *h) __attribute__((unused));
static void                         dump_512_blob(uint8_t *buf) __attribute__((unused));

typedef struct
{
    const t_boot_config_t     *p_config;
    const t_boot_dfu_target_t *p_current_dfu_target;
    uint32_t                   fw_size;
    uint32_t                   bytes_written;
    uint16_t                   next_chunk;
    uint16_t                   number_of_chunks;
    uint8_t                    dfu_components_left;
    uint8_t                    encryption_type;
    dfu_process_state_t        state;
} t_boot_context_sequential_t;

static t_boot_context_sequential_t m_boot;

typedef struct
{
    t_boot_dfu_fw_header_t fw_header;
    uint32_t               bytes_written;
} t_boot_dfu_fw_status_t;

typedef struct
{
    const t_boot_config_t  *p_config;
    t_boot_dfu_fw_status_t *fw_status;
    uint8_t                 fw_status_len;
    uint8_t                 encryption_type;
    uint32_t                bytes_in_chunk;
    uint32_t                number_of_dfu_components;
} t_boot_context_non_sequential_t;

static t_boot_context_non_sequential_t t_boot_ctx;

#ifdef T_BOOT_DRY_RUN
static int dfu_fake_prepare(void)
{
    return 0;
}

static int dfu_fake_write(void)
{
    return 0;
}

static const dfu_target_t dummy_dfu = {
    .name         = "mock",
    .component_id = 0,
    .prepare      = dfu_fake_prepare,
    .write        = dfu_fake_write,
    .get_crc32    = NULL,
    .verify       = NULL,
};
#endif

int t_boot_dfu_init(const t_boot_config_t *p_config)
{
    if (p_config == NULL)
    {
        log_error("t-boot configuration was NULL");
        return -1;
    }

    if ((p_config->p_dfu_target_list == NULL) || (p_config->dfu_target_list_size == 0))
    {
        log_error("DFU target list can't be empty");
        return -1;
    }

    for (size_t i = 0; i < p_config->dfu_target_list_size; i++)
    {
        const t_boot_dfu_target_t *p_dfu_target = &p_config->p_dfu_target_list[i];
        if ((p_dfu_target->prepare == NULL) || (p_dfu_target->write == NULL))
        {
            log_error("DFU target ID %d: prepare/write fn not set", p_dfu_target->component_id);
            return -1;
        }

        if (p_dfu_target->init)
        {
            p_dfu_target->init();
        }
    }

#ifdef T_BOOT_DFU_NON_SEQUENTIAL_MODE
    // In non-sequential mode we have to store *all* fw headers which we receiving, to track the progress.
    uint32_t fw_status_alloc_size;
    fw_status_alloc_size = sizeof(t_boot_dfu_fw_status_t) * p_config->dfu_target_list_size;

    t_boot_ctx.fw_status = (t_boot_dfu_fw_status_t *) malloc(fw_status_alloc_size); // TODO add free
    if (t_boot_ctx.fw_status == NULL)
    {
        log_error("Failed to allocate memory for fw headers");
        return -1;
    }

    memset(t_boot_ctx.fw_status, 0, fw_status_alloc_size);

    // TODO: redunduncy!
    t_boot_ctx.fw_status_len = p_config->dfu_target_list_size;

    // We assign the status list for the target components, to be able to update bytes_written field,
    // when the header is not received yet.
    for (int i = 0; i < p_config->dfu_target_list_size; ++i)
    {
        t_boot_ctx.fw_status[i].fw_header.component_id = p_config->p_dfu_target_list[i].component_id;
    }
#endif

    m_boot.p_config = p_config;
    m_boot.state    = DFU_PROCESS_STATE_WAIT_INIT;
    return 0;
}

void t_boot_dfu_reset_state(void)
{
    if (t_boot_dfu_is_busy() == false)
    {
        m_boot.state = DFU_PROCESS_STATE_WAIT_INIT;
    }
}

bool t_boot_dfu_is_busy(void)
{
    return (m_boot.state == DFU_PROCESS_STATE_WAIT_FW_HEADER) || (m_boot.state == DFU_PROCESS_STATE_WAIT_CHUNK);
}

static bool is_dfu_header(uint8_t *p_buffer, uint16_t length)
{
    (void) length;
    t_boot_dfu_header_t *p_dfu_header = (t_boot_dfu_header_t *) p_buffer;
    return (p_dfu_header->magic == 0xBEEFCAFE) && (p_dfu_header->packet_type == DFU_PACKET_TYPE_INIT);
}

static bool is_fw_header(uint8_t *p_buffer, uint16_t length)
{
    (void) length;
    t_boot_dfu_fw_header_t *p_fw_header = (t_boot_dfu_fw_header_t *) p_buffer;
    return (p_fw_header->magic == 0xBEEFCAFE) && (p_fw_header->packet_type == DFU_PACKET_TYPE_FW_HEADER);
}

static bool is_fw_header_received(const t_boot_context_non_sequential_t *ctx, uint8_t component_id)
{
    for (uint8_t i = 0; i < ctx->fw_status_len; ++i)
    {
        if (ctx->fw_status[i].fw_header.component_id == component_id)
        {
            if (ctx->fw_status[i].fw_header.magic == 0xBEEFCAFE)
                return true;
            else
                return false;
        }
    }
    return false;
}

static void set_fw_bytes_written(t_boot_context_non_sequential_t *ctx, uint8_t component_id, size_t length)
{
    for (uint8_t i = 0; i < ctx->fw_status_len; ++i)
    {
        if (ctx->fw_status[i].fw_header.component_id == component_id)
        {
            ctx->fw_status[i].bytes_written += length;
            return;
        }
    }
}

static void save_received_fw_header(t_boot_context_non_sequential_t *ctx, t_boot_dfu_fw_header_t *p_fw_header)
{
    for (uint8_t i = 0; i < ctx->fw_status_len; ++i)
    {
        if (ctx->fw_status[i].fw_header.component_id == p_fw_header->component_id)
        {
            memcpy(&ctx->fw_status[i].fw_header, p_fw_header, sizeof(t_boot_dfu_fw_header_t));
            dump_fw_header(&ctx->fw_status[i].fw_header);
        }
    }
}

static bool is_fw_complete(const t_boot_context_non_sequential_t *ctx, uint8_t component_id)
{
    for (uint8_t i = 0; i < ctx->fw_status_len; ++i)
    {
        if (ctx->fw_status[i].fw_header.component_id == component_id)
        {
            if (ctx->fw_status[i].fw_header.magic == 0xBEEFCAFE &&
                ctx->fw_status[i].bytes_written == ctx->fw_status[i].fw_header.size)
                return true;
            else
                return false;
        }
    }

    return false;
}

int t_boot_dfu_non_sequential_process_chunk(uint8_t *p_buffer, uint16_t length)
{
    static t_boot_dfu_header_t dfu_header;

    int error_code = 0;

    if (length > T_BOOT_DFU_CHUNK_SIZE)
    {
        error_code = T_BOOT_DFU_ERROR_PACKET_LEN;
        goto error_failed;
    }

    static bool update_start_callback_called = false;
    if (m_boot.p_config->update_start_fn && !update_start_callback_called)
    {
        update_start_callback_called = true;

        // as the update is non-sequential, we do not know how many components to update, we just sent 0
        m_boot.p_config->update_start_fn(0);
    }

    // Check if it's the DFU header
    if (is_dfu_header(p_buffer, length))
    {
        memcpy(&dfu_header, p_buffer, DFU_HEADER_LEN);

        t_boot_dfu_header_t *p_dfu_header = (t_boot_dfu_header_t *) p_buffer;
        log_info("DFU header received");
        log_debug(" - DFU components: %d", p_dfu_header->number_of_dfu_components);
        log_debug(" - Encryption: %d", p_dfu_header->flags.encryption);
        log_debug(" - Reboot after: %d", p_dfu_header->flags.reboot_after);
        log_debug(" - Force update: %d", p_dfu_header->flags.force_update);

        t_boot_ctx.number_of_dfu_components = p_dfu_header->number_of_dfu_components;

        return 0;
    }

    // Check if it's the FW header
    if (is_fw_header(p_buffer, length))
    {
        t_boot_dfu_fw_header_t *p_fw_header = (t_boot_dfu_fw_header_t *) p_buffer;
        // log_info("%s FW header received", m_boot.p_current_dfu_target->name);
        log_debug(" - Component ID: %d", p_fw_header->component_id);
        log_debug(" - Size: %d", p_fw_header->size);
        log_debug(" - Chunks: %d", p_fw_header->chunks);
        log_debug(" - CRC: 0x%08x", p_fw_header->fw_crc32);

        save_received_fw_header(&t_boot_ctx, p_fw_header);

        return 0;
    }

    // Process all regular (chunk) packets

    t_boot_dfu_chunk_header_t *p_chunk_header = (t_boot_dfu_chunk_header_t *) p_buffer;
    if (p_chunk_header->packet_type != DFU_PACKET_TYPE_DATA)
    {
        error_code = T_BOOT_DFU_ERROR_STATE;
        goto error_failed;
    }

    // Look up dfp from the list of pre-defined dfus
    for (int i = 0; i < m_boot.p_config->dfu_target_list_size; i++)
    {
        if (m_boot.p_config->p_dfu_target_list[i].component_id == p_chunk_header->component_id)
        {
            m_boot.p_current_dfu_target = &m_boot.p_config->p_dfu_target_list[i];
            break;
        }
    }

    // Reinit crc every chunk in case if any other(e.g. verification) lib changed it
    t_boot_crc_init();

    if (t_boot_crc_compute((uint32_t *) &p_buffer[DFU_HEADER_LEN], p_chunk_header->length) !=
        p_chunk_header->chunk_crc32)
    {
        // CRC32 calculated every single packet even if T_BOOT_SKIP_CRC_CHECK defined
#ifndef T_BOOT_SKIP_CRC_CHECK
        error_code = T_BOOT_DFU_ERROR_CRC32;
        // dump_512_blob(buf);
        goto error_failed;
#endif
    }

    // The batch size in the chunk is unknown, making it challenging to determine the offset.
    // The final chunk poses a challenge as it could be incomplete, potentially resulting in an incorrect offset.
    // To address this, we store the size of the first chunk and apply it to all subsequent chunks.
    // There is a slight concern with the last chunk: If the host (OS) sends the exact last chunk as the first one,
    // it leads to an incorrect offset, although this scenario is highly unlikely.
    if (t_boot_ctx.bytes_in_chunk == 0)
        t_boot_ctx.bytes_in_chunk = p_chunk_header->length;

    // Offset in bytes
    uint32_t offset = t_boot_ctx.bytes_in_chunk * (p_chunk_header->chunk_number - 1);

    log_debug("Writing %d bytes to offset 0x%x, chunk num: %u", p_chunk_header->length, offset,
              p_chunk_header->chunk_number);
    if (m_boot.p_current_dfu_target->write(&p_buffer[DFU_HEADER_LEN], p_chunk_header->length, offset) != 0)
    {
        error_code = T_BOOT_DFU_ERROR_WRITE;
        goto error_failed;
    }

    set_fw_bytes_written(&t_boot_ctx, p_chunk_header->component_id, p_chunk_header->length);

    // Once FW header received we can start track the progress, since now we know the fw size, amount of chunks, etc.
    if (is_fw_header_received(&t_boot_ctx, p_chunk_header->component_id))
    {
        if (is_fw_complete(&t_boot_ctx, p_chunk_header->component_id))
        {
            if (m_boot.p_config->update_component_done_fn)
            {
                m_boot.p_config->update_component_done_fn(p_chunk_header->component_id);
            }

            // TODO: check the number of components left, and call update_successful_fn only when all components
            // updated.
            if (m_boot.p_config->update_successful_fn)
            {
                m_boot.p_config->update_successful_fn();
            }

            log_info("Update process done!");
            // Update process done!
            return 1;
        }
    }

    // Process ongoing
    return 0;

error_failed:
    log_error("DFU failed (%s)", t_boot_dfu_get_error_desc(error_code));
    if (m_boot.p_config->update_error_fn)
    {
        if (m_boot.p_current_dfu_target)
        {
            m_boot.p_config->update_error_fn(m_boot.p_current_dfu_target->component_id, -error_code);
        }
        else
        {
            m_boot.p_config->update_error_fn(0, -error_code);
        }
    }
    return -error_code;
}

int t_boot_dfu_sequential_process_chunk(uint8_t *p_buffer, uint16_t length)
{
    int error_code = 0;

    if (length > T_BOOT_DFU_CHUNK_SIZE)
    {
        error_code = T_BOOT_DFU_ERROR_PACKET_LEN;
        goto error_failed;
    }

    switch (m_boot.state)
    {
        case DFU_PROCESS_STATE_WAIT_INIT:
        {
            t_boot_dfu_header_t *p_dfu_header = (t_boot_dfu_header_t *) p_buffer;
            if (p_dfu_header->packet_type != DFU_PACKET_TYPE_INIT)
            {
                error_code = T_BOOT_DFU_ERROR_INVALID_FILE;
                goto error_failed;
            }

            if (p_dfu_header->product_type != T_BOOT_DFU_PRODUCT_TYPE_U32 ||
                p_dfu_header->product_id != T_BOOT_DFU_PRODUCT_ID_U32)
            {
                error_code = T_BOOT_DFU_ERROR_PRODUCT_ID;
                goto error_failed;
            }

            log_info("DFU header received");
            log_debug(" - DFU components: %d", p_dfu_header->number_of_dfu_components);
            log_debug(" - Encryption: %d", p_dfu_header->flags.encryption);
            log_debug(" - Reboot after: %d", p_dfu_header->flags.reboot_after);
            log_debug(" - Force update: %d", p_dfu_header->flags.force_update);

            m_boot.state               = DFU_PROCESS_STATE_WAIT_FW_HEADER;
            m_boot.encryption_type     = p_dfu_header->flags.encryption;
            m_boot.dfu_components_left = p_dfu_header->number_of_dfu_components;

            if (m_boot.p_config->update_start_fn)
            {
                m_boot.p_config->update_start_fn(p_dfu_header->number_of_dfu_components);
            }
            break;
        }

        case DFU_PROCESS_STATE_WAIT_FW_HEADER:
        {
            t_boot_dfu_fw_header_t *p_fw_header = (t_boot_dfu_fw_header_t *) p_buffer;

            if (p_fw_header->packet_type != DFU_PACKET_TYPE_FW_HEADER)
            {
                error_code = T_BOOT_DFU_ERROR_STATE;
                goto error_failed;
            }

            m_boot.p_current_dfu_target = NULL;

            // Look up dfp from the list of pre-defined dfus
            for (int i = 0; i < m_boot.p_config->dfu_target_list_size; i++)
            {
                if (m_boot.p_config->p_dfu_target_list[i].component_id == p_fw_header->component_id)
                {
                    m_boot.p_current_dfu_target = &m_boot.p_config->p_dfu_target_list[i];
                    break;
                }
            }

#ifdef T_BOOT_DRY_RUN
            // faking update for debug purpose...
            m_boot.p_current_dfu_target = &dummy_dfu;
#endif
            if (m_boot.p_current_dfu_target == NULL)
            {
                error_code = T_BOOT_DFU_ERROR_UNKNOWN_DFU;
                goto error_failed;
            }

            log_info("%s FW header received", m_boot.p_current_dfu_target->name);
            log_debug(" - Component ID: %d", p_fw_header->component_id);
            log_debug(" - Size: %d", p_fw_header->size);
            log_debug(" - Chunks: %d", p_fw_header->chunks);
            log_debug(" - CRC: 0x%08x", p_fw_header->fw_crc32);

            m_boot.state            = DFU_PROCESS_STATE_WAIT_CHUNK;
            m_boot.next_chunk       = 1;
            m_boot.bytes_written    = 0;
            m_boot.number_of_chunks = p_fw_header->chunks;
            m_boot.fw_size          = p_fw_header->size;

            if (m_boot.p_current_dfu_target->prepare)
            {
                m_boot.p_current_dfu_target->prepare(p_fw_header->size, p_fw_header->fw_crc32);
            }

#if T_BOOT_DFU_SKIP_SIGNATURE == 0
            memcpy(&signature[0], &buf[DFU_HEADER_LEN], 128);

            /* Initialize it the SHA-1 Context */
            /* Default Flags */
            SHA1ctx_st.mFlags = E_HASH_DEFAULT;
            /* 20 byte of output */
            SHA1ctx_st.mTagSize = CRL_SHA1_SIZE;
            /* Init SHA-1 */
            if (SHA1_Init(&SHA1ctx_st))
            {
                error_code = T_BOOT_DFU_ERROR_SIGNATURE;
                goto error_failed;
            }

            mb_st.mSize = sizeof(preallocated_buffer);
            mb_st.mUsed = 0;
            mb_st.pmBuf = preallocated_buffer;
#endif
            break;
        }

        case DFU_PROCESS_STATE_WAIT_CHUNK:
        {
            t_boot_dfu_chunk_header_t *p_chunk_header = (t_boot_dfu_chunk_header_t *) p_buffer;
            if (p_chunk_header->packet_type != DFU_PACKET_TYPE_DATA)
            {
                error_code = T_BOOT_DFU_ERROR_STATE;
                goto error_failed;
            }

            // Reinit crc every chunk in case if any other(e.g. verification) lib changed it
            t_boot_crc_init();

            if (t_boot_crc_compute((uint32_t *) &p_buffer[DFU_HEADER_LEN], p_chunk_header->length) !=
                p_chunk_header->chunk_crc32)
            {
                // CRC32 calculated every single packet even if T_BOOT_SKIP_CRC_CHECK defined
#ifndef T_BOOT_SKIP_CRC_CHECK
                error_code = T_BOOT_DFU_ERROR_CRC32;
                // dump_512_blob(buf);
                goto error_failed;
#endif
            }

            if (m_boot.next_chunk != p_chunk_header->chunk_number)
            {
                error_code = T_BOOT_DFU_ERROR_CHUNK_NUM;
                goto error_failed;
            }

            if (m_boot.encryption_type == T_BOOT_DFU_ENCRYPTION_VIGENERE)
            {
// TODO: workaround to raise error in case if enc was skipped
#ifdef T_BOOT_SKIP_ENCRYPTION
                error_code = T_BOOT_DFU_ERROR_UNKNOWN_ENCRYPTION;
                goto error_failed;
#else
                t_boot_encryption_decode_in_place(&p_buffer[DFU_HEADER_LEN], p_chunk_header->length, &key[0], 9);
#endif
            }
            else if (m_boot.encryption_type == T_BOOT_DFU_ENCRYPTION_NONE)
            {
                // do nothing
            }
            else
            {
                error_code = T_BOOT_DFU_ERROR_UNKNOWN_ENCRYPTION;
                goto error_failed;
            }

#if T_BOOT_DFU_SKIP_SIGNATURE == 0
            // Verify signature
            if (SHA1_Append(&SHA1ctx_st, &buf[DFU_HEADER_LEN], ch_h->len))
            {
                error_code = T_BOOT_DFU_ERROR_SIGNATURE;
                goto error_failed;
            }

            if (next_chunk > chunks)
            {
                /* If this is the las chunk, verification should be calculated first */
                int32_t outputSize;
                if (SHA1_Finish(&SHA1ctx_st, Digest, &outputSize))
                {
                    error_code = T_BOOT_DFU_ERROR_SIGNATURE;
                    goto error_failed;
                }

                log_info("\r\n Verifying Hash ... sha1+ ");
                uint32_t status = RSA_PKCS1v15_Verify(&PubKey_st, Digest, E_SHA1, signature, &mb_st);
                if (status != SIGNATURE_VALID)
                {
                    log_info("FAILED");
                    error_code = T_BOOT_DFU_ERROR_SIGNATURE;
                    goto error_failed;
                }
                else
                {
                    log_info("OK ");
                }
            }
#endif

            if (m_boot.p_current_dfu_target->write(&p_buffer[DFU_HEADER_LEN], p_chunk_header->length, 0u) != 0)
            {
                error_code = T_BOOT_DFU_ERROR_WRITE;
                goto error_failed;
            }

            m_boot.bytes_written += p_chunk_header->length;
            m_boot.next_chunk++;

            if (m_boot.p_config->update_progress_fn)
            {
                m_boot.p_config->update_progress_fn(m_boot.p_current_dfu_target->component_id,
                                                    m_boot.bytes_written * 100 / m_boot.fw_size);
            }

            if (m_boot.next_chunk > m_boot.number_of_chunks)
            {
                log_debug("All FW chunks received");
                if (m_boot.p_current_dfu_target->verify)
                {
                    if (m_boot.p_current_dfu_target->verify())
                    {
                        error_code = T_BOOT_DFU_ERROR_VERIFICATION;
                        goto error_failed;
                    }
                }

                if (m_boot.p_config->update_component_done_fn)
                {
                    m_boot.p_config->update_component_done_fn(m_boot.p_current_dfu_target->component_id);
                }

                m_boot.dfu_components_left--;
                if (m_boot.dfu_components_left == 0)
                {
                    m_boot.state = DFU_PROCESS_STATE_DONE;
                    if (m_boot.p_config->update_successful_fn)
                    {
                        m_boot.p_config->update_successful_fn();
                    }
                    error_code = 1;
                }
                else
                {
                    m_boot.state = DFU_PROCESS_STATE_WAIT_FW_HEADER;
                }
            }
            break;
        }

        case DFU_PROCESS_STATE_DONE:
            // Do nothing
            error_code = 1;
            break;

        case DFU_PROCESS_STATE_FAILURE:
            // Do nothing
            break;

        default:
            // Should never reach this point
            break;
    }
    return error_code;

error_failed:
    m_boot.state = DFU_PROCESS_STATE_FAILURE;
    log_error("DFU failed (%s)", t_boot_dfu_get_error_desc(-error_code));
    if (m_boot.p_config->update_error_fn)
    {
        if (m_boot.p_current_dfu_target)
        {
            m_boot.p_config->update_error_fn(m_boot.p_current_dfu_target->component_id, -error_code);
        }
        else
        {
            m_boot.p_config->update_error_fn(0, -error_code);
        }
    }
    return -error_code;
}

int t_boot_dfu_process_chunk(uint8_t *p_buffer, uint16_t length)
{
#ifdef T_BOOT_DFU_NON_SEQUENTIAL_MODE
    return t_boot_dfu_non_sequential_process_chunk(p_buffer, length);
#else
    return t_boot_dfu_sequential_process_chunk(p_buffer, length);
#endif
}

const char *t_boot_dfu_get_error_desc(int error_code)
{
    switch (error_code)
    {
        case -T_BOOT_DFU_ERROR_PACKET_LEN:
            return "Packet length";
        case -T_BOOT_DFU_ERROR_PRODUCT_ID:
            return "Product ID";
        case -T_BOOT_DFU_ERROR_UNKNOWN_DFU:
            return "Unknown DFU";
        case -T_BOOT_DFU_ERROR_SIGNATURE:
            return "Invalid signature";
        case -T_BOOT_DFU_ERROR_CRC32:
            return "Invalid CRC";
        case -T_BOOT_DFU_ERROR_CHUNK_NUM:
            return "Unexpected chunk number";
        case -T_BOOT_DFU_ERROR_ENCRYPTION:
            return "Encryption";
        case -T_BOOT_DFU_ERROR_UNKNOWN_ENCRYPTION:
            return "Unknown encryption";
        case -T_BOOT_DFU_ERROR_STATE:
            return "Invalid state";
        case -T_BOOT_DFU_ERROR_VERIFICATION:
            return "Verification";
        case -T_BOOT_DFU_ERROR_INVALID_FILE:
            return "Invalid file";
        case -T_BOOT_DFU_ERROR_WRITE:
            return "Write error";
        default:
            return "Unknown";
    }
}

__attribute__((unused)) static void dump_512_blob(uint8_t *buf)
{
    for (int i = 0; i < 512; i++)
    {
        if ((i % 16 == 0) && (i > 0))
            log_dbg_raw("\r\n");
        log_dbg_raw("%02x ", buf[i]);
    }
    log_dbg_raw("\r\n");
}

__attribute__((unused)) static void dump_init_packet(t_boot_dfu_header_t *h)
{
    log_debug("dfu header:");
    log_debug("  product_type 0x%04lx", (h->product_type));
    log_debug("  product_id 0x%04lx", (h->product_id));
    log_debug("  fw_num: %x", h->number_of_dfu_components);
    log_debug("  encryption: %01x", h->flags.encryption);
    log_debug("  reboot_after: %d", h->flags.reboot_after);
    log_debug("  force_update: %d", h->flags.force_update);
}

__attribute__((unused)) static void dump_fw_header(t_boot_dfu_fw_header_t *h)
{
    log_debug("dfu fw header:");
    log_debug("  packet_type %d", (h->packet_type));
    log_debug("  size %ld", (h->size));
    log_debug("  fw_crc32 0x%04lx", (h->fw_crc32));
    log_debug("  component_id %d", (h->component_id));
    log_debug("  chunks %d", (h->chunks));
}

__attribute__((unused)) static void dump_chunk_header(t_boot_dfu_chunk_header_t *h)
{
    log_debug("dfu chunk header:");
    log_debug("  packet_type %d", (h->packet_type));
    log_debug("  component_id %d", (h->component_id));
    log_debug("  chunk_number %d", (h->chunk_number));
    log_debug("  len %lu", (h->length));
    log_debug("  chunk_crc32 0x%04lx", (h->chunk_crc32));
}
