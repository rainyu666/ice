#pragma once

// clang-format off
// Product type: Little endian ASCII '\0\0OA' representation
#define T_BOOT_DFU_PRODUCT_TYPE_U32     (0x414F0000)
// Produce ID: Little endian ASCII '2302' representation
#define T_BOOT_DFU_PRODUCT_ID_U32       (0x32303332)

#define T_BOOT_DFU_SKIP_SIGNATURE       1

#define T_BOOT_RAM_DISK_SIZE            (4 * 1024)

#define T_BOOT_LOG_LEVEL                LOG_LEVEL_INFO

#define T_BOOT_ENCRYPTION_KEY           "TEUFELDEV"

#define T_BOOT_DFU_NON_SEQUENTIAL_MODE  1
// clang-format on