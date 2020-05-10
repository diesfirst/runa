#ifndef _STATE_PAINTER_HPP
#define _STATE_PAINTER_HPP

#include "state.hpp"

namespace sword
{

namespace state
{

class Painter final : public BranchState
{
public:
    const char* getName() const override { return "Painter"; }
    void handleEvent(event::Event*) override;
    Painter(StateArgs, Callbacks);
private:
    enum class Op : Option {init_basic};

    void initBasic();

    CommandPools& cp;
    const state::Register& sr;
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: _STATE_PAINTER_HPP */
