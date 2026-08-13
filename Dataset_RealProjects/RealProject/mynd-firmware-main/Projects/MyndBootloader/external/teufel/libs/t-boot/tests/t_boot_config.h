#pragma once

// Product type: Little endian ASCII '\0\0AA' representation
#define T_BOOT_DFU_PRODUCT_TYPE_U32     (0x41410000)
#define T_BOOT_DFU_PRODUCT_ID_U32       (0x31303132)

// Disable sha1 signature verification
#define T_BOOT_DFU_SKIP_SIGNATURE       1

// Size of the FS localed in RAM
#define T_BOOT_RAM_DISK_SIZE            (4 * 1024)

// Log level of the t-boot library
#define T_BOOT_LOG_LEVEL                LOG_LEVEL_DEBUG

// Encryption key
#define T_BOOT_ENCRYPTION_KEY           "TEUFELDEV"

// Non-sequential mode
#define T_BOOT_DFU_NON_SEQUENTIAL_MODE  1