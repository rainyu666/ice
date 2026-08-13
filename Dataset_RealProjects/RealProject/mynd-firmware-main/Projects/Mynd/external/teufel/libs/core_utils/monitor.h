#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <functional>
#include <type_traits>

template <typename T>
struct Monitor
{
  public:
    Monitor() = delete;

    template <class GetValue, class OnChange>
    explicit Monitor(const char *name, uint32_t period_ms, GetValue get_value, OnChange on_change,
                     uint32_t pause_after_change = 0U)
      : m_name(name)
      , m_period_ms(period_ms)
      , m_get_value(std::move(get_value))
      , m_on_change(std::move(on_change))
      , m_pause_after_change(pause_after_change)
    {
    }

    void operator()()
    {
        if (m_state == State::Stopped)
            return;

        if (m_predicate && !m_predicate())
            return;

        // Remove predicate after first call
        m_predicate = nullptr;

        // If we want to defer the start, then m_delay_after_start_ms will be set to some value.
        // If it's 0 then we can start monitoring.
        if (m_delay_after_start_ms != 0)
        {
            if (m_delay_after_start_ms > get_ms_since(m_last_tick_ms))
                return;
            m_delay_after_start_ms = 0;
        }

        if (m_pause_after_change && m_pause_after_change <= get_ms_since(m_last_change_tick_ms))
            return;

        if (m_period_ms <= get_ms_since(m_last_tick_ms) && m_pause_after_change <= get_ms_since(m_last_change_tick_ms))
        {
            m_last_tick_ms = get_systick();
            if (auto tmp = m_get_value(); tmp)
            {
                if (!m_value.has_value()) // The value wasn't queried before, and this is the first time
                {
                    m_value = tmp.value();
                    if (m_on_change)
                        m_on_change(*m_value, tmp.value());
                }
                else
                {
                    auto v = tmp.value();
                    if (memcmp(&(m_value.value()), &v, sizeof(T)))
                    {
                        if (m_on_change)
                            m_on_change(*m_value, tmp.value());
                        m_value               = tmp;
                        m_last_change_tick_ms = m_last_tick_ms;
                    }
                }
            }
        }
    };

    // clang-format off
    void start()                                                { start_internal({}, 0U); }
    void start(T initial_value)                                 { start_internal(initial_value, 0U); }
    void start_with_delay(uint32_t delay_ms)                    { start_internal({}, delay_ms); }
    void start_with_delay(T initial_value, uint32_t delay_ms)   { start_internal(initial_value, delay_ms); }
    void start_after_predicate(std::function<bool()> predicate) { start_internal({}, 0U, predicate); }
    void start_after_predicate_with_delay(std::function<bool()> predicate, uint32_t delay_ms) { start_internal({}, delay_ms, predicate); }
    // clang-format on

    void stop()
    {
        m_last_tick_ms         = 0;
        m_state                = State::Stopped;
        m_value                = std::nullopt;
        m_delay_after_start_ms = 0;
        m_predicate            = nullptr;
        log_info("Monitor: %s stopped", m_name);
    }

  private:
    void start_internal(std::optional<T> initial_value = std::nullopt, uint32_t delay_ms = 0,
                        std::function<bool()> predicate = nullptr)
    {
        if (m_state == State::Running)
            return;

        m_last_tick_ms         = get_systick();
        m_state                = State::Running;
        m_value                = initial_value;
        m_delay_after_start_ms = delay_ms;
        m_predicate            = std::move(predicate);
        if (delay_ms == 0)
        {
            log_info("Monitor: %s started", m_name);
        }
        else
        {
            log_info("Monitor: %s started with delay of %d ms", m_name, delay_ms);
        }
    }

    static uint32_t get_ms_since(uint32_t tick_ms)
    {
        uint32_t current_tick_ms = get_systick();

        // Handle tick overflow
        if (current_tick_ms < tick_ms)
        {
            // Account for the 0 by adding 1
            // -> if last tick is UINT32_MAX and current tick is 0, this function should return 1
            return (UINT32_MAX - tick_ms) + current_tick_ms + 1;
        }
        else
        {
            return current_tick_ms - tick_ms;
        }
    }

    enum class State
    {
        Stopped,
        Running
    };

    const char                       *m_name;
    State                             m_state = State::Stopped;
    uint32_t                          m_period_ms;
    std::function<std::optional<T>()> m_get_value;
    std::function<void(T, T)>         m_on_change            = nullptr;
    uint32_t                          m_pause_after_change   = 0;
    std::optional<T>                  m_value                = std::nullopt;
    uint32_t                          m_last_tick_ms         = 0;
    uint32_t                          m_last_change_tick_ms  = 0;
    uint32_t                          m_delay_after_start_ms = 0;
    std::function<bool()>             m_predicate            = nullptr;
};

template <class GetValue, class OnChange>
explicit Monitor(const char *name, uint32_t period_ms, GetValue get_value, OnChange on_change,
                 uint32_t pause_after_change = 0) -> Monitor<typename decltype(get_value())::value_type>;
