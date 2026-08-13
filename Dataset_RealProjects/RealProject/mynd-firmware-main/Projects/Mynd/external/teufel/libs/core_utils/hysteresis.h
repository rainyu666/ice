#pragma once

#include <cstdint>
#include <concepts>

/**
 * @brief Hysteresis function
 * @tparam ThresholdValue - Threshold value
 * @tparam RecoveryValue - Recovery value
 * @return - Lambda function, which returns a boolean value if the threshold/recovery values are crossed.
 * @note The function works when the threshold value is greater than the recovery value, and vice versa.
 */
template<auto ThresholdValue, auto RecoveryValue>
requires std::integral<decltype(ThresholdValue)> && std::integral<decltype(RecoveryValue)>
constexpr auto hysteresis(const char *name)
{
    return [](std::integral auto value) {
        static bool in_critical = false;

        static_assert(ThresholdValue != RecoveryValue, "Critical and recovery values must be different");

        if constexpr(ThresholdValue > RecoveryValue)
        {
            in_critical = in_critical ? (value > RecoveryValue) : (value >= ThresholdValue);
        }
        else if constexpr(ThresholdValue < RecoveryValue)
        {
            in_critical = in_critical ? (value < RecoveryValue) : (value <= ThresholdValue);
        }

        return in_critical;
    };
}
