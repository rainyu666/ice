#include <string.h>

#include "t_boot_dfu.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(TbootDfuInit);

TEST_SETUP(TbootDfuInit) {}

TEST_TEAR_DOWN(TbootDfuInit) {}

TEST(TbootDfuInit, test_empty_config)
{
    TEST_ASSERT_EQUAL(t_boot_dfu_init(NULL), -1);

    t_boot_config_t config = {0};
    TEST_ASSERT_EQUAL(t_boot_dfu_init(&config), -1);
}

int stub_prepare() { return 0; };
int stub_write(const uint8_t *data, uint32_t len, uint32_t offset) { printf("write data 0x%x 0x%x... (size: %lu, offset: %lu)\r\n", data[0], data[1], len, offset); return 0; };
int stub_init() { return 0; };
int stub_verify() { return 0; };

uint32_t mock_get_crc32()
{
    return 0x77889900;
}

typedef struct __attribute__((__packed__))
{
    uint32_t magic;        // 0xBEEFCAFE
    uint8_t  packet_type;  // DFU_PACKET_TYPE_FW_HEADER
    uint8_t  component_id; // current component
    uint16_t chunks;       // number of total amount of chunks
    uint32_t size;         // bytes of fw component
    uint32_t fw_crc32;     // crc32 for current component firmware
} t_boot_dfu_fw_header_t;

typedef struct __attribute__((__packed__))
{
    uint32_t magic;        // 0xBEEFCAFE
    uint8_t  packet_type;  // DFU_PACKET_TYPE_DATA
    uint8_t  component_id; // current component
    uint16_t chunk_number; // current chunk number
    uint32_t length;       // bytes of data in chunk
    uint32_t chunk_crc32;  // crc sum for current chunk
} t_boot_dfu_chunk_header_t;

static void make_fw_header_packet(uint8_t *buffer, size_t length, uint8_t component_id, uint16_t chunks, uint32_t fw_length)
{
    memset(buffer, 0, length);

    t_boot_dfu_fw_header_t h = {
        .magic = 0xBEEFCAFE,
        .packet_type = 1, // DFU_PACKET_TYPE_FW_HEADER
        .component_id = component_id,
        .chunks = chunks,
        .size = fw_length,
        .fw_crc32 = 0,
    };

    memcpy(buffer, &h, sizeof(h));
}

static void make_chunk_packet(uint8_t *buffer, size_t length, uint8_t component_id, uint16_t chunk_number, uint8_t value)
{
    memset(buffer, 0, length);

    memset(buffer + sizeof(t_boot_dfu_chunk_header_t), value, length - sizeof(t_boot_dfu_chunk_header_t));

    t_boot_crc_init();
    uint32_t crc = t_boot_crc_compute((uint32_t *)(buffer + sizeof(t_boot_dfu_chunk_header_t)), length);
    t_boot_crc_deinit();

    t_boot_dfu_chunk_header_t h = {
        .magic = 0xBEEFCAFE,
        .packet_type = 2, // DFU_PACKET_TYPE_DATA
        .component_id = component_id,
        .chunk_number = chunk_number,
        .length = length,
        .chunk_crc32 = crc,
    };

    memcpy(buffer, &h, sizeof(h));
}

TEST(TbootDfuInit, test_non_sequential_process)
{
    t_boot_dfu_target_t dfus[] = {
        {
            .name = "test",
            .component_id = T_BOOT_DFU_COMPONENT_ID_MCU,
            .prepare = stub_prepare,
            .write = stub_write,
            .init = stub_init,
            .verify = stub_verify,
            .get_crc32 = mock_get_crc32,
        }
    };

    t_boot_config_t config = {
        .p_dfu_target_list = dfus,
        .dfu_target_list_size = sizeof(dfus) / sizeof(dfus[0]),
        .update_start_fn = NULL,
        .update_successful_fn = NULL,
        .update_error_fn = NULL,
        .update_progress_fn = NULL,
        .update_component_done_fn = NULL,
    };

    TEST_ASSERT_EQUAL(t_boot_dfu_init(&config), 0);

    uint8_t buffer[64];

    // First chunk
    make_chunk_packet(buffer, 32, T_BOOT_DFU_COMPONENT_ID_MCU, 1, 0x55);
    TEST_ASSERT_EQUAL(t_boot_dfu_process_chunk(buffer, 64), 0);

    // second chunk
    make_chunk_packet(buffer, 32, T_BOOT_DFU_COMPONENT_ID_MCU, 2, 0x66);
    TEST_ASSERT_EQUAL(t_boot_dfu_process_chunk(buffer, 64), 0);

    // FW header packet
    make_fw_header_packet(buffer, 32, T_BOOT_DFU_COMPONENT_ID_MCU, 4, 32*4);
    TEST_ASSERT_EQUAL(t_boot_dfu_process_chunk(buffer, 64), 0);

    // fourth chunk (out of order)
    make_chunk_packet(buffer, 32, T_BOOT_DFU_COMPONENT_ID_MCU, 4, 0x88);
    TEST_ASSERT_EQUAL(t_boot_dfu_process_chunk(buffer, 64), 0);

    // third chunk (out of order)
    make_chunk_packet(buffer, 32, T_BOOT_DFU_COMPONENT_ID_MCU, 3, 0x77);
    TEST_ASSERT_EQUAL(t_boot_dfu_process_chunk(buffer, 64), 1);
}

TEST(TbootDfuInit, test_non_sequential_process_sequential_packages)
{
t_boot_dfu_target_t dfus[] = {
    {
        .name = "test",
        .component_id = T_BOOT_DFU_COMPONENT_ID_MCU,
        .prepare = stub_prepare,
        .write = stub_write,
        .init = stub_init,
        .verify = stub_verify,
        .get_crc32 = mock_get_crc32,
    }
};

t_boot_config_t config = {
    .p_dfu_target_list = dfus,
    .dfu_target_list_size = sizeof(dfus) / sizeof(dfus[0]),
    .update_start_fn = NULL,
    .update_successful_fn = NULL,
    .update_error_fn = NULL,
    .update_progress_fn = NULL,
    .update_component_done_fn = NULL,
};

TEST_ASSERT_EQUAL(t_boot_dfu_init(&config), 0);

uint8_t buffer[64];

// FW header packet
make_fw_header_packet(buffer, 32, T_BOOT_DFU_COMPONENT_ID_MCU, 4, 32*4);
TEST_ASSERT_EQUAL(t_boot_dfu_process_chunk(buffer, 64), 0);

// First chunk
make_chunk_packet(buffer, 32, T_BOOT_DFU_COMPONENT_ID_MCU, 1, 0x55);
TEST_ASSERT_EQUAL(t_boot_dfu_process_chunk(buffer, 64), 0);

// second chunk
make_chunk_packet(buffer, 32, T_BOOT_DFU_COMPONENT_ID_MCU, 2, 0x66);
TEST_ASSERT_EQUAL(t_boot_dfu_process_chunk(buffer, 64), 0);

// third chunk
make_chunk_packet(buffer, 32, T_BOOT_DFU_COMPONENT_ID_MCU, 3, 0x77);
TEST_ASSERT_EQUAL(t_boot_dfu_process_chunk(buffer, 64), 0);

// fourth chunk
make_chunk_packet(buffer, 32, T_BOOT_DFU_COMPONENT_ID_MCU, 4, 0x88);
TEST_ASSERT_EQUAL(t_boot_dfu_process_chunk(buffer, 64), 1);

}

TEST(TbootDfuInit, test_non_sequential_process_multiple_dfus)
{
    t_boot_dfu_target_t dfus[] = {
        {
            .name = "test1",
            .component_id = T_BOOT_DFU_COMPONENT_ID_MCU,
            .prepare = stub_prepare,
            .write = stub_write,
            .init = stub_init,
            .verify = stub_verify,
            .get_crc32 = mock_get_crc32,
        },
        {
            .name = "test2",
            .component_id = T_BOOT_DFU_COMPONENT_ID_BT,
            .prepare = stub_prepare,
            .write = stub_write,
            .init = stub_init,
            .verify = stub_verify,
            .get_crc32 = mock_get_crc32,
        },
    };
    t_boot_config_t config = {
        .p_dfu_target_list = dfus,
        .dfu_target_list_size = sizeof(dfus) / sizeof(dfus[0]),
        .update_start_fn = NULL,
        .update_successful_fn = NULL,
        .update_error_fn = NULL,
        .update_progress_fn = NULL,
        .update_component_done_fn = NULL,
    };
    TEST_ASSERT_EQUAL(t_boot_dfu_init(&config), 0);
    uint8_t buffer[64];
    // second chunk BT
    make_chunk_packet(buffer, 32, T_BOOT_DFU_COMPONENT_ID_BT, 2, 0x66);
    TEST_ASSERT_EQUAL(t_boot_dfu_process_chunk(buffer, 64), 0);
    // First chunk MCU
    make_chunk_packet(buffer, 32, T_BOOT_DFU_COMPONENT_ID_MCU, 1, 0x55);
    TEST_ASSERT_EQUAL(t_boot_dfu_process_chunk(buffer, 64), 0);
    // FW header packet BT
    make_fw_header_packet(buffer, 32, T_BOOT_DFU_COMPONENT_ID_BT, 3, 32*3);
    TEST_ASSERT_EQUAL(t_boot_dfu_process_chunk(buffer, 64), 0);
    // third chunk BT
    make_chunk_packet(buffer, 32, T_BOOT_DFU_COMPONENT_ID_BT, 3, 0x11);
    TEST_ASSERT_EQUAL(t_boot_dfu_process_chunk(buffer, 64), 0);
    // second chunk MCU
    make_chunk_packet(buffer, 32, T_BOOT_DFU_COMPONENT_ID_MCU, 2, 0x66);
    TEST_ASSERT_EQUAL(t_boot_dfu_process_chunk(buffer, 64), 0);
    // FW header packet MCU
    make_fw_header_packet(buffer, 32, T_BOOT_DFU_COMPONENT_ID_MCU, 4, 32*4);
    TEST_ASSERT_EQUAL(t_boot_dfu_process_chunk(buffer, 64), 0);
    // first chunk BT
    make_chunk_packet(buffer, 32, T_BOOT_DFU_COMPONENT_ID_BT, 1, 0x55);
    TEST_ASSERT_EQUAL(t_boot_dfu_process_chunk(buffer, 64), 1);
    // fourth chunk MCU (out of order)
    make_chunk_packet(buffer, 32, T_BOOT_DFU_COMPONENT_ID_MCU, 4, 0x88);
    TEST_ASSERT_EQUAL(t_boot_dfu_process_chunk(buffer, 64), 0);
    // third chunk MCU (out of order)
    make_chunk_packet(buffer, 32, T_BOOT_DFU_COMPONENT_ID_MCU, 3, 0x77);
    TEST_ASSERT_EQUAL(t_boot_dfu_process_chunk(buffer, 64), 1);
}

TEST_GROUP_RUNNER(TbootDfuInit)
{
    // Invalid packets - NULL ptr
    RUN_TEST_CASE(TbootDfuInit, test_empty_config);

    RUN_TEST_CASE(TbootDfuInit, test_non_sequential_process);

    RUN_TEST_CASE(TbootDfuInit, test_non_sequential_process_sequential_packages);

    RUN_TEST_CASE(TbootDfuInit, test_non_sequential_process_multiple_dfus);
}
