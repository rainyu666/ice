#pragma once

#include <cstdint>
#include <array>

namespace IndicationEngine
{
enum class PatternResult
{
    FINISHED,
    IN_PROGRESS
};

template <std::size_t LED_NUMS, typename T = uint8_t>
class LedPattern
{
  public:
    virtual PatternResult         step(std::array<T, LED_NUMS> &leds) = 0;
    virtual void                  reset()                             = 0;
    [[nodiscard]] virtual uint8_t getPatternId() const                = 0;
    virtual ~LedPattern()                                             = default;
};

template <typename T = uint8_t>
class PatternSnippet
{
  public:
    //    enum class FnSymmetry
    //            {
    //                    SYMMETRY_NONE,
    //                    SYMMETRY_X,
    //                    SYMMETRY_Y,
    //                    SYMMETRY_ORIGIN
    //            };

    [[nodiscard]] virtual std::size_t steps() const = 0;

    virtual T operator[](int index) const = 0;

    virtual ~PatternSnippet() = default;
};

template <std::size_t SIZE, typename T = uint8_t>
class PatternFn final : public PatternSnippet<T>
{
  private:
    const std::array<T, SIZE> &lookup_table;
    //    PatternSnippet::FnSymmetry       symmetry;

  public:
    explicit PatternFn(const std::array<T, SIZE> &table)
      : lookup_table(table){};

    [[nodiscard]] std::size_t steps() const override
    {
        return lookup_table.size();
    }

    T operator[](int index) const override
    {
        if (index >= 0 && index < static_cast<int>(lookup_table.size()))
        {
            return lookup_table[index];
        }
        return 0U;
    }
};

template <typename T = uint8_t>
class PatternConst final : public PatternSnippet<T>
{
  private:
    T        m_value;
    uint16_t m_steps;

  public:
    PatternConst(std::type_identity_t<T> value, uint16_t steps)
      : m_value(value)
      , m_steps(steps){};

    PatternConst(const PatternConst &)  = delete;
    PatternConst(const PatternConst &&) = delete;

    [[nodiscard]] std::size_t steps() const override
    {
        return static_cast<std::size_t>(m_steps);
    }

    T operator[](int /*index*/) const override
    {
        return m_value;
    }

    void update(T value)
    {
        m_value = value;
    }
};

inline uint8_t _generate_pattern_id()
{
    // Generator for pattern id. It downcounts from 239. This number is chosen
    // to keep the room for the manual IDs which can be set in the constructors, and usually
    // starts from 0. Additionally, some room is left in upper range for the future specific patterns.
    // Warning: This function is not thread safe!
    static uint8_t id = 240;
    return --id;
}

template <std::size_t SIZE, typename T = uint8_t>
class Pattern
{
  public:
    enum class State
    {
        IN_PROGRESS,
        FINISHED
    };

    static uint8_t generatePatternId()
    {
        return _generate_pattern_id();
    }

  private:
    std::array<PatternSnippet<T> *, SIZE> snippets;

  public:
    /* Doesn't need to support such constructors since
     * it holds const reference.
     */

    Pattern(const Pattern &)             = delete;
    Pattern(const Pattern &&)            = delete;
    Pattern &operator=(const Pattern &)  = delete;
    Pattern &operator=(const Pattern &&) = delete;

    template <typename... E>
    explicit Pattern(E &&...e)
      : snippets{{std::forward<E>(e)...}} {};

    ~Pattern() = default;

    T operator[](int index) const
    {
        int steps = 0;
        for (const auto s : snippets)
        {
            if (steps <= index && index <= static_cast<int>(s->steps()) + steps - 1)
            {
                return (*s)[index - steps];
            }
            else
            {
                steps += s->steps();
            }
        }

        return 0U;
    }

    [[nodiscard]] std::size_t steps() const
    {
        size_t steps = 0;
        for (const auto s : snippets)
            steps += s->steps();
        return steps;
    }
};

template <typename... E>
explicit Pattern(E &&...e) -> Pattern<sizeof...(E)>;

}
