#pragma once

#include <cstdint>

#include "ux/system/system.h"

namespace Teufel::Task::Battery
{

void                   init();
void                   poll();
Ux::System::ChargeType toggle_fast_charging();
Ux::System::ChargeType get_charge_type();
void                   set_charge_type(const Ux::System::ChargeType &type);
void                   load_persistent_parameters();
void                   save_persistent_parameters();
void                   set_power_state(const Teufel::Ux::System::PowerState &state);
void                   factory_reset();

#ifdef INCLUDE_PRODUCTION_TESTS
// RAW battery data which are exposed only for prod testing purpose!
int8_t   get_battery_temperature();
uint16_t get_battery_voltage_mv();
#endif // INCLUDE_PRODUCTION_TESTS
}
