#ifndef STATE_STATETYPES_HPP_
#define STATE_STATETYPES_HPP_

#include <types/stack.hpp>

namespace sword
{

namespace state
{

class State;

}; // namespace state

}; // namespace sword

using StateStack = sword::ReverseStack<sword::state::State*>;

#endif /* end of include guard: STATETYPES_HPP_ */
