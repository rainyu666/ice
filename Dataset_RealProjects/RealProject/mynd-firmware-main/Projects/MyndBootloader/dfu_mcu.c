#include "stm32f0xx_hal.h"
#include "bsp/board_hw.h"
#include "logger.h"
#include "dfu_mcu.h"
#include <stdbool.h>

static struct
{
    bool flash_was_erased;
} s_dfu_mcu;

int dfu_mcu_init(void)
{
    log_dbg("%s", __func__);
    s_dfu_mcu.flash_was_erased = false;
    return 0;
}

int dfu_mcu_prepare(uint32_t fw_size, uint32_t crc32)
{
    // Arguments are irrelevant, we are erasing the entire application area
    (void) fw_size;
    (void) crc32;

    log_dbg("%s", __func__);

    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t               PageError;
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = APPLICATION_FLASH_ADDRESS;

    // TODO: Adjust this to not erase the pages reserved for persisted settings storage
    EraseInitStruct.NbPages = APPLICATION_FLASH_SIZE / FLASH_PAGE_SIZE;

    log_info("Erasing application area from 0x%08x pages %d", EraseInitStruct.PageAddress, EraseInitStruct.NbPages);

    HAL_FLASH_Unlock();
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
    {
        log_err("Error erasing flash");
        return -1;
    }

    log_info("Flash erased");
    HAL_FLASH_Lock();

    s_dfu_mcu.flash_was_erased = true;
    return 0;
}

int dfu_mcu_write(const uint8_t *buf, uint32_t len, uint32_t offset)
{
    if (!s_dfu_mcu.flash_was_erased)
    {
        // Erase flash before writing
        dfu_mcu_prepare(0, 0);
    }

    uint32_t *data = ((uint32_t *) buf);
    uint32_t  addr = APPLICATION_FLASH_ADDRESS + offset;

    log_trace("Write %d bytes @ 0x%08X (offset: 0x%x)", len, addr, offset);

    HAL_FLASH_Unlock();
    for (uint32_t i = 0; i < len; i += 4, addr += 4, data++)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, *data) != HAL_OK)
        {
            log_err("Error writing to flash(addr: 0x%08x)", addr);

            HAL_FLASH_Lock();
            return -1;
        }
        if (*(uint32_t *) (addr) != *data)
        {
            log_err("Error verifying flash(addr: 0x%08x)", addr);
        }
    }
    HAL_FLASH_Lock();

    return 0;
}
