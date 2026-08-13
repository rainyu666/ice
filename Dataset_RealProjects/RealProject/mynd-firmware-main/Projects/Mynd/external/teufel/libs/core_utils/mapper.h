#pragma once

#include <optional>
#include <algorithm>
#include <iterator>

namespace Teufel::Core
{
template <typename Mapper, typename K>
static auto mapValue(Mapper &m, K k) -> std::optional<std::decay_t<decltype(std::begin(m)->value)>>
{
    auto res = std::find_if(std::begin(m), std::end(m), [&](const auto &e) { return e.key == k; });
    if (res != std::end(m))
        return res->value;
    return {};
}

template <typename V, typename Mapper, typename K>
static std::optional<V> mapValueReverse(Mapper &m, K k)
{
    auto res = std::find_if(std::begin(m), std::end(m), [&](const auto &e) { return e.value == k; });
    if (res != std::end(m))
        return std::optional<V>(res->key);
    return {};
}
}

#define TS_KEY_VALUE_CONST_MAP(name, key_type, value_type, ...) \
    typedef struct name##_                                   \
    {                                                        \
        key_type key{};                                      \
        value_type value{};                                  \
    } name##_t;                                              \
    static const name##_t name[] = { __VA_ARGS__ };
