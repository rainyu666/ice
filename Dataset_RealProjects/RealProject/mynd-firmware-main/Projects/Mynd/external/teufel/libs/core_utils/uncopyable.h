#pragma once
#include <type_traits>
namespace Teufel::Core
{
class Uncopyable
{
public:
  Uncopyable() = default;

  Uncopyable(const Uncopyable&) = delete;
  Uncopyable& operator=(const Uncopyable&) = delete;

  Uncopyable(Uncopyable&&) = default;
  Uncopyable& operator=(Uncopyable&&) = default;
};

static_assert(!std::is_copy_constructible<Uncopyable>::value, "Uncopyable must not be copy-constructible.");
static_assert(!std::is_copy_assignable<Uncopyable>::value, "Uncopyable must not be copy-assignable.");
static_assert(std::is_move_constructible<Uncopyable>::value, "Uncopyable should be move-constructible.");
static_assert(std::is_move_assignable<Uncopyable>::value, "Uncopyable should be move-assignable.");
}
