#include "usbd_storage.h"
#include "t_boot_ram_disk.h"

/* Application specific configuration options. */
#include "t_boot_stm32_config.h"

#define STORAGE_LUN_NBR 1
#define STORAGE_BLK_NBR 0x10000
#define STORAGE_BLK_SIZ 0x200

#define USE_RAM_STORAGE

#ifdef USE_RAM_STORAGE
// extern const uint8_t DiskImage[];   /* Disk Image */
// extern uint8_t Memory[];
#endif

/* USB Mass storage Standard Inquiry Data */
int8_t STORAGE_Inquirydata[] = {STORAGE_INQUIRYDATA_CONFIG};
/* Private function prototypes -----------------------------------------------*/
int8_t STORAGE_Init(uint8_t lun);
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
int8_t STORAGE_IsReady(uint8_t lun);
int8_t STORAGE_IsWriteProtected(uint8_t lun);
int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t STORAGE_GetMaxLun(void);

USBD_StorageTypeDef USBD_DISK_fops = {
    STORAGE_Init, STORAGE_GetCapacity, STORAGE_IsReady,   STORAGE_IsWriteProtected,
    STORAGE_Read, STORAGE_Write,       STORAGE_GetMaxLun, STORAGE_Inquirydata,
};
/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Initailizes the storage unit (medium)
 * @param  lun: Logical unit number
 * @retval Status (0 : Ok / -1 : Error)
 */
int8_t STORAGE_Init(uint8_t lun)
{
    return 0;
}

/**
 * @brief  Returns the medium capacity.
 * @param  lun: Logical unit number
 * @param  block_num: Number of total block number
 * @param  block_size: Block size
 * @retval Status (0: Ok / -1: Error)
 */
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
    int8_t ret = -1;

    t_boot_ram_disk_get_capacity(block_num, block_size);
    ret = 0;

    return ret;
}

/**
 * @brief  Checks whether the medium is ready.
 * @param  lun: Logical unit number
 * @retval Status (0: Ok / -1: Error)
 */
int8_t STORAGE_IsReady(uint8_t lun)
{
    return 0;
}

/**
 * @brief  Checks whether the medium is write protected.
 * @param  lun: Logical unit number
 * @retval Status (0: write enabled / -1: otherwise)
 */
int8_t STORAGE_IsWriteProtected(uint8_t lun)
{
    return 0;
}

/**
 * @brief  Reads data from the medium.
 * @param  lun: Logical unit number
 * @param  blk_addr: Logical block address
 * @param  blk_len: Blocks number
 * @retval Status (0: Ok / -1: Error)
 */
int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    t_boot_ram_disk_read_block(buf, blk_addr, blk_len);
    // printf("[USB] R Sector %d (%d)\r\n", blk_addr, blk_len);

    return 0;
}

/**
 * @brief  Writes data into the medium.
 * @param  lun: Logical unit number
 * @param  blk_addr: Logical block address
 * @param  blk_len: Blocks number
 * @retval Status (0 : Ok / -1 : Error)
 */
int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
#ifndef USE_RAM_STORAGE
    if (BSP_SD_IsDetected() != SD_NOT_PRESENT)
    {
        BSP_SD_WriteBlocks((uint32_t *) buf, blk_addr * STORAGE_BLK_SIZ, STORAGE_BLK_SIZ, blk_len * STORAGE_BLK_SIZ);
    }
#else
    return t_boot_ram_disk_write_block(buf, blk_addr, blk_len);

    // printf("[USB] W%d Sector %d (%d)\r\n", hasData, blk_addr, blk_len);
#endif

    return 0;
}

/**
 * @brief  Returns the Max Supported LUNs.
 * @param  None
 * @retval Lun(s) number
 */
int8_t STORAGE_GetMaxLun(void)
{
    return (STORAGE_LUN_NBR - 1);
}
