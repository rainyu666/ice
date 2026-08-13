#pragma once

#include <stdint.h>
#include "t_boot_config.h"

/**
 * @brief Creates a file in dynamic area with a given name and extension.
 * @note  Name and extension must be given in capital letters.
 *        For example: VERSION.TXT
 *
 * @param[in] name          file name (up to 8 characters)
 * @param[in] extension     file extension (up to 3 characters)
 * @param[in] p_data        pointer to data to write in the file
 * @param[in] length        data length
 *
 * @return 0 if successful, -1 otherwise
 */
int t_boot_ram_disk_add_file(const char *name, const char *extension, const uint8_t *p_data, uint32_t length);

int t_boot_ram_disk_read_block(uint8_t *p_buffer, uint32_t block_address, uint16_t block_length);
int t_boot_ram_disk_write_block(uint8_t *p_buffer, uint32_t block_address, uint16_t block_length);
void t_boot_ram_disk_get_capacity(uint32_t *p_number_of_blocks, uint16_t *p_block_size);
