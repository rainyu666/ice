#pragma once

#include <variant>
#include <type_traits>

namespace Teufel::Core
{

template <class T, class U> struct is_one_of;

template <class T, class... Ts>
struct is_one_of<T, std::variant<Ts...>>
  : std::bool_constant<(std::is_same_v<T, Ts> || ...)>
{ };

template <class T>
struct remove_cvref
{
    typedef std::remove_cv_t<std::remove_reference_t<T>> type;
};

/**
 * @brief The function returns true if the type has been seen before.
 * @note It _tracks_ known/initialized types and _mark_ them, such that the next call with the same type
 * returns true, which means that this type already has been seen by the function. The list of the known types
 * defined in the SyncedTypesVariant template parameter, which *must* be a std::variant.
 * @note One of the possible use cases: to filter out the initial values from the SOM(SDK) during the bootup.
 * @tparam T - type of the target variable
 * @tparam SyncedTypesVariant - a std::variant of the list of _tracked_ types
 * @return true if the type has been seen before(known), false otherwise
 */
template <typename T, typename SyncedTypesVariant>
bool isSynced(T)
{
    using type = typename remove_cvref<T>::type;

    if constexpr (is_one_of<type, SyncedTypesVariant>::value)
    {
        static bool is_synced = false;
        if (!is_synced)
        {
            is_synced = true;
            return false;
        }
        return true;
    }
    return false;
}

template <typename T>
bool isSynced(T)
{
    static bool is_synced = false;
    if (!is_synced)
    {
        is_synced = true;
        return false;
    }
    return true;
}

/**
 * @brief The function calls the *lambda* only once.
 * @note Please, keep in mind that it accept only lambdas, which are invocable and have no arguments.
 * @tparam C - type of the lambda
 * @param c - lambda to be called
 */
template <typename C>
void callOnce(C&& c)
{
    static_assert(std::is_class_v<C>, "must be a lambda type");
    static_assert(std::is_invocable_v<C>, "must be a lambda type");
    [[maybe_unused]] static bool is_called = (c(), true);
}

}