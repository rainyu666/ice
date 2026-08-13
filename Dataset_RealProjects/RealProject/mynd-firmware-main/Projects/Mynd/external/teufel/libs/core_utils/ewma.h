#pragma once

#include <cstdint>

/**
 * @brief Simple (and optimized) Exponential weighted moving average.
 * The implementation is optimized for for a MCU without FPU unit, and advanced multiplication/division support.
 * However, please keep in mind, that the function not flexible as the classic IIR implementation, because
 * the coefficient is power of 2, therefore the tuning might be not precise enough for a certain task.
 * @tparam K - alpha coefficient that represented as 1/(x^K) (smooth coefficient).
 * @tparam uint_t - Unsigned integer type used for the filter state and input/output values.
 */
template <uint8_t K, class uint_t = uint16_t>
class EWMA
{
  public:
    explicit EWMA(uint_t initial = uint_t(0))
      : m_state(0u + (uint_t(initial) << K) - initial)
    {
    }

    // Update the filter with the given input and return the filtered output.
    uint_t operator()(uint_t input)
    {
        m_state = m_state + input - ((m_state + half) >> K);
        return m_state >> K;
    }

    // Get the current state of the filter.
    uint_t get() const
    {
        return m_state >> K;
    }

    static_assert(uint_t(0) < uint_t(-1), // Check that `uint_t` is an unsigned type
                  "The `uint_t` type should be an unsigned integer, otherwise, "
                  "the division using bit shifts is invalid.");

    // Fixed point representation of one half, used for rounding.
    constexpr static uint_t half = 1 << (K - 1);

  private:
    uint_t m_state;
};
