#include <array>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "IEngine/IndicationEngine.h"

namespace ie = IndicationEngine;

#define PATTERN_TICK_MS            10
#define PATTERN_MS_TO_STEPS_INT(x) (((x) / PATTERN_TICK_MS) + 1)

static constexpr auto linear_up_fn   = [](float x) { return x; };
static constexpr auto linear_down_fn = [](float x) { return 1.f - x; };
static constexpr auto quadratic_fn   = [](float x) { return x * x; };
static constexpr auto cubic_fn       = [](float x) { return x * x * x; };

template <typename T, std::uint32_t DURATION_MS>
struct LookUpTable
{
    static_assert(DURATION_MS % PATTERN_TICK_MS == 0, "DURATION_MS must be a multiple of PATTERN_TICK_MS");

    uint32_t                                                  duration_ms    = DURATION_MS;
    T                                                         duration_steps = PATTERN_MS_TO_STEPS_INT(DURATION_MS);
    std::array<T, PATTERN_MS_TO_STEPS_INT(DURATION_MS)> table{};
};

TEST(IndicationEngineTest, FillLookUpTableShortWithLinearFunction)
{
    auto s = LookUpTable<uint8_t, 30>{};
    ie::FillLookUpTable(linear_up_fn, s.table, 0.f, 1.f);

    EXPECT_EQ(s.table.size(), 4);
    EXPECT_THAT(s.table, ::testing::ContainerEq(std::array<uint8_t, 4>{0, 85, 170, 255}));
}

TEST(IndicationEngineTest, FillLookUpTableShortWithQuadraticFunction)
{
    auto s = LookUpTable<uint8_t, 30>{};
    ie::FillLookUpTable(quadratic_fn, s.table, 0.f, 1.f);

    EXPECT_EQ(s.table.size(), 4);
    EXPECT_THAT(s.table, ::testing::ContainerEq(std::array<uint8_t, 4>{0, 28, 113, 255}));
}

TEST(IndicationEngineTest, FillLookUpTableShortWithCubicFunction)
{
    auto s = LookUpTable<uint8_t, 30>{};
    ie::FillLookUpTable(quadratic_fn, s.table, 0.f, 1.f);

    EXPECT_EQ(s.table.size(), 4);
    EXPECT_THAT(s.table, ::testing::ContainerEq(std::array<uint8_t, 4>{0, 28, 113, 255}));
}

TEST(IndicationEngineTest, FillLookUpTableShortWithCubicFunctionReducedBrightness)
{
    auto s = LookUpTable<uint8_t, 30>{};
    ie::FillLookUpTable(quadratic_fn, s.table, 0.f, 1.f, static_cast<uint8_t>(100u));

    EXPECT_EQ(s.table.size(), 4);
    EXPECT_THAT(s.table, ::testing::ContainerEq(std::array<uint8_t, 4>{0, 11, 44, 100}));
}

TEST(IndicationEngineTest, FillLookUpTableExtraShortWithLinearUpFunctionReducedBrightness)
{
    auto s = LookUpTable<uint8_t, 20>{};
    ie::FillLookUpTable(linear_up_fn, s.table, 0.f, 1.f, static_cast<uint8_t>(100u));

    ASSERT_EQ(s.table.size(), 3);
    EXPECT_THAT(s.table, ::testing::ContainerEq(std::array<uint8_t, 3>{0, 50, 100}));
}

TEST(IndicationEngineTest, FillLookUpTableExtraShortWithLinearDownFunctionReducedBrightness)
{
    auto s = LookUpTable<uint8_t, 20>{};
    ie::FillLookUpTable(linear_down_fn, s.table, 0.f, 1.f, static_cast<uint8_t>(100u));

    ASSERT_EQ(s.table.size(), 3);
    EXPECT_THAT(s.table, ::testing::ContainerEq(std::array<uint8_t, 3>{100, 50, 0}));
}

TEST(IndicationEngineTest, FillLookUpTableExtendedTypeUint16WithQuadraticFunctionReducedBrightness)
{
    auto s = LookUpTable<uint16_t, 20>{};
    ie::FillLookUpTable(quadratic_fn, s.table, 0.f, 1.f, static_cast<uint16_t>(1020u));

    ASSERT_EQ(s.table.size(), 3);
    EXPECT_THAT(s.table, ::testing::ContainerEq(std::array<uint16_t, 3>{0, 255, 1020}));
}
