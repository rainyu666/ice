// Due to the flash size limit, only the WARNING level is available
// for use in the complete firmware (including the bootloader).
#if defined(BOOTLOADER)
#define LOG_LEVEL LOG_LEVEL_ERROR
#else
#define LOG_LEVEL LOG_LEVEL_INFO
#endif

#include "board_link_eeprom.h"
#include "board_link_usb_pd_controller.h"
#include "board_hw.h"
#include "logger.h"

// size 32KB
#define EEPROM_SIZE        (32 * 1024)
#define EEPROM_I2C_ADDRESS 0x50

void board_link_eeprom_init(void) {}

int board_link_eeprom_write(uint16_t address, const uint8_t *data, size_t size)
{
    if (size == 0)
    {
        return 0;
    }

    if (data == NULL)
    {
        return -1;
    }

#if 0
    // if (size > 8)
    // {
    //     log_error("EEPROM write supports only 8 bytes at a time.");
    //     return -1;
    // }

    if (address + size > EEPROM_SIZE)
    {
        return -1;
    }

    uint8_t address_lsb = address & 0xFF;
    uint8_t address_msb = (address >> 8) & 0xFF;

    uint8_t buf[17] = {0};
    buf[0] = address_lsb;
    for (size_t i = 0; i < size; i++)
    {
        buf[i + 1] = data[i];
    }

    return board_link_usb_pd_controller_i2c_write(EEPROM_I2C_ADDRESS, address_msb, buf, size + 1);
#endif

    return board_link_usb_pd_controller_eeprom_write(address, data, size);
}

int board_link_eeprom_read(uint16_t address, uint8_t *data, size_t size)
{
    if (size == 0)
    {
        return 0;
    }

    if (data == NULL)
    {
        return -1;
    }

    if (address + size > EEPROM_SIZE)
    {
        return -1;
    }

    // if (board_link_usb_pd_controller_i2c_write(EEPROM_I2C_ADDRESS, address_msb, &address_lsb, 1) < 0)
    // if (board_link_usb_pd_controller_i2c_write(EEPROM_I2C_ADDRESS, 0xff, &data[0], 2) < 0)
    // {
    //     log_error("Failed to write EEPROM address");
    //     return -1;
    // }

    return board_link_usb_pd_controller_eeprom_read(address, data, size);
}
