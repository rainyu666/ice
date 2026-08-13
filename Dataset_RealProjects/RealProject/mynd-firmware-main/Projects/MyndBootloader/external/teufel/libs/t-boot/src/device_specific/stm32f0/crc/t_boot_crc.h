#pragma once

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Initializes the CRC module used by t-boot.
 */
void t_boot_crc_init(void);

/**
 * @brief Deinitializes the CRC module used by t-boot.
 */
void t_boot_crc_deinit(void);

/**
 * @brief Computes the CRC of a given buffer.
 *
 * @param[in] p_buffer      pointer to buffer
 * @param[in] length        buffer length
 *
 * @return CRC32
 */
uint32_t t_boot_crc_compute(const uint32_t *p_buffer, uint32_t length);

#if defined(__cplusplus)
}
#endif