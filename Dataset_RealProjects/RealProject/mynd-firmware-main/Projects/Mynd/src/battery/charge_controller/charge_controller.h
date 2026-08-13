#pragma once

#include <cstdint>
#include <optional>

#include "logger.h"
#include "bsp/board.h"
#include "ux/system/system.h"

template <typename T, T Threshold, uint32_t DurationMs = 5000u>
struct Counter
{
  public:
    Counter() = default;
    bool operator()(T value)
    {
        if (value > Threshold)
        {
            m_last_match_condition_ts = std::nullopt;
            return false;
        }

        if (m_last_match_condition_ts.has_value())
        {
            // If sampling duration is less than min_sample_duration, reset the counter
            // if (board_get_ms_since(*m_last_match_condition_ts) > min_sample_duration)
            // {
            //     m_last_match_condition_ts = std::nullopt;
            //     return false;
            // }

            if (board_get_ms_since(*m_last_match_condition_ts) > DurationMs)
            {
                return true;
            }
        }
        else
        {
            m_last_match_condition_ts = get_systick();
        }
        return false;
    }
    void reset()
    {
        m_last_match_condition_ts = std::nullopt;
    }

  private:
    // Minimum sample duration in samples (X% of DURATION_MS)
    // constexpr static uint32_t m_min_sample_duration     = DurationMs / 4u;
    std::optional<uint32_t> m_last_match_condition_ts = std::nullopt;
};

struct IChargerLLController
{
    virtual void enable(bool bfc_enabled) = 0;
    virtual void disable()                = 0;

    virtual void on_full_charge() = 0;

    // virtual ~IChargerLLController() = default;
};

class ChargeController
{
    using ChargerStatus = Teufel::Ux::System::ChargerStatus;

  public:
    ChargeController(IChargerLLController &charger_ll_controller)
      : m_charger_ll_controller(charger_ll_controller)
    {
        m_charger_ll_controller.disable();
    }

    ChargerStatus process(uint16_t battery_voltage_mv, int16_t battery_current_ma, bool charger_ntc_allowed,
                          bool ac_plugged, bool bfc_enabled);

  private:
    void m_reset_counters()
    {
        m_battery_below_7900mv_counter.reset();
        m_battery_below_8100mv_counter.reset();
        m_battery_current_below_500ma_counter.reset();
    }

    IChargerLLController &m_charger_ll_controller;
    ChargerStatus         m_charger_status = ChargerStatus::NotConnected;

    Counter<uint32_t, 7900> m_battery_below_7900mv_counter{};
    Counter<uint32_t, 8100> m_battery_below_8100mv_counter{};
    Counter<int32_t, 500>   m_battery_current_below_500ma_counter{};
};
