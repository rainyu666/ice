#pragma once

namespace Teufel::Core
{
// Simplified version of what is proposed for a future C++ standard:
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0051r3.pdf
// Each T has to be a type defining one or more operator() functions.

template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

}
