#pragma once

#include <cstddef>
#include <cstdint>

#include "PatternSnippet.h"

/*
 * This pattern Fades all LEDs to a specified target brightness
 * The curve function is used to determine the brightness change over time
 * with the minVal, maxVal values used to determine the section of the curve to use
 *
 * for the curve the following needs to be given:
 *      f(minVal) <= f(x) for all x in [minVal, maxVal] <= f(maxVal)
 *              or
 *      f(maxVal) <= f(x) for all x in [minVal, maxVal] <= f(minVal)
 *
 *      In other words, the curve should not exceed the limits set by f(minVal) and f(maxVal)
 *      The curve should also not be const (f(x) = c)
 */
template <size_t LED_NUMS, size_t LED_MASK, typename T = uint8_t>
class PatternFade final : public IndicationEngine::LedPattern<LED_NUMS, T>
{
  public:
    explicit PatternFade(T target_brightness, uint16_t steps, float (*curve)(float), float minValue, float maxValue)
      : m_target_brightness(target_brightness)
      , m_steps(steps)
      , m_curve(curve)
      , m_minValue(minValue)
      , m_maxValue(maxValue)
      , m_id(IndicationEngine::Pattern<1, T>::generatePatternId())
    {
    }
    explicit PatternFade(T target_brightness, uint16_t steps, float (*curve)(float), float minValue, float maxValue,
                         uint8_t id)
      : m_target_brightness(target_brightness)
      , m_steps(steps)
      , m_curve(curve)
      , m_minValue(minValue)
      , m_maxValue(maxValue)
      , m_id(id)
    {
    }

    IndicationEngine::PatternResult step(std::array<T, LED_NUMS> &leds) override
    {
        constexpr auto normalize = [](float value, float min, float max) { return (value - min) / (max - min); };

        // Calculate progress from elapsed steps (0-1)
        float progress = static_cast<float>(m_pattern_index + 1) / m_steps;

        // Calculate the y value on the curve that corresponds to the current progress
        float transformed_progress = m_curve(progress * (m_maxValue - m_minValue) + m_minValue);

        // Normalize the y value, based on the min and max y values of the curve
        // This gives us a value that we can use to interpolate between the start and end brightness
        transformed_progress = normalize(transformed_progress, m_curve(m_minValue), m_curve(m_maxValue));

        // Calculate new brightness values
        for (size_t i = 0; i < LED_NUMS; ++i)
        {
            if (LED_MASK & (1u << i))
            {
                float brightness_diff = static_cast<float>(m_target_brightness) - m_fade_begin_values[i];
                leds[i]               = static_cast<T>(m_fade_begin_values[i] + (brightness_diff * transformed_progress));
            }
        }

        m_pattern_index++;

        if (m_pattern_index >= m_steps)
        {
            m_pattern_index = 0;
            return IndicationEngine::PatternResult::FINISHED;
        }

        return IndicationEngine::PatternResult::IN_PROGRESS;
    }

    void set_current_leds(const std::array<T, LED_NUMS> &leds)
    {
        m_fade_begin_values = leds;
    }

    void reset() override
    {
        m_pattern_index = 0;
    }

    uint8_t getPatternId() const override
    {
        return m_id;
    }

  private:
    uint8_t  m_id            = 0;
    uint16_t m_pattern_index = 0;
    T        m_target_brightness;
    uint16_t m_steps;
    float (*m_curve)(float);
    float                   m_minValue;
    float                   m_maxValue;
    std::array<T, LED_NUMS> m_fade_begin_values;
};
