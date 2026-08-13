#include <cmath>
#include <algorithm>
#include <array>
#include "external/teufel/libs/core_utils/hysteresis.h"
#include "config.h"
#include "ux/system/system.h"
#include "board_link_hw_revision.h"
#include "temperature.h"

int8_t ntc_voltage_to_temperature(float ntc)
{
    static const auto ntc_poly_coefs =
        std::array<float, 5>{-0.46999446f, -5.25737016f, 35.33647663f, -98.7806335f, 116.1458077f};

    // clang-format off
    // Approximate via polynomial
    auto temp = ntc_poly_coefs[0] * ntc * ntc * ntc * ntc +
                ntc_poly_coefs[1] * ntc * ntc * ntc +
                ntc_poly_coefs[2] * ntc * ntc +
                ntc_poly_coefs[3] * ntc +
                ntc_poly_coefs[4];
    // clang-format on

    return static_cast<int8_t>(std::roundf(std::clamp(temp, -30.f, 70.f)));
}

static auto charging_over_temp_in_critical =
    hysteresis<CONFIG_CHARGE_OVER_TEMP_PROTECTION_DEGREE, CONFIG_CHARGE_OVER_TEMP_PROTECTION_RECOVERY_DEGREE>(
        "OverTemperatureChargingProtector");
static auto discharge_over_temp_in_critical =
    hysteresis<CONFIG_DISCHARGE_OVER_TEMP_PROTECTION_DEGREE, CONFIG_DISCHARGE_OVER_TEMP_PROTECTION_RECOVERY_DEGREE>(
        "OverTemperatureDischargingProtector");
static auto charging_under_temp_in_critical =
    hysteresis<CONFIG_CHARGE_UNDER_TEMP_PROTECTION_DEGREE, CONFIG_CHARGE_UNDER_TEMP_PROTECTION_RECOVERY_DEGREE>(
        "UnderTemperatureChargingProtector");
static auto discharge_under_temp_in_critical =
    hysteresis<CONFIG_DISCHARGE_UNDER_TEMP_PROTECTION_DEGREE, CONFIG_DISCHARGE_UNDER_TEMP_PROTECTION_RECOVERY_DEGREE>(
        "UnderTemperatureDischargingProtector");

bool is_battery_temperature_in_critical_for_charge(int8_t temperature)
{
    auto is_charging_over_temp_critical  = charging_over_temp_in_critical(temperature);
    auto is_charging_under_temp_critical = charging_under_temp_in_critical(temperature);

    return is_charging_over_temp_critical || is_charging_under_temp_critical;
}

bool is_battery_temperature_in_critical_for_discharge(int8_t temperature)
{
    auto is_discharging_over_temp_critical  = discharge_over_temp_in_critical(temperature);
    auto is_discharging_under_temp_critical = discharge_under_temp_in_critical(temperature);

    return is_discharging_over_temp_critical || is_discharging_under_temp_critical;
}

Teufel::Ux::System::BatteryCriticalTemperature battery_temperature_state(int8_t temperature)
{
    namespace Tus = Teufel::Ux::System;

    auto is_charging_over_temp_critical     = charging_over_temp_in_critical(temperature);
    auto is_charging_under_temp_critical    = charging_under_temp_in_critical(temperature);
    auto is_discharging_over_temp_critical  = discharge_over_temp_in_critical(temperature);
    auto is_discharging_under_temp_critical = discharge_under_temp_in_critical(temperature);

    if (is_charging_under_temp_critical)
        return Tus::BatteryCriticalTemperature::ChargeUnderTemp;
    else if (is_charging_over_temp_critical)
        return Tus::BatteryCriticalTemperature::ChargeOverTemp;
    else if (is_discharging_under_temp_critical)
        return Tus::BatteryCriticalTemperature::DischargeUnderTemp;
    else if (is_discharging_over_temp_critical)
        return Tus::BatteryCriticalTemperature::DischargeOverTemp;
    else
        return Tus::BatteryCriticalTemperature::TempOk;
}
