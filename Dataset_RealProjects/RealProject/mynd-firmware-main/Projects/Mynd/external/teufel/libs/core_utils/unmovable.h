#pragma once
#include <type_traits>
namespace Teufel::Core
{
class Unmovable
{
public:
  Unmovable() = default;

  Unmovable(const Unmovable&) = default;
  Unmovable& operator=(const Unmovable&) = default;

  Unmovable(Unmovable&&) = delete;
  Unmovable& operator=(Unmovable&&) = delete;
};

static_assert(std::is_copy_constructible<Unmovable>::value, "Unmovable should be copy-constructible.");
static_assert(std::is_copy_assignable<Unmovable>::value, "Unmovable should be copy-assignable.");
static_assert(!std::is_move_constructible<Unmovable>::value, "Unmovable must not be move-constructible.");
static_assert(!std::is_move_assignable<Unmovable>::value, "Unmovable must not be move-assignable.");
}
