#pragma once

#include <optional>
#include <tuple>

#if defined(TEUFEL_LOGGER)
#include "logger.h"
#else
#include "dbg_log.h"
#endif

/**
 * Power module is a tiny helper bit which provides primitives
 * for power state handling.
 * WARNING! These assumptions should be taken into account before start using
 * this module:
 * - PowerState type is defined as enum class with mandatory *Transition* field
 * - The source code(project) has to provide generic function getProperty<PowerState>()
 */

namespace Power
{
template <typename PowerState, typename Reason>
struct SetPowerState
{
    PowerState                to;
    std::optional<PowerState> from = std::nullopt;
    std::optional<Reason>     reason = std::nullopt;
#ifdef MCULINK
    // The field is used in TSSDK based projects and can be ignored when not used.
    std::optional<uint32_t> mculink_seq_num{};
#endif
};

template <typename PowerState>
struct SetPowerState<PowerState, void>
{
    PowerState                to;
    std::optional<PowerState> from = std::nullopt;

#ifdef MCULINK
    // The field is used in TSSDK based projects and can be ignored when not used.
    std::optional<uint32_t> mculink_seq_num{};
#endif
};

template <typename PowerState, PowerState TransitionState>
class GlobalPowerState
{
  public:
    GlobalPowerState() = delete;
    explicit GlobalPowerState(PowerState ps) noexcept
      : m_current(ps)
    {
    }

    void set(PowerState ps, const char *ps_desc = nullptr)
    {
        if (ps_desc)
            log_info("Power: set state: %s", ps_desc);

        if (ps == m_current)
        {
            log_warn("Power: (%s) already in the target state", __func__);
            return;
        }

        // static_assert(ps == TransitionState, "Please use setTransition function!");

        m_current = ps;
        m_from    = std::nullopt;
        m_to      = std::nullopt;
    }

    template <typename PowerStateChangeReason>
    SetPowerState<PowerState, PowerStateChangeReason> setTransition(PowerState to, PowerStateChangeReason reason, const char *ps_desc = nullptr)
    {
        if (m_current == TransitionState)
        {
            log_warn("Power: (%s) already in the transition state", __func__);
        }

        if (ps_desc)
            log_info("Power: set transition to: %s", ps_desc);

        m_to      = to;
        m_from    = m_current;
        m_current = TransitionState;

        return {.to = *m_to, .from = *m_from, .reason = reason};
    }

    SetPowerState<PowerState, void> setTransition(PowerState to, const char *ps_desc = nullptr)
    {
        if (m_current == TransitionState)
        {
            log_warn("Power: (%s) already in the transition state", __func__);
        }

        if (ps_desc)
            log_info("Power: set transition to: %s", ps_desc);

        m_to      = to;
        m_from    = m_current;
        m_current = TransitionState;

        return {.to = *m_to, .from = *m_from};
    }

    std::optional<SetPowerState<PowerState, void>> getTransitionMsg()
    {
        if (m_current != TransitionState)
        {
            log_err("Power: (%s) system not in the transition state", __func__);
            return {};
        }

        return SetPowerState<PowerState, void>{.to = *m_to, .from = *m_from};
    }

    template <typename PowerStateChangeReason>
    std::optional<SetPowerState<PowerState, PowerStateChangeReason>> getTransitionMsg()
    {
        if (m_current != TransitionState)
        {
            log_err("Power: (%s) system not in the transition state", __func__);
            return {};
        }

        return SetPowerState<PowerState, PowerStateChangeReason>{.to = *m_to, .from = *m_from, .reason = PowerStateChangeReason{}};
    }

    PowerState get() const
    {
        return m_current;
    }

    std::tuple<PowerState, std::optional<PowerState>, std::optional<PowerState>> getFull() const
    {
        return {m_current, m_from, m_to};
    }

  private:
    PowerState                m_current;
    std::optional<PowerState> m_from = std::nullopt;
    std::optional<PowerState> m_to   = std::nullopt;
};
}
