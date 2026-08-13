#include <gtest/gtest.h>

#include "core_utils/hysteresis.h"

TEST(HysteresisTest, ThresholdGreaterThanRecovery_InactiveToActive)
{
    auto hyst = hysteresis<10, 5>("test");
    ASSERT_FALSE(hyst(4));
    ASSERT_FALSE(hyst(9)); // Still in critical state
    ASSERT_TRUE(hyst(10));
}

TEST(HysteresisTest, ThresholdGreaterThanRecovery_ActiveToInactive)
{
    auto hyst = hysteresis<10, 5>("test");
    ASSERT_TRUE(hyst(10));
    ASSERT_TRUE(hyst(6)); // Still in critical state
    ASSERT_FALSE(hyst(5));
}

TEST(HysteresisTest, ThresholdLessThanRecovery_InactiveToActive)
{
    auto hyst = hysteresis<5, 10>("test");
    ASSERT_FALSE(hyst(11));
    ASSERT_FALSE(hyst(6)); // Still in critical state
    ASSERT_TRUE(hyst(5));
}

TEST(HysteresisTest, ThresholdLessThanRecovery_ActiveToInactive)
{
    auto hyst = hysteresis<5, 10>("test");
    ASSERT_TRUE(hyst(5));
    ASSERT_TRUE(hyst(9)); // Still in critical state
    ASSERT_FALSE(hyst(10));
}
