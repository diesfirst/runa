#ifndef _STATE_PAINTER_HPP
#define _STATE_PAINTER_HPP

#include "command/rendercommands.hpp"
#include "state.hpp"
#include <render/ubotypes.hpp>

namespace sword
{

namespace state
{

class Paint : public LeafState
{
public:
    const char* getName() const override { return "Paint"; }
    void handleEvent(event::Event*) override;
    Paint(StateArgs, Callbacks);
    void setFragInput(render::FragmentInput*);
private:
    render::FragmentInput* fragInput;
    CommandPool<command::Render>& pool;
};

class Painter final : public BranchState
{
public:
    const char* getName() const override { return "Painter"; }
    void handleEvent(event::Event*) override;
    Painter(StateArgs, Callbacks);
private:
    enum class Op : Option {init_basic, paint};

    Paint paint;

    void initBasic();

    render::FragmentInput fragInput;

    CommandPools& cp;
    const state::Register& sr;
};

}; // namespace state

}; // namespace sword

#endif /* end of include guard: _STATE_PAINTER_HPP */
