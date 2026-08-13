#include "battery_indicator.h"

void BatteryIndicator::update_battery_level(uint8_t level, uint32_t ts)
{
    m_check_battery_level(level, ts);
}

void BatteryIndicator::update_charger_status(ChargerStatus status)
{
    if (status == ChargerStatus::Active)
    {
        m_is_low10_indicated = false;
        m_is_low5_indicated  = false;
    }
    m_charger_status = status;
}

void BatteryIndicator::update_power_state(const Teufel::Ux::System::PowerState &ps, uint32_t ts)
{
    if (ps == Teufel::Ux::System::PowerState::On)
    {
        m_is_low10_indicated = false;
        m_is_low5_indicated  = false;

        m_power_on_ts = ts;
        m_power_is_on = true;
    }
    else
    {
        m_power_is_on = false;
    }
}

void BatteryIndicator::m_check_battery_level(uint8_t level, uint32_t ts)
{
    // This is the edge case where the battery level is 0 during power on!
    // According to the spec the unit has to be powered off if the battery level is 0 after power on.
    // All indication must be synchronized with the power state, and it's managed in power transition handlers.
    if (level == 0 && ts < 10000)
    {
        m_is_low5_indicated  = true;
        m_is_low10_indicated = true;
        m_power_on_ts        = UINT32_MAX;
        return;
    }

    if (!m_power_is_on)
        return;

    // Wait 5 seconds after power on to indicate low battery (to avoid sound icons overlapping)
    if (ts - m_power_on_ts < 5000)
    {
        log_debug("Battery level check skipped, waiting for power on indication to finish");
        return;
    }

    if (m_charger_status == ChargerStatus::Active)
        return;

    if (level < 5 && !m_is_low5_indicated)
    {
        m_low5_action();
        m_is_low5_indicated  = true;
        m_is_low10_indicated = true; // Set low10Indicated to true to ignore the 10% indication
    }
    else if (level < 10 && !m_is_low10_indicated)
    {
        m_low10_action();
        m_is_low10_indicated = true;
    }
}
