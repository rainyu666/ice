#pragma once

#include <cstring>
#include <memory>
#include <optional>
#include <type_traits>

#if defined(TEUFEL_LOGGER)
#include "logger.h"
#else
#include "dbg_log.h"
#endif

#include "core_utils/uncopyable.h"

struct IMutex
{
    void (*lock)();
    void (*unlock)();
};

struct PropertyMutex
{
    static IMutex *mutex;
};

// Initialize the mutex with a default value
inline IMutex *PropertyMutex::mutex = nullptr;

enum class PropertyType
{
    Optional,
    NonOptional
};

template <typename T, PropertyType PT, typename Enable = void>
class _Property;

// Alias for property with optional value
template <typename T>
using PropertyOpt = _Property<T, PropertyType::Optional>;

// Alias for property with optional value
template <typename T>
using Property = PropertyOpt<T>;

// Alias for property with non-optional value
template <typename T>
using PropertyNonOpt = _Property<T, PropertyType::NonOptional>;

/* String type */
template <size_t N, PropertyType PT>
class _Property<std::array<uint8_t, N>, PT> : public Teufel::Core::Uncopyable
{
  public:
    // Property w initial value
    _Property(const char *name, const char *default_value, const char *initial_value)
      : m_name(name)
      , m_default_value(default_value)
    {
        if (strlen(initial_value) > N)
            log_err("Property (%s) set: out of range: (%d, %d)", initial_value, N, strlen(initial_value));

        strncpy(m_value.data(), initial_value, N);
        m_is_set = true;
    }

    // Property w/o initial value
    _Property(const char *name, const char *default_value)
      : m_name(name)
      , m_default_value(default_value)
    {
        m_is_set = false;
    }

    ~_Property() = default;

    [[nodiscard]] std::optional<char *> get()
    {
        if (PropertyMutex::mutex)
            PropertyMutex::mutex->lock();

        std::optional<char *> v;
        if (m_is_set)
            v = {std::optional<char *>(m_value.data())};
        else
            v = {};

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->unlock();

        return v;
    }

    void set(char *array)
    {
        if (strlen(array) > N)
            log_err("Property (%s) set: out of range: (%d, %d)", m_name, N, strlen(array));

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->lock();

        strncpy(m_value.data(), array, N);
        m_is_set = true;

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->unlock();

        log_info("Property (%s) set: %s", m_name, m_value.data());
    }

  private:
    const char         *m_name;          /*!< name specialisation */
    const char         *m_default_value; /*!< default value after initialisation */
    std::array<char, N> m_value;
    bool                m_is_set;
};

template <PropertyType PT>
class _Property<bool, PT> : public Teufel::Core::Uncopyable
{
  public:
    // Property w initial value
    _Property(const char *name, bool default_value, bool initial_value)
      : m_name(name)
      , m_default_value(default_value)
      , m_value(initial_value)
    {
    }

    // Property w/o initial value
    _Property(const char *name, bool default_value)
        requires(PT == PropertyType::Optional)
      : m_name(name)
      , m_default_value(default_value)
    {
    }

    ~_Property() = default;

    [[nodiscard]] auto get() const
    {
        if (PropertyMutex::mutex)
            PropertyMutex::mutex->lock();

        auto v = m_value;

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->unlock();

        return v;
    }

    void set(bool v)
    {
        if (m_value.has_value() && *m_value == v)
            return;

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->lock();

        m_value = v;

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->unlock();

        log_info("Property (%s) set: %d", m_name, v);
    }

    void set_default()
    {
        if (PropertyMutex::mutex)
            PropertyMutex::mutex->lock();

        m_value = m_default_value;

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->unlock();

        log_info("Property (%s) set: %d", m_name, m_default_value);
    }

    void invalidate()
        requires(PT == PropertyType::Optional)
    {
        if (PropertyMutex::mutex)
            PropertyMutex::mutex->lock();

        m_value = std::nullopt;

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->unlock();

        log_info("Property (%s) invalidate", m_name);
    }

  private:
    const char         *m_name;          /*!< name specialisation */
    bool                m_default_value; /*!< default value after initialisation */
    std::optional<bool> m_value = std::nullopt;
};

/* Arithmetic type */
template <typename T, PropertyType PT>
class _Property<T, PT, typename std::enable_if<std::is_arithmetic<T>::value>::type> : public Teufel::Core::Uncopyable
{
  public:
    // Property w initial value
    _Property(const char *name, T min, T max, T step, T default_value, T initial_value)
      : m_name(name)
      , m_min(min)
      , m_max(max)
      , m_step(step)
      , m_default_value(default_value)
      , m_value(initial_value)
    {
    }

    // Property w/o initial value
    _Property(const char *name, T min, T max, T step, T default_value)
        requires(PT == PropertyType::Optional)
      : m_name(name)
      , m_min(min)
      , m_max(max)
      , m_step(step)
      , m_default_value(default_value)
    {
    }

    ~_Property() = default;

    [[nodiscard]] constexpr auto get() const
    {
        if (PropertyMutex::mutex)
            PropertyMutex::mutex->lock();

        auto v = m_value;

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->unlock();

        return v;
    }

    void set(T v)
    {
        if (v > m_max || v < m_min)
        {
            log_err("Property (%s) set: out of range", m_name);
            return;
        }

        if (m_value.has_value() && *m_value == v)
            return;

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->lock();

        m_value = v;

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->unlock();

        if constexpr (std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> || std::is_same_v<T, int32_t> ||
                      std::is_same_v<T, int64_t>)
        {
            log_info("Property (%s) set: %d", m_name, v);
        }
        else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>)
        {
            log_info("Property (%s) set: %f", m_name, v);
        }
        else
        {
            log_info("Property (%s) set: %u", m_name, v);
        }
    }

    void set_default()
    {
        if (PropertyMutex::mutex)
            PropertyMutex::mutex->lock();

        m_value = m_default_value;

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->unlock();

        log_info("Property (%s) set: %u", m_name, m_default_value);
    }

    void invalidate()
        requires(PT == PropertyType::Optional)
    {
        if (PropertyMutex::mutex)
            PropertyMutex::mutex->lock();

        m_value = std::nullopt;

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->unlock();

        log_info("Property (%s) invalidate", m_name);
    }

    constexpr T get_min() const
    {
        return m_min;
    }

    constexpr T get_max() const
    {
        return m_max;
    }

    constexpr T get_step() const
    {
        return m_step;
    }

    constexpr T get_count() const
    {
        // Floating points are not supported!
        static_assert(!std::is_same_v<T, float>);
        static_assert(!std::is_same_v<T, double>);

        return (m_max - m_min) / m_step + 1;
    }

  private:
    const char      *m_name; /*!< name specialisation */
    T                m_min;
    T                m_max;
    T                m_step;
    T                m_default_value; /*!< default value (used after set_default() call) */
    std::optional<T> m_value = std::nullopt;
};

/* Enum type */
template <typename T, PropertyType PT>
class _Property<T, PT, typename std::enable_if<std::is_enum<T>::value>::type> : public Teufel::Core::Uncopyable
{
  public:
    _Property(const char *name, T default_value, T initial_value)
      : m_name(name)
      , m_default_value(default_value)
      , m_value(initial_value)
    {
    }

    _Property(const char *name, T default_value)
        requires(PT == PropertyType::Optional)
      : m_name(name)
      , m_default_value(default_value)
    {
    }

    ~_Property() = default;

    std::optional<T> get() const
    {
        if (PropertyMutex::mutex)
            PropertyMutex::mutex->lock();

        std::optional<T> v = m_value;

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->unlock();

        return v;
    }

    void set(T v)
    {
        if (m_value.has_value() && *m_value == v)
            return;

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->lock();

        m_value = v;

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->unlock();

        log_info("Property (%s) set: %u", m_name, v);
    }

    void set(T v, const char *desc)
    {
        if (m_value.has_value() && *m_value == v)
            return;

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->lock();

        m_value = v;

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->unlock();

        log_info("Property (%s) set: %s", m_name, desc);
    }

    void set_default()
    {
        if (PropertyMutex::mutex)
            PropertyMutex::mutex->lock();

        m_value = m_default_value;

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->unlock();

        log_info("Property (%s) set: %u", m_name, m_default_value);
    }

    void invalidate()
        requires(PT == PropertyType::Optional)
    {
        if (PropertyMutex::mutex)
            PropertyMutex::mutex->lock();

        m_value = std::nullopt;

        if (PropertyMutex::mutex)
            PropertyMutex::mutex->unlock();

        log_info("Property (%s) invalidate", m_name);
    }

  private:
    const char      *m_name;
    T                m_default_value;
    std::optional<T> m_value = std::nullopt;
};

// clang-format off

// TODO: It'd probably be nicer if we had the macros as TYPE, VARIABLE instead, without the namespace,
//       and have the caller use TS_GET_PROPERTY_FN(TYPE, NAMESPACE::VARIABLE) instead of TS_GET_PROPERTY_FN(NAMESPACE, VARIABLE, TYPE)

#define TS_GET_PROPERTY_FN(NAMESPACE, VARIABLE, TYPE) \
    std::optional<TYPE> getProperty(TYPE*) { \
      return NAMESPACE::VARIABLE.get().has_value() ? std::optional<TYPE>{{NAMESPACE::VARIABLE.get().value()}} : std::nullopt; \
    }

#define TS_GET_PROPERTY_NON_OPT_FN(NAMESPACE, VARIABLE, TYPE) \
    TYPE getProperty(TYPE*) { \
      return TYPE {NAMESPACE::VARIABLE.get().value()}; \
    }

#define TS_GET_PROPERTY_MIN_MAX_FN(NAMESPACE, VARIABLE, TYPE) \
    std::pair<decltype(TYPE::value), decltype(TYPE::value)> getPropertyMinMax(TYPE*) { \
        return std::make_pair(NAMESPACE::VARIABLE.get_min(), NAMESPACE::VARIABLE.get_max()); \
    }

#define TS_GET_PROPERTY_STEP_FN(NAMESPACE, VARIABLE, TYPE) \
    decltype(TYPE::value) getPropertyStep(TYPE*) { \
        return NAMESPACE::VARIABLE.get_step(); \
    }
// clang-format on
