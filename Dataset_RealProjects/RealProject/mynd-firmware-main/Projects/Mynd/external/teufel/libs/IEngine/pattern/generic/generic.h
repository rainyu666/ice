#pragma once

#include <cstddef>
#include <cstdint>

#include "PatternSnippet.h"

template <size_t PATTERNS_NUMS, size_t LED_NUMS, size_t LED_MASK, typename T = uint8_t>
class PatternGeneric final : public IndicationEngine::LedPattern<LED_NUMS, T>
{
  public:
    explicit PatternGeneric(const IndicationEngine::Pattern<PATTERNS_NUMS, T> &pattern)
      : m_p(pattern)
      , m_id(IndicationEngine::Pattern<PATTERNS_NUMS, T>::generatePatternId())
    {
    }

    explicit PatternGeneric(const IndicationEngine::Pattern<PATTERNS_NUMS, T> &pattern, uint32_t offset)
      : m_p(pattern)
      , m_id(IndicationEngine::Pattern<PATTERNS_NUMS, T>::generatePatternId())
    {
    }

    explicit PatternGeneric(const IndicationEngine::Pattern<PATTERNS_NUMS, T> &pattern, uint8_t id)
      : m_p(pattern)
      , m_id(id)
    {
    }

    IndicationEngine::PatternResult step(std::array<T, LED_NUMS> &leds) override
    {
        for (size_t i = 0; i < LED_NUMS; ++i)
        {
            if (LED_MASK & (1u << i))
                leds[i] = m_p[m_pattern_index];
        }

        m_pattern_index++;

        if (m_pattern_index >= m_p.steps())
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
    const IndicationEngine::Pattern<PATTERNS_NUMS, T> &m_p;
    uint16_t                                           m_pattern_index = 0;
    uint8_t                                            m_id            = 0;
};
