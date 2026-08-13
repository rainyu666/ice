#pragma once

// Product type: Little endian ASCII '\0\0AD' representation
#define T_BOOT_DFU_PRODUCT_TYPE_U32     (0x44410000)

// Product ID: Little endian ASCII '2101' representation
#define T_BOOT_DFU_PRODUCT_ID_U32       (0x31303132)

// Skips signature verification if set
#define T_BOOT_DFU_SKIP_SIGNATURE       1

// Size of the FS localed in RAM
#define T_BOOT_RAM_DISK_SIZE            (4 * 1024)

// Log level of the t-boot library
#define T_BOOT_LOG_LEVEL                LOG_LEVEL_INFO

// Encryption key
#define T_BOOT_ENCRYPTION_KEY           "TEUFELDEV"
