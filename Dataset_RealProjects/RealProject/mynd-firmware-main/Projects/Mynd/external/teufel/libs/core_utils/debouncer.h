#pragma once

#include <cstdint>

template <typename T, uint32_t TIMEOUT_IN, uint32_t TIMEOUT_OUT = UINT32_MAX>
class Debouncer
{
  public:
    using get_systick_fn_t  = uint32_t();
    using get_ms_since_fn_t = uint32_t(uint32_t);

    Debouncer(T init_value, get_systick_fn_t *get_systick, get_ms_since_fn_t *get_ms_since)
      : m_value(init_value)
      , m_get_systick(get_systick)
      , m_get_ms_since(get_ms_since)
    {
    }

    T operator()(T value)
    {
        // WARNING! Only asymmetric timeouts are supported now!
        static_assert(TIMEOUT_OUT == UINT32_MAX, "Only asymmetric timeouts are supported now!");

        if constexpr (TIMEOUT_OUT == UINT32_MAX)
        {
            if (m_value != value)
            {
                if (m_last_change_ts == 0)
                    m_last_change_ts = m_get_systick();
                else if (m_get_ms_since(m_last_change_ts) >= TIMEOUT_IN)
                {
                    m_last_change_ts = 0;
                    return value;
                }
            }
            return m_value;
        }
    }

  private:
    T                  m_value;
    uint32_t           m_last_change_ts = 0u;
    get_systick_fn_t  *m_get_systick;
    get_ms_since_fn_t *m_get_ms_since;
};