#pragma once

#include <cstdint>
#include "ux/system/system.h"

using low_battery_indication_action_fn_t = void (*)();

class BatteryIndicator
{
    using ChargerStatus = Teufel::Ux::System::ChargerStatus;

  public:
    BatteryIndicator() = delete;
    BatteryIndicator(low_battery_indication_action_fn_t low5_action, low_battery_indication_action_fn_t low10_action)
      : m_low5_action(low5_action)
      , m_low10_action(low10_action)
    {
    }

    void update_battery_level(uint8_t level, uint32_t ts);
    void update_charger_status(ChargerStatus status);

    void update_power_state(const Teufel::Ux::System::PowerState &ps, uint32_t ts);

  private:
    void m_check_battery_level(uint8_t level, uint32_t ts);

    bool                               m_is_low5_indicated  = false;
    bool                               m_is_low10_indicated = false;
    ChargerStatus                      m_charger_status     = ChargerStatus::NotConnected;
    low_battery_indication_action_fn_t m_low5_action;
    low_battery_indication_action_fn_t m_low10_action;

    uint32_t m_power_on_ts = UINT32_MAX;
    bool     m_power_is_on = false;
};
