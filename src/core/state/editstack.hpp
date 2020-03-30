#ifndef EDITSTACK_HPP_
#define EDITSTACK_HPP_

#include <types/stack.hpp>

namespace sword
{

namespace state
{

class State;

class EditStack 
{
friend class LeafState;
public:
    void pushState(State* ptr) { stack.push(std::forward<State*>(ptr)); }
    auto begin() const { return stack.begin(); }
    auto end() const { return stack.end(); }
    void clear() { stack.clear(); }
    void popState() { stack.push(nullptr); }
private:
    ReverseStack<State*> stack;
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: EDITSTACK_HPP_ */
