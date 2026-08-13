#pragma once

#include <cstdint>

int8_t                                         ntc_voltage_to_temperature(float ntc);
bool                                           is_battery_temperature_in_critical_for_charge(int8_t temperature);
bool                                           is_battery_temperature_in_critical_for_discharge(int8_t temperature);
Teufel::Ux::System::BatteryCriticalTemperature battery_temperature_state(int8_t temperature);
