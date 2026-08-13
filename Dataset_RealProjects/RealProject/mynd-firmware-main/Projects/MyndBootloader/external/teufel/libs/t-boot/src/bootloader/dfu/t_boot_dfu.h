#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Application specific configuration options
#include "t_boot_config.h"

#ifndef T_BOOT_DFU_CHUNK_SIZE
#define T_BOOT_DFU_CHUNK_SIZE 512
#endif

#define T_BOOT_DFU_ENCRYPTION_NONE      0
#define T_BOOT_DFU_ENCRYPTION_VIGENERE  1

// Important: The order of the variants in this enum
//            should be kept in sync with the script
//            that generates the t-boot bin files.
typedef enum
{
    T_BOOT_DFU_COMPONENT_ID_MCU       = 0,
    T_BOOT_DFU_COMPONENT_ID_DSP       = 1,
    T_BOOT_DFU_COMPONENT_ID_BT        = 2,
    T_BOOT_DFU_COMPONENT_ID_HDMI      = 3,
    T_BOOT_DFU_COMPONENT_ID_SWA_TX    = 4,
    T_BOOT_DFU_COMPONENT_ID_SWA_RX    = 5,
    T_BOOT_DFU_COMPONENT_ID_MCU_BANK0 = 6,
    T_BOOT_DFU_COMPONENT_ID_MCU_BANK1 = 7,
    T_BOOT_DFU_COMPONENT_ID_DAB       = 8,
} t_boot_dfu_component_id_t;

// clang-format off
#define T_BOOT_DFU_ERROR_PACKET_LEN             (1)
#define T_BOOT_DFU_ERROR_PRODUCT_ID             (2)
#define T_BOOT_DFU_ERROR_UNKNOWN_DFU            (3)
#define T_BOOT_DFU_ERROR_SIGNATURE              (4)
#define T_BOOT_DFU_ERROR_CRC32                  (5)
#define T_BOOT_DFU_ERROR_CHUNK_NUM              (6)
#define T_BOOT_DFU_ERROR_ENCRYPTION             (7)
#define T_BOOT_DFU_ERROR_UNKNOWN_ENCRYPTION     (8)
#define T_BOOT_DFU_ERROR_STATE                  (9)
#define T_BOOT_DFU_ERROR_VERIFICATION           (10)
#define T_BOOT_DFU_ERROR_INVALID_FILE           (11)
#define T_BOOT_DFU_ERROR_WRITE                  (12)
// clang-format on

typedef struct
{
    // Mandatory fields
    const char *name;
    t_boot_dfu_component_id_t component_id;
    int (*prepare)(uint32_t fw_size, uint32_t crc32); // Only in sequential mode
    int (*write)(const uint8_t *data, uint32_t len, uint32_t offset);
    int (*init)(void);              // Optional field (can be NULL)
    int (*verify)(void);            // Optional field (can be NULL)
    uint32_t (*get_crc32)(void);    // Optional field (can be NULL)
} t_boot_dfu_target_t;

/**
 * @brief Function to call when an update starts.
 *
 * @param[in] number_of_components      number of components to update
 */
typedef void (*t_boot_update_start_callback_t)(uint8_t number_of_components);

/**
 * @brief Function to call after a successful update.
 */
typedef void (*t_boot_update_successful_callback_t)(void);

/**
 * @brief Function to call after a failed update.
 */
typedef void (*t_boot_update_error_callback_t)(t_boot_dfu_component_id_t component_id, int error_code);

/**
 * @brief Function to call when processing an update to report its progress.
 */
typedef void (*t_boot_update_progress_callback_t)(t_boot_dfu_component_id_t component_id, uint32_t progress);

/**
 * @brief Function to call after a DFU component has been updated successfully.
 */
typedef void (*t_boot_update_component_done_callback_t)(t_boot_dfu_component_id_t component_id);

typedef struct
{
    const t_boot_dfu_target_t *p_dfu_target_list;
    uint8_t dfu_target_list_size;
    t_boot_update_start_callback_t update_start_fn;
    t_boot_update_successful_callback_t update_successful_fn;
    t_boot_update_error_callback_t update_error_fn;
    t_boot_update_progress_callback_t update_progress_fn;
    t_boot_update_component_done_callback_t update_component_done_fn;
} t_boot_config_t;

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Initializes t-boot.
 *
 * @param[in] p_config      pointer to t-boot configuration
 *
 * @return 0 if successful, -1 otherwise
 */
int t_boot_dfu_init(const t_boot_config_t *p_config);

/**
 * @brief Resets t-boot to allow a new DFU process to start.
 * @note  The state will only be reset if t-boot is not busy processing an update.
 */
void t_boot_dfu_reset_state(void);

/**
 * @brief Checks if there's a DFU process in progress.
 *
 * @return true if busy, false otherwise
 */
bool t_boot_dfu_is_busy(void);

/**
 * @brief The main function which processes all requests regarding t-boot protocol.
 *
 * @param[in] p_buffer      pointer to buffer/chunk to process
 * @param[in] length        length of the buffer
 *
 * @return 0 - no errors, waiting for the next chunk
 *         1 - update process finished successfully
 *         < 0 - update process failed
 */
int t_boot_dfu_process_chunk(uint8_t *p_buffer, uint16_t length);

/**
 * @brief Gets a string containing the error description for a given error code.
 *
 * @param[in] error_code        error code
 *
 * @return pointer to string
 */
const char * t_boot_dfu_get_error_desc(int error_code);

#if defined(__cplusplus)
}
#endif
