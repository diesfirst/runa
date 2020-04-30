#ifndef STATE_OPTION_HPP
#define STATE_OPTION_HPP

#include <bitset>
#include <functional>

namespace sword
{

namespace state
{

using OptionMask = std::bitset<32>;
using Option = uint8_t;
using Optional = std::optional<Option>;

template<typename T>
constexpr Option opcast(T op) {return static_cast<Option>(op);}

template<typename T>
constexpr T opcast(Option op) {return static_cast<T>(op);}

}; // namespace state

}; // namespace sword


#endif /* end of include guard: STATE_OPTION_HPP */
