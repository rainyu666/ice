#pragma once

#include <stdint.h>
#include <stddef.h>

#if defined(__cplusplus)
extern "C"
{
#endif

    /**
     * @brief Initializes the EEPROM.
     */
    void board_link_eeprom_init(void);

    /**
     * @brief Reads data from the EEPROM.
     *
     * @param[in] address   the address to read from
     * @param[out] data     pointer to where the data should be written
     * @param[in] size      the number of bytes to read
     *
     * @return 0 if successful, -1 otherwise
     */
    int board_link_eeprom_read(uint16_t address, uint8_t *data, size_t size);

    /**
     * @brief Writes data to the EEPROM.
     *
     * @param[in] address   the address to write to
     * @param[in] data      pointer to the data to write
     * @param[in] size      the number of bytes to write
     *
     * @return 0 if successful, -1 otherwise
     */
    int board_link_eeprom_write(uint16_t address, const uint8_t *data, size_t size);

#if defined(__cplusplus)
}
#endif
