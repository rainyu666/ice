#pragma once

#include <array>
#include <cstdint>
#include <cmath>

#include "ux/system/system.h"

struct IPersistentStorage;

// Battery SOC Estimator, based on coulomb counting method.
// The SOC is calculated by integrating the current over time.
// The SOC is updated at a fixed rate, which is determined by the
// update period.
// The SOC is initialized to a default value, which is determined
// by the initial voltage of the battery.
// The SOC is bounded between 0 and 100.
// The SOC is updated by the following equation:
// SOC = SOC + (current * update_period) / capacity
// where:
// - SOC is the state of charge of the battery
// - current is the current flowing through the battery
// - update_period is the time between updates
// - capacity is the capacity of the battery
class SocEstimator
{
  public:
    SocEstimator() {}

    void add_sample(uint16_t battery_voltage_mv, int16_t battery_in_out_current_ma);

    uint8_t get_battery_level() const;

    /**
     * @brief Get the battery level based on the battery voltage.
     * @param[in] voltage_mv
     * @return uint8_t battery level in percentage
     */
    void stat() const;

    /**
     * @brief Initialize the SOC estimator.
     * Explicit initialization is required to set the initial state of the SOC estimator.
     */
    void init(uint16_t battery_voltage_mv);

    /**
     * @brief Reset the SOC estimator.
     * The SOC estimator is reset to the initial(Reset) state, and the persistent parameters are cleared.
     * @param[in] battery_voltage_mv The initial battery voltage in millivolts. (Required to calculate the inital CHG)
     */
    void reset(uint16_t battery_voltage_mv);

    void on_shutdown();
    void on_charge();
    void on_discharge();
    void save_persistent_parameters();

  private:
    struct vbat_to_charge
    {
      public:
        // To get from VBAT to LUT_code input:
        // apply_lut((VBAT – 6) / 2.4)
        static float convert(uint16_t vbat_mv)
        {
            auto lut_v = apply_lut((static_cast<float>(vbat_mv) / 1000.f - 6.f) / 2.4f, c_vbat_to_charge_lut.data(),
                                   c_vbat_to_charge_lut.size());
            return m_battery_factory_capacity * lut_v;
        }

      private:
        static constexpr std::array c_vbat_to_charge_lut{0.0f,    0.0013f, 0.0128f, 0.0528f, 0.1686f, 0.4544f,
                                                         0.6251f, 0.7484f, 0.8617f, 0.9635f, 1.0f,    1.0f};
    };

    struct charge_to_soc
    {
      public:
        static uint8_t convert(float charge, float capacity)
        {
            auto value = std::clamp(charge / capacity, 0.f, 1.f);
            auto lut_v = apply_lut(value, c_charge_to_soc_lut.data(), c_charge_to_soc_lut.size());
            return static_cast<uint8_t>(100.f * lut_v);
        }

      private:
        static constexpr std::array c_charge_to_soc_lut{0.0f,   0.0907f, 0.185f, 0.281f, 0.378f, 0.476f,
                                                        0.575f, 0.677f,  0.782f, 0.889f, 1.0f,   1.0f};
    };

    static float apply_lut(float value, const float *lut, uint8_t n)
    {
        // n is the number of elements in the LUT
        value *= (n - 2);
        auto lower_index = (int) floorf(value);
        auto upper_index = (int) ceilf(value);
        return std::lerp(lut[lower_index], lut[upper_index], value - lower_index);
    }

    uint16_t m_battery_voltage_mv = 0;

    float m_integrated_charge = 0.f; // Amperes * seconds

    static constexpr float m_battery_factory_capacity = 17'640.f; // Ampere * seconds (4,9.f; Ampere * hours)
    float                  m_capacity;                            // Ampere * seconds

    static constexpr uint16_t c_battery_voltage_mv_min = 6000u;
    static constexpr uint16_t c_battery_voltage_mv_max = 8400u;

    Teufel::Ux::System::BatterySoCAlgoState m_algo_state =
        Teufel::Ux::System::BatterySoCAlgoState::FirstStartOrBatteryReset;
};
