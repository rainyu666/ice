#pragma once

#include <cstddef>
#include <cstdint>

#include "PatternSnippet.h"

template <size_t PATTERNS_NUMS, size_t LED_NUMS, typename T=uint8_t>
class PatternDoubleBlink final : public IndicationEngine::LedPattern<LED_NUMS>
{
  public:
    PatternDoubleBlink() = delete;
    explicit PatternDoubleBlink(const IndicationEngine::Pattern<PATTERNS_NUMS, T> &pattern, uint16_t delay = 0u)
      : m_p(pattern)
      , m_delay(delay)
      , m_id(IndicationEngine::Pattern<PATTERNS_NUMS, T>::generatePatternId())
    {
    }

    explicit PatternDoubleBlink(const IndicationEngine::Pattern<PATTERNS_NUMS, T> &pattern, uint8_t id,
                                uint16_t delay = 0u)
      : m_p(pattern)
      , m_delay(delay)
      , m_id(id)
    {
    }

    IndicationEngine::PatternResult step(std::array<T, LED_NUMS> &leds) override
    {
        memset(&leds[0], 0, LED_NUMS);

        for (size_t i = 0; i < half_leds_no; ++i)
        {
            if (is_even())
                leds[i + m_cur_part * half_leds_no] = m_p[m_pattern_index];
            else
                leds[i + m_cur_part * half_leds_no + m_cur_part * 1] = m_p[m_pattern_index];
        }
        m_pattern_index++;

        if (m_pattern_index >= m_p.steps())
        {
            m_pattern_index = 0;
            m_cur_part      = (m_cur_part == 0) ? 1 : 0;
            if (m_cur_part == 0)
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
    const IndicationEngine::Pattern<PATTERNS_NUMS, T> &m_p;
    uint16_t                                        m_delay         = 0;
    uint16_t                                        m_pattern_index = 0;
    uint8_t                                         m_cur_part      = 0;
    uint8_t                                         m_id            = 0;
    static constexpr uint8_t                        half_leds_no    = LED_NUMS / 2;

    inline constexpr bool is_even()
    {
        return !(LED_NUMS % 2);
    }
};
