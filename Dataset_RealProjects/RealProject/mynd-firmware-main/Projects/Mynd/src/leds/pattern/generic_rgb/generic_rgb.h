#pragma once

#include <cstddef>
#include <cstdint>

#include "PatternSnippet.h"

#define RGB_LED    3
#define RED_LED    (1 << 0)
#define GREEN_LED  (1 << 1)
#define BLUE_LED   (1 << 2)
#define YELLOW_LED (RED_LED | GREEN_LED)
#define PURPLE_LED (RED_LED | BLUE_LED)

template <size_t PATTERNS_NUMS, size_t LED_NUMS, size_t LED_MASK, typename T = uint8_t>
class PatternGenericRgb final : public IndicationEngine::LedPattern<LED_NUMS, T>
{
  public:
    explicit PatternGenericRgb(const IndicationEngine::Pattern<PATTERNS_NUMS, T> &pattern_r,
                               const IndicationEngine::Pattern<PATTERNS_NUMS, T> &pattern_g,
                               const IndicationEngine::Pattern<PATTERNS_NUMS, T> &pattern_b)
      : m_p_r(pattern_r)
      , m_p_g(pattern_g)
      , m_p_b(pattern_b)
      , m_id(IndicationEngine::Pattern<PATTERNS_NUMS, T>::generatePatternId())
    {
    }

    explicit PatternGenericRgb(const IndicationEngine::Pattern<PATTERNS_NUMS, T> &pattern_r,
                               const IndicationEngine::Pattern<PATTERNS_NUMS, T> &pattern_g,
                               const IndicationEngine::Pattern<PATTERNS_NUMS, T> &pattern_b, uint8_t id)
      : m_p_r(pattern_r)
      , m_p_g(pattern_g)
      , m_p_b(pattern_b)
      , m_id(id)
    {
    }

    IndicationEngine::PatternResult step(std::array<T, LED_NUMS> &leds) override
    {
        if constexpr (LED_MASK & RED_LED)
            leds[0] = m_p_r[m_pattern_index];
        if constexpr (LED_MASK & GREEN_LED)
            leds[1] = m_p_g[m_pattern_index];
        if constexpr (LED_MASK & BLUE_LED)
            leds[2] = m_p_b[m_pattern_index];

        m_pattern_index++;

        if (m_pattern_index >= m_p_r.steps())
        {
            m_pattern_index = 0;
            return IndicationEngine::PatternResult::FINISHED;
        }

        return IndicationEngine::PatternResult::IN_PROGRESS;
    }

    void reset() override
    {
        m_pattern_index = 0;
    }

    [[nodiscard]] uint8_t getPatternId() const override
    {
        return m_id;
    };

  private:
    const IndicationEngine::Pattern<PATTERNS_NUMS, T> &m_p_r;
    const IndicationEngine::Pattern<PATTERNS_NUMS, T> &m_p_g;
    const IndicationEngine::Pattern<PATTERNS_NUMS, T> &m_p_b;
    uint16_t                                           m_pattern_index = 0;
    uint8_t                                            m_id            = 0;
};
