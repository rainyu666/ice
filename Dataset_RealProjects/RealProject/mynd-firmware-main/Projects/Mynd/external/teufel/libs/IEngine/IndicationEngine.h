#pragma once

#include <algorithm>
#include <variant>
#include <optional>
#include <cstdio>
#include <atomic>
#include <limits>

#include "PatternSnippet.h"

namespace IndicationEngine
{

template <typename T = uint8_t>
constexpr T norm(float y, float min, float max, T brightness, T lowest_brightness)
{
    return static_cast<T>((brightness - lowest_brightness) * ((y - min) / (max - min)) + lowest_brightness);
}

// Takes a given list of indices and computes the input value for the index across the range
template <class Function, std::size_t... CounterSequence, typename T = uint8_t>
auto constexpr MakeLookUpTableHelper(Function f, const float _min, const float _stepSize, T brightness,
                                     T lowest_brightness, std::index_sequence<CounterSequence...>)
    -> std::array<T, sizeof...(CounterSequence)>
{
    const std::array<float, sizeof...(CounterSequence)> gen_array = {{f(_min + (CounterSequence * _stepSize))...}};

    const float min = *std::min_element(gen_array.begin(), gen_array.end());
    const float max = *std::max_element(gen_array.begin(), gen_array.end());

    return {{norm(gen_array.at(CounterSequence), min, max, brightness, lowest_brightness)...}};
}

template <int N, class Function, typename T = uint8_t>
auto constexpr MakeLookUpTable(Function f, const float _min, const float _max, const uint32_t _steps,
                               T brightness = 255U, T lowest_brightness = 0U) -> std::array<T, N>
{
    static_assert(N > 0, "The snippet duration must be a multiple of the minimum step in milliseconds.");
    return MakeLookUpTableHelper(f, _min, ((_max - _min) / static_cast<float>(_steps)), brightness, lowest_brightness,
                                 std::make_index_sequence<N>{});
}

template <std::size_t N, typename T, class Function>
void FillLookUpTable(Function f, std::array<T, N> &table, const float minValue, const float maxValue,
                     std::type_identity_t<T> maxBrightness = std::numeric_limits<T>::max(),
                     std::type_identity_t<T> minBrightness = 0U)
{
    // The -1 makes it so that the look up table includes the min and max values
    // For example, if you want to generate a look up table with for a pattern that goes from 0 to 100 in 3 steps,
    // the step size would need to be 50 (i.e numSteps = 2) so that the look up table is generated as [0, 50, 100]
    const auto stepSize = (maxValue - minValue) / static_cast<float>(N - 1);

    const auto computeCurve = [&](float value) { return f(value * stepSize + minValue); };

    constexpr auto normalize = [](float value, float min, float max) { return (value - min) / (max - min); };

    const auto toBrightness = [&](float value)
    { return static_cast<T>((maxBrightness - minBrightness) * value) + minBrightness; };

    // Find min and max computed values so we can normalize
    float minComputed = computeCurve(0.f);
    float maxComputed = computeCurve(0.f);

    for (auto i = 1; i < N; i++)
    {
        auto value = computeCurve(static_cast<float>(i));
        if (value < minComputed)
            minComputed = value;
        if (value > maxComputed)
            maxComputed = value;
    }

    for (auto i = 0; i < N; i++)
        table[i] = toBrightness(normalize(computeCurve(static_cast<float>(i)), minComputed, maxComputed));
}

template <uint8_t SIZE, typename T = uint8_t>
void dump_lookup_table(const std::array<T, SIZE> &array)
{
    for (const auto &e : array)
    {
        printf("%d ", e);
    }
    printf("\r\n");
}

// clang-format off
template <class... Ts> struct overloaded : Ts...{ using Ts::operator()...; };
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
// clang-format on

template <std::size_t LED_NUMS, typename T = uint8_t>
struct Engine
{
  private:
    enum class Stage
    {
        Preload,
        Core,
        Afterload
    };

    Stage m_stage;

    LedPattern<LED_NUMS, T> *m_pattern_preload   = nullptr;
    LedPattern<LED_NUMS, T> *m_pattern_afterload = nullptr;
    LedPattern<LED_NUMS, T> *m_pattern_core      = nullptr;

    std::array<T, LED_NUMS> m_latest_led_state;

    // clang-format off
    // Events
    struct event_run_once           { LedPattern<LED_NUMS, T> *pre; LedPattern<LED_NUMS, T> *p; LedPattern<LED_NUMS, T> *post; bool transient{false}; };
    struct event_run_few            { LedPattern<LED_NUMS, T> *pre; LedPattern<LED_NUMS, T> *p; LedPattern<LED_NUMS, T> *post; uint16_t cnt; bool transient{false}; };
    struct event_run_inf            { LedPattern<LED_NUMS, T> *pre; LedPattern<LED_NUMS, T> *p; LedPattern<LED_NUMS, T> *post; bool transient{false}; };
    struct event_finish             {};
    struct event_finish_gently      {};
    struct event_pattern_finished   { LedPattern<LED_NUMS, T> *p; };

    // States
    struct state_stopped            {};
    struct state_run_once           {};
    struct state_run_few            { uint16_t cnt; };
    struct state_run_inf            {};
    struct state_terminating        {};
    // clang-format on

    using state = std::variant<state_stopped, state_run_once, state_run_few, state_run_inf, state_terminating>;
    using event = std::variant<event_run_once, event_run_few, event_run_inf, event_finish, event_finish_gently,
                               event_pattern_finished>;

    state m_state;

    std::atomic<bool> mutex = true;

    void reset()
    {
        if (m_pattern_preload)
            m_pattern_preload->reset();
        if (m_pattern_core)
            m_pattern_core->reset();
        if (m_pattern_afterload)
            m_pattern_afterload->reset();
    }

    /* In order to support _smooth_ transitioning from one pattern to another,
     * the new/next patter has to start from the LED brightness as close as possible
     * to the current one. That means that we need to _scan_ for the nearest LED
     * brightness value in the next pattern and start from it rather than from the
     * beginning.
     * The function search through the pattern by calling step() function and determine
     * the index with the nearest value. It returns index(or amount of steps that needs
     * to be skipped. Since the direct access (by index) in a pattern encapsulate, we need
     * to call step() function as many times as many values we want to skip from the beginning
     * of the pattern. */
    uint16_t find_nearest(LedPattern<LED_NUMS, T> *p, const std::array<T, LED_NUMS> &reference_arr)
    {
        if (!p)
            return 0;

        uint16_t                min_i    = 0;
        T                       min_diff = std::numeric_limits<T>::max();
        T                       min_val  = std::numeric_limits<T>::max(); // for debugging
        PatternResult           s_res    = PatternResult::IN_PROGRESS;
        std::array<T, LED_NUMS> cur_arr  = {0};

        for (uint16_t i = 0; s_res == PatternResult::IN_PROGRESS; i++)
        {
            s_res = p->step(cur_arr);
            if (cur_arr == reference_arr)
            {
                // exact value was found!
                min_i   = i;
                min_val = cur_arr[0];
                break;
            }

            // TODO: reference_arr[0] > cur_arr[0] doesn't make a lot of sense to me here
            T diff = (cur_arr[0] > reference_arr[0]) ? cur_arr[0] - reference_arr[0] : reference_arr[0] > cur_arr[0];
            if (diff < min_diff)
            {
                min_diff = diff;
                min_i    = i;
                min_val  = cur_arr[0];
            }
        }

        // log_dbg("found closest index: %d, value: %d, prev: %d\r\n", min_i, min_val, reference_arr[0]);

        return min_i;
    }

    void dispatch(const event &e)
    {
        m_mutex                        = true;
        std::optional<state> new_state = std::visit(
            overloaded{
                [&](auto &, const event_run_once &e) -> std::optional<state>
                {
                    std::array<T, LED_NUMS> prev_arr = {0};
                    if constexpr (LED_NUMS == 1)
                    {
                        if (e.transient && is_running())
                        {
                            m_pattern_core->step(prev_arr);
                        }
                    }

                    /* No matter what state..., run_once command overwrites it. */
                    m_pattern_preload   = e.pre;
                    m_pattern_core      = e.p;
                    m_pattern_afterload = e.post;
                    reset();

                    if constexpr (LED_NUMS == 1)
                    {
                        if (e.transient && is_running())
                        {
                            auto idx = find_nearest(m_pattern_core, prev_arr);
                            // Do not forget to reset the pattern!
                            reset();

                            // Skip the first N steps to start the new pattern with the same brightness
                            std::array<T, LED_NUMS> skip_arr = {0};
                            for (uint16_t j = 0; j < idx; ++j)
                                m_pattern_core->step(skip_arr);
                        }

                        if (e.transient && !is_running())
                        {
                            // If the pattern is not running, we need to start it manually by
                            // getting the latest LED value.
                            prev_arr[0] = m_latest_led_state[0];
                            auto idx    = find_nearest(m_pattern_core, prev_arr);
                            // Do not forget to reset the pattern!
                            reset();

                            // Skip the first N steps to start the new pattern with the same brightness
                            std::array<T, LED_NUMS> skip_arr = {0};
                            for (uint16_t j = 0; j < idx; ++j)
                                m_pattern_core->step(skip_arr);
                        }
                    }

                    m_stage = m_pattern_preload == nullptr ? Stage::Core : Stage::Preload;
                    return state_run_once{};
                },
                [&](auto &, const event_run_few &e) -> std::optional<state>
                {
                    /* No matter what state..., run_few command overwrites it. */
                    m_pattern_preload   = e.pre;
                    m_pattern_core      = e.p;
                    m_pattern_afterload = e.post;
                    reset();
                    m_stage = m_pattern_preload == nullptr ? Stage::Core : Stage::Preload;
                    return state_run_few{
                        .cnt = e.cnt,
                    };
                },
                [&](auto &, const event_run_inf &e) -> std::optional<state>
                {
                    /* No matter what state..., run_inf command overwrites it. */
                    m_pattern_preload   = e.pre;
                    m_pattern_core      = e.p;
                    m_pattern_afterload = e.post;
                    reset();
                    m_stage = m_pattern_preload == nullptr ? Stage::Core : Stage::Preload;
                    return state_run_inf{};
                },
                [&](auto &, const event_finish &) -> std::optional<state>
                {
                    reset();
                    m_pattern_core = nullptr;
                    return state_stopped{};
                },
                [&](auto &s, const event_finish_gently &) -> std::optional<state>
                {
                    if constexpr (std::is_same_v<decltype(s), state_run_once &> ||
                                  std::is_same_v<decltype(s), state_run_few &> ||
                                  std::is_same_v<decltype(s), state_run_inf &>)
                    {
                        return state_terminating{};
                    }

                    // stopped, terminating
                    return std::nullopt;
                },
                [&](auto &s, const event_pattern_finished &) -> std::optional<state>
                {
                    /* The main lambda which handles events from the current pattern.
                     * Depends on the pattern, state and the last command can terminate,
                     * continue, or repeat a pattern. */
                    if constexpr (std::is_same_v<decltype(s), state_run_once &> ||
                                  std::is_same_v<decltype(s), state_terminating &>)
                    {
                        if (m_stage == Stage::Preload)
                        {
                            m_stage = Stage::Core;
                            return s;
                        }
                        else if (m_stage == Stage::Core)
                        {
                            if (m_pattern_afterload)
                            {
                                m_stage = Stage::Afterload;
                                return s;
                            }
                            else
                            {
                                reset();
                                m_pattern_core = nullptr;
                                return state_stopped{};
                            }
                        }
                        else
                        {
                            reset();
                            m_pattern_core = nullptr;
                            return state_stopped{};
                        }
                    }
                    else if constexpr (std::is_same_v<decltype(s), state_run_few &>)
                    {
                        if (m_stage == Stage::Preload)
                        {
                            m_stage = Stage::Core;
                            return s;
                        }
                        else if (m_stage == Stage::Core)
                        {
                            if (--s.cnt)
                            {
                                return s;
                            }
                            else
                            {
                                // TODO: who check preload/afterload: exec() or visiter?
                                if (m_pattern_afterload)
                                {
                                    m_stage = Stage::Afterload;
                                    return s;
                                }
                                else
                                {
                                    reset();
                                    m_pattern_core = nullptr;
                                    return state_stopped{};
                                }
                            }
                        }
                        else
                        {
                            reset();
                            m_pattern_core = nullptr;
                            return state_stopped{};
                        }
                    }
                    else if constexpr (std::is_same_v<decltype(s), state_run_inf &>)
                    {
                        if (m_stage == Stage::Preload)
                            m_stage = Stage::Core;
                    }

                    // stopped, run_inf
                    return std::nullopt;
                },

                /* Default case was removed intentionally in order to support compile-time variants check. */
            },
            m_state, e);
        if (new_state)
            m_state = *std::move(new_state);
        m_mutex = false;
    }

    [[maybe_unused]] void dump_leds(std::array<T, LED_NUMS> arr)
    {
        for (const auto &a : arr)
            printf("%d ", a);
        printf("\r\n");
    }

    /* TODO: This flag must be more then just variable to support the proper
     * multi-threading control. There is no obvious/simple solution...
     * The challenge is to have platform independent flag from one side,
     * and system independent option from the other side. For example Cortex-M0
     * doesn't support atomic operations in the low-level instructions,
     * and FreeRTOS/AnyOS is not an option because bare-metal firmwares(such as bootloaders)
     * needs to be taken into account. */

    bool m_mutex = false;

  public:
    explicit Engine()
    {
        m_latest_led_state.fill(0);
        m_state = state_stopped{};
    }

    Engine(const Engine &)             = delete;
    Engine(const Engine &&)            = delete;
    Engine &operator=(const Engine &)  = delete;
    Engine &operator=(const Engine &&) = delete;

    // clang-format off
    // TODO: add const?
    int run_once(LedPattern<LED_NUMS, T> &p) { dispatch(event_run_once{.pre = nullptr, .p = &p, .post = nullptr}); return 0; }
    int run_once_transient(LedPattern<LED_NUMS, T> &p) { dispatch(event_run_once{.pre = nullptr, .p = &p, .post = nullptr, .transient = true}); return 0; }
    int run_once_with_preload(LedPattern<LED_NUMS, T> &p, LedPattern<LED_NUMS, T> &pre) { dispatch(event_run_once{.pre = &pre, .p = &p, .post = nullptr}); return 0; }
    int run_once_with_postload(LedPattern<LED_NUMS, T> &p, LedPattern<LED_NUMS, T> &post) { dispatch(event_run_once{.pre = nullptr, .p = &p, .post = &post}); return 0; }
    int run_once_with_preload_and_postload(LedPattern<LED_NUMS, T> &p, LedPattern<LED_NUMS, T> &pre, LedPattern<LED_NUMS, T> &post) { dispatch(event_run_once{.pre = &pre, .p = &p, .post = &post}); return 0; }
    int run_few(LedPattern<LED_NUMS, T> &p, uint16_t cnt) { dispatch(event_run_few{.pre = nullptr, .p = &p, .post = nullptr, .cnt = cnt}); return 0; }
    int run_few_with_preload(LedPattern<LED_NUMS, T> &p, LedPattern<LED_NUMS, T> &pre, uint16_t cnt) { dispatch(event_run_few{.pre = &pre, .p = &p, .post = nullptr, .cnt = cnt}); return 0; }
    int run_few_with_postload(LedPattern<LED_NUMS, T> &p, LedPattern<LED_NUMS, T> &post, uint16_t cnt) { dispatch(event_run_few{.pre = nullptr, .p = &p, .post = &post, .cnt = cnt}); return 0; }
    int run_few_with_preload_and_postload(LedPattern<LED_NUMS, T> &p, LedPattern<LED_NUMS, T> &pre, LedPattern<LED_NUMS, T> &post, uint16_t cnt) { dispatch(event_run_few{.pre = &pre, .p = &p, .post = &post, .cnt = cnt}); return 0; }
    int run_inf(LedPattern<LED_NUMS, T> &p) { dispatch(event_run_inf{.pre = nullptr, .p = &p, .post = nullptr}); return 0; }
    int run_inf_with_preload(LedPattern<LED_NUMS, T> &p, LedPattern<LED_NUMS, T> &pre) { dispatch(event_run_inf{.pre = &pre, .p = &p, .post = nullptr}); return 0; }
    int run_inf_with_preload_and_postload(LedPattern<LED_NUMS, T> &p, LedPattern<LED_NUMS, T> &pre, LedPattern<LED_NUMS, T> &post) { dispatch(event_run_inf{.pre = &pre, .p = &p, .post = &post}); return 0; }
    int finish() { dispatch(event_finish{}); return 0; }
    int finish_gently() { dispatch(event_finish_gently{}); return 0; }

    int finish_and_reset_led_values() { dispatch(event_finish{}); m_latest_led_state.fill(0); return 0; }
    // clang-format on

    std::array<T, LED_NUMS> exec()
    {
        if (m_mutex)
            return m_latest_led_state;

        if (std::get_if<state_stopped>(&m_state))
            return m_latest_led_state;

        std::array<T, LED_NUMS> arr = {0};
        PatternResult           res;

        // if (m_pattern_core == nullptr)
        //     return;

        switch (m_stage)
        {
            case Stage::Preload:
                res = m_pattern_preload == nullptr ? PatternResult::FINISHED : m_pattern_preload->step(arr);
                break;
            case Stage::Core:
                res = m_pattern_core->step(arr);
                break;
            case Stage::Afterload:
                res = m_pattern_afterload == nullptr ? PatternResult::FINISHED : m_pattern_afterload->step(arr);
                break;
            default:
                res = PatternResult::FINISHED;
        }

        if (arr != m_latest_led_state)
        {
            m_latest_led_state = arr;
        }
        if (res == PatternResult::FINISHED)
            dispatch(event_pattern_finished{});

        // Debug led array
        // dump_leds(arr);

        return arr;
    }

    bool is_running()
    {
        /*
         * TODO: NOP instruction here is a dirty hack to eliminate compiler optimization.
         * This function needs to be investigated for a strong optimization, e.g. O3+lto.
         */
        asm("nop");

        if (std::get_if<state_stopped>(&m_state))
            return false;
        return true;
    }

    bool is_running(LedPattern<LED_NUMS, T> &p)
    {
        if (!is_running())
            return false;

        if (auto p_core_pattern_id = this->getPatternId(); p_core_pattern_id)
            return p_core_pattern_id.value() == p.getPatternId();
        return false;
    }

    [[nodiscard]] std::optional<uint8_t> getPatternId() const
    {
        if (std::get_if<state_stopped>(&m_state))
            return {};
        if (!m_pattern_core)
            return {};
        return m_pattern_core->getPatternId();
    }
};
}
