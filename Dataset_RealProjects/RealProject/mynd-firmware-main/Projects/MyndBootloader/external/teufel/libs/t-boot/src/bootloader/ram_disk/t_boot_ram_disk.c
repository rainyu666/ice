#include <stdint.h>
#include <string.h>

#include "t_boot_ram_disk.h"

#include "t_boot_config.h"
#include "t_boot_dfu.h"

#define WBVAL(x) ((x) &0xFF), (((x) >> 8) & 0xFF)
#define QBVAL(x) ((x) &0xFF), (((x) >> 8) & 0xFF), (((x) >> 16) & 0xFF), (((x) >> 24) & 0xFF)

// Size of full FS size (~16M)
#define FLASH_DISK_SIZE (16 * 1024 * 1024)

// clang-format off

#define SECTOR_SIZE             512
#define SECTOR_COUNT            (FLASH_DISK_SIZE / SECTOR_SIZE)
#define SECTORS_PER_CLUSTER     1
#define RESERVED_SECTORS        1
#define FAT_COPIES              2
#define ROOT_ENTRIES            512
#define ROOT_ENTRY_LENGTH       32
#define FILEDATA_START_CLUSTER  2

#define DATA_REGION_SECTOR      (RESERVED_SECTORS + (FAT_COPIES * 127) + \
                                (ROOT_ENTRIES * ROOT_ENTRY_LENGTH) / SECTOR_SIZE)

#define FILEDATA_START_SECTOR   (DATA_REGION_SECTOR + \
                                (FILEDATA_START_CLUSTER - 2) * SECTORS_PER_CLUSTER)

// This stuff is well explained in https://www.win.tue.nl/~aeb/linux/fs/fat/fat-1.html
uint8_t BootSector[] = {
    0xeb, 0x3c, 0x90,                               // code to jump to the bootstrap code
    'm', 'k', 'f', 's', '.', 'f', 'a', 't',         // OEM ID
    WBVAL(SECTOR_SIZE),
    // 0x00, 0x02,                                     // bytes per sector
    SECTORS_PER_CLUSTER,                            // sectors per cluster
    WBVAL(RESERVED_SECTORS),                        // # of reserved sectors (1 boot sector)
    FAT_COPIES,                                     // FAT copies (2)
    WBVAL(ROOT_ENTRIES),                            // root entries (512)
    WBVAL(SECTOR_COUNT),                            // total number of sectors
   // 0x00, 0x80,                                     // total number of sectors
    0xf8,                                           // media descriptor (0xF0 = Floppy, 0xF8 = Fixed disk)
    0x7f, 0x00,                                     // sectors per FAT (127)
    0x20, 0x00,                                     // sectors per track (32)
    0x40, 0x00,                                     // number of heads (64)
    0x00, 0x00, 0x00, 0x00,                         // hidden sectors (0)
    0x00, 0x00, 0x00, 0x00,                         // large number of sectors (0)
    0x00,                                           // drive number (0)
    0x00,                                           // reserved
    0x29,                                           // extended boot signature
    0xfc, 0x93, 0x60, 0x6e,                         // volume serial number
    'T', 'E', 'U', 'F', 'E', 'L', ' ', ' ', ' ', ' ', ' ', // volume label
    'F', 'A', 'T', '1', '6', ' ', ' ', ' ',         // filesystem type

    // Unsorted additional config data
    0x0e, 0x1f, 0xbe, 0x5b, 0x7c, 0xac, 0x22, 0xc0, 0x74, 0x0b, 0x56, 0xb4,
    0x0e, 0xbb, 0x07, 0x00, 0xcd, 0x10, 0x5e, 0xeb, 0xf0, 0x32, 0xe4, 0xcd,
    0x16, 0xcd, 0x19, 0xeb, 0xfe
};

uint8_t FatSector[] = {
    0xF8, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

#define FS_ATTRIBUTE_READ_ONLY       0x01
#define FS_ATTRIBUTE_HIDDEN          0x02
#define FS_ATTRIBUTE_SYSTEM_FILE     0x04
#define FS_ATTRIBUTE_VOLUME_LABEL    0x08
#define FS_ATTRIBUTE_SUB_DIR         0x10
#define FS_ATTRIBUTE_ARCHIVE         0x20
// clang-format on

typedef struct __attribute__((__packed__))
{
    uint8_t name[8];
    uint8_t extension[3];
    uint8_t attributes;
    uint8_t reserved[10];
    uint8_t time[2];
    uint8_t date[2];
    uint8_t cluster[2];
    uint8_t size[4];
} dirEntry_t;

// Hour (5 bits), minute (6 bits), double seconds (5 bits)
#define CONVERT_TIME(_h_, _m_, _s_)                                                                                    \
    {                                                                                                                  \
       (uint8_t)(((_s_) & 0x1F) / 2) | (uint8_t)((_m_) << 5),                                                          \
       (uint8_t)((_m_) >> 3) | (uint8_t)((_h_) << 3)                                                                   \
    }

// Year since 1980 (7 bits), month (4 bits), day (5 bits)
#define CONVERT_DATE(_y_, _m_, _d_)                                                                                    \
    {                                                                                                                  \
        (uint8_t)((_d_) & 0x1F) | (uint8_t)((_m_) << 5),                                                               \
        (uint8_t)((_m_) >> 3) | (uint8_t)(((_y_) - 1980) << 1)                                                         \
    }

// filesize is 64kB (128 * SECTOR_SIZE)
#define FILEDATA_SECTOR_COUNT 8 // 1kB

static uint8_t ramdata[T_BOOT_RAM_DISK_SIZE];

// The size of root dir always 512 bytes for all these projects.
// This reference was created by mkfs.fat util.
// clang-format off
uint8_t DirSector[512] = {
0x54, 0x45, 0x55, 0x46, 0x45, 0x4c, 0x20, 0x20,  0x20, 0x20, 0x20, 0x08, 0x00, 0x00, 0xa3, 0x54, // |TEUFEL     ....T|
0x77, 0x50, 0x77, 0x50, 0x00, 0x00, 0xa3, 0x54,  0x77, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // |wPwP...TwP......|
};
// clang-format on

static void write_fat_string(uint8_t *p_output, uint8_t max_size, const char *p_string);

int t_boot_ram_disk_add_file(const char *name, const char *extension, const uint8_t *p_data, uint32_t length)
{
    // Keeps the next vacant cluster which hasn't used by FS yet.
    static uint16_t next_free_dynamic_cluster = 2;

    // The value represents occupied dir entries gives the pointer to the next
    // free memory region. Keep in mind that one default entries already in used by design.
    static uint8_t num_dir_entries = 1;

    if (p_data == NULL)
    {
        return -1;
    }

    dirEntry_t fd = {
        .name       = {'A', 'B', 'C', 'D', 'E', 'F', ' ', ' '},
        .extension  = {'T', 'X', 'T'},
        .attributes = FS_ATTRIBUTE_READ_ONLY,
        .reserved   = {0x18, 0x0F, 0x13, 0x75, 0x72, 0x4E, 0x72, 0x4E, 0x00, 0x00},
        .time       = CONVERT_TIME(6, 30, 20),
        .date       = CONVERT_DATE(2019, 3, 18),
        .cluster    = { WBVAL(next_free_dynamic_cluster) },
        .size       = { QBVAL(length) },
    };

    write_fat_string(fd.name, 8, name);
    write_fat_string(fd.extension, 3, extension);

    // TODO: check the last empty cluster....
    memcpy(&DirSector[num_dir_entries * ROOT_ENTRY_LENGTH], &fd, ROOT_ENTRY_LENGTH);

    uint32_t data_offset = (next_free_dynamic_cluster - 2) * SECTORS_PER_CLUSTER * SECTOR_SIZE;
    if (data_offset + length > sizeof(ramdata))
    {
        return -1;
    }

    memcpy(&ramdata[data_offset], p_data, length);

    next_free_dynamic_cluster++;
    num_dir_entries++;

    return 0;
}

int t_boot_ram_disk_read_block(uint8_t *p_buffer, uint32_t block_address, uint16_t block_length)
{
    // TODO: We should be using block_length here
    (void) block_length;

    // TODO: Make sure that the buffer is at least SECTOR_SIZE bytes long
    memset(p_buffer, 0, SECTOR_SIZE);
    switch (block_address)
    {
        case 0:   // sector 0 is the boot sector
            memcpy(p_buffer, BootSector, sizeof(BootSector));
            p_buffer[SECTOR_SIZE - 2] = 0x55;
            p_buffer[SECTOR_SIZE - 1] = 0xAA;
            break;
        case 1:   // sector 1 is FAT 1st copy
        case 127: // sector 2 is FAT 2nd copy
            memcpy(p_buffer, FatSector, sizeof(FatSector));
            break;
        case 255: // sector 3 is the directory entry
            memcpy(p_buffer, DirSector, sizeof(DirSector));
            break;
        default:
            // Filesystem data access: ignore reads outside the data section
            if (block_address >= FILEDATA_START_SECTOR && block_address < FILEDATA_START_SECTOR + FILEDATA_SECTOR_COUNT)
            {
                memcpy(p_buffer, ramdata + (block_address - FILEDATA_START_SECTOR) * SECTOR_SIZE, SECTOR_SIZE);
            }
            break;
    }

    return 0;
}

int t_boot_ram_disk_write_block(uint8_t *p_buffer, uint32_t block_address, uint16_t block_length)
{
    static uint32_t saved_addr = 0;

    if (t_boot_dfu_is_busy())
    {
        if (block_address <= saved_addr)
        {
            return 0;
        }

        saved_addr = block_address;
    }

    uint32_t magic = *((uint32_t *)p_buffer);
    if (magic != 0xbeefcafe)
    {
        return 0;
    }

    t_boot_dfu_process_chunk(p_buffer, block_length);
    return 0;
}

void t_boot_ram_disk_get_capacity(uint32_t *p_number_of_blocks, uint16_t *p_block_size)
{
    // BLOCK_SIZE equals to SECTOR_SIZE
    *p_number_of_blocks  = SECTOR_COUNT;
    *p_block_size = SECTOR_SIZE;
}

static void write_fat_string(uint8_t *p_output, uint8_t max_size, const char *p_string)
{
    size_t string_length = strlen(p_string);
    for (uint8_t i = 0; i < max_size; i++)
    {
        if (i < string_length)
        {
            if (p_string[i] >= 'A' && p_string[i] <= 'Z')
            {
                p_output[i] = p_string[i];
            }
            else if (p_string[i] >= 'a' && p_string[i] <= 'z')
            {
                p_output[i] = p_string[i] - 0x20;
            }
            else
            {
                // FAT accepts this character in the file name
                p_output[i] = '~';
            }
        }
        else
        {
            // Fill the rest of the file name with spaces, do not use a null-terminator
            p_output[i] = ' ';
        }
    }
}
