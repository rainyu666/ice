#pragma once

#include <cstdint>
#include <array>

/**
 * @brief The class maps a continuous range of values to a discrete set of values, and optionally applies hysteresis.
 * @note The range divided into N equal segments and the boundaries of these segments are stored in the const static
 * array. The class also implements hysteresis to prevent bouncing between adjacent values, by comparing the new value
 * with the previous value, if the new value is within the hysteresis margin of the previous value, the previous
 * value will be returned.
 * @tparam T - type of the value
 * @tparam MIN - minimum value
 * @tparam MAX - maximum value
 * @tparam NUM - number of discrete values from a given range
 * @tparam MARGIN - margin value (for hysteresis)
 */
template <typename T, T MIN, T MAX, size_t NUM, T MARGIN>
class Quantizator
{
  public:
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    static_assert(NUM > 0, "NUM must be greater than 0");
    static_assert(MARGIN >= 0, "MARGIN must be greater than 0");

    Quantizator() : m_boundaries(create_boundaries()) {}
    Quantizator(std::initializer_list<T> list)
    {
        m_boundaries[0] = MIN < *list.begin() ? MIN : *list.begin();
        for (auto it = list.begin() + 1; it != list.end(); ++it)
        {
            m_boundaries[std::distance(list.begin(), it)] = *(it - 1) + (*it - *(it - 1)) / 2;
        }
        m_boundaries[NUM] = MIN < *(list.end() - 1) ? MAX : *(list.end() - 1);
    }

    T operator()(T input)
    {
        T lb = m_last_value_idx == 0 ? MIN : m_boundaries[m_last_value_idx] - MARGIN;
        T ub = m_last_value_idx == (NUM - 1) ? MAX : m_boundaries[m_last_value_idx + 1] + MARGIN;

        if (input < lb || input > ub)
        {
            for (uint32_t i = 0; i < NUM; i++)
            {
                if (input >= m_boundaries[i] && input < m_boundaries[i + 1])
                {
                    m_last_value_idx = i;
                    return m_last_value_idx;
                }
            }
        }
        return m_last_value_idx;
    }

  private:
    static consteval auto create_boundaries()
    {
        std::array<T, NUM + 1> arr{};
        T                      step = (MAX - MIN) / NUM;
        for (uint32_t i = 0; i < NUM; i++)
            arr[i] = MIN + i * step;
        arr[NUM] = MAX;

        return arr;
    }

    std::array<T, NUM + 1> m_boundaries           = {};
    uint32_t                     m_last_value_idx = 0;
};
