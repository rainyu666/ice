#include <string.h>

#include "t_boot_dfu.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(TbootCrc);

TEST_SETUP(TbootCrc)
{
    t_boot_crc_init();
}

TEST_TEAR_DOWN(TbootCrc)
{
    t_boot_crc_deinit();
}

TEST(TbootCrc, test_buffer1_crc)
{
    uint8_t  buffer[4] = {0xAB, 0xAB, 0xAB, 0xAB};
    uint32_t res       = t_boot_crc_compute((uint32_t *) &buffer, sizeof(buffer));

    TEST_ASSERT_EQUAL(res, 0x62F4ECDB);
}

TEST(TbootCrc, test_buffer2_crc)
{
    uint8_t  buffer[7] = {0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB};
    uint32_t res       = t_boot_crc_compute((uint32_t *) &buffer, sizeof(buffer));

    TEST_ASSERT_EQUAL(res, 0xEF5C2487);
}

TEST(TbootCrc, test_empty_crc)
{
    uint32_t res = t_boot_crc_compute(NULL, 0);

    TEST_ASSERT_EQUAL(res, 0);
}

TEST_GROUP_RUNNER(TbootCrc)
{
    RUN_TEST_CASE(TbootCrc, test_buffer1_crc);

    RUN_TEST_CASE(TbootCrc, test_buffer2_crc);

    RUN_TEST_CASE(TbootCrc, test_empty_crc);
}